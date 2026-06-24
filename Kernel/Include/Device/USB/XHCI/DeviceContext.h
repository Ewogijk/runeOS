
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

#ifndef RUNEOS_DEVICECONTEXT_H
#define RUNEOS_DEVICECONTEXT_H

#include <Ember/Ember.h>

#include <KRE/Collections/Array.h>
#include <KRE/Memory.h>

namespace Rune::Device::USB {
    static constexpr size_t XHCI_CONTEXT_DWORDS = 8;
    static constexpr size_t XHCI_MAX_SLOTS      = 255;

    // ========================================================================================== //
    // Device Context — §6.2
    // ========================================================================================== //

    /// @brief Slot Context — describes the overall device (xHCI 2.0 §6.2.2 Table 6-7).
    struct SlotContext {
        struct DW0 {
            U32 m_register = 0;
            [[nodiscard]] auto route_string()    const -> U32;
            [[nodiscard]] auto speed()           const -> U8;
            [[nodiscard]] auto MTT()             const -> bool;
            [[nodiscard]] auto hub()             const -> bool;
            [[nodiscard]] auto context_entries() const -> U8;
            auto set_route_string(U32 val)    -> void;
            auto set_speed(U8 val)            -> void;
            auto set_MTT(bool v)              -> void;
            auto set_hub(bool v)              -> void;
            auto set_context_entries(U8 val)  -> void;
          private:
            static constexpr U32 ROUTE_STRING_MASK    = 0x000FFFFF; // [19:0]
            static constexpr U32 SPEED_MASK           = 0x00F00000; // [23:20]
            static constexpr U8  MTT_BIT_OFFSET       = 25;
            static constexpr U8  HUB_BIT_OFFSET       = 26;
            static constexpr U32 CONTEXT_ENTRIES_MASK = 0xF8000000; // [31:27]
        } m_dw0;

        struct DW1 {
            U32 m_register = 0;
            [[nodiscard]] auto max_exit_latency()  const -> U16;
            [[nodiscard]] auto root_hub_port_num() const -> U8;
            [[nodiscard]] auto num_ports()         const -> U8;
            auto set_max_exit_latency(U16 val)  -> void;
            auto set_root_hub_port_num(U8 val)  -> void;
            auto set_num_ports(U8 val)          -> void;
          private:
            static constexpr U32 MAX_EXIT_LATENCY_MASK  = 0x0000FFFF; // [15:0]
            static constexpr U32 ROOT_HUB_PORT_NUM_MASK = 0x00FF0000; // [23:16]
            static constexpr U32 NUM_PORTS_MASK         = 0xFF000000; // [31:24]
        } m_dw1;

        struct DW2 {
            U32 m_register = 0;
            [[nodiscard]] auto parent_hub_slot_id() const -> U8;
            [[nodiscard]] auto parent_port_num()    const -> U8;
            [[nodiscard]] auto TTT()                const -> U8;
            [[nodiscard]] auto interrupter_target() const -> U16;
            auto set_parent_hub_slot_id(U8 val)  -> void;
            auto set_parent_port_num(U8 val)     -> void;
            auto set_TTT(U8 val)                 -> void;
            auto set_interrupter_target(U16 val) -> void;
          private:
            static constexpr U32 PARENT_HUB_SLOT_ID_MASK = 0x000000FF; // [7:0]
            static constexpr U32 PARENT_PORT_NUM_MASK     = 0x0000FF00; // [15:8]
            static constexpr U32 TTT_MASK                 = 0x00030000; // [17:16]
            static constexpr U32 INTERRUPTER_TARGET_MASK  = 0xFFC00000; // [31:22]
        } m_dw2;

        struct DW3 {
            static constexpr U8 SLOT_STATE_DISABLED_ENABLED = 0;
            static constexpr U8 SLOT_STATE_DEFAULT          = 1;
            static constexpr U8 SLOT_STATE_ADDRESSED        = 2;
            static constexpr U8 SLOT_STATE_CONFIGURED       = 3;

            U32 m_register = 0;
            [[nodiscard]] auto usb_device_address() const -> U8;
            [[nodiscard]] auto slot_state()         const -> U8;
            auto set_usb_device_address(U8 val) -> void;
            auto set_slot_state(U8 val)         -> void;
          private:
            static constexpr U32 USB_DEVICE_ADDRESS_MASK = 0x000000FF; // [7:0]
            static constexpr U32 SLOT_STATE_MASK         = 0xF8000000; // [31:27]
        } m_dw3;

