
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

#define TRB_TYPE_VALUES(X)                                                                         \
    X(TRBType, NORMAL, 1)                                                                          \
    X(TRBType, SETUP_STAGE, 2)                                                                     \
    X(TRBType, DATA_STAGE, 3)                                                                      \
    X(TRBType, STATUS_STAGE, 4)                                                                    \
    X(TRBType, ISOCH, 5)                                                                           \
    X(TRBType, LINK, 6)                                                                            \
    X(TRBType, EVENT_DATA, 7)                                                                      \
    X(TRBType, NO_OP, 8)                                                                           \
    X(TRBType, ENABLE_SLOT, 9)                                                                     \
    X(TRBType, DISABLE_SLOT, 10)                                                                   \
    X(TRBType, ADDRESS_DEVICE, 11)                                                                 \
    X(TRBType, CONFIGURE_ENDPOINT, 12)                                                             \
    X(TRBType, EVALUATE_CONTEXT, 13)                                                               \
    X(TRBType, RESET_ENDPOINT, 14)                                                                 \
    X(TRBType, STOP_ENDPOINT, 15)                                                                  \
    X(TRBType, SET_TR_DEQUEUE_PTR, 16)                                                             \
    X(TRBType, RESET_DEVICE, 17)                                                                   \
    X(TRBType, FORCE_EVENT, 18)                                                                    \
    X(TRBType, NEGOTIATE_BANDWIDTH, 19)                                                            \
    X(TRBType, SET_LATENCY_TOLERANCE, 20)                                                          \
    X(TRBType, GET_PORT_BANDWIDTH, 21)                                                             \
    X(TRBType, FORCE_HEADER, 22)                                                                   \
    X(TRBType, NO_OP_COMMAND, 23)                                                                  \
    X(TRBType, GET_EXTENDED_PROPERTY, 24)                                                          \
    X(TRBType, SET_EXTENDED_PROPERTY, 25)                                                          \
    X(TRBType, TRANSFER_EVENT, 32)                                                                 \
    X(TRBType, CMD_COMPLETION, 33)                                                                 \
    X(TRBType, PORT_STATUS_CHANGE, 34)                                                             \
    X(TRBType, BANDWIDTH_REQUEST, 35)                                                              \
    X(TRBType, DOORBELL_EVENT, 36)                                                                 \
    X(TRBType, HOST_CONTROLLER_EVENT, 37)                                                          \
    X(TRBType, DEVICE_NOTIFICATION, 38)                                                            \
    X(TRBType, MFINDEX_WRAP, 39)

    DECLARE_TYPED_ENUM(TRBType, U8, TRB_TYPE_VALUES, 0)

    // ========================================================================================== //
    // Completion Codes (xHCI 2.0 §6.4.5 Table 6-90)
    // ========================================================================================== //

