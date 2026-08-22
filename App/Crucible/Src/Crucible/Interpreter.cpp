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

#include <Crucible/Interpreter.h>

#include <Crucible/Action.h>
#include <Crucible/BuiltInCommand.h>
#include <Crucible/Utility.h>

#include <Forge/App.h>

#include <iostream>
#include <vector>

namespace Crucible {
    const KeyCodeDecoder Interpreter::DECODER = {
        {               Ember::VirtualKey::A,  'a'},
        {               Ember::VirtualKey::B,  'b'},
        {               Ember::VirtualKey::C,  'c'},
        {               Ember::VirtualKey::D,  'd'},
        {               Ember::VirtualKey::E,  'e'},
        {               Ember::VirtualKey::F,  'f'},
        {               Ember::VirtualKey::G,  'g'},
        {               Ember::VirtualKey::H,  'h'},
        {               Ember::VirtualKey::I,  'i'},
        {               Ember::VirtualKey::J,  'j'},
        {               Ember::VirtualKey::K,  'k'},
        {               Ember::VirtualKey::L,  'l'},
        {               Ember::VirtualKey::M,  'm'},
        {               Ember::VirtualKey::N,  'n'},
        {               Ember::VirtualKey::O,  'o'},
        {               Ember::VirtualKey::P,  'p'},
        {               Ember::VirtualKey::Q,  'q'},
        {               Ember::VirtualKey::R,  'r'},
        {               Ember::VirtualKey::S,  's'},
        {               Ember::VirtualKey::T,  't'},
        {               Ember::VirtualKey::U,  'u'},
        {               Ember::VirtualKey::V,  'v'},
        {               Ember::VirtualKey::W,  'w'},
        {               Ember::VirtualKey::X,  'x'},
        {               Ember::VirtualKey::Y,  'z'},
        {               Ember::VirtualKey::Z,  'y'},
        {             Ember::VirtualKey::ONE,  '1'},
        {             Ember::VirtualKey::TWO,  '2'},
        {           Ember::VirtualKey::THREE,  '3'},
        {            Ember::VirtualKey::FOUR,  '4'},
        {            Ember::VirtualKey::FIVE,  '5'},
        {             Ember::VirtualKey::SIX,  '6'},
        {           Ember::VirtualKey::SEVEN,  '7'},
        {           Ember::VirtualKey::EIGHT,  '8'},
        {            Ember::VirtualKey::NINE,  '9'},
        {            Ember::VirtualKey::ZERO,  '0'},
        {           Ember::VirtualKey::ENTER, '\n'},
        {       Ember::VirtualKey::BACKSPACE, '\b'},
        {           Ember::VirtualKey::SPACE,  ' '},
        {           Ember::VirtualKey::EQUAL,  '`'}, // "´" on a german keyboard
        {   Ember::VirtualKey::RIGHT_BRACKET,  '+'},
        {       Ember::VirtualKey::BACKSLASH,  '#'},
        {     Ember::VirtualKey::NON_US_HASH,  '#'},
        {           Ember::VirtualKey::GRAVE,  '^'},
        {           Ember::VirtualKey::COMMA,  ','},
        {          Ember::VirtualKey::PERIOD,  '.'},
        {           Ember::VirtualKey::SLASH,  '-'},
        {Ember::VirtualKey::NON_US_BACKSLASH,  '<'},
        {       Ember::VirtualKey::KP_DIVIDE,  '/'},
        {     Ember::VirtualKey::KP_MULTIPLY,  '*'},
        {        Ember::VirtualKey::KP_MINUS,  '-'},
        {         Ember::VirtualKey::KP_PLUS,  '+'},
        {        Ember::VirtualKey::KP_ENTER, '\n'},
        {          Ember::VirtualKey::KP_ONE,  '1'},
        {          Ember::VirtualKey::KP_TWO,  '2'},
        {        Ember::VirtualKey::KP_THREE,  '3'},
        {         Ember::VirtualKey::KP_FOUR,  '4'},
        {         Ember::VirtualKey::KP_FIVE,  '5'},
        {          Ember::VirtualKey::KP_SIX,  '6'},
        {        Ember::VirtualKey::KP_SEVEN,  '7'},
        {        Ember::VirtualKey::KP_EIGHT,  '8'},
        {         Ember::VirtualKey::KP_NINE,  '9'},
        {         Ember::VirtualKey::KP_ZERO,  '0'},
        {       Ember::VirtualKey::KP_PERIOD,  '.'},
    };

