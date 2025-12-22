
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

#ifndef RUNEOS_MEMORY_H
#define RUNEOS_MEMORY_H

#include <Ember/Ember.h>
#include <Ember/Enum.h>

#include <KRE/Build.h>
#include <KRE/Collections/Array.h>
#include <KRE/Utility.h>

namespace Rune {
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                               Smart Pointer
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * Simple std::unique_ptr implementation.
     */
    template <typename T>
    class UniquePointer {
        T* _ptr;

        void swap(UniquePointer<T>& other) noexcept {
            T* temp    = _ptr;
            _ptr       = other._ptr;
            other._ptr = temp;
        }

      public:
        UniquePointer() : _ptr(nullptr) {}

        explicit UniquePointer(T* ptr) : _ptr(ptr) {}

        ~UniquePointer() { delete _ptr; }

        UniquePointer(const UniquePointer<T>& other) = delete;

        auto operator=(const UniquePointer<T>& other) -> UniquePointer& = delete;

        UniquePointer(UniquePointer<T>&& other) noexcept : _ptr(nullptr) { swap(other); }

        auto operator=(UniquePointer<T>&& other) noexcept -> UniquePointer& {
            swap(other);
            return *this;
        }

        auto get() const -> T* { return _ptr; }

        explicit operator bool() const { return _ptr; }

        [[nodiscard]]
        auto operator*() const -> T& {
            return *_ptr;
        }

        auto operator->() const -> T* { return _ptr; }

        auto operator==(const UniquePointer<T>& other) const -> bool { return _ptr == other._ptr; }

        auto operator!=(const UniquePointer<T>& other) const -> bool { return _ptr != other._ptr; }
    };

    /**
     * Construct an object of T and wrap it in a unique pointer.
     * @tparam T Pointer type.
     * @tparam Args Type of the variadic constructor arguments.
     * @param args Variadic arguments passed to the constructor of T.
     * @return A unique pointer wrapping a T instance.
     */
    template <typename T, typename... Args>
    auto make_unique(Args... args) -> UniquePointer<T> {
        return UniquePointer<T>(new T(forward<Args>(args)...));
    }

    template <typename T>
    struct RefControlBlock {
        T*     ptr              = nullptr;
        size_t strong_ref_count = 0;
    };

    /**
     * Simple std::shared_ptr implementation.
     */
    template <typename T>
    class SharedPointer {
        RefControlBlock<T>* _refs;

        void init(T* ptr) {
            if (ptr == nullptr) return;
            _refs      = new RefControlBlock<T>;
            _refs->ptr = ptr;
            ++_refs->strong_ref_count;
        }

        void swap(SharedPointer<T>& other) noexcept {
            RefControlBlock<T>* temp = _refs;
            _refs                    = other._refs;
            other._refs              = temp;
        }

      public:
        SharedPointer() : _refs(nullptr) { init(nullptr); }

        explicit SharedPointer(T* ptr) : _refs(nullptr) { init(ptr); }

        ~SharedPointer() {
            if (_refs == nullptr) return;
            --_refs->strong_ref_count;
            if (_refs->strong_ref_count == 0) {
                delete _refs->ptr;
                delete _refs;
            }
        }

        SharedPointer(const SharedPointer<T>& other) noexcept : _refs(other._refs) {
            if (_refs) ++_refs->strong_ref_count;
        }

        auto operator=(const SharedPointer<T>& other) noexcept -> SharedPointer& {
            if (this == &other) return *this;

            if (_refs == other._refs) // Same ref control block
                return *this;

            // Different pointer -> Decrement current pointer ref count
            if (_refs) {
                --_refs->strong_ref_count;
                if (_refs->strong_ref_count == 0) {
                    delete _refs->ptr;
                    delete _refs;
                }
            }
            _refs = other._refs;
            if (_refs) ++_refs->strong_ref_count;
            return *this;
        }

        SharedPointer(SharedPointer<T>&& other) noexcept : _refs(other._refs) {
            other._refs = nullptr;
        }

        auto operator=(SharedPointer<T>&& other) noexcept -> SharedPointer& {
            SharedPointer<T> tmp(move(other));
            swap(tmp);
            return *this;
        }

        [[nodiscard]] auto get() const -> T* { return _refs ? _refs->ptr : nullptr; }

        auto get_ref_count() -> size_t { return _refs ? _refs->strong_ref_count : 0; }

        explicit operator bool() const { return _refs; }

        [[nodiscard]] auto operator*() const -> T& { return *_refs->ptr; }

        auto operator->() const -> T* { return _refs ? _refs->ptr : nullptr; }

