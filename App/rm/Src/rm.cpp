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
    static constexpr U8 MAX_PATH_SIZE  = 128;
    char                node_path[128] = { };

    bool help      = false;
    bool recursive = false;
};


bool parse_cli_args(int argc, char* argv[], CLIArgs& args_out) {
    bool     node_path_seen = false;
    for (int i              = 0; i < argc; i++) {
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
                    case 'r':
                        args_out.recursive = true;
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
        printl_err("Error: Missing path argument.");
    return node_path_seen || args_out.help;
}


bool delete_node(const Rune::String& node_path) {
    S64 ret = Rune::Pickaxe::vfs_delete(node_path.to_cstr());
    if (ret < 0) {
        if (ret == -3)
            printl_err("'{}': Cannot delete, because the node is used by another app.", node_path);
        else
            printl_err("'{}': IO error.", node_path);
        return false;
    }
    return true;
}


bool delete_dir(const Rune::String& directory_path) {
    S64 dir_stream_handle = Rune::Pickaxe::vfs_directory_stream_open(directory_path.to_cstr());
    if (dir_stream_handle < 0) {
        printl_err("'{}': IO error occurred.", directory_path);
        Rune::Pickaxe::vfs_directory_stream_close(dir_stream_handle);
        return false;
    }

    Rune::Pickaxe::VFSNodeInfo node_info;
    S64                         next      = Rune::Pickaxe::vfs_directory_stream_next(dir_stream_handle, &node_info);
    Rune::String               node_path = node_info.node_path;
    while (next > 0) {
        if (node_path != "." && node_path != "..") {
            if (node_info.is_directory()) {
                if (!delete_dir(directory_path + "/" + node_path)) {
                    Rune::Pickaxe::vfs_directory_stream_close(dir_stream_handle);
                    return false;
                }
            } else {
                if (!delete_node(directory_path + "/" + node_path)) {
                    Rune::Pickaxe::vfs_directory_stream_close(dir_stream_handle);
                    return false;
                }
            }
        }
        next      = Rune::Pickaxe::vfs_directory_stream_next(dir_stream_handle, &node_info);
        node_path = node_info.node_path;
    }
    if (node_path != "." && node_path != "..") {
        // Delete last node
        if (node_info.is_directory()) {
            if (!delete_dir(directory_path + "/" + node_path)) {
                Rune::Pickaxe::vfs_directory_stream_close(dir_stream_handle);
                return false;
            }
        } else {
            if (!delete_node(directory_path + "/" + node_path)) {
                Rune::Pickaxe::vfs_directory_stream_close(dir_stream_handle);
                return false;
            }
        }
    }


    // Delete this directory
    if (!delete_node(directory_path)) {
        Rune::Pickaxe::vfs_directory_stream_close(dir_stream_handle);
        return false;
    }
    Rune::Pickaxe::vfs_directory_stream_close(dir_stream_handle);
    return true;
}


CLINK int main(int argc, char* argv[]) {
    CLIArgs args;
    if (!parse_cli_args(argc, argv, args))
        return -1;

    if (args.help) {
        printl_out("rm [file|directory] [options]...");
        printl_out("    Delete a file or directory.");
        printl_out("Options:");
        printl_out("    -r: Remove the directory and all its content recursively.");
        printl_out("    -h: Print this help menu.");
        return 0;
    }

    Rune::Pickaxe::VFSNodeInfo node_info;
    S64                         ret = Rune::Pickaxe::vfs_get_node_info(args.node_path, &node_info);
    if (ret < 0) {
        switch (ret) {
            case -4:
                printl_err("'{}': File or directory not found.", args.node_path);
                break;
            case -1: // Path too long
            case -2: // Illegal characters on path
            case -5: // Intermediate path element is a file
                printl_err("'{}': Bad path.", args.node_path);
                break;
            default:
                printl_err("'{}': IO error.", args.node_path);
        }
        return -1;
    }

    if (node_info.is_file()) {
        // Node is a file -> Can just delete it
        if (!delete_node(args.node_path))
            return -1;
    } else {
        S64 dir_stream_handle = Rune::Pickaxe::vfs_directory_stream_open(args.node_path);
        if (dir_stream_handle < 0) {
            // We know node path arg is fine, the node is a dir and exists -> Can only be IO error
            printl_err("'{}': IO error occurred.", args.node_path);
            Rune::Pickaxe::vfs_directory_stream_close(dir_stream_handle);
            return -1;
        }
        size_t        nodes_in_dir = 0;
        S64           next         = Rune::Pickaxe::vfs_directory_stream_next(dir_stream_handle, &node_info);
        Rune::String node_path    = node_info.node_path;
        while (next > 0) {
            if (node_path != "." && node_path != "..")
                nodes_in_dir++;
            next      = Rune::Pickaxe::vfs_directory_stream_next(dir_stream_handle, &node_info);
            node_path = node_info.node_path;
        }
        if (node_path != "." && node_path != ".." && (node_info.is_directory() || node_info.is_file()))
            nodes_in_dir++;

        if (nodes_in_dir == 0) {
            if (!delete_node(args.node_path)) {
                Rune::Pickaxe::vfs_directory_stream_close(dir_stream_handle);
                return -1;
            }
        } else {
            if (!args.recursive) {
                printl_out(
                        "'{}': Cannot delete, directory is not empty. Use -r to delete the "
                        "directory and all its content.",
                        args.node_path
                );
                Rune::Pickaxe::vfs_directory_stream_close(dir_stream_handle);
                return -1;
            }

            if (!delete_dir(args.node_path)) {
                Rune::Pickaxe::vfs_directory_stream_close(dir_stream_handle);
                return -1;
            }
        }
        Rune::Pickaxe::vfs_directory_stream_close(dir_stream_handle);
    }

    return 0;
}