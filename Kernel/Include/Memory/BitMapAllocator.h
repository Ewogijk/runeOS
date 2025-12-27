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

#ifndef RUNEOS_BITMAPALLOCATOR_H
#define RUNEOS_BITMAPALLOCATOR_H

#include <Memory/PhysicalMemoryManager.h>

namespace Rune::Memory {
    /**
     * The bitmap allocator stores the state of each page frame in a bitmap where bit i saves the
     * state of page frame i.
     */
    class BitMapAllocator : public PhysicalMemoryManager {
        U8*          _bitmap{nullptr}; // Base address where the bitmap is accessed.
        PhysicalAddr _p_bitmap{0};     // Base address where the bitmap is stored.
        uint32_t     _bitmap_size{0};  // MemorySize of the bitmap in bytes.

        auto is_free(uint32_t page_frame) -> bool;

        void mark(uint32_t page_frame, bool in_use);

        auto mark_memory_block(uint32_t base, uint32_t size, bool in_use) -> bool;

        auto mark_memory_region(PhysicalAddr base_bytes, U64 size_bytes, bool in_use) -> bool;

        auto find_free_region(size_t frames) -> uint32_t;

        auto is_reserved_or_bit_map_address(PhysicalAddr p_addr, size_t frames) -> int;

      protected:
        auto compute_memory_index_size() -> MemorySize override;

        auto init0(VirtualAddr memory_index, PhysicalAddr p_memory_index) -> bool override;

      public:
        BitMapAllocator();

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //
        // Physical Memory Manager Overrides
        //
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        using PhysicalMemoryManager::allocate;
        using PhysicalMemoryManager::allocate_explicit;
        using PhysicalMemoryManager::free;

        [[nodiscard]] auto get_memory_index_region() const -> MemoryRegion override;

        [[nodiscard]] auto get_memory_index() const -> VirtualAddr override;

        void relocate_memory_index(VirtualAddr memory_index) override;

        auto claim_boot_loader_reclaimable_memory() -> bool override;

        auto allocate(PhysicalAddr& p_addr, size_t frames) -> bool override;

        auto allocate_explicit(PhysicalAddr p_addr, size_t frames) -> bool override;

        auto free(PhysicalAddr p_addr, size_t frames) -> bool override;

        auto read_page_frame_states(MemoryRegion* buf,
                                    size_t        buf_size,
                                    PhysicalAddr  start,
                                    PhysicalAddr  end) -> size_t override;
    };
} // namespace Rune::Memory

#endif // RUNEOS_BITMAPALLOCATOR_H
