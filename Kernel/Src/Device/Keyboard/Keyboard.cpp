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

#include <Device/Keyboard/Keyboard.h>


namespace Rune::Device {
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          VirtualKey
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    const VirtualKey VirtualKey::NONE = VirtualKey();


    VirtualKey::VirtualKey() : _key_code(0x8000) {

    }


    VirtualKey::VirtualKey(U16 key_code) : _key_code(key_code) {

    }


    VirtualKey VirtualKey::build(U8 row, U8 col, bool released) {
        U16 key_code = (row & 0x7);
        key_code |= (col & 0x1F) << 3;
        key_code |= (released << 14);
        return VirtualKey(key_code);
    }


    VirtualKey VirtualKey::build_pressed(U8 row, U8 col) {
        return build(row, col, false);
    }


    VirtualKey VirtualKey::build_released(U8 row, U8 col) {
        return build(row, col, true);
    }


    U16 VirtualKey::get_key_code() const {
        return _key_code;
    }


    U8 VirtualKey::get_row() const {
        return _key_code & 0x7; // _key_code & 00000000000000111
    }


    U8 VirtualKey::get_col() const {
        return (_key_code >> 3) & 0x1F; // _key_code & 00000000001111000
    }


    bool VirtualKey::is_pressed() const {
        return ((_key_code >> 14) & 0x1) == 0;
    }


    bool VirtualKey::is_released() const {
        return ((_key_code >> 14) & 0x1) == 1; // _key_code & 010000000000000000
    }


    bool VirtualKey::is_none() const {
        return (_key_code >> 15) & 0x1; // _key_code & 100000000000000000
    }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                          KeyStream
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    bool VirtualKeyboard::is_read_supported() {
        return true;
    }


    bool VirtualKeyboard::is_write_supported() {
        return false;
    }


    bool VirtualKeyboard::write(U8 value) {
        SILENCE_UNUSED(value)
        return false;
    }


    void VirtualKeyboard::close() {
        // The keyboard is not a managed resource
    }


    bool VirtualKeyboard::is_ansi_supported() {
        return false;
    }
}