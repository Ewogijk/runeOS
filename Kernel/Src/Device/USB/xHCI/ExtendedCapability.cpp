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

#include <Device/USB/xHCI/ExtendedCapability.h>

#include <KRE/BitsAndBytes.h>

namespace Rune::Device::USB {

    DEFINE_ENUM(ExtendedCapabilityCode, EXTENDED_CAPABILITY_CODE, 0x0)

    // ========================================================================================== //
    // ExtendedCapabilityPointerRegister::ECPR — xHCI 2.0 §7 Table 7-1
    // ========================================================================================== //

    [[nodiscard]] auto ExtendedCapabilityPointerRegister::ECPR::capability_id() const volatile
        -> U8 {
        return static_cast<U8>(m_register & CAPABILITY_ID_MASK);
    }

    [[nodiscard]] auto ExtendedCapabilityPointerRegister::ECPR::next_capability() const volatile
        -> U8 {
        return static_cast<U8>((m_register & NEXT_CAPABILITY_MASK) >> SHIFT_8);
    }

    [[nodiscard]] auto ExtendedCapabilityPointerRegister::ECPR::capability_specific() const volatile
        -> U16 {
        return static_cast<U16>((m_register & CAPABILITY_SPECIFIC_MASK) >> SHIFT_16);
    }

    auto ExtendedCapabilityPointerRegister::ECPR::set_capability_id(U8 val) volatile -> void {
        m_register = (m_register & ~CAPABILITY_ID_MASK) | static_cast<U32>(val);
    }

    auto ExtendedCapabilityPointerRegister::ECPR::set_next_capability(U8 val) volatile -> void {
        m_register = (m_register & ~NEXT_CAPABILITY_MASK)
                     | ((static_cast<U32>(val) << SHIFT_8) & NEXT_CAPABILITY_MASK);
    }

    auto ExtendedCapabilityPointerRegister::ECPR::set_capability_specific(U16 val) volatile
        -> void {
        m_register = (m_register & ~CAPABILITY_SPECIFIC_MASK)
                     | ((static_cast<U32>(val) << SHIFT_16) & CAPABILITY_SPECIFIC_MASK);
    }

    // ========================================================================================== //
    // SupportedProtocolCapability::ECPR — xHCI 2.0 §7.2
    // ========================================================================================== //

    [[nodiscard]] auto SupportedProtocolCapability::ECPR::capability_id() const volatile -> U8 {
        return static_cast<U8>(m_register & CAPABILITY_ID_MASK);
    }

    [[nodiscard]] auto SupportedProtocolCapability::ECPR::next_capability_pointer() const volatile
        -> U8 {
        return static_cast<U8>((m_register & NEXT_CAPABILITY_POINTER_MASK) >> SHIFT_8);
    }

    [[nodiscard]] auto SupportedProtocolCapability::ECPR::minor_revision() const volatile -> U8 {
        return static_cast<U8>((m_register & MINOR_REVISION_MASK) >> SHIFT_16);
    }

    [[nodiscard]] auto SupportedProtocolCapability::ECPR::major_revision() const volatile -> U8 {
        return static_cast<U8>((m_register & MAJOR_REVISION_MASK) >> SHIFT_24);
    }

    auto SupportedProtocolCapability::ECPR::set_capability_id(U8 val) volatile -> void {
        m_register = (m_register & ~CAPABILITY_ID_MASK) | static_cast<U32>(val);
    }

    auto SupportedProtocolCapability::ECPR::set_next_capability_pointer(U8 val) volatile -> void {
        m_register = (m_register & ~NEXT_CAPABILITY_POINTER_MASK)
                     | ((static_cast<U32>(val) << SHIFT_8) & NEXT_CAPABILITY_POINTER_MASK);
    }

    auto SupportedProtocolCapability::ECPR::set_minor_revision(U8 val) volatile -> void {
        m_register = (m_register & ~MINOR_REVISION_MASK)
                     | ((static_cast<U32>(val) << SHIFT_16) & MINOR_REVISION_MASK);
    }

    auto SupportedProtocolCapability::ECPR::set_major_revision(U8 val) volatile -> void {
        m_register = (m_register & ~MAJOR_REVISION_MASK)
                     | ((static_cast<U32>(val) << SHIFT_24) & MAJOR_REVISION_MASK);
    }

    // ========================================================================================== //
    // SupportedProtocolCapability::PortProtocolRegister — xHCI 2.0 §7.2
    // ========================================================================================== //

    [[nodiscard]] auto SupportedProtocolCapability::PortProtocolRegister::port_offset() const
        volatile -> U8 {
        return static_cast<U8>(m_register & PORT_OFFSET_MASK);
    }

    [[nodiscard]] auto SupportedProtocolCapability::PortProtocolRegister::port_count() const
        volatile -> U8 {
        return static_cast<U8>((m_register & PORT_COUNT_MASK) >> SHIFT_8);
    }

    [[nodiscard]] auto SupportedProtocolCapability::PortProtocolRegister::protocol_defined() const
        volatile -> U16 {
        return static_cast<U16>((m_register & PROTOCOL_DEFINED_MASK) >> SHIFT_16);
    }

