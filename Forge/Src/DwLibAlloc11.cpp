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

#include "LibAlloc11.h"


#include <Pickaxe/AppManagement.h>
#include <Pickaxe/ThreadManagement.h>
#include <Pickaxe/MemoryManagement.h>

U16 MUTEX_HANDLE = 0;


extern int liballoc_lock() {
    if (MUTEX_HANDLE == 0) {
        int ret = Rune::Pickaxe::mutex_create("LibAllocMutex");
        if (ret < 0)
            Rune::Pickaxe::app_exit(-2);
        MUTEX_HANDLE = (U16) ret;
    }
    return Rune::Pickaxe::mutex_lock(MUTEX_HANDLE);
}


extern int liballoc_unlock() {
    return Rune::Pickaxe::mutex_unlock(MUTEX_HANDLE);
}


extern void* liballoc_alloc(size_t mem_size) {
    void* ptr = Rune::Pickaxe::memory_allocate_page(nullptr, mem_size, Rune::Pickaxe::PageProtection::WRITE);
    if ((uintptr_t) ptr == Rune::Pickaxe::MEM_MAP_BAD_ADDRESS
        || (uintptr_t) ptr == Rune::Pickaxe::MEM_MAP_BAD_ALLOC
        || (uintptr_t) ptr == Rune::Pickaxe::MEM_MAP_BAD_PAGE_PROTECTION)
        return nullptr;
    return ptr;
}


extern int liballoc_free(void* ptr, size_t mem_size) {
    return Rune::Pickaxe::memory_free_page(ptr, mem_size);
}