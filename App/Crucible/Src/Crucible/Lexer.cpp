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

#include <Crucible/Lexer.h>

namespace Crucible {

    DEFINE_ENUM(TokenType, TOKEN_TYPES, 0x0) // NOLINT

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Tokenizer
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    bool Lexer::is_digit(char c) { return '0' <= c && c <= '9'; }

    bool Lexer::is_lower_case(char c) { return 'a' <= c && c <= 'z'; }

    bool Lexer::is_upper_case(char c) { return 'A' <= c && c <= 'Z'; }

    bool Lexer::is_esc_ch(char c) {
        return c == '\\' || c == '\'' || c == '$' || c == '=' || c == '>';
    }

    bool Lexer::is_reserved(char c) {
        return c == '$' || c == '=' || c == '\'' || c == '>' || c == '\\';
    }

    bool Lexer::is_path_element(char c) {
        // Allow every char except not printable chars and reserved chars
        return c > 32 && !is_reserved(c);
    }

    bool Lexer::is_identifier(char c) {
        return c == '_' || c == '-' || is_lower_case(c) || is_upper_case(c) || is_digit(c);
    }

    bool Lexer::has_more() { return _cursor < (int) _input.size(); }

    char Lexer::advance() { return has_more() ? _input[_cursor++] : '\0'; }

    char Lexer::peek() { return has_more() ? _input[_cursor] : '\0'; }

    void Lexer::parse_escape_code() {
        const char escaped = advance();
        char       b[2]    = {'\\', escaped};
        const bool is_good = is_esc_ch(escaped);
        _token_buffer.push_back({is_good ? TokenType::ESCAPE_CODE : TokenType::UNEXPECTED_TOKEN,
                                 std::string(b, 2),
                                 is_good ? _cursor - 2 : _cursor - 1});
    }

    void Lexer::parse_identifier_or_path_element(bool include_ws) {
        char      b[BUF_SIZE];
        int       b_pos = 0;
        const int start = _cursor - 1;
        b[b_pos++] = _input[start]; // scan_token has consumed the first identifier character -> add
                                    // it manually

        bool is_path = false;
        // !skip_ws || peek() != ' ' explanation:
        // If skip_ws == true -> !skip_ws is false therefore the final evaluation depends on "peek()
        // != ' '" which means
        //      we parse as long as we don't see any whitespace
        // If skip_ws == false -> !skip_ws is always true therefore we will also parse any
        // whitespace
        while (has_more() && peek() != ' ' && b_pos < static_cast<int>(BUF_SIZE)) {
            // Add all identifier characters to the buffer
            if (is_path)
                while (is_path_element(peek())) b[b_pos++] = advance();
            else
                while (is_identifier(peek())) b[b_pos++] = advance();

            if (char peek_a_boo = peek(); peek_a_boo == '\0' || peek_a_boo == '\''
                                          || peek_a_boo == '\\' || peek_a_boo == '='
                                          || peek_a_boo == '$' || peek_a_boo == '>') {
                // '\0' -> End of input reached
                // Else -> End of IDENTIFIER/PATH_ELEMENT reached
                break;
            } else if (peek_a_boo == ' ') {
                if (include_ws)
                    while (peek() == ' ') b[b_pos++] = advance();
                // else break the outer loop
            } else if (!is_identifier(peek_a_boo)) {
                is_path = true;
            } else {
                // Current char is not an IDENTIFIER or PATH_ELEMENT -> We found an undefined char
                _token_buffer.push_back(
                    {TokenType::UNEXPECTED_TOKEN, std::string(1, peek()), _cursor});
                return;
            }
        }
        _token_buffer.push_back(
            {is_path ? TokenType::PATH : TokenType::IDENTIFIER, std::string(b, b_pos), start});
    }

    void Lexer::parse_string() {
        // The opening quote token is already buffered
        while (has_more() && peek() != '\'') {
            switch (char c = advance()) {
                case '$':
                    _token_buffer.push_back({TokenType::DOLLAR, std::string(1, c), _cursor - 1});
                    advance(); // parse_identifier_or_path_element expects the first character
                               // already to be consumed
                    parse_identifier_or_path_element(false);
                    break;
                case '\\': parse_escape_code(); break;
                default:   parse_identifier_or_path_element(true);
            }
        }
        if (has_more())
            _token_buffer.push_back({TokenType::QUOTE, std::string(1, advance()), _cursor - 1});
    }

    void Lexer::scan_token() {
        while (peek() == ' ') advance(); // skip leading white space

        switch (char c = advance()) {
            case '\0': _token_buffer.push_back({TokenType::END, "", _cursor}); break;
            case '$':
                _token_buffer.push_back({TokenType::DOLLAR, std::string(1, c), _cursor - 1});
                break;
            case '=':
                _token_buffer.push_back({TokenType::ASSIGNMENT, std::string(1, c), _cursor - 1});
                break;
            case '-':
                _token_buffer.push_back({TokenType::DASH, std::string(1, c), _cursor - 1});
                break;
            case '>':
                _token_buffer.push_back({TokenType::REDIRECT, std::string(1, c), _cursor - 1});
                break;
            case '\'':
                _token_buffer.push_back({TokenType::QUOTE, std::string(1, c), _cursor - 1});
                parse_string();
                break;
            case '\\': parse_escape_code(); break;
            default:   parse_identifier_or_path_element(false);
        }
    }

    Lexer::Lexer(const std::string& input)
        : _input(input),
          _cursor(0),
          _token_buffer(),
          _skip_ws(true),
          _current_token({TokenType::END, "", 0}),
          _next_token({TokenType::END, "", 0}) {}

    Token Lexer::next_token() {
        if (!has_more() && _token_buffer.empty())
            return {TokenType::END, "", static_cast<int>(_input.size())};

        if (_token_buffer.empty()) scan_token();

        Token t = _token_buffer.front();
        _token_buffer.erase(_token_buffer.begin());
        return t;
    }

    Token Lexer::peek_token() {
        if (!has_more() && _token_buffer.empty()) return {TokenType::END, "", (int) _input.size()};

        if (_token_buffer.empty()) scan_token();
        return _token_buffer.front();
    }
} // namespace Crucible