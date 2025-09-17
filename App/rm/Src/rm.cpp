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

#include <iostream>
#include <string>

struct CLIArgs {
    std::string node_path = "";

    bool help      = false;
    bool recursive = false;
};

bool parse_cli_args(const int argc, char* argv[], CLIArgs& args_out) {
    bool node_path_seen = false;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg.size() == 0) continue;

        if (arg[0] == '-') {
            for (size_t j = 1; j < arg.size(); j++) {
                switch (arg[j]) {
                    case 'h': args_out.help = true; break;
                    case 'r': args_out.recursive = true; break;
                    default:  {
                        std::cerr << "Unknown option '" << arg << "'" << std::endl;
                        return false;
                    }
                }
            }
        } else {
            if (node_path_seen) {
                std::cerr << "Unknown argument '" << arg << "'" << std::endl;
                return false;
            }
            args_out.node_path = arg;
            node_path_seen     = true;
        }
    }
    if (!node_path_seen && !args_out.help) std::cerr << "Missing node argument" << std::endl;
    return node_path_seen || args_out.help;
}

bool delete_node(const std::string& node_path) {
    if (const Ember::StatusCode ret = Forge::vfs_delete(node_path.c_str());
        ret < Ember::Status::OKAY) {
        if (ret == Ember::Status::NODE_IN_USE)
            std::cerr << "'" << node_path << "': Cannot delete, node is used by another app."
                      << std::endl;
        else
            std::cerr << "'" << node_path << "': IO error." << std::endl;
        return false;
    }
    return true;
}

void close_dir_stream(const S64 dir_stream_ID) {
    if (dir_stream_ID <= 0) return; // Invalid stream ID
    Forge::vfs_directory_stream_close(dir_stream_ID);
    // Both possible return values are fine, no need for error handling
    //   Ember::Status::OKAY -> Stream was closed
    //   Ember::Status::UNKNOWN_ID -> No stream with the ID was found
}

bool delete_dir(const std::string& directory_path) {
    const S64 dir_stream_ID = Forge::vfs_directory_stream_open(directory_path.c_str());
    if (dir_stream_ID < Ember::Status::OKAY) {
        std::cerr << "'" << directory_path << "': IO error." << std::endl;
        close_dir_stream(dir_stream_ID);
        return false;
    }

    Ember::NodeInfo node_info;
    S64             next      = Forge::vfs_directory_stream_next(dir_stream_ID, &node_info);
    std::string     node_path = node_info.node_path;
    while (next > Ember::Status::DIRECTORY_STREAM_EOD) {
        if (node_path != "." && node_path != "..") {
            if (node_info.is_directory()) {
                if (!delete_dir(directory_path + "/" + node_path)) {
                    Forge::vfs_directory_stream_close(dir_stream_ID);
                    return false;
                }
            } else {
                if (!delete_node(directory_path + "/" + node_path)) {
                    Forge::vfs_directory_stream_close(dir_stream_ID);
                    return false;
                }
            }
        }
        next      = Forge::vfs_directory_stream_next(dir_stream_ID, &node_info);
        node_path = node_info.node_path;
    }
    if (node_path != "." && node_path != "..") {
        // Delete last node
        if (node_info.is_directory()) {
            if (!delete_dir(directory_path + "/" + node_path)) {
                Forge::vfs_directory_stream_close(dir_stream_ID);
                return false;
            }
        } else {
            if (!delete_node(directory_path + "/" + node_path)) {
                Forge::vfs_directory_stream_close(dir_stream_ID);
                return false;
            }
        }
    }

    // Delete this directory
    if (!delete_node(directory_path)) {
        Forge::vfs_directory_stream_close(dir_stream_ID);
        return false;
    }
    Forge::vfs_directory_stream_close(dir_stream_ID);
    return true;
}

CLINK int main(const int argc, char* argv[]) {
    CLIArgs args;
    if (!parse_cli_args(argc, argv, args)) return -1;

    if (args.help) {
        std::cout << "rm [file|directory] [options]" << std::endl;
        std::cout << "    Delete a file or directory." << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "    -r: Remove the directory and all its content recursively." << std::endl;
        std::cout << "    -h: Print this help menu." << std::endl;
        return 0;
    }

    Ember::NodeInfo node_info;
    if (const Ember::StatusCode ret = Forge::vfs_get_node_info(args.node_path.c_str(), &node_info);
        ret < Ember::Status::OKAY) {
        switch (ret) {
            case Ember::Status::NODE_NOT_FOUND:
                std::cerr << "'" << args.node_path << "': Node not found." << std::endl;
                break;
            case Ember::Status::BAD_ARG: // Intermediate path element is a file
                std::cerr << "'" << args.node_path << "': Bad path." << std::endl;
                break;
            default: std::cerr << "'" << args.node_path << "': IO error." << std::endl;
        }
        return -1;
    }

    if (node_info.is_file()) {
        // Node is a file -> Can just delete it
        if (!delete_node(args.node_path)) return -1;
    } else {
        const S64 dir_stream_ID = Forge::vfs_directory_stream_open(args.node_path.c_str());
        if (dir_stream_ID < 0) {
            // We know node path arg is fine, the node is a dir and exists -> Can only be IO error
            std::cerr << "'" << args.node_path << "': IO error." << std::endl;
            close_dir_stream(dir_stream_ID);
            return -1;
        }
        size_t            nodes_in_dir = 0;
        Ember::StatusCode next      = Forge::vfs_directory_stream_next(dir_stream_ID, &node_info);
        std::string       node_path = node_info.node_path;
        while (next > Ember::Status::DIRECTORY_STREAM_EOD) {
            if (node_path != "." && node_path != "..") nodes_in_dir++;
            next      = Forge::vfs_directory_stream_next(dir_stream_ID, &node_info);
            node_path = node_info.node_path;
        }
        if (node_path != "." && node_path != ".."
            && (node_info.is_directory() || node_info.is_file()))
            nodes_in_dir++;

        if (nodes_in_dir == 0) {
            if (!delete_node(args.node_path)) {
                close_dir_stream(dir_stream_ID);
                return -1;
            }
        } else {
            if (!args.recursive) {
                std::cerr << "'" << args.node_path
                          << "': Cannot delete, directory is not empty. Use '-r' to delete the "
                             "directory and its content."
                          << std::endl;
                close_dir_stream(dir_stream_ID);
                return -1;
            }

            if (!delete_dir(args.node_path)) {
                close_dir_stream(dir_stream_ID);
                return -1;
            }
        }
        close_dir_stream(dir_stream_ID);
    }

    return 0;
}
