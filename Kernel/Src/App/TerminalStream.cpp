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

#include <App/TerminalStream.h>

#include <Hammer/Math.h>

#include <CPU/Time/Timer.h>

#include <Memory/Paging.h>

#include <CPU/E9Stream.h>


namespace Rune::App {

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Cursor Renderer
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    int render_cursor(int argc, char* argv[]) {
        if (argc != 1)
            return -1;
        uintptr_t ptr = 0;
        if (!parse_int<uintptr_t>(argv[0], 16, ptr))
            return -1;
        auto* state = (TerminalState*) ptr;
        double thickness = 1.0;
        while (state->keep_rendering_cursor) {
            state->mutex->lock();
            if (!state->timeout_cursor_renderer) {
                int screen_line = state->cursor_sbb.line - state->viewport;
                if (0 <= screen_line && screen_line < state->screen_height && !state->scroll_back_buffer.is_empty()) {
                    U32         x       = (U32) state->cursor_sbb.column * state->font->pixel_width;
                    U32         y_start = (U32) screen_line * state->font->pixel_height;
                    U32         y_end   = y_start + state->font->pixel_height;
                    LibK::Pixel c       = state->is_cursor_rendered ? state->default_bg_color : state->default_fg_color;
                    state->frame_buffer->draw_line({ x, y_start }, { x, y_end }, c, thickness);
                    state->is_cursor_rendered = !state->is_cursor_rendered;
                }
            } else {
                state->timeout_cursor_renderer = false;
            }
            state->mutex->unlock();
            state->timer->sleep_milli(state->cursor_blink_freq_ms);
        }
        return 0;
    }


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          TextLine
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    TextLine::TextLine() : styled_text(), line_size(0) {
        styled_text.add_back({ });
    }


    void TextLine::append_char(char ch) {
        if (styled_text.is_empty())
            styled_text.add_back({ });
        styled_text.tail()->text += ch;
        line_size++;
    }


    void TextLine::style_raw_text(LibK::Pixel bg_color, LibK::Pixel fg_color) {
        if (styled_text.tail()->text.is_empty())
            return;
        styled_text.tail()->bg_color = bg_color;
        styled_text.tail()->fg_color = fg_color;
        styled_text.add_back({ });
    }


    void TextLine::erase(size_t off, size_t len) {
        if (off == 0) {
            if (len == line_size) {
                // whole line was erased -> just clear the line buffer
                clear();
                append_char('\n');  // Add \n so the empty line is rendered
                return;
            } else {
                // start of line until cursor was erased -> pad the start of the line with spaces
                size_t line_offset = 0;
                for (auto& st: styled_text) {
                    if (line_offset > len)
                        // All erased strings are replaced with spaces
                        break;

                    size_t      num_spaces = min(st.text.size(), len - line_offset);
                    String      spaces;
                    for (size_t i          = 0; i < num_spaces; i++)
                        spaces += ' ';

                    if (num_spaces < st.text.size())
                        // We are in the last text slice and only part of it got deleted -> Add the end of the text
                        st.text = spaces + st.text.substring(num_spaces);
                    else
                        st.text = spaces;
                    line_offset += st.text.size();
                }
            }
        } else {
            // cursor until end of line was erased
            LinkedList<size_t> indices_to_remove;
            size_t             line_offset = 0;
            for (size_t        i           = 0; i < styled_text.size(); i++) {
                auto* st = styled_text[i];
                size_t txt_size = st->text.size();
                if (line_offset < off + len && off < line_offset + txt_size) {
                    if (line_offset < off) {
                        // Only part of the text is erased -> Keep the beginning
                        st->text = st->text.substring(0, off - line_offset);
                        line_size -= txt_size - off + line_offset;
                    } else {
                        indices_to_remove.add_back(i);
                        line_size -= txt_size;
                    }
                }
                line_offset += txt_size;
            }
            for (auto& index: indices_to_remove)
                styled_text.remove_at(index);
        }
    }


