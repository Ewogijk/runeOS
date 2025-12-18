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

#include <Crucible/Action.h>

#include <iostream>
#include <string>
#include <vector>

#include <Crucible/Utility.h>

namespace Crucible {
    void command_history_scroll_up(Environment& shell_env) {
        // Arrow up pressed
        if (shell_env.command_history_cursor == shell_env.command_history.size()) {
            // The user was entering some command and then started scrolling through older commands
            // -> Save the current input, so it can be restored
            shell_env.input_buffer_backup =
                std::string(shell_env.input_buffer, shell_env.input_buffer_size);
        }

        if (shell_env.command_history_cursor > 0) shell_env.command_history_cursor--;
        if (shell_env.command_history_cursor < shell_env.command_history.size())
            shell_env.input_set(
                shell_env.command_history[shell_env.command_history_cursor].c_str());
    }

    void command_history_scroll_down(Environment& shell_env) {
        if (shell_env.command_history_cursor < shell_env.command_history.size()) {
            shell_env.command_history_cursor++;
            if (shell_env.command_history_cursor < shell_env.command_history.size()) {
                shell_env.input_set(
                    shell_env.command_history[shell_env.command_history_cursor].c_str());
            } else {
                shell_env.input_set(shell_env.input_buffer_backup);
            }
        }
    }

    void cursor_move_left(Environment& shell_env) {
        if (shell_env.input_buffer_cursor > 0) {
            std::cout << "\033[1D";
            shell_env.input_buffer_cursor--;
            shell_env.ac_used = false;
            std::cout.flush();
        }
    }

    void cursor_move_right(Environment& shell_env) {
        if (shell_env.input_buffer_cursor < shell_env.input_buffer_size) {
            std::cout << "\033[1C";
            shell_env.input_buffer_cursor++;
            shell_env.ac_used = false;
            std::cout.flush();
        }
    }

    void delete_forward(Environment& shell_env) { shell_env.input_delete(true); }

    void perform_auto_completion(Environment& shell_env) {
        if (shell_env.input_buffer_size == 0) return; // No input -> Do not auto complete

        if (!shell_env.ac_used || shell_env.ac_word_suggestions.size() == 1) {
            const std::string input(shell_env.input_buffer, shell_env.input_buffer_cursor);
            const std::vector<std::string> parts = str_split(input, ' ');
            if (const bool has_ws_suffix = input[input.size() - 1] == ' ';
                parts.size() == 1 && !has_ws_suffix) {
                // Input contains one part and no ws at end e.g. "cle" -> A command is being entered
                shell_env.ac_word_suggestions =
                    shell_env.auto_completion.auto_complete_command(input);
                shell_env.ac_prefix = "";
            } else {
                // Input contains multiple parts or a finished command e.g. "clear " or "ls myfi"
                // -> A flag or file is being entered
                if (const std::string last_arg = parts.back();
                    str_is_prefix("-", last_arg) || str_is_prefix("--", last_arg)) {
                    // Tab completion on flags is not supported -> Clear last word suggestions
                    shell_env.ac_word_suggestions.clear();
                } else {
                    // A file input is being entered
                    shell_env.ac_word_suggestions = shell_env.auto_completion.auto_complete_node(
                        shell_env.working_directory,
                        Path(has_ws_suffix ? "" : last_arg));

                    std::string input_pref;
                    for (size_t i = 0; i < parts.size() - 1; i++) input_pref += parts[i] + ' ';

                    if (has_ws_suffix) input_pref += last_arg + ' ';
                    shell_env.ac_prefix = input_pref;
                }
            }
            shell_env.ac_word_suggestions_cursor = 0;
        } else {
            shell_env.ac_word_suggestions_cursor =
                (shell_env.ac_word_suggestions_cursor + 1) % shell_env.ac_word_suggestions.size();
        }

        if (!shell_env.ac_word_suggestions.empty()) {
            shell_env.input_set(
                shell_env.ac_prefix
                + shell_env.ac_word_suggestions[shell_env.ac_word_suggestions_cursor]);
            shell_env.ac_used = true;
        }
    }

    void register_hotkey_actions(Environment& shell_env) {
        // Arrow up
        shell_env.action_table[Ember::VirtualKey::build(4, 15, false)] = &command_history_scroll_up;
        // Arrow down
        shell_env.action_table[Ember::VirtualKey::build(5, 15, false)] =
            &command_history_scroll_down;
        // Arrow left
        shell_env.action_table[Ember::VirtualKey::build(5, 14, false)] = &cursor_move_left;
        // Arrow right
        shell_env.action_table[Ember::VirtualKey::build(5, 16, false)] = &cursor_move_right;
        // Entf
        shell_env.action_table[Ember::VirtualKey::build(3, 14, false)] = &delete_forward;
        // Tab
        shell_env.action_table[Ember::VirtualKey::build(2, 0, false)] = &perform_auto_completion;
    }
} // namespace Crucible
