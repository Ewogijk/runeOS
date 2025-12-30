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

#include <Crucible/Parser.h>

#include <vector>

namespace Crucible {
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          ParsedInput
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    auto ParsedInput::make_good(std::unique_ptr<ASTNode> ast_node) -> ParsedInput {
        ParsedInput good;
        good.ast_node  = std::move(ast_node);
        good.has_error = false;
        good.actual    = {};
        good.expected  = TokenType::NONE;
        return good;
    }

    auto ParsedInput::make_error(const Token& actual, const TokenType expected) -> ParsedInput {
        ParsedInput err;
        err.ast_node  = std::unique_ptr<ASTNode>();
        err.has_error = true;
        err.actual    = actual;
        err.expected  = expected;
        return err;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Parser
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    auto Parser::parse_input() -> ParsedInput {
        switch (_lexer.peek_token().type) {
            case TokenType::PATH:
            case TokenType::IDENTIFIER: return parse_command_sequence();
            case TokenType::DOLLAR:     return parse_env_var_declaration();
            default:                    return ParsedInput::make_error(_lexer.peek_token(), TokenType::IDENTIFIER);
        }
    }

    auto Parser::parse_command_sequence() -> ParsedInput {
        ParsedInput path_or_identifier;
        if (_lexer.peek_token().type == TokenType::PATH)
            path_or_identifier = parse_path();
        else
            path_or_identifier = parse_identifier();

        if (path_or_identifier.has_error) return path_or_identifier;

        std::vector<std::unique_ptr<ASTNode>> args;
        Token                                 peek_a_boo = _lexer.peek_token();
        while (peek_a_boo.type != TokenType::REDIRECT && peek_a_boo.type != TokenType::END) {
            ParsedInput arg = parse_argument();
            if (arg.has_error) return arg;
            args.push_back(std::move(arg.ast_node));
            peek_a_boo = _lexer.peek_token();
        }
        std::string redirect_str;
        if (_lexer.peek_token().type == TokenType::REDIRECT) {
            _lexer.next_token();
            const Token redirect_token = _lexer.next_token();
            if (redirect_token.type != TokenType::PATH
                && redirect_token.type != TokenType::IDENTIFIER)
                return ParsedInput::make_error(redirect_token, TokenType::PATH);
            redirect_str = redirect_token.text;
        }
        return ParsedInput::make_good(
            std::make_unique<CommandSequence>(std::move(path_or_identifier.ast_node),
                                              std::move(args),
                                              Path(redirect_str)));
    }

    auto Parser::parse_argument() -> ParsedInput {
        switch (const Token identifier_path_or_string = _lexer.peek_token();
                identifier_path_or_string.type) {
            case TokenType::IDENTIFIER: return parse_identifier();
            case TokenType::PATH:       return parse_path();
            case TokenType::DOLLAR:     return parse_env_var();
            case TokenType::QUOTE:      return parse_string();
            case TokenType::DASH:       return parse_flag();
            default:                    {
                return ParsedInput::make_error(identifier_path_or_string, TokenType::IDENTIFIER);
            }
        }
    }

    auto Parser::parse_flag() -> ParsedInput {
        const Token dash               = _lexer.next_token();
        Token       dash_or_identifier = _lexer.next_token();
        bool        double_dash        = false;
        if (dash.type == TokenType::DASH) {
            if (dash_or_identifier.type != TokenType::DASH
                && dash_or_identifier.type != TokenType::IDENTIFIER)
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
        const std::string dashes = double_dash ? "--" : "-";
        return ParsedInput::make_good(
            std::make_unique<IdentifierOrPath>(dashes + dash_or_identifier.text));
    }

    auto Parser::parse_env_var_declaration() -> ParsedInput {
        ParsedInput env_var = parse_env_var();
        if (env_var.has_error) return env_var;
        if (Token token = _lexer.next_token(); token.type != TokenType::ASSIGNMENT)
            return ParsedInput::make_error(token, TokenType::ASSIGNMENT);

        std::vector<std::unique_ptr<ASTNode>> value;
        Token                                 t = _lexer.peek_token();
        while (t.type != TokenType::END) {
            switch (t.type) {
                case TokenType::IDENTIFIER: {
                    ParsedInput identifier = parse_identifier();
                    if (identifier.has_error) return identifier;
                    value.push_back(std::move(identifier.ast_node));
                    break;
                }
                case TokenType::ESCAPE_CODE: {
                    ParsedInput escape_code = parse_escape_code();
                    if (escape_code.has_error) return escape_code;
                    value.push_back(std::move(escape_code.ast_node));
                    break;
                }
                case TokenType::DOLLAR: {
                    ParsedInput more_env_var = parse_env_var();
                    if (more_env_var.has_error) return more_env_var;
                    value.push_back(std::move(more_env_var.ast_node));
                    break;
                }
                case TokenType::PATH: {
                    ParsedInput path = parse_path();
                    if (path.has_error) return path;
                    value.push_back(std::move(path.ast_node));
                    break;
                }
                case TokenType::QUOTE: {
                    ParsedInput sh_string = parse_string();
                    if (sh_string.has_error) return sh_string;
                    value.push_back(std::move(sh_string.ast_node));
                    break;
                }
                default: return ParsedInput::make_error(t, TokenType::IDENTIFIER);
            }
            t = _lexer.peek_token();
        }

        if (value.empty())
            return ParsedInput::make_error(_lexer.peek_token(), TokenType::IDENTIFIER);

        return ParsedInput::make_good(
            std::make_unique<EnvVarDecl>(std::move(env_var.ast_node), std::move(value)));
    }

    auto Parser::parse_string() -> ParsedInput {
        Token opening_quote = _lexer.next_token();
        if (opening_quote.type != TokenType::QUOTE)
            return ParsedInput::make_error(opening_quote, TokenType::QUOTE);

        std::vector<std::unique_ptr<ASTNode>> content;
        Token                                 t = _lexer.peek_token();
        while (t.type != TokenType::QUOTE && t.type != TokenType::END) {
            switch (t.type) {
                case TokenType::IDENTIFIER: {
                    ParsedInput identifier = parse_identifier();
                    if (identifier.has_error) return identifier;
                    content.push_back(std::move(identifier.ast_node));
                    break;
                }
                case TokenType::ESCAPE_CODE: {
                    ParsedInput escape_code = parse_escape_code();
                    if (escape_code.has_error) return escape_code;
                    content.push_back(std::move(escape_code.ast_node));
                    break;
                }
                case TokenType::DOLLAR: {
                    ParsedInput env_var = parse_env_var();
                    if (env_var.has_error) return env_var;
                    content.push_back(std::move(env_var.ast_node));
                    break;
                }
                case TokenType::PATH: {
                    ParsedInput path = parse_path();
                    if (path.has_error) return path;
                    content.push_back(std::move(path.ast_node));
                    break;
                }
                default: return ParsedInput::make_error(t, TokenType::IDENTIFIER);
            }
            t = _lexer.peek_token();
        }

        Token closing_quote = _lexer.next_token();
        if (opening_quote.type != TokenType::QUOTE)
            return ParsedInput::make_error(closing_quote, TokenType::QUOTE);

        return ParsedInput::make_good(std::make_unique<ShellString>(std::move(content)));
    }

    auto Parser::parse_env_var() -> ParsedInput {
        if (const Token token = _lexer.next_token(); token.type != TokenType::DOLLAR)
            return ParsedInput::make_error(token, TokenType::DOLLAR);
        ParsedInput env_var = parse_identifier();
        if (env_var.has_error) return env_var;
        return ParsedInput::make_good(std::make_unique<EnvVar>(std::move(env_var.ast_node)));
    }

    auto Parser::parse_path() -> ParsedInput {
        Token token = _lexer.next_token();
        if (token.type != TokenType::PATH) return ParsedInput::make_error(token, TokenType::PATH);
        return ParsedInput::make_good(std::make_unique<IdentifierOrPath>(token.text));
    }

    auto Parser::parse_identifier() -> ParsedInput {
        Token token = _lexer.next_token();
        if (token.type != TokenType::IDENTIFIER)
            return ParsedInput::make_error(token, TokenType::IDENTIFIER);
        return ParsedInput::make_good(std::make_unique<IdentifierOrPath>(token.text));
    }

    auto Parser::parse_escape_code() -> ParsedInput {
        const Token token = _lexer.next_token();
        if (token.type != TokenType::ESCAPE_CODE)
            return ParsedInput::make_error(token, TokenType::ESCAPE_CODE);
        return ParsedInput::make_good(
            std::make_unique<IdentifierOrPath>(std::string(1, token.text[1])));
    }

    Parser::Parser() : _lexer("") {}

    auto Parser::parse_shell_input(const std::string& input) -> ParsedInput {
        _lexer = Lexer(input);
        return parse_input();
    }
} // namespace Crucible
