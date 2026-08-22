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

#ifndef CRUCIBLE_INTERPRETER_H
#define CRUCIBLE_INTERPRETER_H

#include <Ember/AppBits.h>
#include <Ember/Ember.h>

#include <Crucible/AST.h>
#include <Crucible/Environment.h>
#include <Crucible/Parser.h>

#include <string>
#include <unordered_map>

namespace Crucible {

    struct Pixel {
        U8 red;
        U8 green;
        U8 blue;
    };

    /// @brief Maps a virtual key to the ASCII char it produces on a german QWERTZ keyboard.
    using KeyCodeDecoder = std::unordered_map<Ember::VirtualKey, char>;

    class Interpreter {
        static constexpr Pixel  GRAPE             = {.red = 0x6E, .green = 0x17, .blue = 0xB5};
        static constexpr size_t INPUT_BUFFER_SIZE = 128;

        // Decoding tables for the unmodified keys, the shift/caps lock layer and the alt gr layer.
        static const KeyCodeDecoder DECODER;
        static const KeyCodeDecoder DECODER_UPPER;
        static const KeyCodeDecoder DECODER_ALT_GR;

        /// @brief Decode a key event to the ASCII char produced by the pressed key and active
        ///         modifiers.
        /// @param key_event A key event.
        /// @return The ASCII char of the key or '\0' if the key produces no ASCII char.
        [[nodiscard]] auto decode(const Ember::KeyEvent& key_event) const -> char;

        bool        m_is_caps_on = false;
        Environment _env;
        Parser      _parser;

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
        auto setup_environment(const char* wd) -> bool;

        /**
         * @brief Run the command line interpreter.
         */
        void run();
    };
} // namespace Crucible

#endif // CRUCIBLE_INTERPRETER_H
