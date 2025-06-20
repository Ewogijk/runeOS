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


struct CLIArgs {
    Rune::String src_path  = "";
    Rune::String dest_path = "";

    bool help      = false;
    bool recursive = false;
};


bool parse_cli_args(int argc, char* argv[], CLIArgs& args_out) {
    bool     src_found  = false;
    bool     dest_found = false;
    for (int i          = 0; i < argc; i++) {
        Rune::String arg = argv[i];
        if (arg.size() == 0)
            continue;

        if (arg[0] == '-') {
            for (size_t j = 1; j < arg.size(); j++) {
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
            if (src_found && dest_found) {
                printl_err("Error: Unknown arg {}.", arg);
                return false;
            }
            if (!src_found) {
                args_out.src_path = arg;
                src_found = true;
            } else if (!dest_found) {
                args_out.dest_path = arg;
                dest_found = true;
            }
        }
    }
    if (!src_found && !args_out.help)
        printl_err("Error: Missing src argument.");
    else if (!dest_found && !args_out.help)
        printl_err("Error: Missing dest argument.");

    return (src_found && dest_found) || args_out.help;
}

/**
 * @brief Try to get the node info of the node path.
 * @param node
 * @param out  This will contain the node info if no error happens.
 * @return 1: Got the node info.
 *           0: The node does not exist.
 *          -1: An error happened.
 */
int get_node_info(const Rune::String& node, Rune::Pickaxe::VFSNodeInfo &out) {
    Rune::Pickaxe::VFSNodeInfo node_info;
    S64                         ret = Rune::Pickaxe::vfs_get_node_info(node.to_cstr(), &node_info);
    int function_ret = -1;
    switch (ret) {
        case 0:
            function_ret = 1;
            break;
        case -4:
            function_ret = 0;
            break;
        case -1: // Path too long
        case -2: // Illegal characters on path
        case -5: // Intermediate path element is a file
            printl_err("'{}': {} - Bad path.", node, ret);
            break;
        default:
            printl_err("'{}': {} - IO error.", node, ret);
    }
    out = node_info;
    return function_ret;
}


S64 open_node(const Rune::String& node_path, Rune::Pickaxe::NodeIOMode io_mode) {
    S64 file_handle = Rune::Pickaxe::vfs_open(node_path.to_cstr(), io_mode);
    if (file_handle < 0) {
        switch (file_handle) {
            case -4:
                printl_err("'{}': {} - File not found.", node_path, file_handle);
                break;
            case -1:
            case -3:
                printl_err("'{}': {} - Bad path.", node_path, file_handle);
                break;
            default:
                printl_err("'{}': {} - IO error.", node_path, file_handle);
                break;
        }
    }
    return file_handle;
}


bool create_node(const Rune::String& node_path, U8 attr) {
    S64 ret = Rune::Pickaxe::vfs_create(node_path.to_cstr(), attr);
    switch (ret) {
        case -1: // Bad buffer
        case -2: // Unknown attributes
        case -5: // Bad attributes
        case -6: // IO error
            printl_err("'{}': {} - IO error.", node_path, ret);
            break;
        case -3: // Bad Path
            printl_err("'{}': {} - Bad path.", node_path, ret);
            break;
        default:
            break; // Good cases: Node created or already exists
    }
    return ret >= 0;
}


void close_node(S64 node_handle) {
    if (node_handle <= 0)
        return; // Invalid node handle
    Rune::Pickaxe::vfs_close(node_handle);
    // Both possible return values are fine, no need for error handling
    //   0 -> Node was closed
    //  -2 -> No node with the handle was found
}


bool copy_file_content(const Rune::String& src, const Rune::String& dest) {
    Rune::String dest_node = dest;
    Rune::Pickaxe::VFSNodeInfo dest_node_info;
    if (get_node_info(dest, dest_node_info) < 0)
        return false;

    if (dest_node_info.is_directory()) {
        // dest is a directory -> append the src file name to dest
        // We know source is a file, therefore it must contain at least a valid filename -> parts.size() > 0
        dest_node += '/';
        dest_node += *src.split('/').tail();
    }
    if (!create_node(dest_node, (int) Rune::Pickaxe::NodeAttribute::FILE))
        return false;

    S64 src_file_handle = open_node(src, Rune::Pickaxe::NodeIOMode::READ);
    if (src_file_handle < 0)
        return false;
    S64 dest_file_handle = open_node(dest_node, Rune::Pickaxe::NodeIOMode::WRITE);
    if (dest_file_handle < 0)
        return false;

    U8  buf[Rune::Pickaxe::MAX_STRING_SIZE];
    S64 bytes_read = Rune::Pickaxe::vfs_read(src_file_handle, buf, Rune::Pickaxe::MAX_STRING_SIZE);
    while (bytes_read > 0) {
        S64 bytes_written = Rune::Pickaxe::vfs_write(dest_file_handle, buf, Rune::Pickaxe::MAX_STRING_SIZE);
        if (bytes_written < 0) {
            if (bytes_written == -5) {
                printl_err("'{}': {} - Writing the file is not supported.", dest_node, bytes_written);
            } else {
                printl_err("'{}': {} - IO error.", dest_node, bytes_written);
            }
            close_node(src_file_handle);
            close_node(dest_file_handle);
            return -1;
        }
        bytes_read = Rune::Pickaxe::vfs_read(src_file_handle, buf, Rune::Pickaxe::MAX_STRING_SIZE);
    }
    if (bytes_read < 0) {
        if (bytes_read == -5) {
            printl_err("'{}': {} - Reading the file is not supported.", src, bytes_read);
        } else {
            printl_err("'{}': {} - IO error.", src, bytes_read);
        }
    }
    close_node(src_file_handle);
    close_node(dest_file_handle);
    return bytes_read >= 0;
}


void close_dir_stream(S64 dir_stream_handle) {
    if (dir_stream_handle <= 0)
        return; // Invalid Dir stream handle
    Rune::Pickaxe::vfs_close(dir_stream_handle);
    // Both possible return values are fine, no need for error handling
    //   0 -> Dir stream was closed
    //  -2 -> No Dir stream with the handle was found
}


bool copy_dir_content(const Rune::String& src, const Rune::String& dest) {
    Rune::String dest_node = dest;
    Rune::Pickaxe::VFSNodeInfo dest_node_info;
    if (get_node_info(dest, dest_node_info) < 0)
        return false;

    if (dest_node_info.is_directory()) {
        // dest is a directory -> append the src file name to dest
        // We know source is a file, therefore it must contain at least a valid filename -> parts.size() > 0
        dest_node += '/';
        dest_node += *src.split('/').tail();
    }
    if (!create_node(dest_node, (int) Rune::Pickaxe::NodeAttribute::DIRECTORY))
        return false;

    S64 dir_stream_handle = Rune::Pickaxe::vfs_directory_stream_open(src.to_cstr());
    if (dir_stream_handle < 0) {
        printl_err("'{}': {} - IO error.", src, dir_stream_handle);
        Rune::Pickaxe::vfs_directory_stream_close(dir_stream_handle);
        return false;
    }

    Rune::Pickaxe::VFSNodeInfo node_info;
    S64                         next      = Rune::Pickaxe::vfs_directory_stream_next(dir_stream_handle, &node_info);
    Rune::String               node_path = node_info.node_path;
    while (next > 0) {
        if (node_path != "." && node_path != "..") {
            Rune::String src_node_to_cp = src + '/' + node_path;
            Rune::String dest_node_to_cp = dest_node + '/' + node_path;
            if (node_info.is_directory()) {


                if (!copy_dir_content(src_node_to_cp, dest_node_to_cp)) {
                    close_dir_stream(dir_stream_handle);
                    return false;
                }
            } else {
                if (!copy_file_content(src_node_to_cp, dest_node_to_cp)) {
                    close_dir_stream(dir_stream_handle);
                    return false;
                }
            }
        }
        next      = Rune::Pickaxe::vfs_directory_stream_next(dir_stream_handle, &node_info);
        node_path = node_info.node_path;
    }
    if (node_path != "." && node_path != "..") {
        // Copy last node
        Rune::String src_node_to_cp = src + '/' + node_path;
        Rune::String dest_node_to_cp = dest_node + '/' + node_path;
        if (node_info.is_directory()) {
            if (!copy_dir_content(src_node_to_cp, dest_node_to_cp)) {
                close_dir_stream(dir_stream_handle);
                return false;
            }
        } else {
            if (!copy_file_content(src_node_to_cp, dest_node_to_cp)) {
                close_dir_stream(dir_stream_handle);
                return false;
            }
        }
    }
    close_dir_stream(dir_stream_handle);
    return true;
}


CLINK int main(int argc, char* argv[]) {
    CLIArgs args;
    if (!parse_cli_args(argc, argv, args))
        return -1;

    if (args.help) {
        printl_out("cp [src] [dest] [options]...");
        printl_out("    Copy the src file/directory to dest.");
        printl_out("Options:");
        printl_out("    -h: Print this help menu.");
        printl_out("    -r: Copy the content of the src directory recursively.");
        return 0;
    }

    Rune::Pickaxe::VFSNodeInfo node_info;
    int ret = get_node_info(args.src_path, node_info);
    if (ret < 1) {
        if (ret == 0)
            printl_err("'{}': File not found.", args.src_path);
        return -1;
    }

    if (node_info.is_file()) {
        if (!copy_file_content(args.src_path, args.dest_path))
            return -1;
    } else {
        if (args.recursive)
            copy_dir_content(args.src_path, args.dest_path);
        else
            printl_out("'{}': Cannot copy directory. Use -r to copy the directory and its content.", args.src_path);
    }
    return 0;
}