    void TextLine::clear() {
        styled_text.clear();
        line_size = 0;
        styled_text.add_back({ });
    }


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Text Buffering Functions
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    TextLine* TerminalStream::scroll_back_buffer_get_last_line() {
        if (_state.scroll_back_buffer.is_empty())
            _state.scroll_back_buffer.add_back({ });
        return _state.scroll_back_buffer.tail();
    }


    void TerminalStream::scroll_back_buffer_append_new_line() {
        if (_state.scroll_back_buffer.is_empty()) {
            // The scroll back buffer is empty -> Add the first line
            _state.scroll_back_buffer.add_back({ });
            return;
        }
        // Ditch the oldest entries to make space
        while (_state.scroll_back_buffer.size() >= SCROLL_BACK_BUFFER_LIMIT)
            _state.scroll_back_buffer.remove_front();

        // The newest lines are at the back, this makes the scroll back buffer chronologically ordered
        _state.scroll_back_buffer.tail()->style_raw_text(_state.bg_color, _state.fg_color);
        _state.scroll_back_buffer.add_back({ });
    }


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Render Functions
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    void TerminalStream::scroll_back(int lines) {
        _state.mutex->lock();
        if (lines == 0)
            return;

        U64 terminal_line_pixels = _state.frame_buffer->get_pitch() * _state.font->pixel_height;
        int render_line_begin    = 0;
        int render_line_end      = 0;
        if (lines > 0) {
            // Scroll down
            // We limit the scroll down so that the end of the viewport cannot go past the last line in the scroll back
            // buffer
            //   line 0
            // ---------- <-- viewport start
            // | line 1 |
            // | line 2 |
            // | line 3 | <-- view port end - The end of the viewport is not allowed to go past this line
            // ----------
            int new_viewport = min(
                    _state.viewport + lines,
                    (int) _state.scroll_back_buffer.size() - _state.screen_height
            );
            int scroll_dist  = abs(new_viewport - _state.viewport);
            _state.viewport = new_viewport;

            if (scroll_dist >= _state.screen_height) {
                // Scrolled past the whole current screen -> Just clear the whole screen
                memset(
                        (void*) ((uintptr_t) _state.frame_buffer->get_address()),
                        0,
                        _state.screen_height * terminal_line_pixels
                );
            } else {
                // Move the rendered lines by "lines" lines up
                // "first line"             "second line"
                // "second line"    --->    "third line"
                // "third line"             "third line"
                memcpy(
                        (void*) ((uintptr_t) _state.frame_buffer->get_address()),
                        (void*) ((uintptr_t) _state.frame_buffer->get_address() + terminal_line_pixels * scroll_dist),
                        (_state.screen_height - scroll_dist) * terminal_line_pixels
                );
                // Clear the last "lines" lines
                // "second line"            "second line"
                // "third line"     --->    "third line"
                // "third line"
                memset(
                        (void*) ((uintptr_t) _state.frame_buffer
                                                   ->get_address() + (_state.screen_height - scroll_dist) * terminal_line_pixels),
                        0,
                        scroll_dist * terminal_line_pixels
                );
            }

            // Render the last "scroll_dist" lines in the scroll back buffer
            // "first line"             "first line"
            // "second line"   --->     "second line"
            //                          "some buffered line"
            render_line_begin = scroll_dist < _state.screen_height
                                ? _state.viewport + (_state.screen_height - scroll_dist)
                                : _state.viewport;
            render_line_end   = min(_state.viewport + _state.screen_height, (int) _state.scroll_back_buffer.size());
        } else {
            // Scroll up
            if (_state.viewport == 0)
                // We are already scrolled up to the beginning of the scroll back buffer
                return;
            lines *= -1;    // The math is based on the positive value of lines
            int new_viewport = _state.viewport >= lines ? _state.viewport - lines : 0;
            int scroll_dist  = abs(new_viewport - _state.viewport);
            _state.viewport = new_viewport;

            if (scroll_dist >= _state.screen_height) {
                // Scrolled past the whole current screen -> Just clear the whole screen
                memset(
                        (void*) ((uintptr_t) _state.frame_buffer->get_address()),
                        0,
                        _state.screen_height * terminal_line_pixels
                );
            } else {
                // Scrolled past a portion of the screen
                // Move the rendered lines by "lines" lines down
                // "first line"             "second line"
                // "second line"    --->    "first line"
                // "third line"             "second line"
                memmove(
                        (void*) ((uintptr_t) _state.frame_buffer->get_address() + terminal_line_pixels * scroll_dist),
                        (void*) ((uintptr_t) _state.frame_buffer->get_address()),
                        (_state.screen_height - scroll_dist) * terminal_line_pixels
                );
                // Clear the first "lines" lines
                // "second line"
                // "first line"    --->     "first line"
                // "second line"            "second line"
                memset(
                        (void*) ((uintptr_t) _state.frame_buffer->get_address()),
                        0,
                        scroll_dist * terminal_line_pixels
                );
            }

            // Render the first "scroll_dist" lines in the scroll back buffer
            //                          "some buffered line"
            // "first line"    --->     "first line"
            // "second line"            "second line"
            render_line_begin = _state.viewport;
            render_line_end   = min(_state.viewport + scroll_dist, (int) _state.scroll_back_buffer.size());
        }

        // Render the missing lines on the screen from the scroll back buffer
        for (int i = render_line_begin; i < render_line_end; i++) {
            size_t y = i - _state.viewport;
            size_t x = 0;
            for (auto& st: _state.scroll_back_buffer[i]->styled_text) {
                for (size_t xx = 0; xx < st.text.size(); xx++)
                    draw_char(st.text[xx], x + xx, y, st.bg_color, st.fg_color);
                x += st.text.size();
            }
        }
        _state.mutex->unlock();
    }


