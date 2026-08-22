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

#ifndef EMBER_APP_H
#define EMBER_APP_H

#include <Ember/Ember.h>

namespace Ember {

#define STD_IO_TARGETS(X)                                                                          \
    X(StdIOTarget, VOID, 0x1)                                                                      \
    X(StdIOTarget, INHERIT, 0x2)                                                                   \
    X(StdIOTarget, FILE, 0x3)                                                                      \
    X(StdIOTarget, PIPE, 0x4)

    /// @brief Describes to which target a std IO stream of an application can be connected.
    ///
    /// VOID: Connect the standard stream to the void stream, which provides no output and ignores
    ///         all input.<br>
    /// INHERIT: Connect stdin/stdout/stderr of the application to the std streams of the parent
    ///             app.<br>
    /// FILE: Only stdout/stderr: Redirect to the specified file, if it does not exist it will be
    ///         created.<br>
    /// PIPE: Connect the standard stream to the requested pipe.<br>
    DECLARE_ENUM(StdIOTarget, STD_IO_TARGETS, 0x0) // NOLINT

    /// @brief The configuration of a std IO stream.
    ///
    /// Each std IO target can have an optional argument:
    /// VOID: -<br>
    /// INHERIT: -<br>
    /// FILE: A relative or absolute path to a file.<br>
    /// PIPE: -<br>
    struct StdIOConfig {
        StdIOTarget target;
        char        argument[STRING_SIZE_LIMIT] = {}; // NOLINT
    };

