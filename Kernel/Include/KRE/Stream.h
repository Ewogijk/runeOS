
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

#ifndef RUNEOS_STREAM_H
#define RUNEOS_STREAM_H

#include <KRE/String.h>
#include <KRE/Utility.h>

#include <KRE/Collections/Array.h>

namespace Rune {
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      Stream API
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * @brief A Stream is an abstraction of any source of bytes that can be read from, written to or
     * both. Examples for streams are files, the keyboard, monitor, sockets, etc.
     */
    class Stream {
      public:
        virtual ~Stream() = default;

        /**
         * @brief
         * @return True: The stream supports reading, False: It does not.
         */
        virtual auto is_read_supported() -> bool = 0;

        /**
         * @brief Read a single byte from the stream.
         * @return -1: The stream has no byte left, Else: A byte from the stream.
         */
        virtual auto read() -> int = 0;

        /**
         * @brief Read at most size bytes at the given offset into the buffer.
         * @param buffer
         * @param offset
         * @param size
         * @return The number of bytes that have been read from the stream.
         */
        template <size_t N>
        auto read(Array<U8, N> buffer, size_t offset) -> size_t {
            size_t bytes_read = 0;
            while (bytes_read < buffer.size()) {
                const int byte = read();
                if (byte < 0) break;
                buffer[offset + bytes_read] = byte;
                bytes_read++;
            }
            return bytes_read;
        }

        /**
         * @brief Read at most size bytes to the beginning of the buffer.
         * @param buffer
         * @param size
         * @return The number of bytes that have been read from the stream.
         */
        template <size_t N>
        auto read(Array<U8, N> buffer) -> size_t {
            return read(buffer, 0);
        }

        /**
         * @brief
         * @return True: The stream supports writing, False: It does not.
         */
        virtual auto is_write_supported() -> bool = 0;

        /**
         * @brief Write a single byte to the stream.
         * @param value
         * @return True: The byte was written, False: It was not.
         */
        virtual auto write(U8 value) -> bool = 0;

        /**
         * @brief Write size number of bytes from the offset in the buffer to the stream.
         * @param buffer
         * @param offset
         * @param size
         * @return The number of bytes written.
         */
        template <size_t N>
        auto write(Array<U8, N> buffer, size_t offset) -> size_t {
            size_t bytes_written = 0;
            while (bytes_written < buffer.size()) {
                if (!write(buffer[offset + bytes_written])) break;
                bytes_written++;
            }
            return bytes_written;
        }
        /**
         * @brief Write size number of bytes from the beginning of the buffer to the stream.
         * @param buffer
         * @param size
         * @return The number of bytes written.
         */
        template <size_t N>
        auto write(Array<U8, N> buffer) -> size_t {
            return write(buffer, 0);
        }

        /**
         * @brief If the stream supports buffering, write any bytes in the buffer immediately to the
         * stream.
         */
        virtual void flush() = 0;

        /**
         * @brief free any resources associated with the stream. After a stream has been closed it
         * is no longer possible to read or write any bytes.
         */
        virtual void close() = 0;
    };

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      Text Stream API
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * <p>
     *  Note that while the TextStream interface supports writing ANSI "Select Graphic Rendition
     * (SGR)" commands, the TextStream implementation may not implement ANSI parsing. In this case
     * the ANSI escape codes will be directly sent to the destination.
     * </p>
     * <p>
     *  The text stream supports the following SGR commands:
     *  <ol>
     *      <li>Reset</li>
     *      <li>Set foreground color</li>
     *      <li>Set background color</li>
     *  </ol>
     * </p>
     *
     * @brief A TextStream is an extension of the Stream API that allows formatted and styled output
     * to a stream.
     */
    class TextStream : public Stream {
        static constexpr size_t BUF_SIZE = 4096;
        /**
         * @brief Formatted strings are temporarily stored in this buffer.
         */
        // char _formatted_buf[BUF_SIZE];
        Array<char, BUF_SIZE> _formatted_buf;

        /**
         * @brief Zero out _formattedBuf.
         */
        void clear_buf();

      public:
        using Stream::write;

        explicit TextStream();

        ~TextStream() override = default;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                      Stream API Overrides
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        auto is_read_supported() -> bool override = 0;

        auto read() -> int override = 0;

        auto is_write_supported() -> bool override = 0;

        auto write(U8 value) -> bool override = 0;

        void flush() override = 0;

        void close() override = 0;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                      Stream API Extension
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        /**
         * @brief Write the string to the stream.
         * @param msg
         * @return The number of characters written.
         */
        auto write(const String& msg) -> size_t;

        /**
         * @brief Write the string followed by a new line character to the stream.
         * @param msg
         * @return The number of characters written.
         */
        auto write_line(const String& msg) -> size_t;

        /**
         * @brief First format the string then write it to the stream.
         * @param format  Template string.
         * @param arg_list Array of arguments.
         * @param arg_size Size of the argument array.
         * @return The number of characters written.
         */
        auto write_formatted(const String& format, Argument* arg_list, size_t arg_size) -> size_t;

        /**
         * @brief First format the string then write it to the stream.
         * @param fmt Template string.
         * @param args   Varargs of template arguments.
         * @return The number of characters written.
         */
        template <typename... Args>
        auto write_formatted(const String& fmt, Args... args) -> size_t {
            Argument arg_array[] = {args...}; // NOLINT size is dynamic
            interpolate(fmt.to_cstr(), _formatted_buf.data(), BUF_SIZE, arg_array, sizeof...(Args));
            size_t out = write(_formatted_buf.data());
            clear_buf();
            return out;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                      ANSI Support
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        /**
         * @brief True: This text stream supports ANSI escape codes, False: It does not.
         */
        virtual auto is_ansi_supported() -> bool = 0;

        /**
         * @brief Set the background color.
         * @param color
         */
        void set_background_color(const Pixel& color);

        /**
         * @brief Set the foreground color, that is the color of the glyphs.
         * @param color
         */
        void set_foreground_color(const Pixel& color);

        /**
         * @brief Reset all style attributes to their default values.
         */
        void reset_style();
    };
} // namespace Rune

#endif // RUNEOS_STREAM_H
