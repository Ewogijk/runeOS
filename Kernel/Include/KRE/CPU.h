
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

#ifndef RUNEOS_CPU_H
#define RUNEOS_CPU_H

#include <Ember/Ember.h>

#include <KRE/Build.h>

namespace Rune {
    // Size of a CPU register
#ifdef BIT64
    using Register = U64;
#else
    using Register = U32;
#endif

    /// @brief The current value of the stack pointer.
    CLINK auto cpu_get_stack_pointer() -> Register;

    /// @brief Halt the CPU until an interrupt occurs.
    CLINK void cpu_halt();

    /// @brief Pause the CPU in an optimized way in terms of performance/power usage. This function
    ///         is intended to be used when waiting in a loop, e.g. in a spinlock.
    CLINK void cpu_pause();

    /// @brief Get the virtual address that was responsible for a page fault.
    ///
    /// Important: The returned virtual address is only valid during handling of a page fault
    /// otherwise the virtual address is undefined.
    CLINK auto cpu_get_page_fault_address() -> Register;

    struct ThreadStartupPacket;

    /// @brief Main function of a thread. It has the signature int(StartInfo*). The start
    /// info contains argc/argv parameters as well as other information. The return value is the
    /// thread status after it finished. status >= 0 -> everything fine, status < 0 -> exit with
    /// error.
    using ThreadMain = int (*)(ThreadStartupPacket*);

    /// @brief The thread arguments, dynamic linker information and other useful information.
    ///
    /// A thread is either an application main thread or a minor thread. The type of thread
    /// determines how much information shall be passed in the start info.
    ///
    /// The information passed in the StartInfo is defined as followed:
    ///  - Application Main Thread: All StartInfo information shall be provided.
    ///  - Minor Thread: Argc, argv and main shall be provided, the state of the other fields is
    ///     undefined.
    struct ThreadStartupPacket {
        /// @brief Number of arguments.
        int argc{0};

        /// @brief A null terminated array of string arguments.
        char** argv{nullptr};

        /// @brief Low and high bytes of a random 16 byte value.
        U64 random_low{0};
        U64 random_high{0};

        /// @brief Virtual address of an array where the ELF program headers are stored.
        void* program_header_address{nullptr};

        /// @brief Size of a program header.
        size_t program_header_size{0};

        /// @brief Size of the program header array.
        size_t program_header_count{0};

        /// @brief Main function of the thread.
        ThreadMain main{};

        /// @brief Address of a 16-byte random value.
        void* random{nullptr};
    };

} // namespace Rune

#endif // RUNEOS_CPU_H
