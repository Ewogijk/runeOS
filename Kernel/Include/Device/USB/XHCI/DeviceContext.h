
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

namespace Rune::Device::USB {
    // Spec-mandated sizes and limits used in template constraints and static_asserts.
    static constexpr size_t XHCI_CONTEXT_DWORDS = 8; // every context is exactly 8 DWORDs
    static constexpr size_t XHCI_MAX_SLOTS      = 255;

    // ========================================================================================== //
    // Device Context — §6.2
    // ========================================================================================== //

    /// @brief Slot Context — describes the overall device (xHCI 2.0 §6.2.2 Table 6-7).
    struct SlotContext {
        union DW0 {
            U32 m_as_u32 = 0;
            struct {
                U32 m_route_string    : 20;
                U32 m_speed           : 4;
                U32 m_reserved_0      : 1;
                U32 m_mtt             : 1; // Multi-TT hub
                U32 m_hub             : 1;
                U32 m_context_entries : 5; // index of the last valid EP context (1-31)
            };
        } m_dw0;

        union DW1 {
            U32 m_as_u32 = 0;
            struct {
                U32 m_max_exit_latency  : 16;
                U32 m_root_hub_port_num : 8;
                U32 m_num_ports         : 8; // valid only when hub == 1
            };
        } m_dw1;

        union DW2 {
            U32 m_as_u32 = 0;
            struct {
                U32 m_parent_hub_slot_id : 8;
                U32 m_parent_port_num    : 8;
                U32 m_ttt                : 2; // TT Think Time (hub only)
                U32 m_reserved_0         : 4;
                U32 m_interrupter_target : 10;
            };
        } m_dw2;

        union DW3 {
            static constexpr U8 SLOT_STATE_DISABLED_ENABLED = 0;
            static constexpr U8 SLOT_STATE_DEFAULT          = 1;
            static constexpr U8 SLOT_STATE_ADDRESSED        = 2;
            static constexpr U8 SLOT_STATE_CONFIGURED       = 3;

            U32 m_as_u32 = 0;
            struct {
                U32 m_usb_device_address : 8;
                U32 m_reserved_0         : 19;
                U32 m_slot_state         : 5;
            };
        } m_dw3;

        U32 m_reserved_0 = 0;
        U32 m_reserved_1 = 0;
        U32 m_reserved_2 = 0;
        U32 m_reserved_3 = 0;
    };
    static_assert(sizeof(SlotContext) == XHCI_CONTEXT_DWORDS * sizeof(U32));

    /// @brief Endpoint Context — describes one endpoint (xHCI 2.0 §6.2.3 Table 6-9).
    struct EndpointContext {
        // EP Type values (DW1 bits 5:3)
        static constexpr U8 EP_TYPE_NOT_VALID     = 0;
        static constexpr U8 EP_TYPE_ISOCH_OUT     = 1;
        static constexpr U8 EP_TYPE_BULK_OUT      = 2;
        static constexpr U8 EP_TYPE_INTERRUPT_OUT = 3;
        static constexpr U8 EP_TYPE_CONTROL       = 4;
        static constexpr U8 EP_TYPE_ISOCH_IN      = 5;
        static constexpr U8 EP_TYPE_BULK_IN       = 6;
        static constexpr U8 EP_TYPE_INTERRUPT_IN  = 7;

        // EP State values (DW0 bits 2:0)
        static constexpr U8 EP_STATE_DISABLED = 0;
        static constexpr U8 EP_STATE_RUNNING  = 1;
        static constexpr U8 EP_STATE_HALTED   = 2;
        static constexpr U8 EP_STATE_STOPPED  = 3;
        static constexpr U8 EP_STATE_ERROR    = 4;

