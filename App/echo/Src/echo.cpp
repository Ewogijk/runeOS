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
void print_out(const char* fmt, Args... args) {
    Rune::Argument arg_array[] = { args... };
    char            b[128];
    memset(b, 0, 128);
    int s = Rune::interpolate(fmt, b, 128, arg_array, sizeof...(Args));
    Rune::Pickaxe::write_std_out((const char*) b, s);
}


CLINK int main(int argc, char* argv[]) {
    for (int i = 0; i < argc; i++)
        print_out("{}{}", argv[i], i < argc - 1 ? " " : "");
    print_out("\n");
    return 0;
}