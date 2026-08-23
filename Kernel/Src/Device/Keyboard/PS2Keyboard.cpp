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

#include <Device/Keyboard/PS2Keyboard.h>

#include <Ember/AppBits.h>

#include <CPU/IO.h>

#include <KRE/BitsAndBytes.h>

#include <Device/KeyEventBuffer.h>

namespace Rune::Device {
#define PORTS(X)                                                                                   \
    X(Port, DATA, 0x60)                                                                            \
    X(Port, COMMAND_WRITE, 0x64)                                                                   \
    X(Port, STATUS_READ, 0x64)

    DECLARE_TYPED_ENUM(Port, U8, PORTS, 0x0) // NOLINT
    DEFINE_TYPED_ENUM(Port, U8, PORTS, 0x0)

#define COMMANDS(X) X(Command, GET_SET_SCANCODE_SET, 0xF0)

    DECLARE_TYPED_ENUM(Command, U8, COMMANDS, 0x0) // NOLINT
    DEFINE_TYPED_ENUM(Command, U8, COMMANDS, 0x0)

#define RESPONSES(X)                                                                               \
    X(Response, KEY_ERROR_OR_BUF_OVERRUN, 0x00)                                                    \
    X(Response, SELF_TEST_PASSED, 0xAA)                                                            \
    X(Response, ECHO, 0xEE)                                                                        \
    X(Response, ACK, 0xFA)                                                                         \
    X(Response, SELF_TEST_FAILED, 0xFC)                                                            \
    X(Response, SELF_TEST_FAILED2, 0xFD)                                                           \
    X(Response, RESEND, 0xFE)                                                                      \
    X(Response, KEY_ERROR_OR_BUF_OVERRUN2, 0xFF)

    DECLARE_TYPED_ENUM(Response, U8, RESPONSES, 0x01) // NOLINT
    DEFINE_TYPED_ENUM(Response, U8, RESPONSES, 0x01)  // NOLINT

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          PS2 Scan Code Set 1
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /// @brief Scan code set 1 defines make codes in the 0x00-0x7F range, the break code of a key is
    ///         it's make code with the break code flag set.
    constexpr U8 MAKE_CODE_COUNT = 0x80;
    constexpr U8 BREAK_CODE_FLAG = 0x80;

    using Key = Ember::VirtualKey;

    /// @brief Maps a scan code set 1 make code to it's virtual key.
    ///
    /// Make codes that are not listed are implicitly Key::NONE, that is the key is either unknown
    /// or a modifier key, which is decoded by "decode_modifier_bit" instead.
    // clang-format off
    constexpr Key::_E SCAN_CODE_DECODER[MAKE_CODE_COUNT] = {
        /* 0x00 */ Key::NONE,          Key::ESCAPE,        Key::ONE,           Key::TWO,
        /* 0x04 */ Key::THREE,         Key::FOUR,          Key::FIVE,          Key::SIX,
        /* 0x08 */ Key::SEVEN,         Key::EIGHT,         Key::NINE,          Key::ZERO,
        /* 0x0C */ Key::MINUS,         Key::EQUAL,         Key::BACKSPACE,     Key::TAB,
        /* 0x10 */ Key::Q,             Key::W,             Key::E,             Key::R,
        /* 0x14 */ Key::T,             Key::Y,             Key::U,             Key::I,
        /* 0x18 */ Key::O,             Key::P,             Key::LEFT_BRACKET,  Key::RIGHT_BRACKET,
        /* 0x1C */ Key::ENTER,         Key::NONE,          Key::A,             Key::S,
        /* 0x20 */ Key::D,             Key::F,             Key::G,             Key::H,
        /* 0x24 */ Key::J,             Key::K,             Key::L,             Key::SEMICOLON,
        /* 0x28 */ Key::APOSTROPHE,    Key::GRAVE,         Key::NONE,          Key::BACKSLASH,
        /* 0x2C */ Key::Z,             Key::X,             Key::C,             Key::V,
        /* 0x30 */ Key::B,             Key::N,             Key::M,             Key::COMMA,
        /* 0x34 */ Key::PERIOD,        Key::SLASH,         Key::NONE,          Key::KP_MULTIPLY,
        /* 0x38 */ Key::NONE,          Key::SPACE,         Key::CAPS_LOCK,     Key::F1,
        /* 0x3C */ Key::F2,            Key::F3,            Key::F4,            Key::F5,
        /* 0x40 */ Key::F6,            Key::F7,            Key::F8,            Key::F9,
        /* 0x44 */ Key::F10,           Key::NUM_LOCK,      Key::SCROLL_LOCK,   Key::KP_SEVEN,
        /* 0x48 */ Key::KP_EIGHT,      Key::KP_NINE,       Key::KP_MINUS,      Key::KP_FOUR,
        /* 0x4C */ Key::KP_FIVE,       Key::KP_SIX,        Key::KP_PLUS,       Key::KP_ONE,
        /* 0x50 */ Key::KP_TWO,        Key::KP_THREE,      Key::KP_ZERO,       Key::KP_PERIOD,
        /* 0x54 */ Key::NONE,          Key::NONE,          Key::NON_US_BACKSLASH, Key::F11,
        /* 0x58 */ Key::F12,
    };

