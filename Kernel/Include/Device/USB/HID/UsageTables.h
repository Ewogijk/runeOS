
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

#ifndef RUNEOS_USAGETABLES_H
#define RUNEOS_USAGETABLES_H

#include <Ember/Enum.h>

#include <KRE/String.h>

namespace Rune::Device::USB {
    // ========================================================================================== //
    // Usage Pages — HID Usage Tables 1.7 §3
    // ========================================================================================== //

#define HID_USAGE_PAGES(X)                                                                         \
    X(HIDUsagePage, GENERIC_DESKTOP, 0x01)                                                         \
    X(HIDUsagePage, KEYBOARD_KEYPAD, 0x07)                                                         \
    X(HIDUsagePage, LED, 0x08)

    /// @brief The usage page in the high 16 bits of a HIDExtendedUsage.
    DECLARE_TYPED_ENUM(HIDUsagePage, U16, HID_USAGE_PAGES, 0x0000) // NOLINT

    // ========================================================================================== //
    // Generic Desktop Page — HID Usage Tables 1.7 §4
    // ========================================================================================== //

#define HID_GENERIC_DESKTOP_PAGES(X)                                                               \
    X(HIDGenericDesktopPage, KEYBOARD, 0x06)                                                       \
    X(HIDGenericDesktopPage, KEYPAD, 0x07)

    /// @brief Generic Desktop Pages
    DECLARE_TYPED_ENUM(HIDGenericDesktopPage, U16, HID_GENERIC_DESKTOP_PAGES, 0x0000) // NOLINT

