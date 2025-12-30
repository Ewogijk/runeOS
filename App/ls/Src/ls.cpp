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

#include <Forge/App.h>
#include <Forge/VFS.h>

#include <array>
#include <format>
#include <iostream>
#include <string>

struct CLIArgs {
    std::string dir;

    bool help = false;
    bool all  = false;
    bool list = false;
};

auto parse_cli_args(int argc, char* argv[], CLIArgs& args_out) -> bool { // NOLINT
    bool dir_seen = false;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg.empty()) continue;

        if (arg[0] == '-') {
            for (size_t j = 1; j < arg.size(); j++) {
                switch (arg[j]) {
                    case 'a': args_out.all = true; break;
                    case 'l': args_out.list = true; break;
                    case 'h': args_out.help = true; break;
                    default:  {
                        std::cerr << "Unknown option '" << arg << "'" << std::endl;
                        return false;
                    }
                }
            }
        } else {
            if (dir_seen) {
                std::cerr << "Unknown argument '" << arg << "'" << std::endl;
                return false;
            }
            args_out.dir = arg;
            dir_seen     = true;
        }
    }

    if (!dir_seen) {
        std::array<char, Ember::STRING_SIZE_LIMIT> c_path{};
        if (const Ember::StatusCode ret =
                Forge::app_current_directory(c_path.data(), Ember::STRING_SIZE_LIMIT);
            ret < 0) {
            std::cerr << "IO error: Cannot get current directory." << std::endl;
            return false;
        }
        args_out.dir = c_path.data();
    }
    return true;
}

void print_node_info(const CLIArgs& args, const Ember::NodeInfo& node_info) {
    if (const std::string node_path = node_info.node_path;
        args.all || (!node_info.is_hidden() && node_path != "." && node_path != "..")) {
        if (args.list) {
            std::array<char, 4> attr = {'-', '-', '-', '-'};
            if (node_info.is_file())
                attr[0] = 'F';
            else
                attr[0] = 'D';
            if (node_info.is_hidden()) attr[1] = 'H';
            if (node_info.is_system_node()) {
                attr[2] = 'S';
            }
            if (node_info.is_readonly()) {
                attr[3] = 'R';
            }
            std::cout << std::format("{:<10} {:<15} {}",
                                     attr.data(),
                                     node_info.size,
                                     node_info.node_path)
                      << std::endl;
        } else {
            std::cout << node_info.node_path << std::endl;
        }
    }
}

CLINK auto main(const int argc, char* argv[]) -> int {
    CLIArgs args;
    if (!parse_cli_args(argc, argv, args)) return -1;

    if (args.help) {
        std::cout << "ls [directory] [options]" << std::endl;
        std::cout << "    List the content of a directory" << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "    -a: Include hidden files." << std::endl;
        std::cout << "    -h: Print this help menu." << std::endl;
        std::cout << "    -l: Print detailed information about each node." << std::endl;
        return 0;
    }

    Ember::StatusCode dir_stream_ID = Forge::vfs_directory_stream_open(args.dir.c_str());
    if (dir_stream_ID < 0) {
        switch (dir_stream_ID) {
            case Ember::Status::BAD_ARG: std::cerr << "'" << args.dir << "': Bad path."; break;
            case Ember::Status::NODE_NOT_FOUND:
                std::cerr << "'" << args.dir << "': Directory not found.";
                break;
            case Ember::Status::NODE_IS_FILE:
                std::cerr << "'" << args.dir << "': Not a directory.";
                break;
            default: std::cerr << "'" << args.dir << "': IO error.";
        }
        return -1;
    }

    if (args.list) std::cout << "Attributes Size            Name" << std::endl;

    Ember::NodeInfo   node_info;
    Ember::StatusCode next = Forge::vfs_directory_stream_next(dir_stream_ID, &node_info);
    while (next > Ember::Status::DIRECTORY_STREAM_EOD) {
        print_node_info(args, node_info);
        next = Forge::vfs_directory_stream_next(dir_stream_ID, &node_info);
    }
    return 0;
}