    void TerminalStream::draw_char(char ch) {
        _state.mutex->lock();
        start_cursor_movement();
        _state.frame_buffer->draw_glyph(
                _state.font,
                _state.cursor_sbb.column * _state.font->pixel_width,
                (_state.cursor_sbb.line - _state.viewport) * _state.font->pixel_height,
                _state.bg_color,
                _state.fg_color,
                ch
        );
        _state.cursor_sbb.column++;
        if (_state.cursor_sbb.column >= _state.screen_width) {
            _state.cursor_sbb.column = 0;
            _state.cursor_sbb.line++;
        }
        end_cursor_movement();
        _state.mutex->unlock();
    }


    void TerminalStream::draw_char(char ch, U16 x, U16 y, LibK::Pixel bg_color, LibK::Pixel fg_color) const {
        _state.mutex->lock();
        _state.frame_buffer->draw_glyph(
                _state.font,
                x * _state.font->pixel_width,
                y * _state.font->pixel_height,
                bg_color,
                fg_color,
                ch
        );
        _state.mutex->unlock();
    }


    void TerminalStream::draw_cursor(const LibK::Pixel& color) const {
        // The cursor will be drawn in the first column of the glyph is, this is fine for most glyphs as they are padded
        // by at least on pixel and for the other case we do not care atm. In the pic the cursor is donated by the "C"
        // (yeah you guessed it right :)
        //     C
        // |---|---|
        // |   |   |
        // |---|---|
        //
        // Adding explicit space in between glyphs makes them fare away and looks ugly that's why this solution is fine
        // enough
        U32 x         = (U32) _state.cursor_sbb.column * _state.font->pixel_width;
        U32 y_start   = (U32) (_state.cursor_sbb.line - _state.viewport) * _state.font->pixel_height;
        U32 y_end     = y_start + _state.font->pixel_height;
        int thickness = 1.0;
        _state.frame_buffer->draw_line({ x, y_start }, { x, y_end }, color, thickness);
    }


    void TerminalStream::start_cursor_movement() {
        if (_state.is_cursor_rendered)
            // Clear the cursor at the current location
            draw_cursor(_state.default_bg_color);
    }


    void TerminalStream::end_cursor_movement() {
        // Draw it at the new position
        draw_cursor(_state.default_fg_color);
        _state.is_cursor_rendered      = true;
        _state.timeout_cursor_renderer = true;
    }


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Cursor Functions
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    bool TerminalStream::is_cursor_visible() const {
        int screen_line = _state.cursor_sbb.line - _state.viewport;
        return 0 <= screen_line && screen_line < _state.screen_height;
    }


