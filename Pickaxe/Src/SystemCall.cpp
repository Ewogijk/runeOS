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

#include <Pickaxe/SystemCall.h>


namespace Rune::Pickaxe {

    S64 system_call0(U16 handle) {
        S64 res = -1;
        __asm__ __volatile__ ("syscall" : "=a" (res) : "a" (handle) : "rcx", "r11", "memory");
        return res;
    }


    S64 system_call1(U16 handle, U64 arg1) {
        S64 res = -1;
        __asm__ __volatile__ ("syscall" : "=a" (res) : "a" (handle), "D" (arg1) : "rcx", "r11", "memory");
        return res;
    }


    S64 system_call2(U16 handle, U64 arg1, U64 arg2) {
        S64 res = -1;
        __asm__ __volatile__ ("syscall"
                : "=a" (res)
                : "a" (handle), "D" (arg1), "S" (arg2)
                : "rcx", "r11", "memory");
        return res;
    }


    S64 system_call3(U16 handle, U64 arg1, U64 arg2, U64 arg3) {
        S64 res = -1;
        __asm__ __volatile__ ("syscall"
                : "=a" (res)
                : "a" (handle), "D" (arg1), "S" (arg2), "d" (arg3)
                : "rcx", "r11", "memory");
        return res;
    }


    S64 system_call4(U16 handle, U64 arg1, U64 arg2, U64 arg3, U64 arg4) {
        S64 res = -1;
        register U64 r8 __asm__("r8") = arg4;
        __asm__ __volatile__ ("syscall"
                : "=a" (res)
                : "a" (handle), "D" (arg1), "S" (arg2), "d" (arg3), "r" (r8)
                : "rcx", "r11", "memory");
        return res;
    }


    S64 system_call5(U16 handle, U64 arg1, U64 arg2, U64 arg3, U64 arg4, U64 arg5) {
        S64 res = -1;
        register U64 r8 __asm__("r8") = arg4;
        register U64 r9 __asm__("r9") = arg5;
        __asm__ __volatile__ ("syscall"
                : "=a" (res)
                : "a" (handle), "D" (arg1), "S" (arg2), "d" (arg3), "r" (r8), "r" (r9)
                : "rcx", "r11", "memory");
        return res;
    }


    S64 system_call6(U16 handle, U64 arg1, U64 arg2, U64 arg3, U64 arg4, U64 arg5, U64 arg6) {
        S64 res = -1;
        register U64 r8 __asm__("r8") = arg4;
        register U64 r9 __asm__("r9") = arg5;
        register U64 r10 __asm__("r10") = arg6;
        __asm__ __volatile__ ("syscall"
                : "=a" (res)
                : "a" (handle), "D" (arg1), "S" (arg2), "d" (arg3), "r" (r8), "r" (r9), "r" (r10)
                : "rcx", "r11", "memory");
        return res;
    }
}