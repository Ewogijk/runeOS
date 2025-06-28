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

#ifndef RUNEOS_SYSTEMCALL_H
#define RUNEOS_SYSTEMCALL_H


#include <stdint.h>
#include <stddef.h>


using U8 = uint8_t;
using U16 = uint16_t;
using U64 = uint64_t;
using S64 = int64_t;

namespace Rune::Pickaxe {
    /**
     * @brief Maximum allowed string size (including the null terminator).
     */
    static const U16 MAX_STRING_SIZE = 128;


    /**
     * @brief Make a system call with zero arguments.
     * @param handle System call handle.
     * @return A handle returned by the system call.
     */
    S64 system_call0(U16 handle);


    /**
     * @brief Make a system call with zero arguments.
     * @param handle System call handle.
     * @param arg1   First system call argument.
     * @return A handle returned by the system call.
     */
    S64 system_call1(U16 handle, U64 arg1);


    /**
     * @brief Make a system call with zero arguments.
     * @param handle System call handle.
     * @param arg1   First system call argument.
     * @param arg2   Second system call argument.
     * @return A handle returned by the system call.
     */
    S64 system_call2(U16 handle, U64 arg1, U64 arg2);


    /**
     * @brief Make a system call with zero arguments.
     * @param handle System call handle.
     * @param arg1   First system call argument.
     * @param arg2   Second system call argument.
     * @param arg3   Third system call argument.
     * @return A handle returned by the system call.
     */
    S64 system_call3(U16 handle, U64 arg1, U64 arg2, U64 arg3);


    /**
     * @brief Make a system call with zero arguments.
     * @param handle System call handle.
     * @param arg1   1st system call argument.
     * @param arg2   2nd system call argument.
     * @param arg3   3rd system call argument.
     * @param arg4   4th system call argument.
     * @return A handle returned by the system call.
     */
    S64 system_call4(U16 handle, U64 arg1, U64 arg2, U64 arg3, U64 arg4);


    /**
     * @brief Make a system call with zero arguments.
     * @param handle System call handle.
     * @param arg1   1st system call argument.
     * @param arg2   2nd system call argument.
     * @param arg3   3rd system call argument.
     * @param arg4   4th system call argument.
     * @param arg5   5th system call argument.
     * @return A handle returned by the system call.
     */
    S64 system_call5(U16 handle, U64 arg1, U64 arg2, U64 arg3, U64 arg4, U64 arg5);


    /**
     * @brief Make a system call with zero arguments.
     * @param handle System call handle.
     * @param arg1   1st system call argument.
     * @param arg2   2nd system call argument.
     * @param arg3   3rd system call argument.
     * @param arg4   4th system call argument.
     * @param arg5   5th system call argument.
     * @param arg6   6th system call argument.
     * @return A handle returned by the system call.
     */
    S64 system_call6(U16 handle, U64 arg1, U64 arg2, U64 arg3, U64 arg4, U64 arg5, U64 arg6);
}

#endif //RUNEOS_SYSTEMCALL_H
