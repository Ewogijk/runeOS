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


#include <Hammer/Definitions.h>


CLINK void* memset(void* dest, int ch, size_t count);


CLINK void* memcpy(void* dest, void* src, size_t count);


CLINK void* memmove(void* dest, void* src, size_t count);


CLINK int memcmp(const void* lhs, const void* rhs, size_t count);


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                          New / Delete operators
// Note: Implementations must be provided by the Kernel or Userspace
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


void* operator new(size_t size);


void* operator new[](size_t size);


// Non-allocating placement allocation
void* operator new(size_t count, void* ptr);


void* operator new[](size_t count, void* ptr);


void operator delete(void* p) noexcept;


void operator delete(void* p, size_t size) noexcept;


void operator delete[](void* p) noexcept;


void operator delete[](void* p, size_t size) noexcept;


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


        void swap(UniquePointer<T>& other) {
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
            _refs->strong_ref_count++;
        }


        void swap(SharedPointer<T>& other) {
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
            _refs->strong_ref_count--;
            if (_refs->strong_ref_count == 0) {
                delete _refs->ptr;
                delete _refs;
            }
        }


        SharedPointer(const SharedPointer<T>& o) noexcept: _refs(o._refs) {
            if (_refs)
                _refs->strong_ref_count++;
        }


        SharedPointer& operator=(const SharedPointer<T>& o) noexcept {
            if (this == &o)
                return *this;

            if (_refs == o._refs) // Same ref control block
                return *this;

            // Different pointer -> Decrement current pointer ref count
            if (_refs) {
                _refs->strong_ref_count--;
                if (_refs->strong_ref_count == 0) {
                    delete _refs->ptr;
                    delete _refs;
                }
            }
            _refs = o._refs;
            if (_refs)
                _refs->strong_ref_count++;
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
}

#endif //RUNEOS_MEMORY_H
