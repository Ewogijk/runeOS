
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

#ifndef RUNEOS_CPPLANGUAGESUPPORT_H
#define RUNEOS_CPPLANGUAGESUPPORT_H

#include <Ember/Ember.h>


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                              C/C++ Compiler expects these to be defined
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


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


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
// Will be called when there is no implementation of a pure virtual function
// (should not happen because compiler cries)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


CLINK void __cxa_pure_virtual();


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                      Stack smash protection
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


CLINK void __stack_chk_fail(void);


namespace Rune {

    /**
     * @brief Init the kernel runtime environment.
     * @param on_cxa_pure_virtual Callback for "__cxa_pure_virtual"
     * @param on_stack_guard_fail Callback for "__stack_chk_fail"
     */
    void init_kernel_runtime_env(void(* on_cxa_pure_virtual)(), void(* on_stack_guard_fail)());
}

#endif //RUNEOS_CPPLANGUAGESUPPORT_H
