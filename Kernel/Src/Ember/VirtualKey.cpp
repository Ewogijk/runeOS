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


#include <Ember/VirtualKey.h>


namespace Ember {
    const VirtualKey VirtualKey::NONE = VirtualKey();


    auto VirtualKey::build(const U8 row, const U8 col, bool released) -> VirtualKey {
        U16 key_code = (row & 0x7);
        key_code |= (col & 0x1F) << 3;
        key_code |= (released << 14);
        return VirtualKey(key_code);
    }

    auto VirtualKey::build_pressed(const U8 row, const U8 col) -> VirtualKey {
        return build(row, col, false);
    }


    VirtualKey VirtualKey::build_released(const U8 row, const U8 col) {
        return build(row, col, true);
    }


    VirtualKey::VirtualKey() : _key_code(0x8000) { }


    VirtualKey::VirtualKey(const U16 key_code) : _key_code(key_code) { }


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
}
