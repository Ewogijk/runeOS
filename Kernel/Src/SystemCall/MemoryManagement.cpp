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

#include <SystemCall/MemoryManagement.h>

#include <Hammer/Algorithm.h>


namespace Rune::SystemCall {

    DEFINE_TYPED_ENUM(MemoryMapReturnCode, LibK::VirtualAddr, MEM_MAP_RC, 0x0)


    DEFINE_ENUM(PageProtection, PAGE_PROTECTIONS, 0x0)


    S64 memory_get_page_size(void* sys_call_ctx) {
        SILENCE_UNUSED(sys_call_ctx)
        return (S64) Memory::get_page_size();
    }


    S64 memory_allocate_page(void* sys_call_ctx, U64 v_addr, U64 num_pages, U64 page_protection) {
        auto* mem_ctx = (MemoryManagementContext*) sys_call_ctx;
        auto* vmm     = mem_ctx->mem_subsys->get_virtual_memory_manager();

        App::Info* app = mem_ctx->app_subsys->get_active_app();
        Memory::PageTable base_pt   = Memory::get_base_page_table();
        LibK::MemorySize  page_size = Memory::get_page_size();
        auto              kv_addr   = (LibK::VirtualAddr) v_addr;
        if (kv_addr == 0) {
            // No specific memory location is requested -> The kernel selects where to map the pages

            // The heap can have gaps due to freeing pages in the middle of it, we will try to reuse those gaps if they
            // are large enough to fit the requested amount of memory.
            // This counter counts the number of consecutive pages in a heap gap.
            size_t                 consecutive_free = 0;
            LibK::VirtualAddr      gap_start        = 0x0;
            for (LibK::VirtualAddr c_addr           = app->heap_start; c_addr < app->heap_limit; c_addr += page_size) {
                Memory::PageTableAccess pta = Memory::find_page(base_pt, c_addr);
                if (pta.status == Memory::PageTableAccessStatus::PAGE_TABLE_ENTRY_MISSING) {
                    // c_addr is not mapped -> This is a heap gap
                    if (consecutive_free == 0)
                        // c_addr is pointing to the start of the heap gap
                        gap_start = c_addr;

                    consecutive_free++;
                    if (consecutive_free == num_pages) {
                        // The heap gap contains enough free memory -> Use it
                        break;
                    }
                } else {
                    // c_addr is mapped -> The end of a heap gap is reached (or this is the heap start)
                    gap_start        = 0x0;
                    consecutive_free = 0;
                }
            }

            // If a large enough heap gap was found use it else we map the memory at the end of the heap
            kv_addr = gap_start == 0 ? app->heap_limit : gap_start;
            // TODO check for heap overflow (into the user stack)
        } else {
            // A specific memory location is requested -> Align the address to a page boundary (if needed) and verify
            // that the requested memory region does not intersect kernel memory
            if (!LibK::memory_is_aligned(kv_addr, page_size))
                kv_addr = LibK::memory_align(kv_addr, page_size, true);
            if (!mem_ctx->k_guard->verify_user_buffer((void*) kv_addr, num_pages * page_size))
                return MemoryMapReturnCode::BAD_ADDRESS;
        }


        if (page_protection > PageProtection::READ + PageProtection::WRITE)
            // The requested page protection contains unknown flags
            return MemoryMapReturnCode::BAD_PAGE_PROTECTION;

        // Map it with write rights so the memory region can be initialized
        U16               page_flags           = Memory::PageFlag::PRESENT
                                                 | Memory::PageFlag::USER_MODE_ACCESS
                                                 | Memory::PageFlag::WRITE_ALLOWED;

        if (!vmm->allocate(kv_addr, page_flags, num_pages))
            return MemoryMapReturnCode::BAD_ALLOC;

        // Zero init the memory region
        // TODO allow init with buffer
        memset((void*) kv_addr, 0, num_pages * page_size);

        if (!check_bit(page_protection, 1)) {
            // Memory was requested as readonly -> remove the write allowed flag
            for (LibK::VirtualAddr c_addr = kv_addr; c_addr < kv_addr + num_pages * page_size; c_addr += page_size) {
                Memory::PageTableAccess pta = Memory::modify_page_flags(
                        base_pt,
                        c_addr,
                        Memory::PageFlag::WRITE_ALLOWED,
                        false
                );
                if (pta.status != Memory::PageTableAccessStatus::OKAY)
                    return MemoryMapReturnCode::BAD_ALLOC;
            }
        }
        LibK::VirtualAddr maybe_new_heap_limit = kv_addr + num_pages * page_size;
        if (maybe_new_heap_limit > app->heap_limit)
            app->heap_limit = maybe_new_heap_limit;

        return (S64) kv_addr;
    }


    S64 memory_free_page(void* sys_call_ctx, U64 v_addr, U64 num_pages) {
        auto* mem_ctx = (MemoryManagementContext*) sys_call_ctx;
        auto* vmm     = mem_ctx->mem_subsys->get_virtual_memory_manager();
        auto* app     = mem_ctx->app_subsys->get_active_app();

        LibK::MemorySize page_size = Memory::get_page_size();
        auto             kv_addr   = (LibK::VirtualAddr) v_addr;

        // Align the address to a page boundary (if needed) and verify that the requested memory region does not
        // intersect kernel memory
        if (!LibK::memory_is_aligned(kv_addr, page_size))
            kv_addr = LibK::memory_align(kv_addr, page_size, true);
        if (!mem_ctx->k_guard->verify_user_buffer((void*) kv_addr, num_pages * page_size))
            return -1;

        if (!vmm->free(kv_addr, num_pages))
            return -2;

        LibK::VirtualAddr mem_region_end = kv_addr + num_pages * page_size;
        if (mem_region_end == app->heap_limit)
            app->heap_limit = kv_addr;

        return 0;
    }
}