    void TerminalStream::scroll_to_cursor() {
        if (!is_cursor_visible()) {
            int scroll_direction = _state.cursor_sbb.line >= _state.viewport ? 1 : -1;
            int scroll_dist      = scroll_direction >= 0
                                   ? _state.cursor_sbb.line - (_state.viewport + _state.screen_height) + 1
                                   : _state.viewport - _state.cursor_sbb.line;
            scroll_back(scroll_direction * scroll_dist);
        }
    }


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          ANSI Interpreter Functions
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    bool TerminalStream::is_csi_command_selector(char ch) {
        return ch == 'A'
               || ch == 'B'
               || ch == 'C'
               || ch == 'D'
               || ch == 'H'
               || ch == 'J'
               || ch == 'K'
               || ch == 'S'
               || ch == 'T'
               || ch == 'm';
    }


    U8 TerminalStream::parse_csi_arg() {
        int val           = 0;
        int power_ten[]   = { 100, 10, 1 };
        // _digit_buf_offset is filled from left to right, we parse from right to left
        // If the number has three digits then "digit * powerTen[pos]" is its actual numerical value
        // but if the number has less then we need "digit * powerTen[pos + diffToPTen]" where diffToPTen
        // is the index offset to get the correct power of ten
        // e.g. [3, 8, 0] aka the decimal value of 38 -> we need to add diffToPen=1 to pos to get the correct
        // power of ten for the digits
        U8  diff_to_p_ten = DIGIT_BUF_SIZE - _digit_buf_offset;

        // pos  = DIGIT_BUF_SIZE - 1 - diffToPTen
        //      = DIGIT_BUF_SIZE - 1 - (DIGIT_BUF_SIZE - _digit_buf_offset)
        //      = DIGIT_BUF_SIZE - 1 - DIGIT_BUF_SIZE + _digit_buf_offset
        //      = - 1 + _digit_buf_offset
        //      = _digit_buf_offset - 1
        int pos = _digit_buf_offset - 1;
        while (pos >= 0) {
            U8 digit = _digit_buf[pos] > 0 ? ((int) _digit_buf[pos] - 0x30) : 0;
            val += digit * power_ten[pos + diff_to_p_ten];
            pos--;
        }
        // Only bytes allowed here!
        if (val > 255)
            val = 255;

        memset(_digit_buf, 0, DIGIT_BUF_SIZE);
        _digit_buf_offset = 0;
        return val;
    }


