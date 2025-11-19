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

#ifndef CRUCIBLE_LEXER_H
#define CRUCIBLE_LEXER_H

#include <Ember/Enum.h>

#include <string>
#include <vector>

namespace Crucible {
    /**
     * @brief All types of shell tokens for the parser.
     * <ul>
     *  <li>END:                End of input reached.</li>
     *  <li>UNEXPECTED_TOKEN:   Unknown token found.</li>
     *  <li>PATH:               See Grammar (OS/Shell/Parser.h)</li>
     *  <li>IDENTIFIER:         See Grammar (OS/Shell/Parser.h)</li>
     *  <li>ESCAPE_CODE:        See Grammar (OS/Shell/Parser.h)</li>
     *  <li>DOLLAR:             $</li>
     *  <li>DASH:               -</li>
     *  <li>ASSIGNMENT:         =</li>
     *  <li>QUOTE:              '</li>
     *  <li>REDIRECT:           ></li>
     * </ul>
     */
#define TOKEN_TYPES(X)                                                                             \
    X(TokenType, END, 0x1)                                                                         \
    X(TokenType, UNEXPECTED_TOKEN, 0x2)                                                            \
    X(TokenType, PATH, 0x3)                                                                        \
    X(TokenType, IDENTIFIER, 0x4)                                                                  \
    X(TokenType, ESCAPE_CODE, 0x5)                                                                 \
    X(TokenType, DOLLAR, 0x6)                                                                      \
    X(TokenType, DASH, 0x7)                                                                        \
    X(TokenType, ASSIGNMENT, 0x8)                                                                  \
    X(TokenType, QUOTE, 0x9)                                                                       \
    X(TokenType, REDIRECT, 0xA)

    DECLARE_ENUM(TokenType, TOKEN_TYPES, 0x0) // NOLINT

    /**
     * @brief A token from an input string, e.g. $hi -> Token(ENV_NAME, "hi")
     */
    struct Token {
        TokenType   type;
        std::string text; // Value of the token e.g. "hi"
        int position; // Start index of this token relative to the input e.g "123 hi" -> Token "hi",
                      // character=4
    };

    class Lexer {
        static constexpr size_t BUF_SIZE = 64;

        std::string _input;
        int         _cursor;

        std::vector<Token> _token_buffer;

        // OLD STUFF
        bool  _skip_ws;
        Token _current_token;
        Token _next_token;

        static bool is_digit(char c);

        static bool is_lower_case(char c);

        static bool is_upper_case(char c);

        static bool is_reserved(char c);

        // Check if c is in [a-zA-Z0-9_-]
        static bool is_path_element(char c);

        // Check if c is in [a-zA-Z0-9_]
        static bool is_identifier(char c);

        // Check if c is in [\'$=]
        static bool is_esc_ch(char c);

        bool has_more();

        // Get char at cursor then increment the cursor by one
        char advance();

        // Get chat at cursor without incrementing the cursor
        char peek();

        void parse_escape_code();

        void parse_identifier_or_path_element(bool include_ws);

        void parse_string();

        void scan_token();

      public:
        explicit Lexer(const std::string& input);

        Token next_token();

        Token peek_token();
    };
} // namespace Rune::Shell

#endif // CRUCIBLE_LEXER_H
