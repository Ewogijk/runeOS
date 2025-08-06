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

#ifndef RUNEOS_MEMORYMANAGEMENT_H
#define RUNEOS_MEMORYMANAGEMENT_H


#include <Ember/Ember.h>

#include <SystemCall/KernelGuardian.h>

#include <Memory/MemorySubsystem.h>

#include <App/AppSubsystem.h>


namespace Rune::SystemCall {
    /**
     * @brief The context for all memory management system calls.
     */
    struct MemorySystemCallContext {
        KernelGuardian   * k_guard    = nullptr;
        Memory::MemorySubsystem* mem_subsys = nullptr;
        App::AppSubsystem   * app_subsys = nullptr;
    };


    /**
     * @brief Gets the size of a virtual page in bytes.
     * @param sys_call_ctx The memory management context.
     * @return Page size.
     */
    Ember::StatusCode memory_get_page_size(const void* sys_call_ctx);


    /**
     * If v_addr is zero then the location of the memory region will be chosen by the kernel. When v_addr is >0 then the
     * kernel will use the value as a hint to where the memory region should be mapped. v_addr will always be page
     * aligned if needed.
     *
     * <p>
     *  The memory region will always be zero initialized.
     * </p>
     *
     * @brief Allocate the requested amount of memory in the active applications virtual address space.
     * @param sys_call_ctx    The memory management context.
     * @param v_addr          Requested start address of the memory region that will be allocated. If zero the kernel
     *                          chooses where to map the memory region.
     * @param num_pages       Number of pages that should be mapped.
     * @param page_protection Requested page protection level.
     * @return Success:  A pointer to the start of the mapped memory region.<br>
     *          BAD_ARG: The requested memory region intersects kernel memory or the page protection
     *                   flag is invalid.<br>
     *          FAULT:   The memory allocation failed.
     */
    Ember::StatusCode memory_allocate_page(void* sys_call_ctx, U64 v_addr, U64 num_pages, U64 page_protection);


    /**
     * If v_addr is not page aligned it will be aligned to a page boundary.
     *
     * @brief Remove the requested memory region from the active applications virtual address space.
     * @param sys_call_ctx The memory management context.
     * @param v_addr       Staring address of the memory region that will be freed.
     * @param num_pages    Number of pages that should be freed.
     * @return OKAY:     The memory region is freed.<br>
     *          BAD_ARG: The requested memory region intersects kernel memory.<br>
     *          FAULT:   The memory free failed.
     */
    Ember::StatusCode memory_free_page(void* sys_call_ctx, U64 v_addr, U64 num_pages);
}

#endif //RUNEOS_MEMORYMANAGEMENT_H