        auto operator==(const SharedPointer<T>& other) const -> bool {
            return _refs ? _refs->ptr == other._refs->ptr : _refs == other._refs;
        }

        auto operator!=(const SharedPointer<T>& other) const -> bool {
            return _refs ? _refs->ptr != other._refs->ptr : _refs != other._refs;
        }
    };

    /**
     * Construct an object of T and wrap it in a shared pointer.
     * @tparam T Pointer type.
     * @tparam Args Type of the variadic constructor arguments.
     * @param args Variadic arguments passed to the constructor of T.
     * @return A shared pointer wrapping a T instance.
     */
    template <typename T, typename... Args>
    auto make_shared(Args... args) -> SharedPointer<T> {
        return SharedPointer<T>(new T(forward<Args>(args)...));
    }

#ifdef BIT64
    // A memory address e.g. 0x7328FAD123
    using MemoryAddr = U64;

    // A physical memory address e.g. 0x7328FAD123
    using PhysicalAddr = U64;

    // A virtual memory address e.g. 0x7328FAD123
    using VirtualAddr = U64;

    // A memory size e.g. 4298392 bytes
    using MemorySize = U64;

    // A memory size in floating point precision e.g. 4.327 MiB
    using MemoryFloatSize = long double;
#else
    // A memory address e.g. 0x7328FAD123
    using MemoryAddr = U32;

    // A physical memory address e.g. 0x7328FAD123
    using PhysicalAddr = U32;

    // A virtual memory address e.g. 0x7328FAD123
    using VirtualAddr = U32;

    // A memory size e.g. 4298392 bytes
    using MemorySize = U32;

    // A memory size in floating point precision e.g. 4.327 MiB
    using MemoryFloatSize = double;
#endif

#define MEMORY_UNITS(X)                                                                            \
    X(MemoryUnit, B, 1)                                                                            \
    X(MemoryUnit, KB, 1000)                                                                        \
    X(MemoryUnit, MB, 1000000)                                                                     \
    X(MemoryUnit, GB, 1000000000)                                                                  \
    X(MemoryUnit, TB, 1000000000000)                                                               \
    X(MemoryUnit, KiB, 1024)                                                                       \
    X(MemoryUnit, MiB, 1048576)                                                                    \
    X(MemoryUnit, GiB, 1073741824)                                                                 \
    X(MemoryUnit, TiB, 1099511627776)

    /**
     * A memory unit e.g. kilobytes.
     */
    DECLARE_TYPED_ENUM(MemoryUnit, MemorySize, MEMORY_UNITS, 0x0) // NOLINT

    /**
     * Reformat the given bytes to another memory unit.
     *
     * @param bytes Bytes.
     * @param unit  Memory unit.
     *
     * @return Bytes converted to another unit.
     */
    auto memory_bytes_in(MemorySize bytes, MemoryUnit unit) -> MemoryFloatSize;

    /**
     * Check if the memory address is aligned to the given boundary.
     *
     * @param mem_addr Memory address to checked.
     * @param boundary Memory boundary.
     * @return True if aligned else false.
     */
    auto memory_is_aligned(MemoryAddr mem_addr, MemoryAddr boundary) -> bool;

    /**
     * Align the memory address to the given boundary. If roundUp is true the memory address will be
     * rounded up to the next aligned memory address (e.g. 4KB boundary: 5KB -> 8KB) else rounded
     * down (e.g. 4KB boundary: 5KB -> 4KB)
     *
     * @param mem_addr  Memory address.
     * @param page_boundary Memory boundary.
     * @param round_up  If true the memory address is rounded up to the next boundary else rounded
     * down.
     *
     * @return The aligned memory address.
     */
    auto memory_align(MemoryAddr mem_addr, MemoryAddr page_boundary, bool round_up) -> MemoryAddr;

    /// @brief
    /// @tparam T Pointer type.
    /// @param v_addr
    /// @return Pointer to the numerical value of the virtual address.
    template <typename T>
    constexpr auto memory_addr_to_pointer(VirtualAddr v_addr) -> T* {
        return reinterpret_cast<T*>(v_addr); // NOLINT
    }

    /// @brief
    /// @tparam T Pointer type.
    /// @param pointer
    /// @return Memory address of the pointer as numerical value.
    template <typename T>
    constexpr auto memory_pointer_to_addr(T* pointer) -> MemoryAddr {
        return reinterpret_cast<uintptr_t>(pointer); // NOLINT
    }

