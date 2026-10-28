
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
    X(GENERIC_DESKTOP, 0x01)                                                                       \
    X(KEYBOARD_KEYPAD, 0x07)                                                                       \
    X(LED, 0x08)

    /// @brief The usage page in the high 16 bits of a HIDExtendedUsage.
    EMBER_DEFINE_SMART_ENUM(HIDUsagePage, HID_USAGE_PAGES, 0x0000, U16) // NOLINT

    // ========================================================================================== //
    // Generic Desktop Page — HID Usage Tables 1.7 §4
    // ========================================================================================== //

#define HID_GENERIC_DESKTOP_PAGES(X)                                                               \
    X(KEYBOARD, 0x06)                                                                              \
    X(KEYPAD, 0x07)

    /// @brief Generic Desktop Pages
    EMBER_DEFINE_SMART_ENUM(HIDGenericDesktopPage, HID_GENERIC_DESKTOP_PAGES, 0x0000, U16) // NOLINT

    // ========================================================================================== //
    // Keyboard Page — HID Usage Tables 1.7 §10
    // ========================================================================================== //

#define HID_KEYBOARD_USAGES(_X)                                                                    \
    _X(ERROR_ROLL_OVER, 0x01)                                                                      \
    _X(POST_FAIL, 0x02)                                                                            \
    _X(ERROR_UNDEFINED, 0x03)                                                                      \
    _X(A, 0x04)                                                                                    \
    _X(B, 0x05)                                                                                    \
    _X(C, 0x06)                                                                                    \
    _X(D, 0x07)                                                                                    \
    _X(E, 0x08)                                                                                    \
    _X(F, 0x09)                                                                                    \
    _X(G, 0x0A)                                                                                    \
    _X(H, 0x0B)                                                                                    \
    _X(I, 0x0C)                                                                                    \
    _X(J, 0x0D)                                                                                    \
    _X(K, 0x0E)                                                                                    \
    _X(L, 0x0F)                                                                                    \
    _X(M, 0x10)                                                                                    \
    _X(N, 0x11)                                                                                    \
    _X(O, 0x12)                                                                                    \
    _X(P, 0x13)                                                                                    \
    _X(Q, 0x14)                                                                                    \
    _X(R, 0x15)                                                                                    \
    _X(S, 0x16)                                                                                    \
    _X(T, 0x17)                                                                                    \
    _X(U, 0x18)                                                                                    \
    _X(V, 0x19)                                                                                    \
    _X(W, 0x1A)                                                                                    \
    _X(X, 0x1B)                                                                                    \
    _X(Y, 0x1C)                                                                                    \
    _X(Z, 0x1D)                                                                                    \
    _X(ONE, 0x1E)                                                                                  \
    _X(TWO, 0x1F)                                                                                  \
    _X(THREE, 0x20)                                                                                \
    _X(FOUR, 0x21)                                                                                 \
    _X(FIVE, 0x22)                                                                                 \
    _X(SIX, 0x23)                                                                                  \
    _X(SEVEN, 0x24)                                                                                \
    _X(EIGHT, 0x25)                                                                                \
    _X(NINE, 0x26)                                                                                 \
    _X(ZERO, 0x27)                                                                                 \
    _X(RETURN_ENTER, 0x28)                                                                         \
    _X(ESCAPE, 0x29)                                                                               \
    _X(BACKSPACE, 0x2A)                                                                            \
    _X(TAB, 0x2B)                                                                                  \
    _X(SPACEBAR, 0x2C)                                                                             \
    _X(MINUS, 0x2D)                                                                                \
    _X(EQUAL, 0x2E)                                                                                \
    _X(LEFT_BRACKET, 0x2F)                                                                         \
    _X(RIGHT_BRACKET, 0x30)                                                                        \
    _X(BACKSLASH, 0x31)                                                                            \
    _X(NON_US_HASH, 0x32)                                                                          \
    _X(SEMICOLON, 0x33)                                                                            \
    _X(APOSTROPHE, 0x34)                                                                           \
    _X(GRAVE_ACCENT, 0x35)                                                                         \
    _X(COMMA, 0x36)                                                                                \
    _X(PERIOD, 0x37)                                                                               \
    _X(SLASH, 0x38)                                                                                \
    _X(CAPS_LOCK, 0x39)                                                                            \
    _X(F1, 0x3A)                                                                                   \
    _X(F2, 0x3B)                                                                                   \
    _X(F3, 0x3C)                                                                                   \
    _X(F4, 0x3D)                                                                                   \
    _X(F5, 0x3E)                                                                                   \
    _X(F6, 0x3F)                                                                                   \
    _X(F7, 0x40)                                                                                   \
    _X(F8, 0x41)                                                                                   \
    _X(F9, 0x42)                                                                                   \
    _X(F10, 0x43)                                                                                  \
    _X(F11, 0x44)                                                                                  \
    _X(F12, 0x45)                                                                                  \
    _X(PRINT_SCREEN, 0x46)                                                                         \
    _X(SCROLL_LOCK, 0x47)                                                                          \
    _X(PAUSE, 0x48)                                                                                \
    _X(INSERT, 0x49)                                                                               \
    _X(HOME, 0x4A)                                                                                 \
    _X(PAGE_UP, 0x4B)                                                                              \
    _X(DELETE_FORWARD, 0x4C)                                                                       \
    _X(END, 0x4D)                                                                                  \
    _X(PAGE_DOWN, 0x4E)                                                                            \
    _X(RIGHT_ARROW, 0x4F)                                                                          \
    _X(LEFT_ARROW, 0x50)                                                                           \
    _X(DOWN_ARROW, 0x51)                                                                           \
    _X(UP_ARROW, 0x52)                                                                             \
    _X(KEYPAD_NUM_LOCK, 0x53)                                                                      \
    _X(KEYPAD_SLASH, 0x54)                                                                         \
    _X(KEYPAD_ASTERISK, 0x55)                                                                      \
    _X(KEYPAD_MINUS, 0x56)                                                                         \
    _X(KEYPAD_PLUS, 0x57)                                                                          \
    _X(KEYPAD_ENTER, 0x58)                                                                         \
    _X(KEYPAD_ONE, 0x59)                                                                           \
    _X(KEYPAD_TWO, 0x5A)                                                                           \
    _X(KEYPAD_THREE, 0x5B)                                                                         \
    _X(KEYPAD_FOUR, 0x5C)                                                                          \
    _X(KEYPAD_FIVE, 0x5D)                                                                          \
    _X(KEYPAD_SIX, 0x5E)                                                                           \
    _X(KEYPAD_SEVEN, 0x5F)                                                                         \
    _X(KEYPAD_EIGHT, 0x60)                                                                         \
    _X(KEYPAD_NINE, 0x61)                                                                          \
    _X(KEYPAD_ZERO, 0x62)                                                                          \
    _X(KEYPAD_PERIOD, 0x63)                                                                        \
    _X(NON_US_BACKSLASH, 0x64)                                                                     \
    _X(APPLICATION, 0x65)                                                                          \
    _X(POWER, 0x66)                                                                                \
    _X(KEYPAD_EQUAL, 0x67)                                                                         \
    _X(F13, 0x68)                                                                                  \
    _X(F14, 0x69)                                                                                  \
    _X(F15, 0x6A)                                                                                  \
    _X(F16, 0x6B)                                                                                  \
    _X(F17, 0x6C)                                                                                  \
    _X(F18, 0x6D)                                                                                  \
    _X(F19, 0x6E)                                                                                  \
    _X(F20, 0x6F)                                                                                  \
    _X(F21, 0x70)                                                                                  \
    _X(F22, 0x71)                                                                                  \
    _X(F23, 0x72)                                                                                  \
    _X(F24, 0x73)                                                                                  \
    _X(EXECUTE, 0x74)                                                                              \
    _X(HELP, 0x75)                                                                                 \
    _X(MENU, 0x76)                                                                                 \
    _X(SELECT, 0x77)                                                                               \
    _X(STOP, 0x78)                                                                                 \
    _X(AGAIN, 0x79)                                                                                \
    _X(UNDO, 0x7A)                                                                                 \
    _X(CUT, 0x7B)                                                                                  \
    _X(COPY, 0x7C)                                                                                 \
    _X(PASTE, 0x7D)                                                                                \
    _X(FIND, 0x7E)                                                                                 \
    _X(MUTE, 0x7F)                                                                                 \
    _X(VOLUME_UP, 0x80)                                                                            \
    _X(VOLUME_DOWN, 0x81)                                                                          \
    _X(LOCKING_CAPS_LOCK, 0x82)                                                                    \
    _X(LOCKING_NUM_LOCK, 0x83)                                                                     \
    _X(LOCKING_SCROLL_LOCK, 0x84)                                                                  \
    _X(KEYPAD_COMMA, 0x85)                                                                         \
    _X(KEYPAD_EQUAL_SIGN, 0x86)                                                                    \
    _X(INTERNATIONAL_1, 0x87)                                                                      \
    _X(INTERNATIONAL_2, 0x88)                                                                      \
    _X(INTERNATIONAL_3, 0x89)                                                                      \
    _X(INTERNATIONAL_4, 0x8A)                                                                      \
    _X(INTERNATIONAL_5, 0x8B)                                                                      \
    _X(INTERNATIONAL_6, 0x8C)                                                                      \
    _X(INTERNATIONAL_7, 0x8D)                                                                      \
    _X(INTERNATIONAL_8, 0x8E)                                                                      \
    _X(INTERNATIONAL_9, 0x8F)                                                                      \
    _X(LANG_1, 0x90)                                                                               \
    _X(LANG_2, 0x91)                                                                               \
    _X(LANG_3, 0x92)                                                                               \
    _X(LANG_4, 0x93)                                                                               \
    _X(LANG_5, 0x94)                                                                               \
    _X(LANG_6, 0x95)                                                                               \
    _X(LANG_7, 0x96)                                                                               \
    _X(LANG_8, 0x97)                                                                               \
    _X(LANG_9, 0x98)                                                                               \
    _X(ALTERNATE_ERASE, 0x99)                                                                      \
    _X(SYS_REQ_ATTENTION, 0x9A)                                                                    \
    _X(CANCEL, 0x9B)                                                                               \
    _X(CLEAR, 0x9C)                                                                                \
    _X(PRIOR, 0x9D)                                                                                \
    _X(RETURN, 0x9E)                                                                               \
    _X(SEPARATOR, 0x9F)                                                                            \
    _X(OUT, 0xA0)                                                                                  \
    _X(OPER, 0xA1)                                                                                 \
    _X(CLEAR_AGAIN, 0xA2)                                                                          \
    _X(CR_SEL_PROPS, 0xA3)                                                                         \
    _X(EX_SEL, 0xA4)                                                                               \
    _X(KEYPAD_DOUBLE_ZERO, 0xB0)                                                                   \
    _X(KEYPAD_TRIPLE_ZERO, 0xB1)                                                                   \
    _X(THOUSANDS_SEPARATOR, 0xB2)                                                                  \
    _X(DECIMAL_SEPARATOR, 0xB3)                                                                    \
    _X(CURRENCY_UNIT, 0xB4)                                                                        \
    _X(CURRENCY_SUB_UNIT, 0xB5)                                                                    \
    _X(KEYPAD_LEFT_PARENTHESIS, 0xB6)                                                              \
    _X(KEYPAD_RIGHT_PARENTHESIS, 0xB7)                                                             \
    _X(KEYPAD_LEFT_BRACE, 0xB8)                                                                    \
    _X(KEYPAD_RIGHT_BRACE, 0xB9)                                                                   \
    _X(KEYPAD_TAB, 0xBA)                                                                           \
    _X(KEYPAD_BACKSPACE, 0xBB)                                                                     \
    _X(KEYPAD_A, 0xBC)                                                                             \
    _X(KEYPAD_B, 0xBD)                                                                             \
    _X(KEYPAD_C, 0xBE)                                                                             \
    _X(KEYPAD_D, 0xBF)                                                                             \
    _X(KEYPAD_E, 0xC0)                                                                             \
    _X(KEYPAD_F, 0xC1)                                                                             \
    _X(KEYPAD_XOR, 0xC2)                                                                           \
    _X(KEYPAD_CARET, 0xC3)                                                                         \
    _X(KEYPAD_PERCENT, 0xC4)                                                                       \
    _X(KEYPAD_LESS_THAN, 0xC5)                                                                     \
    _X(KEYPAD_GREATER_THAN, 0xC6)                                                                  \
    _X(KEYPAD_AMPERSAND, 0xC7)                                                                     \
    _X(KEYPAD_DOUBLE_AMPERSAND, 0xC8)                                                              \
    _X(KEYPAD_PIPE, 0xC9)                                                                          \
    _X(KEYPAD_DOUBLE_PIPE, 0xCA)                                                                   \
    _X(KEYPAD_COLON, 0xCB)                                                                         \
    _X(KEYPAD_HASH, 0xCC)                                                                          \
    _X(KEYPAD_SPACE, 0xCD)                                                                         \
    _X(KEYPAD_AT, 0xCE)                                                                            \
    _X(KEYPAD_EXCLAMATION, 0xCF)                                                                   \
    _X(KEYPAD_MEMORY_STORE, 0xD0)                                                                  \
    _X(KEYPAD_MEMORY_RECALL, 0xD1)                                                                 \
    _X(KEYPAD_MEMORY_CLEAR, 0xD2)                                                                  \
    _X(KEYPAD_MEMORY_ADD, 0xD3)                                                                    \
    _X(KEYPAD_MEMORY_SUBTRACT, 0xD4)                                                               \
    _X(KEYPAD_MEMORY_MULTIPLY, 0xD5)                                                               \
    _X(KEYPAD_MEMORY_DIVIDE, 0xD6)                                                                 \
    _X(KEYPAD_PLUS_MINUS, 0xD7)                                                                    \
    _X(KEYPAD_CLEAR, 0xD8)                                                                         \
    _X(KEYPAD_CLEAR_ENTRY, 0xD9)                                                                   \
    _X(KEYPAD_BINARY, 0xDA)                                                                        \
    _X(KEYPAD_OCTAL, 0xDB)                                                                         \
    _X(KEYPAD_DECIMAL, 0xDC)                                                                       \
    _X(KEYPAD_HEXADECIMAL, 0xDD)                                                                   \
    _X(LEFT_CONTROL, 0xE0)                                                                         \
    _X(LEFT_SHIFT, 0xE1)                                                                           \
    _X(LEFT_ALT, 0xE2)                                                                             \
    _X(LEFT_GUI, 0xE3)                                                                             \
    _X(RIGHT_CONTROL, 0xE4)                                                                        \
    _X(RIGHT_SHIFT, 0xE5)                                                                          \
    _X(RIGHT_ALT, 0xE6)                                                                            \
    _X(RIGHT_GUI, 0xE7)

    /// @brief Keyboard/Keypad Page (0x07): Every key code a USB keyboard with US layout can report
    ///         — HID Usage Tables 1.7 §10.
    ///
    /// ERROR_ROLL_OVER (0x01), POST_FAIL (0x02), ERROR_UNDEFINED (0x03): Not physical keys.
    /// Status codes the device puts in the key array, ERROR_ROLL_OVER when more keys are held
    /// down than the array can carry.
    EMBER_DEFINE_SMART_ENUM(HIDKeyboardUsage, HID_KEYBOARD_USAGES, 0x0000, U16) // NOLINT

    // ========================================================================================== //
    // LED Page — HID Usage Tables 1.7 §11
    // ========================================================================================== //

#define HID_LED_USAGES(X)                                                                          \
    X(NUM_LOCK, 0x01)                                                                              \
    X(CAPS_LOCK, 0x02)                                                                             \
    X(SCROLL_LOCK, 0x03)                                                                           \
    X(COMPOSE, 0x04)                                                                               \
    X(KANA, 0x05)                                                                                  \
    X(SHIFT, 0x07)

    /// @brief LED Page (0x08): the keyboard indicators of HID Usage Tables 1.7 §11.1.
    EMBER_DEFINE_SMART_ENUM(HIDLEDUsage, HID_LED_USAGES, 0x0000, U16) // NOLINT

    /// @brief Try to decode a usage based on its usage page.
    /// @param usage_page Usage page.
    /// @param usage Usage code.
    /// @return The string representation of the usage if it could be decoded, otherwise the
    ///         undecoded usage ID as hex.
    auto hid_decode_usage(HIDUsagePage usage_page, U16 usage) -> String;
} // namespace Rune::Device::USB

#endif // RUNEOS_USAGETABLES_H