#define COMPLETION_CODE_VALUES(X)                                                                  \
    X(CompletionCode, INVALID, 0)                                                                  \
    X(CompletionCode, SUCCESS, 1)                                                                  \
    X(CompletionCode, DATA_BUFFER_ERROR, 2)                                                        \
    X(CompletionCode, BABBLE_ERROR, 3)                                                             \
    X(CompletionCode, USB_TRANSACTION_ERROR, 4)                                                    \
    X(CompletionCode, TRB_ERROR, 5)                                                                \
    X(CompletionCode, STALL_ERROR, 6)                                                              \
    X(CompletionCode, RESOURCE_ERROR, 7)                                                           \
    X(CompletionCode, BANDWIDTH_ERROR, 8)                                                          \
    X(CompletionCode, NO_SLOTS_AVAILABLE, 9)                                                       \
    X(CompletionCode, SHORT_PACKET, 13)                                                            \
    X(CompletionCode, RING_UNDERRUN, 14)                                                           \
    X(CompletionCode, RING_OVERRUN, 15)                                                            \
    X(CompletionCode, VF_ENVENT_RING_FULL, 16)                                                     \
    X(CompletionCode, PARAMETER_ERROR, 17)                                                         \
    X(CompletionCode, BANDWIDTH_OVERRUN, 18)                                                       \
    X(CompletionCode, CONTEXT_STATE_ERROR, 19)                                                     \
    X(CompletionCode, NO_PING_RESPONSE, 20)                                                        \
    X(CompletionCode, EVENT_RING_FULL, 21)                                                         \
    X(CompletionCode, INCOMPATIBLE_DEVICE, 22)                                                     \
    X(CompletionCode, MISSED_SERVICE, 23)                                                          \
    X(CompletionCode, CMD_RING_STOPPED, 24)                                                        \
    X(CompletionCode, CMD_ABORTED, 25)                                                             \
    X(CompletionCode, STOPPED, 26)                                                                 \
    X(CompletionCode, STOPPED_LEN_INVAL, 27)                                                       \
    X(CompletionCode, STOPPED_SHORT_PACKET, 28)                                                    \
    X(CompletionCode, MAX_EXIT_LATENCY_TOO_LARGE, 29)                                              \
    X(CompletionCode, ISOCH_BUFFER_OVERRUN, 31)                                                    \
    X(CompletionCode, EVENT_LOST, 32)                                                              \
    X(CompletionCode, UNDEFINED, 33)                                                               \
    X(CompletionCode, INVALID_STREAM_ID, 34)                                                       \
    X(CompletionCode, SECONDARY_BANDWIDTH_ERROR, 35)                                               \
    X(CompletionCode, SPLIT_TRANSACTION, 36)

    DECLARE_TYPED_ENUM(CompletionCode, U8, COMPLETION_CODE_VALUES, 255)

    // ========================================================================================== //
    // Transfer Request Block (TRB) — xHCI 2.0 §6.4
    // All TRBs are exactly 16 bytes (four DWORDs).
    // ========================================================================================== //

    /// @brief Generic TRB providing raw 4-DWORD access.
    struct TRB {
        static constexpr U16 DEFAULT_AVERAGE_TRB_LENGTH = 8;
        static constexpr U8  PTR_ADDR_SHIFT            = 6;

        U32 m_dw0 = 0;
        U32 m_dw1 = 0;
        U32 m_dw2 = 0;
        U32 m_dw3 = 0;

        [[nodiscard]] auto cycle() const -> bool;
        [[nodiscard]] auto trb_type() const -> TRBType;

        auto set_cycle(bool cycle_bit) -> void;
        auto set_trb_type(U8 val) -> void;

      private:
        static constexpr U32 TRB_TYPE_MASK  = 0x0000FC00; // [15:10]
        static constexpr U8  TRB_TYPE_SHIFT = 10;
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
            U32                m_register = 0;
            [[nodiscard]] auto trb_transfer_length() const -> U32;
            [[nodiscard]] auto td_size() const -> U8;
            [[nodiscard]] auto interrupter_target() const -> U16;
            auto               set_trb_transfer_length(U32 val) -> void;
            auto               set_td_size(U8 val) -> void;
            auto               set_interrupter_target(U16 val) -> void;

          private:
            static constexpr U32 TRB_TRANSFER_LENGTH_MASK = 0x0001FFFF; // [16:0]
            static constexpr U32 TD_SIZE_MASK             = 0x003E0000; // [21:17]
            static constexpr U8  TD_SIZE_SHIFT            = 17;
            static constexpr U32 INTERRUPTER_TARGET_MASK  = 0xFFC00000; // [31:22]
            static constexpr U8  INTERRUPTER_TARGET_SHIFT = 22;
        } m_status;

        struct ControlDWord {
            U32                m_register = 0;
            [[nodiscard]] auto cycle() const -> bool;
            [[nodiscard]] auto ENT() const -> bool;
            [[nodiscard]] auto ISP() const -> bool;
            [[nodiscard]] auto NS() const -> bool;
            [[nodiscard]] auto chain() const -> bool;
            [[nodiscard]] auto IOC() const -> bool;
            [[nodiscard]] auto IDT() const -> bool;
            [[nodiscard]] auto BEI() const -> bool;
            [[nodiscard]] auto trb_type() const -> TRBType;
            [[nodiscard]] auto td_size_extended() const -> U8;
            auto               set_cycle(bool v) -> void;
            auto               set_ENT(bool v) -> void;
            auto               set_ISP(bool v) -> void;
            auto               set_NS(bool v) -> void;
            auto               set_chain(bool v) -> void;
            auto               set_IOC(bool v) -> void;
            auto               set_IDT(bool v) -> void;
            auto               set_BEI(bool v) -> void;
            auto               set_trb_type(U8 val) -> void;
            auto               set_td_size_extended(U8 val) -> void;

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
            static constexpr U8  TRB_TYPE_SHIFT        = 10;
            static constexpr U32 TD_SIZE_EXTENDED_MASK = 0x007F0000; // [22:16]
        } m_control;
    };
    static_assert(sizeof(NormalTRB) == sizeof(TRB));

    /// @brief Isochronous Transfer TRB (xHCI 2.0 §6.4.1.3, type = 5). Always the first TRB of an
    ///         Isoch TD; zero or more Normal TRBs may be chained after it for scatter/gather.
    struct IsochTRB {
        static constexpr U8 TYPE = TRBType::ISOCH;

        U32 m_data_buffer_pointer_lo = 0;
        U32 m_data_buffer_pointer_hi = 0;

        struct StatusDWord {
            U32                m_register = 0;
            [[nodiscard]] auto trb_transfer_length() const -> U32;
            [[nodiscard]] auto td_size() const -> U8;
            [[nodiscard]] auto interrupter_target() const -> U16;
            auto               set_trb_transfer_length(U32 val) -> void;
            auto               set_td_size(U8 val) -> void;
            auto               set_interrupter_target(U16 val) -> void;

          private:
            static constexpr U32 TRB_TRANSFER_LENGTH_MASK = 0x0001FFFF; // [16:0]
            static constexpr U32 TD_SIZE_MASK             = 0x003E0000; // [21:17]
            static constexpr U8  TD_SIZE_SHIFT            = 17;
            static constexpr U32 INTERRUPTER_TARGET_MASK  = 0xFFC00000; // [31:22]
            static constexpr U8  INTERRUPTER_TARGET_SHIFT = 22;
        } m_status;

        struct ControlDWord {
            U32                m_register = 0;
            [[nodiscard]] auto cycle() const -> bool;
            [[nodiscard]] auto ENT() const -> bool;
            [[nodiscard]] auto ISP() const -> bool;
            [[nodiscard]] auto NS() const -> bool;
            [[nodiscard]] auto chain() const -> bool;
            [[nodiscard]] auto IOC() const -> bool;
            [[nodiscard]] auto IDT() const -> bool;
            [[nodiscard]] auto TBC() const -> U8;
            [[nodiscard]] auto BEI() const -> bool;
            [[nodiscard]] auto trb_type() const -> TRBType;
            [[nodiscard]] auto TLBPC() const -> U8;
            [[nodiscard]] auto frame_ID() const -> U16;
            [[nodiscard]] auto SIA() const -> bool;
            auto               set_cycle(bool v) -> void;
            auto               set_ENT(bool v) -> void;
            auto               set_ISP(bool v) -> void;
            auto               set_NS(bool v) -> void;
            auto               set_chain(bool v) -> void;
            auto               set_IOC(bool v) -> void;
            auto               set_IDT(bool v) -> void;
            auto               set_TBC(U8 val) -> void;
            auto               set_BEI(bool v) -> void;
            auto               set_trb_type(U8 val) -> void;
            auto               set_TLBPC(U8 val) -> void;
            auto               set_frame_ID(U16 val) -> void;
            auto               set_SIA(bool v) -> void;

          private:
            static constexpr U8  CYCLE_BIT_OFFSET = 0;
            static constexpr U8  ENT_BIT_OFFSET   = 1;
            static constexpr U8  ISP_BIT_OFFSET   = 2;
            static constexpr U8  NS_BIT_OFFSET    = 3;
            static constexpr U8  CHAIN_BIT_OFFSET = 4;
            static constexpr U8  IOC_BIT_OFFSET   = 5;
            static constexpr U8  IDT_BIT_OFFSET   = 6;
            static constexpr U8  BEI_BIT_OFFSET   = 9;
            static constexpr U8  SIA_BIT_OFFSET   = 31;
            static constexpr U32 TBC_MASK         = 0x00000180; // [8:7]
            static constexpr U8  TBC_SHIFT        = 7;
            static constexpr U32 TRB_TYPE_MASK    = 0x0000FC00; // [15:10]
            static constexpr U8  TRB_TYPE_SHIFT   = 10;
            static constexpr U32 TLBPC_MASK       = 0x000F0000; // [19:16]
            static constexpr U8  TLBPC_SHIFT      = 16;
            static constexpr U32 FRAME_ID_MASK    = 0x7FF00000; // [30:20]
            static constexpr U8  FRAME_ID_SHIFT   = 20;
        } m_control;
    };
    static_assert(sizeof(IsochTRB) == sizeof(TRB));

    /// @brief Setup Stage TRB for control transfers (xHCI 2.0 §6.4.1.2.1, type = 2).
    struct SetupStageTRB {
        static constexpr U8 TYPE         = TRBType::SETUP_STAGE;
        static constexpr U8 TRT_NO_DATA  = 0;
        static constexpr U8 TRT_OUT_DATA = 2;
        static constexpr U8 TRT_IN_DATA  = 3;

        struct RequestDWord {
            U32                m_register = 0;
            [[nodiscard]] auto bm_request_type() const -> U8;
            [[nodiscard]] auto b_request() const -> U8;
            [[nodiscard]] auto w_value() const -> U16;
            auto               set_bm_request_type(U8 val) -> void;
            auto               set_b_request(U8 val) -> void;
            auto               set_w_value(U16 val) -> void;

          private:
            static constexpr U32 BM_REQUEST_TYPE_MASK = 0x000000FF; // [7:0]
            static constexpr U32 B_REQUEST_MASK       = 0x0000FF00; // [15:8]
            static constexpr U32 W_VALUE_MASK         = 0xFFFF0000; // [31:16]
        } m_request;

        struct IndexLengthDWord {
            U32                m_register = 0;
            [[nodiscard]] auto w_index() const -> U16;
            [[nodiscard]] auto w_length() const -> U16;
            auto               set_w_index(U16 val) -> void;
            auto               set_w_length(U16 val) -> void;

          private:
            static constexpr U32 W_INDEX_MASK  = 0x0000FFFF; // [15:0]
            static constexpr U32 W_LENGTH_MASK = 0xFFFF0000; // [31:16]
        } m_index_length;

        struct StatusDWord {
            U32                m_register = 0;
            [[nodiscard]] auto trb_transfer_length() const -> U32;
            [[nodiscard]] auto interrupter_target() const -> U16;
            auto               set_trb_transfer_length(U32 val) -> void;
            auto               set_interrupter_target(U16 val) -> void;

          private:
            static constexpr U32 TRB_TRANSFER_LENGTH_MASK = 0x0001FFFF; // [16:0]
            static constexpr U32 INTERRUPTER_TARGET_MASK  = 0xFFC00000; // [31:22]
            static constexpr U8  INTERRUPTER_TARGET_SHIFT = 22;
        } m_status;

        struct ControlDWord {
            U32                m_register = 0;
            [[nodiscard]] auto cycle() const -> bool;
            [[nodiscard]] auto IOC() const -> bool;
            [[nodiscard]] auto IDT() const -> bool;
            [[nodiscard]] auto trb_type() const -> TRBType;
            [[nodiscard]] auto TRT() const -> U8;
            auto               set_cycle(bool v) -> void;
            auto               set_IOC(bool v) -> void;
            auto               set_IDT(bool v) -> void;
            auto               set_trb_type(U8 val) -> void;
            auto               set_TRT(U8 val) -> void;

          private:
            static constexpr U8  CYCLE_BIT_OFFSET = 0;
            static constexpr U8  IOC_BIT_OFFSET   = 5;
            static constexpr U8  IDT_BIT_OFFSET   = 6;
            static constexpr U32 TRB_TYPE_MASK    = 0x0000FC00; // [15:10]
            static constexpr U8  TRB_TYPE_SHIFT   = 10;
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
            U32                m_register = 0;
            [[nodiscard]] auto trb_transfer_length() const -> U32;
            [[nodiscard]] auto td_size() const -> U8;
            [[nodiscard]] auto interrupter_target() const -> U16;
            auto               set_trb_transfer_length(U32 val) -> void;
            auto               set_td_size(U8 val) -> void;
            auto               set_interrupter_target(U16 val) -> void;

          private:
            static constexpr U32 TRB_TRANSFER_LENGTH_MASK = 0x0001FFFF; // [16:0]
            static constexpr U32 TD_SIZE_MASK             = 0x003E0000; // [21:17]
            static constexpr U8  TD_SIZE_SHIFT            = 17;
            static constexpr U32 INTERRUPTER_TARGET_MASK  = 0xFFC00000; // [31:22]
            static constexpr U8  INTERRUPTER_TARGET_SHIFT = 22;
        } m_status;

        struct ControlDWord {
            U32                m_register = 0;
            [[nodiscard]] auto cycle() const -> bool;
            [[nodiscard]] auto ENT() const -> bool;
            [[nodiscard]] auto ISP() const -> bool;
            [[nodiscard]] auto NS() const -> bool;
            [[nodiscard]] auto chain() const -> bool;
            [[nodiscard]] auto IOC() const -> bool;
            [[nodiscard]] auto IDT() const -> bool;
            [[nodiscard]] auto trb_type() const -> TRBType;
            [[nodiscard]] auto DIR() const -> bool;
            auto               set_cycle(bool v) -> void;
            auto               set_ENT(bool v) -> void;
            auto               set_ISP(bool v) -> void;
            auto               set_NS(bool v) -> void;
            auto               set_chain(bool v) -> void;
            auto               set_IOC(bool v) -> void;
            auto               set_IDT(bool v) -> void;
            auto               set_trb_type(U8 val) -> void;
            auto               set_DIR(bool v) -> void;

          private:
            static constexpr U8  CYCLE_BIT_OFFSET = 0;
            static constexpr U8  ENT_BIT_OFFSET   = 1;
            static constexpr U8  ISP_BIT_OFFSET   = 2;
            static constexpr U8  NS_BIT_OFFSET    = 3;
            static constexpr U8  CHAIN_BIT_OFFSET = 4;
            static constexpr U8  IOC_BIT_OFFSET   = 5;
            static constexpr U8  IDT_BIT_OFFSET   = 6;
            static constexpr U32 TRB_TYPE_MASK    = 0x0000FC00; // [15:10]
            static constexpr U8  TRB_TYPE_SHIFT   = 10;
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
            U32                m_register = 0;
            [[nodiscard]] auto interrupter_target() const -> U16;
            auto               set_interrupter_target(U16 val) -> void;

          private:
            static constexpr U32 INTERRUPTER_TARGET_MASK  = 0xFFC00000; // [31:22]
            static constexpr U8  INTERRUPTER_TARGET_SHIFT = 22;
        } m_status;

        struct ControlDWord {
            U32                m_register = 0;
            [[nodiscard]] auto cycle() const -> bool;
            [[nodiscard]] auto ENT() const -> bool; // Evaluate Next TRB
            [[nodiscard]] auto CH() const -> bool;  // Chain bit
            [[nodiscard]] auto IOC() const -> bool;
            [[nodiscard]] auto trb_type() const -> TRBType;
            [[nodiscard]] auto DIR() const -> bool;
            auto               set_cycle(bool v) -> void;
            auto               set_ENT(bool v) -> void;
            auto               set_CH(bool v) -> void;
            auto               set_IOC(bool v) -> void;
            auto               set_trb_type(U8 val) -> void;
            auto               set_DIR(bool v) -> void;

          private:
            static constexpr U8  CYCLE_BIT_OFFSET = 0;
            static constexpr U8  ENT_BIT_OFFSET   = 1;
            static constexpr U8  CH_BIT_OFFSET    = 4;
            static constexpr U8  IOC_BIT_OFFSET   = 5;
            static constexpr U32 TRB_TYPE_MASK    = 0x0000FC00; // [15:10]
            static constexpr U8  TRB_TYPE_SHIFT   = 10;
            static constexpr U8  DIR_BIT_OFFSET   = 16;
        } m_control;
    };
    static_assert(sizeof(StatusStageTRB) == sizeof(TRB));

    /// @brief No Op Transfer TRB (xHCI 2.0 §6.4.1.4, type = 8).
    ///
    /// Placed on an endpoint Transfer Ring to verify the basic Transfer Ring mechanism: it
    /// transfers no data and generates a Transfer Event (Completion Code = Success) when IOC is
    /// set. Only the Interrupter Target, Cycle, ENT, Chain, IOC and TRB Type fields are defined;
    /// all other bits are RsvdZ and left at their zero default.
    struct NoOpTransferTRB {
        static constexpr U8 TYPE = TRBType::NO_OP;

        U32 m_reserved_0 = 0;
        U32 m_reserved_1 = 0;

        struct StatusDWord {
            U32                m_register = 0;
            [[nodiscard]] auto interrupter_target() const -> U16;
            auto               set_interrupter_target(U16 val) -> void;

          private:
            static constexpr U32 INTERRUPTER_TARGET_MASK  = 0xFFC00000; // [31:22]
            static constexpr U8  INTERRUPTER_TARGET_SHIFT = 22;
        } m_status;

        struct ControlDWord {
            U32                m_register = 0;
            [[nodiscard]] auto cycle() const -> bool;
            [[nodiscard]] auto ENT() const -> bool;   // Evaluate Next TRB
            [[nodiscard]] auto chain() const -> bool; // Chain bit
            [[nodiscard]] auto IOC() const -> bool;
            [[nodiscard]] auto trb_type() const -> TRBType;
            auto               set_cycle(bool v) -> void;
            auto               set_ENT(bool v) -> void;
            auto               set_chain(bool v) -> void;
            auto               set_IOC(bool v) -> void;
            auto               set_trb_type(U8 val) -> void;

          private:
            static constexpr U8  CYCLE_BIT_OFFSET = 0;
            static constexpr U8  ENT_BIT_OFFSET   = 1;
            static constexpr U8  CHAIN_BIT_OFFSET = 4;
            static constexpr U8  IOC_BIT_OFFSET   = 5;
            static constexpr U32 TRB_TYPE_MASK    = 0x0000FC00; // [15:10]
            static constexpr U8  TRB_TYPE_SHIFT   = 10;
        } m_control;
    };
    static_assert(sizeof(NoOpTransferTRB) == sizeof(TRB));

    /// @brief Link TRB — links ring segments (xHCI 2.0 §6.4.4.1, type = 6).
    struct LinkTRB {
        static constexpr U8 TYPE = TRBType::LINK;

        struct RingSegmentPtrLoDWord {
            U32                m_register = 0;
            [[nodiscard]] auto ptr() const -> U32;       // bits [31:4], val = phys >> 4
            auto               set_ptr(U32 val) -> void; // val = phys >> 4
          private:
            static constexpr U32 PTR_MASK  = 0xFFFFFFF0; // [31:4]
            static constexpr U8  PTR_SHIFT = 4;
        } m_ring_segment_pointer_lo;

        U32 m_ring_segment_pointer_hi = 0;

        struct StatusDWord {
            U32                m_register = 0;
            [[nodiscard]] auto interrupter_target() const -> U16;
            auto               set_interrupter_target(U16 val) -> void;

          private:
            static constexpr U32 INTERRUPTER_TARGET_MASK  = 0xFFC00000; // [31:22]
            static constexpr U8  INTERRUPTER_TARGET_SHIFT = 22;
        } m_status;

        struct ControlDWord {
            U32                m_register = 0;
            [[nodiscard]] auto cycle() const -> bool;
            [[nodiscard]] auto toggle_cycle() const -> bool;
            [[nodiscard]] auto chain() const -> bool;
            [[nodiscard]] auto IOC() const -> bool;
            [[nodiscard]] auto trb_type() const -> TRBType;
            auto               set_cycle(bool v) -> void;
            auto               set_toggle_cycle(bool v) -> void;
            auto               set_chain(bool v) -> void;
            auto               set_IOC(bool v) -> void;
            auto               set_trb_type(U8 val) -> void;

          private:
            static constexpr U8  CYCLE_BIT_OFFSET        = 0;
            static constexpr U8  TOGGLE_CYCLE_BIT_OFFSET = 1;
            static constexpr U8  CHAIN_BIT_OFFSET        = 4;
            static constexpr U8  IOC_BIT_OFFSET          = 5;
            static constexpr U32 TRB_TYPE_MASK           = 0x0000FC00; // [15:10]
            static constexpr U8  TRB_TYPE_SHIFT          = 10;
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
            U32                m_register = 0;
            [[nodiscard]] auto cycle() const -> bool;
            [[nodiscard]] auto trb_type() const -> TRBType;
            [[nodiscard]] auto slot_type() const -> U8;
            auto               set_cycle(bool v) -> void;
            auto               set_trb_type(U8 val) -> void;
            auto               set_slot_type(U8 val) -> void;

          private:
            static constexpr U8  CYCLE_BIT_OFFSET = 0;
            static constexpr U32 TRB_TYPE_MASK    = 0x0000FC00; // [15:10]
            static constexpr U8  TRB_TYPE_SHIFT   = 10;
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
            U32                m_register = 0;
            [[nodiscard]] auto cycle() const -> bool;
            [[nodiscard]] auto trb_type() const -> TRBType;
            [[nodiscard]] auto slot_id() const -> U8;
            auto               set_cycle(bool v) -> void;
            auto               set_trb_type(U8 val) -> void;
            auto               set_slot_id(U8 val) -> void;

          private:
            static constexpr U8  CYCLE_BIT_OFFSET = 0;
            static constexpr U32 TRB_TYPE_MASK    = 0x0000FC00; // [15:10]
            static constexpr U8  TRB_TYPE_SHIFT   = 10;
            static constexpr U32 SLOT_ID_MASK     = 0xFF000000; // [31:24]
        } m_control;
    };
    static_assert(sizeof(DisableSlotCommandTRB) == sizeof(TRB));

    /// @brief Address Device Command TRB (xHCI 2.0 §6.4.3.4, type = 11).
    struct AddressDeviceCommandTRB {
        static constexpr U8 TYPE = TRBType::ADDRESS_DEVICE;

        struct InputContextPtrLoDWord {
            U32                m_register = 0;
            [[nodiscard]] auto ptr() const -> U32; // bits [31:4], val = phys >> 4
            auto               set_ptr(U32 val) -> void;

          private:
            static constexpr U32 PTR_MASK  = 0xFFFFFFF0; // [31:4]
            static constexpr U8  PTR_SHIFT = 4;
        } m_input_context_ptr_lo;

        U32 m_input_context_ptr_hi = 0;
        U32 m_reserved_0           = 0;

        struct ControlDWord {
            U32                m_register = 0;
            [[nodiscard]] auto cycle() const -> bool;
            [[nodiscard]] auto BSR() const -> bool;
            [[nodiscard]] auto trb_type() const -> TRBType;
            [[nodiscard]] auto slot_id() const -> U8;
            auto               set_cycle(bool v) -> void;
            auto               set_BSR(bool v) -> void;
            auto               set_trb_type(U8 val) -> void;
            auto               set_slot_id(U8 val) -> void;

          private:
            static constexpr U8  CYCLE_BIT_OFFSET = 0;
            static constexpr U8  BSR_BIT_OFFSET   = 9;
            static constexpr U32 TRB_TYPE_MASK    = 0x0000FC00; // [15:10]
            static constexpr U8  TRB_TYPE_SHIFT   = 10;
            static constexpr U32 SLOT_ID_MASK     = 0xFF000000; // [31:24]
        } m_control;
    };
    static_assert(sizeof(AddressDeviceCommandTRB) == sizeof(TRB));

    /// @brief Configure Endpoint Command TRB (xHCI 2.0 §6.4.3.5, type = 12).
    struct ConfigureEndpointCommandTRB {
        static constexpr U8 TYPE = TRBType::CONFIGURE_ENDPOINT;

        struct InputContextPtrLoDWord {
            U32                m_register = 0;
            [[nodiscard]] auto ptr() const -> U32; // bits [31:4], val = phys >> 4
            auto               set_ptr(U32 val) -> void;

          private:
            static constexpr U32 PTR_MASK  = 0xFFFFFFF0; // [31:4]
            static constexpr U8  PTR_SHIFT = 4;
        } m_input_context_ptr_lo;

        U32 m_input_context_ptr_hi = 0;
        U32 m_reserved_0           = 0;

        struct ControlDWord {
            U32                m_register = 0;
            [[nodiscard]] auto cycle() const -> bool;
            [[nodiscard]] auto DC() const -> bool;
            [[nodiscard]] auto trb_type() const -> TRBType;
            [[nodiscard]] auto slot_id() const -> U8;
            auto               set_cycle(bool v) -> void;
            auto               set_DC(bool v) -> void;
            auto               set_trb_type(U8 val) -> void;
            auto               set_slot_id(U8 val) -> void;

          private:
            static constexpr U8  CYCLE_BIT_OFFSET = 0;
            static constexpr U8  DC_BIT_OFFSET    = 9;
            static constexpr U32 TRB_TYPE_MASK    = 0x0000FC00; // [15:10]
            static constexpr U8  TRB_TYPE_SHIFT   = 10;
            static constexpr U32 SLOT_ID_MASK     = 0xFF000000; // [31:24]
        } m_control;
    };
    static_assert(sizeof(ConfigureEndpointCommandTRB) == sizeof(TRB));

    /// @brief Evaluate Context Command TRB (xHCI 2.0 §6.4.3.6, type = 13).
    struct EvaluateContextCommandTRB {
        static constexpr U8 TYPE = TRBType::EVALUATE_CONTEXT;

        struct InputContextPtrLoDWord {
            U32                m_register = 0;
            [[nodiscard]] auto ptr() const -> U32;
            auto               set_ptr(U32 val) -> void;

          private:
            static constexpr U32 PTR_MASK  = 0xFFFFFFF0; // [31:4]
            static constexpr U8  PTR_SHIFT = 4;
        } m_input_context_ptr_lo;

        U32 m_input_context_ptr_hi = 0;
        U32 m_reserved_0           = 0;

        struct ControlDWord {
            U32                m_register = 0;
            [[nodiscard]] auto cycle() const -> bool;
            [[nodiscard]] auto trb_type() const -> TRBType;
            [[nodiscard]] auto slot_id() const -> U8;
            auto               set_cycle(bool v) -> void;
            auto               set_trb_type(U8 val) -> void;
            auto               set_slot_id(U8 val) -> void;

          private:
            static constexpr U8  CYCLE_BIT_OFFSET = 0;
            static constexpr U32 TRB_TYPE_MASK    = 0x0000FC00; // [15:10]
            static constexpr U8  TRB_TYPE_SHIFT   = 10;
            static constexpr U32 SLOT_ID_MASK     = 0xFF000000; // [31:24]
        } m_control;
    };
    static_assert(sizeof(EvaluateContextCommandTRB) == sizeof(TRB));

    /// @brief No-Op Command TRB (xHCI 2.0 §6.4.3.1, type = 23).
    struct NoOpCommandTRB {
        static constexpr U8 TYPE = TRBType::NO_OP_COMMAND;

        U32 m_reserved_0 = 0;
        U32 m_reserved_1 = 0;
        U32 m_reserved_2 = 0;

        struct ControlDWord {
            U32                m_register = 0;
            [[nodiscard]] auto cycle() const -> bool;
            [[nodiscard]] auto trb_type() const -> TRBType;
            auto               set_cycle(bool v) -> void;
            auto               set_trb_type(U8 val) -> void;

          private:
            static constexpr U8  CYCLE_BIT_OFFSET = 0;
            static constexpr U32 TRB_TYPE_MASK    = 0x0000FC00; // [15:10]
            static constexpr U8  TRB_TYPE_SHIFT   = 10;
        } m_control;
    };
    static_assert(sizeof(NoOpCommandTRB) == sizeof(TRB));

    // ========================================================================================== //
    // Event TRBs — §6.4.2
    // ========================================================================================== //

    /// @brief Generic Event TRB providing access to the fields common to every Event TRB
    /// (xHCI 2.0 §6.4.2): the Completion Code, the Cycle bit and the TRB Type.
    struct EventTRB {
        U32 m_dw0 = 0;
        U32 m_dw1 = 0;

        struct StatusDWord {
            U32                m_register = 0;
            [[nodiscard]] auto completion_code() const -> CompletionCode;
            auto               set_completion_code(U8 val) -> void;

          private:
            static constexpr U32 COMPLETION_CODE_MASK = 0xFF000000; // [31:24]
        } m_status;

        struct ControlDWord {
            U32                m_register = 0;
            [[nodiscard]] auto cycle() const -> bool;
            [[nodiscard]] auto trb_type() const -> TRBType;
            auto               set_cycle(bool v) -> void;
            auto               set_trb_type(U8 val) -> void;

          private:
            static constexpr U8  CYCLE_BIT_OFFSET = 0;
            static constexpr U32 TRB_TYPE_MASK    = 0x0000FC00; // [15:10]
            static constexpr U8  TRB_TYPE_SHIFT   = 10;
        } m_control;
    };
    static_assert(sizeof(EventTRB) == sizeof(TRB));

    /// @brief Transfer Event TRB (xHCI 2.0 §6.4.2.1, type = 32).
    struct TransferEventTRB {
        static constexpr U8 TYPE = TRBType::TRANSFER_EVENT;

        U32 m_trb_pointer_lo = 0;
        U32 m_trb_pointer_hi = 0;

        struct StatusDWord {
            U32                m_register = 0;
            [[nodiscard]] auto trb_transfer_length() const -> U32;
            [[nodiscard]] auto completion_code() const -> CompletionCode;
            auto               set_trb_transfer_length(U32 val) -> void;
            auto               set_completion_code(U8 val) -> void;

          private:
            static constexpr U32 TRB_TRANSFER_LENGTH_MASK = 0x00FFFFFF; // [23:0]
            static constexpr U32 COMPLETION_CODE_MASK     = 0xFF000000; // [31:24]
        } m_status;

        struct ControlDWord {
            U32                m_register = 0;
            [[nodiscard]] auto cycle() const -> bool;
            [[nodiscard]] auto ED() const -> bool;
            [[nodiscard]] auto trb_type() const -> TRBType;
            [[nodiscard]] auto endpoint_id() const -> U8;
            [[nodiscard]] auto slot_id() const -> U8;
            auto               set_cycle(bool v) -> void;
            auto               set_ED(bool v) -> void;
            auto               set_trb_type(U8 val) -> void;
            auto               set_endpoint_id(U8 val) -> void;
            auto               set_slot_id(U8 val) -> void;

          private:
            static constexpr U8  CYCLE_BIT_OFFSET = 0;
            static constexpr U8  ED_BIT_OFFSET    = 2;
            static constexpr U32 TRB_TYPE_MASK    = 0x0000FC00; // [15:10]
            static constexpr U8  TRB_TYPE_SHIFT   = 10;
            static constexpr U32 ENDPOINT_ID_MASK = 0x001F0000; // [20:16]
            static constexpr U32 SLOT_ID_MASK     = 0xFF000000; // [31:24]
        } m_control;
    };
    static_assert(sizeof(TransferEventTRB) == sizeof(TRB));

    /// @brief Command Completion Event TRB (xHCI 2.0 §6.4.2.2, type = 33).
    struct CommandCompletionEventTRB {
        static constexpr U8 TYPE = TRBType::CMD_COMPLETION;

        struct CommandTrbPtrLoDWord {
            U32                m_register = 0;
            [[nodiscard]] auto ptr() const -> U32; // bits [31:4], val = phys >> 4
            auto               set_ptr(U32 val) -> void;

          private:
            static constexpr U32 PTR_MASK  = 0xFFFFFFF0; // [31:4]
            static constexpr U8  PTR_SHIFT = 4;
        } m_command_trb_pointer_lo;

        U32 m_command_trb_pointer_hi = 0;

        struct StatusDWord {
            U32                m_register = 0;
            [[nodiscard]] auto completion_parameter() const -> U32;
            [[nodiscard]] auto completion_code() const -> CompletionCode;
            auto               set_completion_parameter(U32 val) -> void;
            auto               set_completion_code(U8 val) -> void;

          private:
            static constexpr U32 COMPLETION_PARAMETER_MASK = 0x00FFFFFF; // [23:0]
            static constexpr U32 COMPLETION_CODE_MASK      = 0xFF000000; // [31:24]
        } m_status;

        struct ControlDWord {
            U32                m_register = 0;
            [[nodiscard]] auto cycle() const -> bool;
            [[nodiscard]] auto trb_type() const -> TRBType;
            [[nodiscard]] auto vf_id() const -> U8;
            [[nodiscard]] auto slot_id() const -> U8;
            auto               set_cycle(bool v) -> void;
            auto               set_trb_type(U8 val) -> void;
            auto               set_vf_id(U8 val) -> void;
            auto               set_slot_id(U8 val) -> void;

          private:
            static constexpr U8  CYCLE_BIT_OFFSET = 0;
            static constexpr U32 TRB_TYPE_MASK    = 0x0000FC00; // [15:10]
            static constexpr U8  TRB_TYPE_SHIFT   = 10;
            static constexpr U32 VF_ID_MASK       = 0x00FF0000; // [23:16]
            static constexpr U32 SLOT_ID_MASK     = 0xFF000000; // [31:24]
        } m_control;
    };
    static_assert(sizeof(CommandCompletionEventTRB) == sizeof(TRB));

    /// @brief Port Status Change Event TRB (xHCI 2.0 §6.4.2.3, type = 34).
    struct PortStatusChangeEventTRB {
        static constexpr U8 TYPE = TRBType::PORT_STATUS_CHANGE;

        struct PortIdDWord {
            U32                m_register = 0;
            [[nodiscard]] auto port_id() const -> U8; // bits [31:24]
            auto               set_port_id(U8 val) -> void;

          private:
            static constexpr U32 PORT_ID_MASK = 0xFF000000; // [31:24]
        } m_port_id;

        U32 m_reserved_0 = 0;

        struct StatusDWord {
            U32                m_register = 0;
            [[nodiscard]] auto completion_code() const -> CompletionCode; // bits [31:24]
            auto               set_completion_code(U8 val) -> void;

          private:
            static constexpr U32 COMPLETION_CODE_MASK = 0xFF000000; // [31:24]
        } m_status;

        struct ControlDWord {
            U32                m_register = 0;
            [[nodiscard]] auto cycle() const -> bool;
            [[nodiscard]] auto trb_type() const -> TRBType;
            auto               set_cycle(bool v) -> void;
            auto               set_trb_type(U8 val) -> void;

          private:
            static constexpr U8  CYCLE_BIT_OFFSET = 0;
            static constexpr U32 TRB_TYPE_MASK    = 0x0000FC00; // [15:10]
            static constexpr U8  TRB_TYPE_SHIFT   = 10;
        } m_control;
    };
    static_assert(sizeof(PortStatusChangeEventTRB) == sizeof(TRB));

} // namespace Rune::Device::USB

#endif // RUNEOS_TRB_H