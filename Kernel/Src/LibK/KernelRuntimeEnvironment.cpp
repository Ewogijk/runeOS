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

#include <LibK/KernelRuntimeEnv.h>


void (* ON_CXA_PURE_VIRTUAL)() = [] { };


void (* ON_STACK_GUARD_FAIL)() = [] { };


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