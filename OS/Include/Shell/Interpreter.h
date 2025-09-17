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

#ifndef RUNEOS_INTERPRETER_H
#define RUNEOS_INTERPRETER_H

#include <Ember/Ember.h>

#include <Shell/AST.h>
#include <Shell/Parser.h>

#include <string>

namespace Rune::Shell {

    struct Pixel {
        U8 red;
        U8 green;
        U8 blue;
    };

    /**
     * @brief State of the usual keyboard modifiers ctrl, etc.
     */
    struct KeyboardModifierState {
        bool ctrl_pressed   = false;
        bool shift_pressed  = false;
        bool alt_pressed    = false;
        bool alt_gr_pressed = false;
        bool angry_mode_on  = false; // Caps lock
    };

    class Interpreter {
        static constexpr Pixel GRAPE             = {0x6E, 0x17, 0xB5};
        static constexpr U8    MAX_ROWS          = 8;
        static constexpr U8    MAX_COLS          = 32;
        static constexpr U8    INPUT_BUFFER_SIZE = 128;

        // Maps a virtual keycode to an ascii char
        char _key_code_decoder[MAX_ROWS * MAX_COLS] = {
            '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
            '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
            '\0', '\0', '\0', '\0', '^',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',
            '0',  '\0', '\0', '\b', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
            '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', 'q',  'w',  'e',  'r',  't',
            'z',  'u',  'i',  'o',  'p',  '\0', '+',  '\n', '\0', '\0', '\0', '\0', '\0', '\0',
            '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', 'a',
            's',  'd',  'f',  'g',  'h',  'j',  'k',  'l',  '\0', '\0', '#',  '\n', '\0', '\0',
            '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
            '\0', '\0', '\0', '<',  'y',  'x',  'c',  'v',  'b',  'n',  'm',  ',',  '.',  '-',
            '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
            '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', ' ',  ' ',  ' ',  ' ',
            ' ',  ' ',  '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
            '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};

        // Maps a virtual keycode to an ascii char
        char _key_code_decoder_upper[MAX_ROWS * MAX_COLS] = {
            '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
            '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
            '\0', '\0', '\0', '\0', '\0', '!',  '"',  '\0', '$',  '%',  '&',  '/',  '(',  ')',
            '=',  '?',  '`',  '\b', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
            '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', 'Q',  'W',  'E',  'R',  'T',
            'Z',  'U',  'I',  'O',  'P',  '\0', '*',  '\0', '\0', '\0', '\0', '\0', '\0', '\0',
            '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', 'A',
            'S',  'D',  'F',  'G',  'H',  'J',  'K',  'L',  '\0', '\0', '\'', '\n', '\0', '\0',
            '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
            '\0', '\0', '\0', '>',  'Y',  'X',  'C',  'V',  'B',  'N',  'M',  ';',  ':',  '_',
            '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
            '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', ' ',  ' ',  ' ',  ' ',
            ' ',  ' ',  '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
            '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};

        // Maps a virtual keycode to an ascii char
        char _key_code_decoder_alt_gr[MAX_ROWS * MAX_COLS] = {
            '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
            '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
            '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '{',  '[',  ']',
            '}',  '\\', '\0', '\b', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
            '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '@',  '\0', '\0', '\0', '\0',
            '\0', '\0', '\0', '\0', '\0', '\0', '~',  '\0', '\0', '\0', '\0', '\0', '\0', '\0',
            '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
            '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\n', '\0', '\0',
            '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
            '\0', '\0', '\0', '|',  '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
            '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
            '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', ' ',  ' ',  ' ',  ' ',
            ' ',  ' ',  '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
            '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};

        KeyboardModifierState _keyboard_modifier;
        Environment           _env;
        Parser                _parser;

        void print_pretty_line_start() const;

        void exec(const std::string& input);

      public:
        explicit Interpreter();

        /**
         * The shell working directory, $PATH environment variable, built-in commands, hotkeys and
         * last the auto completion will be setup.
         *
         * @brief Configure the shell environment.
         * @param wd Shell working directory.
         * @return True: The shell is setup, False: Configuration error.
         */
        bool setup_environment(const char* wd);

        /**
         * @brief Run the command line interpreter.
         */
        void run();
    };
} // namespace Rune::Shell

#endif // RUNEOS_INTERPRETER_H
