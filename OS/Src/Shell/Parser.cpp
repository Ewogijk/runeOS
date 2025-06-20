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

#include <Shell/Parser.h>


namespace Rune::Shell {

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          ParsedInput
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    ParsedInput ParsedInput::make_good(UniquePointer<Shell::ASTNode> ast_node) {
        ParsedInput good;
        good.ast_node  = move(ast_node);
        good.has_error = false;
        good.actual    = { };
        good.expected  = TokenType::NONE;
        return good;
    }


    ParsedInput ParsedInput::make_error(const Shell::Token& actual, Shell::TokenType expected) {
        ParsedInput err;
        err.ast_node  = UniquePointer<ASTNode>();
        err.has_error = true;
        err.actual    = actual;
        err.expected  = expected;
        return err;
    }


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Parser
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    ParsedInput Parser::parse_input() {
        switch (_lexer.peek_token().type) {
            case TokenType::PATH:
            case TokenType::IDENTIFIER:
                return parse_command_sequence();
            case TokenType::DOLLAR:
                return parse_env_var_declaration();
            default:
                return ParsedInput::make_error(_lexer.peek_token(), TokenType::IDENTIFIER);
        }
    }


    ParsedInput Parser::parse_command_sequence() {
        ParsedInput path_or_identifier;
        if (_lexer.peek_token().type == TokenType::PATH)
            path_or_identifier = parse_path();
        else
            path_or_identifier = parse_identifier();

        if (path_or_identifier.has_error)
            return path_or_identifier;

        LinkedList<UniquePointer<ASTNode>> args;
        Token peek_a_boo = _lexer.peek_token();
        while (peek_a_boo.type != TokenType::REDIRECT && peek_a_boo.type != TokenType::END) {
            ParsedInput arg = parse_argument();
            if (arg.has_error)
                return arg;
            args.add_back(move(arg.ast_node));
            peek_a_boo = _lexer.peek_token();
        }
        String                             redirect_str;
        if (_lexer.peek_token().type == TokenType::REDIRECT) {
            _lexer.next_token();
            Token redirect_token = _lexer.next_token();
            if (redirect_token.type != TokenType::PATH && redirect_token.type != TokenType::IDENTIFIER)
                return ParsedInput::make_error(redirect_token, TokenType::PATH);
            redirect_str = redirect_token.text;
        }
        return ParsedInput::make_good(
                UniquePointer<ASTNode>(
                        new CommandSequence(
                                move(path_or_identifier.ast_node),
                                move(args),
                                Path(redirect_str)
                        )
                )
        );
    }


    ParsedInput Parser::parse_argument() {
        Token identifier_path_or_string = _lexer.peek_token();
        switch (identifier_path_or_string.type) {
            case TokenType::IDENTIFIER:
                return parse_identifier();
            case TokenType::PATH:
                return parse_path();
            case TokenType::DOLLAR:
                return parse_env_var();
            case TokenType::QUOTE:
                return parse_string();
            case TokenType::DASH:
                return parse_flag();
            default: {
                return ParsedInput::make_error(identifier_path_or_string, TokenType::IDENTIFIER);
            }
        }
    }


    ParsedInput Parser::parse_flag() {
        Token dash               = _lexer.next_token();
        Token dash_or_identifier = _lexer.next_token();
        bool  double_dash        = false;
        if (dash.type == TokenType::DASH) {
            if (dash_or_identifier.type != TokenType::DASH && dash_or_identifier.type != TokenType::IDENTIFIER)
                return ParsedInput::make_error(dash_or_identifier, TokenType::IDENTIFIER);

            if (dash_or_identifier.type == TokenType::DASH) {
                dash_or_identifier = _lexer.next_token();
                double_dash        = true;
            }

        } else {
            return ParsedInput::make_error(dash, TokenType::DASH);
        }

        if (dash_or_identifier.type != TokenType::IDENTIFIER)
            return ParsedInput::make_error(dash_or_identifier, TokenType::IDENTIFIER);
        String dashes = double_dash ? "--" : "-";
        return ParsedInput::make_good(UniquePointer<ASTNode>(new IdentifierOrPath(dashes + dash_or_identifier.text)));
    }


