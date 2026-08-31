
//  Copyright 2026 Ewogijk
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#ifndef RUNEOS_THREADINGBITS_H
#define RUNEOS_THREADINGBITS_H

#include <Ember/Ember.h>

namespace Ember {
    struct ThreadLaunchPacket;

    /// @brief Main function of a thread. It has the signature int(StartInfo*). The start
    /// info contains argc/argv parameters as well as other information. The return value is the
    /// thread status after it finished. status >= 0 -> everything fine, status < 0 -> exit with
    /// error.
    using ThreadMain = int (*)(ThreadLaunchPacket*);

    /// @brief The thread arguments, dynamic linker information, and other useful information.
    struct ThreadLaunchPacket {
        /// @brief Maximum number of argument characters.
        static constexpr U16 ARGS_LIMIT = 4096; // 4KiB
        /// @brief Maximum number of arguments in argv.
        static constexpr U16 ARGV_LIMIT = 32;

        /// @brief A buffer for the string arguments.
        char m_args[ARGS_LIMIT] = {}; // NOLINT modernize-avoid-c-arrays: Is ABI

        /// @brief Array offsets to the beginnings of the arguments.
        U16 m_args_offsets[ARGV_LIMIT] = {}; // NOLINT modernize-avoid-c-arrays: Is ABI

        /// @brief Number of arguments.
        int m_argc{0};

        /// @brief Main function of the thread.
        ThreadMain m_main{};

        /// @brief Low and high bytes of a random 16-byte value.
        U64 m_random_low{0};
        U64 m_random_high{0};

        /// @brief Virtual address of an array where the ELF program headers are stored.
        void* m_program_header_address{nullptr};

        /// @brief Size of a program header.
        size_t m_program_header_size{0};

        /// @brief Size of the program header array.
        size_t m_program_header_count{0};

        auto argv(size_t arg_idx) -> char*;

        auto random() -> void*;
    };
} // namespace Ember

#endif // RUNEOS_THREADINGBITS_H