    // ========================================================================================== //
    // Keyboard Page — HID Usage Tables 1.7 §10
    // ========================================================================================== //

#define HID_KEYBOARD_USAGES(_X)                                                                    \
    _X(HIDKeyboardUsage, ERROR_ROLL_OVER, 0x01)                                                    \
    _X(HIDKeyboardUsage, POST_FAIL, 0x02)                                                          \
    _X(HIDKeyboardUsage, ERROR_UNDEFINED, 0x03)                                                    \
    _X(HIDKeyboardUsage, A, 0x04)                                                                  \
    _X(HIDKeyboardUsage, B, 0x05)                                                                  \
    _X(HIDKeyboardUsage, C, 0x06)                                                                  \
    _X(HIDKeyboardUsage, D, 0x07)                                                                  \
    _X(HIDKeyboardUsage, E, 0x08)                                                                  \
    _X(HIDKeyboardUsage, F, 0x09)                                                                  \
    _X(HIDKeyboardUsage, G, 0x0A)                                                                  \
    _X(HIDKeyboardUsage, H, 0x0B)                                                                  \
    _X(HIDKeyboardUsage, I, 0x0C)                                                                  \
    _X(HIDKeyboardUsage, J, 0x0D)                                                                  \
    _X(HIDKeyboardUsage, K, 0x0E)                                                                  \
    _X(HIDKeyboardUsage, L, 0x0F)                                                                  \
    _X(HIDKeyboardUsage, M, 0x10)                                                                  \
    _X(HIDKeyboardUsage, N, 0x11)                                                                  \
    _X(HIDKeyboardUsage, O, 0x12)                                                                  \
    _X(HIDKeyboardUsage, P, 0x13)                                                                  \
    _X(HIDKeyboardUsage, Q, 0x14)                                                                  \
    _X(HIDKeyboardUsage, R, 0x15)                                                                  \
    _X(HIDKeyboardUsage, S, 0x16)                                                                  \
    _X(HIDKeyboardUsage, T, 0x17)                                                                  \
    _X(HIDKeyboardUsage, U, 0x18)                                                                  \
    _X(HIDKeyboardUsage, V, 0x19)                                                                  \
    _X(HIDKeyboardUsage, W, 0x1A)                                                                  \
    _X(HIDKeyboardUsage, X, 0x1B)                                                                  \
    _X(HIDKeyboardUsage, Y, 0x1C)                                                                  \
    _X(HIDKeyboardUsage, Z, 0x1D)                                                                  \
    _X(HIDKeyboardUsage, ONE, 0x1E)                                                                \
    _X(HIDKeyboardUsage, TWO, 0x1F)                                                                \
    _X(HIDKeyboardUsage, THREE, 0x20)                                                              \
    _X(HIDKeyboardUsage, FOUR, 0x21)                                                               \
    _X(HIDKeyboardUsage, FIVE, 0x22)                                                               \
    _X(HIDKeyboardUsage, SIX, 0x23)                                                                \
    _X(HIDKeyboardUsage, SEVEN, 0x24)                                                              \
    _X(HIDKeyboardUsage, EIGHT, 0x25)                                                              \
    _X(HIDKeyboardUsage, NINE, 0x26)                                                               \
    _X(HIDKeyboardUsage, ZERO, 0x27)                                                               \
    _X(HIDKeyboardUsage, RETURN_ENTER, 0x28)                                                       \
    _X(HIDKeyboardUsage, ESCAPE, 0x29)                                                             \
    _X(HIDKeyboardUsage, BACKSPACE, 0x2A)                                                          \
    _X(HIDKeyboardUsage, TAB, 0x2B)                                                                \
    _X(HIDKeyboardUsage, SPACEBAR, 0x2C)                                                           \
    _X(HIDKeyboardUsage, MINUS, 0x2D)                                                              \
    _X(HIDKeyboardUsage, EQUAL, 0x2E)                                                              \
    _X(HIDKeyboardUsage, LEFT_BRACKET, 0x2F)                                                       \
    _X(HIDKeyboardUsage, RIGHT_BRACKET, 0x30)                                                      \
    _X(HIDKeyboardUsage, BACKSLASH, 0x31)                                                          \
    _X(HIDKeyboardUsage, NON_US_HASH, 0x32)                                                        \
    _X(HIDKeyboardUsage, SEMICOLON, 0x33)                                                          \
    _X(HIDKeyboardUsage, APOSTROPHE, 0x34)                                                         \
    _X(HIDKeyboardUsage, GRAVE_ACCENT, 0x35)                                                       \
    _X(HIDKeyboardUsage, COMMA, 0x36)                                                              \
    _X(HIDKeyboardUsage, PERIOD, 0x37)                                                             \
    _X(HIDKeyboardUsage, SLASH, 0x38)                                                              \
    _X(HIDKeyboardUsage, CAPS_LOCK, 0x39)                                                          \
    _X(HIDKeyboardUsage, F1, 0x3A)                                                                 \
    _X(HIDKeyboardUsage, F2, 0x3B)                                                                 \
    _X(HIDKeyboardUsage, F3, 0x3C)                                                                 \
    _X(HIDKeyboardUsage, F4, 0x3D)                                                                 \
    _X(HIDKeyboardUsage, F5, 0x3E)                                                                 \
    _X(HIDKeyboardUsage, F6, 0x3F)                                                                 \
    _X(HIDKeyboardUsage, F7, 0x40)                                                                 \
    _X(HIDKeyboardUsage, F8, 0x41)                                                                 \
    _X(HIDKeyboardUsage, F9, 0x42)                                                                 \
    _X(HIDKeyboardUsage, F10, 0x43)                                                                \
    _X(HIDKeyboardUsage, F11, 0x44)                                                                \
    _X(HIDKeyboardUsage, F12, 0x45)                                                                \
    _X(HIDKeyboardUsage, PRINT_SCREEN, 0x46)                                                       \
    _X(HIDKeyboardUsage, SCROLL_LOCK, 0x47)                                                        \
    _X(HIDKeyboardUsage, PAUSE, 0x48)                                                              \
    _X(HIDKeyboardUsage, INSERT, 0x49)                                                             \
    _X(HIDKeyboardUsage, HOME, 0x4A)                                                               \
    _X(HIDKeyboardUsage, PAGE_UP, 0x4B)                                                            \
    _X(HIDKeyboardUsage, DELETE_FORWARD, 0x4C)                                                     \
    _X(HIDKeyboardUsage, END, 0x4D)                                                                \
    _X(HIDKeyboardUsage, PAGE_DOWN, 0x4E)                                                          \
    _X(HIDKeyboardUsage, RIGHT_ARROW, 0x4F)                                                        \
    _X(HIDKeyboardUsage, LEFT_ARROW, 0x50)                                                         \
    _X(HIDKeyboardUsage, DOWN_ARROW, 0x51)                                                         \
    _X(HIDKeyboardUsage, UP_ARROW, 0x52)                                                           \
    _X(HIDKeyboardUsage, KEYPAD_NUM_LOCK, 0x53)                                                    \
    _X(HIDKeyboardUsage, KEYPAD_SLASH, 0x54)                                                       \
    _X(HIDKeyboardUsage, KEYPAD_ASTERISK, 0x55)                                                    \
    _X(HIDKeyboardUsage, KEYPAD_MINUS, 0x56)                                                       \
    _X(HIDKeyboardUsage, KEYPAD_PLUS, 0x57)                                                        \
    _X(HIDKeyboardUsage, KEYPAD_ENTER, 0x58)                                                       \
    _X(HIDKeyboardUsage, KEYPAD_ONE, 0x59)                                                         \
    _X(HIDKeyboardUsage, KEYPAD_TWO, 0x5A)                                                         \
    _X(HIDKeyboardUsage, KEYPAD_THREE, 0x5B)                                                       \
    _X(HIDKeyboardUsage, KEYPAD_FOUR, 0x5C)                                                        \
    _X(HIDKeyboardUsage, KEYPAD_FIVE, 0x5D)                                                        \
    _X(HIDKeyboardUsage, KEYPAD_SIX, 0x5E)                                                         \
    _X(HIDKeyboardUsage, KEYPAD_SEVEN, 0x5F)                                                       \
    _X(HIDKeyboardUsage, KEYPAD_EIGHT, 0x60)                                                       \
    _X(HIDKeyboardUsage, KEYPAD_NINE, 0x61)                                                        \
    _X(HIDKeyboardUsage, KEYPAD_ZERO, 0x62)                                                        \
    _X(HIDKeyboardUsage, KEYPAD_PERIOD, 0x63)                                                      \
    _X(HIDKeyboardUsage, NON_US_BACKSLASH, 0x64)                                                   \
    _X(HIDKeyboardUsage, APPLICATION, 0x65)                                                        \
    _X(HIDKeyboardUsage, POWER, 0x66)                                                              \
    _X(HIDKeyboardUsage, KEYPAD_EQUAL, 0x67)                                                       \
    _X(HIDKeyboardUsage, F13, 0x68)                                                                \
    _X(HIDKeyboardUsage, F14, 0x69)                                                                \
    _X(HIDKeyboardUsage, F15, 0x6A)                                                                \
    _X(HIDKeyboardUsage, F16, 0x6B)                                                                \
    _X(HIDKeyboardUsage, F17, 0x6C)                                                                \
    _X(HIDKeyboardUsage, F18, 0x6D)                                                                \
    _X(HIDKeyboardUsage, F19, 0x6E)                                                                \
    _X(HIDKeyboardUsage, F20, 0x6F)                                                                \
    _X(HIDKeyboardUsage, F21, 0x70)                                                                \
    _X(HIDKeyboardUsage, F22, 0x71)                                                                \
    _X(HIDKeyboardUsage, F23, 0x72)                                                                \
    _X(HIDKeyboardUsage, F24, 0x73)                                                                \
    _X(HIDKeyboardUsage, EXECUTE, 0x74)                                                            \
    _X(HIDKeyboardUsage, HELP, 0x75)                                                               \
    _X(HIDKeyboardUsage, MENU, 0x76)                                                               \
    _X(HIDKeyboardUsage, SELECT, 0x77)                                                             \
    _X(HIDKeyboardUsage, STOP, 0x78)                                                               \
    _X(HIDKeyboardUsage, AGAIN, 0x79)                                                              \
    _X(HIDKeyboardUsage, UNDO, 0x7A)                                                               \
    _X(HIDKeyboardUsage, CUT, 0x7B)                                                                \
    _X(HIDKeyboardUsage, COPY, 0x7C)                                                               \
    _X(HIDKeyboardUsage, PASTE, 0x7D)                                                              \
    _X(HIDKeyboardUsage, FIND, 0x7E)                                                               \
    _X(HIDKeyboardUsage, MUTE, 0x7F)                                                               \
    _X(HIDKeyboardUsage, VOLUME_UP, 0x80)                                                          \
    _X(HIDKeyboardUsage, VOLUME_DOWN, 0x81)                                                        \
    _X(HIDKeyboardUsage, LOCKING_CAPS_LOCK, 0x82)                                                  \
    _X(HIDKeyboardUsage, LOCKING_NUM_LOCK, 0x83)                                                   \
    _X(HIDKeyboardUsage, LOCKING_SCROLL_LOCK, 0x84)                                                \
    _X(HIDKeyboardUsage, KEYPAD_COMMA, 0x85)                                                       \
    _X(HIDKeyboardUsage, KEYPAD_EQUAL_SIGN, 0x86)                                                  \
    _X(HIDKeyboardUsage, INTERNATIONAL_1, 0x87)                                                    \
    _X(HIDKeyboardUsage, INTERNATIONAL_2, 0x88)                                                    \
    _X(HIDKeyboardUsage, INTERNATIONAL_3, 0x89)                                                    \
    _X(HIDKeyboardUsage, INTERNATIONAL_4, 0x8A)                                                    \
    _X(HIDKeyboardUsage, INTERNATIONAL_5, 0x8B)                                                    \
    _X(HIDKeyboardUsage, INTERNATIONAL_6, 0x8C)                                                    \
    _X(HIDKeyboardUsage, INTERNATIONAL_7, 0x8D)                                                    \
    _X(HIDKeyboardUsage, INTERNATIONAL_8, 0x8E)                                                    \
    _X(HIDKeyboardUsage, INTERNATIONAL_9, 0x8F)                                                    \
    _X(HIDKeyboardUsage, LANG_1, 0x90)                                                             \
    _X(HIDKeyboardUsage, LANG_2, 0x91)                                                             \
    _X(HIDKeyboardUsage, LANG_3, 0x92)                                                             \
    _X(HIDKeyboardUsage, LANG_4, 0x93)                                                             \
    _X(HIDKeyboardUsage, LANG_5, 0x94)                                                             \
    _X(HIDKeyboardUsage, LANG_6, 0x95)                                                             \
    _X(HIDKeyboardUsage, LANG_7, 0x96)                                                             \
    _X(HIDKeyboardUsage, LANG_8, 0x97)                                                             \
    _X(HIDKeyboardUsage, LANG_9, 0x98)                                                             \
    _X(HIDKeyboardUsage, ALTERNATE_ERASE, 0x99)                                                    \
    _X(HIDKeyboardUsage, SYS_REQ_ATTENTION, 0x9A)                                                  \
    _X(HIDKeyboardUsage, CANCEL, 0x9B)                                                             \
    _X(HIDKeyboardUsage, CLEAR, 0x9C)                                                              \
    _X(HIDKeyboardUsage, PRIOR, 0x9D)                                                              \
    _X(HIDKeyboardUsage, RETURN, 0x9E)                                                             \
    _X(HIDKeyboardUsage, SEPARATOR, 0x9F)                                                          \
    _X(HIDKeyboardUsage, OUT, 0xA0)                                                                \
    _X(HIDKeyboardUsage, OPER, 0xA1)                                                               \
    _X(HIDKeyboardUsage, CLEAR_AGAIN, 0xA2)                                                        \
    _X(HIDKeyboardUsage, CR_SEL_PROPS, 0xA3)                                                       \
    _X(HIDKeyboardUsage, EX_SEL, 0xA4)                                                             \
    _X(HIDKeyboardUsage, KEYPAD_DOUBLE_ZERO, 0xB0)                                                 \
    _X(HIDKeyboardUsage, KEYPAD_TRIPLE_ZERO, 0xB1)                                                 \
    _X(HIDKeyboardUsage, THOUSANDS_SEPARATOR, 0xB2)                                                \
    _X(HIDKeyboardUsage, DECIMAL_SEPARATOR, 0xB3)                                                  \
    _X(HIDKeyboardUsage, CURRENCY_UNIT, 0xB4)                                                      \
    _X(HIDKeyboardUsage, CURRENCY_SUB_UNIT, 0xB5)                                                  \
    _X(HIDKeyboardUsage, KEYPAD_LEFT_PARENTHESIS, 0xB6)                                            \
    _X(HIDKeyboardUsage, KEYPAD_RIGHT_PARENTHESIS, 0xB7)                                           \
    _X(HIDKeyboardUsage, KEYPAD_LEFT_BRACE, 0xB8)                                                  \
    _X(HIDKeyboardUsage, KEYPAD_RIGHT_BRACE, 0xB9)                                                 \
    _X(HIDKeyboardUsage, KEYPAD_TAB, 0xBA)                                                         \
    _X(HIDKeyboardUsage, KEYPAD_BACKSPACE, 0xBB)                                                   \
    _X(HIDKeyboardUsage, KEYPAD_A, 0xBC)                                                           \
    _X(HIDKeyboardUsage, KEYPAD_B, 0xBD)                                                           \
    _X(HIDKeyboardUsage, KEYPAD_C, 0xBE)                                                           \
    _X(HIDKeyboardUsage, KEYPAD_D, 0xBF)                                                           \
    _X(HIDKeyboardUsage, KEYPAD_E, 0xC0)                                                           \
    _X(HIDKeyboardUsage, KEYPAD_F, 0xC1)                                                           \
    _X(HIDKeyboardUsage, KEYPAD_XOR, 0xC2)                                                         \
    _X(HIDKeyboardUsage, KEYPAD_CARET, 0xC3)                                                       \
    _X(HIDKeyboardUsage, KEYPAD_PERCENT, 0xC4)                                                     \
    _X(HIDKeyboardUsage, KEYPAD_LESS_THAN, 0xC5)                                                   \
    _X(HIDKeyboardUsage, KEYPAD_GREATER_THAN, 0xC6)                                                \
    _X(HIDKeyboardUsage, KEYPAD_AMPERSAND, 0xC7)                                                   \
    _X(HIDKeyboardUsage, KEYPAD_DOUBLE_AMPERSAND, 0xC8)                                            \
    _X(HIDKeyboardUsage, KEYPAD_PIPE, 0xC9)                                                        \
    _X(HIDKeyboardUsage, KEYPAD_DOUBLE_PIPE, 0xCA)                                                 \
    _X(HIDKeyboardUsage, KEYPAD_COLON, 0xCB)                                                       \
    _X(HIDKeyboardUsage, KEYPAD_HASH, 0xCC)                                                        \
    _X(HIDKeyboardUsage, KEYPAD_SPACE, 0xCD)                                                       \
    _X(HIDKeyboardUsage, KEYPAD_AT, 0xCE)                                                          \
    _X(HIDKeyboardUsage, KEYPAD_EXCLAMATION, 0xCF)                                                 \
    _X(HIDKeyboardUsage, KEYPAD_MEMORY_STORE, 0xD0)                                                \
    _X(HIDKeyboardUsage, KEYPAD_MEMORY_RECALL, 0xD1)                                               \
    _X(HIDKeyboardUsage, KEYPAD_MEMORY_CLEAR, 0xD2)                                                \
    _X(HIDKeyboardUsage, KEYPAD_MEMORY_ADD, 0xD3)                                                  \
    _X(HIDKeyboardUsage, KEYPAD_MEMORY_SUBTRACT, 0xD4)                                             \
    _X(HIDKeyboardUsage, KEYPAD_MEMORY_MULTIPLY, 0xD5)                                             \
    _X(HIDKeyboardUsage, KEYPAD_MEMORY_DIVIDE, 0xD6)                                               \
    _X(HIDKeyboardUsage, KEYPAD_PLUS_MINUS, 0xD7)                                                  \
    _X(HIDKeyboardUsage, KEYPAD_CLEAR, 0xD8)                                                       \
    _X(HIDKeyboardUsage, KEYPAD_CLEAR_ENTRY, 0xD9)                                                 \
    _X(HIDKeyboardUsage, KEYPAD_BINARY, 0xDA)                                                      \
    _X(HIDKeyboardUsage, KEYPAD_OCTAL, 0xDB)                                                       \
    _X(HIDKeyboardUsage, KEYPAD_DECIMAL, 0xDC)                                                     \
    _X(HIDKeyboardUsage, KEYPAD_HEXADECIMAL, 0xDD)                                                 \
    _X(HIDKeyboardUsage, LEFT_CONTROL, 0xE0)                                                       \
    _X(HIDKeyboardUsage, LEFT_SHIFT, 0xE1)                                                         \
    _X(HIDKeyboardUsage, LEFT_ALT, 0xE2)                                                           \
    _X(HIDKeyboardUsage, LEFT_GUI, 0xE3)                                                           \
    _X(HIDKeyboardUsage, RIGHT_CONTROL, 0xE4)                                                      \
    _X(HIDKeyboardUsage, RIGHT_SHIFT, 0xE5)                                                        \
    _X(HIDKeyboardUsage, RIGHT_ALT, 0xE6)                                                          \
    _X(HIDKeyboardUsage, RIGHT_GUI, 0xE7)

