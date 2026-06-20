
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

#ifndef RUNEOS_TRB_H
#define RUNEOS_TRB_H

#include <Ember/Ember.h>

namespace Rune::Device::USB {
    // ========================================================================================== //
    // TRB Type Codes (xHCI 2.0 §6.4.6 Table 6-91)
    // ========================================================================================== //

    namespace TRBType {
        // Transfer TRBs
        static constexpr U8 NORMAL       = 1;
        static constexpr U8 SETUP_STAGE  = 2;
        static constexpr U8 DATA_STAGE   = 3;
        static constexpr U8 STATUS_STAGE = 4;
        static constexpr U8 ISOCH        = 5;
        static constexpr U8 LINK         = 6;
        static constexpr U8 EVENT_DATA   = 7;
        static constexpr U8 NO_OP        = 8;
        // Command TRBs
        static constexpr U8 ENABLE_SLOT           = 9;
        static constexpr U8 DISABLE_SLOT          = 10;
        static constexpr U8 ADDRESS_DEVICE        = 11;
        static constexpr U8 CONFIGURE_ENDPOINT    = 12;
        static constexpr U8 EVALUATE_CONTEXT      = 13;
        static constexpr U8 RESET_ENDPOINT        = 14;
        static constexpr U8 STOP_ENDPOINT         = 15;
        static constexpr U8 SET_TR_DEQUEUE_PTR    = 16;
        static constexpr U8 RESET_DEVICE          = 17;
        static constexpr U8 FORCE_EVENT           = 18;
        static constexpr U8 NEGOTIATE_BANDWIDTH   = 19;
        static constexpr U8 SET_LATENCY_TOLERANCE = 20;
        static constexpr U8 GET_PORT_BANDWIDTH    = 21;
        static constexpr U8 FORCE_HEADER          = 22;
        static constexpr U8 NO_OP_COMMAND         = 23;
        static constexpr U8 GET_EXTENDED_PROPERTY = 24;
        static constexpr U8 SET_EXTENDED_PROPERTY = 25;
        // Event TRBs
        static constexpr U8 TRANSFER_EVENT        = 32;
        static constexpr U8 CMD_COMPLETION        = 33;
        static constexpr U8 PORT_STATUS_CHANGE    = 34;
        static constexpr U8 BANDWIDTH_REQUEST     = 35;
        static constexpr U8 DOORBELL_EVENT        = 36;
        static constexpr U8 HOST_CONTROLLER_EVENT = 37;
        static constexpr U8 DEVICE_NOTIFICATION   = 38;
        static constexpr U8 MFINDEX_WRAP          = 39;
    } // namespace TRBType

    // ========================================================================================== //
    // Completion Codes (xHCI 2.0 §6.4.5 Table 6-90)
    // ========================================================================================== //

    namespace CompletionCode {
        static constexpr U8 INVALID                    = 0;
        static constexpr U8 SUCCESS                    = 1;
        static constexpr U8 DATA_BUFFER_ERROR          = 2;
        static constexpr U8 BABBLE_ERROR               = 3;
        static constexpr U8 USB_TRANSACTION_ERROR      = 4;
        static constexpr U8 TRB_ERROR                  = 5;
        static constexpr U8 STALL_ERROR                = 6;
        static constexpr U8 RESOURCE_ERROR             = 7;
        static constexpr U8 BANDWIDTH_ERROR            = 8;
        static constexpr U8 NO_SLOTS_AVAILABLE         = 9;
        static constexpr U8 SHORT_PACKET               = 13;
        static constexpr U8 RING_UNDERRUN              = 14;
        static constexpr U8 RING_OVERRUN               = 15;
        static constexpr U8 VF_ENVENT_RING_FULL        = 16;
        static constexpr U8 PARAMETER_ERROR            = 17;
        static constexpr U8 BANDWIDTH_OVERRUN          = 18;
        static constexpr U8 CONTEXT_STATE_ERROR        = 19;
        static constexpr U8 NO_PING_RESPONSE           = 20;
        static constexpr U8 EVENT_RING_FULL            = 21;
        static constexpr U8 INCOMPATIBLE_DEVICE        = 22;
        static constexpr U8 MISSED_SERVICE             = 23;
        static constexpr U8 CMD_RING_STOPPED           = 24;
        static constexpr U8 CMD_ABORTED                = 25;
        static constexpr U8 STOPPED                    = 26;
        static constexpr U8 STOPPED_LEN_INVAL          = 27;
        static constexpr U8 STOPPED_SHORT_PACKET       = 28;
        static constexpr U8 MAX_EXIT_LATENCY_TOO_LARGE = 29;
        static constexpr U8 ISOCH_BUFFER_OVERRUN       = 31;
        static constexpr U8 EVENT_LOST                 = 32;
        static constexpr U8 UNDEFINED                  = 33;
        static constexpr U8 INVALID_STREAM_ID          = 34;
        static constexpr U8 SECONDARY_BANDWIDTH_ERROR  = 35;
        static constexpr U8 SPLIT_TRANSACTION          = 36;
    } // namespace CompletionCode

