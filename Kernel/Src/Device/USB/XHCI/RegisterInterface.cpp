//  Copyright 2025 Ewogijk
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

#include <Device/USB/XHCI/RegisterInterface.h>

namespace Rune::Device::USB {

    DEFINE_TYPED_ENUM(PortLinkState, U8, PORT_LINK_STATES, 0xFF) // NOLINT

    auto RegisterInterface::from_base(void* base) -> RegisterInterface {
        auto* b   = reinterpret_cast<U8*>(base);
        auto* cap = reinterpret_cast<volatile CapabilityRegisters*>(b);
        return {
            .m_capability  = cap,
            .m_operational = reinterpret_cast<volatile OperationalRegisters*>(b + cap->m_caplength),
            .m_runtime = reinterpret_cast<volatile RuntimeRegisters*>(b + (cap->m_rtsoff & ~0x1Fu)),
            .m_doorbell = reinterpret_cast<volatile DoorbellRegister*>(b + (cap->m_dboff & ~0x3u)),
        };
    }

} // namespace Rune::Device::USB