    /// Describes if a memory region is free to use or reserved for something else. If further
    /// information is available the type may also describe which type of data is stored in the
    /// region (e.g. kernel code).
#define MEMORY_REGION_TYPES(X)                                                                     \
    X(MemoryRegionType, USABLE, 0x1)                                                               \
    X(MemoryRegionType, USED, 0x2)                                                                 \
    X(MemoryRegionType, RESERVED, 0x3)                                                             \
    X(MemoryRegionType, USERSPACE, 0x4)                                                            \
    X(MemoryRegionType, HHDM, 0x5)                                                                 \
    X(MemoryRegionType, PMM_RESERVED, 0x6)                                                         \
    X(MemoryRegionType, VMM_RESERVED, 0x7)                                                         \
    X(MemoryRegionType, KERNEL_HEAP, 0x8)                                                          \
    X(MemoryRegionType, KERNEL_CODE, 0x9)                                                          \
    X(MemoryRegionType, BOOTLOADER_RECLAIMABLE, 0xA)

    DECLARE_ENUM(MemoryRegionType, MEMORY_REGION_TYPES, 0x0) // NOLINT

    /**
     * A region of memory in the computers RAM.
     */
    struct MemoryRegion {
        MemoryAddr       start       = 0x0;
        MemorySize       size        = 0x0;
        MemoryRegionType memory_type = MemoryRegionType::NONE;

        [[nodiscard]] auto end() const -> MemoryAddr;

        [[nodiscard]] auto size_in(MemoryUnit unit) const -> MemoryFloatSize;

        [[nodiscard]] auto contains(const MemoryRegion& other) const -> bool;

        auto operator==(const MemoryRegion& other) const -> bool;

        auto operator!=(const MemoryRegion& other) const -> bool;

        auto operator<=(const MemoryRegion& other) const -> bool;
    };

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                               Memory Map
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * A map of the computer's physical or virtual RAM.
     */
    class MemoryMap {
      public: // Maximum number of allowed memory regions in a map.
        static constexpr size_t LIMIT = 64;

      private:
        Array<MemoryRegion, LIMIT> _map;

        U64 _free_mem;

        U64 _reserved_mem;

        size_t _num_regions;

      public:
        explicit MemoryMap(Array<MemoryRegion, LIMIT> regions);

        MemoryMap(std::initializer_list<MemoryRegion> regions);

        /**
         *
         * @return Number of memory regions with memory type unequal to "None" in the map.
         */
        [[nodiscard]] auto size() const -> size_t;

        /**
         *
         * @return Usable memory in bytes.
         */
        [[nodiscard]] auto get_free_memory() const -> MemorySize;

        /**
         *
         * @param unit Memory unit.
         * @return Usable memory converted to a memory unit.
         */
        [[nodiscard]] auto get_free_memory_in(MemoryUnit unit) const -> MemoryFloatSize;

        /**
         *
         * @return Reserved memory in bytes.
         */
        [[nodiscard]] auto get_reserved_memory() const -> MemorySize;

        /**
         *
         * @param unit Memory unit.
         * @return Reserved memory converted to a memory unit.
         */
        [[nodiscard]] auto get_reserved_memory_in(MemoryUnit unit) const -> MemoryFloatSize;

        /**
         *
         * @return Total memory in bytes.
         */
        [[nodiscard]] auto get_total_memory() const -> MemorySize;

        /**
         *
         * @param unit Memory unit.
         *
         * @return Total memory converted to a memory unit.
         */
        [[nodiscard]] auto get_total_memory_in(MemoryUnit unit) const -> MemoryFloatSize;

        /**
         * Claim the memory region defined by the `claimant` and mark it with the memory type of the
         * `claimant`. The `claimant` must lie completely within the targeted memory region and will
         * be aligned to given `boundary` if not properly aligned.
         *
         * <p>
         * Claiming fails if the claimed region needs to be split but no more space for more memory
         * regions is available.
         * </p>
         *
         * <p>
         * If the memory type of the `claimant` is `Usable` the memory will be freed otherwise
         * reserved.
         * </p>
         *
         * @param claimant Memory region that should be
         * @param boundary Page boundary.
         *
         * @return True if the region got claimed else false.
         */
        auto claim(MemoryRegion& claimant, uint32_t boundary) -> bool;

        /**
         * Merge adjacent regions of the same type into bigger regions.
         */
        void merge();

        auto operator[](size_t index) const -> const MemoryRegion&;

        [[nodiscard]]
        auto begin() const -> const MemoryRegion*;

        [[nodiscard]]
        auto end() const -> const MemoryRegion*;
    };
} // namespace Rune

#endif // RUNEOS_MEMORY_H