    /// @brief Maps a scan code set 1 make code that is prefixed with the extended byte to it's
    ///         virtual key.
    constexpr Key::_E E_0_SCAN_CODE_DECODER[MAKE_CODE_COUNT] = {
        /* 0x00 */ Key::NONE,          Key::NONE,          Key::NONE,          Key::NONE,
        /* 0x04 */ Key::NONE,          Key::NONE,          Key::NONE,          Key::NONE,
        /* 0x08 */ Key::NONE,          Key::NONE,          Key::NONE,          Key::NONE,
        /* 0x0C */ Key::NONE,          Key::NONE,          Key::NONE,          Key::NONE,
        /* 0x10 */ Key::NONE,          Key::NONE,          Key::NONE,          Key::NONE,
        /* 0x14 */ Key::NONE,          Key::NONE,          Key::NONE,          Key::NONE,
        /* 0x18 */ Key::NONE,          Key::NONE,          Key::NONE,          Key::NONE,
        /* 0x1C */ Key::KP_ENTER,      Key::NONE,          Key::NONE,          Key::NONE,
        /* 0x20 */ Key::NONE,          Key::NONE,          Key::NONE,          Key::NONE,
        /* 0x24 */ Key::NONE,          Key::NONE,          Key::NONE,          Key::NONE,
        /* 0x28 */ Key::NONE,          Key::NONE,          Key::NONE,          Key::NONE,
        /* 0x2C */ Key::NONE,          Key::NONE,          Key::NONE,          Key::NONE,
        /* 0x30 */ Key::NONE,          Key::NONE,          Key::NONE,          Key::NONE,
        /* 0x34 */ Key::NONE,          Key::KP_DIVIDE,     Key::NONE,          Key::PRINT_SCREEN,
        /* 0x38 */ Key::NONE,          Key::NONE,          Key::NONE,          Key::NONE,
        /* 0x3C */ Key::NONE,          Key::NONE,          Key::NONE,          Key::NONE,
        /* 0x40 */ Key::NONE,          Key::NONE,          Key::NONE,          Key::NONE,
        /* 0x44 */ Key::NONE,          Key::NONE,          Key::NONE,          Key::HOME,
        /* 0x48 */ Key::ARROW_UP,      Key::PAGE_UP,       Key::NONE,          Key::ARROW_LEFT,
        /* 0x4C */ Key::NONE,          Key::ARROW_RIGHT,   Key::NONE,          Key::END,
        /* 0x50 */ Key::ARROW_DOWN,    Key::PAGE_DOWN,     Key::INSERT,        Key::DELETE,
        /* 0x54 */ Key::NONE,          Key::NONE,          Key::NONE,          Key::NONE,
        /* 0x58 */ Key::NONE,          Key::NONE,          Key::NONE,          Key::NONE,
        /* 0x5C */ Key::NONE,          Key::APPLICATION,
    };
    // clang-format on

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Key Events
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    // Bit positions of the modifier keys, matches the HID keyboard modifier byte layout
    constexpr U8 LCTRL_BIT  = 0;
    constexpr U8 LSHIFT_BIT = 1;
    constexpr U8 LALT_BIT   = 2;
    constexpr U8 LGUI_BIT   = 3;
    constexpr U8 RCTRL_BIT  = 4;
    constexpr U8 RSHIFT_BIT = 5;
    constexpr U8 RALT_BIT   = 6;
    constexpr U8 RGUI_BIT   = 7;

