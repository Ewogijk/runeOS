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
     * @brief The handle of a system call with up to 6 arguments that will be passed to the kernel.
     */
    struct SystemCallPayload {
        U16 handle = 0;
        U64 arg_1  = 0;
        U64 arg_2  = 0;
        U64 arg_3  = 0;
        U64 arg_4  = 0;
        U64 arg_5  = 0;
        U64 arg_6  = 0;
    };


    /**
     * @brief Make a system call with the given arguments.
     *
     * This function makes the system call directly at assembly level, it will simply pass the arguments to the kernel
     * as specified in the SystemCall ABI.
     *
     * @param sys_call_args A pointer to the system call arguments.
     * @return The return value from the kernel.
     */
    extern "C" int make_system_call(SystemCallPayload* sys_call_payload);


    /**
     * @brief Create a system call payload with zero args.
     * @param id
     * @return
     */
    SystemCallPayload create_payload0(U16 handle);


    /**
     * @brief Create a system call payload with one args.
     * @param id
     * @return
     */
    SystemCallPayload create_payload1(U16 handle, U64 arg_1);


    /**
     * @brief Create a system call payload with two args.
     * @param id
     * @return
     */
    SystemCallPayload create_payload2(U16 handle, U64 arg_1, U64 arg_2);


    /**
     * @brief Create a system call payload with three args.
     * @param id
     * @return
     */
    SystemCallPayload create_payload3(U16 handle, U64 arg_1, U64 arg_2, U64 arg_3);


    /**
     * @brief Create a system call payload with four args.
     * @param id
     * @return
     */
    SystemCallPayload create_payload4(U16 handle, U64 arg_1, U64 arg_2, U64 arg_3, U64 arg_4);


    /**
     * @brief Create a system call payload with five args.
     * @param id
     * @return
     */
    SystemCallPayload create_payload5(U16 handle, U64 arg_1, U64 arg_2, U64 arg_3, U64 arg_4, U64 arg_5);


    /**
     * @brief Create a system call payload with six args.
     * @param id
     * @return
     */
    SystemCallPayload create_payload6(U16 handle, U64 arg_1, U64 arg_2, U64 arg_3, U64 arg_4, U64 arg_5, U64 arg_6);
}

#endif //RUNEOS_SYSTEMCALL_H
