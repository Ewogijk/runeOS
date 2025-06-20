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

#include <Hammer//Definitions.h>
#include <Hammer/String.h>

#include <Pickaxe/AppManagement.h>
#include <Pickaxe/VFS.h>


template<typename... Args>
void print_out(const char* fmt, Args... args) {
    Rune::Argument arg_array[] = { args... };
    char            b[128];
    memset(b, 0, 128);
    int s = Rune::interpolate(fmt, b, 128, arg_array, sizeof...(Args));
    Rune::Pickaxe::write_std_out((const char*) b, s);
}

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
    char                file[128]     = { };

    bool help = false;
};


bool parse_cli_args(int argc, char* argv[], CLIArgs& args_out) {
    bool     file_seen = false;
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
            if (file_seen) {
                printl_err("Error: Unknown arg {}.", arg);
                return false;
            }
            memcpy(args_out.file, (char*) arg, get_cstr_size(arg));
            file_seen = true;
        }
    }
    if (!file_seen && !args_out.help)
        printl_err("Error: Missing file argument.");
    return file_seen || args_out.help;
}


CLINK int main(int argc, char* argv[]) {
    CLIArgs args;
    if (!parse_cli_args(argc, argv, args))
        return -1;

    if (args.help) {
        printl_out("cat [file] [options]...");
        printl_out("    Print file content to standard output.");
        printl_out("Options:");
        printl_out("    -h: Print this help menu.");
        return 0;
    }
    S64 file_handle = Rune::Pickaxe::vfs_open(args.file, Rune::Pickaxe::NodeIOMode::READ);
    if (file_handle < 0) {
        switch (file_handle) {
            case -4:
                printl_err("Error: '{}' - File not found.", args.file);
                break;
            case -1:
            case -3:
                printl_err("Error: '{}' - Bad path.", args.file);
                break;
            default:
                printl_err("Error: '{}' - IO error.", args.file);
                break;
        }
        return -1;
    }

    U8  buf[Rune::Pickaxe::MAX_STRING_SIZE];
    S64 bytes_read = Rune::Pickaxe::vfs_read(file_handle, buf, Rune::Pickaxe::MAX_STRING_SIZE - 1);
    while (bytes_read > 0) {
        buf[Rune::Pickaxe::MAX_STRING_SIZE - 1] = 0;
        print_out((const char*) buf);
        bytes_read = Rune::Pickaxe::vfs_read(file_handle, buf, Rune::Pickaxe::MAX_STRING_SIZE - 1);
    }
    if (bytes_read < 0) {
        switch (bytes_read) {
            case -3:
            case -5:
                printl_err("Error: '{}' - Not a file.", args.file);
                break;
            default:
                printl_err("Error: '{}' - IO error.", args.file);
                break;
        }
        Rune::Pickaxe::vfs_close(file_handle);
        return -1;
    }
    Rune::Pickaxe::vfs_close(file_handle);
    return 0;
}