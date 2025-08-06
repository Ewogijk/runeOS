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

#include <KernelRuntime/CppLanguageSupport.h>


void (* ON_CXA_PURE_VIRTUAL)() = [] { };


void (* ON_STACK_GUARD_FAIL)() = [] { };


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                              C/C++ Compiler expects these to be defined
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//



void* memset(void* dest, const int ch, const size_t count) {
    auto* d = static_cast<unsigned char*>(dest);

    for (size_t i = 0; i < count; i++)
        d[i]      = ch;
    return dest;
}


void* memcpy(void* dest, void* src, const size_t count) {
    auto*       d = static_cast<unsigned char*>(dest);
    const auto* s = static_cast<unsigned char*>(src);

    for (size_t i = 0; i < count; i++)
        d[i]      = s[i];

    return dest;
}


void* memmove(void* dest, void* src, const size_t count) {
    const uintptr_t sourceEnd = reinterpret_cast<uintptr_t>(src) + count;
    auto*           d         = static_cast<unsigned char*>(dest);
    const auto*     s         = static_cast<unsigned char*>(src);
    if (reinterpret_cast<uintptr_t>(dest) <= sourceEnd && sourceEnd <= reinterpret_cast<uintptr_t>(dest) + count) {
        // Source overlaps from left -> Copy from end
        //     dddddd
        //  ssssss
        for (size_t i = count; i > 0; i--)
            d[i - 1]  = s[i - 1];
    } else {
        // Source overlaps from right or no overlap -> Copy from start
        //    ssssss
        // dddddd
        // or
        // ssssss
        //        dddddd
        for (size_t i = 0; i < count; i++)
            d[i]      = s[i];
    }
    return dest;
}


int memcmp(const void* lhs, const void* rhs, const size_t count) {
    const auto* l = static_cast<const unsigned char*>(lhs);
    const auto* r = static_cast<const unsigned char*>(rhs);

    for (size_t i = 0; i < count; i++) {
        if (l[i] < r[i])
            return -1;
        else if (l[i] > r[i])
            return 1;
    }
    return 0;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
// Will be called when there is no implementation of a pure virtual function
// (should not happen because compiler cries)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


CLINK void __cxa_pure_virtual() {
    ON_CXA_PURE_VIRTUAL();
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

CLINK void __stack_chk_fail(void) {
    ON_STACK_GUARD_FAIL();
}

namespace Rune {


    void init_kernel_runtime_env(void(* on_cxa_pure_virtual)(), void(* on_stack_guard_fail)()) {
        ON_CXA_PURE_VIRTUAL = on_cxa_pure_virtual;
        ON_STACK_GUARD_FAIL = on_stack_guard_fail;
    }
}