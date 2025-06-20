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

#ifndef RUNEOS_AST_H
#define RUNEOS_AST_H


#include <Hammer/String.h>
#include <Hammer/Collection.h>
#include <Hammer/Utility.h>

#include <Shell/Environment.h>


namespace Rune::Shell {

    /**
     * @brief A node in the abstract syntax tree of the shell interpreter that represents
     *          commands, environment variables, arguments, etc.
     */
    class ASTNode {
    public:
        virtual ~ASTNode() = default;


        /**
         * @brief
         * @return The text content without any reserved character e.g. $stuff -> stuff, 'hi 123' -> hi 123
         */
        virtual String get_text() = 0;


        /**
         * @brief Evaluate the node which could mean resolving an environment variable or executing a command.
         * @param shell_env Environment of the running shell.
         * @return A string representation of the evaluated node.
         */
        virtual String evaluate(Environment& shell_env) = 0;
    };


    /**
     * @brief User input for the shell interpreter e.g. "foo a 1 2 3", "$env_var=value", etc.
     */
    class Input : public ASTNode {
        UniquePointer<ASTNode> _cs_evd_or_ev;

    public:
        explicit Input(UniquePointer<ASTNode> cs_evd_or_ev);


        String get_text() override;


        String evaluate(Environment& shell_env) override;
    };


    /**
     * @brief A command sequence represents a built-in or external command and all arguments e.g. "foo a 1 2 3".
     */
    class CommandSequence : public ASTNode {
        static constexpr int ARGV_SIZE = 2048;

        UniquePointer<ASTNode>             _command;
        LinkedList<UniquePointer<ASTNode>> _arguments_or_flags;
        Path _redirect_file;
    public:
        explicit CommandSequence(
                UniquePointer<ASTNode> command,
                LinkedList<UniquePointer<ASTNode>> arguments_or_flags,
                Path redirect_file
        );


        String get_text() override;


        String evaluate(Environment& shell_env) override;
    };


    /**
     * @brief An environment variable declaration e.g. $key=value, $key2='more value'
     */
    class EnvVarDecl : public ASTNode {
        UniquePointer<ASTNode>             _env_var;
        LinkedList<UniquePointer<ASTNode>> _value;
    public:
        explicit EnvVarDecl(UniquePointer<ASTNode> env_var, LinkedList<UniquePointer<ASTNode>> value);


        String get_text() override;


        String evaluate(Environment& shell_env) override;
    };


    /**
     * @brief An environment variable declaration e.g. $key=value, $key2='more value'
     */
    class EnvVar : public ASTNode {
        UniquePointer<ASTNode> _env_var;
    public:
        explicit EnvVar(UniquePointer<ASTNode> env_var);


        String get_text() override;


        String evaluate(Environment& shell_env) override;
    };


    /**
     * @brief A string with its individual components, e.g. 'A $cool \$string\$!!!'
     */
    class ShellString : public ASTNode {
        LinkedList<UniquePointer<ASTNode>> _content;
    public:
        explicit ShellString(LinkedList<UniquePointer<ASTNode>> content);


        String get_text() override;


        String evaluate(Environment& shell_env) override;
    };


    /**
     * @brief An identifier or path e.g. a/b or Hi123.
     */
    class IdentifierOrPath : public ASTNode {
        String _value;
    public:
        explicit IdentifierOrPath(const String& value);


        String get_text() override;


        String evaluate(Environment& shell_env) override;
    };
}

#endif //RUNEOS_AST_H
