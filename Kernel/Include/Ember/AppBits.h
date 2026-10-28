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

#ifndef EMBER_APP_H
#define EMBER_APP_H

#include <Ember/Ember.h>

namespace Ember {

#define STD_IO_TARGETS(X)                                                                          \
    X(VOID, 0x1)                                                                                   \
    X(INHERIT, 0x2)                                                                                \
    X(FILE, 0x3)                                                                                   \
    X(PIPE, 0x4)

    /// @brief Describes to which target a std IO stream of an application can be connected.
    ///
    /// VOID: Connect the standard stream to the void stream, which provides no output and ignores
    ///         all input.<br>
    /// INHERIT: Connect stdin/stdout/stderr of the application to the std streams of the parent
    ///             app.<br>
    /// FILE: Only stdout/stderr: Redirect to the specified file, if it does not exist it will be
    ///         created.<br>
    /// PIPE: Connect the standard stream to the requested pipe.<br>
    EMBER_DEFINE_SMART_ENUM(StdIOTarget, STD_IO_TARGETS, 0x0) // NOLINT

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
    VK(A, 0x04)                                                                                    \
    VK(B, 0x05)                                                                                    \
    VK(C, 0x06)                                                                                    \
    VK(D, 0x07)                                                                                    \
    VK(E, 0x08)                                                                                    \
    VK(F, 0x09)                                                                                    \
    VK(G, 0x0A)                                                                                    \
    VK(H, 0x0B)                                                                                    \
    VK(I, 0x0C)                                                                                    \
    VK(J, 0x0D)                                                                                    \
    VK(K, 0x0E)                                                                                    \
    VK(L, 0x0F)                                                                                    \
    VK(M, 0x10)                                                                                    \
    VK(N, 0x11)                                                                                    \
    VK(O, 0x12)                                                                                    \
    VK(P, 0x13)                                                                                    \
    VK(Q, 0x14)                                                                                    \
    VK(R, 0x15)                                                                                    \
    VK(S, 0x16)                                                                                    \
    VK(T, 0x17)                                                                                    \
    VK(U, 0x18)                                                                                    \
    VK(V, 0x19)                                                                                    \
    VK(W, 0x1A)                                                                                    \
    VK(X, 0x1B)                                                                                    \
    VK(Y, 0x1C)                                                                                    \
    VK(Z, 0x1D)                                                                                    \
    VK(ONE, 0x1E)                                                                                  \
    VK(TWO, 0x1F)                                                                                  \
    VK(THREE, 0x20)                                                                                \
    VK(FOUR, 0x21)                                                                                 \
    VK(FIVE, 0x22)                                                                                 \
    VK(SIX, 0x23)                                                                                  \
    VK(SEVEN, 0x24)                                                                                \
    VK(EIGHT, 0x25)                                                                                \
    VK(NINE, 0x26)                                                                                 \
    VK(ZERO, 0x27)                                                                                 \
    VK(ENTER, 0x28)                                                                                \
    VK(ESCAPE, 0x29)                                                                               \
    VK(BACKSPACE, 0x2A)                                                                            \
    VK(TAB, 0x2B)                                                                                  \
    VK(SPACE, 0x2C)                                                                                \
    VK(MINUS, 0x2D)                                                                                \
    VK(EQUAL, 0x2E)                                                                                \
    VK(LEFT_BRACKET, 0x2F)                                                                         \
    VK(RIGHT_BRACKET, 0x30)                                                                        \
    VK(BACKSLASH, 0x31)                                                                            \
    VK(NON_US_HASH, 0x32)                                                                          \
    VK(SEMICOLON, 0x33)                                                                            \
    VK(APOSTROPHE, 0x34)                                                                           \
    VK(GRAVE, 0x35)                                                                                \
    VK(COMMA, 0x36)                                                                                \
    VK(PERIOD, 0x37)                                                                               \
    VK(SLASH, 0x38)                                                                                \
    VK(CAPS_LOCK, 0x39)                                                                            \
    VK(F1, 0x3A)                                                                                   \
    VK(F2, 0x3B)                                                                                   \
    VK(F3, 0x3C)                                                                                   \
    VK(F4, 0x3D)                                                                                   \
    VK(F5, 0x3E)                                                                                   \
    VK(F6, 0x3F)                                                                                   \
    VK(F7, 0x40)                                                                                   \
    VK(F8, 0x41)                                                                                   \
    VK(F9, 0x42)                                                                                   \
    VK(F10, 0x43)                                                                                  \
    VK(F11, 0x44)                                                                                  \
    VK(F12, 0x45)                                                                                  \
    VK(PRINT_SCREEN, 0x46)                                                                         \
    VK(SCROLL_LOCK, 0x47)                                                                          \
    VK(PAUSE, 0x48)                                                                                \
    VK(INSERT, 0x49)                                                                               \
    VK(HOME, 0x4A)                                                                                 \
    VK(PAGE_UP, 0x4B)                                                                              \
    VK(DELETE, 0x4C)                                                                               \
    VK(END, 0x4D)                                                                                  \
    VK(PAGE_DOWN, 0x4E)                                                                            \
    VK(ARROW_RIGHT, 0x4F)                                                                          \
    VK(ARROW_LEFT, 0x50)                                                                           \
    VK(ARROW_DOWN, 0x51)                                                                           \
    VK(ARROW_UP, 0x52)                                                                             \
    VK(NUM_LOCK, 0x53)                                                                             \
    VK(KP_DIVIDE, 0x54)                                                                            \
    VK(KP_MULTIPLY, 0x55)                                                                          \
    VK(KP_MINUS, 0x56)                                                                             \
    VK(KP_PLUS, 0x57)                                                                              \
    VK(KP_ENTER, 0x58)                                                                             \
    VK(KP_ONE, 0x59)                                                                               \
    VK(KP_TWO, 0x5A)                                                                               \
    VK(KP_THREE, 0x5B)                                                                             \
    VK(KP_FOUR, 0x5C)                                                                              \
    VK(KP_FIVE, 0x5D)                                                                              \
    VK(KP_SIX, 0x5E)                                                                               \
    VK(KP_SEVEN, 0x5F)                                                                             \
    VK(KP_EIGHT, 0x60)                                                                             \
    VK(KP_NINE, 0x61)                                                                              \
    VK(KP_ZERO, 0x62)                                                                              \
    VK(KP_PERIOD, 0x63)                                                                            \
    VK(NON_US_BACKSLASH, 0x64)                                                                     \
    VK(APPLICATION, 0x65)

    /// @brief 1-to-1 mapping of HID keyboard usages to virtual key codes.
    EMBER_DEFINE_SMART_ENUM(VirtualKey, VIRTUAL_KEYS, 0x0, U8) // NOLINT

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