        U32 m_reserved_0 = 0;
        U32 m_reserved_1 = 0;
        U32 m_reserved_2 = 0;
        U32 m_reserved_3 = 0;
    };
    static_assert(sizeof(SlotContext) == XHCI_CONTEXT_DWORDS * sizeof(U32));

    /// @brief Endpoint Context — describes one endpoint (xHCI 2.0 §6.2.3 Table 6-9).
    struct EndpointContext {
        static constexpr U8 EP_TYPE_NOT_VALID     = 0;
        static constexpr U8 EP_TYPE_ISOCH_OUT     = 1;
        static constexpr U8 EP_TYPE_BULK_OUT      = 2;
        static constexpr U8 EP_TYPE_INTERRUPT_OUT = 3;
        static constexpr U8 EP_TYPE_CONTROL       = 4;
        static constexpr U8 EP_TYPE_ISOCH_IN      = 5;
        static constexpr U8 EP_TYPE_BULK_IN       = 6;
        static constexpr U8 EP_TYPE_INTERRUPT_IN  = 7;

        static constexpr U8 EP_STATE_DISABLED = 0;
        static constexpr U8 EP_STATE_RUNNING  = 1;
        static constexpr U8 EP_STATE_HALTED   = 2;
        static constexpr U8 EP_STATE_STOPPED  = 3;
        static constexpr U8 EP_STATE_ERROR    = 4;

        struct DW0 {
            U32 m_register = 0;
            [[nodiscard]] auto ep_state()      const -> U8;
            [[nodiscard]] auto mult()          const -> U8;
            [[nodiscard]] auto max_p_streams() const -> U8;
            [[nodiscard]] auto LSA()           const -> bool;
            [[nodiscard]] auto interval()      const -> U8;
            [[nodiscard]] auto max_esit_hi()   const -> U8;
            auto set_ep_state(U8 val)      -> void;
            auto set_mult(U8 val)          -> void;
            auto set_max_p_streams(U8 val) -> void;
            auto set_LSA(bool v)           -> void;
            auto set_interval(U8 val)      -> void;
            auto set_max_esit_hi(U8 val)   -> void;
          private:
            static constexpr U32 EP_STATE_MASK      = 0x00000007; // [2:0]
            static constexpr U32 MULT_MASK          = 0x00000300; // [9:8]
            static constexpr U32 MAX_P_STREAMS_MASK = 0x00007C00; // [14:10]
            static constexpr U8  LSA_BIT_OFFSET     = 15;
            static constexpr U32 INTERVAL_MASK      = 0x00FF0000; // [23:16]
            static constexpr U32 MAX_ESIT_HI_MASK   = 0xFF000000; // [31:24]
        } m_dw0;

        struct DW1 {
            U32 m_register = 0;
            [[nodiscard]] auto CERR()            const -> U8;
            [[nodiscard]] auto ep_type()         const -> U8;
            [[nodiscard]] auto HID()             const -> bool;
            [[nodiscard]] auto max_burst_size()  const -> U8;
            [[nodiscard]] auto max_packet_size() const -> U16;
            auto set_CERR(U8 val)            -> void;
            auto set_ep_type(U8 val)         -> void;
            auto set_HID(bool v)             -> void;
            auto set_max_burst_size(U8 val)  -> void;
            auto set_max_packet_size(U16 val) -> void;
          private:
            static constexpr U32 CERR_MASK            = 0x00000006; // [2:1]
            static constexpr U32 EP_TYPE_MASK         = 0x00000038; // [5:3]
            static constexpr U8  HID_BIT_OFFSET       = 7;
            static constexpr U32 MAX_BURST_SIZE_MASK  = 0x0000FF00; // [15:8]
            static constexpr U32 MAX_PACKET_SIZE_MASK = 0xFFFF0000; // [31:16]
        } m_dw1;

