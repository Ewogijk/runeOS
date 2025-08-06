
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

#ifndef RUNEOS_MEMORY_H
#define RUNEOS_MEMORY_H


#include <Ember/Ember.h>
#include <Ember/Enum.h>

#include <KernelRuntime/Build.h>
#include <KernelRuntime/Stream.h>


namespace Rune {
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Smart Pointer
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * Simple std::unique_ptr implementation.
     */
    template<typename T>
    class UniquePointer {
        T* _ptr;


        void swap(UniquePointer<T>& other) noexcept {
            T* temp = _ptr;
            _ptr = other._ptr;
            other._ptr = temp;
        }


    public:
        UniquePointer() : _ptr(nullptr) {

        }


        explicit UniquePointer(T* ptr) : _ptr(ptr) {

        }


        ~UniquePointer() {
            delete _ptr;
        }


        UniquePointer(const UniquePointer<T>& o) = delete;


        UniquePointer& operator=(const UniquePointer<T>& o) = delete;


        UniquePointer(UniquePointer<T>&& o) noexcept: _ptr(nullptr) {
            swap(o);
        }


        UniquePointer& operator=(UniquePointer<T>&& o) noexcept {
            swap(o);
            return *this;
        }


        T* get() const {
            return _ptr;
        }


        explicit operator bool() const {
            return _ptr;
        }


        [[nodiscard]] T& operator*() const {
            return *_ptr;
        }


        T* operator->() const {
            return _ptr;
        }


        bool operator==(const UniquePointer<T>& o) const {
            return _ptr == o._ptr;
        }


        bool operator!=(const UniquePointer<T>& o) const {
            return _ptr != o._ptr;
        }
    };


    template<typename T>
    struct RefControlBlock {
        T* ptr = nullptr;
        size_t strong_ref_count = 0;
    };


    /**
     * Simple std::shared_ptr implementation.
     */
    template<typename T>
    class SharedPointer {
        RefControlBlock<T>* _refs;


        void init(T* ptr) {
            if (!ptr)
                return;
            _refs = new RefControlBlock<T>;
            _refs->ptr = ptr;
            ++_refs->strong_ref_count;
        }


        void swap(SharedPointer<T>& other) noexcept {
            RefControlBlock<T>* temp = _refs;
            _refs = other._refs;
            other._refs = temp;
        }


    public:
        SharedPointer() : _refs(nullptr) {
            init(nullptr);
        }


        explicit SharedPointer(T* ptr) : _refs(nullptr) {
            init(ptr);
        }


        ~SharedPointer() {
            if (!_refs)
                return;
            --_refs->strong_ref_count;
            if (_refs->strong_ref_count == 0) {
                delete _refs->ptr;
                delete _refs;
            }
        }


        SharedPointer(const SharedPointer<T>& o) noexcept: _refs(o._refs) {
            if (_refs) ++_refs->strong_ref_count;
        }


        SharedPointer& operator=(const SharedPointer<T>& o) noexcept {
            if (this == &o)
                return *this;

            if (_refs == o._refs) // Same ref control block
                return *this;

            // Different pointer -> Decrement current pointer ref count
            if (_refs) {
                --_refs->strong_ref_count;
                if (_refs->strong_ref_count == 0) {
                    delete _refs->ptr;
                    delete _refs;
                }
            }
            _refs = o._refs;
            if (_refs) ++_refs->strong_ref_count;
            return *this;
        }


        SharedPointer(SharedPointer<T>&& o) noexcept: _refs(o._refs) {
            o._refs = nullptr;
        }


        SharedPointer& operator=(SharedPointer<T>&& o) noexcept {
            SharedPointer<T> tmp(move(o));
            swap(tmp);
            return *this;
        }


        T* get() const {
            return _refs ? _refs->ptr : nullptr;
        }


        size_t get_ref_count() {
            return _refs ? _refs->strong_ref_count : 0;
        }


