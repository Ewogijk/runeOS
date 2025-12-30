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

#include <Crucible.h>

#include <Forge/App.h>

#include <Crucible/BuiltInCommand.h>
#include <Crucible/Interpreter.h>

#include <Build.h>

#include <iostream>

namespace Crucible {
    auto main(const int argc, char* argv[]) -> int {
        SILENCE_UNUSED(argc)
        SILENCE_UNUSED(argv)

        constexpr char wd[128] = {};
        if (const Ember::StatusCode ret = Forge::app_current_directory(wd, 128); ret != 0)
            // Failed to get the working directory
            Forge::app_exit(-1);

        Interpreter interpreter;
        if (!interpreter.setup_environment(wd)) return -1;

        std::cout << "\033[3J\033[1;1H";
        std::cout.flush();
        std::cout << "Crucible v" << MAJOR << "." << MINOR << "." << PATCH;
        if (!std::string(PRERELEASE).empty())
            std::cout << "-" << PRERELEASE << std::endl;
        else
            std::cout << std::endl;
        std::cout << std::endl;

        std::cout << "Use the 'help' command to get more information about the shell." << std::endl
                  << std::endl;

        interpreter.run();
        return 0;
    }
} // namespace Crucible
