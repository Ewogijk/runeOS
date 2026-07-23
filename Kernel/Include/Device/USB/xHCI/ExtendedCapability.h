
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

#ifndef RUNEOS_EXTENDEDCAPABILITY_H
#define RUNEOS_EXTENDEDCAPABILITY_H

#include <Ember/Ember.h>

namespace Rune::Device::USB {

    // ========================================================================================== //
    // Extended Capability Pointer Register — xHCI 2.0 §7 Table 7-1
    // ========================================================================================== //

    struct ExtendedCapabilityPointerRegister {
        struct ECPR {
            U32 m_register = 0;

            [[nodiscard]] auto capability_id()       const volatile -> U8;
            [[nodiscard]] auto next_capability()     const volatile -> U8;
            [[nodiscard]] auto capability_specific() const volatile -> U16;

            auto set_capability_id(U8 val)         volatile -> void;
            auto set_next_capability(U8 val)       volatile -> void;
            auto set_capability_specific(U16 val)  volatile -> void;

          private:
            static constexpr U32 CAPABILITY_ID_MASK       = 0x000000FF; // [7:0]
            static constexpr U32 NEXT_CAPABILITY_MASK     = 0x0000FF00; // [15:8]
            static constexpr U32 CAPABILITY_SPECIFIC_MASK = 0xFFFF0000; // [31:16]
        } m_extended_capability_pointer_register;
    };

#define EXTENDED_CAPABILITY_CODE(X)                                                                \
    X(ExtendedCapabilityCode, USB_LEGACY_SUPPORT, 0x1)                                             \
    X(ExtendedCapabilityCode, SUPPORTED_PROTOCOL, 0x2)                                             \
    X(ExtendedCapabilityCode, EXTENDED_POWER_MANAGEMENT, 0x3)                                      \
    X(ExtendedCapabilityCode, IO_VIRTUALIZATION, 0x4)                                              \
    X(ExtendedCapabilityCode, MESSAGE_INTERRUPT, 0x5)                                              \
    X(ExtendedCapabilityCode, LOCAL_MEMORY, 0x6)                                                   \
    X(ExtendedCapabilityCode, USB_DEBUG_CAPABILITY, 0xA)                                           \
    X(ExtendedCapabilityCode, EXTENDED_MESSAGE_INTERRUPT, 0xF2)                                    \
    X(ExtendedCapabilityCode, USB3_TUNNELING_SUPPORT, 0xF3)

    /// @brief xHCI Extended Capability Codes — §7 Table 7-2
    DECLARE_ENUM(ExtendedCapabilityCode, EXTENDED_CAPABILITY_CODE, 0x0) // NOLINT

    // ========================================================================================== //
    // Supported Protocol Capability — xHCI 2.0 §7.2
    // ========================================================================================== //

    struct SupportedProtocolCapability {
        struct ECPR {
            U32 m_register = 0;

            [[nodiscard]] auto capability_id()           const volatile -> U8;
            [[nodiscard]] auto next_capability_pointer() const volatile -> U8;
            [[nodiscard]] auto minor_revision()          const volatile -> U8;
            [[nodiscard]] auto major_revision()          const volatile -> U8;

            auto set_capability_id(U8 val)           volatile -> void;
            auto set_next_capability_pointer(U8 val) volatile -> void;
            auto set_minor_revision(U8 val)          volatile -> void;
            auto set_major_revision(U8 val)          volatile -> void;

          private:
            static constexpr U32 CAPABILITY_ID_MASK           = 0x000000FF; // [7:0]
            static constexpr U32 NEXT_CAPABILITY_POINTER_MASK = 0x0000FF00; // [15:8]
            static constexpr U32 MINOR_REVISION_MASK          = 0x00FF0000; // [23:16]
            static constexpr U32 MAJOR_REVISION_MASK          = 0xFF000000; // [31:24]
        } m_extended_capability_pointer_register;

        U32 m_name_string = 0;

        struct PortProtocolRegister {
            U32 m_register = 0;

            [[nodiscard]] auto port_offset()      const volatile -> U8;
            [[nodiscard]] auto port_count()       const volatile -> U8;
            [[nodiscard]] auto protocol_defined() const volatile -> U16; // 12-bit field [27:16]
            [[nodiscard]] auto PSIC()             const volatile -> U8;  // Protocol Speed ID Count [31:28]

            auto set_port_offset(U8 val)        volatile -> void;
            auto set_port_count(U8 val)         volatile -> void;
            auto set_protocol_defined(U16 val)  volatile -> void;
            auto set_PSIC(U8 val)               volatile -> void;

          private:
            static constexpr U32 PORT_OFFSET_MASK      = 0x000000FF; // [7:0]
            static constexpr U32 PORT_COUNT_MASK       = 0x0000FF00; // [15:8]
            static constexpr U32 PROTOCOL_DEFINED_MASK = 0x0FFF0000; // [27:16]
            static constexpr U32 PSIC_MASK             = 0xF0000000; // [31:28]
            static constexpr U8  PSIC_SHIFT            = 28;
        } m_port_protocol_register;

        struct ProtocolSlotType {
            U32 m_register = 0;

            [[nodiscard]] auto protocol_slot_type() const volatile -> U8; // [4:0]
            auto set_protocol_slot_type(U8 val) volatile -> void;

          private:
            static constexpr U32 PROTOCOL_SLOT_TYPE_MASK = 0x0000001F; // [4:0]
        } m_protocol_slot_type_register;

        struct ProtocolSpeed {
            U32 m_register = 0;

            [[nodiscard]] auto PSIV() const volatile -> U8;   // Protocol Speed ID Value [3:0]
            [[nodiscard]] auto PSIE() const volatile -> U8;   // Protocol Speed ID Exponent [5:4]
            [[nodiscard]] auto PLT()  const volatile -> U8;   // PSI Type [7:6]
            [[nodiscard]] auto PFD()  const volatile -> bool; // PSI Full-duplex [8]
            [[nodiscard]] auto LP()   const volatile -> U8;   // Link Protocol [15:14]
            [[nodiscard]] auto PSIM() const volatile -> U16;  // Protocol Speed ID Mantissa [31:16]

            auto set_PSIV(U8 val)  volatile -> void;
            auto set_PSIE(U8 val)  volatile -> void;
            auto set_PLT(U8 val)   volatile -> void;
            auto set_PFD(bool v)   volatile -> void;
            auto set_LP(U8 val)    volatile -> void;
            auto set_PSIM(U16 val) volatile -> void;

          private:
            static constexpr U32 PSIV_MASK         = 0x0000000F; // [3:0]
            static constexpr U32 PSIE_MASK         = 0x00000030; // [5:4]
            static constexpr U8  PSIE_SHIFT        = 4;
            static constexpr U32 PLT_MASK          = 0x000000C0; // [7:6]
            static constexpr U8  PLT_SHIFT         = 6;
            static constexpr U8  PFD_BIT_OFFSET    = 8;
            static constexpr U32 LP_MASK           = 0x0000C000; // [15:14]
            static constexpr U8  LP_SHIFT          = 14;
            static constexpr U32 PSIM_MASK         = 0xFFFF0000; // [31:16]
        } m_protocol_speed_register;
    };

} // namespace Rune::Device::USB

#endif // RUNEOS_EXTENDEDCAPABILITY_H