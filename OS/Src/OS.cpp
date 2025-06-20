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

#include <OS.h>

#include <StdIO.h>
#include <Build.h>

#include <Shell/Interpreter.h>


namespace Rune {
    int main(int argc, char* argv[]) {
        SILENCE_UNUSED(argc)
        SILENCE_UNUSED(argv)

        char wd[128];
        memset(wd, '\0', 128);
        S64  ret = Pickaxe::app_get_working_directory(wd, 128);
        if (ret != 0)
            // Failed to get the working directory
            Pickaxe::app_exit(-1);

        Shell::Interpreter interpreter;

        if (!interpreter.setup_environment(wd))
            return -1;

        String version = String::format("v{}.{}.{}", OS_MAJOR, OS_MINOR, OS_PATCH);
        if (!String(OS_PRERELEASE).is_empty())
            version += String::format("-{}", OS_PRERELEASE);
        print_out("Welcome to runeOS {}!\n\n", version);

        print_out("Use the 'help' command to get more information about the shell.\n\n", version);

        interpreter.run();
        return 0;
    }
}