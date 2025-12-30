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

#include <Ember/Ember.h>

#include <Forge/VFS.h>

#include <iostream>
#include <string>

constexpr U16 BUF_SIZE = 4096;

struct CLIArgs {
    std::string file;

    bool help = false;
};

auto parse_cli_args(const int argc, char* argv[], CLIArgs& args_out) -> bool { // NOLINT
    bool file_seen = false;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg.empty()) continue;

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
            if (file_seen) {
                std::cerr << "Unknown argument '" << arg << "'" << std::endl;
                return false;
            }
            args_out.file = arg;
            file_seen     = true;
        }
    }
    if (!file_seen && !args_out.help) std::cerr << "Missing file argument." << std::endl;
    return file_seen || args_out.help;
}

CLINK auto main(const int argc, char* argv[]) -> int {
    CLIArgs args;
    if (!parse_cli_args(argc, argv, args)) return -1;

    if (args.help) {
        std::cout << "cat [file] [options]" << std::endl;
        std::cout << "    Print file content to standard output." << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "    -h: Print this help menu." << std::endl;
        return 0;
    }
    const Ember::StatusCode file_ID = Forge::vfs_open(args.file.c_str(), Ember::IOMode::READ);
    if (file_ID < Ember::Status::OKAY) {
        switch (file_ID) {
            case Ember::Status::BAD_ARG:
                std::cerr << "'" << args.file << "' - Bad path." << std::endl;
                break;
            case Ember::Status::NODE_NOT_FOUND:
                std::cerr << "'" << args.file << "' - File not found." << std::endl;
                break;
            default: std::cerr << "'" << args.file << "' - IO error." << std::endl; break;
        }
        return -1;
    }

    U8                buf[BUF_SIZE];
    Ember::StatusCode bytes_read =
        Forge::vfs_read(file_ID, buf, BUF_SIZE - 1); // leave space for null terminator
    while (bytes_read > Ember::Status::OKAY) {
        buf[BUF_SIZE - 1] = 0;
        std::cout << reinterpret_cast<const char*>(buf);
        bytes_read = Forge::vfs_read(file_ID, buf, BUF_SIZE - 1);
    }
    if (bytes_read < Ember::Status::OKAY) {
        switch (bytes_read) {
            case Ember::Status::BAD_ARG:
                std::cerr << "'" << args.file << "' - Bad path." << std::endl;
                break;
            case Ember::Status::NODE_IS_DIRECTORY:
                std::cerr << "'" << args.file << "' - Not a file." << std::endl;
                break;
            case Ember::Status::NODE_NOT_FOUND:
                std::cerr << "'" << args.file << "' - File not found." << std::endl;
                break;
            default: std::cerr << "'" << args.file << "' - IO error." << std::endl; break;
        }
        Forge::vfs_close(file_ID);
        return -1;
    }
    Forge::vfs_close(file_ID);
    std::cout.flush();
    return 0;
}