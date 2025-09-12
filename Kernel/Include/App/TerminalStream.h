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

#ifndef RUNEOS_TERMINALSTREAM_H
#define RUNEOS_TERMINALSTREAM_H

#include <Ember/Enum.h>
#include <KernelRuntime/Collection.h>
#include <KernelRuntime/String.h>

#include <KernelRuntime/FrameBuffer.h>
#include <KernelRuntime/Stream.h>

#include <CPU/CPUSubsystem.h>

namespace Rune::App {

    /**
     * The line can be either relative to the scroll back buffer or the screen. The column has the same value from both
     * view points.
     *
     * @brief The position of the cursor.
     */
    struct TerminalCursor {
        int line   = 0;
        int column = 0;
    };

    /**
     * @brief A piece of text with style information e.g. color.
     */
    struct StyledText {
        String text     = "";
        Pixel  bg_color = {};
        Pixel  fg_color = {};
    };

    /**
     * @brief A single line of styled text in the terminal.
     */
    struct TextLine {
        // The last entry in the list is the currently entered text
        LinkedList<StyledText> styled_text;
        size_t                 line_size; // Size of the line in characters as if it was a single string

        TextLine();

        /**
         * @brief Append a char to the line buffer.
         * @param ch
         */
        void append_char(char ch);

        /**
         * @brief Append all content of the raw text buffer to the line buffer with the currently used fg and bg colors
         *          then clear the line buffer
         * @param bg_color
         * @param fg_color
         */
        void style_raw_text(Pixel bg_color, Pixel fg_color);

        /**
         * @brief Delete "len" characters starting from "off" as if the text line was a single string.
         * @param off
         * @param len
         */
        void erase(size_t off, size_t len);

        /**
         * @brief Clear the styled_text list and set the line_size=0.
         */
        void clear();
    };

#define ANSI_INTERPRETER_STATES(X)                                                                                     \
    X(ANSIInterpreterState, CHARACTER, 1)                                                                              \
    X(ANSIInterpreterState, C0_CONTROL_CODE, 2)                                                                        \
    X(ANSIInterpreterState, CSI_BEGIN, 3)                                                                              \
    X(ANSIInterpreterState, CSI_ARG, 4)                                                                                \
    X(ANSIInterpreterState, CSI_END, 5)

    DECLARE_ENUM(ANSIInterpreterState, ANSI_INTERPRETER_STATES, 0) // NOLINT

    /**
     * The state is shared with the cursor render thread, a pointer to the state will be passed to it when it is
     * created.
     *
     * @brief The internal state of the terminal.
     */
    struct TerminalState {
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          Text Buffering
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        // Buffer of all text that was ever written to the terminal, the buffer is used to implement scroll back
        // thus not all lines in the buffer will be rendered all the time
        // Optimization note: Should use array list for random access
        LinkedList<TextLine> scroll_back_buffer = LinkedList<TextLine>();
        // Cursor position relative to the scroll back buffer
        TerminalCursor cursor_sbb = {};

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          Rendering
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        FrameBuffer* frame_buffer = nullptr; // Framebuffer of the monitor
        BitMapFont*  font         = nullptr; // Font for glyph rendering

        int screen_width  = 0; // Number of glyphs that fit in a row
        int screen_height = 0; // Number of glyphs that fit in a column

        // Will be used when attributes are reset
        Pixel default_bg_color = {};
        Pixel default_fg_color = {};

        // Foreground and background colors for rendering
        Pixel bg_color = {};
        Pixel fg_color = {};

        // The view port describes the first line that is rendered on the display and is essentially an offset into
        // the scroll back buffer. The view port is limited by the "_screen_height" property therefore it is not defined
        // separately.
        // A visualization of the view port:
        //      ...
        //      Line1     <- Lines are stored in the scroll back buffer
        //  ------------- <- Screen begin
        //  |   Line2   |
        //  |   Line3   |
        //  |   Line4   |
        //  ------------- <- Screen end
        //      Line5
        //      ...
        int viewport = 0;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                    Cursor Renderer Settings
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        CPU::Timer* timer = nullptr; // For sleeping purposes

        // For synchronization between the render thread and others
        SharedPointer<CPU::Mutex> mutex = SharedPointer<CPU::Mutex>();
        // Blink speed of the cursor in milliseconds
        U16 cursor_blink_freq_ms = 0;
        // True: The cursor is visible, False: It is not.
        bool is_cursor_rendered = false;
        // True: The render thread will skip a loop iteration, False: The render thread makes no skips.
        bool timeout_cursor_renderer = false;

        // This flag controls if the cursor rendering thread keeps rendering the cursor, if set to false the render
        // thread will terminate
        bool keep_rendering_cursor = true;
    };

