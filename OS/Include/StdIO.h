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

#ifndef RUNEOS_STDIO_H
#define RUNEOS_STDIO_H

#include <Pickaxe/AppManagement.h>

#include <Hammer/String.h>

namespace Rune {
    void print_out(char ch);


    template<typename... Args>
    void print_out(const char* fmt, Args... args) {
        Argument arg_array[] = { args... };
        String str = String::format(fmt, arg_array, sizeof...(Args));
        Pickaxe::write_std_out(str.to_cstr(), str.size());
    }

    void print_err(char ch);

    template<typename... Args>
    void print_err(const char* fmt, Args... args) {
        Argument arg_array[] = { args... };
        String str = String::format(fmt, arg_array, sizeof...(Args));
        Pickaxe::write_std_err(str.to_cstr(), str.size());
    }
}

#endif //RUNEOS_STDIO_H
