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

#include <KernelRuntime/String.h>

#include <KernelRuntime/Memory.h>

#include <float.h>

namespace Rune {
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          String Formatting
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    constexpr char HEX_CHARS[]          = "0123456789ABCDEF";
    constexpr U8   DEF_STRING_PRECISION = 255;
    constexpr U8   DEF_FLOAT_PRECISION  = 3;

    enum class ParserState { START, INDEX, FILL_ALIGN, PREFIX, WIDTH, PRECISION, TYPE, END };

    size_t detect_number(const char* fmt) {
        size_t look_ahead = 0;
        while (48 <= *(fmt + look_ahead) && *(fmt + look_ahead) <= 57) {
            look_ahead++;
        }
        return look_ahead;
    }

    U8 parse_number(const char* num_str, const size_t num_end) {
        size_t pow = num_end - 1;
        int    num = 0;
        for (size_t i = 0; i < num_end; i++) {
            constexpr int pow10[3]  = {1, 10, 100};
            num                    += (*num_str - 48) * pow10[pow];
            pow--;
            num_str++;
        }
        return num;
    }

    size_t write_reverse(char* buf, const size_t off, const size_t buf_len, const char* msg, int len, const int limit) {
        size_t buf_pos = off;
        while (--len >= limit && buf_pos < buf_len)
            buf[buf_pos++] = msg[len];
        return buf_pos;
    }

    template <typename TNumber> U8 int_to_buf(TNumber num, char buf[], U8 radix, const size_t precision) {
        U8 pos = 0;
        do {
            TNumber rem  = num % radix;
            buf[pos++]   = HEX_CHARS[rem];
            num         /= radix;
        } while (num > 0 && pos < precision);
        return pos;
    }

    size_t format_string(char*        buf,
                         const size_t off,
                         const size_t buf_len,
                         const char*  msg,
                         const char   fill,
                         const char   align,
                         const size_t width,
                         const size_t precision) {
        size_t buf_pos = off;
        char   b[precision];
        U8     pos = 0;
        while (*msg && pos < precision) {
            b[pos++] = *msg;
            msg++;
        }

        const size_t padding   = width > pos ? width - pos : 0;
        size_t       pad_left  = 0;
        size_t       pad_right = 0;
        if (align == '^') {
            pad_left  = padding / 2;
            pad_right = padding - pad_left;
        } else if (align == '<') {
            pad_right = padding;
        } else {
            pad_left = padding;
        }

        for (size_t i = 0; i < pad_left && buf_pos < buf_len; i++) {
            buf[buf_pos++] = fill;
        }

        for (size_t i = 0; i < pos && buf_pos < buf_len; i++) {
            buf[buf_pos++] = b[i];
        }

        for (size_t i = 0; i < pad_right && buf_pos < buf_len; i++) {
            buf[buf_pos++] = fill;
        }

        return buf_pos;
    }

    template <typename TNumber>
    size_t format_number(char*        buf,
                         const size_t off,
                         const size_t buf_len,
                         TNumber      num,
                         const char   fill,
                         const char   align,
                         const size_t width,
                         U8           radix,
                         const bool   use_prefix) {
        const bool negative = num < 0;
        if (negative) num = 0 - num;
        char b[32];
        U8   pos = int_to_buf(num, b, radix, 256); // Integer have no precision -> Set extremely high
        if (negative) {
            b[pos++] = '-';
        }

        const size_t padding   = width > pos ? width - pos : 0;
        size_t       pad_left  = 0;
        size_t       pad_right = 0;
        if (align == '^') {
            pad_left  = padding / 2 + 1;
            pad_right = padding - pad_left;
        } else if (align == '<') {
            pad_right = padding;
        } else {
            pad_left = padding;
        }

        size_t buf_pos = off;
        if (use_prefix && align == '=') {
            if (radix == 2) {
                buf[buf_pos++] = '0';
                buf[buf_pos++] = 'b';
            } else if (radix == 8) {
                buf[buf_pos++] = '0';
                buf[buf_pos++] = 'o';
            } else if (radix == 16) {
                buf[buf_pos++] = '0';
                buf[buf_pos++] = 'x';
            }
        }
        for (size_t i = 0; i < pad_left && buf_pos < buf_len; i++)
            buf[buf_pos++] = fill;

        if (use_prefix && align != '=') {
            if (radix == 2) {
                buf[buf_pos++] = '0';
                buf[buf_pos++] = 'b';
            } else if (radix == 8) {
                buf[buf_pos++] = '0';
                buf[buf_pos++] = 'o';
            } else if (radix == 16) {
                buf[buf_pos++] = '0';
                buf[buf_pos++] = 'x';
            }
        }

        buf_pos = write_reverse(buf, buf_pos, buf_len, b, pos, 0);

        for (size_t i = 0; i < pad_right && buf_pos < buf_len; i++)
            buf[buf_pos++] = fill;

        return buf_pos;
    }