        explicit operator bool() const {
            return _refs;
        }


        [[nodiscard]] T& operator*() const {
            return *_refs->ptr;
        }


        T* operator->() const {
            return _refs ? _refs->ptr : nullptr;
        }


        bool operator==(const SharedPointer<T>& o) const {
            return _refs ? _refs->ptr == o._refs->ptr : _refs == o._refs;
        }


        bool operator!=(const SharedPointer<T>& o) const {
            return _refs ? _refs->ptr != o._refs->ptr : _refs != o._refs;
        }
    };


    #ifdef IS_64_BIT
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

#define MEMORY_UNITS(X)                 \
    X(MemoryUnit, B, 1)                 \
    X(MemoryUnit, KB, 1000)             \
    X(MemoryUnit, MB, 1000000)          \
    X(MemoryUnit, GB, 1000000000)       \
    X(MemoryUnit, TB, 1000000000000)    \
    X(MemoryUnit, KiB, 1024)            \
    X(MemoryUnit, MiB, 1048576)         \
    X(MemoryUnit, GiB, 1073741824)      \
    X(MemoryUnit, TiB, 1099511627776)   \

    /**
     * A memory unit e.g. kilobytes.
     */
    DECLARE_TYPED_ENUM(MemoryUnit, MemorySize, MEMORY_UNITS, 0x0) //NOLINT


    /**
     * Reformat the given bytes to another memory unit.
     *
     * @param bytes Bytes.
     * @param unit  Memory unit.
     *
     * @return Bytes converted to another unit.
     */
    MemoryFloatSize memory_bytes_in(MemorySize bytes, MemoryUnit unit);


    /**
     * Check if the memory address is aligned to the given boundary.
     *
     * @param mem_addr Memory address to checked.
     * @param boundary Memory boundary.
     * @return True if aligned else false.
     */
    bool memory_is_aligned(MemoryAddr mem_addr, MemoryAddr boundary);


    /**
     * Align the memory address to the given boundary. If roundUp is true the memory address will be rounded up to the
     * next aligned memory address (e.g. 4KB boundary: 5KB -> 8KB) else rounded down (e.g. 4KB boundary: 5KB -> 4KB)
     *
     * @param mem_addr  Memory address.
     * @param page_boundary Memory boundary.
     * @param round_up  If true the memory address is rounded up to the next boundary else rounded down.
     *
     * @return The aligned memory address.
     */
    MemoryAddr memory_align(MemoryAddr mem_addr, MemoryAddr page_boundary, bool round_up);


    /**
     *
     * @tparam T Pointer type.
     * @param v_addr
     * @return Pointer to the numerical value of the virtual address.
     */
    template<typename T>
    T* memory_addr_to_pointer(VirtualAddr v_addr) {
        return reinterpret_cast<T*>(v_addr);
    }


    /**
     *
     * @tparam T Pointer type.
     * @param pointer
     *
     * @return Virtual address of the pointer as numerical value.
     */
    template<typename T>
    MemoryAddr memory_pointer_to_addr(T* pointer) {
        return reinterpret_cast<uintptr_t>(pointer);
    }


    /**
     * Describes if a memory region is free to use or reserved for something else. If further information is available
     * the type may also describe which type of data is stored in the region (e.g. kernel code).
     */
#define MEMORY_REGION_TYPES(X)                          \
    X(MemoryRegionType, USABLE, 0x1)                    \
    X(MemoryRegionType, USED, 0x2)                      \
    X(MemoryRegionType, RESERVED, 0x3)                  \
    X(MemoryRegionType, USERSPACE, 0x4)                 \
    X(MemoryRegionType, HHDM, 0x5)                      \
    X(MemoryRegionType, PMM_RESERVED, 0x6)              \
    X(MemoryRegionType, VMM_RESERVED, 0x7)              \
    X(MemoryRegionType, KERNEL_HEAP, 0x8)               \
    X(MemoryRegionType, KERNEL_CODE, 0x9)               \
    X(MemoryRegionType, BOOTLOADER_RECLAIMABLE, 0xA)    \



