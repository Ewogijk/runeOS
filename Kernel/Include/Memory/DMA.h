
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

#ifndef RUNEOS_DMA_H
#define RUNEOS_DMA_H

#include <KRE/System/System.h>

#include <Memory/MemoryModule.h>

namespace Rune::Memory::DMA {

    /// @brief Allocate obj_size bytes of uninitialized DMA memory.
    /// @param mem_size Number of bytes to allocate.
    /// @return A ptr to the beginning of the allocated memory.
    ///         If out of memory: nullptr.
    auto allocate(size_t mem_size) -> void*;

    /// @brief Allocate obj_size bytes of zeroed DMA memory.
    /// @param mem_size Number of bytes to allocate.
    /// @return A ptr to the beginning of the allocated memory.
    ///         If out of memory: nullptr.
    auto allocate_zeroed(size_t mem_size) -> void*;

    /// @brief Allocate sizeof(T) bytes of DMA memory and initialize the memory by calling the
    ///         default constructor of T.
    /// @tparam T Type of object to allocate.
    /// @return A pointer to the initialized object.
    ///         If out of memory: nullptr.
    template <class T>
    auto allocate_object() -> T* {
        auto obj = reinterpret_cast<T*>(allocate(sizeof(T)));
        if (obj != nullptr) new (obj) T();
        return obj;
    }

    /// @brief Free the DMA memory.
    /// @param memory Pointer to DMA memory.
    void free(void* memory);
} // namespace Rune::Memory::DMA

#endif // RUNEOS_DMA_H
