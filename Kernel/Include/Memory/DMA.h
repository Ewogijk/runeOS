
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

    /// @brief Allocate sizeof(T) bytes of DMA memory and initialize the memory by calling the
    ///         constructor of T with args.
    /// @tparam T Type of object to allocate.
    /// @tparam Args
    /// @param args Constructor arguments.
    /// @return A pointer to the initialized object.
    ///         If out of memory: nullptr.
    template <class T, class... Args>
    auto allocate_object(Args&&... args) -> T* {
        auto obj = reinterpret_cast<T*>(allocate(sizeof(T)));
        if (obj != nullptr) new (obj) T(forward<Args>(args)...);
        return obj;
    }

    /// @brief Free the DMA memory.
    /// @param memory Pointer to DMA memory.
    void free(void* memory);

    /// @brief Allocate an instance of T with DMA memory and wrap it in a unique pointer with a
    ///         custom deleter calling DMA::free.
    /// @tparam T Type of object to allocate.
    /// @tparam Args
    /// @param args Arguments passed to the constructor of T.
    /// @return A unique pointer wrapping a T instance.
    template <class T, class... Args>
    auto unique_pointer_of(Args... args) -> UniquePointer<T> {
        return UniquePointer<T>(allocate_object<T>(forward<Args>(args)...),
                                [](void* ptr) -> void { DMA::free(ptr); });
    }

    /// @brief Allocate an instance of T with DMA memory and wrap it in a shared pointer with a
    ///         custom deleter calling DMA::free.
    /// @tparam T Type of object to allocate.
    /// @tparam Args
    /// @param args Arguments passed to the constructor of T.
    /// @return A unique pointer wrapping a T instance.
    template <class T, class... Args>
    auto shared_pointer_of(Args... args) -> SharedPointer<T> {
        return SharedPointer<T>(allocate_object<T>(forward<Args>(args)...),
                                [](void* ptr) -> void { DMA::free(ptr); });
    }
} // namespace Rune::Memory::DMA
#endif // RUNEOS_DMA_H
