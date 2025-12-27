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

#include <Memory/Paging.h>

#include <Memory/VirtualMemory.h>

namespace Rune::Memory {
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                  Helper functions and definitions
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    constexpr U16 PT_MAX_SIZE            = 512;
    constexpr U8  MAX_PT_LEVEL           = 4; // Only 4 level paging is supported.
    constexpr U16 PT_IDX_MASK            = 0x1FF;
    constexpr U16 PAGE_FRAME_OFFSET_MASK = 0xFFF;
    constexpr U8  VIRTUAL_ADDR_SIZE      = 48; // bits, for 4 level paging
    constexpr U8  PTTE_BIT_SIZE          = 12; // bits, size of page-translation-table entry
    // Mask to extract the 4 level paging addr sign extended bits -> bits 48-63
    constexpr U64 MASK_ADDR_PREFIX = 0xFFFFF00000000000;
    // Mask to extract the 4 level paging addr -> bits 0-47
    constexpr U64 MASK_ADDRESS = 0x0000FFFFFFFFFFFF;
    // Bit shift amount to get the physical page offset
    constexpr U8 PHYSICAL_PAGE_OFFSET = 12;
    // Difference to translate from one page-translation hierarchy to another e.g. PML4 -> PDP
    constexpr U8 PAGE_TRANSLATION_OFFSET_DIFF = 9;

    // Cache the physical address width to avoid asking the CPU everytime for it
    U8 PHYSICAL_ADDRESS_WIDTH = 0; // NOLINT

    // Bit offsets to the PTTE control fields
    constexpr U8 IS_PRESENT_BIT          = 0;
    constexpr U8 IS_WRITE_ALLOWED_BIT    = 1;
    constexpr U8 IS_USER_MODE_ACCESS_BIT = 2;
    constexpr U8 IS_ACCESSED_BIT         = 5;
    constexpr U8 IS_DIRTY_BIT            = 6;

    enum x86_64PageFlag : U16 {
        PRESENT          = 0x01,
        WRITE_ALLOWED    = 0x02,
        USER_MODE_ACCESS = 0x04,
        WRITE_THROUGH    = 0x08,
        CACHE_DISABLE    = 0x10,
        ACCESSED         = 0x20,
        DIRTY            = 0x40,
    };

    auto to_x86_64_flags(U16 flags) -> U16 {
        return flags; // Page flag values are already x86_64 flags
    }

    auto addr_prefix(VirtualAddr v_addr) -> VirtualAddr { return v_addr & MASK_ADDR_PREFIX; }

