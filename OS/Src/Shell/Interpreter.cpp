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

#include <Shell/Interpreter.h>

#include <Shell/Action.h>
#include <Shell/BuiltInCommand.h>

#include <Forge/App.h>

#include <iostream>
#include <vector>

#include "Shell/Utility.h"

namespace Rune::Shell {
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
            std::string       highlight("");
            for (size_t i = 0; i < actual.position + error_prefix.size(); i++)
                highlight += ' ';
            for (size_t i = 0; i < actual.text.size(); i++)
                highlight += '^';

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

    Interpreter::Interpreter() : _keyboard_modifier(), _env(), _parser() {}

    bool Interpreter::setup_environment(const char* wd) {
        _env.working_directory     = Path(wd);
        _env.env_var_table["PATH"] = "/Apps";
        register_builtin_commands(_env);
        register_hotkey_actions(_env);

        std::vector<std::string> b_cmd_list;
        for (auto& [fst, snd] : _env.command_table)
            b_cmd_list.push_back(fst);
        return _env.auto_completion.init_vocabulary(
            b_cmd_list,
            str_split(_env.env_var_table.find("PATH")->second, ':'));
    }

    void Interpreter::run() {
        print_pretty_line_start();
        while (true) {
            if (Ember::VirtualKey key = Forge::app_read_stdin(); !key.is_none()) {
                // Update modifiers
                if ((key.get_row() == 4 && key.get_col() == 0)
                    || (key.get_row() == 4 && key.get_col() == 12)) {
                    _keyboard_modifier.shift_pressed = key.is_pressed();
                    continue;
                } else if (key.get_row() == 5 && key.get_col() == 0) {
                    _keyboard_modifier.ctrl_pressed = key.is_pressed();
                    continue;
                } else if (key.get_row() == 5 && key.get_col() == 2) {
                    _keyboard_modifier.alt_pressed = key.is_pressed();
                    continue;
                } else if (key.get_row() == 5 && key.get_col() == 10) {
                    _keyboard_modifier.alt_gr_pressed = key.is_pressed();
                    continue;
                } else if (key.get_row() == 3 && key.get_col() == 0) {
                    if (key.is_pressed())
                        _keyboard_modifier.angry_mode_on = !_keyboard_modifier.angry_mode_on;
                    continue;
                }

                if (key.is_pressed()) {
                    // Convert keycode to ascii
                    char         ch = '\0';
                    const size_t kd_off =
                        key.get_row() * Ember::VirtualKey::MAX_COLS + key.get_col();
                    if (_keyboard_modifier.shift_pressed || _keyboard_modifier.angry_mode_on) {
                        ch = _key_code_decoder_upper[kd_off];
                    } else if (_keyboard_modifier.alt_gr_pressed) {
                        ch = _key_code_decoder_alt_gr[kd_off];
                    } else {
                        ch = _key_code_decoder[kd_off];
                    }

                    // Print if the pressed key is human-readable
                    if (ch != '\0') {
                        switch (ch) {
                            case '\b': _env.input_delete(false); break;
                            case '\n': {
                                std::cout << ch;
                                std::cout.flush();
                                if (_env.input_buffer_size > 0) {
                                    auto cmd =
                                        std::string(_env.input_buffer, _env.input_buffer_size);
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
                        if (auto action = _env.action_table.find(key);
                            action != _env.action_table.end())
                            (action->second)(_env);
                    }
                }
            }
        }
    }
} // namespace Rune::Shell