    /**
     * The terminal stream has an integrated ANSI interpreter that interprets incoming characters on the fly.
     * <p>
     *  The formal grammar is:
     *  <ul>
     *  <li>EscapeCode           = C0ControlCode | FEEscapeSequence</li>
     *  <li>FEEscapeSequence     = "\033", "[", CSICommand</li>
     *  <li>CSICommand           = ([0-9], ";")*, CSICommandSelector</li>
     *  <li>CSICommandSelector   = [ABCDHJKSTm]</li>
     *  <li>C0ControlCode        = [\b\t\r\n]</li>
     *  </ul>
     * </p>
     *
     * @brief A terminal emulator that renders bitmap fonts to the framebuffer of a monitor thus providing an text
     *          output for applications.
     */
    class TerminalStream : public TextStream {
        // Maximum size of the scroll back buffer, in case the scroll back buffer gets bigger than the limit the oldest
        // text lines must be discarded
        static constexpr U8 SCROLL_BACK_BUFFER_LIMIT = 128;

        // The amount of time in millis the cursor render thread sleeps before redrawing the cursor aka the blink speed
        static constexpr U16 CURSOR_BLINK_FREQ = 500;

        CPU::CPUSubsystem* _cpu_subsys;
        TerminalState      _state;

        U16            _render_thread_ID;
        String         _render_thread_arg;
        char*          _render_thread_argv[2];
        CPU::StartInfo _render_thread_start_info;

        bool _initialized;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          ANSI Interpreter
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        static constexpr U8 CSI_ARGV_BUF_SIZE = 5;      // Max number of csi args that can be parsed
        static constexpr U8 DIGIT_BUF_SIZE    = 3;      // Max number of digits a csi arg can have
        static constexpr U8 ESC               = '\033'; // csi command escape character
        static constexpr U8 TAB_STOP          = 4;      // Size of a tab

        ANSIInterpreterState _interpreter_state; // Current state of the interpreter

        U8   _csi_argv[CSI_ARGV_BUF_SIZE]; // Buffer for the csi args
        U8   _csi_argc;                    // Number of parsed csi args
        char _csi_cmd_selector;            // The parsed csi command selector

        char _digit_buf[DIGIT_BUF_SIZE]; // Buffer of digits for the currently parsed csi arg
        U8   _digit_buf_offset;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          Text Buffering Functions
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        /**
         * @brief Get the last line in the scroll back buffer.
         * @return
         */
        TextLine* scroll_back_buffer_get_last_line();

        /**
         * @brief Set the style of the last line in the scroll back buffer to the current fg and bg color and append a
         *          new line.
         */
        void scroll_back_buffer_append_new_line();

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          Render Functions
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        void scroll_back(int lines);

        // Draw the char and advance the cursor
        void draw_char(char ch);

        // Draw the char at the requested position without modifying the cursor position
        void draw_char(char ch, U16 x, U16 y, Pixel bg_color, Pixel fg_color) const;

        // Draw the cursor
        void draw_cursor(const Pixel& color) const;

        // If the cursor is rendered, clear it at its current position. Call this before moving the cursor.
        void start_cursor_movement();

        // Render the cursor at its current position. Call this after moving the cursor.
        void end_cursor_movement();

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          Cursor Functions
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        /**
         * @brief
         * @return True: The cursor is visible on the screen, False: It is not.
         */
        [[nodiscard]]
        bool is_cursor_visible() const;

        /**
         * If the cursor is below the viewport it is scrolled until the cursor is in the last line on the screen. If
         * it is above the viewport then scroll until it is the first line.
         *
         * @brief Scroll to the cursor if it is not visible.
         */
        void scroll_to_cursor();

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          ANSI Interpreter Functions
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        static bool is_csi_command_selector(char ch);

        // Parse an U8 from string
        U8 parse_csi_arg();

        // Modify the display render settings base on parsed CSI command
        void exec_csi_command();

        /**
         * @brief Interpret the next char in the stream.
         * @param ch
         * @return True: The char belongs to an ANSI escape sequence -> do not render!,
         *          False: The char is not ANSI -> render it!
         */
        bool interpret_char(char ch);

      public:
        TerminalStream(CPU::CPUSubsystem* cpu_subsys,
                       FrameBuffer*       frame_buffer,
                       BitMapFont*        font,
                       Pixel              def_bg_color,
                       Pixel              def_fg_color);

        using TextStream::write;

        bool is_read_supported() override;

        int read() override;

        bool is_write_supported() override;

        bool write(U8 value) override;

        void flush() override;

        void close() override;

        bool is_ansi_supported() override;
    };
} // namespace Rune::App

#endif // RUNEOS_TERMINALSTREAM_H