        union DW0 {
            U32 m_as_u32 = 0;
            struct {
                U32 m_ep_state      : 3;
                U32 m_reserved_0    : 5;
                U32 m_mult          : 2; // SS/SSP isoch only: additional transaction bursts
                U32 m_max_p_streams : 5; // max primary stream array size exponent
                U32 m_lsa           : 1; // Linear Stream Array
                U32 m_interval      : 8; // service interval exponent: 125 µs * 2^interval
                U32 m_max_esit_hi   : 8; // Max ESIT Payload (bits 15:8)
            };
        } m_dw0;

        union DW1 {
            U32 m_as_u32 = 0;
            struct {
                U32 m_reserved_0      : 1;
                U32 m_cerr            : 2; // Error Count: retries before reporting Stall
                U32 m_ep_type         : 3; // EP_TYPE_*
                U32 m_reserved_1      : 1;
                U32 m_hid             : 1; // Host Initiate Disable
                U32 m_max_burst_size  : 8;
                U32 m_max_packet_size : 16;
            };
        } m_dw1;

        // DW2-DW3: TR Dequeue Pointer — 16-byte aligned, bit 0 = DCS.
        union TRDequeuePtr {
            U64 m_as_u64 = 0;
            struct {
                U64 m_dcs            : 1; // Dequeue Cycle State
                U64 m_reserved_0     : 3;
                U64 m_tr_dequeue_ptr : 60; // physical base address >> 4
            };
        } m_tr_dequeue_ptr;

        union DW4 {
            U32 m_as_u32 = 0;
            struct {
                U32 m_average_trb_length : 16;
                U32 m_max_esit_lo        : 16; // Max ESIT Payload bits 7:0 (bits 15:8 in DW0)
            };
        } m_dw4;

        U32 m_reserved_0 = 0;
        U32 m_reserved_1 = 0;
        U32 m_reserved_2 = 0;
    };
    static_assert(sizeof(EndpointContext) == XHCI_CONTEXT_DWORDS * sizeof(U32));

    /// @brief Output Device Context — one Slot Context plus up to 31 Endpoint Contexts
    ///        (xHCI 2.0 §6.2.1).  Index 0 = slot, 1 = EP0 control, 2-31 = EP1–EP15.
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

    /// @brief Input Control Context — selects which contexts the HC should add or drop
    ///        (xHCI 2.0 §6.2.5.1 Table 6-13).
    struct InputControlContext {
        union DropContextFlags {
            U32 m_as_u32 = 0;
            struct {
                U32 m_reserved_0 : 2;  // D0-D1 not usable
                U32 m_d          : 30; // D2-D31: drop endpoint contexts 2-31
            };
        } m_drop_context_flags;

        // A0 = update Slot Context; A1-A31 = add/update endpoint contexts 1-31.
        U32 m_add_context_flags = 0;

        U32 m_reserved_0 = 0;
        U32 m_reserved_1 = 0;
        U32 m_reserved_2 = 0;
        U32 m_reserved_3 = 0;
        U32 m_reserved_4 = 0;
        union InputControlContextFieldDefs {
            U32 m_as_u32 = 0;
            struct {
                U32 m_configuration_value : 8;
                U32 m_interface_number    : 8;
                U32 m_alternate_setting   : 8;
                U32 m_reserved_0          : 8;
            };
        } m_input_control_context_field_defs;
    };
    static_assert(sizeof(InputControlContext) == sizeof(SlotContext));

    /// @brief Input Context — passed to Address Device / Configure Endpoint commands
    ///        (xHCI 2.0 §6.2.5).
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

    /// @brief DCBAA — the array of Output Device Context pointers given to the HC at init
    ///        (xHCI 2.0 §6.1).  Entry 0 is the Scratchpad Buffer Array pointer;
    ///        entries 1-MaxSlots are physical addresses of Output Device Contexts.
    ///        The whole array must be 64-byte aligned.
    template <size_t MaxSlots>
    struct DeviceContextBaseAddressArray {
        static_assert(MaxSlots >= 1 && MaxSlots <= XHCI_MAX_SLOTS);
        Array<U64, MaxSlots + 1> m_entries{};
    };
} // namespace Rune::Device::USB

#endif // RUNEOS_DEVICECONTEXT_H
