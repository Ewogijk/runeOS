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

#include <Forge/VFS.h>

#include <string>
#include <iostream>
#include <vector>
#include <sstream>

constexpr U16 BUF_SIZE = 4096;


std::vector<std::string> str_split(const std::string& s, const char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream       tokenStream(s);
    std::string              token;
    while (std::getline(tokenStream, token, delimiter)) tokens.push_back(token);
    return tokens;
}


struct CLIArgs {
    std::string src_path  = "";
    std::string dest_path = "";

    bool help      = false;
    bool recursive = false;
};


bool parse_cli_args(const int argc, char* argv[], CLIArgs& args_out) {
    bool src_found  = false;
    bool dest_found = false;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
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
                        std::cerr << "Unknown option '" << arg << "'" << std::endl;
                        return false;
                    }
                }
            }
        } else {
            if (src_found && dest_found) {
                std::cerr << "Unknown argument '" << arg << "'" << std::endl;
                return false;
            }
            if (!src_found) {
                args_out.src_path = arg;
                src_found         = true;
            } else if (!dest_found) {
                args_out.dest_path = arg;
                dest_found         = true;
            }
        }
    }
    if (!src_found && !args_out.help)
        std::cerr << "Missing src argument" << std::endl;
    else if (!dest_found && !args_out.help)
        std::cerr << "Missing dest argument" << std::endl;

    return (src_found && dest_found) || args_out.help;
}


/**
 * @brief Try to get the node info of the node path.
 * @param node Path to a node.
 * @param out  This will contain the node info if no error happens.
 * @return 1: Got the node info.
 *           0: The node does not exist.
 *          -1: An error happened.
 */
int get_node_info(const std::string& node, Ember::NodeInfo& out) {
    Ember::NodeInfo         node_info;
    const Ember::StatusCode ret          = Forge::vfs_get_node_info(node.c_str(), &node_info);
    int                     function_ret = -1;
    switch (ret) {
        case Ember::Status::OKAY:
            function_ret = 1;
            break;
        case Ember::Status::NODE_NOT_FOUND:
            function_ret = 0;
            break;
        case Ember::Status::BAD_ARG:
            std::cerr << "'" << node << "': Bad path." << std::endl;
            break;
        default:
            std::cerr << "'" << node << "': IO error." << std::endl;
    }
    out = node_info;
    return function_ret;
}


Ember::StatusCode open_node(const std::string& node_path, const Ember::IOMode io_mode) {
    const Ember::StatusCode file_ID = Forge::vfs_open(node_path.c_str(), io_mode);
    if (file_ID < Ember::Status::OKAY) {
        switch (file_ID) {
            case Ember::Status::NODE_NOT_FOUND:
                std::cerr << "'" << node_path << "': Node not found." << std::endl;
                break;
            case Ember::Status::BAD_ARG:
                std::cerr << "'" << node_path << "': Bad path." << std::endl;
                break;
            default:
                std::cerr << "'" << node_path << "': IO error." << std::endl;
                break;
        }
    }
    return file_ID;
}


bool create_node(const std::string& node_path, const U8 attr) {
    const Ember::StatusCode ret = Forge::vfs_create(node_path.c_str(), attr);
    switch (ret) {
        case Ember::Status::BAD_ARG:
            std::cerr << "'" << node_path << "': Bad path." << std::endl;
            break;
        case Ember::Status::NODE_EXISTS:
            std::cerr << "'" << node_path << "': Node exists." << std::endl;
            break;
        case Ember::Status::IO_ERROR:
            std::cerr << "'" << node_path << "': IO error." << std::endl;
            break;
        default:
            break; // Node is created.
    }
    return ret >= 0;
}


void close_node(const Ember::StatusCode node_ID) {
    if (node_ID <= 0)
        return; // Invalid node ID
    Forge::vfs_close(node_ID);
    // Both possible return values are fine, no need for error handling
    //   Ember::Status::OKAY -> Node was closed
    //   Ember::Status::UNKNOWN_ID -> No node with the ID was found
}


