
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
        T*   m_ptr;
        void (*m_deleter)(void*);

        void swap(UniquePointer<T>& other) noexcept {
            T* temp     = m_ptr;
            m_ptr       = other.m_ptr;
            other.m_ptr = temp;

            void (*temp_deleter)(void*) = m_deleter;
            m_deleter                   = other.m_deleter;
            other.m_deleter             = temp_deleter;
        }

      public:
        UniquePointer() : m_ptr(nullptr), m_deleter(nullptr) {}

        explicit UniquePointer(T* ptr)
            : m_ptr(ptr),
              m_deleter([](void* p) -> void { delete static_cast<T*>(p); }) {}

        ~UniquePointer() {
            if (m_ptr) m_deleter(m_ptr);
        }

        UniquePointer(const UniquePointer<T>& other) = delete;

        auto operator=(const UniquePointer<T>& other) -> UniquePointer& = delete;

        UniquePointer(UniquePointer<T>&& other) noexcept : m_ptr(nullptr), m_deleter(nullptr) {
            swap(other);
        }

        auto operator=(UniquePointer<T>&& other) noexcept -> UniquePointer& {
            swap(other);
            return *this;
        }

        [[nodiscard]] auto get() const -> T* { return m_ptr; }

        explicit operator bool() const { return m_ptr; }

        [[nodiscard]]
        auto operator*() const -> AddLValueRef<T>::type {
            return *m_ptr;
        }

        auto operator->() const -> T* { return m_ptr; }

        auto operator==(const UniquePointer<T>& other) const -> bool {
            return m_ptr == other.m_ptr;
        }

        auto operator!=(const UniquePointer<T>& other) const -> bool {
            return m_ptr != other.m_ptr;
        }
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

    struct RefControlBlock {
        size_t m_strong_ref_count  = 0;
        void   (*m_deleter)(void*) = nullptr;
        void*  m_raw_ptr           = nullptr;
    };

    /**
     * Simple std::shared_ptr implementation.
     */
    template <typename T>
    class SharedPointer {
        template <typename U>
        friend class SharedPointer;

        T*               m_ptr;
        RefControlBlock* m_refs;

        void init(T* ptr) {
            m_ptr = ptr;
            if (ptr == nullptr) return;
            m_refs            = new RefControlBlock;
            m_refs->m_raw_ptr = ptr;
            m_refs->m_deleter = [](void* p) -> void { delete static_cast<T*>(p); };
            ++m_refs->m_strong_ref_count;
        }

        void swap(SharedPointer<T>& other) noexcept {
            T* tmp_ptr  = m_ptr;
            m_ptr       = other.m_ptr;
            other.m_ptr = tmp_ptr;

            RefControlBlock* tmp_refs = m_refs;
            m_refs                    = other.m_refs;
            other.m_refs              = tmp_refs;
        }

        void reset0() {
            --m_refs->m_strong_ref_count;
            if (m_refs->m_strong_ref_count == 0) {
                m_refs->m_deleter(m_refs->m_raw_ptr);
                delete m_refs;
            }
            m_ptr  = nullptr;
            m_refs = nullptr;
        }

      public:
        SharedPointer() : m_ptr(nullptr), m_refs(nullptr) {}

        explicit SharedPointer(T* ptr) : m_ptr(nullptr), m_refs(nullptr) { init(ptr); }

        ~SharedPointer() {
            if (m_refs == nullptr) return;
            reset0();
        }

        SharedPointer(const SharedPointer<T>& other) noexcept
            : m_ptr(other.m_ptr),
              m_refs(other.m_refs) {
            if (m_refs != nullptr) ++m_refs->m_strong_ref_count;
        }

        template <class U>
        SharedPointer(const SharedPointer<U>& other) noexcept
            : m_ptr(static_cast<T*>(other.m_ptr)),
              m_refs(other.m_refs) {
            if (m_refs) ++m_refs->m_strong_ref_count;
        }

        auto operator=(const SharedPointer<T>& other) noexcept -> SharedPointer& {
            if (this == &other) return *this;
            if (m_refs == other.m_refs) return *this;

            if (m_refs != nullptr) reset0();

            m_ptr  = other.m_ptr;
            m_refs = other.m_refs;
            if (m_refs != nullptr) ++m_refs->m_strong_ref_count;
            return *this;
        }

        SharedPointer(SharedPointer<T>&& other) noexcept
            : m_ptr(other.m_ptr),
              m_refs(other.m_refs) {
            other.m_ptr  = nullptr;
            other.m_refs = nullptr;
        }

        auto operator=(SharedPointer<T>&& other) noexcept -> SharedPointer& {
            SharedPointer<T> tmp(move(other));
            swap(tmp);
            return *this;
        }

        [[nodiscard]] auto get() const -> T* { return m_ptr; }

        auto get_ref_count() -> size_t {
            return m_refs != nullptr ? m_refs->m_strong_ref_count : 0;
        }

        auto get_ref_count() const -> size_t {
            return m_refs != nullptr ? m_refs->m_strong_ref_count : 0;
        }

        auto reset() -> void {
            if (m_refs == nullptr) return;
            reset0();
        }

        explicit operator bool() const { return m_ptr != nullptr; }

        [[nodiscard]] auto operator*() const -> AddLValueRef<T>::type { return *m_ptr; }

        auto operator->() const -> T* { return m_ptr; }

        auto operator==(const SharedPointer<T>& other) const -> bool {
            return m_ptr == other.m_ptr;
        }

        auto operator!=(const SharedPointer<T>& other) const -> bool {
            return m_ptr != other.m_ptr;
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
    /// information is available, the type may also describe which type of data is stored in the
    /// region (e.g. kernel code).
#define MEMORY_REGION_TYPES(X)                                                                     \
    X(MemoryRegionType, USABLE, 0x1)                                                               \
    X(MemoryRegionType, USED, 0x2)                                                                 \
    X(MemoryRegionType, RESERVED, 0x3)                                                             \
    X(MemoryRegionType, USERSPACE, 0x4)                                                            \
    X(MemoryRegionType, HHDM, 0x5)                                                                 \
    X(MemoryRegionType, PMM_RESERVED, 0x6)                                                         \
    X(MemoryRegionType, KERNEL_HEAP, 0x7)                                                          \
    X(MemoryRegionType, KERNEL_CODE, 0x8)                                                          \
    X(MemoryRegionType, ACPI, 0x9)                                                                 \
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

        /// @brief Get the type of the given memory region.
        /// @param region A memory region.
        /// @return The type of the memory region, MemoryRegionType::NONE if it is not part of the
        ///         memory map or intersects multiple memory regions.
        ///
        /// The memory type of the memory region in the memory map that contains the given memory
        /// region will be returned. Therefore, 'region' is not allowed to intersect multiple memory
        /// regions.
        [[nodiscard]] auto find_type_of(const MemoryRegion& region) -> MemoryRegionType;

        auto operator[](size_t index) const -> const MemoryRegion&;

        [[nodiscard]]
        auto begin() const -> const MemoryRegion*;

        [[nodiscard]]
        auto end() const -> const MemoryRegion*;
    };
} // namespace Rune

#endif // RUNEOS_MEMORY_H
