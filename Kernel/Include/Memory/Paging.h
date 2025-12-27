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

#ifndef RUNEOS_PAGING_H
#define RUNEOS_PAGING_H

#include <Ember/Enum.h>

#include <KRE/Collections/Array.h>

#include <KRE/BitsAndBytes.h>
#include <KRE/Memory.h>

#include <Memory/PhysicalMemoryManager.h>

namespace Rune::Memory {

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Page Table Entry
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * @brief A native page table entry (PTE) is simply the numerical entry in a page table.
     */
    using NativePageTableEntry = VirtualAddr;

    /**
     * @brief A page table entry (PTE) acts as an architecture independent interface to the
     * architecture dependant paging implementation, a native page table entry (NPTE).
     *
     * <p>
     *  A PTE exposes all paging properties supported by the kernel in an architecture independent
     * way, that is the NPTE could have more properties that are not explicitly supported by the
     * kernel.
     * </p>
     * <p>
     *  The Level describes the position of this PTE in the page table hierarchy starting from the
     * base page table (BPT). The BTP is the entry point to the page table hierarchy, e.g. in x86_64
     * 4-level paging the BTP would be the Page-Map Level-4 Table.<br> Counting starts from the
     * highest level down to zero, where zero is effectively the physical page offset. This means
     * that practically the page table hierarchy ends at level 1 (L1), since L0 is is not a PTE. If
     * level=0xFF it indicates that is PTE is invalid and should not be used.
     * </p>
     * <p>
     *  Lastly the PTE exposes the NPTE which can be used to create page table objects.
     * </p>
     */
    struct PageTableEntry {
        static constexpr U8  BAD_LEVEL    = 0xFF;
        NativePageTableEntry native_entry = 0x0;
        U8                   level        = BAD_LEVEL;

        /**
         *
         * @return True if the PTE is used else false.
         */
        [[nodiscard]] auto is_present() const -> bool;

        /**
         *
         * @return True if the PTE has been accessed else false.
         */
        [[nodiscard]] auto is_accessed() const -> bool;

        /**
         *
         * @return True if the PTE has been changed else false.
         */
        [[nodiscard]] auto is_dirty() const -> bool;

        /**
         *
         * @return True if data can be written to the PTE else false.
         */
        [[nodiscard]] auto is_write_allowed() const -> bool;

        /**
         *
         * @return True if the PTE can be accessed by a userspace process else false.
         */
        [[nodiscard]] auto is_user_mode_access_allowed() const -> bool;

        /**
         * @brief
         * @return True: The PTE points to a page frame, False: The PTE points to a page table.
         */
        [[nodiscard]] auto is_pointing_to_page_frame() const -> bool;

        /**
         *
         * @return If the PTE points to a page frame, return its address else the address will point
         * to another page table.
         */
        [[nodiscard]] auto get_address() const -> PhysicalAddr;

        /**
         * @brief
         * @return The native flags.
         */
        [[nodiscard]] auto get_flags() const -> U16;
    };

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Page Table
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * @brief A page table (PT) exposes an architecture independent way of iterating and accessing
     * the page table hierarchy.
     */
    class PageTable {
        NativePageTableEntry  _npte;
        NativePageTableEntry* _pt;
        U8                    _level;

      public:
        explicit PageTable(NativePageTableEntry n_pte, NativePageTableEntry* pt, U8 level);

        /**
         * @brief
         * @return Number of entries in the page table.
         */
        [[nodiscard]] static auto get_size() -> U16;

        /**
         * @brief
         * @return True: This is a base page table, False: Nope.
         */
        [[nodiscard]] auto is_base_page_table() const -> bool;

        /**
         * @brief The native page table entry as found in the PT that references this page table.
         *
         * In case of the Base PT, all flags in NPTE will be zero.
         * @return The NPTE of this PT.
         */
        [[nodiscard]] auto get_native_page_table_entry() const -> NativePageTableEntry;

        /**
         * @brief A level of 0xFF indicates an invalid PTE
         * @return The level in the page table hierarchy.
         */
        [[nodiscard]] auto get_level() const -> U8;

        /**
         * @brief
         * @return Convert the page table to a page table entry.
         */
        [[nodiscard]] auto to_page_table_entry() const -> PageTableEntry;