    // ========================================================================================== //
    // Transfer Request Block (TRB) — xHCI 2.0 §6.4
    // All TRBs are exactly 16 bytes (four DWORDs).
    // ========================================================================================== //

    /// @brief Generic TRB providing raw 4-DWORD access. Used where a ring slot is addressed
    ///        without knowing its concrete type yet.
    struct TRB {
        U32 m_dw0 = 0;
        U32 m_dw1 = 0;
        U32 m_dw2 = 0;
        U32 m_dw3 = 0;
    };
    static_assert(sizeof(TRB) == 4 * sizeof(U32));

    // ========================================================================================== //
    // Transfer TRBs — §6.4.1
    // ========================================================================================== //

    /// @brief Normal Transfer TRB (xHCI 2.0 §6.4.1.1, type = 1).
    struct NormalTRB {
        static constexpr U8 TYPE = TRBType::NORMAL;

        U32 m_data_buffer_pointer_lo = 0;
        U32 m_data_buffer_pointer_hi = 0;

        union StatusDWord {
            U32 m_as_u32 = 0;
            struct {
                U32 m_trb_transfer_length : 17; // bytes remaining in the TD
                U32 m_td_size             : 5;  // number of packets remaining
                U32 m_interrupter_target  : 10;
            };
        } m_status;

        union ControlDWord {
            U32 m_as_u32 = 0;
            struct {
                U32 m_cycle            : 1;
                U32 m_ent              : 1; // Evaluate Next TRB
                U32 m_isp              : 1; // Interrupt-on Short Packet
                U32 m_ns               : 1; // No Snoop
                U32 m_chain            : 1;
                U32 m_ioc              : 1; // Interrupt on Completion
                U32 m_idt              : 1; // Immediate Data
                U32 m_reserved_0       : 2;
                U32 m_bei              : 1; // Block Event Interrupt
                U32 m_trb_type         : 6; // = TYPE
                U32 m_td_size_extended : 7;
                U32 m_reserved_1       : 9;
            };
        } m_control;
    };
    static_assert(sizeof(NormalTRB) == sizeof(TRB));

    /// @brief Setup Stage TRB for control transfers (xHCI 2.0 §6.4.1.2.1, type = 2).
    struct SetupStageTRB {
        static constexpr U8 TYPE         = TRBType::SETUP_STAGE;
        static constexpr U8 TRT_NO_DATA  = 0; // Transfer Type — no data stage
        static constexpr U8 TRT_OUT_DATA = 2; // Transfer Type — OUT data stage
        static constexpr U8 TRT_IN_DATA  = 3; // Transfer Type — IN data stage

        union RequestDWord {
            U32 m_as_u32 = 0;
            struct {
                U32 m_bm_request_type : 8;
                U32 m_b_request       : 8;
                U32 m_w_value         : 16;
            };
        } m_request;

        union IndexLengthDWord {
            U32 m_as_u32 = 0;
            struct {
                U32 m_w_index  : 16;
                U32 m_w_length : 16;
            };
        } m_index_length;

        union StatusDWord {
            U32 m_as_u32 = 0;
            struct {
                U32 m_trb_transfer_length : 17; // always 8 for Setup Stage
                U32 m_reserved_0          : 5;
                U32 m_interrupter_target  : 10;
            };
        } m_status;

