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

#include <Crucible/Environment.h>

#include <cstring>
#include <iostream>

namespace Crucible {
    const std::string Environment::PATH = "PATH";

    void Environment::input_append(const char ch) {
        if (input_buffer_size >= INPUT_BUFFER_LIMIT) return; // input buffer is full

        // Append the ch to the input buffer
        const size_t ch_move_count = input_buffer_size - input_buffer_cursor;
        memmove(&input_buffer[input_buffer_cursor + 1],
                &input_buffer[input_buffer_cursor],
                ch_move_count);
        input_buffer[input_buffer_cursor] = ch;
        input_buffer_size++;

        // Update the terminal with the new input buffer content
        if (ch_move_count > 0)
            // The char got appended in the middle of the input -> Erase from cursor to end of line
            std::cout << "\033[0K";

        // Print input buffer starting from position
        std::cout << &input_buffer[input_buffer_cursor];
        input_buffer_cursor++;

        if (ch_move_count > 0)
            // The char got appended in the middle of the input -> the terminal cursor is now at the
            // end of the input
            // -> move it to the input buffer cursor position
            std::cout << "\033[" << (input_buffer_size - input_buffer_cursor) << "D";
        ac_used = false;
        std::cout.flush();
    }

    void Environment::input_delete(const bool forward) {
        if ((input_buffer_cursor == 0 && !forward)
            || (input_buffer_cursor >= input_buffer_size && forward))
            // Delete to left and cursor is at beginning of the input
            //      or
            // Delete to right and cursor is at the end of the input
            // -> Cannot delete
            return;

        if (forward) {
            // Delete to the right of the cursor
            const size_t ch_move_count = input_buffer_size - (input_buffer_cursor + 1);
            // Remove the char from the input buffer, e.g. 12|34 -> 12|44 -> 12|4
            memmove(&input_buffer[input_buffer_cursor],
                    &input_buffer[input_buffer_cursor + 1],
                    ch_move_count);
            input_buffer[--input_buffer_size] = '\0';

            // Update the display
            // Erase from cursor to end of line and redraw the input from the cursor
            std::cout << "\033[0K" << &input_buffer[input_buffer_cursor];
            // Actually want check this: ch_move_count - 1 > 0, But possible underflow!!
            // -> Therefore we do THIS: ch_move_count >= 1 (but need to include 1, since this means
            // need to move the cursor)
            if (ch_move_count >= 1)
                // More characters follow after the deleted char
                // -> move the terminal cursor back to the input cursor position
                std::cout << "\033[" << (input_buffer_size - input_buffer_cursor) << "D";
        } else {
            // Delete to the left of the cursor
            const size_t ch_move_count = input_buffer_size - input_buffer_cursor;
            // Remove the char from the input buffer, e.g. 12|34 -> 13|44 -> 13|4 -> 1|34
            memmove(&input_buffer[input_buffer_cursor - 1],
                    &input_buffer[input_buffer_cursor],
                    ch_move_count);
            input_buffer[--input_buffer_size] = '\0';
            input_buffer_cursor--;

            // Update the display
            // Move the cursor backwards, erase from the cursor to end of line and redraw the input
            // from the cursor
            std::cout << "\b\033[0K" << &input_buffer[input_buffer_cursor];
            if (ch_move_count > 0)
                // More characters follow before the deleted char
                // -> move the terminal cursor back to the input cursor position
                std::cout << "\033[" << (input_buffer_size - input_buffer_cursor) << "D";
        }
        ac_used = false;
        std::cout.flush();
    }

    void Environment::input_delete_all(const bool erase_on_display) {
        if (input_buffer_size == 0) return; // Nothing to delete

        memset(input_buffer.data(), '\0', input_buffer_size);
        if (erase_on_display) {
            if (input_buffer_cursor > 0)
                std::cout << "\033[" << input_buffer_cursor << "D\033[0K";
            else
                std::cout << "\033[0K";
        }
        input_buffer_size   = 0;
        input_buffer_cursor = 0;
        ac_used             = false;
        std::cout.flush();
    }

    void Environment::input_set(const std::string& str) {
        input_delete_all(true);
        for (const char ch : str) input_append(ch);
    }
} // namespace Crucible