        /**
         * @brief Get the page table entry at idx.
         * @param idx
         * @return A page table entry.
         */
        [[nodiscard]] auto operator[](U16 idx) const -> PageTableEntry;

        /**
         * @brief Interpret the PTE at idx as PT.
         * @param idx
         * @return A PTE interpreted as PT.
         */
        [[nodiscard]] auto entry_as_page_table(U16 idx) const -> PageTable;

        /**
         * @brief Update the PTE at idx with the new native PTE value.
         * @param idx
         * @param n_pte
         */
        void update(size_t idx, NativePageTableEntry n_pte);
    };

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Paging Configuration
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * @brief Load a new base page table for the CPU.
     *
     * Important note: This will invalidate all currently used pages! The new base page table must
     * at least have the kernel pages mapped, otherwise the system will crash immediately.
     */
    CLINK void load_base_page_table(PhysicalAddr base_pt);

    /**
     * @brief Flush the TLB entries given page.
     */
    CLINK void invalidate_page(VirtualAddr page);

    /**
     * @brief Flush all entries in the TLB.
     */
    CLINK void flush_tlb();

    /**
     * @return The size of a page in bytes.
     */
    auto get_page_size() -> MemorySize;

    /**
     * @brief
     * @param physical_address_width
     */
    void init_paging(U8 physical_address_width);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      Page Table Hierarchy Access
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     *
     * @return The physical address of the base PT loaded in the CPU.
     */
    CLINK auto get_base_page_table_address() -> PhysicalAddr;

    /**
     * @brief Interpret the given physical address as the base page table for another virtual
     * address space that is not loaded at the moment.
     * @param p_addr Physical address of a base page table.
     * @return Base page table from the physical address.
     */
    auto interp_as_base_page_table(PhysicalAddr p_addr) -> PageTable;

    /**
     * Get the currently used base page table which is the entry point to the page table hierarchy,
     * thus is never referenced by another page table.
     *
     * @return The core page table.
     */
    auto get_base_page_table() -> PageTable;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      Virtual Address Manipulations
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * @brief If the given virtual address is not in canonical form, convert it to its canonical
     * form.
     * @param v_addr
     * @return The virtual address in canonical form.
     */
    auto to_canonical_form(VirtualAddr v_addr) -> VirtualAddr;

    /**
     *
     * @param p_addr Physical Address.
     *
     * @return The virtual address pointing to the physical address.
     */
    auto physical_to_virtual_address(PhysicalAddr p_addr) -> VirtualAddr;

    /**
     *
     * @param v_addr Virtual address.
     * @param out   If true is returned, the variable will contain the physical address for the
     * given virtual address; otherwise, the value is undefined.
     *
     * @return True if the physical address mapped to the virtual address has been found.
     */
    auto virtual_to_physical_address(VirtualAddr v_addr, PhysicalAddr& p_addr_out) -> bool;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                  Page Table Hierarchy Manipulations
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * A page flag represents a property of a page table entry.
     */
#define PAGE_FLAGS(X)                                                                              \
    X(PageFlag, PRESENT, 0x01)                                                                     \
    X(PageFlag, WRITE_ALLOWED, 0x02)                                                               \
    X(PageFlag, USER_MODE_ACCESS, 0x04)                                                            \
    X(PageFlag, WRITE_THROUGH, 0x08)                                                               \
    X(PageFlag, CACHE_DISABLE, 0x10)                                                               \
    X(PageFlag, ACCESSED, 0x20)                                                                    \
    X(PageFlag, DIRTY, 0x40)

    DECLARE_TYPED_ENUM(PageFlag, U16, PAGE_FLAGS, 0) // NOLINT

    /**
     * A status code describing the outcome of accessing the page table structures.
     * <ol>
     *  <li>OKAY: The page table access is successful.</li>
     *  <li>ALLOC_ERROR: The page was already allocated or the allocation of it failed.</li>
     *  <li>FREE_ERROR: The free of the page failed.</li>
     *  <li>PAGE_TABLE_ENTRY_MISSING: The entry of an intermediate page table is missing.</li>
     * </ol>
     */
#define PAGE_TABLE_ACCESS_STATUSES(X)                                                              \
    X(PageTableAccessStatus, OKAY, 0x1)                                                            \
    X(PageTableAccessStatus, ALLOC_ERROR, 0x2)                                                     \
    X(PageTableAccessStatus, FREE_ERROR, 0x3)                                                      \
    X(PageTableAccessStatus, PAGE_TABLE_ENTRY_MISSING, 0x4)

