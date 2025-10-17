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

#include <KRE/CppRuntimeSupport.h>

void (*ON_CXA_PURE_VIRTUAL)() = [] {}; // NOLINT

void (*ON_STACK_GUARD_FAIL)() = [] {}; // NOLINT

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                              C/C++ Compiler expects these to be defined
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

auto memset(void* dest, const int chr, const size_t count) -> void* {
    auto* dest_ch = static_cast<unsigned char*>(dest);
    for (size_t i = 0; i < count; i++) {
        dest_ch[i] = chr;
    }
    return dest;
}

auto memcpy(void* dest, const void* src, const size_t count) -> void* {
    auto*       dest_ch = static_cast<unsigned char*>(dest);
    const auto* src_ch  = static_cast<const unsigned char*>(src);

    for (size_t i = 0; i < count; i++) {
        dest_ch[i] = src_ch[i];
    }

    return dest;
}

auto memmove(void* dest, const void* src, const size_t count) -> void* {
    const uintptr_t sourceEnd = reinterpret_cast<uintptr_t>(src) + count; // NOLINT
    auto*           dest_ch   = static_cast<unsigned char*>(dest);
    const auto*     src_ch    = static_cast<const unsigned char*>(src);
    if (reinterpret_cast<uintptr_t>(dest) <= sourceEnd               // NOLINT
        && sourceEnd <= reinterpret_cast<uintptr_t>(dest) + count) { // NOLINT
        // Source overlaps from left -> Copy from end
        //     dddddd
        //  ssssss
        for (size_t i = count; i > 0; i--) {
            dest_ch[i - 1] = src_ch[i - 1];
        }
    } else {
        // Source overlaps from right or no overlap -> Copy from start
        //    ssssss
        // dddddd
        // or
        // ssssss
        //        dddddd
        for (size_t i = 0; i < count; i++) {
            dest_ch[i] = src_ch[i];
        }
    }
    return dest;
}

auto memcmp(const void* lhs, const void* rhs, const size_t count) -> int {
    const auto* left  = static_cast<const unsigned char*>(lhs);
    const auto* right = static_cast<const unsigned char*>(rhs);

    for (size_t i = 0; i < count; i++) {
        if (left[i] < right[i]) {
            return -1;
        }
        if (left[i] > right[i]) {
            return 1;
        }
    }
    return 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
// Will be called when there is no implementation of a pure virtual function
// (should not happen because compiler cries)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

CLINK void __cxa_pure_virtual() { ON_CXA_PURE_VIRTUAL(); } // NOLINT

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                      Stack smash protection
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

// TODO __stack_chk_guard should be a randomly generated value
#if UINT32_MAX == UINTPTR_MAX
#define STACK_CHK_GUARD 0xe2dee396 // NOLINT
#else
#define STACK_CHK_GUARD 0x595e9fbd94fda766 // NOLINT
#endif

uintptr_t __stack_chk_guard = STACK_CHK_GUARD; // NOLINT

CLINK void __stack_chk_fail(void) { ON_STACK_GUARD_FAIL(); } // NOLINT


auto atexit(void (*func)()) -> int {
    // NOP, because after the kernel exits there is only darkness
    return 0;
}

namespace Rune {

    void init_cpp_runtime_support(void (*on_cxa_pure_virtual)(), void (*on_stack_guard_fail)()) {
        ON_CXA_PURE_VIRTUAL = on_cxa_pure_virtual;
        ON_STACK_GUARD_FAIL = on_stack_guard_fail;
    }
} // namespace Rune