    /// @brief Keyboard/Keypad Page (0x07): Every key code a USB keyboard with US layout can report
    ///         — HID Usage Tables 1.7 §10.
    ///
    /// ERROR_ROLL_OVER (0x01), POST_FAIL (0x02), ERROR_UNDEFINED (0x03): Not physical keys.
    /// Status codes the device puts in the key array, ERROR_ROLL_OVER when more keys are held
    /// down than the array can carry.
    DECLARE_TYPED_ENUM(HIDKeyboardUsage, U16, HID_KEYBOARD_USAGES, 0x0000) // NOLINT

    // ========================================================================================== //
    // LED Page — HID Usage Tables 1.7 §11
    // ========================================================================================== //

#define HID_LED_USAGES(X)                                                                          \
    X(HIDLEDUsage, NUM_LOCK, 0x01)                                                                 \
    X(HIDLEDUsage, CAPS_LOCK, 0x02)                                                                \
    X(HIDLEDUsage, SCROLL_LOCK, 0x03)                                                              \
    X(HIDLEDUsage, COMPOSE, 0x04)                                                                  \
    X(HIDLEDUsage, KANA, 0x05)                                                                     \
    X(HIDLEDUsage, SHIFT, 0x07)

    /// @brief LED Page (0x08): the keyboard indicators of HID Usage Tables 1.7 §11.1.
    DECLARE_TYPED_ENUM(HIDLEDUsage, U16, HID_LED_USAGES, 0x0000) // NOLINT

    /// @brief Try to decode a usage based on its usage page.
    /// @param usage_page Usage page.
    /// @param usage Usage code.
    /// @return The string representation of the usage if it could be decoded, otherwise the
    ///         undecoded usage ID as hex.
    auto hid_decode_usage(HIDUsagePage usage_page, U16 usage) -> String;
} // namespace Rune::Device::USB

#endif // RUNEOS_USAGETABLES_H