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

#include <Pickaxe/AppManagement.h>


namespace Rune::Pickaxe {


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Virtual Key
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    const VirtualKey VirtualKey::NONE = VirtualKey();


    VirtualKey VirtualKey::build(U8 row, U8 col, bool released) {
        U16 key_code = (row & 0x7);
        key_code |= (col & 0x1F) << 3;
        key_code |= (released << 14);
        return VirtualKey(key_code);
    }


    VirtualKey::VirtualKey() : _key_code(0x8000) {

    }


    VirtualKey::VirtualKey(U16 key_code) : _key_code(key_code) {

    }


    U16 VirtualKey::get_key_code() const {
        return _key_code;
    }


    U8 VirtualKey::get_row() const {
        return _key_code & 0x7; // key_code & 00000000000000111
    }


    U8 VirtualKey::get_col() const {
        return (_key_code >> 3) & 0x1F; // key_code & 00000000001111000
    }


    bool VirtualKey::is_pressed() const {
        return ((_key_code >> 14) & 0x1) == 0;
    }


    bool VirtualKey::is_released() const {
        return ((_key_code >> 14) & 0x1) == 1; // key_code & 010000000000000000
    }


    bool VirtualKey::is_none() const {
        return (_key_code >> 15) & 0x1; // key_code & 100000000000000000
    }


    bool operator==(const VirtualKey& one, const VirtualKey& two) {
        return one.get_row() == two.get_row() && one.get_col() == two.get_col();
    }


    bool operator!=(const VirtualKey& one, const VirtualKey& two) {
        return one.get_row() != two.get_row() || one.get_col() != two.get_col();
    }


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          System Calls
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    VirtualKey read_std_in() {
        U16               key_code_out = 0;
        SystemCallPayload payload      = create_payload1(200, (uintptr_t) &key_code_out);
        int               ret          = make_system_call(&payload);
        return ret >= 0 ? VirtualKey(key_code_out) : VirtualKey::NONE;
    }


    void write_std_out(const char* msg, size_t length) {
        SystemCallPayload payload = create_payload2(201, (uintptr_t) msg, length);
        make_system_call(&payload);
    }


    void write_std_err(const char* msg, size_t length) {
        SystemCallPayload payload = create_payload2(202, (uintptr_t) msg, length);
        make_system_call(&payload);
    }


    int app_start(
            const char* app_path,
            const char** argv,
            const char* working_directory,
            const char* stdin_target,
            const char* stdout_target,
            const char* stderr_target
    ) {
        SystemCallPayload payload = create_payload6(
                203,
                (uintptr_t) app_path,
                (uintptr_t) argv,
                (uintptr_t) working_directory,
                (uintptr_t) stdin_target,
                (uintptr_t) stdout_target,
                (uintptr_t) stderr_target
        );
        return make_system_call(&payload);
    }


    void app_exit(int exit_code) {
        SystemCallPayload payload = create_payload1(204, exit_code);
        make_system_call(&payload);
    }


    int app_join(int app_handle) {
        SystemCallPayload payload = create_payload1(205, app_handle);
        return make_system_call(&payload);
    }


    S64 app_get_working_directory(const char* wd_out, int wd_size) {
        SystemCallPayload payload = create_payload2(206, (uintptr_t) wd_out, wd_size);
        return make_system_call(&payload);
    }


    S64 app_change_working_directory(const char* new_wd) {
        SystemCallPayload payload = create_payload1(207, (uintptr_t) new_wd);
        return make_system_call(&payload);
    }
}