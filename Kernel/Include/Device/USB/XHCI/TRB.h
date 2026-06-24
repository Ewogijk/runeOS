
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

    /// @brief Generic TRB providing raw 4-DWORD access.
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

        struct StatusDWord {
            U32 m_register = 0;
            [[nodiscard]] auto trb_transfer_length() const -> U32;
            [[nodiscard]] auto td_size()             const -> U8;
            [[nodiscard]] auto interrupter_target()  const -> U16;
            auto set_trb_transfer_length(U32 val) -> void;
            auto set_td_size(U8 val)              -> void;
            auto set_interrupter_target(U16 val)  -> void;
          private:
            static constexpr U32 TRB_TRANSFER_LENGTH_MASK = 0x0001FFFF; // [16:0]
            static constexpr U32 TD_SIZE_MASK             = 0x003E0000; // [21:17]
            static constexpr U32 INTERRUPTER_TARGET_MASK  = 0xFFC00000; // [31:22]
        } m_status;

        struct ControlDWord {
            U32 m_register = 0;
            [[nodiscard]] auto cycle()            const -> bool;
            [[nodiscard]] auto ENT()              const -> bool;
            [[nodiscard]] auto ISP()              const -> bool;
            [[nodiscard]] auto NS()               const -> bool;
            [[nodiscard]] auto chain()            const -> bool;
            [[nodiscard]] auto IOC()              const -> bool;
            [[nodiscard]] auto IDT()              const -> bool;
            [[nodiscard]] auto BEI()              const -> bool;
            [[nodiscard]] auto trb_type()         const -> U8;
            [[nodiscard]] auto td_size_extended() const -> U8;
            auto set_cycle(bool v)            -> void;
            auto set_ENT(bool v)              -> void;
            auto set_ISP(bool v)              -> void;
            auto set_NS(bool v)               -> void;
            auto set_chain(bool v)            -> void;
            auto set_IOC(bool v)              -> void;
            auto set_IDT(bool v)              -> void;
            auto set_BEI(bool v)              -> void;
            auto set_trb_type(U8 val)         -> void;
            auto set_td_size_extended(U8 val) -> void;
          private:
            static constexpr U8  CYCLE_BIT_OFFSET      = 0;
            static constexpr U8  ENT_BIT_OFFSET        = 1;
            static constexpr U8  ISP_BIT_OFFSET        = 2;
            static constexpr U8  NS_BIT_OFFSET         = 3;
            static constexpr U8  CHAIN_BIT_OFFSET      = 4;
            static constexpr U8  IOC_BIT_OFFSET        = 5;
            static constexpr U8  IDT_BIT_OFFSET        = 6;
            static constexpr U8  BEI_BIT_OFFSET        = 9;
            static constexpr U32 TRB_TYPE_MASK         = 0x0000FC00; // [15:10]
            static constexpr U32 TD_SIZE_EXTENDED_MASK = 0x007F0000; // [22:16]
        } m_control;
    };
    static_assert(sizeof(NormalTRB) == sizeof(TRB));

    /// @brief Setup Stage TRB for control transfers (xHCI 2.0 §6.4.1.2.1, type = 2).
    struct SetupStageTRB {
        static constexpr U8 TYPE         = TRBType::SETUP_STAGE;
        static constexpr U8 TRT_NO_DATA  = 0;
        static constexpr U8 TRT_OUT_DATA = 2;
        static constexpr U8 TRT_IN_DATA  = 3;

        struct RequestDWord {
            U32 m_register = 0;
            [[nodiscard]] auto bm_request_type() const -> U8;
            [[nodiscard]] auto b_request()       const -> U8;
            [[nodiscard]] auto w_value()         const -> U16;
            auto set_bm_request_type(U8 val)  -> void;
            auto set_b_request(U8 val)        -> void;
            auto set_w_value(U16 val)         -> void;
          private:
            static constexpr U32 BM_REQUEST_TYPE_MASK = 0x000000FF; // [7:0]
            static constexpr U32 B_REQUEST_MASK       = 0x0000FF00; // [15:8]
            static constexpr U32 W_VALUE_MASK         = 0xFFFF0000; // [31:16]
        } m_request;

        struct IndexLengthDWord {
            U32 m_register = 0;
            [[nodiscard]] auto w_index()  const -> U16;
            [[nodiscard]] auto w_length() const -> U16;
            auto set_w_index(U16 val)  -> void;
            auto set_w_length(U16 val) -> void;
          private:
            static constexpr U32 W_INDEX_MASK  = 0x0000FFFF; // [15:0]
            static constexpr U32 W_LENGTH_MASK = 0xFFFF0000; // [31:16]
        } m_index_length;

        struct StatusDWord {
            U32 m_register = 0;
            [[nodiscard]] auto trb_transfer_length() const -> U32;
            [[nodiscard]] auto interrupter_target()  const -> U16;
            auto set_trb_transfer_length(U32 val) -> void;
            auto set_interrupter_target(U16 val)  -> void;
          private:
            static constexpr U32 TRB_TRANSFER_LENGTH_MASK = 0x0001FFFF; // [16:0]
            static constexpr U32 INTERRUPTER_TARGET_MASK  = 0xFFC00000; // [31:22]
        } m_status;

        struct ControlDWord {
            U32 m_register = 0;
            [[nodiscard]] auto cycle()    const -> bool;
            [[nodiscard]] auto IOC()      const -> bool;
            [[nodiscard]] auto IDT()      const -> bool;
            [[nodiscard]] auto trb_type() const -> U8;
            [[nodiscard]] auto TRT()      const -> U8;
            auto set_cycle(bool v)    -> void;
            auto set_IOC(bool v)      -> void;
            auto set_IDT(bool v)      -> void;
            auto set_trb_type(U8 val) -> void;
            auto set_TRT(U8 val)      -> void;
          private:
            static constexpr U8  CYCLE_BIT_OFFSET = 0;
            static constexpr U8  IOC_BIT_OFFSET   = 5;
            static constexpr U8  IDT_BIT_OFFSET   = 6;
            static constexpr U32 TRB_TYPE_MASK    = 0x0000FC00; // [15:10]
            static constexpr U32 TRT_MASK         = 0x00030000; // [17:16]
        } m_control;
    };
    static_assert(sizeof(SetupStageTRB) == sizeof(TRB));

    /// @brief Data Stage TRB for control transfers (xHCI 2.0 §6.4.1.2.2, type = 3).
    struct DataStageTRB {
        static constexpr U8 TYPE = TRBType::DATA_STAGE;

        U32 m_data_buffer_pointer_lo = 0;
        U32 m_data_buffer_pointer_hi = 0;

        struct StatusDWord {
            U32 m_register = 0;
            [[nodiscard]] auto trb_transfer_length() const -> U32;
            [[nodiscard]] auto td_size()             const -> U8;
            [[nodiscard]] auto interrupter_target()  const -> U16;
            auto set_trb_transfer_length(U32 val) -> void;
            auto set_td_size(U8 val)              -> void;
            auto set_interrupter_target(U16 val)  -> void;
          private:
            static constexpr U32 TRB_TRANSFER_LENGTH_MASK = 0x0001FFFF; // [16:0]
            static constexpr U32 TD_SIZE_MASK             = 0x003E0000; // [21:17]
            static constexpr U32 INTERRUPTER_TARGET_MASK  = 0xFFC00000; // [31:22]
        } m_status;

        struct ControlDWord {
            U32 m_register = 0;
            [[nodiscard]] auto cycle()    const -> bool;
            [[nodiscard]] auto ENT()      const -> bool;
            [[nodiscard]] auto ISP()      const -> bool;
            [[nodiscard]] auto NS()       const -> bool;
            [[nodiscard]] auto chain()    const -> bool;
            [[nodiscard]] auto IOC()      const -> bool;
            [[nodiscard]] auto IDT()      const -> bool;
            [[nodiscard]] auto trb_type() const -> U8;
            [[nodiscard]] auto DIR()      const -> bool;
            auto set_cycle(bool v)    -> void;
            auto set_ENT(bool v)      -> void;
            auto set_ISP(bool v)      -> void;
            auto set_NS(bool v)       -> void;
            auto set_chain(bool v)    -> void;
            auto set_IOC(bool v)      -> void;
            auto set_IDT(bool v)      -> void;
            auto set_trb_type(U8 val) -> void;
            auto set_DIR(bool v)      -> void;
          private:
            static constexpr U8  CYCLE_BIT_OFFSET = 0;
            static constexpr U8  ENT_BIT_OFFSET   = 1;
            static constexpr U8  ISP_BIT_OFFSET   = 2;
            static constexpr U8  NS_BIT_OFFSET    = 3;
            static constexpr U8  CHAIN_BIT_OFFSET = 4;
            static constexpr U8  IOC_BIT_OFFSET   = 5;
            static constexpr U8  IDT_BIT_OFFSET   = 6;
            static constexpr U32 TRB_TYPE_MASK    = 0x0000FC00; // [15:10]
            static constexpr U8  DIR_BIT_OFFSET   = 16;
        } m_control;
    };
    static_assert(sizeof(DataStageTRB) == sizeof(TRB));

    /// @brief Status Stage TRB for control transfers (xHCI 2.0 §6.4.1.2.3, type = 4).
    struct StatusStageTRB {
        static constexpr U8 TYPE = TRBType::STATUS_STAGE;

        U32 m_reserved_0 = 0;
        U32 m_reserved_1 = 0;

        struct StatusDWord {
            U32 m_register = 0;
            [[nodiscard]] auto interrupter_target() const -> U16;
            auto set_interrupter_target(U16 val) -> void;
          private:
            static constexpr U32 INTERRUPTER_TARGET_MASK = 0xFFC00000; // [31:22]
        } m_status;

        struct ControlDWord {
            U32 m_register = 0;
            [[nodiscard]] auto cycle()    const -> bool;
            [[nodiscard]] auto ENT()      const -> bool;
            [[nodiscard]] auto IOC()      const -> bool;
            [[nodiscard]] auto trb_type() const -> U8;
            [[nodiscard]] auto DIR()      const -> bool;
            auto set_cycle(bool v)    -> void;
            auto set_ENT(bool v)      -> void;
            auto set_IOC(bool v)      -> void;
            auto set_trb_type(U8 val) -> void;
            auto set_DIR(bool v)      -> void;
          private:
            static constexpr U8  CYCLE_BIT_OFFSET = 0;
            static constexpr U8  ENT_BIT_OFFSET   = 1;
            static constexpr U8  IOC_BIT_OFFSET   = 5;
            static constexpr U32 TRB_TYPE_MASK    = 0x0000FC00; // [15:10]
            static constexpr U8  DIR_BIT_OFFSET   = 16;
        } m_control;
    };
    static_assert(sizeof(StatusStageTRB) == sizeof(TRB));

    /// @brief Link TRB — links ring segments (xHCI 2.0 §6.4.4.1, type = 6).
    struct LinkTRB {
        static constexpr U8 TYPE = TRBType::LINK;

        struct RingSegmentPtrLoDWord {
            U32 m_register = 0;
            [[nodiscard]] auto ptr() const -> U32; // bits [31:4], val = phys >> 4
            auto set_ptr(U32 val) -> void;         // val = phys >> 4
          private:
            static constexpr U32 PTR_MASK = 0xFFFFFFF0; // [31:4]
        } m_ring_segment_pointer_lo;

        U32 m_ring_segment_pointer_hi = 0;

        struct StatusDWord {
            U32 m_register = 0;
            [[nodiscard]] auto interrupter_target() const -> U16;
            auto set_interrupter_target(U16 val) -> void;
          private:
            static constexpr U32 INTERRUPTER_TARGET_MASK = 0xFFC00000; // [31:22]
        } m_status;

        struct ControlDWord {
            U32 m_register = 0;
            [[nodiscard]] auto cycle()        const -> bool;
            [[nodiscard]] auto toggle_cycle() const -> bool;
            [[nodiscard]] auto chain()        const -> bool;
            [[nodiscard]] auto IOC()          const -> bool;
            [[nodiscard]] auto trb_type()     const -> U8;
            auto set_cycle(bool v)        -> void;
            auto set_toggle_cycle(bool v) -> void;
            auto set_chain(bool v)        -> void;
            auto set_IOC(bool v)          -> void;
            auto set_trb_type(U8 val)     -> void;
          private:
            static constexpr U8  CYCLE_BIT_OFFSET        = 0;
            static constexpr U8  TOGGLE_CYCLE_BIT_OFFSET = 1;
            static constexpr U8  CHAIN_BIT_OFFSET        = 4;
            static constexpr U8  IOC_BIT_OFFSET          = 5;
            static constexpr U32 TRB_TYPE_MASK           = 0x0000FC00; // [15:10]
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

        struct ControlDWord {
            U32 m_register = 0;
            [[nodiscard]] auto cycle()     const -> bool;
            [[nodiscard]] auto trb_type()  const -> U8;
            [[nodiscard]] auto slot_type() const -> U8;
            auto set_cycle(bool v)     -> void;
            auto set_trb_type(U8 val)  -> void;
            auto set_slot_type(U8 val) -> void;
          private:
            static constexpr U8  CYCLE_BIT_OFFSET = 0;
            static constexpr U32 TRB_TYPE_MASK    = 0x0000FC00; // [15:10]
            static constexpr U32 SLOT_TYPE_MASK   = 0x001F0000; // [20:16]
        } m_control;
    };
    static_assert(sizeof(EnableSlotCommandTRB) == sizeof(TRB));

    /// @brief Disable Slot Command TRB (xHCI 2.0 §6.4.3.3, type = 10).
    struct DisableSlotCommandTRB {
        static constexpr U8 TYPE = TRBType::DISABLE_SLOT;

        U32 m_reserved_0 = 0;
        U32 m_reserved_1 = 0;
        U32 m_reserved_2 = 0;

        struct ControlDWord {
            U32 m_register = 0;
            [[nodiscard]] auto cycle()    const -> bool;
            [[nodiscard]] auto trb_type() const -> U8;
            [[nodiscard]] auto slot_id()  const -> U8;
            auto set_cycle(bool v)    -> void;
            auto set_trb_type(U8 val) -> void;
            auto set_slot_id(U8 val)  -> void;
          private:
            static constexpr U8  CYCLE_BIT_OFFSET = 0;
            static constexpr U32 TRB_TYPE_MASK    = 0x0000FC00; // [15:10]
            static constexpr U32 SLOT_ID_MASK     = 0xFF000000; // [31:24]
        } m_control;
    };
    static_assert(sizeof(DisableSlotCommandTRB) == sizeof(TRB));

    /// @brief Address Device Command TRB (xHCI 2.0 §6.4.3.4, type = 11).
    struct AddressDeviceCommandTRB {
        static constexpr U8 TYPE = TRBType::ADDRESS_DEVICE;

        struct InputContextPtrLoDWord {
            U32 m_register = 0;
            [[nodiscard]] auto ptr() const -> U32; // bits [31:4], val = phys >> 4
            auto set_ptr(U32 val) -> void;
          private:
            static constexpr U32 PTR_MASK = 0xFFFFFFF0; // [31:4]
        } m_input_context_ptr_lo;

        U32 m_input_context_ptr_hi = 0;
        U32 m_reserved_0           = 0;

        struct ControlDWord {
            U32 m_register = 0;
            [[nodiscard]] auto cycle()    const -> bool;
            [[nodiscard]] auto BSR()      const -> bool;
            [[nodiscard]] auto trb_type() const -> U8;
            [[nodiscard]] auto slot_id()  const -> U8;
            auto set_cycle(bool v)    -> void;
            auto set_BSR(bool v)      -> void;
            auto set_trb_type(U8 val) -> void;
            auto set_slot_id(U8 val)  -> void;
          private:
            static constexpr U8  CYCLE_BIT_OFFSET = 0;
            static constexpr U8  BSR_BIT_OFFSET   = 9;
            static constexpr U32 TRB_TYPE_MASK    = 0x0000FC00; // [15:10]
            static constexpr U32 SLOT_ID_MASK     = 0xFF000000; // [31:24]
        } m_control;
    };
    static_assert(sizeof(AddressDeviceCommandTRB) == sizeof(TRB));

    /// @brief Configure Endpoint Command TRB (xHCI 2.0 §6.4.3.5, type = 12).
    struct ConfigureEndpointCommandTRB {
        static constexpr U8 TYPE = TRBType::CONFIGURE_ENDPOINT;

        struct InputContextPtrLoDWord {
            U32 m_register = 0;
            [[nodiscard]] auto ptr() const -> U32; // bits [31:4], val = phys >> 4
            auto set_ptr(U32 val) -> void;
          private:
            static constexpr U32 PTR_MASK = 0xFFFFFFF0; // [31:4]
        } m_input_context_ptr_lo;

        U32 m_input_context_ptr_hi = 0;
        U32 m_reserved_0           = 0;

        struct ControlDWord {
            U32 m_register = 0;
            [[nodiscard]] auto cycle()    const -> bool;
            [[nodiscard]] auto DC()       const -> bool;
            [[nodiscard]] auto trb_type() const -> U8;
            [[nodiscard]] auto slot_id()  const -> U8;
            auto set_cycle(bool v)    -> void;
            auto set_DC(bool v)       -> void;
            auto set_trb_type(U8 val) -> void;
            auto set_slot_id(U8 val)  -> void;
          private:
            static constexpr U8  CYCLE_BIT_OFFSET = 0;
            static constexpr U8  DC_BIT_OFFSET    = 9;
            static constexpr U32 TRB_TYPE_MASK    = 0x0000FC00; // [15:10]
            static constexpr U32 SLOT_ID_MASK     = 0xFF000000; // [31:24]
        } m_control;
    };
    static_assert(sizeof(ConfigureEndpointCommandTRB) == sizeof(TRB));

    /// @brief No-Op Command TRB (xHCI 2.0 §6.4.3.1, type = 23).
    struct NoOpCommandTRB {
        static constexpr U8 TYPE = TRBType::NO_OP_COMMAND;

        U32 m_reserved_0 = 0;
        U32 m_reserved_1 = 0;
        U32 m_reserved_2 = 0;

        struct ControlDWord {
            U32 m_register = 0;
            [[nodiscard]] auto cycle()    const -> bool;
            [[nodiscard]] auto trb_type() const -> U8;
            auto set_cycle(bool v)    -> void;
            auto set_trb_type(U8 val) -> void;
          private:
            static constexpr U8  CYCLE_BIT_OFFSET = 0;
            static constexpr U32 TRB_TYPE_MASK    = 0x0000FC00; // [15:10]
        } m_control;
    };
    static_assert(sizeof(NoOpCommandTRB) == sizeof(TRB));

    // ========================================================================================== //
    // Event TRBs — §6.4.2
    // ========================================================================================== //

    /// @brief Transfer Event TRB (xHCI 2.0 §6.4.2.1, type = 32).
    struct TransferEventTRB {
        static constexpr U8 TYPE = TRBType::TRANSFER_EVENT;

        U32 m_trb_pointer_lo = 0;
        U32 m_trb_pointer_hi = 0;

        struct StatusDWord {
            U32 m_register = 0;
            [[nodiscard]] auto trb_transfer_length() const -> U32;
            [[nodiscard]] auto completion_code()     const -> U8;
            auto set_trb_transfer_length(U32 val) -> void;
            auto set_completion_code(U8 val)      -> void;
          private:
            static constexpr U32 TRB_TRANSFER_LENGTH_MASK = 0x00FFFFFF; // [23:0]
            static constexpr U32 COMPLETION_CODE_MASK     = 0xFF000000; // [31:24]
        } m_status;

        struct ControlDWord {
            U32 m_register = 0;
            [[nodiscard]] auto cycle()       const -> bool;
            [[nodiscard]] auto ED()          const -> bool;
            [[nodiscard]] auto trb_type()    const -> U8;
            [[nodiscard]] auto endpoint_id() const -> U8;
            [[nodiscard]] auto slot_id()     const -> U8;
            auto set_cycle(bool v)       -> void;
            auto set_ED(bool v)          -> void;
            auto set_trb_type(U8 val)    -> void;
            auto set_endpoint_id(U8 val) -> void;
            auto set_slot_id(U8 val)     -> void;
          private:
            static constexpr U8  CYCLE_BIT_OFFSET   = 0;
            static constexpr U8  ED_BIT_OFFSET      = 2;
            static constexpr U32 TRB_TYPE_MASK      = 0x0000FC00; // [15:10]
            static constexpr U32 ENDPOINT_ID_MASK   = 0x001F0000; // [20:16]
            static constexpr U32 SLOT_ID_MASK       = 0xFF000000; // [31:24]
        } m_control;
    };
    static_assert(sizeof(TransferEventTRB) == sizeof(TRB));

    /// @brief Command Completion Event TRB (xHCI 2.0 §6.4.2.2, type = 33).
    struct CommandCompletionEventTRB {
        static constexpr U8 TYPE = TRBType::CMD_COMPLETION;

        struct CommandTrbPtrLoDWord {
            U32 m_register = 0;
            [[nodiscard]] auto ptr() const -> U32; // bits [31:4], val = phys >> 4
            auto set_ptr(U32 val) -> void;
          private:
            static constexpr U32 PTR_MASK = 0xFFFFFFF0; // [31:4]
        } m_command_trb_pointer_lo;

        U32 m_command_trb_pointer_hi = 0;

        struct StatusDWord {
            U32 m_register = 0;
            [[nodiscard]] auto completion_parameter() const -> U32;
            [[nodiscard]] auto completion_code()      const -> U8;
            auto set_completion_parameter(U32 val) -> void;
            auto set_completion_code(U8 val)       -> void;
          private:
            static constexpr U32 COMPLETION_PARAMETER_MASK = 0x00FFFFFF; // [23:0]
            static constexpr U32 COMPLETION_CODE_MASK      = 0xFF000000; // [31:24]
        } m_status;

        struct ControlDWord {
            U32 m_register = 0;
            [[nodiscard]] auto cycle()    const -> bool;
            [[nodiscard]] auto trb_type() const -> U8;
            [[nodiscard]] auto vf_id()    const -> U8;
            [[nodiscard]] auto slot_id()  const -> U8;
            auto set_cycle(bool v)    -> void;
            auto set_trb_type(U8 val) -> void;
            auto set_vf_id(U8 val)    -> void;
            auto set_slot_id(U8 val)  -> void;
          private:
            static constexpr U8  CYCLE_BIT_OFFSET = 0;
            static constexpr U32 TRB_TYPE_MASK    = 0x0000FC00; // [15:10]
            static constexpr U32 VF_ID_MASK       = 0x00FF0000; // [23:16]
            static constexpr U32 SLOT_ID_MASK     = 0xFF000000; // [31:24]
        } m_control;
    };
    static_assert(sizeof(CommandCompletionEventTRB) == sizeof(TRB));

    /// @brief Port Status Change Event TRB (xHCI 2.0 §6.4.2.3, type = 34).
    struct PortStatusChangeEventTRB {
        static constexpr U8 TYPE = TRBType::PORT_STATUS_CHANGE;

        struct PortIdDWord {
            U32 m_register = 0;
            [[nodiscard]] auto port_id() const -> U8; // bits [31:24]
            auto set_port_id(U8 val) -> void;
          private:
            static constexpr U32 PORT_ID_MASK = 0xFF000000; // [31:24]
        } m_port_id;

        U32 m_reserved_0 = 0;

        struct StatusDWord {
            U32 m_register = 0;
            [[nodiscard]] auto completion_code() const -> U8; // bits [31:24]
            auto set_completion_code(U8 val) -> void;
          private:
            static constexpr U32 COMPLETION_CODE_MASK = 0xFF000000; // [31:24]
        } m_status;

        struct ControlDWord {
            U32 m_register = 0;
            [[nodiscard]] auto cycle()    const -> bool;
            [[nodiscard]] auto trb_type() const -> U8;
            auto set_cycle(bool v)    -> void;
            auto set_trb_type(U8 val) -> void;
          private:
            static constexpr U8  CYCLE_BIT_OFFSET = 0;
            static constexpr U32 TRB_TYPE_MASK    = 0x0000FC00; // [15:10]
        } m_control;
    };
    static_assert(sizeof(PortStatusChangeEventTRB) == sizeof(TRB));

} // namespace Rune::Device::USB

#endif // RUNEOS_TRB_H