    DECLARE_ENUM(PageTableAccessStatus, PAGE_TABLE_ACCESS_STATUSES, 0) // NOLINT

    /**
     * @brief Describes the outcome of accessing the page table hierarchy and performing an
     * operation (e.g allocation).
     *
     * <p>
     * Field descriptions:
     * <ol>
     *  <li>Status:        Outcome of the access.</li>
     *  <li>Path:          All PTEs that have been accessed before any modification until the access
     * ended.</li> <li>Level:         Page table level where the access ended.</li>
     *  <li>PageTableLeak: True if the physical memory on an intermediate page table could not be
     * freed.</li> <li>PTEAfter:      Copy of the accessed page table entry after modification
     * (currently in use).</li>
     * </ol>
     * </p>
     *
     * <p>
     *  The path array is filled in reverse, that is the first accessed PTE will not be at index 0
     * but at index 3. In general a Level n PTE will be found at index (n - 1), e.g. the L4 PTE can
     * be accessed at path[3].
     * </p>
     * <p>
     *  The path array may not contain valid PTEs at all indices, this is the case if the access
     * ended early due to an error. The number of valid PTEs in the path is always equal to the
     * (MaxPathLength - Level).
     * </p>
     */
    struct PageTableAccess {
        static constexpr U8 MAX_PATH_LENGTH = 5;

        PageTableAccessStatus                  status = PageTableAccessStatus::NONE;
        Array<PageTableEntry, MAX_PATH_LENGTH> path;
        U8                                     level            = 0;
        Array<bool, MAX_PATH_LENGTH>           pt_leak_map      = {false};
        PhysicalAddr                           physical_address = 0;
        PageTableEntry                         pte_after        = {.native_entry = 0};
    };

    /**
     * allocate a page mapping the vAddr to the pAddr in the virtual address space defined by the
     * basePT using the given flags.
     *
     * <p>
     * If intermediate page tables on the path to the page table entry are missing they will be
     * allocated using the physical memory manager.
     * </p>
     *
     * <p>
     *  If allocation of any intermediate page table fails then the allocation will be stopped and
     * memory allocated for already allocated intermediate page tables will be freed.
     * </p>
     *
     * @param base_pt       Base page table.
     * @param v_addr        Virtual address.
     * @param page_frame    Physical address.
     * @param flags         Page table entry flags.
     * @param pmm           Physical memory manager.
     *
     * @return Result of the page table access.
     */
    auto allocate_page(const PageTable&       base_pt,
                       VirtualAddr            v_addr,
                       PhysicalAddr           page_frame,
                       U16                    flags,
                       PhysicalMemoryManager* pmm) -> PageTableAccess;

    /**
     * free the page of the given vAddr and free the associated page frame using the physical memory
     * manager.
     *
     * <p>
     *  If any intermediate page table has no other entries except the page table entry for vAddr or
     * another page table that has no entries then it will be freed.
     * </p>
     *
     * @param base_pt Base page table.
     * @param v_addr  Virtual address.
     * @param pmm    Physical memory manager.
     *
     * @return Result of the page table access.
     */
    auto free_page(const PageTable& base_pt, VirtualAddr v_addr, PhysicalMemoryManager* pmm)
        -> PageTableAccess;

    /**
     * Modify the flags of the page for a virtual address.
     *
     * @param base_pt Base page table.
     * @param v_addr  Virtual address.
     * @param flags  Page flags that will be modified.
     * @param set    True: Sets the page flags, False: Clears the page flags.
     *
     * @return Result of the page table access.
     */
    auto modify_page_flags(const PageTable& base_pt, VirtualAddr v_addr, U16 flags, bool set)
        -> PageTableAccess;

    /**
     * Try to find the page for the given virtual address.
     *
     * @param base_pt Base page table.
     * @param v_addr  Virtual address.
     *
     * @return Result of the page table access.
     */
    auto find_page(const PageTable& base_pt, VirtualAddr v_addr) -> PageTableAccess;
} // namespace Rune::Memory

#endif // RUNEOS_PAGING_H
