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


#include <Hammer/String.h>
#include <Hammer/Memory.h>

#include <LibK/FrameBuffer.h>


namespace Rune::LibK {


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Stream API
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    /**
     * @brief A Stream is an abstraction of any source of bytes that can be read from, written to or both. Examples for
     *          streams are files, the keyboard, monitor, sockets, etc.
     */
    class Stream {
        SharedPointer<Stream> _next;

    public:

        virtual ~Stream() = default;


        /**
         * @brief
         * @return True: The stream supports reading, False: It does not.
         */
        virtual bool is_read_supported() = 0;


        /**
         * @brief Read a single byte from the stream.
         * @return -1: The stream has no byte left, Else: A byte from the stream.
         */
        virtual int read() = 0;


        /**
         * @brief Read at most size bytes at the given offset into the buffer.
         * @param buffer
         * @param offset
         * @param size
         * @return The number of bytes that have been read from the stream.
         */
        size_t read(U8 buffer[], size_t offset, size_t size);


        /**
         * @brief Read at most size bytes to the beginning of the buffer.
         * @param buffer
         * @param size
         * @return The number of bytes that have been read from the stream.
         */
        size_t read(U8 buffer[], size_t size);


        /**
         * @brief
         * @return True: The stream supports writing, False: It does not.
         */
        virtual bool is_write_supported() = 0;


        /**
         * @brief Write a single byte to the stream.
         * @param value
         * @return True: The byte was written, False: It was not.
         */
        virtual bool write(U8 value) = 0;


        /**
         * @brief Write size number of bytes from the offset in the buffer to the stream.
         * @param buffer
         * @param offset
         * @param size
         * @return The number of bytes written.
         */
        size_t write(U8 buffer[], size_t offset, size_t size);


        /**
         * @brief Write size number of bytes from the beginning of the buffer to the stream.
         * @param buffer
         * @param size
         * * @return The number of bytes written.
         */
        size_t write(U8 buffer[], size_t size);


        /**
         * @brief If the stream supports buffering, write any bytes in the buffer immediately to the stream.
         */
        virtual void flush() = 0;


        /**
         * @brief free any resources associated with the stream. After a stream has been closed it is no longer possible
         *          to read or write any bytes.
         */
        virtual void close() = 0;
    };


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Text Stream API
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    /**
     * <p>
     *  Note that while the TextStream interface supports writing ANSI "Select Graphic Rendition (SGR)" commands, the
     *  TextStream implementation may not implement ANSI parsing. In this case the ANSI escape codes will be directly
     *  sent to the destination.
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
     * @brief A TextStream is an extension of the Stream API that allows formatted and styled output to a stream.
     */
    class TextStream : public Stream {
        static constexpr size_t BUF_SIZE = 256;
        /**
         * @brief Formatted strings are temporarily stored in this buffer.
         */
        char                    _formatted_buf[BUF_SIZE];


        /**
         * @brief Zero out _formattedBuf.
         */
        void clear_buf();


    public:
        using Stream::write;


        explicit TextStream();


        ~TextStream() override = default;


        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          Stream API Overrides
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


        bool is_read_supported() override = 0;


        int read() override = 0;


        bool is_write_supported() override = 0;


        bool write(U8 value) override = 0;


        void flush() override = 0;


        void close() override = 0;


        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          Stream API Extension
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


        /**
         * @brief Write the string to the stream.
         * @param msg
         * @return The number of characters written.
         */
        size_t write(const String& msg);


        /**
         * @brief Write the string followed by a new line character to the stream.
         * @param msg
         * @return The number of characters written.
         */
        size_t write_line(const String& msg);


        /**
         * @brief First format the string then write it to the stream.
         * @param format  Template string.
         * @param arg_list Array of arguments.
         * @param arg_size Size of the argument array.
         * @return The number of characters written.
         */
        size_t write_formatted(const String& format, Argument* arg_list, size_t arg_size);


        /**
         * @brief First format the string then write it to the stream.
         * @param format Template string.
         * @param args   Varargs of template arguments.
         * @return The number of characters written.
         */
        template<typename... Args>
        size_t write_formatted(const String& fmt, Args... args) {
            Argument arg_array[] = { args... };
            interpolate(fmt.to_cstr(), _formatted_buf, BUF_SIZE, arg_array, sizeof...(Args));
            size_t out = write(_formatted_buf);
            clear_buf();
            return out;
        }


        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          ANSI Support
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


        /**
         * @brief True: This text stream supports ANSI escape codes, False: It does not.
         */
        virtual bool is_ansi_supported() = 0;


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
}

#endif //RUNEOS_STREAM_H
