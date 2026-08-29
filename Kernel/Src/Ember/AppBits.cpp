
//  Copyright 2026 Ewogijk
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
    DEFINE_ENUM(StdIOTarget, STD_IO_TARGETS, 0x0)

    // ========================================================================================== //
    // KeyEvent
    // ========================================================================================== //

    DEFINE_TYPED_ENUM(VirtualKey, U8, VIRTUAL_KEYS, 0x0)

    const KeyEvent KeyEvent::NONE = KeyEvent();

    KeyEvent::KeyEvent() : m_event_code(EVENT_CODE_NONE) {}

    KeyEvent::KeyEvent(const U32 event_code) : m_event_code(event_code) {}

    auto KeyEvent::event_code() const -> U32 { return m_event_code.m_as_u32; }

    auto KeyEvent::virtual_key() const -> VirtualKey {
        return VirtualKey(static_cast<U8>(m_event_code.m_virtual_key));
    }

    auto KeyEvent::is_key_down() const -> bool { return m_event_code.m_key_down > 0; }

    auto KeyEvent::is_lctrl_down() const -> bool { return m_event_code.m_lctrl > 0; }

    auto KeyEvent::is_lshift_down() const -> bool { return m_event_code.m_lshift > 0; }

    auto KeyEvent::is_lalt_down() const -> bool { return m_event_code.m_lalt > 0; }

    auto KeyEvent::is_lgui_down() const -> bool { return m_event_code.m_lgui > 0; }

    auto KeyEvent::is_rctrl_down() const -> bool { return m_event_code.m_rctrl > 0; }

    auto KeyEvent::is_rshift_down() const -> bool { return m_event_code.m_rshift > 0; }

    auto KeyEvent::is_ralt_down() const -> bool { return m_event_code.m_ralt > 0; }

    auto KeyEvent::is_rgui_down() const -> bool { return m_event_code.m_rgui > 0; }

    auto operator==(const KeyEvent& one, const KeyEvent& two) -> bool {
        return one.m_event_code.m_as_u32 == two.m_event_code.m_as_u32;
    }

    auto operator!=(const KeyEvent& one, const KeyEvent& two) -> bool {
        return one.m_event_code.m_as_u32 != two.m_event_code.m_as_u32;
    }

    // ========================================================================================== //
    // Key Event Builder
    // ========================================================================================== //

    constexpr U32 VIRTUAL_KEY_MASK = 0xFF;

    constexpr U8 LCTRL_OFFSET    = 8;
    constexpr U8 LSHIFT_OFFSET   = 9;
    constexpr U8 LALT_OFFSET     = 10;
    constexpr U8 LGUI_OFFSET     = 11;
    constexpr U8 RCTRL_OFFSET    = 12;
    constexpr U8 RSHIFT_OFFSET   = 13;
    constexpr U8 RALT_OFFSET     = 14;
    constexpr U8 RGUI_OFFSET     = 15;
    constexpr U8 KEY_DOWN_OFFSET = 16;

    auto bit_modify(U32 num, U8 bit_offset, bool bit_value) -> U32 {
        return bit_value ? num | (1 << bit_offset) : num & ~(1 << bit_offset);
    }

    auto KeyEventBuilder::with_virtual_key(const VirtualKey vk) -> KeyEventBuilder& {
        m_event_code = (m_event_code & ~VIRTUAL_KEY_MASK) | vk.to_value();
        return *this;
    }

    auto KeyEventBuilder::with_lctrl(const bool lctrl) -> KeyEventBuilder& {
        m_event_code = bit_modify(m_event_code, LCTRL_OFFSET, lctrl);
        return *this;
    }

    auto KeyEventBuilder::with_lshift(const bool lshift) -> KeyEventBuilder& {
        m_event_code = bit_modify(m_event_code, LSHIFT_OFFSET, lshift);
        return *this;
    }

    auto KeyEventBuilder::with_lalt(const bool lalt) -> KeyEventBuilder& {
        m_event_code = bit_modify(m_event_code, LALT_OFFSET, lalt);
        return *this;
    }

    auto KeyEventBuilder::with_lgui(const bool lgui) -> KeyEventBuilder& {
        m_event_code = bit_modify(m_event_code, LGUI_OFFSET, lgui);
        return *this;
    }

    auto KeyEventBuilder::with_rctrl(const bool rctrl) -> KeyEventBuilder& {
        m_event_code = bit_modify(m_event_code, RCTRL_OFFSET, rctrl);
        return *this;
    }

    auto KeyEventBuilder::with_rshift(const bool rshift) -> KeyEventBuilder& {
        m_event_code = bit_modify(m_event_code, RSHIFT_OFFSET, rshift);
        return *this;
    }

    auto KeyEventBuilder::with_ralt(const bool ralt) -> KeyEventBuilder& {
        m_event_code = bit_modify(m_event_code, RALT_OFFSET, ralt);
        return *this;
    }

    auto KeyEventBuilder::with_rgui(const bool rgui) -> KeyEventBuilder& {
        m_event_code = bit_modify(m_event_code, RGUI_OFFSET, rgui);
        return *this;
    }

    auto KeyEventBuilder::with_key_down(const bool pressed) -> KeyEventBuilder& {
        m_event_code = bit_modify(m_event_code, KEY_DOWN_OFFSET, pressed);
        return *this;
    }

    auto KeyEventBuilder::build() const -> KeyEvent { return KeyEvent(m_event_code); }
} // namespace Ember
