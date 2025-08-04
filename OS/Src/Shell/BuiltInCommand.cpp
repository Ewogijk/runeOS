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

#include <Shell/BuiltInCommand.h>

#include <Forge/App.h>

#include <iostream>
#include <unordered_map>
#include <string>

#include <Build.h>


namespace Rune::Shell {
    std::unordered_map<std::string, std::string> HELP_TEXT_TABLE;


    int cd(const int argc, char* argv[], Environment& shell_env) {
        if (argc == 0)
            // Stay in the current directory
            return 0;
        if (argc > 1) {
            std::cerr << "Error: Too many arguments" << std::endl;
            return -1;
        }

        const char*       new_wd = argv[0];
        Ember::StatusCode ret    = Forge::app_change_directory(new_wd);
        if (ret < Ember::Status::OKAY) {
            switch (ret) {
                case Ember::Status::BAD_ARG:
                    std::cerr << "'" << new_wd << "': Bad Path." << std::endl;
                    break;
                case Ember::Status::NODE_NOT_FOUND:
                    std::cerr << "'" << new_wd << "': Directory not found." << std::endl;
                    break;
                case Ember::Status::NODE_IS_FILE:
                    std::cerr << "'" << new_wd << "': Not a directory." << std::endl;
                    break;
                default:
                    std::cerr << "'" << new_wd << "': IO error." << std::endl;
            }
            return -1;
        }

        char new_wd_resolved[128];
        ret = Forge::app_current_directory(new_wd_resolved, 128);
        if (ret < Ember::Status::OKAY) {
            std::cerr << "'" << new_wd << "': Bad path." << std::endl;
            return -1;
        }

        shell_env.working_directory = Path(new_wd_resolved);
        return 0;
    }


    int pwd(const int argc, char* argv[], const Environment& shell_env) {
        SILENCE_UNUSED(argc)
        SILENCE_UNUSED(argv)
        std::cout << shell_env.working_directory.to_string().c_str() << std::endl;
        return 0;
    }


    int clear(const int argc, char* argv[], const Environment& shell_env) {
        SILENCE_UNUSED(argc)
        SILENCE_UNUSED(argv)
        SILENCE_UNUSED(shell_env)

        // ANSI Escape Code explanations
        // \033[3J   -> Clear the screen and scroll back buffer
        // \033[1;1H -> Set the cursor to the top left corner
        std::cout << "\033[3J\033[1;1H";
        std::cout.flush();
        return 0;
    }


    int help(const int argc, char* argv[], const Environment& shell_env) {
        SILENCE_UNUSED(shell_env)
        if (argc == 1) {
            const std::string cmd            = argv[0];
            if (const auto maybe_help_txt = HELP_TEXT_TABLE.find(cmd); maybe_help_txt != HELP_TEXT_TABLE.end())
                std::cout << maybe_help_txt->second;
            else
                std::cout << "Unknown command: " << cmd << std::endl;
            return 0;
        }

        std::cout << "runeOS v" << MAJOR << "." << MINOR << "." << PATCH;
        if (!std::string(PRERELEASE).empty())
            std::cout << "-" << PRERELEASE << std::endl;
        std::cout << std::endl;

        std::cout << "The shell executes built-in and external commands." << std::endl;
        std::cout << "Scroll with arrow down/up through the command history." << std::endl;
        std::cout << "Press 'Tab' to auto complete commands and file paths." << std::endl;
        std::cout << "External commands are applications found in '/Apps/' which in return are files with the '.app'"
            " extension." << std::endl;
        std::cout << "Type 'help [command]' to get more info about a built-in command, e.g. 'help cd'." << std::endl;
        std::cout << std::endl;

        std::cout << "Following built-in commands are available:" << std::endl;
        std::cout << "    cd [directory]" << std::endl;
        std::cout << "    pwd" << std::endl;
        std::cout << "    clear" << std::endl;
        std::cout << "    help [command]" << std::endl;
        return 0;
    }


    void register_builtin_commands(Environment& shell_env) {

        shell_env.command_table["cd"] = &cd;
        shell_env.command_table["pwd"] = &pwd;
        shell_env.command_table["clear"] = &clear;
        shell_env.command_table["help"] = &help;

        HELP_TEXT_TABLE["cd"] =
            "cd [directory]\n"
            "    Change the current directory of the shell to another directory.\n";
        HELP_TEXT_TABLE["pwd"] =
            "pwd\n"
            "    Print the current directory of the shell.\n";
        HELP_TEXT_TABLE["clear"] =
            "clear\n"
            "    Clear the screen and the terminal scroll back buffer.\n";
        HELP_TEXT_TABLE["help"] =
            "help [command]\n"
            "    Display information about the shell or a built-in command.\n";
    }
}
