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

#ifndef RUNEOS_PARSER_H
#define RUNEOS_PARSER_H


#include <Hammer/String.h>

#include <Shell/AST.h>
#include <Shell/Lexer.h>


namespace Rune::Shell {
    /**
     * @brief Parsed shell input either the AST node or error information.
     */
    struct ParsedInput {
        UniquePointer<ASTNode> ast_node;
        bool has_error;
        Token actual;
        TokenType expected;


        /**
         * @brief Create a parsed input object with has_error=false, expected=TokenType::NONE, token={} and the given
         * ast_node.
         * @param ast_node
         * @return
         */
        static ParsedInput make_good(UniquePointer<ASTNode> ast_node);


        /**
         * @brief Create a parsed input object with ast_node=null, has_error=true and the given values.
         * @param actual
         * @param expected
         * @return
         */
        static ParsedInput make_error(const Token& actual, TokenType expected);
    };

    /**
     * The grammar of the parser is defined as followed:
     * <ul>
     *  <li>Input               = CommandSequence, CmdSeqPostfix? | EnvVarDeclaration | Redirection</li>
     *  <li>Redirection         = CommandSequence, ">", Path | Identifier<li>
     *  <li>CommandSequence     = (Path | Identifier), Argument*</li>
     *  <li>CmdSeqPostfix       = ">", Path<li>
     *  <li>Argument            = Identifier | Path | EnvVar | String | Flag</li>
     *  <li>Flag                = "-", ["-"], Identifier</li>
     *  <li>EnvVarDeclaration   = Identifier, "=", (Identifier | Path | EnvVar | String | EscapeCode)+</li>
     *  <li>String              = "'", (" " | Identifier | Path | EnvVar | EscapeCode)*, "'"</li>
     *  <li>Path                = PathElement, ("/", PathElement+)*</li>
     *  <li>EnvVar              = "$", Identifier </li>
     *  <li>PathElement         = ^[>\'$=]+</li>
     *  <li>Identifier          = [a-zA-Z0-9_-]+</li>
     *  <li>EscapeCode          = "\", [>\'$=]</li>
     * <li></li>
     * </ul>
     * @brief The parser of the shell interpreter.
     */
    class Parser {
        Lexer _lexer;


        ParsedInput parse_input();


        ParsedInput parse_command_sequence();


        ParsedInput parse_argument();


        ParsedInput parse_flag();


        ParsedInput parse_env_var_declaration();


        ParsedInput parse_string();


        ParsedInput parse_env_var();


        ParsedInput parse_path();


        ParsedInput parse_identifier();


        ParsedInput parse_escape_code();


    public:
        Parser();


        ParsedInput parse_shell_input(const String& input);
    };
}

#endif //RUNEOS_PARSER_H