        union ControlDWord {
            U32 m_as_u32 = 0;
            struct {
                U32 m_cycle      : 1;
                U32 m_reserved_0 : 4;
                U32 m_ioc        : 1; // Interrupt on Completion
                U32 m_idt        : 1; // Immediate Data — must be 1 for Setup Stage
                U32 m_reserved_1 : 3;
                U32 m_trb_type   : 6; // = TYPE
                U32 m_trt        : 2; // Transfer Type (TRT_*)
                U32 m_reserved_2 : 14;
            };
        } m_control;
    };
    static_assert(sizeof(SetupStageTRB) == sizeof(TRB));

    /// @brief Data Stage TRB for control transfers (xHCI 2.0 §6.4.1.2.2, type = 3).
    struct DataStageTRB {
        static constexpr U8 TYPE = TRBType::DATA_STAGE;

        U32 m_data_buffer_pointer_lo = 0;
        U32 m_data_buffer_pointer_hi = 0;

        union StatusDWord {
            U32 m_as_u32 = 0;
            struct {
                U32 m_trb_transfer_length : 17;
                U32 m_td_size             : 5;
                U32 m_interrupter_target  : 10;
            };
        } m_status;

        union ControlDWord {
            U32 m_as_u32 = 0;
            struct {
                U32 m_cycle      : 1;
                U32 m_ent        : 1; // Evaluate Next TRB
                U32 m_isp        : 1; // Interrupt-on Short Packet
                U32 m_ns         : 1; // No Snoop
                U32 m_chain      : 1;
                U32 m_ioc        : 1; // Interrupt on Completion
                U32 m_idt        : 1; // Immediate Data
                U32 m_reserved_0 : 3;
                U32 m_trb_type   : 6; // = TYPE
                U32 m_dir        : 1; // Transfer Direction: 0=OUT, 1=IN
                U32 m_reserved_1 : 15;
            };
        } m_control;
    };
    static_assert(sizeof(DataStageTRB) == sizeof(TRB));

    /// @brief Status Stage TRB for control transfers (xHCI 2.0 §6.4.1.2.3, type = 4).
    struct StatusStageTRB {
        static constexpr U8 TYPE = TRBType::STATUS_STAGE;

        U32 m_reserved_0 = 0;
        U32 m_reserved_1 = 0;

        union StatusDWord {
            U32 m_as_u32 = 0;
            struct {
                U32 m_reserved_0         : 22;
                U32 m_interrupter_target : 10;
            };
        } m_status;

        union ControlDWord {
            U32 m_as_u32 = 0;
            struct {
                U32 m_cycle      : 1;
                U32 m_ent        : 1; // Evaluate Next TRB
                U32 m_reserved_0 : 3;
                U32 m_ioc        : 1; // Interrupt on Completion
                U32 m_reserved_1 : 4;
                U32 m_trb_type   : 6; // = TYPE
                U32 m_dir        : 1; // Transfer Direction: 0=OUT, 1=IN
                U32 m_reserved_2 : 15;
            };
        } m_control;
    };
    static_assert(sizeof(StatusStageTRB) == sizeof(TRB));

    /// @brief Link TRB — links ring segments within a Transfer or Command Ring
    ///        (xHCI 2.0 §6.4.4.1, type = 6).
    struct LinkTRB {
        static constexpr U8 TYPE = TRBType::LINK;

        union RingSegmentPtrLoDWord {
            U32 m_as_u32 = 0;
            struct {
                U32 m_reserved_0              : 4; // must be zero (16-byte aligned)
                U32 m_ring_segment_pointer_lo : 28;
            };
        } m_ring_segment_pointer_lo;

        U32 m_ring_segment_pointer_hi = 0;

        union StatusDWord {
            U32 m_as_u32 = 0;
            struct {
                U32 m_reserved_0         : 22;
                U32 m_interrupter_target : 10;
            };
        } m_status;

        union ControlDWord {
            U32 m_as_u32 = 0;
            struct {
                U32 m_cycle        : 1;
                U32 m_toggle_cycle : 1; // TC: invert the cycle bit for the next segment
                U32 m_reserved_0   : 2;
                U32 m_chain        : 1;
                U32 m_ioc          : 1; // Interrupt on Completion
                U32 m_reserved_1   : 4;
                U32 m_trb_type     : 6; // = TYPE
                U32 m_reserved_2   : 16;
            };
        } m_control;
    };
    static_assert(sizeof(LinkTRB) == sizeof(TRB));