    template <typename TFloat>
    size_t format_floating_point_number(char*        buf,
                                        const size_t off,
                                        const size_t buf_len,
                                        TFloat       num,
                                        const char   fill,
                                        const char   align,
                                        const size_t width,
                                        size_t       precision) {
        if (num != num)
            return format_string(buf, off, buf_len, "nan", ' ', '>', 0, 3);
        else if (num < -DBL_MAX)
            return format_string(buf, off, buf_len, "-inf", ' ', '>', 0, 4);
        else if (num > DBL_MAX)
            return format_string(buf, off, buf_len, "+inf", ' ', '>', 0, 4);

        if (precision >= 10) {
            precision = 9;
        }

        size_t buf_pos = off;
        if (num < 0 && buf_pos < buf_len) {
            buf[buf_pos++] = '-';
            num            = 0 - num;
        }

        static const int pow10[]  = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000};
        const bool       negative = num < 0;
        if (negative) num = 0 - num;

        long         whole = static_cast<long>(num);
        const double tmp   = (num - whole) * pow10[precision];
        long         frac  = static_cast<long>(tmp);
        double       diff  = tmp - static_cast<double>(frac);

        if (diff > 0.5) {
            // round up
            ++frac;
            // rollover, e.g. 0.99 with precision 1
            // diff = 0.9, frac = 9 -> (frac+1) frac = 10 -> would print 0.10 (not good)
            // with case: frac = 0 and whole = 1 -> prints 1.0 (good)
            if (frac >= pow10[precision]) {
                frac = 0;
                ++whole;
            }
        } else if (diff < 0.5) {
            // round down
        } else {
            // diff == 0.5 -> round up
            ++frac;
        }

        if (precision == 0) {
            // no precision -> frac = 0 and round up if number >= x.5
            diff = num - static_cast<double>(whole);
            if (diff >= 0.5) {
                ++whole;
            }
            frac = 0;
        }

        char   whole_buf[32];
        size_t whole_pos = int_to_buf(whole, whole_buf, 10, 32);
        if (negative) {
            whole_buf[whole_pos++] = '-';
        }

        char   frac_buf[32];
        size_t frac_pos   = int_to_buf(frac, frac_buf, 10, precision);
        size_t frac_limit = 0;
        if (frac_pos < precision) {
            // Case: num=0.001, Precision=3. Would print 0.1 -> Need to add the zeroes after the dot
            for (size_t i = frac_pos; i < precision; ++i) {
                frac_buf[i] = '0';
                ++frac_pos;
            }
        }

        // Skip trailing zeroes
        while (frac_buf[frac_limit] == '0' && frac_limit < frac_pos)
            frac_limit++;

        const size_t padding =
            width > whole_pos + 1 + frac_pos - frac_limit ? width - whole_pos - 1 - frac_pos + frac_limit : 0;
        size_t pad_left  = 0;
        size_t pad_right = 0;
        if (align == '^') {
            pad_left  = padding / 2;
            pad_right = padding - pad_left;
        } else if (align == '<') {
            pad_right = padding;
        } else {
            pad_left = padding;
        }

        for (size_t i = 0; i < pad_left && buf_pos < buf_len; i++)
            buf[buf_pos++] = fill;

        buf_pos = write_reverse(buf, buf_pos, buf_len, whole_buf, static_cast<int>(whole_pos), 0);
        if (buf_pos < buf_len) buf[buf_pos++] = '.';

