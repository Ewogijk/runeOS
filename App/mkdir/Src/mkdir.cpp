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
    std::string node_path;
    bool        help = false;
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
    if (!node_path_seen && !args_out.help) std::cerr << "Missing node argument." << std::endl;
    return node_path_seen || args_out.help;
}

int main(const int argc, char* argv[]) {
    CLIArgs args;
    if (!parse_cli_args(argc, argv, args)) return -1;

    if (args.help) {
        std::cout << "mkdir [node] [options]" << std::endl;
        std::cout << "    Create a directory." << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "    -h: Print this help menu." << std::endl;
        return 0;
    }

    if (const Ember::StatusCode ret =
            Forge::vfs_create(args.node_path.c_str(), Ember::NodeAttribute::DIRECTORY);
        ret < Ember::Status::OKAY) {
        switch (ret) {
            case Ember::Status::BAD_ARG:
                std::cerr << "'" << args.node_path << "' - Bad path." << std::endl;
                break;
            case Ember::Status::NODE_EXISTS:
                std::cerr << "'" << args.node_path << "' - Node exists." << std::endl;
                break;
            default: std::cerr << "'" << args.node_path << "' - IO error." << std::endl; break;
        }
    }
    return 0;
}