    void TerminalStream::exec_csi_command() {
        _state.mutex->lock();
        switch (_csi_cmd_selector) {
            case 'm':
                // First append raw text buffer content with current color
                scroll_back_buffer_get_last_line()->style_raw_text(_state.bg_color, _state.fg_color);
                if (_csi_argv[0] == 38 && _csi_argv[1] == 2) {
                    // Change foreground color
                    _state.fg_color = { _csi_argv[2], _csi_argv[3], _csi_argv[4] };
                } else if (_csi_argv[0] == 48 && _csi_argv[1] == 2) {
                    // Change background color
                    _state.bg_color = { _csi_argv[2], _csi_argv[3], _csi_argv[4] };
                } else if (_csi_argv[0] == 0) {
                    // Reset all render settings
                    _state.fg_color = _state.default_fg_color;
                    _state.bg_color = _state.default_bg_color;
                }
                break;
            case 'A': {
                int steps = _csi_argv[0];
                if (steps == 0)
                    steps = 1;

                scroll_to_cursor();
                start_cursor_movement();
                int screen_line = _state.cursor_sbb.line - _state.viewport;
                // Make the bounds check relative and calculation relative to the screen but store the scroll back
                // buffer line in the cursor
                _state.cursor_sbb.line = (screen_line >= steps ? screen_line - steps + _state.viewport : 0);
                end_cursor_movement();
                break;
            }
            case 'B': {
                int steps = _csi_argv[0];
                if (steps == 0)
                    steps = 1;

                scroll_to_cursor();
                start_cursor_movement();
                int screen_line = _state.cursor_sbb.line - _state.viewport;
                _state.cursor_sbb.line = (steps < (_state.screen_height - screen_line)
                                          ? screen_line + steps
                                          : _state.screen_height - 1) + _state.viewport;
                end_cursor_movement();
                break;
            }
            case 'C': {
                int steps = _csi_argv[0];
                if (steps == 0)
                    steps = 1;

                start_cursor_movement();
                _state.cursor_sbb.column = steps < (_state.screen_width - _state.cursor_sbb.column)
                                           ? _state.cursor_sbb.column + steps
                                           : _state.screen_width - 1;
                end_cursor_movement();
                break;
            }
            case 'D': {
                int steps = _csi_argv[0];
                if (steps == 0)
                    steps = 1;

                start_cursor_movement();
                _state.cursor_sbb.column = _state.cursor_sbb.column >= steps ? _state.cursor_sbb.column - steps : 0;
                end_cursor_movement();
                break;
            }
            case 'H': {
                start_cursor_movement();
                int row                        = _csi_argc < 2 ? 0 : (_csi_argv[1] - 1);   // Arg starts from 1
                if (row < 0)
                    row = 0;
                else if (row >= _state.screen_height)
                    row = _state.screen_height - 1;
                _state.cursor_sbb.line = row + _state.viewport;

                int col = _csi_argc < 1 ? 0 : (_csi_argv[0] - 1);   // Arg starts from 1
                if (col < 0)
                    col = 0;
                else if (col >= _state.screen_width)
                    col = _state.screen_width - 1;
                _state.cursor_sbb.column = col;
                end_cursor_movement();
                break;
            }
            case 'J': {
                if (_state.scroll_back_buffer.is_empty())
                    break;

                U8     del_op                   = _csi_argv[0];
                size_t scroll_back_buffer_start = 0;
                size_t scroll_back_buffer_end   = 0;
                size_t x_start                  = 0;
                size_t x_end                    = 0;
                bool   clear_scroll_back_buffer = false;
                TextLine* cursor_line = _state.scroll_back_buffer[_state.cursor_sbb.line];
                if (del_op == 0) {
                    // Clear from cursor to end of display
                    scroll_back_buffer_start = _state.cursor_sbb.line + 1;
                    scroll_back_buffer_end   = min((int) _state.scroll_back_buffer.size(),
                                                   _state.viewport + _state.screen_height
                    );
                    x_start                  = _state.cursor_sbb.column;
                    x_end                    = cursor_line->line_size;
                } else if (del_op == 1) {
                    // Clear from start of display to cursor
                    scroll_back_buffer_start = _state.viewport;
                    scroll_back_buffer_end   = _state.cursor_sbb.line;
                    x_start                  = 0;
                    x_end                    = _state.cursor_sbb.column;
                } else if (del_op == 2) {
                    // Clear whole screen
                    scroll_back_buffer_start = _state.viewport;
                    scroll_back_buffer_end   = min((int) _state.scroll_back_buffer.size(),
                                                   _state.viewport + _state.screen_height
                    );
                } else if (del_op == 3) {
                    // Clear whole screen and clear the scroll back buffer
                    scroll_back_buffer_start = _state.viewport;
                    scroll_back_buffer_end   = min((int) _state.scroll_back_buffer.size(),
                                                   _state.viewport + _state.screen_height
                    );
                    clear_scroll_back_buffer = true;
                }

                scroll_to_cursor();
                for (size_t line_num = scroll_back_buffer_start; line_num < scroll_back_buffer_end; line_num++) {
                    TextLine* text_line = _state.scroll_back_buffer[line_num];
                    for (size_t x = 0; x < text_line->line_size; x++)
                        draw_char(' ', x, line_num - _state.viewport, _state.bg_color, _state.fg_color);
                    if (!clear_scroll_back_buffer) {
                        text_line->clear();
                        text_line->append_char('\n');
                    }
                }
                for (size_t x        = x_start; x < x_end; x++)
                    draw_char(' ', x, _state.cursor_sbb.line - _state.viewport, _state.bg_color, _state.fg_color);

                if (clear_scroll_back_buffer)
                    // Clear the scroll back and line buffer
                    _state.scroll_back_buffer.clear();
                else
                    // Erase part of the line where the cursor is
                    // Does nothing in case of del_op 2 or 3 because x_start = x_end = 0
                    cursor_line->erase(x_start, x_end - x_start);
                break;
            }
            case 'K': {
                if (_state.scroll_back_buffer.is_empty())
                    break;

                U8     del_op  = _csi_argv[0];
                size_t x_start = 0;
                size_t x_end   = 0;
                TextLine* text_line = _state.scroll_back_buffer[_state.cursor_sbb.line];
                if (del_op == 0) {
                    x_start = _state.cursor_sbb.column;
                    x_end   = text_line->line_size;
                } else if (del_op == 1) {
                    x_start = 0;
                    x_end   = _state.cursor_sbb.column;
                } else if (del_op == 2) {
                    x_start = 0;
                    x_end   = text_line->line_size;
                }
                scroll_to_cursor();
                for (size_t x = x_start; x < x_end; x++)
                    draw_char(' ', x, _state.cursor_sbb.line - _state.viewport, _state.bg_color, _state.fg_color);
                text_line->erase(x_start, x_end - x_start);
                break;
            }
            case 'S': {
                U8 scroll_amount  = _csi_argv[0];
                if (scroll_amount == 0)
                    scroll_amount = 1;
                scroll_back(-scroll_amount);
                break;
            }
            case 'T': {
                U8 scroll_amount  = _csi_argv[0];
                if (scroll_amount == 0)
                    scroll_amount = 1;
                scroll_back(scroll_amount);
                break;
            }
            default:
                // Do nothing
                break;
        }

        // Reset CSI args and command selector
        memset(_csi_argv, 0, CSI_ARGV_BUF_SIZE);
        _csi_argc         = 0;
        _csi_cmd_selector = '\0';
        _state.mutex->unlock();
    }


