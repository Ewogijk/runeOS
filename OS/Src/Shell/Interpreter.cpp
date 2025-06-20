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

#include <Shell/BuiltInCommand.h>
#include <Shell/Action.h>

#include <StdIO.h>

#include <Pickaxe/AppManagement.h>


namespace Rune::Shell {


    void Interpreter::print_pretty_line_start() {
        print_out(
                "\033[38;2;{};{};{}m{}\033[0m> ",
                GRAPE.red,
                GRAPE.green,
                GRAPE.blue,
                _env.working_directory.to_string().to_cstr()
        );
    }


    void Interpreter::exec(const String& input) {
        Shell::ParsedInput parsed_input = _parser.parse_shell_input(input);
        if (parsed_input.has_error) {
            // Print some error information that hopefully helps to resolve the issue
            String      error_prefix = String::format("Error parsing: ", input);
            String      highlight("");
            for (size_t i            = 0; i < parsed_input.actual.position + error_prefix.size(); i++)
                highlight += ' ';
            for (size_t i            = 0; i < parsed_input.actual.text.size(); i++)
                highlight += '^';

            print_err("{}{}\n", error_prefix, input);
            print_err("{}\n", highlight);
            print_err(
                    "Unexpected token at {}: {}\n",
                    parsed_input.actual.position,
                    parsed_input.actual.type.to_string()
            );
            print_err("Expected: {}\n\n", parsed_input.expected.to_string());
        } else {
            // Execute the parsed command
            parsed_input.ast_node->evaluate(_env);
        }
    }


    Interpreter::Interpreter() : _keyboard_modifier(),
                                 _env(),
                                 _parser() {


    }


    bool Interpreter::setup_environment(const char* wd) {
        _env.working_directory = Path(wd);
        _env.env_var_table.put("PATH", "/Apps");
        register_builtin_commands(_env);
        register_hotkey_actions(_env);

        LinkedList<String> b_cmd_list;
        for (auto& kv_pair: _env.command_table)
            b_cmd_list.add_back(*kv_pair.key);
        return _env.auto_completion.init_vocabulary(b_cmd_list, _env.env_var_table.find("PATH")->value->split(':'));
    }


    void Interpreter::run() {
        print_pretty_line_start();
        while (true) {
            Pickaxe::VirtualKey key = Pickaxe::read_std_in();
            if (!key.is_none()) {
                // Update modifiers
                if ((key.get_row() == 4 && key.get_col() == 0) || (key.get_row() == 4 && key.get_col() == 12)) {
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
                    char   ch     = '\0';
                    size_t kd_off = key.get_row() * Pickaxe::VirtualKey::MAX_COLS + key.get_col();
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
                            case '\b':
                                _env.input_delete(false);
                                break;
                            case '\n': {
                                print_out(ch);
                                if (_env.input_buffer_size > 0) {
                                    String cmd = String(_env.input_buffer, _env.input_buffer_size);
                                    _env.command_history.add_back(cmd);
                                    _env.command_history_cursor = _env.command_history.size();
                                    exec(cmd);
                                    _env.input_delete_all(false);
                                }
                                print_pretty_line_start();
                                break;
                            }
                            default:
                                _env.input_append(ch);
                        }
                    } else {
                        auto action = _env.action_table.find(key);
                        if (action != _env.action_table.end())
                            (*action->value)(_env);
                    }
                }
            }
        }
    }
}