bool copy_file_content(const std::string& src, const std::string& dest) {
    std::string     dest_node = dest;
    Ember::NodeInfo dest_node_info;
    if (get_node_info(dest, dest_node_info) < 0)
        return false;

    if (dest_node_info.is_directory()) {
        // dest is a directory -> append the src file name to dest
        // We know source is a file, therefore it must contain at least a valid filename -> parts.size() > 0
        dest_node += '/';
        dest_node += str_split(src, '/').back();
    }
    if (!create_node(dest_node, Ember::NodeAttribute::FILE))
        return false;

    const Ember::StatusCode src_file_ID = open_node(src, Ember::IOMode::READ);
    if (src_file_ID < Ember::Status::OKAY)
        return false;
    const Ember::StatusCode dest_file_ID = open_node(dest_node, Ember::IOMode::WRITE);
    if (dest_file_ID < Ember::Status::OKAY)
        return false;

    U8                buf[BUF_SIZE];
    Ember::StatusCode bytes_read = Forge::vfs_read(src_file_ID, buf, BUF_SIZE);
    while (bytes_read > 0) {
        if (const Ember::StatusCode bytes_written = Forge::vfs_write(dest_file_ID, buf, BUF_SIZE);
            bytes_written < 0) {
            if (bytes_written == Ember::Status::NODE_IS_DIRECTORY) {
                std::cerr << "'" << dest_node << "': Not a file." << std::endl;
            } else {
                std::cerr << "'" << dest_node << "': IO Error." << std::endl;
            }
            close_node(src_file_ID);
            close_node(dest_file_ID);
            return false;
        }
        bytes_read = Forge::vfs_read(src_file_ID, buf, BUF_SIZE);
    }
    if (bytes_read < 0) {
        if (bytes_read == Ember::Status::NODE_IS_DIRECTORY) {
            std::cerr << "'" << src << "': Not a file." << std::endl;
        } else {
            std::cerr << "'" << src << "': IO Error." << std::endl;
        }
    }
    close_node(src_file_ID);
    close_node(dest_file_ID);
    return bytes_read >= 0;
}


void close_dir_stream(const S64 dir_stream_ID) {
    if (dir_stream_ID <= 0)
        return; // Invalid stream ID
    Forge::vfs_directory_stream_close(dir_stream_ID);
    // Both possible return values are fine, no need for error handling
    //   Ember::Status::OKAY -> Stream was closed
    //   Ember::Status::UNKNOWN_ID -> No stream with the ID was found
}


bool copy_dir_content(const std::string& src, const std::string& dest) {
    std::string     dest_node = dest;
    Ember::NodeInfo dest_node_info;
    if (get_node_info(dest, dest_node_info) < 0)
        return false;

    if (dest_node_info.is_directory()) {
        // dest is a directory -> append the src file name to dest
        // We know source is a file, therefore it must contain at least a valid filename -> parts.size() > 0
        dest_node += '/';
        dest_node += str_split(src, '/').back();
    }
    if (!create_node(dest_node, Ember::NodeAttribute::DIRECTORY))
        return false;

    const Ember::StatusCode dir_stream_ID = Forge::vfs_directory_stream_open(src.c_str());
    if (dir_stream_ID < 0) {
        std::cerr << "'" << src << "': IO Error." << std::endl;
        close_dir_stream(dir_stream_ID);
        return false;
    }

    Ember::NodeInfo   node_info;
    Ember::StatusCode next      = Forge::vfs_directory_stream_next(dir_stream_ID, &node_info);
    std::string       node_path = node_info.node_path;
    while (next > Ember::Status::DIRECTORY_STREAM_EOD) {
        if (node_path != "." && node_path != "..") {
            std::string src_node_to_cp  = src + '/' + node_path;
            std::string dest_node_to_cp = dest_node + '/' + node_path;
            if (node_info.is_directory()) {
                if (!copy_dir_content(src_node_to_cp, dest_node_to_cp)) {
                    close_dir_stream(dir_stream_ID);
                    return false;
                }
            } else {
                if (!copy_file_content(src_node_to_cp, dest_node_to_cp)) {
                    close_dir_stream(dir_stream_ID);
                    return false;
                }
            }
        }
        next      = Forge::vfs_directory_stream_next(dir_stream_ID, &node_info);
        node_path = node_info.node_path;
    }
    if (node_path != "." && node_path != "..") {
        // Copy last node
        const std::string src_node_to_cp  = src + '/' + node_path;
        const std::string dest_node_to_cp = dest_node + '/' + node_path;
        if (node_info.is_directory()) {
            if (!copy_dir_content(src_node_to_cp, dest_node_to_cp)) {
                close_dir_stream(dir_stream_ID);
                return false;
            }
        } else {
            if (!copy_file_content(src_node_to_cp, dest_node_to_cp)) {
                close_dir_stream(dir_stream_ID);
                return false;
            }
        }
    }
    close_dir_stream(dir_stream_ID);
    return true;
}


CLINK int main(const int argc, char* argv[]) {
    CLIArgs args;
    if (!parse_cli_args(argc, argv, args))
        return -1;

    if (args.help) {
        std::cout << "cp [src] [dest] [options]" << std::endl;
        std::cout << "    Copy the src file/directory to dest." << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "    -h: Print this help menu." << std::endl;
        std::cout << "    -r: Copy the content of the src directory recursively." << std::endl;
        return 0;
    }

    Ember::NodeInfo node_info;
    if (const int ret = get_node_info(args.src_path, node_info); ret < 1) {
        if (ret == 0)
            std::cerr << "'" << args.src_path << "': Node not found." << std::endl;
        return -1;
    }

    if (node_info.is_file()) {
        if (!copy_file_content(args.src_path, args.dest_path))
            return -1;
    } else {
        if (args.recursive)
            copy_dir_content(args.src_path, args.dest_path);
        else
            std::cerr << "'" << args.src_path << "': Is a directory. Use '-r' to copy directories." << std::endl;
    }
    return 0;
}
