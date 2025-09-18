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

#ifndef RUNEOS_ACTION_H
#define RUNEOS_ACTION_H

#include <Shell/Environment.h>

namespace Rune::Shell {

    /**
     * @brief Scroll up in the command history of the shell.
     * @param shell_env
     */
    void command_history_scroll_up(Environment& shell_env);

    /**
     * @brief Scroll down in the command history of the shell.
     * @param shell_env
     */
    void command_history_scroll_down(Environment& shell_env);

    /**
     * @brief Move the cursor one position to the left.
     * @param shell_env
     */
    void cursor_move_left(Environment& shell_env);

    /**
     * @brief Move the cursor one position to the right.
     * @param shell_env
     */
    void cursor_move_right(Environment& shell_env);

    /**
     * @brief Delete the character to the right of the cursor.
     * @param shell_env
     */
    void delete_forward(Environment& shell_env);

    /**
     * @brief Perform autocompletion on the current shell input.
     * @param shell_env
     */
    void perform_auto_completion(Environment& shell_env);

    /**
     * @brief Add all
     * @param shell_env
     */
    void register_hotkey_actions(Environment& shell_env);
} // namespace Rune::Shell

#endif // RUNEOS_ACTION_H
