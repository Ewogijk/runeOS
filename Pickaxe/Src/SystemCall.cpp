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
    SystemCallPayload create_payload0(U16 handle) {
        return { handle, 0, 0, 0, 0, 0, 0 };
    }


    SystemCallPayload create_payload1(U16 handle, U64 arg_1) {
        return { handle, arg_1, 0, 0, 0, 0, 0 };
    }


    SystemCallPayload create_payload2(U16 handle, U64 arg_1, U64 arg_2) {
        return { handle, arg_1, arg_2, 0, 0, 0, 0 };
    }


    SystemCallPayload create_payload3(U16 handle, U64 arg_1, U64 arg_2, U64 arg_3) {
        return { handle, arg_1, arg_2, arg_3, 0, 0, 0 };
    }


    SystemCallPayload create_payload4(U16 handle, U64 arg_1, U64 arg_2, U64 arg_3, U64 arg_4) {
        return { handle, arg_1, arg_2, arg_3, arg_4, 0, 0 };
    }


    SystemCallPayload create_payload5(U16 handle, U64 arg_1, U64 arg_2, U64 arg_3, U64 arg_4, U64 arg_5) {
        return { handle, arg_1, arg_2, arg_3, arg_4, arg_5, 0 };
    }


    SystemCallPayload create_payload6(U16 handle, U64 arg_1, U64 arg_2, U64 arg_3, U64 arg_4, U64 arg_5, U64 arg_6) {
        return { handle, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6 };
    }
}