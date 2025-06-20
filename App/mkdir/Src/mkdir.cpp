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

#include <Hammer/Definitions.h>
#include <Hammer/String.h>

#include <Pickaxe/AppManagement.h>
#include <Pickaxe/VFS.h>

template<typename... Args>
void printl_out(const char* fmt, Args... args) {
    Rune::Argument arg_array[] = { args... };
    char            b[128];
    memset(b, 0, 128);
    int s = Rune::interpolate(fmt, b, 128, arg_array, sizeof...(Args));
    Rune::Pickaxe::write_std_out((const char*) b, s);
    Rune::Pickaxe::write_std_out("\n", 1);
}


template<typename... Args>
void printl_err(const char* fmt, Args... args) {
    Rune::Argument arg_array[] = { args... };
    char            b[128];
    memset(b, 0, 128);
    int s = Rune::interpolate(fmt, b, 128, arg_array, sizeof...(Args));
    Rune::Pickaxe::write_std_err((const char*) b, s);
    Rune::Pickaxe::write_std_err("\n", 1);
}


size_t get_cstr_size(const char* c_str) {
    size_t size = 0;
    const char* c_pos = c_str;
    while (*c_pos) {
        c_pos++;
        size++;
    }
    return size;
}


struct CLIArgs {
    static constexpr U8 MAX_PATH_SIZE = 128;
    char                node_path[128]     = { };

    bool help = false;
};


bool parse_cli_args(int argc, char* argv[], CLIArgs& args_out) {
    bool     node_path_seen = false;
    for (int i         = 0; i < argc; i++) {
        const char* arg = argv[i];
        size_t size = get_cstr_size(arg);
        if (size == 0)
            continue;

        if (arg[0] == '-') {
            for (size_t j = 1; j < size; j++) {
                switch (arg[j]) {
                    case 'h':
                        args_out.help = true;
                        break;
                    default: {
                        printl_err("Error: Unknown option {}.", argv[i][j]);
                        return false;
                    }
                }
            }
        } else {
            if (node_path_seen) {
                printl_err("Error: Unknown arg {}.", arg);
                return false;
            }
            memcpy(args_out.node_path, (char*) arg, get_cstr_size(arg));
            node_path_seen = true;
        }
    }
    if (!node_path_seen && !args_out.help)
        printl_err("Error: Missing node path argument.");
    return node_path_seen || args_out.help;
}


CLINK int main(int argc, char* argv[]) {
    CLIArgs args;
    if (!parse_cli_args(argc, argv, args))
        return -1;

    if (args.help) {
        printl_out("mkdir [file] [options]...");
        printl_out("    Create a directory.");
        printl_out("Options:");
        printl_out("    -h: Print this help menu.");
        return 0;
    }


    S64 ret = Rune::Pickaxe::vfs_create(args.node_path, (int) Rune::Pickaxe::NodeAttribute::DIRECTORY);
    if (ret < 0) {
        switch (ret) {
            case -4:
                printl_err("Error: '{}' - Exists already.", args.node_path);
                break;
            case -1:
            case -3:
                printl_err("Error: '{}' - Bad path.", args.node_path);
                break;
            default:
                printl_err("Error: '{}' - IO error.", args.node_path);
                break;
        }
    }
    return 0;
}