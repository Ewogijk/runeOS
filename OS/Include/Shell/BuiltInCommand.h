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

#ifndef RUNEOS_BUILTINCOMMAND_H
#define RUNEOS_BUILTINCOMMAND_H


#include <StdIO.h>

#include <Shell/AST.h>

#include <Pickaxe/AppManagement.h>


namespace Rune::Shell {
    /**
     * @brief Change the working directory of the shell interpreter.
     * @param argc
     * @param argv
     * @param shell_env
     * @return
     */
    int cd(int argc, char* argv[], Environment& shell_env);

    /**
     * @brief Print the working directory of the shell interpreter.
     * @param argc
     * @param argv
     * @param shell_env
     * @return
     */
    int pwd(int argc, char* argv[], Environment& shell_env);


    /**
     * @brief Clear the terminal screen.
     * @param argc
     * @param argv
     * @param shell_env
     * @return
     */
    int clear(int argc, char* argv[], Environment& shell_env);

    /**
     * @brief Print the shell help menu.
     * @param argc
     * @param argv
     * @param shell_env
     * @return
     */
    int help(int argc, char* argv[], Environment& shell_env);


    /**
     * @brief Add all built-in shell commands to the command table of the shell env.
     * @param shell_env
     */
    void register_builtin_commands(Environment& shell_env);
}

#endif //RUNEOS_BUILTINCOMMAND_H
