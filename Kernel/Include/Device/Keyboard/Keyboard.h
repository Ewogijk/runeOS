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

#ifndef RUNEOS_KEYBOARD_H
#define RUNEOS_KEYBOARD_H


#include <Hammer/Definitions.h>

#include <LibK/Stream.h>


namespace Rune::Device {
    /**
     * <p>
     *  A virtual key on the virtual keyboard which defines the keyboard as a 2D matrix of keys. Each key is defined by
     *  it's keycode which is an 16 bit unsigned integer defined as followed:
     * </p>
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
     */
    class VirtualKey {
        U16 _key_code;

    public:
        static constexpr U16    NONE_KEY_CODE = 0x8000;
        static const VirtualKey NONE;
        static constexpr U8     MAX_ROWS      = 8;
        static constexpr U8     MAX_COLS      = 32;


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


        explicit VirtualKey(U16 code);


        /**
         *
         * @return The integer representation of the key code.
         */
        [[nodiscard]] U16 get_key_code() const;


        /**
         *
         * @return Row position of the pressed key.
         */
        [[nodiscard]] U8 get_row() const;


        /**
         *
         * @return Column position of the pressed key.
         */
        [[nodiscard]] U8 get_col() const;


        /**
         *
         * @return True if the key is currently pressed.
         */
        [[nodiscard]] bool is_pressed() const;


        /**
         *
         * @return True if the key has been released.
         */
        [[nodiscard]] bool is_released() const;


        /**
         *
         * @return True if this virtual key code represents a "null" key.
         */
        [[nodiscard]] bool is_none() const;
    };


    /**
     * @brief The virtual keyboard maps physical keyboard scancodes to virtual keycodes and provides them to the system
     *          as a stream.
     */
    class VirtualKeyboard : public LibK::TextStream {
    public:

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          Virtual Keyboard API
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


        /**
         * @brief
         * @return
         */
        virtual bool start() = 0;



        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          Stream functions
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        bool is_read_supported() override;


        /**
         * @brief Read a single virtual key press from the buffer.
         * @return NoneKeyCode: The buffer is empty, Else: A virtual key code of the oldest pressed key.
         */
        int read() override = 0;


        bool is_write_supported() override;


        bool write(U8 value) override;


        /**
         * @brief Flush a virtual keys in the buffer.
         */
        void flush() override = 0;


        void close() override;


        bool is_ansi_supported() override;
    };
}

#endif //RUNEOS_KEYBOARD_H
