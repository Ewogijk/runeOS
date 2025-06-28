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

#include <Pickaxe/MemoryManagement.h>


namespace Rune::Pickaxe {

    size_t memory_get_page_size() {
        return (size_t) system_call0(0);
    }


    void* memory_allocate_page(void* v_addr, size_t num_pages, PageProtection page_protection) {
        return (void*) (uintptr_t) system_call3(1, (U64) (uintptr_t) v_addr, num_pages, (U64) page_protection);
    }


    int memory_free_page(void* v_addr, size_t num_pages) {
        return system_call2(2, (U64) (uintptr_t) v_addr, num_pages);
    }
}