    [[nodiscard]] auto SupportedProtocolCapability::PortProtocolRegister::PSIC() const volatile
        -> U8 {
        return static_cast<U8>((m_register & PSIC_MASK) >> 28);
    }

    auto SupportedProtocolCapability::PortProtocolRegister::set_port_offset(U8 val) volatile
        -> void {
        m_register = (m_register & ~PORT_OFFSET_MASK) | static_cast<U32>(val);
    }

    auto SupportedProtocolCapability::PortProtocolRegister::set_port_count(U8 val) volatile
        -> void {
        m_register = (m_register & ~PORT_COUNT_MASK)
                     | ((static_cast<U32>(val) << SHIFT_8) & PORT_COUNT_MASK);
    }

    auto SupportedProtocolCapability::PortProtocolRegister::set_protocol_defined(U16 val) volatile
        -> void {
        m_register = (m_register & ~PROTOCOL_DEFINED_MASK)
                     | ((static_cast<U32>(val) << SHIFT_16) & PROTOCOL_DEFINED_MASK);
    }

    auto SupportedProtocolCapability::PortProtocolRegister::set_PSIC(U8 val) volatile -> void {
        m_register = (m_register & ~PSIC_MASK) | ((static_cast<U32>(val) << 28) & PSIC_MASK);
    }

    // ========================================================================================== //
    // SupportedProtocolCapability::ProtocolSlotType — xHCI 2.0 §7.2
    // ========================================================================================== //

    [[nodiscard]] auto SupportedProtocolCapability::ProtocolSlotType::protocol_slot_type() const
        volatile -> U8 {
        return static_cast<U8>(m_register & PROTOCOL_SLOT_TYPE_MASK);
    }

    auto SupportedProtocolCapability::ProtocolSlotType::set_protocol_slot_type(U8 val) volatile
        -> void {
        m_register = (m_register & ~PROTOCOL_SLOT_TYPE_MASK)
                     | (static_cast<U32>(val) & PROTOCOL_SLOT_TYPE_MASK);
    }

    // ========================================================================================== //
    // SupportedProtocolCapability::ProtocolSpeed — xHCI 2.0 §7.2
    // ========================================================================================== //

    [[nodiscard]] auto SupportedProtocolCapability::ProtocolSpeed::PSIV() const volatile -> U8 {
        return static_cast<U8>(m_register & PSIV_MASK);
    }

    [[nodiscard]] auto SupportedProtocolCapability::ProtocolSpeed::PSIE() const volatile -> U8 {
        return static_cast<U8>((m_register & PSIE_MASK) >> 4);
    }

    [[nodiscard]] auto SupportedProtocolCapability::ProtocolSpeed::PLT() const volatile -> U8 {
        return static_cast<U8>((m_register & PLT_MASK) >> 6);
    }

    [[nodiscard]] auto SupportedProtocolCapability::ProtocolSpeed::PFD() const volatile -> bool {
        return bit_check(m_register, PFD_BIT_OFFSET);
    }

    [[nodiscard]] auto SupportedProtocolCapability::ProtocolSpeed::LP() const volatile -> U8 {
        return static_cast<U8>((m_register & LP_MASK) >> 14);
    }

    [[nodiscard]] auto SupportedProtocolCapability::ProtocolSpeed::PSIM() const volatile -> U16 {
        return static_cast<U16>((m_register & PSIM_MASK) >> SHIFT_16);
    }

    auto SupportedProtocolCapability::ProtocolSpeed::set_PSIV(U8 val) volatile -> void {
        m_register = (m_register & ~PSIV_MASK) | (static_cast<U32>(val) & PSIV_MASK);
    }

    auto SupportedProtocolCapability::ProtocolSpeed::set_PSIE(U8 val) volatile -> void {
        m_register = (m_register & ~PSIE_MASK) | ((static_cast<U32>(val) << 4) & PSIE_MASK);
    }

    auto SupportedProtocolCapability::ProtocolSpeed::set_PLT(U8 val) volatile -> void {
        m_register = (m_register & ~PLT_MASK) | ((static_cast<U32>(val) << 6) & PLT_MASK);
    }

    auto SupportedProtocolCapability::ProtocolSpeed::set_PFD(bool v) volatile -> void {
        m_register =
            v ? bit_set(m_register, PFD_BIT_OFFSET) : bit_clear(m_register, PFD_BIT_OFFSET);
    }

    auto SupportedProtocolCapability::ProtocolSpeed::set_LP(U8 val) volatile -> void {
        m_register = (m_register & ~LP_MASK) | ((static_cast<U32>(val) << 14) & LP_MASK);
    }

    auto SupportedProtocolCapability::ProtocolSpeed::set_PSIM(U16 val) volatile -> void {
        m_register = (m_register & ~PSIM_MASK) | ((static_cast<U32>(val) << SHIFT_16) & PSIM_MASK);
    }

} // namespace Rune::Device::USB
