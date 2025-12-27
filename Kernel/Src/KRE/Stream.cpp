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

#include <KRE/Stream.h>

namespace Rune {
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      TextStream API
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    TextStream::TextStream() : _formatted_buf() {}

    void TextStream::clear_buf() { memset(_formatted_buf.data(), 0, BUF_SIZE); }

    auto TextStream::write(const String& msg) -> size_t {
        size_t chars_written = 0;
        for (const auto& ch : msg) {
            if (!write(static_cast<U8>(ch))) break;
            chars_written++;
        }
        return chars_written;
    }

    auto TextStream::write_line(const String& msg) -> size_t {
        const size_t chars_written = write(msg);
        if (chars_written == msg.size()) write('\n');
        return chars_written + 1;
    }

    auto TextStream::write_formatted(const String& format, Argument* arg_list, size_t arg_size)
        -> size_t {
        interpolate(format.to_cstr(), _formatted_buf.data(), BUF_SIZE, arg_list, arg_size);
        const size_t out = write(_formatted_buf.data());
        clear_buf();
        return out;
    }

    void TextStream::set_background_color(const Pixel& color) {
        const String ansi_bg_selector =
            String::format("\033[48;2;{};{};{}m", color.red, color.green, color.blue);
        write(ansi_bg_selector);
    }

    void TextStream::set_foreground_color(const Pixel& color) {
        const String ansi_bg_selector =
            String::format("\033[38;2;{};{};{}m", color.red, color.green, color.blue);
        write(ansi_bg_selector);
    }

    void TextStream::reset_style() {
        const String ansi_reset_attr = "\033[0m";
        write(ansi_reset_attr);
    }
} // namespace Rune