    auto access_page_hierarchy(const PageTable& base_pt, VirtualAddr v_addr) -> PageTableAccess {
        // Bit shift amount to get the index into the PML4
        constexpr U8 PML4_OFFSET = 39;

        U8              shift = PML4_OFFSET;
        PageTableAccess pta;
        PageTable       pt = base_pt;
        PageTableEntry  pte;

        pta.path[4] = pt.to_page_table_entry();
        while (shift >= PHYSICAL_PAGE_OFFSET) {
            U16 pt_idx          = (v_addr >> shift) & PT_IDX_MASK;
            pte                 = pt[pt_idx];
            pta.path[pte.level] = pte;
            if (!pte.is_present()) {
                pta.status = PageTableAccessStatus::PAGE_TABLE_ENTRY_MISSING;
                pta.level  = pte.level;
                return pta;
            }
            shift -= PAGE_TRANSLATION_OFFSET_DIFF;
            pt     = pt.entry_as_page_table(pt_idx);
        }
        pta.status           = PageTableAccessStatus::OKAY;
        pta.level            = pt.get_level();
        pta.path[0]          = pte;
        pta.physical_address = pte.get_address() + (v_addr & PAGE_FRAME_OFFSET_MASK);
        return pta;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Page Table Entry
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    auto PageTableEntry::is_present() const -> bool {
        return bit_check(native_entry, IS_PRESENT_BIT);
    }

    auto PageTableEntry::is_write_allowed() const -> bool {
        return bit_check(native_entry, IS_WRITE_ALLOWED_BIT);
    }

    auto PageTableEntry::is_user_mode_access_allowed() const -> bool {
        return bit_check(native_entry, IS_USER_MODE_ACCESS_BIT);
    }

    auto PageTableEntry::is_accessed() const -> bool {
        return bit_check(native_entry, IS_ACCESSED_BIT);
    }

    auto PageTableEntry::is_dirty() const -> bool { return bit_check(native_entry, IS_DIRTY_BIT); }

    auto PageTableEntry::is_pointing_to_page_frame() const -> bool {
        // We only support 4KiB pages, so only page table entries (Level 1) can point to page frames
        // -> We can simply check by the PTE level, if it points to a page frame or another page
        // table
        return level == 1;
    }

    auto PageTableEntry::get_address() const -> PhysicalAddr {
        //   63        M M-1    12 11         0
        //  | ShiftLeft | Address | ShiftRight |
        // -> Shift by (ShiftLeft + ShiftRight) amount of bits to get address mask
        U8                   p_addr_width = PHYSICAL_ADDRESS_WIDTH;
        NativePageTableEntry mask =
            ((NativePageTableEntry) -1) >> (BIT_COUNT_QWORD - p_addr_width + PTTE_BIT_SIZE);
        return ((native_entry >> PTTE_BIT_SIZE) & mask) << PTTE_BIT_SIZE;
    }

    auto PageTableEntry::get_flags() const -> U16 { return native_entry & PAGE_FRAME_OFFSET_MASK; }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Page Table
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    PageTable::PageTable(NativePageTableEntry n_pte, NativePageTableEntry* pt, U8 level)
        : _npte(n_pte),
          _pt(pt),
          _level(level) {}

    auto PageTable::get_size() -> U16 { return PT_MAX_SIZE; }

    auto PageTable::is_base_page_table() const -> bool { return _level == MAX_PT_LEVEL; }

    auto PageTable::get_native_page_table_entry() const -> NativePageTableEntry { return _npte; }

    auto PageTable::get_level() const -> U8 { return _level; }

    auto PageTable::to_page_table_entry() const -> PageTableEntry {
        return {.native_entry = _npte, .level = _level};
    }

    auto PageTable::operator[](U16 idx) const -> PageTableEntry {
        if (idx >= PT_MAX_SIZE) return {.native_entry = 0, .level = PageTableEntry::BAD_LEVEL};
        return {.native_entry = _pt[idx], .level = (U8) (_level - 1)};
    }

    auto PageTable::entry_as_page_table(U16 idx) const -> PageTable {
        if (idx >= PT_MAX_SIZE) return PageTable(0, nullptr, PageTableEntry::BAD_LEVEL);
        PageTableEntry pte = (*this)[idx];
        return PageTable(
            pte.native_entry,
            reinterpret_cast<NativePageTableEntry*>(physical_to_virtual_address(pte.get_address())),
            _level - 1);
    }

    void PageTable::update(size_t idx, NativePageTableEntry n_pte) { _pt[idx] = n_pte; }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Paging Configuration
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    auto get_page_size() -> MemorySize { return 4 * (MemorySize) MemoryUnit::KiB; }

    void init_paging(U8 physical_address_width) { PHYSICAL_ADDRESS_WIDTH = physical_address_width; }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Page Table Hierarchy Access
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    auto interp_as_base_page_table(PhysicalAddr p_addr) -> PageTable {
        return PageTable(
            p_addr,
            reinterpret_cast<NativePageTableEntry*>(physical_to_virtual_address(p_addr)),
            MAX_PT_LEVEL);
    }

    auto get_base_page_table() -> PageTable {
        return interp_as_base_page_table(get_base_page_table_address());
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      Virtual Address Manipulations
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    auto to_canonical_form(VirtualAddr v_addr) -> VirtualAddr {
        if ((v_addr >> (VIRTUAL_ADDR_SIZE - 1) & 1) == 1)
            // Bit 47 is 1 -> apply sign extension
            v_addr =
                (((NativePageTableEntry) -1) >> VIRTUAL_ADDR_SIZE) << VIRTUAL_ADDR_SIZE | v_addr;
        else
            // Bit 47 is 0 -> ensure that bits 49-63 are zero
            v_addr = v_addr & MASK_ADDRESS;
        return v_addr;
    }

    auto physical_to_virtual_address(PhysicalAddr p_addr) -> VirtualAddr {
        return to_canonical_form(p_addr + get_virtual_kernel_space_layout().higher_half_direct_map);
    }

    auto virtual_to_physical_address(VirtualAddr v_addr, PhysicalAddr& p_addr_out) -> bool {
        if (addr_prefix(v_addr) == get_virtual_kernel_space_layout().higher_half_direct_map) {
            // The virtual address starts with 0xFFFF80... -> We can make a fast translation
            // by subtracting the higher half direct map offset
            p_addr_out = v_addr - get_virtual_kernel_space_layout().higher_half_direct_map;
        } else {
            // The virtual address has another prefix -> We cannot translate by subtracting an
            // offset This means we have to (slowly) walk the page tables to get the physical
            // address
            PageTableAccess pta = access_page_hierarchy(get_base_page_table(), v_addr);
            if (pta.status != PageTableAccessStatus::OKAY) return false;
            p_addr_out = pta.physical_address;
        }
        return true;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                  Page Table Hierarchy Manipulations
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    DEFINE_TYPED_ENUM(PageFlag, U16, PAGE_FLAGS, 0)

    DEFINE_ENUM(PageTableAccessStatus, PAGE_TABLE_ACCESS_STATUSES, 0)

    auto allocate_page(const PageTable&       base_pt,
                       VirtualAddr            v_addr,
                       PhysicalAddr           page_frame,
                       U16                    flags,
                       PhysicalMemoryManager* pmm) -> PageTableAccess {
        PageTableAccess pta = access_page_hierarchy(base_pt, v_addr);
        if (pta.status == PageTableAccessStatus::OKAY) {
            // The page is already allocated
            pta.status = PageTableAccessStatus::ALLOC_ERROR;
            pta.level  = 0;
            return pta;
        }

        // Allocate the missing page tables and finally the page itself
        // We start at the level where the first page table entry is missing
        // and go down to the L0 entry
        for (int i = pta.level; i >= 0; i--) {
            // The vAddr shift to get the page table index is at minimum 12 (the first 12 bits are
            // the page frame offset) and is encoded by 9 bits (512 entries per page table) L1 shift
            // = 12, L2 shift = 21, ... BUT we include the L0 page table entry as the page frame, so
            // we cannot do shift(i) = 12 + 9i (shift(1) = 21 but we need 12 therefore: shift(i) =
            // 12 + 9(i - 1) Now: Since we need to set the page table entry in the parent page table
            // the final calculation is
            //      shift(i) = 12 + 9(i - 1 + 1) = 12 + 9i
            U16       shift = PHYSICAL_PAGE_OFFSET + (PAGE_TRANSLATION_OFFSET_DIFF * i);
            PageTable parent_pt(pta.path[i + 1].get_address(),
                                reinterpret_cast<NativePageTableEntry*>(
                                    physical_to_virtual_address(pta.path[i + 1].get_address())),
                                i + 1);

            bool         alloc{false};
            PhysicalAddr pt_page_frame = 0;
            U16          pt_flags      = to_x86_64_flags(flags);
            if (i == 0) {
                alloc         = true;
                pt_page_frame = page_frame;
            } else {
                alloc = pmm->allocate(pt_page_frame);
            }

            if (!alloc) {
                // An allocation of a needed page table failed -> Free the other intermediate page
                // tables e.g. cr3 -> pml4 -> pdpe -> pd -> pt we need to allocate all page tables
                // starting from pdpe, but oh noooo allocation of pt failed
                // -> So we need to free pd and pdpe again
                // (pml4 is always allocated to begin with, it's the base page table, freeing it
                // deletes the whole virtual address space!)
                for (U8 j = i + 1; j <= pta.level; j++) {
                    // We want to free the L-j entry, but we need to get the physical address from
                    // its parent page table
                    // -> so we actually the shift for L-(j+1) and then we can look up the page
                    // table entry for L-j in its parent, thus getting its physical address
                    // shift = 12 + 9(j-1+1)
                    shift = PHYSICAL_PAGE_OFFSET + (PAGE_TRANSLATION_OFFSET_DIFF * j);
                    parent_pt =
                        PageTable(pta.path[j + 1].get_address(),
                                  reinterpret_cast<NativePageTableEntry*>(
                                      physical_to_virtual_address(pta.path[j + 1].get_address())),
                                  j + 1);
                    U16            plz_free_idx = (v_addr >> shift) & PT_IDX_MASK;
                    PageTableEntry plz_free     = parent_pt[plz_free_idx];
                    // If the free fails, just mark the memory leak and let the caller decide what
                    // to do
                    if (!pmm->free(plz_free.get_address())) pta.pt_leak_map[j] = true;
                    parent_pt.update(plz_free_idx, 0x0);
                }
                pta.status = PageTableAccessStatus::ALLOC_ERROR;
                pta.level  = i;
                break;
            }
            if (i > 0)
                memset(reinterpret_cast<void*>(physical_to_virtual_address(pt_page_frame)),
                       0,
                       get_page_size());

            NativePageTableEntry n_pte = pt_page_frame | pt_flags;
            parent_pt.update((v_addr >> shift) & PT_IDX_MASK, n_pte);
            pta.path[i] = {.native_entry = n_pte, .level = (U8) i};
            if (i == 0) pta.pte_after = {.native_entry = n_pte, .level = (U8) i};
        }

        // At least the page frame was not allocated and no errors happened during allocation ->
        // Everything went fine
        if (pta.status == PageTableAccessStatus::PAGE_TABLE_ENTRY_MISSING)
            pta.status = PageTableAccessStatus::OKAY;
        return pta;
    }

    auto free_page(const PageTable& base_pt, VirtualAddr v_addr, PhysicalMemoryManager* pmm)
        -> PageTableAccess {
        PageTableAccess pta = access_page_hierarchy(base_pt, v_addr);
        if (pta.status != PageTableAccessStatus::OKAY) return pta;

        U8 shift = PHYSICAL_PAGE_OFFSET;
        // We only the page tables until the L3 page table since the L4 page table is the base page
        // table and freeing it would delete the whole virtual address space
        for (int i = 0; i < 4; i++) {
            PageTable      parent_pt(pta.path[i + 1].get_address(),
                                reinterpret_cast<NativePageTableEntry*>(
                                    physical_to_virtual_address(pta.path[i + 1].get_address())),
                                i + 1);
            PageTableEntry pte = pta.path[i];

            bool do_free{false};
            if (i == 0) {
                do_free = pte.is_present();
            } else {
                PageTable pt(pte.get_address(),
                             reinterpret_cast<NativePageTableEntry*>(
                                 physical_to_virtual_address(pte.get_address())),
                             i);
                int       pte_count = 0;
                for (size_t j = 0; j < PT_MAX_SIZE; j++) {
                    if (pt[j].is_present()) pte_count++;
                }
                do_free = pte_count == 0;
            }

            if (do_free) {
                if (!pmm->free(pte.get_address())) {
                    pta.status = PageTableAccessStatus::FREE_ERROR;
                    pta.level  = i;
                    break;
                }
                parent_pt.update((v_addr >> shift) & PT_IDX_MASK, 0x0);
                if (i == 0) pta.pte_after = parent_pt[(v_addr >> shift) & PT_IDX_MASK];
            }
            shift += PAGE_TRANSLATION_OFFSET_DIFF;
        }
        return pta;
    }

    auto modify_page_flags(const PageTable& base_pt, VirtualAddr v_addr, U16 flags, bool set)
        -> PageTableAccess {
        PageTableAccess pta = access_page_hierarchy(base_pt, v_addr);
        if (pta.status != PageTableAccessStatus::OKAY) return pta;
        NativePageTableEntry updated_entry =
            set ? pta.path[0].native_entry | to_x86_64_flags(flags)
                : pta.path[0].native_entry & ~to_x86_64_flags(flags);
        PageTable pt(pta.path[1].get_address(),
                     reinterpret_cast<NativePageTableEntry*>(
                         physical_to_virtual_address(pta.path[1].get_address())),
                     1);
        U16       pt_idx = (v_addr >> PHYSICAL_PAGE_OFFSET) & PT_IDX_MASK;
        pt.update(pt_idx, updated_entry);
        pta.pte_after = pt[pt_idx];
        return pta;
    }

    auto find_page(const PageTable& base_pt, VirtualAddr v_addr) -> PageTableAccess {
        return access_page_hierarchy(base_pt, v_addr);
    }
} // namespace Rune::Memory
