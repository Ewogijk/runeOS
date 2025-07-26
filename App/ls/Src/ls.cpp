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

#include <Ember/Definitions.h>
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
    char                dir[128]      = { };

    bool help = false;
    bool all  = false;
    bool list = false;
};


bool parse_cli_args(int argc, char* argv[], CLIArgs& args_out) {
    bool     dir_seen = false;
    for (int i        = 0; i < argc; i++) {
        const char* arg = argv[i];
        size_t size = get_cstr_size(arg);
        if (size == 0)
            continue;

        if (arg[0] == '-') {
            for (size_t j = 1; j < size; j++) {
                switch (arg[j]) {
                    case 'a':
                        args_out.all = true;
                        break;
                    case 'l':
                        args_out.list = true;
                        break;
                    case 'h':
                        args_out.help = true;
                        break;
                    default: {
                        printl_err("Unknown option: {}", argv[i][j]);
                        return false;
                    }
                }
            }
        } else {
            if (dir_seen) {
                printl_err("Unknown arg: {}", arg);
                return false;
            }
            memcpy(args_out.dir, (char*) arg, get_cstr_size(arg));
            dir_seen = true;
        }
    }

    if (!dir_seen) {
        S64 ret = Rune::Pickaxe::app_get_working_directory(args_out.dir, CLIArgs::MAX_PATH_SIZE);
        if (ret < 0) {
            printl_err("IO error: Could not get current directory.\n");
            return false;
        }
    }
    return true;
}


void print_node_info(const CLIArgs& args, const Rune::Pickaxe::VFSNodeInfo& node_info) {
    Rune::String node_path = node_info.node_path;
    if (args.all || (!node_info.is_hidden() && node_path != "." && node_path != "..")) {
        if (args.list) {
            char attr[4] = { '-', '-', '-', '-' };
            if (node_info.is_file())
                attr[0] = 'F';
            else
                attr[0] = 'D';
            if (node_info.is_hidden())
                attr[1] = 'H';
            if (node_info.is_system_node()) {
                attr[2] = 'S';
            }
            if (node_info.is_readonly()) {
                attr[3] = 'R';
            }
            printl_out("{:<10} {:<15} {}", attr, node_info.size, node_info.node_path);
        } else {
            printl_out("{} ", node_info.node_path);
        }
    }
}


CLINK int main(int argc, char* argv[]) {
    CLIArgs args;
    if (!parse_cli_args(argc, argv, args))
        return -1;

    if (args.help) {
        printl_out("ls [directory] [options]...");
        printl_out("    List the content of a directory");
        printl_out("Options:");
        printl_out("    -a: Include hidden files.");
        printl_out("    -h: Print this help menu.");
        printl_out("    -l: Print detailed information about each node.");
        return 0;
    }

    S64 dir_stream_handle = Rune::Pickaxe::vfs_directory_stream_open(args.dir);
    if (dir_stream_handle < 0) {
        switch (dir_stream_handle) {
            case -4:
                printl_err("'{}': Directory not found.", args.dir);
                break;
            case -5:
                printl_err("'{}': Not a directory.", args.dir);
                break;
            default:
                printl_err("'{}': IO error occurred.", args.dir);
        }
        return -1;
    }


    if (args.list)
        printl_out("Attributes Size            Name");

    Rune::Pickaxe::VFSNodeInfo node_info;
    S64                         next = Rune::Pickaxe::vfs_directory_stream_next(dir_stream_handle, &node_info);
    while (next > 0) {
        print_node_info(args, node_info);
        next = Rune::Pickaxe::vfs_directory_stream_next(dir_stream_handle, &node_info);
    }
    print_node_info(args, node_info); // Print the last node
    return 0;
}