    /// @brief Returned by "decode_modifier_bit" when a make code is not a modifier key.
    constexpr U8 NO_MODIFIER_BIT = 0xFF;

    // Make codes of the modifier keys, the extended ones are prefixed with the extended byte
    constexpr U8 MAKE_CODE_CTRL     = 0x1D;
    constexpr U8 MAKE_CODE_LSHIFT   = 0x2A;
    constexpr U8 MAKE_CODE_RSHIFT   = 0x36;
    constexpr U8 MAKE_CODE_ALT      = 0x38;
    constexpr U8 MAKE_CODE_E_0_LGUI = 0x5B;
    constexpr U8 MAKE_CODE_E_0_RGUI = 0x5C;

    /// @brief Decode the modifier key that is identified by a make code.
    /// @param make_code A scan code set 1 make code.
    /// @param extended True: The make code was prefixed with the extended byte.
    /// @return The bit position of the modifier key in the modifier bitmap or NO_MODIFIER_BIT if
    ///         the make code does not belong to a modifier key.
    auto decode_modifier_bit(U8 make_code, bool extended) -> U8 {
        if (extended) {
            switch (make_code) {
                case MAKE_CODE_CTRL:     return RCTRL_BIT;
                case MAKE_CODE_ALT:      return RALT_BIT;
                case MAKE_CODE_E_0_LGUI: return LGUI_BIT;
                case MAKE_CODE_E_0_RGUI: return RGUI_BIT;
                default:                 return NO_MODIFIER_BIT;
            }
        }
        switch (make_code) {
            case MAKE_CODE_CTRL:   return LCTRL_BIT;
            case MAKE_CODE_LSHIFT: return LSHIFT_BIT;
            case MAKE_CODE_RSHIFT: return RSHIFT_BIT;
            case MAKE_CODE_ALT:    return LALT_BIT;
            default:               return NO_MODIFIER_BIT;
        }
    }

    /// @brief Build the key event for a pressed/released key.
    /// @param vk        Virtual key of the pressed/released key.
    /// @param modifiers Modifier bitmap of the currently pressed modifier keys.
    /// @param key_down  True: The key was pressed, False: The key was released.
    /// @return The key event.
    auto build_ps2_key_event(Ember::VirtualKey vk, U8 modifiers, bool key_down) -> Ember::KeyEvent {
        return Ember::KeyEventBuilder()
            .with_virtual_key(vk)
            .with_lctrl(bit_check(modifiers, LCTRL_BIT))
            .with_lshift(bit_check(modifiers, LSHIFT_BIT))
            .with_lalt(bit_check(modifiers, LALT_BIT))
            .with_lgui(bit_check(modifiers, LGUI_BIT))
            .with_rctrl(bit_check(modifiers, RCTRL_BIT))
            .with_rshift(bit_check(modifiers, RSHIFT_BIT))
            .with_ralt(bit_check(modifiers, RALT_BIT))
            .with_rgui(bit_check(modifiers, RGUI_BIT))
            .with_key_down(key_down)
            .build();
    }

