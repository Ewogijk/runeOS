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

#include <KRE/CPU.h>

#include <KRE/BitsAndBytes.h>

namespace Rune {

    // ========================================================================================== //
    // ThreadLaunchPacketBuilder
    // ========================================================================================== //

    ThreadLaunchPacketBuilder::ThreadLaunchPacketBuilder() {
        memset(m_args.data(), 0, m_args.size());
        memset(static_cast<void*>(m_args_offsets.data()), 0, m_args_offsets.size() * sizeof(U16));
    }

    auto ThreadLaunchPacketBuilder::add_argument(const String& arg) -> ThreadLaunchPacketBuilder& {
        if (m_args_offset + arg.size() + 1 > Ember::ThreadLaunchPacket::ARGS_LIMIT) {
            return *this;
        }
        if (m_argc >= Ember::ThreadLaunchPacket::ARGV_LIMIT - 1) {
            return *this;
        }
        memcpy(&m_args[m_args_offset], arg.to_cstr(), arg.size());
        m_args[m_args_offset + arg.size()]  = '\0';
        m_args_offsets[m_argc]              = m_args_offset;
        m_args_offset                      += arg.size() + 1;
        m_argc++;
        return *this;
    }

    auto ThreadLaunchPacketBuilder::main(Ember::ThreadMain main) -> ThreadLaunchPacketBuilder& {
        m_main = main;
        return *this;
    }

    auto ThreadLaunchPacketBuilder::random(U64 random_low, U64 random_high)
        -> ThreadLaunchPacketBuilder& {
        m_random_low  = random_low;
        m_random_high = random_high;
        return *this;
    }

    auto ThreadLaunchPacketBuilder::program_headers(void*  program_headers,
                                                    size_t size,
                                                    size_t count) -> ThreadLaunchPacketBuilder& {
        m_program_header_address = program_headers;
        m_program_header_size    = size;
        m_program_header_count   = count;
        return *this;
    }

    auto ThreadLaunchPacketBuilder::build() -> UniquePointer<Ember::ThreadLaunchPacket> {
        UniquePointer<Ember::ThreadLaunchPacket> tlp(new Ember::ThreadLaunchPacket);
        memcpy(tlp->m_args, m_args.data(), Ember::ThreadLaunchPacket::ARGS_LIMIT);
        memcpy(tlp->m_args_offsets,
               m_args_offsets.data(),
               sizeof(U16) * Ember::ThreadLaunchPacket::ARGV_LIMIT);
        tlp->m_argc                   = m_argc;
        tlp->m_main                   = m_main;
        tlp->m_random_low             = m_random_low;
        tlp->m_random_high            = m_random_high;
        tlp->m_program_header_address = m_program_header_address;
        tlp->m_program_header_size    = m_program_header_size;
        tlp->m_program_header_count   = m_program_header_count;
        return tlp;
    }

} // namespace Rune
