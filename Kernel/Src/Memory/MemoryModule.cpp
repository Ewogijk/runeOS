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

#include <Memory/MemoryModule.h>

#include <KRE/Memory.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                   Kernel Runtime Support
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

Rune::Memory::MemoryModule* MEM_MODULE; // NOLINT

auto operator new(size_t size) -> void* { return MEM_MODULE->get_heap()->allocate(size); }

auto operator new[](size_t size) -> void* { return MEM_MODULE->get_heap()->allocate(size); }

auto operator new(size_t count, void* ptr) -> void* {
    SILENCE_UNUSED(count)
    return ptr;
}

void operator delete(void* p) noexcept { MEM_MODULE->get_heap()->free(p); }

void operator delete(void* p, size_t size) noexcept {
    SILENCE_UNUSED(size);
    MEM_MODULE->get_heap()->free(p);
}

void operator delete[](void* p) noexcept { MEM_MODULE->get_heap()->free(p); }

void operator delete[](void* p, size_t size) noexcept {
    SILENCE_UNUSED(size);
    MEM_MODULE->get_heap()->free(p);
}

namespace Rune::Memory {
    const SharedPointer<Logger> LOGGER = LogContext::instance().get_logger("Memory.MemoryModule");

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Subsystem
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    MemoryModule::MemoryModule()
        : _p_map({}),
          _v_map({}),
          _vmm(&_pmm),
          _boot_loader_mem_claim_failed(false) {}

    auto MemoryModule::get_name() const -> String { return "Memory"; }

    auto MemoryModule::load(const BootInfo& boot_info) -> bool {
        _p_map = boot_info.physical_memory_map;
        _v_map = create_virtual_memory_map();

        KernelSpaceLayout k_space_layout = get_virtual_kernel_space_layout();
        // Init pmm
        if (_pmm.start(&_p_map, get_page_size(), k_space_layout.higher_half_direct_map)
            != PMMStartFailure::NONE)
            return false;

        // Init vmm
        init_paging(boot_info.physical_address_width);
        if (_vmm.start(&_p_map, &_v_map, k_space_layout, HEAP_SIZE * (MemorySize) MemoryUnit::MiB)
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

        MEM_MODULE = this;
        return true;
    }

    auto MemoryModule::get_physical_memory_map() -> MemoryMap& { return _p_map; }

    auto MemoryModule::get_virtual_memory_map() -> MemoryMap& { return _v_map; }

    auto MemoryModule::get_physical_memory_manager() -> PhysicalMemoryManager* { return &_pmm; }

    auto MemoryModule::get_virtual_memory_manager() -> VirtualMemoryManager* { return &_vmm; }

    auto MemoryModule::get_heap() -> SlabAllocator* { return &_heap; }

    void MemoryModule::log_post_load() const {
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