    // ========================================================================================== //
    // Command TRBs — §6.4.3
    // ========================================================================================== //

    /// @brief Enable Slot Command TRB (xHCI 2.0 §6.4.3.2, type = 9).
    struct EnableSlotCommandTRB {
        static constexpr U8 TYPE = TRBType::ENABLE_SLOT;

        U32 m_reserved_0 = 0;
        U32 m_reserved_1 = 0;
        U32 m_reserved_2 = 0;

        union ControlDWord {
            U32 m_as_u32 = 0;
            struct {
                U32 m_cycle      : 1;
                U32 m_reserved_0 : 9;
                U32 m_trb_type   : 6; // = TYPE
                U32 m_slot_type  : 5;
                U32 m_reserved_1 : 11;
            };
        } m_control;
    };
    static_assert(sizeof(EnableSlotCommandTRB) == sizeof(TRB));

    /// @brief Disable Slot Command TRB (xHCI 2.0 §6.4.3.3, type = 10).
    struct DisableSlotCommandTRB {
        static constexpr U8 TYPE = TRBType::DISABLE_SLOT;

        U32 m_reserved_0 = 0;
        U32 m_reserved_1 = 0;
        U32 m_reserved_2 = 0;

        union ControlDWord {
            U32 m_as_u32 = 0;
            struct {
                U32 m_cycle      : 1;
                U32 m_reserved_0 : 9;
                U32 m_trb_type   : 6; // = TYPE
                U32 m_reserved_1 : 8;
                U32 m_slot_id    : 8;
            };
        } m_control;
    };
    static_assert(sizeof(DisableSlotCommandTRB) == sizeof(TRB));

    /// @brief Address Device Command TRB (xHCI 2.0 §6.4.3.4, type = 11).
    struct AddressDeviceCommandTRB {
        static constexpr U8 TYPE = TRBType::ADDRESS_DEVICE;

        union InputContextPtrLoDWord {
            U32 m_as_u32 = 0;
            struct {
                U32 m_reserved_0           : 4; // must be zero (16-byte aligned)
                U32 m_input_context_ptr_lo : 28;
            };
        } m_input_context_ptr_lo;

        U32 m_input_context_ptr_hi = 0;
        U32 m_reserved_0           = 0;

        union ControlDWord {
            U32 m_as_u32 = 0;
            struct {
                U32 m_cycle      : 1;
                U32 m_reserved_0 : 8;
                U32 m_bsr        : 1; // Block Set Address Request: keep device in Default state
                U32 m_trb_type   : 6; // = TYPE
                U32 m_reserved_1 : 8;
                U32 m_slot_id    : 8;
            };
        } m_control;
    };
    static_assert(sizeof(AddressDeviceCommandTRB) == sizeof(TRB));

    /// @brief Configure Endpoint Command TRB (xHCI 2.0 §6.4.3.5, type = 12).
    struct ConfigureEndpointCommandTRB {
        static constexpr U8 TYPE = TRBType::CONFIGURE_ENDPOINT;

        union InputContextPtrLoDWord {
            U32 m_as_u32 = 0;
            struct {
                U32 m_reserved_0           : 4;
                U32 m_input_context_ptr_lo : 28;
            };
        } m_input_context_ptr_lo;

        U32 m_input_context_ptr_hi = 0;
        U32 m_reserved_0           = 0;

        union ControlDWord {
            U32 m_as_u32 = 0;
            struct {
                U32 m_cycle      : 1;
                U32 m_reserved_0 : 8;
                U32 m_dc         : 1; // Deconfigure: release all allocated bandwidth
                U32 m_trb_type   : 6; // = TYPE
                U32 m_reserved_1 : 8;
                U32 m_slot_id    : 8;
            };
        } m_control;
    };
    static_assert(sizeof(ConfigureEndpointCommandTRB) == sizeof(TRB));

    /// @brief No-Op Command TRB (xHCI 2.0 §6.4.3.1, type = 23).
    struct NoOpCommandTRB {
        static constexpr U8 TYPE = TRBType::NO_OP_COMMAND;

