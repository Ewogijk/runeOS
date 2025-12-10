/*Heimdall
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

#include <Test/Heimdall/Heimdall.h>

#include <Test/IntegrationTest/tests/Dummy.h>

#include <iostream>
#include <string>

struct CLIArgs {
    bool use_junit_reporter = false;
};

bool parse_cli_args(const int argc, char* argv[], CLIArgs& args_out) {
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg.size() == 0) continue;

        if (arg[0] == '-') {
            for (size_t j = 1; j < arg.size(); j++) {
                switch (arg[j]) {
                    case 'j': args_out.use_junit_reporter = true; break;
                    default:  {
                        std::cerr << "Unknown option '" << arg << "'" << std::endl;
                        return false;
                    }
                }
            }
        } else {
            std::cerr << "Not a flag: '" << arg << "'" << std::endl;
            return false;
        }
    }
    return true;
}

int main(const int argc, char* argv[]) {
    CLIArgs args;
    if (!parse_cli_args(argc, argv, args)) return -1;

    Heimdall::OptionList options;
    if (args.use_junit_reporter) {
        options.insert({.name = Heimdall::Engine::JUNIT_REPORTER, .value = ""});
        options.insert({.name = Heimdall::Engine::GNOME_REPORTER, .value = ""});
    } else {
        options.insert({.name = Heimdall::Engine::CONSOLE_REPORTER, .value = ""});
    }
    options.insert({.name  = Heimdall::Engine::TEST_REPORT_DIRECTORY,
                    .value = "/System/Heimdall/IntegrationTest/"});
    Heimdall::execute_tests(options);
    return 0;
}