    // ========================================================================================== //
    // VirtualKey
    // ========================================================================================== //

#define VIRTUAL_KEYS(VK)                                                                           \
    VK(VirtualKey, A, 0x04)                                                                        \
    VK(VirtualKey, B, 0x05)                                                                        \
    VK(VirtualKey, C, 0x06)                                                                        \
    VK(VirtualKey, D, 0x07)                                                                        \
    VK(VirtualKey, E, 0x08)                                                                        \
    VK(VirtualKey, F, 0x09)                                                                        \
    VK(VirtualKey, G, 0x0A)                                                                        \
    VK(VirtualKey, H, 0x0B)                                                                        \
    VK(VirtualKey, I, 0x0C)                                                                        \
    VK(VirtualKey, J, 0x0D)                                                                        \
    VK(VirtualKey, K, 0x0E)                                                                        \
    VK(VirtualKey, L, 0x0F)                                                                        \
    VK(VirtualKey, M, 0x10)                                                                        \
    VK(VirtualKey, N, 0x11)                                                                        \
    VK(VirtualKey, O, 0x12)                                                                        \
    VK(VirtualKey, P, 0x13)                                                                        \
    VK(VirtualKey, Q, 0x14)                                                                        \
    VK(VirtualKey, R, 0x15)                                                                        \
    VK(VirtualKey, S, 0x16)                                                                        \
    VK(VirtualKey, T, 0x17)                                                                        \
    VK(VirtualKey, U, 0x18)                                                                        \
    VK(VirtualKey, V, 0x19)                                                                        \
    VK(VirtualKey, W, 0x1A)                                                                        \
    VK(VirtualKey, X, 0x1B)                                                                        \
    VK(VirtualKey, Y, 0x1C)                                                                        \
    VK(VirtualKey, Z, 0x1D)                                                                        \
    VK(VirtualKey, ONE, 0x1E)                                                                      \
    VK(VirtualKey, TWO, 0x1F)                                                                      \
    VK(VirtualKey, THREE, 0x20)                                                                    \
    VK(VirtualKey, FOUR, 0x21)                                                                     \
    VK(VirtualKey, FIVE, 0x22)                                                                     \
    VK(VirtualKey, SIX, 0x23)                                                                      \
    VK(VirtualKey, SEVEN, 0x24)                                                                    \
    VK(VirtualKey, EIGHT, 0x25)                                                                    \
    VK(VirtualKey, NINE, 0x26)                                                                     \
    VK(VirtualKey, ZERO, 0x27)                                                                     \
    VK(VirtualKey, ENTER, 0x28)                                                                    \
    VK(VirtualKey, ESCAPE, 0x29)                                                                   \
    VK(VirtualKey, BACKSPACE, 0x2A)                                                                \
    VK(VirtualKey, TAB, 0x2B)                                                                      \
    VK(VirtualKey, SPACE, 0x2C)                                                                    \
    VK(VirtualKey, MINUS, 0x2D)                                                                    \
    VK(VirtualKey, EQUAL, 0x2E)                                                                    \
    VK(VirtualKey, LEFT_BRACKET, 0x2F)                                                             \
    VK(VirtualKey, RIGHT_BRACKET, 0x30)                                                            \
    VK(VirtualKey, BACKSLASH, 0x31)                                                                \
    VK(VirtualKey, NON_US_HASH, 0x32)                                                              \
    VK(VirtualKey, SEMICOLON, 0x33)                                                                \
    VK(VirtualKey, APOSTROPHE, 0x34)                                                               \
    VK(VirtualKey, GRAVE, 0x35)                                                                    \
    VK(VirtualKey, COMMA, 0x36)                                                                    \
    VK(VirtualKey, PERIOD, 0x37)                                                                   \
    VK(VirtualKey, SLASH, 0x38)                                                                    \
    VK(VirtualKey, CAPS_LOCK, 0x39)                                                                \
    VK(VirtualKey, F1, 0x3A)                                                                       \
    VK(VirtualKey, F2, 0x3B)                                                                       \
    VK(VirtualKey, F3, 0x3C)                                                                       \
    VK(VirtualKey, F4, 0x3D)                                                                       \
    VK(VirtualKey, F5, 0x3E)                                                                       \
    VK(VirtualKey, F6, 0x3F)                                                                       \
    VK(VirtualKey, F7, 0x40)                                                                       \
    VK(VirtualKey, F8, 0x41)                                                                       \
    VK(VirtualKey, F9, 0x42)                                                                       \
    VK(VirtualKey, F10, 0x43)                                                                      \
    VK(VirtualKey, F11, 0x44)                                                                      \
    VK(VirtualKey, F12, 0x45)                                                                      \
    VK(VirtualKey, PRINT_SCREEN, 0x46)                                                             \
    VK(VirtualKey, SCROLL_LOCK, 0x47)                                                              \
    VK(VirtualKey, PAUSE, 0x48)                                                                    \
    VK(VirtualKey, INSERT, 0x49)                                                                   \
    VK(VirtualKey, HOME, 0x4A)                                                                     \
    VK(VirtualKey, PAGE_UP, 0x4B)                                                                  \
    VK(VirtualKey, DELETE, 0x4C)                                                                   \
    VK(VirtualKey, END, 0x4D)                                                                      \
    VK(VirtualKey, PAGE_DOWN, 0x4E)                                                                \
    VK(VirtualKey, ARROW_RIGHT, 0x4F)                                                              \
    VK(VirtualKey, ARROW_LEFT, 0x50)                                                               \
    VK(VirtualKey, ARROW_DOWN, 0x51)                                                               \
    VK(VirtualKey, ARROW_UP, 0x52)                                                                 \
    VK(VirtualKey, NUM_LOCK, 0x53)                                                                 \
    VK(VirtualKey, KP_DIVIDE, 0x54)                                                                \
    VK(VirtualKey, KP_MULTIPLY, 0x55)                                                              \
    VK(VirtualKey, KP_MINUS, 0x56)                                                                 \
    VK(VirtualKey, KP_PLUS, 0x57)                                                                  \
    VK(VirtualKey, KP_ENTER, 0x58)                                                                 \
    VK(VirtualKey, KP_ONE, 0x59)                                                                   \
    VK(VirtualKey, KP_TWO, 0x5A)                                                                   \
    VK(VirtualKey, KP_THREE, 0x5B)                                                                 \
    VK(VirtualKey, KP_FOUR, 0x5C)                                                                  \
    VK(VirtualKey, KP_FIVE, 0x5D)                                                                  \
    VK(VirtualKey, KP_SIX, 0x5E)                                                                   \
    VK(VirtualKey, KP_SEVEN, 0x5F)                                                                 \
    VK(VirtualKey, KP_EIGHT, 0x60)                                                                 \
    VK(VirtualKey, KP_NINE, 0x61)                                                                  \
    VK(VirtualKey, KP_ZERO, 0x62)                                                                  \
    VK(VirtualKey, KP_PERIOD, 0x63)                                                                \
    VK(VirtualKey, NON_US_BACKSLASH, 0x64)                                                         \
    VK(VirtualKey, APPLICATION, 0x65)

    /// @brief 1-to-1 mapping of HID keyboard usages to virtual key codes.
    DECLARE_TYPED_ENUM(VirtualKey, U8, VIRTUAL_KEYS, 0x0) // NOLINT

