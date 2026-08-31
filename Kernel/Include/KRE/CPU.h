
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
#include <Ember/ThreadingBits.h>

#include <KRE/Build.h>
#include <KRE/Collections/Array.h>
#include <KRE/Memory.h>
#include <KRE/String.h>

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

    /// @brief Builder class for thread launch packets.
    class ThreadLaunchPacketBuilder {
        Array<char, Ember::ThreadLaunchPacket::ARGS_LIMIT> m_args{};
        Array<U16, Ember::ThreadLaunchPacket::ARGV_LIMIT>  m_args_offsets{};
        size_t                                             m_args_offset = 0;
        int                                                m_argc        = 0;

        Ember::ThreadMain m_main = nullptr;

        U64 m_random_low  = 0;
        U64 m_random_high = 0;

        void*  m_program_header_address = nullptr;
        size_t m_program_header_size    = 0;
        size_t m_program_header_count   = 0;

      public:
        ThreadLaunchPacketBuilder();

        /// @brief Add the argument to argv.
        /// @param arg
        /// @return
        auto add_argument(const String& arg) -> ThreadLaunchPacketBuilder&;

        /// @brief Define the thread main function.
        /// @param main
        /// @return
        auto main(Ember::ThreadMain main) -> ThreadLaunchPacketBuilder&;

        /// @brief The seed for a pseudo random number generator.
        /// @param random
        /// @return
        auto random(U64 random_low, U64 random_high) -> ThreadLaunchPacketBuilder&;

        /// @brief Define program header information for a dynamic linker.
        /// @param program_headers
        /// @param size
        /// @param count
        /// @return
        auto program_headers(void* program_headers, size_t size, size_t count)
            -> ThreadLaunchPacketBuilder&;

        /// @brief Create the thread launch packet with the provided information.
        /// @return
        auto build() -> UniquePointer<Ember::ThreadLaunchPacket>;
    };

} // namespace Rune

#endif // RUNEOS_CPU_H