    const KeyCodeDecoder Interpreter::DECODER_UPPER = {
        {               Ember::VirtualKey::A,  'A'},
        {               Ember::VirtualKey::B,  'B'},
        {               Ember::VirtualKey::C,  'C'},
        {               Ember::VirtualKey::D,  'D'},
        {               Ember::VirtualKey::E,  'E'},
        {               Ember::VirtualKey::F,  'F'},
        {               Ember::VirtualKey::G,  'G'},
        {               Ember::VirtualKey::H,  'H'},
        {               Ember::VirtualKey::I,  'I'},
        {               Ember::VirtualKey::J,  'J'},
        {               Ember::VirtualKey::K,  'K'},
        {               Ember::VirtualKey::L,  'L'},
        {               Ember::VirtualKey::M,  'M'},
        {               Ember::VirtualKey::N,  'N'},
        {               Ember::VirtualKey::O,  'O'},
        {               Ember::VirtualKey::P,  'P'},
        {               Ember::VirtualKey::Q,  'Q'},
        {               Ember::VirtualKey::R,  'R'},
        {               Ember::VirtualKey::S,  'S'},
        {               Ember::VirtualKey::T,  'T'},
        {               Ember::VirtualKey::U,  'U'},
        {               Ember::VirtualKey::V,  'V'},
        {               Ember::VirtualKey::W,  'W'},
        {               Ember::VirtualKey::X,  'X'},
        {               Ember::VirtualKey::Y,  'Z'},
        {               Ember::VirtualKey::Z,  'Y'},
        {             Ember::VirtualKey::ONE,  '!'},
        {             Ember::VirtualKey::TWO,  '"'},
        // THREE is "§" on a german keyboard -> no ASCII char
        {            Ember::VirtualKey::FOUR,  '$'},
        {            Ember::VirtualKey::FIVE,  '%'},
        {             Ember::VirtualKey::SIX,  '&'},
        {           Ember::VirtualKey::SEVEN,  '/'},
        {           Ember::VirtualKey::EIGHT,  '('},
        {            Ember::VirtualKey::NINE,  ')'},
        {            Ember::VirtualKey::ZERO,  '='},
        {           Ember::VirtualKey::MINUS,  '?'}, // "ß" on a german keyboard
        {           Ember::VirtualKey::ENTER, '\n'},
        {       Ember::VirtualKey::BACKSPACE, '\b'},
        {           Ember::VirtualKey::SPACE,  ' '},
        {   Ember::VirtualKey::RIGHT_BRACKET,  '*'},
        {       Ember::VirtualKey::BACKSLASH, '\''},
        {     Ember::VirtualKey::NON_US_HASH, '\''},
        {           Ember::VirtualKey::COMMA,  ';'},
        {          Ember::VirtualKey::PERIOD,  ':'},
        {           Ember::VirtualKey::SLASH,  '_'},
        {Ember::VirtualKey::NON_US_BACKSLASH,  '>'},
        {       Ember::VirtualKey::KP_DIVIDE,  '/'},
        {     Ember::VirtualKey::KP_MULTIPLY,  '*'},
        {        Ember::VirtualKey::KP_MINUS,  '-'},
        {         Ember::VirtualKey::KP_PLUS,  '+'},
        {        Ember::VirtualKey::KP_ENTER, '\n'},
    };

    const KeyCodeDecoder Interpreter::DECODER_ALT_GR = {
        {               Ember::VirtualKey::Q,  '@'},
        {           Ember::VirtualKey::SEVEN,  '{'},
        {           Ember::VirtualKey::EIGHT,  '['},
        {            Ember::VirtualKey::NINE,  ']'},
        {            Ember::VirtualKey::ZERO,  '}'},
        {           Ember::VirtualKey::MINUS, '\\'}, // "ß" on a german keyboard
        {   Ember::VirtualKey::RIGHT_BRACKET,  '~'},
        {Ember::VirtualKey::NON_US_BACKSLASH,  '|'},
        {           Ember::VirtualKey::ENTER, '\n'},
        {        Ember::VirtualKey::KP_ENTER, '\n'},
        {       Ember::VirtualKey::BACKSPACE, '\b'},
        {           Ember::VirtualKey::SPACE,  ' '},
    };

