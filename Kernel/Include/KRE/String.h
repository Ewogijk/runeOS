
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

#ifndef RUNEOS_STRING_H
#define RUNEOS_STRING_H

#include <Ember/Ember.h>

#include <KRE/Collections/LinkedList.h>
#include <KRE/CppRuntimeSupport.h>

namespace Rune {
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      String Formatting
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    class String;

    const char* string_to_cstr(const String& str);

    struct Argument {
        enum AType {
            SIGNED_CHAR,
            CHAR,
            SHORT,
            INT,
            LONG,
            LONG_LONG,
            U_CHAR,
            U_SHORT,
            U_INT,
            U_LONG,
            U_LONG_LONG,
            FLOAT,
            DOUBLE,
            LONG_DOUBLE,
            BOOL,
            C_STRING
        };

        union {
            signed char SChar = 0;
            char        Char;
            short       Short;
            int         Int;
            long        Long;
            long long   LongLong;

            unsigned char      UChar;
            unsigned short     UShort;
            unsigned int       UInt;
            unsigned long      ULong;
            unsigned long long ULongLong;

            float       Float;
            double      Double;
            long double LongDouble;
            bool        Bool;
            const char* CString;
        } values;
        AType type;

        Argument(signed char v) : type(SIGNED_CHAR) { values.SChar = v; }

        Argument(char v) : type(CHAR) { values.Char = v; }

        Argument(short v) : type(SHORT) { values.Short = v; }

        Argument(int v) : type(INT) { values.Int = v; }

        Argument(long v) : type(LONG) { values.Long = v; }

        Argument(long long v) : type(LONG_LONG) { values.LongLong = v; }

        Argument(unsigned char v) : type(U_CHAR) { values.UChar = v; }

        Argument(unsigned short v) : type(U_SHORT) { values.UShort = v; }

        Argument(unsigned int v) : type(U_INT) { values.UInt = v; }

        Argument(unsigned long v) : type(U_LONG) { values.ULong = v; }

        Argument(unsigned long long v) : type(U_LONG_LONG) { values.ULongLong = v; }

        Argument(float v) : type(FLOAT) { values.Float = v; }

        Argument(double v) : type(DOUBLE) { values.Double = v; }

        Argument(long double v) : type(LONG_DOUBLE) { values.LongDouble = v; }

        Argument(bool v) : type(BOOL) { values.Bool = v; }

        Argument(const char* v) : type(C_STRING) { values.CString = v; }

        Argument(const String& v) : type(C_STRING) { values.CString = string_to_cstr(v); }
    };