        // DW2-DW3: TR Dequeue Pointer — 16-byte aligned, bit 0 = DCS.
        struct TRDequeuePtr {
            U64 m_register = 0;
            [[nodiscard]] auto DCS() const -> bool; // bit 0 — Dequeue Cycle State
            [[nodiscard]] auto ptr() const -> U64;  // bits [63:4], val = phys >> 4
            auto set_DCS(bool v)  -> void;
            auto set_ptr(U64 val) -> void; // val = phys >> 4
          private:
            static constexpr U8  DCS_BIT_OFFSET = 0;
            static constexpr U64 PTR_MASK        = 0xFFFFFFFFFFFFFFF0ULL; // [63:4]
        } m_tr_dequeue_ptr;

        struct DW4 {
            U32 m_register = 0;
            [[nodiscard]] auto average_trb_length() const -> U16;
            [[nodiscard]] auto max_esit_lo()        const -> U16;
            auto set_average_trb_length(U16 val) -> void;
            auto set_max_esit_lo(U16 val)        -> void;
          private:
            static constexpr U32 AVERAGE_TRB_LENGTH_MASK = 0x0000FFFF; // [15:0]
            static constexpr U32 MAX_ESIT_LO_MASK        = 0xFFFF0000; // [31:16]
        } m_dw4;

        U32 m_reserved_0 = 0;
        U32 m_reserved_1 = 0;
        U32 m_reserved_2 = 0;
    };
    static_assert(sizeof(EndpointContext) == XHCI_CONTEXT_DWORDS * sizeof(U32));

    /// @brief Output Device Context (xHCI 2.0 §6.2.1).
    struct DeviceContext {
        static constexpr size_t MAX_ENDPOINTS = 31;

        SlotContext                           m_slot_context{};
        Array<EndpointContext, MAX_ENDPOINTS> m_endpoint_contexts{};
    };
    static_assert(sizeof(DeviceContext)
                  == sizeof(SlotContext)
                         + (DeviceContext::MAX_ENDPOINTS * sizeof(EndpointContext)));

    // ========================================================================================== //
    // Input Context — §6.2.5
    // ========================================================================================== //

    /// @brief Input Control Context (xHCI 2.0 §6.2.5.1 Table 6-13).
    struct InputControlContext {
        struct DropContextFlags {
            U32 m_register = 0;
            [[nodiscard]] auto D() const -> U32; // bits [31:2] — drop flags for endpoints 2-31
            auto set_D(U32 val) -> void;
          private:
            static constexpr U32 D_MASK = 0xFFFFFFFC; // [31:2]
        } m_drop_context_flags;

        // A0 = update Slot Context; A1-A31 = add/update endpoint contexts 1-31.
        U32 m_add_context_flags = 0;

        U32 m_reserved_0 = 0;
        U32 m_reserved_1 = 0;
        U32 m_reserved_2 = 0;
        U32 m_reserved_3 = 0;
        U32 m_reserved_4 = 0;

        struct InputControlContextFieldDefs {
            U32 m_register = 0;
            [[nodiscard]] auto configuration_value() const -> U8;
            [[nodiscard]] auto interface_number()    const -> U8;
            [[nodiscard]] auto alternate_setting()   const -> U8;
            auto set_configuration_value(U8 val) -> void;
            auto set_interface_number(U8 val)    -> void;
            auto set_alternate_setting(U8 val)   -> void;
          private:
            static constexpr U32 CONFIGURATION_VALUE_MASK = 0x000000FF; // [7:0]
            static constexpr U32 INTERFACE_NUMBER_MASK    = 0x0000FF00; // [15:8]
            static constexpr U32 ALTERNATE_SETTING_MASK   = 0x00FF0000; // [23:16]
        } m_input_control_context_field_defs;
    };
    static_assert(sizeof(InputControlContext) == sizeof(SlotContext));

    /// @brief Input Context — passed to Address Device / Configure Endpoint commands (§6.2.5).
    struct InputContext {
        InputControlContext                                  m_input_control_context{};
        SlotContext                                          m_slot_context{};
        Array<EndpointContext, DeviceContext::MAX_ENDPOINTS> m_endpoint_contexts{};
    };
    static_assert(sizeof(InputContext)
                  == sizeof(InputControlContext) + sizeof(SlotContext)
                         + (DeviceContext::MAX_ENDPOINTS * sizeof(EndpointContext)));

    // ========================================================================================== //
    // Device Context Base Address Array (DCBAA) — §6.1
    // ========================================================================================== //

    using DeviceContextBaseAddressArray = PhysicalAddr*;

} // namespace Rune::Device::USB

#endif // RUNEOS_DEVICECONTEXT_H