    auto Interpreter::decode(const Ember::KeyEvent& key_event) const -> char {
        const KeyCodeDecoder* decoder = &DECODER;
        if (key_event.is_lshift_down() || key_event.is_rshift_down() || m_is_caps_on) {
            decoder = &DECODER_UPPER;
        } else if (key_event.is_lalt_down() || key_event.is_ralt_down()) {
            decoder = &DECODER_ALT_GR;
        }
        const auto ch = decoder->find(key_event.virtual_key());
        return ch != decoder->end() ? ch->second : '\0';
    }

    void Interpreter::print_pretty_line_start() const {
        std::cout << "\033[38;2;" << static_cast<int>(GRAPE.red) << ";"
                  << static_cast<int>(GRAPE.green) << ";" << static_cast<int>(GRAPE.blue) << "m"
                  << _env.working_directory.to_string() << "\033[0m> ";
        std::cout.flush();
    }

    void Interpreter::exec(const std::string& input) {
        auto [ast_node, has_error, actual, expected] = _parser.parse_shell_input(input);
        if (has_error) {
            // Print some error information that hopefully helps to resolve the issue
            const std::string error_prefix = "Error parsing: ";
            std::string       highlight;
            for (size_t i = 0; i < actual.position + error_prefix.size(); i++) highlight += ' ';
            for (size_t i = 0; i < actual.text.size(); i++) highlight += '^';

            std::cerr << error_prefix << input << std::endl;
            std::cerr << highlight << std::endl;
            std::cerr << "Unexpected token at " << actual.position << ": "
                      << actual.type.to_string() << std::endl;
            std::cerr << "Expected: " << expected.to_string() << std::endl << std::endl;
        } else {
            // Execute the parsed command
            ast_node->evaluate(_env);
        }
    }

    Interpreter::Interpreter() : _env() {}

    auto Interpreter::setup_environment(const char* wd) -> bool {
        _env.working_directory     = Path(wd);
        _env.env_var_table["PATH"] = "/Apps";
        register_builtin_commands(_env);
        register_hotkey_actions(_env);

        std::vector<std::string> b_cmd_list;
        for (auto& [fst, snd] : _env.command_table) b_cmd_list.push_back(fst);
        return _env.auto_completion.init_vocabulary(
            b_cmd_list,
            str_split(_env.env_var_table.find("PATH")->second, ':'));
    }

    auto Interpreter::run() -> void { // NOLINT
        print_pretty_line_start();
        while (true) {
            if (Ember::KeyEvent key_event = Forge::app_read_stdin();
                key_event != Ember::KeyEvent::NONE) {
                if (key_event.virtual_key() == Ember::VirtualKey::CAPS_LOCK) {
                    if (key_event.is_key_down()) m_is_caps_on = !m_is_caps_on;
                    continue;
                }

                if (key_event.is_key_down()) {
                    // Convert the virtual key to ascii
                    const char ch = decode(key_event);

                    // Print if the pressed key is human-readable
                    if (ch != '\0') {
                        switch (ch) {
                            case '\b': _env.input_delete(false); break;
                            case '\n': {
                                std::cout << ch;
                                std::cout.flush();
                                if (_env.input_buffer_size > 0) {
                                    auto cmd = std::string(_env.input_buffer.data(),
                                                           _env.input_buffer_size);
                                    _env.command_history.push_back(cmd);
                                    _env.command_history_cursor = _env.command_history.size();
                                    exec(cmd);
                                    _env.input_delete_all(false);
                                }
                                print_pretty_line_start();
                                break;
                            }
                            default: _env.input_append(ch);
                        }
                    } else {
                        if (auto action = _env.action_table.find(key_event.virtual_key());
                            action != _env.action_table.end())
                            (action->second)(_env);
                    }
                }
            }
        }
    }
} // namespace Crucible