    const BasicDeviceID PS2Keyboard::ID_PS2_KEYBOARD("PS2 Keyboard");

    PS2Keyboard::PS2Keyboard()
        : _irq_handler([](CPU::InterruptFrame* i_frame) -> Rune::CPU::InterruptState::_E {
              SILENCE_UNUSED(i_frame);
              return CPU::InterruptState::PENDING;
          }) {}

    void PS2Keyboard::handle_scan_code(U8 scan_code) {
        if (_pause_bytes_left > 0) {
            // The pause key sends "E1 1D 45" when it is pressed and "E1 9D C5" when it is
            // released, only the break code flag of the first byte after the pause byte tells
            // both sequences apart.
            if (_pause_bytes_left == PAUSE_SEQUENCE_LENGTH)
                _pause_key_down = (scan_code & BREAK_CODE_FLAG) == 0;
            _pause_bytes_left--;
            if (_pause_bytes_left == 0)
                g_key_event_buffer.append(
                    build_ps2_key_event(Ember::VirtualKey::PAUSE, _modifiers, _pause_key_down));
            return;
        }

        if (scan_code == PAUSE_BYTE) {
            _pause_bytes_left = PAUSE_SEQUENCE_LENGTH;
            return;
        }

        if (scan_code == EXTENDED_BYTE) {
            _wait_key_e0 = true;
            return;
        }

        // Clear the flag for unknown extended scan codes too, e.g. the fake shifts that are sent
        // with the print screen key, else the next key would be decoded with the E0 table.
        const bool extended = _wait_key_e0;
        _wait_key_e0        = false;

        const bool key_down  = (scan_code & BREAK_CODE_FLAG) == 0;
        const U8   make_code = scan_code & ~BREAK_CODE_FLAG;

        const U8 modifier_bit = decode_modifier_bit(make_code, extended);
        if (modifier_bit != NO_MODIFIER_BIT) {
            // Modifier keys have no virtual key, they are only reported as part of the key events
            // of the other keys.
            _modifiers =
                key_down ? bit_set(_modifiers, modifier_bit) : bit_clear(_modifiers, modifier_bit);
            return;
        }

        Ember::VirtualKey key =
            extended ? E_0_SCAN_CODE_DECODER[make_code] : SCAN_CODE_DECODER[make_code];
        if (key == Ember::VirtualKey::NONE) return;

        g_key_event_buffer.append(build_ps2_key_event(key, _modifiers, key_down));
    }

    auto PS2Keyboard::vendor() const -> String { return "Ewogjik"; };

    auto PS2Keyboard::version() const -> Version { return {.major = 1, .minor = 0, .patch = 0}; }

    auto PS2Keyboard::can_bind(const DeviceID* device_ID) -> bool {
        return ID_PS2_KEYBOARD.equals(device_ID);
    }

    auto PS2Keyboard::bind(const SharedPointer<Device>& device) -> bool {
        SILENCE_UNUSED(device)
        _irq_handler = [this](CPU::InterruptFrame* i_frame) -> Rune::CPU::InterruptState::_E {
            SILENCE_UNUSED(i_frame);
            handle_scan_code(CPU::in_b(DATA_REGISTER));
            return CPU::InterruptState::HANDLED;
        };
        return CPU::irq_install_handler(1, 0, "PS2 Keyboard", _irq_handler);
    }

    void PS2Keyboard::unbind(const SharedPointer<Device>& device) {
        SILENCE_UNUSED(device)
        CPU::irq_uninstall_handler(1, 1);
    }

    auto PS2Keyboard::handle_request(const SharedPointer<Device>& device, IORequest request)
        -> CPU::Future<IORequestStatus> {
        SILENCE_UNUSED(device)
        SILENCE_UNUSED(request)
        CPU::Promise<IORequestStatus> p;
        p.set_value(IORequestStatus::UNSUPPORTED);
        return p.get_future();
    }

} // namespace Rune::Device