    bool TerminalStream::interpret_char(char ch) {
        switch (_interpreter_state) {
            case ANSIInterpreterState::CHARACTER:
                if (ch == ESC) {
                    _interpreter_state = ANSIInterpreterState::CSI_BEGIN;
                    return true;
                } else {
                    // Parse a C0 control code
                    bool ret = false;
                    _state.mutex->lock();
                    switch (ch) {
                        case '\b':
                            start_cursor_movement();
                            if (_state.cursor_sbb.column > 0)
                                // Underflow protection -> Do not decrement if x == 0!
                                _state.cursor_sbb.column--;
                            end_cursor_movement();
                            ret = true;
                            break;
                        case '\t': {
                            int      spaces = TAB_STOP - (_state.cursor_sbb.column % TAB_STOP);
                            for (int i      = 0; i < spaces; i++) {
                                scroll_back_buffer_get_last_line()->append_char(' ');
                                draw_char(' ');
                            }
                            ret = true;
                            break;
                        }
                        case '\n':
                            start_cursor_movement();
                            scroll_back_buffer_append_new_line();
                            _state.cursor_sbb.line++;
                            _state.cursor_sbb.column = 0;
                            if (_state.cursor_sbb.line - _state.viewport == _state.screen_height)
                                scroll_back(1);
                            end_cursor_movement();
                            ret = true;
                            break;
                        case '\r':
                            start_cursor_movement();
                            _state.cursor_sbb.column = 0;
                            scroll_back_buffer_get_last_line()->clear();
                            end_cursor_movement();
                            ret = true;
                            break;
                        default:
                            break;
                    }
                    _state.mutex->unlock();
                    return ret;
                }
            case ANSIInterpreterState::CSI_BEGIN:
                if (ch == '[') {
                    _interpreter_state = ANSIInterpreterState::CSI_ARG;
                    return true;
                } else {
                    // Found ESC but CSI missing -> Start printing again
                    _interpreter_state = ANSIInterpreterState::CHARACTER;
                    return false;
                }
            case ANSIInterpreterState::CSI_ARG:
                if (0x30 <= ch && ch <= 0x39) {
                    // Try parse a digit
                    if (_digit_buf_offset < DIGIT_BUF_SIZE && _csi_argc < CSI_ARGV_BUF_SIZE) {
                        // We have enough space in the digit buf and the CSI argument buf
                        _digit_buf[_digit_buf_offset++] = ch;
                        return true;
                    } else {
                        // Argument too long or too many arguments -> Start printing again
                        _interpreter_state = ANSIInterpreterState::CHARACTER;
                        return false;
                    }
                } else if (ch == ';' || is_csi_command_selector(ch)) {
                    // Either end of an argument or end of the CSI command
                    // Convert string to U8
                    if (_csi_argc < CSI_ARGV_BUF_SIZE)
                        _csi_argv[_csi_argc++] = parse_csi_arg();

                    if (is_csi_command_selector(ch)) {
                        // Exec it aka modify the render settings
                        _csi_cmd_selector = ch;
                        exec_csi_command();
                        _interpreter_state = ANSIInterpreterState::CHARACTER;
                    } // else -> Parse the next CSI argument
                    return true;
                } else {
                    // Unexpected char found -> Start printing again
                    _interpreter_state = ANSIInterpreterState::CHARACTER;
                    return false;
                }
            default:
                // Default should never be reached, but just in case
                _interpreter_state = ANSIInterpreterState::CHARACTER;
                return false;
        }
    }


