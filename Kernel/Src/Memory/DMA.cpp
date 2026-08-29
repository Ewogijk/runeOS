//  Copyright 2026 Ewogijk
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

#include <Memory/DMA.h>

namespace Rune::Memory::DMA {
    auto allocate(size_t mem_size) -> void* {
        auto* mm = System::instance().get_module<Memory::MemoryModule>(ModuleSelector::MEMORY);
        return mm->get_heap()->allocate_dma(mem_size);
    }

    auto allocate_zeroed(size_t mem_size) -> void* {
        void* buf = allocate(mem_size);
        if (buf != nullptr) memset(buf, 0, mem_size);
        return buf;
    }

    void free(void* memory) {
        auto* mm = System::instance().get_module<Memory::MemoryModule>(ModuleSelector::MEMORY);
        mm->get_heap()->free(memory);
    }
} // namespace Rune::Memory::DMA
