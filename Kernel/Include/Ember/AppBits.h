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

    /**
     * @brief A virtual key on the virtual keyboard which defines the keyboard as a 2D matrix of
     * keys. Each key is defined by it's keycode which is an 16 bit unsigned integer defined as
     * followed:
     *
     * <p>
     *        15      14      13       9 8   4 2   0 <br>
     *    | None | Released | Reserved | Col | Row |
     * </p>
     *
     * <p>
     *  Row     : Row of the key.<br>
     *  Col     : Column of the key.<br>
     *  Released: 1 - Key is released, 0 - Key is pressed<br>
     *  None    : 1 - This key represents a "null" key, 0 - This key represents an actual key press.
     * </p>
     *
     * <p>
     *  Note: This is a duplication of the struct in Device/Keyboard/Keyboard.h.
     * </p>
     */
    class VirtualKey {
        U16 _key_code;

      public:
        static constexpr U16    NONE_KEY_CODE = 0x8000;
        static const VirtualKey NONE;
        static constexpr U8     MAX_ROWS = 8;
        static constexpr U8     MAX_COLS = 32;

        /**
         * Create virtual key code in the given row and col that has been released or pressed.
         *
         * @param row       Row.
         * @param col       Column.
         * @param released  True if the key has been released, false if pressed.
         * @return A virtual key code.
         */
        static VirtualKey build(U8 row, U8 col, bool released);

        /**
         * Create virtual key code in the given row and col that has been pressed.
         *
         * @param row       Row.
         * @param col       Column.
         * @return A virtual key code.
         */
        static VirtualKey build_pressed(U8 row, U8 col);

        /**
         * Create virtual key code in the given row and col that has been released.
         *
         * @param row       Row.
         * @param col       Column.
         * @return A virtual key code.
         */
        static VirtualKey build_released(U8 row, U8 col);

        VirtualKey();

        explicit VirtualKey(U16 key_code);

        /**
         *
         * @return The integer representation of the key code.
         */
        [[nodiscard]]
        U16 get_key_code() const;

        /**
         *
         * @return Row position of the pressed key.
         */
        [[nodiscard]]
        U8 get_row() const;

        /**
         *
         * @return Column position of the pressed key.
         */
        [[nodiscard]]
        U8 get_col() const;

        /**
         *
         * @return True if the key is currently pressed.
         */
        [[nodiscard]]
        bool is_pressed() const;

        /**
         *
         * @return True if the key has been released.
         */
        [[nodiscard]]
        bool is_released() const;

        /**
         *
         * @return True if this virtual key code represents a "null" key.
         */
        [[nodiscard]]
        bool is_none() const;

        friend bool operator==(const VirtualKey& one, const VirtualKey& two);

        friend bool operator!=(const VirtualKey& one, const VirtualKey& two);
    };
} // namespace Ember

#endif // EMBER_APP_H
