
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

#include <Ember/AppBits.h>

namespace Ember {
    constexpr U8 ROW_MASK = 0x7;
    constexpr U8 COL_MASK = 0x1F;

    constexpr U8 COl_SHIFT      = 3;
    constexpr U8 RELEASED_SHIFT = 14;
    constexpr U8 NONE_SHIFT     = 15;

    DEFINE_ENUM(StdIOTarget, STD_IO_TARGETS, 0x0)

    const VirtualKey VirtualKey::NONE = VirtualKey();

    auto VirtualKey::build(const U8 row, const U8 col, bool released) -> VirtualKey {
        U16 key_code  = (row & ROW_MASK);
        key_code     |= (col & COL_MASK) << COl_SHIFT;
        key_code     |= (static_cast<int>(released) << RELEASED_SHIFT);
        return VirtualKey(key_code);
    }

    auto VirtualKey::build_pressed(const U8 row, const U8 col) -> VirtualKey {
        return build(row, col, false);
    }

    auto VirtualKey::build_released(const U8 row, const U8 col) -> VirtualKey {
        return build(row, col, true);
    }

    VirtualKey::VirtualKey() : _key_code(NONE_KEY_CODE) {}

    VirtualKey::VirtualKey(const U16 key_code) : _key_code(key_code) {}

    auto VirtualKey::get_key_code() const -> U16 { return _key_code; }

    auto VirtualKey::get_row() const -> U8 {
        return _key_code & ROW_MASK; // key_code & 00000000000000111
    }

    auto VirtualKey::get_col() const -> U8 {
        return (_key_code >> COl_SHIFT) & COL_MASK; // key_code & 00000000001111000
    }

    auto VirtualKey::is_pressed() const -> bool {
        return ((_key_code >> RELEASED_SHIFT) & 0x1) == 0;
    }

    auto VirtualKey::is_released() const -> bool {
        return ((_key_code >> RELEASED_SHIFT) & 0x1) == 1; // key_code & 010000000000000000
    }

    auto VirtualKey::is_none() const -> bool {
        return ((_key_code >> NONE_SHIFT) & 0x1) != 0; // key_code & 100000000000000000
    }

    auto operator==(const VirtualKey& one, const VirtualKey& two) -> bool {
        return one.get_row() == two.get_row() && one.get_col() == two.get_col();
    }

    auto operator!=(const VirtualKey& one, const VirtualKey& two) -> bool {
        return one.get_row() != two.get_row() || one.get_col() != two.get_col();
    }
} // namespace Ember
