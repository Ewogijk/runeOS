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

#include <Memory/PhysicalMemoryManager.h>

#include <KernelRuntime/Math.h>


namespace Rune::Memory {
    constexpr char const* FILE = "Physical Memory Manager";


    DEFINE_ENUM(PMMStartFailure, PMM_START_FAILURES, 0x0)


    bool PhysicalMemoryManager::detect_memory_range() {
        _mem_base = (PhysicalAddr) -1;
        PhysicalAddr mem_end = 0;

        for (auto& reg: *_mem_map) {
            if (reg.memory_type == MemoryRegionType::NONE)
                continue;

            if (reg.start < _mem_base)
                _mem_base = reg.start;

            if (reg.end() > mem_end)
                mem_end = reg.end();
        }

        if (mem_end == 0)
            return false;
        _mem_size = (mem_end - _mem_base) / _page_size;
        return true;
    }


    PageFrameIndex PhysicalMemoryManager::to_page_frame(PhysicalAddr addr) const {
        return (addr - _mem_base) / _page_size;
    }


    PageFrameIndex PhysicalMemoryManager::to_page_frame_round_up(PhysicalAddr addr) const {
        return div_round_up((addr - _mem_base), _page_size);
    }


    PhysicalAddr PhysicalMemoryManager::to_address(PageFrameIndex page_frame) const {

        return _mem_base + page_frame * _page_size;
    }


    PhysicalMemoryManager::PhysicalMemoryManager() :
            _page_size(0),
            _mem_base(0),
            _mem_size(0),
            _mem_map(nullptr),
            _init(false),
            _start_fail(PMMStartFailure::NONE),
            _largest_free_block(0) {

    }


    PMMStartFailure PhysicalMemoryManager::start(
            MemoryMap* mem_map,
            U64 page_size,
            VirtualAddr memory_index_offset

    ) {
        _page_size = page_size;
        _mem_map   = mem_map;
        if (!detect_memory_range()) {
            _start_fail = PMMStartFailure::MEMORY_RANGE_DETECTION_FAILED;
            return _start_fail;
        }

        MemorySize pmm_b_mem_req    = compute_memory_index_size();
        auto             pmm_data_start_p = (PhysicalAddr) -1;
        for (auto& reg: *_mem_map) {
            if (reg.memory_type == MemoryRegionType::USABLE && reg.size >= pmm_b_mem_req) {
                pmm_data_start_p = reg.start;
                break;
            }
            if (reg.size > _largest_free_block)
                _largest_free_block = reg.size;
        }
        if (pmm_data_start_p == (PhysicalAddr) -1) {
            _start_fail = PMMStartFailure::OUT_OF_MEMORY;
            return _start_fail;
        }

        if (!init0(pmm_data_start_p + memory_index_offset, pmm_data_start_p)) {
            _start_fail = PMMStartFailure::MEMORY_INDEX_INIT_FAILED;
            return _start_fail;
        }

        return PMMStartFailure::NONE;
    }


    MemoryRegion PhysicalMemoryManager::get_managed_memory() const {
        return MemoryRegion{
                _mem_base,
                _mem_base - 1 + _mem_size * _page_size, // will overflow because it will be max value of some type e.g. U32
                MemoryRegionType::RESERVED
        };
    }


    void PhysicalMemoryManager::log_start_routine_phases() const {
        _logger->info(
                FILE,
                "Detected physical memory range: {:0=#16x}-{:0=#16x}",
                _mem_base,
                _mem_base - 1 + _mem_size * _page_size // will overflow because it will be max value of some type e.g. U32
        );
        MemoryRegion mem_idx = get_memory_index_region();
        _logger->info(
                FILE,
                "Physical memory index region: {:0=#16x}-{:0=#16x} (Size: {} bytes)",
                mem_idx.start,
                mem_idx.end(),
                mem_idx.size
        );
        _logger->info(FILE, "Memory index can be accessed at virtual address: {:0=#16x}", get_memory_index());
    }


    bool PhysicalMemoryManager::allocate(PhysicalAddr& p_addr) {
        return allocate(p_addr, 1);
    }


    bool PhysicalMemoryManager::allocate_explicit(PhysicalAddr p_addr) {
        return allocate_explicit(p_addr, 1);
    }


    bool PhysicalMemoryManager::free(PhysicalAddr p_addr) {
        return free(p_addr, 1);
    }


    void PhysicalMemoryManager::set_logger(SharedPointer<Logger> logger) {
        _logger = move(logger);
    }
}