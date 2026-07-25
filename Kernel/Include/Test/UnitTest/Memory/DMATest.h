
//  Copyright 2025 Ewogijk
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#ifndef RUNEOS_DMATEST_H
#define RUNEOS_DMATEST_H

#include <Memory/DMA.h>

#include <Test/Heimdall/Heimdall.h>

#include <Memory/Paging.h>

using namespace Rune;

static constexpr size_t MEM_SIZE = 32;

struct TestMemory {
    U8   m_byte = 1;
    bool m_bool = true;
};

TEST("allocate", "DMA") {
    void* dma_mem = Memory::DMA::allocate(MEM_SIZE);
    auto  pta = Memory::find_page(Memory::get_base_page_table(), memory_pointer_to_addr(dma_mem));
    if (pta.status != Memory::PageTableAccessStatus::OKAY) REQUIRE(1 == 0);

    REQUIRE(pta.path[0].is_write_allowed());
    REQUIRE(pta.path[0].is_cache_disabled());

    // Clean up
    Memory::DMA::free(dma_mem);
}

TEST("allocate_zeroed", "DMA") {
    void* dma_mem = Memory::DMA::allocate(MEM_SIZE);
    auto  pta = Memory::find_page(Memory::get_base_page_table(), memory_pointer_to_addr(dma_mem));
    if (pta.status != Memory::PageTableAccessStatus::OKAY) REQUIRE(1 == 0);

    REQUIRE(pta.path[0].is_write_allowed());
    REQUIRE(pta.path[0].is_cache_disabled());

    Array<U8, MEM_SIZE> mem_array = {0};
    REQUIRE(memcmp(dma_mem, mem_array.data(), MEM_SIZE) == 0)

    // Clean up
    Memory::DMA::free(dma_mem);
}

TEST("allocate_object", "DMA") {
    auto* test_mem = Memory::DMA::allocate_object<TestMemory>();
    auto  pta = Memory::find_page(Memory::get_base_page_table(), memory_pointer_to_addr(test_mem));
    if (pta.status != Memory::PageTableAccessStatus::OKAY) REQUIRE(1 == 0);

    REQUIRE(pta.path[0].is_write_allowed());
    REQUIRE(pta.path[0].is_cache_disabled());

    REQUIRE(test_mem->m_byte == 1);
    REQUIRE(test_mem->m_bool == true);

    // Clean up
    Memory::DMA::free(test_mem);
}

#endif // RUNEOS_DMATEST_H
