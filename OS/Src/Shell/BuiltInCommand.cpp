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


#include <Build.h>


namespace Rune::Shell {
    HashMap<String, String> HELP_TEXT_TABLE;


    int cd(int argc, char* argv[], Environment& shell_env) {
        if (argc == 0)
            // Stay in the current directory
            return 0;
        if (argc > 1) {
            print_err("Error: Too many arguments.\n");
            return -1;
        }

        const char* new_wd = argv[0];
        S64 ret = Pickaxe::app_change_working_directory(new_wd);
        if (ret < 0) {
            switch (ret) {
                case -1:
                case -2:
                    print_err("'{}': Bad path.\n", new_wd);
                    break;
                case -3:
                    print_err("'{}': Directory not found.\n", new_wd);
                    break;
                case -4:
                    print_err("'{}': Not a directory.\n", new_wd);
                    break;
                default:
                    print_err("'{}': IO error.\n", new_wd);
            }
            return -1;
        }

        char new_wd_resolved[128];
        ret = Pickaxe::app_get_working_directory(new_wd_resolved, 128);
        if (ret < 0) {
            print_err("'{}': Bad path.\n");
            return -1;
        }

        shell_env.working_directory = Path(new_wd_resolved);
        return 0;
    }


    int pwd(int argc, char* argv[], Environment& shell_env) {
        SILENCE_UNUSED(argc)
        SILENCE_UNUSED(argv)
        print_out(shell_env.working_directory.to_string().to_cstr());
        print_out("\n");
        return 0;
    }


    int clear(int argc, char* argv[], Environment& shell_env) {
        SILENCE_UNUSED(argc)
        SILENCE_UNUSED(argv)
        SILENCE_UNUSED(shell_env)

        // ANSI Escape Code explanations
        // \033[3J   -> Clear the screen and scroll back buffer
        // \033[1;1H -> Set the cursor to the top left corner
        print_out("\033[3J\033[1;1H");
        return 0;
    }


    int help(int argc, char* argv[], Environment& shell_env) {
        SILENCE_UNUSED(shell_env)
        if (argc == 1) {
            String cmd            = argv[0];
            auto   maybe_help_txt = HELP_TEXT_TABLE.find(cmd);
            if (maybe_help_txt != HELP_TEXT_TABLE.end()) {
                print_out((*maybe_help_txt->value).to_cstr());
            } else {
                print_out("Unknown command: {}\n", cmd);
            }
            return 0;
        }

        String version = String::format("v{}.{}.{}", OS_MAJOR, OS_MINOR, OS_PATCH);
        if (!String(OS_PRERELEASE).is_empty())
            version += String::format("-{}", OS_PRERELEASE);
        print_out("runeOS {}\n\n", version);

        print_out("The shell executes built-in and external commands.\n");
        print_out("Scroll with arrow down/up through the command history.\n");
        print_out("Press 'Tab' to auto complete commands and file paths.\n");
        print_out(
                "External commands are applications found in '/Apps/' which in return are files with the '.app'"
                " extension.\n"
        );
        print_out("Type 'help [command]' to get more info about a built-in command, e.g. 'help cd'.\n\n");

        print_out("Following built-in commands are available:\n\n");
        print_out("    cd [directory]\n");
        print_out("    pwd\n");
        print_out("    clear\n");
        print_out("    help [command]\n");
        return 0;
    }


    void register_builtin_commands(Environment& shell_env) {
        shell_env.command_table.put("cd", &cd);
        shell_env.command_table.put("pwd", &pwd);
        shell_env.command_table.put("clear", &clear);
        shell_env.command_table.put("help", &help);

        HELP_TEXT_TABLE = HashMap<String, String>();
        HELP_TEXT_TABLE.put(
                "cd",
                "cd [directory]\n"
                "    Change the current directory of the shell to another directory.\n"
        );
        HELP_TEXT_TABLE.put(
                "pwd",
                "pwd\n"
                "    Print the current directory of the shell.\n"
        );
        HELP_TEXT_TABLE.put(
                "clear",
                "clear\n"
                "    Clear the screen and the terminal scroll back buffer.\n"
        );
        HELP_TEXT_TABLE.put(
                "help",
                "help [command]\n"
                "    Display information about the shell or a built-in command.\n"
        );
    }
}
