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

#ifndef RUNEOS_UTILITY_H
#define RUNEOS_UTILITY_H


#include <Ember/Definitions.h>
#include <Hammer/Memory.h>


namespace Rune {
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Move&Forward Semantics
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    template<typename T>
    struct RemoveRef {
        typedef T Type;
    };

    template<typename T>
    struct RemoveRef<T&> {
        typedef T Type;
    };

    template<typename T>
    struct RemoveRef<T&&> {
        typedef T Type;
    };


    /**
     * Implementation of std::move.
     *
     * @tparam T
     * @param param
     * @return
     */
    template<typename T>
    typename RemoveRef<T>::Type&& move(T&& param) {
        return static_cast<typename RemoveRef<T>::Type&&>(param);
    }


    /**
     * Implementation of std::forward.
     *
     * @tparam T
     * @param param
     * @return
     */
    template<typename T>
    constexpr T&& forward(typename RemoveRef<T>::Type& param) {
        return static_cast<T&&>(param);
    }


    /**
     * Implementation of std::forward.
     *
     * @tparam T
     * @param param
     * @return
     */
    template<typename T>
    constexpr T&& forward(typename RemoveRef<T>::Type&& param) {
        return static_cast<T&&>(param);
    }


    /**
     * Swap the position of the two pointers.
     *
     * @param one
     * @param two
     */
    template<typename T>
    void swap(T& one, T& two) {
        T temp = move(one);
        one = move(two);
        two = move(temp);
    }


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          std::function Port
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    template<typename R, typename ...A>
    class ICallable {
    public:
        virtual ~ICallable() = default;


        virtual R operator()(A ...) const = 0;


        virtual void copy_to(void* dest) const = 0;


        virtual ICallable<R, A...>* clone() const = 0;
    };


    template<typename F, typename R, typename ...A>
    class Callable : public ICallable<R, A...> {
        F _function;

    public:
        explicit Callable(F function) : _function(function) {
        }


        R operator()(A ... a) const override {
            return _function(forward<A>(a)...);
        }


        void copy_to(void* destination) const override {
            new(destination) Callable(_function);
        }


        ICallable<R, A...>* clone() const override {
            return new Callable(_function);
        }
    };


    template<typename>
    class Function;


    template<typename R, typename... A>
    class Function<R(A...)> {
        static constexpr size_t STACK_LIMIT = 24;

        ICallable<R, A...>* _function;
        unsigned char _stack_ptr[STACK_LIMIT];

    public:

        template<typename Func>
        Function(Func function) : _function(nullptr), _stack_ptr() {
            if (sizeof(Callable<Func, R, A...>) <= STACK_LIMIT) {
                _function = (decltype(_function)) &_stack_ptr;
                new(_function) Callable<Func, R, A...>(function);
            } else {
                _function = new Callable<Func, R, A...>(function);
            }
        }


        ~Function() {
            if (_function == (decltype(_function)) &_stack_ptr)
                _function->~ICallable();
            else
                delete _function;
        }


        Function(const Function& o) : _function(nullptr), _stack_ptr() {
            if (o._function != nullptr) {
                if (o._function == (decltype(o._function)) &o._stack_ptr) {
                    _function = (decltype(_function)) &_stack_ptr;
                    o._function->copy_to(_function);
                } else {
                    _function = o._function->clone();
                }
            }
        }


        Function& operator=(const Function& o) {
            if (_function != nullptr) {
                if (_function == (decltype(_function)) &_stack_ptr)
                    _function->~ICallable();
                else
                    delete _function;
            }

            if (o._function != nullptr) {
                if (o._function == (decltype(o._function)) &o._stack_ptr) {
                    _function = (decltype(_function)) &_stack_ptr;
                    o._function->copy_to(_function);
                } else {
                    _function = o._function->clone();
                }
            } else {
                _function = nullptr;
            }
            return *this;
        }


        Function(Function&& o) noexcept: _function(nullptr), _stack_ptr() {
            if (o._function == (decltype(o._function)) &o._stack_ptr) {
                _function = (decltype(_function)) &_stack_ptr;
                o._function->copy_to(_function);
                o._function->~ICallable();
                o._function = nullptr;
            } else if (o._function) {
                swap(_function, o._function);
            }
        }


        Function& operator=(Function&& o) {
            if (_function != nullptr) {
                if (_function == (decltype(_function)) &_stack_ptr)
                    _function->~ICallable();
                else
                    delete _function;
            }

            if (o._function != nullptr) {
                if (o._function == (decltype(o._function)) &o._stack_ptr) {
                    _function = (decltype(_function)) &_stack_ptr;
                    o._function->copy_to(_function);
                    o._function->~ICallable();
                } else {
                    swap(_function, o._function);
                    //delete o._function; dont need -> points to old _function
                }
            } else {
                _function = nullptr;
            }
            o._function = nullptr;
            return *this;
        }


        R operator()(A ... args) const {
            return (*_function)(forward<A>(args)...);
        }
    };
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                          Initializer List
// initializer_list needs to be in the "std" namespace because this is a compiler requirement
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

namespace std {
    template<class _E>
    class initializer_list {
    public:
        typedef _E value_type;
        typedef const _E& reference;
        typedef const _E& const_reference;
        typedef size_t size_type;
        typedef const _E* iterator;
        typedef const _E* const_iterator;

    private:
        iterator  _M_array;
        size_type _M_len;


        // The compiler can call a private constructor.
        constexpr initializer_list(const_iterator __a, size_type __l)
                : _M_array(__a), _M_len(__l) {
        }


    public:
        constexpr initializer_list() noexcept
                : _M_array(0), _M_len(0) {
        }


        // Number of elements.
        constexpr size_type
        size() const noexcept {
            return _M_len;
        }


        // First element.
        constexpr const_iterator
        begin() const noexcept {
            return _M_array;
        }


        // One past the last element.
        constexpr const_iterator
        end() const noexcept {
            return begin() + size();
        }
    };


    template<class _Tp>
    constexpr const _Tp*
    begin(initializer_list<_Tp> __ils) noexcept {
        return __ils.begin();
    }


    template<class _Tp>
    constexpr const _Tp*
    end(initializer_list<_Tp> __ils) noexcept {
        return __ils.end();
    }
}


#endif //RUNEOS_UTILITY_H
