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

#ifndef RUNEOS_PHYSICALMEMORYMANAGER_H
#define RUNEOS_PHYSICALMEMORYMANAGER_H


#include <Ember/Enum.h>
#include <Hammer/Utility.h>

#include <LibK/Logging.h>
#include <LibK/KMemory.h>


namespace Rune::Memory {
    using PageFrameIndex = U32;

    /**
     * Reasons why the start of the pmm can fail.
     */
#define PMM_START_FAILURES(X)                               \
    X(PMMStartFailure, MEMORY_RANGE_DETECTION_FAILED, 0x1)  \
    X(PMMStartFailure, OUT_OF_MEMORY, 0x2)                  \
    X(PMMStartFailure, MEMORY_INDEX_INIT_FAILED, 0x3)       \



    DECLARE_ENUM(PMMStartFailure, PMM_START_FAILURES, 0x0) //NOLINT


    /**
     * The physical memory manager allocates and frees page frames. It also ensures that reserved memory regions
     * are not accidentally freed.
     */
    class PhysicalMemoryManager {
        bool detect_memory_range();


    protected:
        U64                _page_size;    // LibK::MemSize of a page frame
        LibK::PhysicalAddr _mem_base;     // Address of the memory start
        U32                _mem_size;     // Memory size in page frames
        LibK::MemoryMap* _mem_map;              // Memory map, required for memory protection
        bool _init;

        PMMStartFailure _start_fail;
        U32             _largest_free_block;

        SharedPointer<LibK::Logger> _logger;


        [[nodiscard]] PageFrameIndex to_page_frame(LibK::PhysicalAddr addr) const;


        [[nodiscard]] PageFrameIndex to_page_frame_round_up(LibK::PhysicalAddr addr) const;


        [[nodiscard]] LibK::PhysicalAddr to_address(PageFrameIndex page_frame) const;


        virtual LibK::MemorySize compute_memory_index_size() = 0;


        virtual bool init0(LibK::VirtualAddr memory_index, LibK::PhysicalAddr p_memory_index) = 0;


    public:
        PhysicalMemoryManager();


        /**
         * The physical memory manager (pmm) initialization will include the following steps:
         * <ol>
         *  <li>Detect the pmm managed memory range defined by the lowest and highest address in the physical
         *      memory map.</li>
         *  <li>Search the first usable memory region that has enough space to fit the memory index (MI).</li>
         *  <li>Initialization of the MI which is implementation dependent.</li>
         * <ol>
         *
         * <p>
         * The `memoryIndexOffset` is the memory offset towards the virtual memory address that will be used by the pmm
         * to access the MI.
         * </p>
         *
         * @param pMap              Physical memory map.
         * @param page_size          Page frame size.
         * @param memory_index_offset Address offset until the address that will be used to access the memory index.
         *
         * @return True if the each step of the initialization was successful.
         */
        [[nodiscard]] PMMStartFailure start(
                LibK::MemoryMap* mem_map,
                U64 page_size,
                LibK::VirtualAddr memory_index_offset
        );


        /**
         *
         * @param logger
         */
        void set_logger(SharedPointer<LibK::Logger> logger);


        /**
         * Log the intermediate steps of the start routine.
         */
        void log_start_routine_phases() const;


        /**
         *
         * @return The managed physical memory region.
         */
        LibK::MemoryRegion get_managed_memory() const;


        /**
         * After the first initialization phase ended successfully the region will contain a valid size and if the
         * second initialization phase ended successfully it will also contain a valid start address.
         *
         * @return The physical memory region where the memory index is saved.
         */
        [[nodiscard]] virtual LibK::MemoryRegion get_memory_index_region() const = 0;


        /**
         *
         * @return The virtual address where the memory index can be accessed.
         */
        [[nodiscard]] virtual LibK::VirtualAddr get_memory_index() const = 0;


        /**
         * Swap out the current memory index location with the new `memoryIndex`.
         *
         * @param memory_index Start address of the memory region where memory index can be accessed.
         */
        virtual void relocate_memory_index(LibK::VirtualAddr memory_index) = 0;


        /**
         * Make the bootloader reclaimable memory regions usable.
         *
         * @return True if the memory is usable else false.
         */
        virtual bool claim_boot_loader_reclaimable_memory() = 0;


        /**
         * Try to allocate a single page frame and save the physical start address of it in the given pAddr.
         *
         * @param p_addr The start address of the page frame will be saved to the variable on successful allocation.
         *
         * @return True if the allocation succeeded, false if not enough physical memory is available.
         */
        bool allocate(LibK::PhysicalAddr& p_addr);


        /**
         * Try to allocate the requested number of page frames which are guaranteed to be consecutive in the memory.
         * The start address of the memory region is saved to the pAddr variable.
         *
         * @param pAddr  The start address of the page frame will be saved to the variable on successful allocation.
         * @param frames Number of requested page frames.
         *
         * @return True if the allocation succeeded, false if not enough physical memory is available.
         */
        virtual bool allocate(LibK::PhysicalAddr& pAddr, size_t frames) = 0;


        /**
         * Try to allocate the the page frame at the specified physical address.
         *
         * @param p_addr The start address of the page frame will be saved to the variable on successful allocation.
         *
         * @return True if the allocation succeeded, false if not enough physical memory is available.
         */
        bool allocate_explicit(LibK::PhysicalAddr p_addr);


        /**
         * Try to allocate the requested number of page frames starting at the specified physical address which are
         * guaranteed to be consecutive in the memory.
         *
         * @param pAddr The start address of the page frame will be saved to the variable on successful allocation.
         * @param frames Number of requested page frames.
         *
         * @return True if the allocation succeeded, false if not enough physical memory is available.
         */
        virtual bool allocate_explicit(LibK::PhysicalAddr pAddr, size_t frames) = 0;


        /**
         * Try to free a single page frame with the given start address.<br>
         *
         * @param p_addr Physical address of the page frame to be freed.
         *
         * @return True if the free succeeded, false if not enough physical memory is available.
         */
        bool free(LibK::PhysicalAddr p_addr);


        /**
         * Try to free a the requested number of consecutive page frames with the given start address.
         *
         * @param p_addr Physical address of the page frame to be freed.
         *
         * @return True if the free succeeded, false if not enough physical memory is available.
         */
        virtual bool free(LibK::PhysicalAddr p_addr, size_t frames) = 0;


        /**
         * Try to read the status pmm managed memory region from `start` to `end` into the given `buf` until `bufSize`
         * regions have been inserted. Consecutive regions free or used regions will be merged to a single region
         * before being inserted into the `buf`.
         *
         * <p>
         * The memory regions inserted into the buffer may not fully represent the requested memory region if the
         * `start` is not page frame aligned it will be rounded down to the next page frame aligned memory address, the
         * `end` address will be rounded up to the next page frame aligned memory address.
         * </p>
         *
         * <p>
         * If `start` or `end` lie outside of the pmm managed memory region than nothing will be read into the buffer.
         * </p>
         *
         * @param buf      Buffer.
         * @param buf_size Buffer size.
         * @param start    Start address of the memory region.
         * @param end      End address of the memory region.
         *
         * @return Number regions read into the buffer.
         */
        virtual size_t read_page_frame_states(
                LibK::MemoryRegion* buf,
                size_t buf_size,
                LibK::PhysicalAddr start,
                LibK::PhysicalAddr end
        ) = 0;
    };
}

#endif //RUNEOS_PHYSICALMEMORYMANAGER_H