    DECLARE_ENUM(MemoryRegionType, MEMORY_REGION_TYPES, 0x0) //NOLINT

    /**
     * A region of memory in the computers RAM.
     */
    struct MemoryRegion {
        MemoryAddr       start       = 0x0;
        MemorySize       size        = 0x0;
        MemoryRegionType memory_type = MemoryRegionType::NONE;


        [[nodiscard]] MemoryAddr end() const;


        [[nodiscard]] MemoryFloatSize size_in(MemoryUnit unit) const;


        [[nodiscard]] bool contains(const MemoryRegion& other) const;


        bool operator==(const MemoryRegion& b) const;


        bool operator!=(const MemoryRegion& b) const;


        bool operator<=(const MemoryRegion& o) const;
    };


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Memory Map
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    /**
     * A map of the computer's physical or virtual RAM.
     */
    class MemoryMap {
    public:// Maximum number of allowed memory regions in a map.
        static constexpr size_t LIMIT = 64;

    private:
        MemoryRegion _map[LIMIT];

        U64 _free_mem;

        U64 _reserved_mem;

        size_t _num_regions;
    public:

        explicit MemoryMap(MemoryRegion regions[LIMIT]);


        MemoryMap(std::initializer_list<MemoryRegion> regions);


        /**
         *
         * @return Number of memory regions with memory type unequal to "None" in the map.
         */
        [[nodiscard]] size_t size() const;


        /**
         *
         * @return Usable memory in bytes.
         */
        [[nodiscard]] MemorySize get_free_memory() const;


        /**
         *
         * @param unit Memory unit.
         * @return Usable memory converted to a memory unit.
         */
        [[nodiscard]] MemoryFloatSize get_free_memory_in(MemoryUnit unit) const;


        /**
         *
         * @return Reserved memory in bytes.
         */
        [[nodiscard]] MemorySize get_reserved_memory() const;


        /**
         *
         * @param unit Memory unit.
         * @return Reserved memory converted to a memory unit.
         */
        [[nodiscard]] MemoryFloatSize get_reserved_memory_in(MemoryUnit unit) const;


        /**
         *
         * @return Total memory in bytes.
         */
        [[nodiscard]] MemorySize get_total_memory() const;


        /**
         *
         * @param unit Memory unit.
         *
         * @return Total memory converted to a memory unit.
         */
        [[nodiscard]] MemoryFloatSize get_total_memory_in(MemoryUnit unit) const;


        /**
         * Claim the memory region defined by the `claimant` and mark it with the memory type of the `claimant`. The
         * `claimant` must lie completely within the targeted memory region and will be aligned to given `boundary` if
         * not properly aligned.
         *
         * <p>
         * Claiming fails if the claimed region needs to be split but no more space for more memory regions is
         * available.
         * </p>
         *
         * <p>
         * If the memory type of the `claimant` is `Usable` the memory will be freed otherwise reserved.
         * </p>
         *
         * @param claimant Memory region that should be
         * @param boundary Page boundary.
         *
         * @return True if the region got claimed else false.
         */
        bool claim(MemoryRegion& claimant, uint32_t boundary);


        /**
         * Merge adjacent regions of the same type into bigger regions.
         */
        void merge();


        /**
         *  Print the memory map to the text output.
         *
         * @param out       Text output.
         * @param region_unit Memory region sizes will be converted to this memory unit.
         * @param map_unit    Memory map statistics will be converted to this memory unit.
         */
        void dump(TextStream* out, MemoryUnit region_unit, MemoryUnit map_unit) const;


        const MemoryRegion& operator[](size_t index) const;


        [[nodiscard]] const MemoryRegion* begin() const;


        [[nodiscard]] const MemoryRegion* end() const;
    };
}

#endif //RUNEOS_MEMORY_H