        U32 m_reserved_0 = 0;
        U32 m_reserved_1 = 0;
        U32 m_reserved_2 = 0;

        union ControlDWord {
            U32 m_as_u32 = 0;
            struct {
                U32 m_cycle      : 1;
                U32 m_reserved_0 : 9;
                U32 m_trb_type   : 6; // = TYPE
                U32 m_reserved_1 : 16;
            };
        } m_control;
    };
    static_assert(sizeof(NoOpCommandTRB) == sizeof(TRB));

    // ========================================================================================== //
    // Event TRBs — §6.4.2
    // ========================================================================================== //

    /// @brief Transfer Event TRB — posted by the HC after a transfer completes
    ///        (xHCI 2.0 §6.4.2.1, type = 32).
    struct TransferEventTRB {
        static constexpr U8 TYPE = TRBType::TRANSFER_EVENT;

        U32 m_trb_pointer_lo = 0; // pointer to the TRB that caused this event
        U32 m_trb_pointer_hi = 0;

        union StatusDWord {
            U32 m_as_u32 = 0;
            struct {
                U32 m_trb_transfer_length : 24; // bytes NOT transferred
                U32 m_completion_code     : 8;  // CompletionCode::*
            };
        } m_status;

        union ControlDWord {
            U32 m_as_u32 = 0;
            struct {
                U32 m_cycle       : 1;
                U32 m_reserved_0  : 1;
                U32 m_ed          : 1; // Event Data: trb_pointer points to an Event Data TRB
                U32 m_reserved_1  : 7;
                U32 m_trb_type    : 6; // = TYPE
                U32 m_endpoint_id : 5; // 1-based endpoint that generated this event
                U32 m_reserved_2  : 3;
                U32 m_slot_id     : 8;
            };
        } m_control;
    };
    static_assert(sizeof(TransferEventTRB) == sizeof(TRB));

    /// @brief Command Completion Event TRB — posted after a command TRB completes
    ///        (xHCI 2.0 §6.4.2.2, type = 33).
    struct CommandCompletionEventTRB {
        static constexpr U8 TYPE = TRBType::CMD_COMPLETION;

        union CommandTrbPtrLoDWord {
            U32 m_as_u32 = 0;
            struct {
                U32 m_reserved_0             : 4;
                U32 m_command_trb_pointer_lo : 28;
            };
        } m_command_trb_pointer_lo;

        U32 m_command_trb_pointer_hi = 0;

        union StatusDWord {
            U32 m_as_u32 = 0;
            struct {
                U32 m_completion_parameter : 24; // command-specific
                U32 m_completion_code      : 8;  // CompletionCode::*
            };
        } m_status;

        union ControlDWord {
            U32 m_as_u32 = 0;
            struct {
                U32 m_cycle      : 1;
                U32 m_reserved_0 : 9;
                U32 m_trb_type   : 6; // = TYPE
                U32 m_vf_id      : 8;
                U32 m_slot_id    : 8;
            };
        } m_control;
    };
    static_assert(sizeof(CommandCompletionEventTRB) == sizeof(TRB));

    /// @brief Port Status Change Event TRB — posted when a root-hub port changes state
    ///        (xHCI 2.0 §6.4.2.3, type = 34).
    struct PortStatusChangeEventTRB {
        static constexpr U8 TYPE = TRBType::PORT_STATUS_CHANGE;

        union PortIdDWord {
            U32 m_as_u32 = 0;
            struct {
                U32 m_reserved_0 : 24;
                U32 m_port_id    : 8; // 1-based root-hub port number
            };
        } m_port_id;

        U32 m_reserved_0 = 0;

        union StatusDWord {
            U32 m_as_u32 = 0;
            struct {
                U32 m_reserved_0      : 24;
                U32 m_completion_code : 8; // CompletionCode::*
            };
        } m_status;

        union ControlDWord {
            U32 m_as_u32 = 0;
            struct {
                U32 m_cycle      : 1;
                U32 m_reserved_0 : 9;
                U32 m_trb_type   : 6; // = TYPE
                U32 m_reserved_1 : 16;
            };
        } m_control;
    };
    static_assert(sizeof(PortStatusChangeEventTRB) == sizeof(TRB));
}

#endif // RUNEOS_TRB_H
