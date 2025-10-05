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

#include <Memory/MemorySubsystem.h>

#include <KRE/Memory.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                   Kernel Runtime Support
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

Rune::Memory::MemorySubsystem* MEM_SUBSYS;

void* operator new(size_t size) { return MEM_SUBSYS->get_heap()->allocate(size); }

void* operator new[](size_t size) { return MEM_SUBSYS->get_heap()->allocate(size); }

void* operator new(size_t count, void* ptr) {
    SILENCE_UNUSED(count)
    return ptr;
}

void operator delete(void* p) noexcept { MEM_SUBSYS->get_heap()->free(p); }

void operator delete(void* p, size_t size) noexcept {
    SILENCE_UNUSED(size);
    MEM_SUBSYS->get_heap()->free(p);
}

void operator delete[](void* p) noexcept { MEM_SUBSYS->get_heap()->free(p); }

void operator delete[](void* p, size_t size) noexcept {
    SILENCE_UNUSED(size);
    MEM_SUBSYS->get_heap()->free(p);
}

namespace Rune::Memory {
    const SharedPointer<Logger> LOGGER = LogContext::instance().get_logger("Memory.MemorySubsystem");

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Subsystem
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    MemorySubsystem::MemorySubsystem()
        : Subsystem(),
          _p_map({}),
          _v_map({}),
          _pmm(),
          _vmm(&_pmm),
          _heap(),
          _boot_loader_mem_claim_failed(false) {}

    String MemorySubsystem::get_name() const { return "Memory"; }

    bool MemorySubsystem::start(const BootLoaderInfo&    boot_info,
                                const SubsystemRegistry& k_subsys_reg) {
        SILENCE_UNUSED(k_subsys_reg)
        _p_map = boot_info.physical_memory_map;
        _v_map = create_virtual_memory_map();

        KernelSpaceLayout k_space_layout = get_virtual_kernel_space_layout();
        // Init pmm
        if (_pmm.start(&_p_map, get_page_size(), k_space_layout.higher_half_direct_map)
            != PMMStartFailure::NONE)
            return false;

        // Init vmm
        init_paging(boot_info.physical_address_width);
        if (_vmm.start(&_p_map, &_v_map, k_space_layout, 128 * (MemorySize) MemoryUnit::MiB)
            != VMMStartFailure::NONE)
            return false;

        // Adjust pmm to new virtual memory space
        _pmm.relocate_memory_index(k_space_layout.pmm_reserved);
        if (!_pmm.claim_boot_loader_reclaimable_memory()) {
            _boot_loader_mem_claim_failed = true;
            return false;
        }
        _p_map.merge();

        if (_heap.start(&_v_map, &_vmm) != HeapStartFailureCode::NONE) return false;

        MEM_SUBSYS = this;
        return true;
    }

    MemoryMap& MemorySubsystem::get_physical_memory_map() { return _p_map; }

    MemoryMap& MemorySubsystem::get_virtual_memory_map() { return _v_map; }

    PhysicalMemoryManager* MemorySubsystem::get_physical_memory_manager() { return &_pmm; }

    VirtualMemoryManager* MemorySubsystem::get_virtual_memory_manager() { return &_vmm; }

    SlabAllocator* MemorySubsystem::get_heap() { return &_heap; }

    void MemorySubsystem::log_start_routine_phases() const {
        LOGGER->debug("The bootloader reclaimable memory has been claimed.");

        MemoryRegion managed = _pmm.get_managed_memory();
        LOGGER->debug("Detected physical memory range: {:0=#16x}-{:0=#16x}",
                      managed.start,
                      managed.end());
        MemoryRegion memIdx = _pmm.get_memory_index_region();
        LOGGER->debug("Physical memory index region: {:0=#16x}-{:0=#16x} (MemorySize: {} bytes)",
                      memIdx.start,
                      memIdx.end(),
                      memIdx.size);
        LOGGER->debug("Memory index can be accessed at virtual address: {:0=#16x}",
                      _pmm.get_memory_index());

        LOGGER->debug("The base page table is located at physical address: {:0=#16x}",
                      get_base_page_table_address());

        LOGGER->debug("Bootstrap caches are initialized.");
        LOGGER->debug(
            "General purpose and DMA caches are initialized. MemorySize range: {}-{} bytes.",
            _heap.get_min_cache_size(),
            _heap.get_max_cache_size());
    }
} // namespace Rune::Memory