    /// @brief A key event represents a pressed/released key and the active modifiers at the time
    ///         of the key press.
    ///
    /// A key event is encoded by 32-bit unsigned integers as follows:
    ///
    ///   31     17     16     15      8  7        0<br>
    /// | Reserved | KeyDown | Modifiers | VirtualKey |
    ///
    /// - VirtualKey: Virtual key code.
    /// - Modifiers: Bitmap of pressed modifiers.
    /// - KeyDown: 1 - The key was pressed, 0 - The key was released.
    ///
    /// The Modifiers bitmap is defined as:
    ///
    ///     15    14      13       12     11     10      9        8<br>
    /// | RGUI | RALT | RSHIFT | RCTRL | LGUI | LALT | LSHIFT | LCTRL |
    class KeyEvent {
        union {
            U32 m_as_u32;
            struct {
                U32 m_virtual_key : 8;
                U32 m_lctrl       : 1;
                U32 m_lshift      : 1;
                U32 m_lalt        : 1;
                U32 m_lgui        : 1; // e.g. Windows key, command key
                U32 m_rctrl       : 1;
                U32 m_rshift      : 1;
                U32 m_ralt        : 1;
                U32 m_rgui        : 1;
                U32 m_key_down    : 1;
                U32 m_reserved    : 15;
            };
        } m_event_code;

      public:
        static constexpr U32  EVENT_CODE_NONE = 0x0;
        static const KeyEvent NONE;

        KeyEvent();

        explicit KeyEvent(U32 event_code);

        /// @brief
        /// @return The virtual keycode
        [[nodiscard]] auto event_code() const -> U32;

        /// @brief
        /// @return The virtual key code of the key.
        [[nodiscard]] auto virtual_key() const -> VirtualKey;

        /// @brief
        /// @return True: The key was pressed
        ///         False: Otherwise
        [[nodiscard]] auto is_key_down() const -> bool;

        /// @brief
        /// @return True: Left Ctrl is pressed
        ///         False: Otherwise
        [[nodiscard]] auto is_lctrl_down() const -> bool;

        /// @brief
        /// @return True: Left Shift is pressed
        ///         False: Otherwise
        [[nodiscard]] auto is_lshift_down() const -> bool;

        /// @brief
        /// @return True: Left Alt is pressed
        ///         False: Otherwise
        [[nodiscard]] auto is_lalt_down() const -> bool;

        /// @brief
        /// @return True: Left GUI is pressed
        ///         False: Otherwise
        [[nodiscard]] auto is_lgui_down() const -> bool;

        /// @brief
        /// @return True: Right Ctrl is pressed
        ///         False: Otherwise
        [[nodiscard]] auto is_rctrl_down() const -> bool;

        /// @brief
        /// @return True: Right Shift is pressed
        ///         False: Otherwise
        [[nodiscard]] auto is_rshift_down() const -> bool;

        /// @brief
        /// @return True: Right Alt is pressed
        ///         False: Otherwise
        [[nodiscard]] auto is_ralt_down() const -> bool;

        /// @brief
        /// @return True: Right GUI is pressed
        ///         False: Otherwise
        [[nodiscard]] auto is_rgui_down() const -> bool;

        friend auto operator==(const KeyEvent& one, const KeyEvent& two) -> bool;

        friend auto operator!=(const KeyEvent& one, const KeyEvent& two) -> bool;
    };

    // ========================================================================================== //
    // Key Event Builder
    // ========================================================================================== //

    /// @brief Incrementally assembles the event code of a KeyEvent.
    class KeyEventBuilder {
        U32 m_event_code = 0x0;

      public:
        /// @brief Set the key that triggered the event.
        auto with_virtual_key(Ember::VirtualKey vk) -> KeyEventBuilder&;

        /// @brief Set or clear the left Ctrl modifier.
        auto with_lctrl(bool lctrl) -> KeyEventBuilder&;

        /// @brief Set or clear the left Shift modifier.
        auto with_lshift(bool lshift) -> KeyEventBuilder&;

        /// @brief Set or clear the left Alt modifier.
        auto with_lalt(bool lalt) -> KeyEventBuilder&;

        /// @brief Set or clear the left GUI modifier.
        auto with_lgui(bool lgui) -> KeyEventBuilder&;

        /// @brief Set or clear the right Ctrl modifier.
        auto with_rctrl(bool rctrl) -> KeyEventBuilder&;

        /// @brief Set or clear the right Shift modifier.
        auto with_rshift(bool rshift) -> KeyEventBuilder&;

        /// @brief Set or clear the right Alt modifier.
        auto with_ralt(bool ralt) -> KeyEventBuilder&;

        /// @brief Set or clear the right GUI modifier.
        auto with_rgui(bool rgui) -> KeyEventBuilder&;

        /// @brief Mark the key as pressed (true) or released (false).
        auto with_key_down(bool pressed) -> KeyEventBuilder&;

        /// @brief
        /// @return A key event with the configured key, modifiers and press state.
        [[nodiscard]] auto build() const -> KeyEvent;
    };
} // namespace Ember

#endif // EMBER_APP_H
