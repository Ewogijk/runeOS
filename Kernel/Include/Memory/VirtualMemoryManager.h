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

#ifndef RUNEOS_VIRTUALMEMORYMANAGER_H
#define RUNEOS_VIRTUALMEMORYMANAGER_H

#include <Memory/Paging.h>
#include <Memory/PhysicalMemoryManager.h>
#include <Memory/VirtualMemory.h>

namespace Rune::Memory {

#define VMMStartFailures(X)                                                                        \
    X(VMMStartFailure, BASE_PT_ALLOC_FAIL, 0x1)                                                    \
    X(VMMStartFailure, HHDM_MAPPING_FAIL, 0x2)                                                     \
    X(VMMStartFailure, KERNEL_CODE_MAPPING_FAIL, 0x3)                                              \
    X(VMMStartFailure, PMM_MAPPING_FAIL, 0x4)                                                      \
    X(VMMStartFailure, KERNEL_HEAP_MAPPING_FAIL, 0x5)

    DECLARE_ENUM(VMMStartFailure, VMMStartFailures, 0x0) // NOLINT

    /**
     * Result of allocating pages for a kernel memory region.
     */
    struct KernelSpaceEntryAllocResult {
        const char*     region    = "";
        bool            has_error = false;
        PageTableAccess alloc_pta;
        PageTableAccess free_pta;
        bool            claim_error = false;
    };

    /**
     * The Virtual Memory Allocator is responsible for allocating/freeing pages in a virtual address
     * space and managing whole virtual address spaces (VAS) which includes creating new VAS's and
     * swapping a current VAS for another existing VAS.
     */
    class VirtualMemoryManager {
        PhysicalMemoryManager* _pmm;
        SharedPointer<Logger>  _logger;

        VirtualAddr _user_space_end;

        VMMStartFailure             _start_fail;
        KernelSpaceEntryAllocResult _ksear;

        // Map a coherent physical address range to a virtual address range and claim the region in
        // the virtual memory map
        KernelSpaceEntryAllocResult allocate_kernel_space_entries(const Memory::PageTable& base_pt,
                                                                  VirtualAddr              v_start,
                                                                  const MemoryRegion&      p_reg,
                                                                  U16                      flags,
                                                                  MemoryRegionType claim_type,
                                                                  MemoryMap*       v_map,
                                                                  const char*      region_name);

        bool free_virtual_address_space_rec(const PageTableEntry& pte);

      public:
        VirtualMemoryManager(PhysicalMemoryManager* pmm);

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //
        // Functions
        //
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        /**
         * Load the initial virtual address space (VAS) using the given physical memory map, virtual
         * memory map and virtual kernel space layout. The VAS will contain mappings for the higher
         * half direct map (HHDM) of the physical memory, kernel code, physical memory manager and
         * kernel heap.
         *
         * <p>
         *  The PMM reserved and kernel code area sizes are determined by the sizes of the pmm
         * memory index and kernel size. Whereas the size of the HHDM is determined by the physical
         * memory address range, this means the size of the HHDM will be the largest possible
         * physical address. Lastly the kernel heap will have the requested `kernelHeapSize`.
         * </p>
         *
         * <p>
         *  The starting addresses of said regions is determined by the given `kSpaceLayout` and
         * combined with the sizes, the resulting areas will be claimed in the given `VMap`. The
         * `PMap` is used to get the locations of all regions in the physical memory.
         * </p>
         *
         * @param pMap           Physical memory map used to create page table entries.
         * @param vMap           Virtual memory map
         * @param kSpaceLayout   Kernel space layout.
         * @param kernelHeapSize Requested kernel size.
         *
         * @return True if the initial VAS got loaded else false.
         */
        [[nodiscard]]
        VMMStartFailure start(MemoryMap*        p_map,
                              MemoryMap*        v_map,
                              KernelSpaceLayout k_space_layout,
                              MemorySize        heap_size);

        /**
         *
         * @param logger
         */
        void set_logger(SharedPointer<Logger> logger);

        /**
         *
         * @return The last user space memory address.
         */
        [[nodiscard]]
        VirtualAddr get_user_space_end() const;

        /**
         * allocate a new virtual address space containing the kernel space index page tables and an
         * empty user space. If the allocation succeeds the physical address of the base page table
         * will be assigned to `basePTAddr`.
         *
         * @param base_pt_addr On success the physical address of the created base page table will
         * be assigned to this address else the value of the variable is undefined.
         *
         * @return True if the new virtual address space got created else false.
         */
        bool allocate_virtual_address_space(PhysicalAddr& base_pt_addr);

        /**
         * free the user space of the virtual address space determined by the base page table loaded
         * at `basePTAddr`.
         *
         * @param base_pt_addr Physical address of the base page table that will be freed.
         *
         * @return True if the user space of the virtual address space got freed else false.
         */
        bool free_virtual_address_space(PhysicalAddr base_pt_addr);

        /**
         * Load the base page table into the core CPU register if it is not already loaded.
         *
         * @param base_pt_addr Physical address of the base page table.
         */
        void load_virtual_address_space(PhysicalAddr base_pt_addr);

        /**
         * Try to allocate a page in the base page table loaded by the CPU for the page aligned
         * `vAddr` using the `pFlags` to a page frame which will be requested from the physical
         * memory manager.
         *
         * @param vAddr    Virtual address of a page to map to a page frame.
         * @param flags    Page flags.
         *
         * @return True if the allocation succeeded, false if the page is already allocated or out
         * of physical memory.
         */
        bool allocate(VirtualAddr v_addr, U16 flags);

        /**
         * Try to allocate `pages` pages in the base page table loaded by the CPU for the page
         * aligned `vAddr` using the `pFlags` to a page frame which will be requested from the
         * physical memory manager.
         *
         * @param vAddr Virtual address of a page to map to a page frame.
         * @param flags Page flags.
         * @param pages Number of pages to allocate.
         *
         * @return True if the allocation succeeded, false if the page is already allocated or out
         * of physical memory.
         */
        bool allocate(VirtualAddr v_addr, U16 flags, size_t pages);

        /**
         * Try to free the page aligned `vAddr` in the base page table loaded by the CPU by removing
         * the mappings in the page tables.
         *
         * @param vAddr Virtual address of a page to free.
         *
         * @return True if the free succeeded, false if the virtual address is not mapped or the
         * physical memory manager fails to free the associated page frame.
         */
        bool free(VirtualAddr v_addr);

        /**
         * Try to free `pages` pages aligned `vAddr` in the base page table loaded by the CPU by
         * removing the mappings in the page tables.
         *
         * @param vAddr Virtual address of a page to free.
         * @param pages Number of pages to free.
         *
         * @return True if the free succeeded, false if the virtual address is not mapped or the
         * physical memory manager fails to free the associated page frame.
         */
        bool free(VirtualAddr v_addr, size_t pages);
    };
} // namespace Rune::Memory

#endif // RUNEOS_VIRTUALMEMORYMANAGER_H
