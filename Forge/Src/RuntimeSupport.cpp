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

#include <Hammer/String.h>
#include <Hammer/Memory.h>

#include <Pickaxe/AppManagement.h>

#include "LibAlloc11.h"


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
// Will be called when there is no implementation of a pure virtual function
// (should not happen because compiler cries)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


extern "C" void __cxa_pure_virtual() {
    Rune::String m = "Pure virtual function without implementation called!";
    Rune::Pickaxe::write_std_err(m.to_cstr(), m.size());
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                      Stack smash protection
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


// TODO __stack_chk_guard should be a randomly generated value
#if UINT32_MAX == UINTPTR_MAX
#define STACK_CHK_GUARD 0xe2dee396
#else
#define STACK_CHK_GUARD 0x595e9fbd94fda766
#endif

uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

extern "C" void __stack_chk_fail(void) {
    Rune::String m = "Yoho, the stack got smashed real hard!";
    Rune::Pickaxe::write_std_err(m.to_cstr(), m.size());
}


void* operator new(size_t size) {
    return malloc(size);
}


void* operator new[](size_t size) {
    return malloc(size);
}


// Non-allocating placement allocation
void* operator new(size_t count, void* ptr) {
    SILENCE_UNUSED(count)
    return ptr;
}


void* operator new[](size_t count, void* ptr) {
    SILENCE_UNUSED(count)
    return ptr;
}


void operator delete(void* p) noexcept {
    return free(p);
}


void operator delete(void* p, size_t size) noexcept {
    SILENCE_UNUSED(size)
    return free(p);
}


void operator delete[](void* p) noexcept {
    return free(p);
}


void operator delete[](void* p, size_t size) noexcept {
    SILENCE_UNUSED(size)
    return free(p);
}