    ParsedInput Parser::parse_env_var_declaration() {
        ParsedInput env_var = parse_env_var();
        if (env_var.has_error)
            return env_var;
        Token token = _lexer.next_token();
        if (token.type != TokenType::ASSIGNMENT)
            return ParsedInput::make_error(token, TokenType::ASSIGNMENT);

        LinkedList<UniquePointer<ASTNode>> value;
        Token                              t = _lexer.peek_token();
        while (t.type != TokenType::END) {
            switch (t.type) {
                case TokenType::IDENTIFIER: {
                    ParsedInput identifier = parse_identifier();
                    if (identifier.has_error)
                        return identifier;
                    value.add_back(move(identifier.ast_node));
                    break;
                }
                case TokenType::ESCAPE_CODE: {
                    ParsedInput escape_code = parse_escape_code();
                    if (escape_code.has_error)
                        return escape_code;
                    value.add_back(move(escape_code.ast_node));
                    break;
                }
                case TokenType::DOLLAR: {
                    ParsedInput more_env_var = parse_env_var();
                    if (more_env_var.has_error)
                        return more_env_var;
                    value.add_back(move(more_env_var.ast_node));
                    break;
                }
                case TokenType::PATH: {
                    ParsedInput path = parse_path();
                    if (path.has_error)
                        return path;
                    value.add_back(move(path.ast_node));
                    break;
                }
                case TokenType::QUOTE: {
                    ParsedInput sh_string = parse_string();
                    if (sh_string.has_error)
                        return sh_string;
                    value.add_back(move(sh_string.ast_node));
                    break;
                }
                default:
                    return ParsedInput::make_error(t, TokenType::IDENTIFIER);
            }
            t = _lexer.peek_token();
        }

        if (value.is_empty())
            return ParsedInput::make_error(_lexer.peek_token(), TokenType::IDENTIFIER);

        return ParsedInput::make_good(UniquePointer<ASTNode>(new EnvVarDecl(move(env_var.ast_node), move(value))));
    }


    ParsedInput Parser::parse_string() {
        Token opening_quote = _lexer.next_token();
        if (opening_quote.type != TokenType::QUOTE)
            return ParsedInput::make_error(opening_quote, TokenType::QUOTE);

        LinkedList<UniquePointer<ASTNode>> content;
        Token                              t = _lexer.peek_token();
        while (t.type != TokenType::QUOTE && t.type != TokenType::END) {
            switch (t.type) {
                case TokenType::IDENTIFIER: {
                    ParsedInput identifier = parse_identifier();
                    if (identifier.has_error)
                        return identifier;
                    content.add_back(move(identifier.ast_node));
                    break;
                }
                case TokenType::ESCAPE_CODE: {
                    ParsedInput escape_code = parse_escape_code();
                    if (escape_code.has_error)
                        return escape_code;
                    content.add_back(move(escape_code.ast_node));
                    break;
                }
                case TokenType::DOLLAR: {
                    ParsedInput env_var = parse_env_var();
                    if (env_var.has_error)
                        return env_var;
                    content.add_back(move(env_var.ast_node));
                    break;
                }
                case TokenType::PATH: {
                    ParsedInput path = parse_path();
                    if (path.has_error)
                        return path;
                    content.add_back(move(path.ast_node));
                    break;
                }
                default:
                    return ParsedInput::make_error(t, TokenType::IDENTIFIER);
            }
            t = _lexer.peek_token();
        }

        Token closing_quote = _lexer.next_token();
        if (opening_quote.type != TokenType::QUOTE)
            return ParsedInput::make_error(closing_quote, TokenType::QUOTE);

        return ParsedInput::make_good(UniquePointer<ASTNode>(new ShellString(move(content))));
    }


    ParsedInput Parser::parse_env_var() {
        Token token = _lexer.next_token();
        if (token.type != TokenType::DOLLAR)
            return ParsedInput::make_error(token, TokenType::DOLLAR);
        ParsedInput env_var = parse_identifier();
        if (env_var.has_error)
            return env_var;
        return ParsedInput::make_good(UniquePointer<ASTNode>(new EnvVar(move(env_var.ast_node))));
    }


    ParsedInput Parser::parse_path() {
        Token token = _lexer.next_token();
        if (token.type != TokenType::PATH)
            return ParsedInput::make_error(token, TokenType::PATH);
        return ParsedInput::make_good(UniquePointer<ASTNode>(new IdentifierOrPath(token.text)));
    }


    ParsedInput Parser::parse_identifier() {
        Token token = _lexer.next_token();
        if (token.type != TokenType::IDENTIFIER)
            return ParsedInput::make_error(token, TokenType::IDENTIFIER);
        return ParsedInput::make_good(UniquePointer<ASTNode>(new IdentifierOrPath(token.text)));
    }


    ParsedInput Parser::parse_escape_code() {
        Token token = _lexer.next_token();
        if (token.type != TokenType::ESCAPE_CODE)
            return ParsedInput::make_error(token, TokenType::ESCAPE_CODE);
        return ParsedInput::make_good(UniquePointer<ASTNode>(new IdentifierOrPath(String(token.text[1]))));
    }


    Parser::Parser() : _lexer("") {

    }


    ParsedInput Parser::parse_shell_input(const String& input) {
        _lexer = Lexer(input);
        return parse_input();
    }
}