        // When we print 0.001 for example, then fracBuf will only contain the 1 and not 001
        // that's why we need to print the zeroes explicitly
        buf_pos =
            write_reverse(buf, buf_pos, buf_len, frac_buf, static_cast<int>(frac_pos), static_cast<int>(frac_limit));

        for (size_t i = 0; i < pad_right && buf_pos < buf_len; i++)
            buf[buf_pos++] = fill;

        return buf_pos;
    }

    size_t interpolate(const char* fmt, char* buf, const size_t buf_size, const Argument* args, const size_t arg_size) {
        auto   state            = ParserState::START;
        size_t buf_pos          = 0;
        U8     auto_idx_listing = 0; // 0: tbd, 1: no, 2: yes

        size_t arg_pos    = -1;
        char   fill       = ' ';
        char   align      = '>';
        bool   use_prefix = false;
        U8     width      = 0;
        U8     precision  = 0;
        U8     radix      = 10;

        while (buf_pos < buf_size && *fmt) {
            switch (state) {
                case ParserState::START:
                    if (*fmt == '{')
                        state = ParserState::INDEX;
                    else
                        buf[buf_pos++] = *fmt;
                    fmt++;
                    break;
                case ParserState::INDEX: {
                    if (const size_t num_length = detect_number(fmt); num_length > 0) {
                        if (auto_idx_listing == 2)
                            // ERROR auto arg list -> manual arg list
                            return 0;

                        if (auto_idx_listing == 0) auto_idx_listing = 1;

                        arg_pos  = parse_number(fmt, num_length);
                        fmt     += num_length;
                        if (arg_pos >= arg_size)
                            // ERROR argument out of bounds
                            return 0;
                    } else if (*fmt == '}' || *fmt == ':') {
                        if (auto_idx_listing == 1)
                            // ERROR manual arg list -> auto arg list
                            return 0;

                        if (auto_idx_listing == 0) auto_idx_listing = 2;
                        arg_pos++;
                    } else {
                        // ERROR non number inside brackets found e.g. {a}
                        return 0;
                    }

                    if (*fmt == ':') {
                        state = ParserState::FILL_ALIGN;
                        fmt++; // consume ':'
                    } else {
                        state = ParserState::END;
                    }
                    break;
                }
                case ParserState::FILL_ALIGN: {
                    const char c_token = *fmt;

                    if (const char n_token = *(fmt + 1);
                        n_token == '<' || n_token == '^' || n_token == '>' || n_token == '=') {
                        if (c_token == '{' || c_token == '}')
                            // ERROR { or } not allowed as fill
                            return 0;
                        fill   = c_token;
                        align  = n_token;
                        fmt   += 2; // consume fill and align
                    } else {
                        if (c_token == '<' || c_token == '^' || c_token == '>' || c_token == '=') {
                            align = c_token;
                            fmt++; // consume align
                        }
                    }
                    state = ParserState::PREFIX;
                    break;
                }
                case ParserState::PREFIX:
                    use_prefix = *fmt == '#';
                    if (use_prefix) fmt++; // consume '#'
                    state = ParserState::WIDTH;
                    break;
                case ParserState::WIDTH: {
                    if (const size_t num_length = detect_number(fmt); num_length > 0) {
                        width  = parse_number(fmt, num_length);
                        fmt   += num_length;
                    }
                    state = ParserState::PRECISION;
                    break;
                }
                case ParserState::PRECISION: {
                    if (*fmt == '.') {
                        if (args[arg_pos].type != Argument::AType::FLOAT
                            && args[arg_pos].type != Argument::AType::DOUBLE
                            && args[arg_pos].type != Argument::AType::LONG_DOUBLE
                            && args[arg_pos].type != Argument::AType::C_STRING)
                            // ERROR precision not allowed for integer types and bool
                            return 0;

                        fmt++; // consume '.'
                        U8 num_length = detect_number(fmt);
                        if (num_length > 0) {
                            precision  = parse_number(fmt, num_length);
                            fmt       += num_length;
                        } else {
                            // ERROR precision expected
                            return 0;
                        }
                    }
                    state = ParserState::TYPE;
                    break;
                }
                case ParserState::TYPE: {
                    if (*fmt == 'b' || *fmt == 'o' || *fmt == 'x') {
                        if (args[arg_pos].type != Argument::AType::SIGNED_CHAR
                            && args[arg_pos].type != Argument::AType::CHAR
                            && args[arg_pos].type != Argument::AType::SHORT
                            && args[arg_pos].type != Argument::AType::INT && args[arg_pos].type != Argument::AType::LONG
                            && args[arg_pos].type != Argument::AType::LONG_LONG
                            && args[arg_pos].type != Argument::AType::U_CHAR
                            && args[arg_pos].type != Argument::AType::U_SHORT
                            && args[arg_pos].type != Argument::AType::U_INT
                            && args[arg_pos].type != Argument::AType::U_LONG
                            && args[arg_pos].type != Argument::AType::U_LONG_LONG)
                            // ERROR types only supported for numbers
                            return 0;

                        switch (*fmt) {
                            case 'b': radix = 2; break;
                            case 'o': radix = 8; break;
                            case 'x': radix = 16; break;
                        }
                        fmt++;
                    }
                    state = ParserState::END;
                    break;
                }
                case ParserState::END:
                    if (*fmt == '}') {
                        Argument arg = args[arg_pos];
                        switch (arg.type) {
                            case Argument::SIGNED_CHAR:
                                // print numerical value because no negative ASCII. Good??
                                buf_pos = format_number(buf,
                                                        buf_pos,
                                                        buf_size,
                                                        arg.values.SChar,
                                                        fill,
                                                        align,
                                                        width,
                                                        radix,
                                                        use_prefix);
                                break;
                            case Argument::CHAR:
                                buf[buf_pos++] = arg.values.Char; // print ASCII code
                                break;
                            case Argument::SHORT:
                                buf_pos = format_number(buf,
                                                        buf_pos,
                                                        buf_size,
                                                        arg.values.Short,
                                                        fill,
                                                        align,
                                                        width,
                                                        radix,
                                                        use_prefix);
                                break;
                            case Argument::INT:
                                buf_pos = format_number(buf,
                                                        buf_pos,
                                                        buf_size,
                                                        arg.values.Int,
                                                        fill,
                                                        align,
                                                        width,
                                                        radix,
                                                        use_prefix);
                                break;
                            case Argument::LONG:
                                buf_pos = format_number(buf,
                                                        buf_pos,
                                                        buf_size,
                                                        arg.values.Long,
                                                        fill,
                                                        align,
                                                        width,
                                                        radix,
                                                        use_prefix);
                                break;
                            case Argument::LONG_LONG:
                                buf_pos = format_number(buf,
                                                        buf_pos,
                                                        buf_size,
                                                        arg.values.LongLong,
                                                        fill,
                                                        align,
                                                        width,
                                                        radix,
                                                        use_prefix);
                                break;
                            case Argument::U_CHAR:
                                buf_pos = format_number(buf,
                                                        buf_pos,
                                                        buf_size,
                                                        arg.values.UChar,
                                                        fill,
                                                        align,
                                                        width,
                                                        radix,
                                                        use_prefix);
                                break;
                            case Argument::U_SHORT:
                                buf_pos = format_number(buf,
                                                        buf_pos,
                                                        buf_size,
                                                        arg.values.UShort,
                                                        fill,
                                                        align,
                                                        width,
                                                        radix,
                                                        use_prefix);
                                break;
                            case Argument::U_INT:
                                buf_pos = format_number(buf,
                                                        buf_pos,
                                                        buf_size,
                                                        arg.values.UInt,
                                                        fill,
                                                        align,
                                                        width,
                                                        radix,
                                                        use_prefix);
                                break;
                            case Argument::U_LONG:
                                buf_pos = format_number(buf,
                                                        buf_pos,
                                                        buf_size,
                                                        arg.values.ULong,
                                                        fill,
                                                        align,
                                                        width,
                                                        radix,
                                                        use_prefix);
                                break;
                            case Argument::U_LONG_LONG:
                                buf_pos = format_number(buf,
                                                        buf_pos,
                                                        buf_size,
                                                        arg.values.ULongLong,
                                                        fill,
                                                        align,
                                                        width,
                                                        radix,
                                                        use_prefix);
                                break;
                            case Argument::FLOAT:
                                buf_pos = format_floating_point_number(buf,
                                                                       buf_pos,
                                                                       buf_size,
                                                                       arg.values.Float,
                                                                       fill,
                                                                       align,
                                                                       width,
                                                                       precision > 0 ? precision : DEF_FLOAT_PRECISION);
                                break;
                            case Argument::DOUBLE:
                                buf_pos = format_floating_point_number(buf,
                                                                       buf_pos,
                                                                       buf_size,
                                                                       arg.values.Double,
                                                                       fill,
                                                                       align,
                                                                       width,
                                                                       precision > 0 ? precision : DEF_FLOAT_PRECISION);
                                break;
                            case Argument::LONG_DOUBLE:
                                buf_pos = format_floating_point_number(buf,
                                                                       buf_pos,
                                                                       buf_size,
                                                                       arg.values.LongDouble,
                                                                       fill,
                                                                       align,
                                                                       width,
                                                                       precision > 0 ? precision : DEF_FLOAT_PRECISION);
                                break;
                            case Argument::BOOL:
                                buf_pos = format_string(buf,
                                                        buf_pos,
                                                        buf_size,
                                                        arg.values.Bool ? "True" : "False",
                                                        fill,
                                                        align,
                                                        width,
                                                        precision > 0 ? precision : DEF_STRING_PRECISION);
                                break;
                            case Argument::C_STRING:
                                buf_pos = format_string(buf,
                                                        buf_pos,
                                                        buf_size,
                                                        arg.values.CString,
                                                        fill,
                                                        align,
                                                        width,
                                                        precision > 0 ? precision : DEF_STRING_PRECISION);
                                break;
                        }
                        fill       = ' ';
                        align      = '>';
                        use_prefix = false;
                        width      = 0;
                        precision  = 0;
                        radix      = 10;
                        state      = ParserState::START;
                        fmt++;
                    } else {
                        // Error: Expected '}'
                        return 0;
                    }
                    break;
            }
        }
        return buf_pos;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          String class
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      Formating string converter
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    const char* string_to_cstr(const String& str) { return str.to_cstr(); }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Private Functions
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    size_t String::get_cstr_size(const char* c_str) {
        size_t      size  = 0;
        const char* c_pos = c_str;
        while (*c_pos) {
            c_pos++;
            size++;
        }
        return size;
    }

    const char* String::get_buf() const { return _size < BUF_SIZE ? _storage._stackBuf : _storage._heapBuf; }

    void String::init(const char* c_str, const size_t offset, const size_t size) {
        if (size < BUF_SIZE) {
            memcpy((void*) _storage._stackBuf, (void*) &c_str[offset], size);
            memset(&_storage._stackBuf[size], '\0', BUF_SIZE - size);
            _capacity = BUF_SIZE;
        } else {
            _storage._heapBuf = new char[size + 1];
            memcpy((void*) _storage._heapBuf, (void*) &c_str[offset], size);
            _storage._heapBuf[size] = '\0'; // add null terminator
            _capacity               = size + 1;
        }
        _size = size;
    }

    void String::concat(const char* o_buf, const size_t o_size) {
        const size_t new_size = _size + o_size;
        if (new_size < BUF_SIZE) {
            memcpy((void*) &_storage._stackBuf[_size], (void*) o_buf, o_size);
            memset(&_storage._stackBuf[new_size], '\0', BUF_SIZE - new_size);
        } else {
            const auto n_buf = new char[new_size + 1];
            memcpy(n_buf, (void*) get_buf(), _size);
            memcpy(&n_buf[_size], (void*) o_buf, o_size);
            n_buf[new_size] = '\0';
            if (_size >= BUF_SIZE) {
                memset((void*) _storage._heapBuf, '\0', _size);
                delete _storage._heapBuf;
            }
            _storage._heapBuf = n_buf;
            _capacity         = new_size + 1;
        }
        _size = new_size;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Constructors
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    String::String(const char* arr_one, const char* arr_two, size_t size_one, size_t size_two) {
        const size_t size = size_one + size_two;
        if (size < BUF_SIZE) {
            memcpy((void*) _storage._stackBuf, (void*) arr_one, size_one);
            memcpy((void*) &_storage._stackBuf[size_one], (void*) arr_two, size_two);
            memset(&_storage._stackBuf[size], '\0', BUF_SIZE - size);
            _capacity = BUF_SIZE;
        } else {
            _storage._heapBuf = new char[size + 1];
            memcpy((void*) _storage._heapBuf, (void*) arr_one, size_one);
            memcpy(&_storage._heapBuf[size_one], (void*) arr_two, size_two);
            _storage._heapBuf[size] = 0; // add null terminator
            _capacity               = size + 1;
        }
        _size = size;
    }

    String::String() : _size(0), _capacity(0) { init("", 0, 0); }

    String::String(const char ch) : _size(0), _capacity(0) {
        if (ch == '\0') {
            init("", 0, 0);
        } else {
            const char tmp[2] = {ch, 0};
            init(tmp, 0, 1);
        }
    }

    String::String(const char* c_str) : _size(0), _capacity(0) { init(c_str, 0, get_cstr_size(c_str)); }

    String::String(const char* c_str, const size_t size) : _size(0), _capacity(0) { init(c_str, 0, size); }

    String::String(const char* c_str, const size_t offset, const size_t size) : _size(0), _capacity(0) {
        init(c_str, offset, size);
    }

    String::~String() {
        if (_size >= BUF_SIZE) {
            memset(_storage._heapBuf, '\0', _size);
            delete[] _storage._heapBuf;
        }
    }

    String::String(const String& o) : _size(0), _capacity(0) { init(o.to_cstr(), 0, o.size()); }

    String& String::operator=(const String& o) {
        if (this != &o) init(o.to_cstr(), 0, o.size());
        return *this;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Static Functions
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    String String::format(const String& fmt_str, const Argument* args, const size_t arg_size) {
        char         buf[FMT_BUF_SIZE];
        const size_t ch_written = interpolate(fmt_str.to_cstr(), buf, FMT_BUF_SIZE, args, arg_size);
        memset(&buf[ch_written], '\0', FMT_BUF_SIZE - ch_written);
        return {buf};
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      Public (nonstatic) Functions
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    size_t String::size() const { return _size; }

    bool String::is_empty() const { return _size == 0; }

    const char* String::to_cstr() const { return _size < BUF_SIZE ? _storage._stackBuf : _storage._heapBuf; }

    String String::lower() const {
        char        tmp_buf[size() + 1];
        const auto* buf = const_cast<char*>(get_buf());
        for (size_t i = 0; i < _size; i++) {
            U8 offset = 0;
            if (65 <= buf[i] && buf[i] <= 90) offset = 32;
            tmp_buf[i] = static_cast<char>(buf[i] + offset);
        }
        tmp_buf[size()] = '\0';
        return {tmp_buf};
    }

    String String::upper() const {
        char        tmp_buf[size() + 1];
        const auto* buf = const_cast<char*>(get_buf());
        for (size_t i = 0; i < _size; i++) {
            U8 offset = 0;
            if (97 <= buf[i] && buf[i] <= 122) offset = 32;
            tmp_buf[i] = static_cast<char>(buf[i] - offset);
        }
        tmp_buf[size()] = '\0';
        return {tmp_buf};
    }

    LinkedList<String> String::split(const char separator) const {
        LinkedList<String> parts;
        const char*        buf   = get_buf();
        size_t             start = 0;
        for (size_t i = 0; i < _size; i++) {
            if (buf[i] == separator) {
                if (const size_t part_len = i - start; part_len > 0) parts.add_back(String(buf, start, part_len));
                start = i + 1;
            }
        }

        if (start < _size) parts.add_back(String(get_buf(), start, _size - start));
        return parts;
    }

    String String::replace(const char c, const char replacement) {
        const char* buf = get_buf();
        char        new_buf[_size];
        for (size_t i = 0; i < _size; i++)
            new_buf[i] = buf[i] == c ? replacement : buf[i];

        // When the replacement is the null terminator we must recalculate the size of the new string
        return {new_buf, 0, replacement == '\0' ? get_cstr_size(new_buf) : _size};
    }

    int String::last_index_of(const char ch) const {
        const auto b = get_buf();
        for (int i = static_cast<int>(size()) - 1; i >= 0; i--)
            if (b[i] == ch) return i;
        return -1;
    }

    bool String::starts_with(const String& prefix) const {
        if (prefix.is_empty()) return true;      // Every string is prefixed with the empty string
        if (_size < prefix.size()) return false; // prefix is longer -> cannot be prefix e.g. abc cannot prefix ab

        const auto t_b = get_buf();
        const auto p_b = prefix.get_buf();
        for (size_t i = 0; i < prefix.size(); i++)
            if (t_b[i] != p_b[i]) return false;
        return true;
    }

    String String::substring(size_t start_idx) const { return {get_buf(), start_idx, size() - start_idx}; }

    String String::substring(size_t start_idx, size_t len) const { return {get_buf(), start_idx, len}; }

    String String::operator+(const String& o) const {
        if (o.size() == 0) return {_size < BUF_SIZE ? _storage._stackBuf : _storage._heapBuf, 0, _size};

        return {_size < BUF_SIZE ? _storage._stackBuf : _storage._heapBuf,
                o.size() < BUF_SIZE ? o._storage._stackBuf : o._storage._heapBuf,
                _size,
                o.size()};
    }

    String String::operator+(const String&& o) const {
        return {_size < BUF_SIZE ? _storage._stackBuf : _storage._heapBuf,
                o.size() < BUF_SIZE ? o._storage._stackBuf : o._storage._heapBuf,
                _size,
                o.size()};
    }

    String String::operator+(const char* o) const {
        size_t o_size = get_cstr_size(o);
        if (o_size == 0) return {_size < BUF_SIZE ? _storage._stackBuf : _storage._heapBuf, 0, _size};

        return {_size < BUF_SIZE ? _storage._stackBuf : _storage._heapBuf, o, _size, o_size};
    }

    String String::operator+(char ch) const {
        char n_buf[size() + 2]; // new char + null terminator
        memcpy(n_buf, (void*) get_buf(), size());
        n_buf[size()]     = ch;
        n_buf[size() + 1] = 0;
        return {n_buf};
    }

    String& String::operator+=(const String& o) {
        if (o.size() == 0) return *this;

        concat(o.get_buf(), o.size());
        return *this;
    }

    String& String::operator+=(const String&& o) { return this->operator+=(o); }

    String& String::operator+=(const char* o) {
        const size_t o_size = get_cstr_size(o);
        if (o_size == 0) return *this;

        concat(o, o_size);
        return *this;
    }

    String& String::operator+=(const char ch) {
        const char tmp[2] = {ch, '\0'};
        return operator+=(tmp);
    }

    char String::operator[](const size_t index) const {
        return _size < BUF_SIZE ? _storage._stackBuf[index] : _storage._heapBuf[index];
    }

    const char* String::begin() const { return _size < BUF_SIZE ? &_storage._stackBuf[0] : &_storage._heapBuf[0]; }

    const char* String::end() const {
        return _size < BUF_SIZE ? &_storage._stackBuf[_size] : &_storage._heapBuf[_size];
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Operator Overloads
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    String operator+(const char* c_string, String str) {
        size_t o_size = String::get_cstr_size(c_string);
        return {c_string,
                str._size < String::BUF_SIZE ? str._storage._stackBuf : str._storage._heapBuf,
                o_size,
                str._size};
    }

    bool operator==(const String& a, const String& b) {
        if (a._size != b._size) return false;
        if (a._size < String::BUF_SIZE)
            return memcmp((void*) a._storage._stackBuf, (void*) b._storage._stackBuf, a._size) == 0;
        else
            return memcmp((void*) a._storage._heapBuf, (void*) b._storage._heapBuf, a._size) == 0;
    }

    bool operator!=(const String& a, const String& b) { return !(a == b); }
} // namespace Rune
