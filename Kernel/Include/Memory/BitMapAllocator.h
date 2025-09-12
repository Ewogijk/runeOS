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
     * The bitmap allocator stores the state of each page frame in a bitmap where bit i saves the state of page frame i.
     */
    class BitMapAllocator : public PhysicalMemoryManager {
        U8*          _bitmap;      // Base address where the bitmap is accessed.
        PhysicalAddr _p_bitmap;    // Base address where the bitmap is stored.
        uint32_t     _bitmap_size; // MemorySize of the bitmap in bytes.

        bool is_free(uint32_t page_frame);

        void mark(uint32_t page_frame, bool in_use);

        bool mark_memory_block(uint32_t base, uint32_t size, bool in_use);

        bool mark_memory_region(PhysicalAddr base_bytes, U64 size_bytes, bool in_use);

        uint32_t find_free_region(size_t frames);

        int is_reserved_or_bit_map_address(PhysicalAddr p_addr, size_t frames);

      protected:
        MemorySize compute_memory_index_size() override;

        bool init0(VirtualAddr memory_index, PhysicalAddr p_memory_index) override;

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

        [[nodiscard]]
        MemoryRegion get_memory_index_region() const override;

        [[nodiscard]]
        VirtualAddr get_memory_index() const override;

        void relocate_memory_index(VirtualAddr memory_index) override;

        bool claim_boot_loader_reclaimable_memory() override;

        bool allocate(PhysicalAddr& p_addr, size_t frames) override;

        bool allocate_explicit(PhysicalAddr p_addr, size_t frames) override;

        bool free(PhysicalAddr p_addr, size_t frames) override;

        size_t
        read_page_frame_states(MemoryRegion* buf, size_t buf_size, PhysicalAddr start, PhysicalAddr end) override;
    };
} // namespace Rune::Memory

#endif // RUNEOS_BITMAPALLOCATOR_H
