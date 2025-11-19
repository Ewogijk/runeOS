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

#ifndef CRUCIBLE_AST_H
#define CRUCIBLE_AST_H

#include <Crucible/Environment.h>

#include <memory>
#include <string>
#include <vector>

namespace Crucible {

    /**
     * @brief A node in the abstract syntax tree of the shell interpreter that represents
     *          commands, environment variables, arguments, etc.
     */
    class ASTNode {
      public:
        virtual ~ASTNode() = default;

        /**
         * @brief
         * @return The text content without any reserved character e.g. $stuff -> stuff, 'hi 123' ->
         * hi 123
         */
        virtual std::string get_text() = 0;

        /**
         * @brief Evaluate the node which could mean resolving an environment variable or executing
         * a command.
         * @param shell_env Environment of the running shell.
         * @return A string representation of the evaluated node.
         */
        virtual std::string evaluate(Environment& shell_env) = 0;
    };

    /**
     * @brief User input for the shell interpreter e.g. "foo a 1 2 3", "$env_var=value", etc.
     */
    class Input final : public ASTNode {
        std::unique_ptr<ASTNode> _cs_evd_or_ev;

      public:
        explicit Input(std::unique_ptr<ASTNode> cs_evd_or_ev);

        std::string get_text() override;

        std::string evaluate(Environment& shell_env) override;
    };

    /**
     * @brief A command sequence represents a built-in or external command and all arguments e.g.
     * "foo a 1 2 3".
     */
    class CommandSequence final : public ASTNode {
        /**
         * @brief The maximum size of all command line arguments in byte.
         */
        static constexpr int ARGV_LIMIT = 2048;

        std::unique_ptr<ASTNode>              _command;
        std::vector<std::unique_ptr<ASTNode>> _arguments_or_flags;
        Path                                  _redirect_file;

      public:
        explicit CommandSequence(std::unique_ptr<ASTNode>              command,
                                 std::vector<std::unique_ptr<ASTNode>> arguments_or_flags,
                                 Path                                  redirect_file);

        std::string get_text() override;

        std::string evaluate(Environment& shell_env) override;
    };

    /**
     * @brief An environment variable declaration e.g. $key=value, $key2='more value'
     */
    class EnvVarDecl final : public ASTNode {
        std::unique_ptr<ASTNode>              _env_var;
        std::vector<std::unique_ptr<ASTNode>> _value;

      public:
        explicit EnvVarDecl(std::unique_ptr<ASTNode>              env_var,
                            std::vector<std::unique_ptr<ASTNode>> value);

        std::string get_text() override;

        std::string evaluate(Environment& shell_env) override;
    };

    /**
     * @brief An environment variable declaration e.g. $key=value, $key2='more value'
     */
    class EnvVar final : public ASTNode {
        std::unique_ptr<ASTNode> _env_var;

      public:
        explicit EnvVar(std::unique_ptr<ASTNode> env_var);

        std::string get_text() override;

        std::string evaluate(Environment& shell_env) override;
    };

    /**
     * @brief A string with its individual components, e.g. 'A $cool \$string\$!!!'
     */
    class ShellString final : public ASTNode {
        std::vector<std::unique_ptr<ASTNode>> _content;

      public:
        explicit ShellString(std::vector<std::unique_ptr<ASTNode>> content);

        std::string get_text() override;

        std::string evaluate(Environment& shell_env) override;
    };

    /**
     * @brief An identifier or path e.g. a/b or Hi123.
     */
    class IdentifierOrPath final : public ASTNode {
        std::string _value;

      public:
        explicit IdentifierOrPath(const std::string& value);

        std::string get_text() override;

        std::string evaluate(Environment& shell_env) override;
    };
} // namespace Rune::Shell

#endif // CRUCIBLE_AST_H