    /**
     * Replace placeholders in the format string and put the formatted string in the output buffer.
     *
     * <p>
     * Grammar<br>
     *      format      :=  '{'<index>:<<fill>align>'#'<width>'.'<precision><type>'}'<br>
     *      index       :=  Index of a positional argument<br>
     *      fill        :=  Any character<br>
     *      align       :=  '<' | '>' | '^'<br>
     *      width       :=  digit+<br>
     *      precision   :=  digit+<br>
     *      type        :=  'b' | 'x' | 'o' | 'B'<br>
     * </p>
     *
     * <p>
     *  Index: The position of an argument in the argument list. Is optional, if not declared
     * arguments are formatted in order of the argument list, meaning indices 0,1,2,... are assumed.
     * </p>
     *
     * <p>
     *  Align: '<' is left alignment, '^' is center alignment, '>' is right alignment and '=' right
     * alignment but a prefix is used padding will be applied between prefix and argument. e.g.
     * 0x000A (0's are padded). A fill character can be specified if not an empty space is used.
     * </p>
     *
     * <p>
     *  '#': Use a prefix before the argument, only applied to non decimal integer arguments. For
     * binary '0b', octal '0o' and hex '0x' is prepended.
     * </p>
     *
     * <p>
     *  Width: Minimum length the of the formatted argument including prefix, decimal dots, etc.
     * </p>
     *
     * <p>
     *  Precision: Maximum length of the formatted argument. This is for floating points the number
     * of digits after the decimal point and for strings the number of characters printed. Integer
     * arguments are not allowed to have a precision.
     * </p>
     *
     * <p>
     *  Type: Defines the representation of an integer. 'b' for hex, 'o' for octal and 'x' for hex.
     * </p>
     *
     * @param fmt       Format string.
     * @param buf       Output buffer.
     * @param buf_size  Buffer size.
     * @param args      Array of arguments.
     * @param arg_size  Number of arguments.
     *
     * @return Index of the last character that was placed in the output buffer. Zero has two
     * meanings no character was actually placed or an error while parsing the format string
     * happened.
     */
    size_t
    interpolate(const char* fmt, char* buf, size_t buf_size, const Argument* args, size_t arg_size);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          String class
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    class String {
        static constexpr U16 FMT_BUF_SIZE = 256;

        // Small string optimization (SSO)
        // If a c string is smaller the BufSize we allocate it in the stack buffer else we make a
        // heap allocation
        static constexpr size_t BUF_SIZE = 24;
        union {
            char* _heapBuf = nullptr;
            char  _stackBuf[BUF_SIZE];
        } _storage;

        size_t _size;     // size of the string without null terminator
        size_t _capacity; // size of the buffer

        [[nodiscard]]
        const char* get_buf() const;

        void init(const char* c_str, size_t offset, size_t size);

        void concat(const char* o_buf, size_t o_size);

        String(const char* arr_one, const char* arr_two, size_t size_one, size_t size_two);

      public:
        static size_t get_cstr_size(const char* c_str);

        String();

        explicit String(char ch);

        String(const char* c_str);

        String(const char* c_str, size_t size);

        String(const char* c_str, size_t offset, size_t size);

        ~String();

        // Copy constructor and assignment
        String(const String& o);

        String& operator=(const String& o);

        /**
         * Replace all placeholders in the format string with the provided arguments.
         *
         * @param fmt_str  Formatting string.
         * @param args     Argument list.
         * @param arg_size Size of the argument list.
         *
         * @return Formatted string.
         */
        static String format(const String& fmt_str, const Argument* args, size_t arg_size);

        /**
         * Replace all placeholders in the format string with the provided arguments.
         *
         * @tparam Args   Variadic arguments.
         * @param fmt_str Formatting string.
         * @param args    Argument list.
         *
         * @return Formatted string.
         */
        template <typename... Args>
        static String format(const String& fmt_str, Args... args) {
            const Argument arg_array[] = {args...};
            const size_t   arg_size    = sizeof...(Args);
            return format(fmt_str, arg_array, arg_size);
        }

        /**
         *
         * @return The number of characters without the null terminator.
         */
        [[nodiscard]]
        size_t size() const;

        /**
         *
         * @return True if the string contains no characters.
         */
        [[nodiscard]]
        bool is_empty() const;

        /**
         *
         * @return The string in c string representation.
         */
        [[nodiscard]]
        const char* to_cstr() const;

        /**
         *
         * @return A copy of the string where all characters are upper case.
         */
        String lower() const;

        /**
         *
         * @return A copy of the string where all characters are upper case.
         */
        String upper() const;

        /**
         * Split the string along all occurrences of the separator and return a list of all
         * generated substrings.
         *
         * @param separator
         *
         * @return A list of substrings.
         */
        [[nodiscard]]
        LinkedList<String> split(char separator) const;

        /**
         *
         * @param c
         * @param replacement
         *
         * @return A string where all occurrences of c are replaced with the replacement char.
         */
        String replace(char c, char replacement);

        /**
         *
         * @param ch
         *
         * @return Index of the last occurrence of ch.
         */
        [[nodiscard]]
        int last_index_of(char ch) const;

        /**
         * @brief
         * @param prefix
         * @return True: The string starts with prefix, False: It does not.
         */
        [[nodiscard]]
        bool starts_with(const String& prefix) const;

        /**
         *
         * @param start_idx
         *
         * @return A substring starting at the char from startIdx to the end of the this string.
         */
        [[nodiscard]]
        String substring(size_t start_idx) const;

        /**
         *
         * @param start_idx
         * @param len
         *
         * @return A substring starting at the char from startIdx containing the next len number of
         *         characters of this string.
         */
        [[nodiscard]]
        String substring(size_t start_idx, size_t len) const;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                  String concatenation overloads
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        String operator+(const String& o) const;

        String operator+(const String&& o) const;

        String operator+(const char* o) const;

        String operator+(char ch) const;

        friend String operator+(const char* c_string, String str);

        String& operator+=(const String& o);

        String& operator+=(const String&& o);

        String& operator+=(const char* o);

        String& operator+=(char ch);

        /**
         *
         * @param index
         *
         * @return The character at given index.
         */
        char operator[](size_t index) const;

        friend bool operator==(const String& a, const String& b);

        friend bool operator!=(const String& a, const String& b);

        [[nodiscard]]
        const char* begin() const;

        [[nodiscard]]
        const char* end() const;
    };

    template <>
    struct Hash<String> {
        Hash<const char*> c_str_hash;
        auto operator()(const String& key) const -> size_t { return c_str_hash(key.to_cstr()); }
    };

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          String Conversions
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    template <class TNum>
    String int_to_string(TNum num, int radix) {
        constexpr char hex_chars[] = "0123456789ABCDEF";
        char           buf[32];
        memset(buf, 0, 32);
        size_t pos = 0;
        do {
            TNum rem = num % radix;
            // Fill the buffer in reverse order
            buf[31 - pos]  = hex_chars[rem];
            num           /= radix;
            pos++;
        } while (num > 0 && pos < 32);

        return {buf, 32 - pos, pos};
    }

    template <class TNum>
    bool parse_int(const String& str, TNum radix, TNum& out) {
        if (radix < 0 || 16 < radix)
            // Only radixes 0-16 are supported
            return false;
        if (str.is_empty()) return false;

        TNum num   = 0;
        TNum pow   = 1;
        int  limit = 0;
        int  sign  = 1;
        if (str[0] == '-' || str[0] == '+') {
            if (str[0] == '-') sign = -1;
            limit = 1; // Do not parse first char
            if (str.size() == 1)
                // String is only '-'
                return false;
        }

        for (int i = (int) str.size() - 1; i >= limit; i--) {
            char ch  = str[i];
            TNum val = 0;
            if (48 <= ch && ch <= 57) {
                val = ch - 48;
            } else if ('A' <= ch && ch <= 'F') {
                val = ch - 55; // 65 - 10 -> to get decimal 10 for 'A', etc.
            } else if ('a' <= ch && ch <= 'f') {
                val = ch - 87; // 97 - 10 -> to get decimal 10 for 'A', etc.
            } else {
                // Non digit character encountered
                return false;
            }

            if (val >= radix)
                // A digit character is bigger than the radix e.g. 'F' was found bug radix is 8
                return false;
            val *= pow;
            if (num + val < num)
                // Overflow occurred
                return false;
            out += val;
            pow *= radix;
        }
        out *= sign;
        return true;
    }
} // namespace Rune

#endif // RUNEOS_STRING_H