    TerminalStream::TerminalStream(
            CPU::Subsystem* cpu_subsys,
            LibK::FrameBuffer* frame_buffer,
            LibK::BitMapFont* font,
            LibK::Pixel def_bg_color,
            LibK::Pixel def_fg_color
    ) : _cpu_subsys(cpu_subsys),
        _state(),
        _render_thread_handle(0),
        _render_thread_arg(""),
        _render_thread_argv(),
        _initialized(false),
        _interpreter_state(ANSIInterpreterState::CHARACTER),
        _csi_argv(),
        _csi_argc(0),
        _csi_cmd_selector('\0'),
        _digit_buf(),
        _digit_buf_offset(0) {

        _state.frame_buffer         = frame_buffer;
        _state.font                 = font;
        _state.default_bg_color     = move(def_bg_color);
        _state.default_fg_color     = move(def_fg_color);
        _state.bg_color             = _state.default_bg_color;
        _state.fg_color             = _state.default_fg_color;
        _state.screen_width         = (int) (frame_buffer->get_width() / font->pixel_width);
        _state.screen_height        = (int) (frame_buffer->get_height() / font->pixel_height);
        _state.cursor_blink_freq_ms = CURSOR_BLINK_FREQ;

        _state.timer = cpu_subsys->get_system_timer();
        _state.mutex = cpu_subsys->create_mutex("Terminal");

        // The arguments to the cursor render thread have to be maintained until the thread actually is running
        // else they are stack allocated and gone after the constructor is finished and then boom
        _render_thread_arg = int_to_string((uintptr_t) &_state, 16);
        _render_thread_argv[0] = (char*) _render_thread_arg.to_cstr();
        _render_thread_argv[1] = nullptr;
        if (_state.mutex)
            _initialized = true;
    }


    bool TerminalStream::is_read_supported() {
        return false;
    }


    int TerminalStream::read() {
        return -1;
    }


    bool TerminalStream::is_write_supported() {
        return true;
    }


    bool TerminalStream::write(U8 value) {
        if (!_initialized)
            return false;

        if (_render_thread_handle == 0) {
            _cpu_subsys->get_scheduler()->lock();
            _render_thread_handle = _cpu_subsys->schedule_new_thread(
                    "Terminal-Cursor Render Thread",
                    &render_cursor,
                    1,
                    _render_thread_argv,
                    Memory::get_base_page_table_address(),
                    CPU::SchedulingPolicy::LOW_LATENCY,
                    0x0
            );
            if (_render_thread_handle == 0)
                _initialized      = false;
            _cpu_subsys->get_scheduler()->unlock();
        }

        char ch = (char) value;
        if (!interpret_char(ch) && ch != '\0') {
            draw_char(ch);
            scroll_back_buffer_get_last_line()->append_char(ch);
        }
        return true;
    }


    void TerminalStream::flush() {
        // No buffering is used
    }


    void TerminalStream::close() {
        // No resources to free
    }


    bool TerminalStream::is_ansi_supported() {
        return true;
    }
}