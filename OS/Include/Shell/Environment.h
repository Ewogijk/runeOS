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

#ifndef RUNEOS_ENVIRONMENT_H
#define RUNEOS_ENVIRONMENT_H


#include <Hammer/String.h>
#include <Hammer/Collection.h>
#include <Hammer/Path.h>

#include <Shell/AutoCompletion.h>

#include <Pickaxe/AppManagement.h>


namespace Rune {
    // Add hash support for virtual keys.
    template<>
    struct Hash<Pickaxe::VirtualKey> {
        Hash& operator=(Hash&& o) = default;


        Hash& operator=(const Hash& o) = default;


        size_t operator()(const Pickaxe::VirtualKey& key) const {
            return 37 * key.get_key_code();
        }
    };
}


namespace Rune::Shell {
    struct Environment;

    /**
     * @brief An action to execute when a hotkey is pressed.
     */
    using Action = Function<void(Environment&)>;


    /**
     * @brief The shell environment provides the environment variables and built-in commands.
     */
    struct Environment {
        static constexpr U8 INPUT_BUFFER_LIMIT = 128;

        static const String PATH;

        /**
         * @brief All currently defined environment variables in the shell.
         */
        HashMap<String, String>                                   env_var_table = HashMap<String, String>();

        /**
         * @brief Contains all built-in commands of the shell.
         */
        HashMap<String, Function<int(int, char**, Environment&)>> command_table = HashMap<String, Function<int(
                int,
                char**,
                Environment&
        )>>();

        /**
         * @brief Contains all actions bound to non-ascii key presses e.g arrow up.
         */
        HashMap<Pickaxe::VirtualKey, Action> action_table;

        /**
         * More precisely whenever the user writes a new line character (by pressing "enter") the content of the input
         * buffer will be appended to this list.
         *
         * @brief The command history contains all user input that has been executed (or tried to).
         */
        LinkedList<String> command_history = LinkedList<String>();

        /**
         * If command_history_cursor >= command_history.size() -> The input buffer shall be displayed.
         *
         * @brief The currently displayed command from the command history.
         */
        size_t command_history_cursor = 0;

        /**
         * @brief A backup of the input buffer, when the user starts scrolling the command history.
         */
        String input_buffer_backup;

        /**
         * @brief A buffer for the input the user is writing currently aka the last line in the terminal.
         */
        char   input_buffer[INPUT_BUFFER_LIMIT];
        U8     input_buffer_size   = 0;            // Number of characters in the buffer
        size_t input_buffer_cursor = 0;            // Cursor position in the buffer


        /**
         * @brief Auto completion support for the shell.
         */
        AutoCompletion     auto_completion;
        String             ac_prefix;
        bool               ac_used   = false;
        LinkedList<String> ac_word_suggestions;
        size_t             ac_word_suggestions_cursor = 0;


        /**
         * @brief A user space copy of the shell working directory to minimize system calls.
         */
        Path working_directory = Path("");


        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          Input Buffer Functions
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


        /**
         * If the cursor is positioned in the middle of the input buffer, following characters will be shifted by one
         * position to the right.
         *
         * @brief Append the character to the input buffer at the cursor position.
         * @param ch
         */
        void input_append(char ch);


        /**
         * @brief Delete the char at the cursor position.
         * @param forward True: Delete to the right of the cursor, False: Delete to the left.
         */
        void input_delete(bool forward);


        /**
         * @brief Clear the input buffer and if requested also erase the input on the display.
         * @param erase_on_display True: Erase the input on the display, False: Do not touch the display.
         */
        void input_delete_all(bool erase_on_display);


        /**
         * @brief Delete the input buffer, append the string to it.
         * @param str New input buffer content.
         */
        void input_set(const String& str);
    };
}

#endif //RUNEOS_ENVIRONMENT_H
