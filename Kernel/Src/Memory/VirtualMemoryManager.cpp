/*
 *  Copyright 2025 Ewogijk
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <Memory/VirtualMemoryManager.h>

#include <Memory/Paging.h>
#include <Memory/VirtualMemory.h>

namespace Rune::Memory {
    constexpr char const* FILE = "VirtualMemoryManager";

    DEFINE_ENUM(VMMStartFailure, VMMStartFailures, 0x0)

    KernelSpaceEntryAllocResult VirtualMemoryManager::allocate_kernel_space_entries(const Memory::PageTable& base_pt,
                                                                                    VirtualAddr              v_start,
                                                                                    const MemoryRegion&      p_reg,
                                                                                    U16                      flags,
                                                                                    MemoryRegionType         claim_type,
                                                                                    MemoryMap*               v_map,
                                                                                    const char* region_name) {
        MemorySize      page_size   = get_page_size();
        MemorySize      alloc_limit = 0;
        PageTableAccess alloc_pta;
        for (MemorySize i = 0; i < p_reg.size; i += page_size) {
            alloc_pta = allocate_page(base_pt, v_start + i, p_reg.start + i, flags, _pmm);
            if (alloc_pta.status != PageTableAccessStatus::OKAY) {
                alloc_limit = i;
                break;
            }
        }
        PageTableAccess free_pta;
        free_pta.status = PageTableAccessStatus::OKAY;
        if (alloc_limit > 0) {
            for (MemorySize i = 0; i < alloc_limit; i += page_size) {
                free_pta = free_page(base_pt, v_start + i, _pmm);
                if (free_pta.status != PageTableAccessStatus::OKAY) break;
            }

            return {region_name, true, alloc_pta, free_pta, false};
        }

        MemoryRegion r = {v_start, p_reg.size, claim_type};
        return {region_name, false, alloc_pta, free_pta, v_map->claim(r, get_page_size())};
    }

    bool VirtualMemoryManager::free_virtual_address_space_rec(const PageTableEntry& pte) {
        if (pte.level > 0) {
            // LN-L1 page table -> First recursively free all entries in the page table
            // then afterward free the page frame of the page table.
            PageTable pt(pte.native_entry,
                         (NativePageTableEntry*) Memory::physical_to_virtual_address(pte.get_address()),
                         pte.level);
            // Important: If this is a base page table we only free the first half of VAS, the user mode memory.
            // We do not want to free the kernel mode memory!! Never!! The kernel mode memory is shared across all
            // VAS's -> If it is freed in one VAS it is freed in all other VAS, thus making the whole system unusable!
            U16 free_limit = PageTable::get_size();
            if (pt.is_base_page_table()) free_limit /= 2;
            for (U16 i = 0; i < free_limit; i++) {
                PageTableEntry ptee = pt[i];
                if (ptee.is_present()) {
                    if (!free_virtual_address_space_rec(ptee)) return false;
                    pt.update(i, 0x0);
                }
            }

            // Do not free the base page table!
            if (pt.is_base_page_table()) return true;
        }

        // Free the page frame of the page table
        _logger->trace(FILE, "Freeing page frame {:0=#16x}.", pte.get_address());
        if (!_pmm->free(pte.get_address())) {
            _logger->warn(FILE, "Failed to free page frame {:0=#16x}", pte.get_address());
            return false;
        }
        return true;
    }

    VirtualMemoryManager::VirtualMemoryManager(PhysicalMemoryManager* pmm)
        : _pmm(pmm),
          _user_space_end(0),
          _start_fail(VMMStartFailure::NONE),
          _ksear() {}

    VMMStartFailure VirtualMemoryManager::start(MemoryMap*        p_map,
                                                MemoryMap*        v_map,
                                                KernelSpaceLayout k_space_layout,
                                                MemorySize        heap_size) {
        U16          p_flags = PageFlag::PRESENT | PageFlag::WRITE_ALLOWED;
        PhysicalAddr base_pt_addr;
        if (!_pmm->allocate(base_pt_addr)) {
            _start_fail = VMMStartFailure::BASE_PT_ALLOC_FAIL;
            return _start_fail;
        }
        memset((void*) physical_to_virtual_address(base_pt_addr), 0, get_page_size()); // Also initializes user space
        auto base_pt = interp_as_base_page_table(base_pt_addr);

        MemoryRegion kernel_code;
        MemorySize   hhdm_size = 0x0;
        for (auto& reg : *p_map) {
            if (reg.memory_type == MemoryRegionType::KERNEL_CODE) kernel_code = reg;
            if (reg.end() > hhdm_size) hhdm_size = reg.end();
        }

        // Create higher half direct map kernel space entries
        MemoryRegion                hhdm  = {0x0, hhdm_size};
        KernelSpaceEntryAllocResult ksear = allocate_kernel_space_entries(base_pt,
                                                                          k_space_layout.higher_half_direct_map,
                                                                          hhdm,
                                                                          p_flags,
                                                                          MemoryRegionType::HHDM,
                                                                          v_map,
                                                                          "Higher Half Direct Map");
        if (ksear.has_error) {
            _start_fail = VMMStartFailure::HHDM_MAPPING_FAIL;
            _ksear      = ksear;
            return _start_fail;
        }

        // Create PMM reserved kernel space entries
        MemoryRegion pmm_bk = _pmm->get_memory_index_region();
        ksear               = allocate_kernel_space_entries(base_pt,
                                              k_space_layout.pmm_reserved,
                                              pmm_bk,
                                              p_flags,
                                              MemoryRegionType::PMM_RESERVED,
                                              v_map,
                                              "Physical Memory Manager");
        if (ksear.has_error) {
            _start_fail = VMMStartFailure::PMM_MAPPING_FAIL;
            _ksear      = ksear;
            return _start_fail;
        }

        // No page frame allocation because the heap grows dynamically
        MemoryRegion kernel_heap = {k_space_layout.kernel_heap, heap_size, MemoryRegionType::KERNEL_HEAP};
        if (!v_map->claim(kernel_heap, get_page_size())) {
            _start_fail = VMMStartFailure::KERNEL_HEAP_MAPPING_FAIL;
            _ksear      = {"Kernel Heap", true, {PageTableAccessStatus::OKAY}, {PageTableAccessStatus::OKAY}, true};
            return _start_fail;
        }

        // Create kernel code kernel space entries
        ksear = allocate_kernel_space_entries(base_pt,
                                              k_space_layout.kernel_code,
                                              kernel_code,
                                              p_flags,
                                              MemoryRegionType::KERNEL_CODE,
                                              v_map,
                                              "Kernel Code");
        if (ksear.has_error) {
            _start_fail = VMMStartFailure::KERNEL_CODE_MAPPING_FAIL;
            _ksear      = ksear;
            return _start_fail;
        }

        for (auto& reg : *v_map) {
            if (reg.memory_type == MemoryRegionType::USERSPACE) {
                _user_space_end = Memory::to_canonical_form(reg.end());
                break;
            }
        }
        load_base_page_table(base_pt_addr);
        flush_tlb();
        return VMMStartFailure::NONE;
    }

    void VirtualMemoryManager::set_logger(SharedPointer<Logger> logger) { _logger = move(logger); }

    VirtualAddr VirtualMemoryManager::get_user_space_end() const { return _user_space_end; }

    bool VirtualMemoryManager::allocate_virtual_address_space(PhysicalAddr& base_pt_addr) {
        PhysicalAddr base_addr;
        if (!_pmm->allocate(base_addr)) {
            _logger->critical(FILE, "L0 page table allocation error.");
            return false;
        }
        PageTable new_base_pt    = interp_as_base_page_table(base_addr);
        PageTable loaded_base_pt = get_base_page_table();

        for (size_t i = 0; i < PageTable::get_size(); i++) {
            NativePageTableEntry b_pte = 0x0;
            if (i >= PageTable::get_size() / 2) {
                PageTableEntry pte = loaded_base_pt[i];
                if (pte.is_present()) {
                    b_pte = pte.native_entry;
                }
            }
            new_base_pt.update(i, b_pte);
        }

        base_pt_addr = base_addr;
        return true;
    }

    bool VirtualMemoryManager::free_virtual_address_space(PhysicalAddr base_pt_addr) {
        return free_virtual_address_space_rec(interp_as_base_page_table(base_pt_addr).to_page_table_entry());
    }

    void VirtualMemoryManager::load_virtual_address_space(PhysicalAddr base_pt_addr) {
        if (base_pt_addr != get_base_page_table_address()) {
            PageTable new_base_pt    = interp_as_base_page_table(base_pt_addr);
            PageTable loaded_base_pt = get_base_page_table();
            for (size_t i = PageTable::get_size() / 2; i < PageTable::get_size(); i++)
                new_base_pt.update(i, loaded_base_pt[i].native_entry);

            load_base_page_table(base_pt_addr);
            flush_tlb();
        }
    }

    bool VirtualMemoryManager::allocate(VirtualAddr v_addr, U16 flags) {
        PhysicalAddr pAddr;
        if (!_pmm->allocate(pAddr)) {
            _logger->warn(FILE, "Page allocation fail: Out of physical memory for page.");
            return false;
        }
        PageTable base_pt = get_base_page_table();
        if (allocate_page(base_pt, v_addr, pAddr, flags, _pmm).status != PageTableAccessStatus::OKAY) {
            _logger->warn(FILE, "Page allocation fail: {:0=#16x}", v_addr);
            if (!_pmm->free(pAddr)) {
                _logger->warn(FILE, "Page allocation fail: Failed to free page frame of page.");
            }
            return false;
        }
        return true;
    }

    bool VirtualMemoryManager::allocate(VirtualAddr v_addr, U16 flags, size_t pages) {
        MemorySize page_size  = get_page_size();
        size_t     alloc_fail = 0;
        for (size_t i = 0; i < pages; i++) {
            if (!allocate(v_addr + (i * page_size), flags)) {
                _logger->warn(FILE,
                              "Page allocation fail: Failed to allocate page {:0=#16x} ({}/{})",

                              v_addr + (i * page_size),
                              i + 1,
                              pages + 1);
                alloc_fail = i;
                break;
            }
        }
        if (alloc_fail > 0) {
            for (size_t i = 0; i < alloc_fail; i++) {
                if (!free(v_addr + (i * page_size))) {
                    _logger->warn(FILE, "Failed to free {:0=#16x}", v_addr + (i * page_size));
                }
            }
            return false;
        }
        return true;
    }

    bool VirtualMemoryManager::free(VirtualAddr v_addr) {
        PageTable       base_pt = get_base_page_table();
        PageTableAccess pta     = free_page(base_pt, v_addr, _pmm);
        if (pta.status == PageTableAccessStatus::FREE_ERROR) {
            _logger->warn(FILE, "Page free fail: Failed to free {:0=#16x}", v_addr);
            return false;
        }
        return true;
    }

    bool VirtualMemoryManager::free(VirtualAddr v_addr, size_t pages) {
        MemorySize page_size = get_page_size();
        bool       all_free  = true;
        for (size_t i = 0; i < pages; i++) {
            if (!free(v_addr + (i * page_size))) {
                _logger->warn(FILE,
                              "Page free fail: Failed to allocate page {:0=#16x} ({}/{})",

                              v_addr + (i * page_size),
                              i + 1,
                              pages + 1);
                all_free = false;
            }
        }
        return all_free;
    }
} // namespace Rune::Memory
