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

#include <Device/USB/XHCI/TRB.h>

#include <KRE/BitsAndBytes.h>

namespace Rune::Device::USB {

    // ========================================================================================== //
    // NormalTRB — xHCI 2.0 §6.4.1.1
    // ========================================================================================== //

    [[nodiscard]] auto NormalTRB::StatusDWord::trb_transfer_length() const -> U32 {
        return m_register & TRB_TRANSFER_LENGTH_MASK;
    }
    [[nodiscard]] auto NormalTRB::StatusDWord::td_size() const -> U8 {
        return static_cast<U8>((m_register & TD_SIZE_MASK) >> 17);
    }
    [[nodiscard]] auto NormalTRB::StatusDWord::interrupter_target() const -> U16 {
        return static_cast<U16>((m_register & INTERRUPTER_TARGET_MASK) >> 22);
    }
    auto NormalTRB::StatusDWord::set_trb_transfer_length(U32 val) -> void {
        m_register = (m_register & ~TRB_TRANSFER_LENGTH_MASK) | (val & TRB_TRANSFER_LENGTH_MASK);
    }
    auto NormalTRB::StatusDWord::set_td_size(U8 val) -> void {
        m_register = (m_register & ~TD_SIZE_MASK) | ((static_cast<U32>(val) << 17) & TD_SIZE_MASK);
    }
    auto NormalTRB::StatusDWord::set_interrupter_target(U16 val) -> void {
        m_register = (m_register & ~INTERRUPTER_TARGET_MASK)
                     | ((static_cast<U32>(val) << 22) & INTERRUPTER_TARGET_MASK);
    }

    [[nodiscard]] auto NormalTRB::ControlDWord::cycle() const -> bool {
        return bit_check(m_register, CYCLE_BIT_OFFSET);
    }
    [[nodiscard]] auto NormalTRB::ControlDWord::ENT() const -> bool {
        return bit_check(m_register, ENT_BIT_OFFSET);
    }
    [[nodiscard]] auto NormalTRB::ControlDWord::ISP() const -> bool {
        return bit_check(m_register, ISP_BIT_OFFSET);
    }
    [[nodiscard]] auto NormalTRB::ControlDWord::NS() const -> bool {
        return bit_check(m_register, NS_BIT_OFFSET);
    }
    [[nodiscard]] auto NormalTRB::ControlDWord::chain() const -> bool {
        return bit_check(m_register, CHAIN_BIT_OFFSET);
    }
    [[nodiscard]] auto NormalTRB::ControlDWord::IOC() const -> bool {
        return bit_check(m_register, IOC_BIT_OFFSET);
    }
    [[nodiscard]] auto NormalTRB::ControlDWord::IDT() const -> bool {
        return bit_check(m_register, IDT_BIT_OFFSET);
    }
    [[nodiscard]] auto NormalTRB::ControlDWord::BEI() const -> bool {
        return bit_check(m_register, BEI_BIT_OFFSET);
    }
    [[nodiscard]] auto NormalTRB::ControlDWord::trb_type() const -> U8 {
        return static_cast<U8>((m_register & TRB_TYPE_MASK) >> 10);
    }
    [[nodiscard]] auto NormalTRB::ControlDWord::td_size_extended() const -> U8 {
        return static_cast<U8>((m_register & TD_SIZE_EXTENDED_MASK) >> SHIFT_16);
    }
    auto NormalTRB::ControlDWord::set_cycle(bool v) -> void {
        m_register =
            v ? bit_set(m_register, CYCLE_BIT_OFFSET) : bit_clear(m_register, CYCLE_BIT_OFFSET);
    }
    auto NormalTRB::ControlDWord::set_ENT(bool v) -> void {
        m_register =
            v ? bit_set(m_register, ENT_BIT_OFFSET) : bit_clear(m_register, ENT_BIT_OFFSET);
    }
    auto NormalTRB::ControlDWord::set_ISP(bool v) -> void {
        m_register =
            v ? bit_set(m_register, ISP_BIT_OFFSET) : bit_clear(m_register, ISP_BIT_OFFSET);
    }
    auto NormalTRB::ControlDWord::set_NS(bool v) -> void {
        m_register = v ? bit_set(m_register, NS_BIT_OFFSET) : bit_clear(m_register, NS_BIT_OFFSET);
    }
    auto NormalTRB::ControlDWord::set_chain(bool v) -> void {
        m_register =
            v ? bit_set(m_register, CHAIN_BIT_OFFSET) : bit_clear(m_register, CHAIN_BIT_OFFSET);
    }
    auto NormalTRB::ControlDWord::set_IOC(bool v) -> void {
        m_register =
            v ? bit_set(m_register, IOC_BIT_OFFSET) : bit_clear(m_register, IOC_BIT_OFFSET);
    }
    auto NormalTRB::ControlDWord::set_IDT(bool v) -> void {
        m_register =
            v ? bit_set(m_register, IDT_BIT_OFFSET) : bit_clear(m_register, IDT_BIT_OFFSET);
    }
    auto NormalTRB::ControlDWord::set_BEI(bool v) -> void {
        m_register =
            v ? bit_set(m_register, BEI_BIT_OFFSET) : bit_clear(m_register, BEI_BIT_OFFSET);
    }
    auto NormalTRB::ControlDWord::set_trb_type(U8 val) -> void {
        m_register = (m_register & ~TRB_TYPE_MASK)
                     | ((static_cast<U32>(val) << 10) & TRB_TYPE_MASK);
    }
    auto NormalTRB::ControlDWord::set_td_size_extended(U8 val) -> void {
        m_register = (m_register & ~TD_SIZE_EXTENDED_MASK)
                     | ((static_cast<U32>(val) << SHIFT_16) & TD_SIZE_EXTENDED_MASK);
    }

    // ========================================================================================== //
    // SetupStageTRB — xHCI 2.0 §6.4.1.2.1
    // ========================================================================================== //

    [[nodiscard]] auto SetupStageTRB::RequestDWord::bm_request_type() const -> U8 {
        return static_cast<U8>(m_register & BM_REQUEST_TYPE_MASK);
    }
    [[nodiscard]] auto SetupStageTRB::RequestDWord::b_request() const -> U8 {
        return static_cast<U8>((m_register & B_REQUEST_MASK) >> SHIFT_8);
    }
    [[nodiscard]] auto SetupStageTRB::RequestDWord::w_value() const -> U16 {
        return static_cast<U16>((m_register & W_VALUE_MASK) >> SHIFT_16);
    }
    auto SetupStageTRB::RequestDWord::set_bm_request_type(U8 val) -> void {
        m_register = (m_register & ~BM_REQUEST_TYPE_MASK) | static_cast<U32>(val);
    }
    auto SetupStageTRB::RequestDWord::set_b_request(U8 val) -> void {
        m_register = (m_register & ~B_REQUEST_MASK)
                     | ((static_cast<U32>(val) << SHIFT_8) & B_REQUEST_MASK);
    }
    auto SetupStageTRB::RequestDWord::set_w_value(U16 val) -> void {
        m_register = (m_register & ~W_VALUE_MASK)
                     | ((static_cast<U32>(val) << SHIFT_16) & W_VALUE_MASK);
    }

    [[nodiscard]] auto SetupStageTRB::IndexLengthDWord::w_index() const -> U16 {
        return static_cast<U16>(m_register & W_INDEX_MASK);
    }
    [[nodiscard]] auto SetupStageTRB::IndexLengthDWord::w_length() const -> U16 {
        return static_cast<U16>((m_register & W_LENGTH_MASK) >> SHIFT_16);
    }
    auto SetupStageTRB::IndexLengthDWord::set_w_index(U16 val) -> void {
        m_register = (m_register & ~W_INDEX_MASK) | static_cast<U32>(val);
    }
    auto SetupStageTRB::IndexLengthDWord::set_w_length(U16 val) -> void {
        m_register = (m_register & ~W_LENGTH_MASK)
                     | ((static_cast<U32>(val) << SHIFT_16) & W_LENGTH_MASK);
    }

    [[nodiscard]] auto SetupStageTRB::StatusDWord::trb_transfer_length() const -> U32 {
        return m_register & TRB_TRANSFER_LENGTH_MASK;
    }
    [[nodiscard]] auto SetupStageTRB::StatusDWord::interrupter_target() const -> U16 {
        return static_cast<U16>((m_register & INTERRUPTER_TARGET_MASK) >> 22);
    }
    auto SetupStageTRB::StatusDWord::set_trb_transfer_length(U32 val) -> void {
        m_register = (m_register & ~TRB_TRANSFER_LENGTH_MASK) | (val & TRB_TRANSFER_LENGTH_MASK);
    }
    auto SetupStageTRB::StatusDWord::set_interrupter_target(U16 val) -> void {
        m_register = (m_register & ~INTERRUPTER_TARGET_MASK)
                     | ((static_cast<U32>(val) << 22) & INTERRUPTER_TARGET_MASK);
    }

    [[nodiscard]] auto SetupStageTRB::ControlDWord::cycle() const -> bool {
        return bit_check(m_register, CYCLE_BIT_OFFSET);
    }
    [[nodiscard]] auto SetupStageTRB::ControlDWord::IOC() const -> bool {
        return bit_check(m_register, IOC_BIT_OFFSET);
    }
    [[nodiscard]] auto SetupStageTRB::ControlDWord::IDT() const -> bool {
        return bit_check(m_register, IDT_BIT_OFFSET);
    }
    [[nodiscard]] auto SetupStageTRB::ControlDWord::trb_type() const -> U8 {
        return static_cast<U8>((m_register & TRB_TYPE_MASK) >> 10);
    }
    [[nodiscard]] auto SetupStageTRB::ControlDWord::TRT() const -> U8 {
        return static_cast<U8>((m_register & TRT_MASK) >> SHIFT_16);
    }
    auto SetupStageTRB::ControlDWord::set_cycle(bool v) -> void {
        m_register =
            v ? bit_set(m_register, CYCLE_BIT_OFFSET) : bit_clear(m_register, CYCLE_BIT_OFFSET);
    }
    auto SetupStageTRB::ControlDWord::set_IOC(bool v) -> void {
        m_register =
            v ? bit_set(m_register, IOC_BIT_OFFSET) : bit_clear(m_register, IOC_BIT_OFFSET);
    }
    auto SetupStageTRB::ControlDWord::set_IDT(bool v) -> void {
        m_register =
            v ? bit_set(m_register, IDT_BIT_OFFSET) : bit_clear(m_register, IDT_BIT_OFFSET);
    }
    auto SetupStageTRB::ControlDWord::set_trb_type(U8 val) -> void {
        m_register = (m_register & ~TRB_TYPE_MASK)
                     | ((static_cast<U32>(val) << 10) & TRB_TYPE_MASK);
    }
    auto SetupStageTRB::ControlDWord::set_TRT(U8 val) -> void {
        m_register = (m_register & ~TRT_MASK)
                     | ((static_cast<U32>(val) << SHIFT_16) & TRT_MASK);
    }

    // ========================================================================================== //
    // DataStageTRB — xHCI 2.0 §6.4.1.2.2
    // ========================================================================================== //

    [[nodiscard]] auto DataStageTRB::StatusDWord::trb_transfer_length() const -> U32 {
        return m_register & TRB_TRANSFER_LENGTH_MASK;
    }
    [[nodiscard]] auto DataStageTRB::StatusDWord::td_size() const -> U8 {
        return static_cast<U8>((m_register & TD_SIZE_MASK) >> 17);
    }
    [[nodiscard]] auto DataStageTRB::StatusDWord::interrupter_target() const -> U16 {
        return static_cast<U16>((m_register & INTERRUPTER_TARGET_MASK) >> 22);
    }
    auto DataStageTRB::StatusDWord::set_trb_transfer_length(U32 val) -> void {
        m_register = (m_register & ~TRB_TRANSFER_LENGTH_MASK) | (val & TRB_TRANSFER_LENGTH_MASK);
    }
    auto DataStageTRB::StatusDWord::set_td_size(U8 val) -> void {
        m_register = (m_register & ~TD_SIZE_MASK) | ((static_cast<U32>(val) << 17) & TD_SIZE_MASK);
    }
    auto DataStageTRB::StatusDWord::set_interrupter_target(U16 val) -> void {
        m_register = (m_register & ~INTERRUPTER_TARGET_MASK)
                     | ((static_cast<U32>(val) << 22) & INTERRUPTER_TARGET_MASK);
    }

    [[nodiscard]] auto DataStageTRB::ControlDWord::cycle() const -> bool {
        return bit_check(m_register, CYCLE_BIT_OFFSET);
    }
    [[nodiscard]] auto DataStageTRB::ControlDWord::ENT() const -> bool {
        return bit_check(m_register, ENT_BIT_OFFSET);
    }
    [[nodiscard]] auto DataStageTRB::ControlDWord::ISP() const -> bool {
        return bit_check(m_register, ISP_BIT_OFFSET);
    }
    [[nodiscard]] auto DataStageTRB::ControlDWord::NS() const -> bool {
        return bit_check(m_register, NS_BIT_OFFSET);
    }
    [[nodiscard]] auto DataStageTRB::ControlDWord::chain() const -> bool {
        return bit_check(m_register, CHAIN_BIT_OFFSET);
    }
    [[nodiscard]] auto DataStageTRB::ControlDWord::IOC() const -> bool {
        return bit_check(m_register, IOC_BIT_OFFSET);
    }
    [[nodiscard]] auto DataStageTRB::ControlDWord::IDT() const -> bool {
        return bit_check(m_register, IDT_BIT_OFFSET);
    }
    [[nodiscard]] auto DataStageTRB::ControlDWord::trb_type() const -> U8 {
        return static_cast<U8>((m_register & TRB_TYPE_MASK) >> 10);
    }
    [[nodiscard]] auto DataStageTRB::ControlDWord::DIR() const -> bool {
        return bit_check(m_register, DIR_BIT_OFFSET);
    }
    auto DataStageTRB::ControlDWord::set_cycle(bool v) -> void {
        m_register =
            v ? bit_set(m_register, CYCLE_BIT_OFFSET) : bit_clear(m_register, CYCLE_BIT_OFFSET);
    }
    auto DataStageTRB::ControlDWord::set_ENT(bool v) -> void {
        m_register =
            v ? bit_set(m_register, ENT_BIT_OFFSET) : bit_clear(m_register, ENT_BIT_OFFSET);
    }
    auto DataStageTRB::ControlDWord::set_ISP(bool v) -> void {
        m_register =
            v ? bit_set(m_register, ISP_BIT_OFFSET) : bit_clear(m_register, ISP_BIT_OFFSET);
    }
    auto DataStageTRB::ControlDWord::set_NS(bool v) -> void {
        m_register = v ? bit_set(m_register, NS_BIT_OFFSET) : bit_clear(m_register, NS_BIT_OFFSET);
    }
    auto DataStageTRB::ControlDWord::set_chain(bool v) -> void {
        m_register =
            v ? bit_set(m_register, CHAIN_BIT_OFFSET) : bit_clear(m_register, CHAIN_BIT_OFFSET);
    }
    auto DataStageTRB::ControlDWord::set_IOC(bool v) -> void {
        m_register =
            v ? bit_set(m_register, IOC_BIT_OFFSET) : bit_clear(m_register, IOC_BIT_OFFSET);
    }
    auto DataStageTRB::ControlDWord::set_IDT(bool v) -> void {
        m_register =
            v ? bit_set(m_register, IDT_BIT_OFFSET) : bit_clear(m_register, IDT_BIT_OFFSET);
    }
    auto DataStageTRB::ControlDWord::set_trb_type(U8 val) -> void {
        m_register = (m_register & ~TRB_TYPE_MASK)
                     | ((static_cast<U32>(val) << 10) & TRB_TYPE_MASK);
    }
    auto DataStageTRB::ControlDWord::set_DIR(bool v) -> void {
        m_register =
            v ? bit_set(m_register, DIR_BIT_OFFSET) : bit_clear(m_register, DIR_BIT_OFFSET);
    }

    // ========================================================================================== //
    // StatusStageTRB — xHCI 2.0 §6.4.1.2.3
    // ========================================================================================== //

    [[nodiscard]] auto StatusStageTRB::StatusDWord::interrupter_target() const -> U16 {
        return static_cast<U16>((m_register & INTERRUPTER_TARGET_MASK) >> 22);
    }
    auto StatusStageTRB::StatusDWord::set_interrupter_target(U16 val) -> void {
        m_register = (m_register & ~INTERRUPTER_TARGET_MASK)
                     | ((static_cast<U32>(val) << 22) & INTERRUPTER_TARGET_MASK);
    }

    [[nodiscard]] auto StatusStageTRB::ControlDWord::cycle() const -> bool {
        return bit_check(m_register, CYCLE_BIT_OFFSET);
    }
    [[nodiscard]] auto StatusStageTRB::ControlDWord::ENT() const -> bool {
        return bit_check(m_register, ENT_BIT_OFFSET);
    }
    [[nodiscard]] auto StatusStageTRB::ControlDWord::IOC() const -> bool {
        return bit_check(m_register, IOC_BIT_OFFSET);
    }
    [[nodiscard]] auto StatusStageTRB::ControlDWord::trb_type() const -> U8 {
        return static_cast<U8>((m_register & TRB_TYPE_MASK) >> 10);
    }
    [[nodiscard]] auto StatusStageTRB::ControlDWord::DIR() const -> bool {
        return bit_check(m_register, DIR_BIT_OFFSET);
    }
    auto StatusStageTRB::ControlDWord::set_cycle(bool v) -> void {
        m_register =
            v ? bit_set(m_register, CYCLE_BIT_OFFSET) : bit_clear(m_register, CYCLE_BIT_OFFSET);
    }
    auto StatusStageTRB::ControlDWord::set_ENT(bool v) -> void {
        m_register =
            v ? bit_set(m_register, ENT_BIT_OFFSET) : bit_clear(m_register, ENT_BIT_OFFSET);
    }
    auto StatusStageTRB::ControlDWord::set_IOC(bool v) -> void {
        m_register =
            v ? bit_set(m_register, IOC_BIT_OFFSET) : bit_clear(m_register, IOC_BIT_OFFSET);
    }
    auto StatusStageTRB::ControlDWord::set_trb_type(U8 val) -> void {
        m_register = (m_register & ~TRB_TYPE_MASK)
                     | ((static_cast<U32>(val) << 10) & TRB_TYPE_MASK);
    }
    auto StatusStageTRB::ControlDWord::set_DIR(bool v) -> void {
        m_register =
            v ? bit_set(m_register, DIR_BIT_OFFSET) : bit_clear(m_register, DIR_BIT_OFFSET);
    }

    // ========================================================================================== //
    // LinkTRB — xHCI 2.0 §6.4.4.1
    // ========================================================================================== //

    [[nodiscard]] auto LinkTRB::RingSegmentPtrLoDWord::ptr() const -> U32 {
        return (m_register & PTR_MASK) >> 4;
    }
    auto LinkTRB::RingSegmentPtrLoDWord::set_ptr(U32 val) -> void {
        m_register = (val << 4) & PTR_MASK;
    }

    [[nodiscard]] auto LinkTRB::StatusDWord::interrupter_target() const -> U16 {
        return static_cast<U16>((m_register & INTERRUPTER_TARGET_MASK) >> 22);
    }
    auto LinkTRB::StatusDWord::set_interrupter_target(U16 val) -> void {
        m_register = (m_register & ~INTERRUPTER_TARGET_MASK)
                     | ((static_cast<U32>(val) << 22) & INTERRUPTER_TARGET_MASK);
    }

    [[nodiscard]] auto LinkTRB::ControlDWord::cycle() const -> bool {
        return bit_check(m_register, CYCLE_BIT_OFFSET);
    }
    [[nodiscard]] auto LinkTRB::ControlDWord::toggle_cycle() const -> bool {
        return bit_check(m_register, TOGGLE_CYCLE_BIT_OFFSET);
    }
    [[nodiscard]] auto LinkTRB::ControlDWord::chain() const -> bool {
        return bit_check(m_register, CHAIN_BIT_OFFSET);
    }
    [[nodiscard]] auto LinkTRB::ControlDWord::IOC() const -> bool {
        return bit_check(m_register, IOC_BIT_OFFSET);
    }
    [[nodiscard]] auto LinkTRB::ControlDWord::trb_type() const -> U8 {
        return static_cast<U8>((m_register & TRB_TYPE_MASK) >> 10);
    }
    auto LinkTRB::ControlDWord::set_cycle(bool v) -> void {
        m_register =
            v ? bit_set(m_register, CYCLE_BIT_OFFSET) : bit_clear(m_register, CYCLE_BIT_OFFSET);
    }
    auto LinkTRB::ControlDWord::set_toggle_cycle(bool v) -> void {
        m_register = v ? bit_set(m_register, TOGGLE_CYCLE_BIT_OFFSET)
                       : bit_clear(m_register, TOGGLE_CYCLE_BIT_OFFSET);
    }
    auto LinkTRB::ControlDWord::set_chain(bool v) -> void {
        m_register =
            v ? bit_set(m_register, CHAIN_BIT_OFFSET) : bit_clear(m_register, CHAIN_BIT_OFFSET);
    }
    auto LinkTRB::ControlDWord::set_IOC(bool v) -> void {
        m_register =
            v ? bit_set(m_register, IOC_BIT_OFFSET) : bit_clear(m_register, IOC_BIT_OFFSET);
    }
    auto LinkTRB::ControlDWord::set_trb_type(U8 val) -> void {
        m_register = (m_register & ~TRB_TYPE_MASK)
                     | ((static_cast<U32>(val) << 10) & TRB_TYPE_MASK);
    }

    // ========================================================================================== //
    // EnableSlotCommandTRB — xHCI 2.0 §6.4.3.2
    // ========================================================================================== //

    [[nodiscard]] auto EnableSlotCommandTRB::ControlDWord::cycle() const -> bool {
        return bit_check(m_register, CYCLE_BIT_OFFSET);
    }
    [[nodiscard]] auto EnableSlotCommandTRB::ControlDWord::trb_type() const -> U8 {
        return static_cast<U8>((m_register & TRB_TYPE_MASK) >> 10);
    }
    [[nodiscard]] auto EnableSlotCommandTRB::ControlDWord::slot_type() const -> U8 {
        return static_cast<U8>((m_register & SLOT_TYPE_MASK) >> SHIFT_16);
    }
    auto EnableSlotCommandTRB::ControlDWord::set_cycle(bool v) -> void {
        m_register =
            v ? bit_set(m_register, CYCLE_BIT_OFFSET) : bit_clear(m_register, CYCLE_BIT_OFFSET);
    }
    auto EnableSlotCommandTRB::ControlDWord::set_trb_type(U8 val) -> void {
        m_register = (m_register & ~TRB_TYPE_MASK)
                     | ((static_cast<U32>(val) << 10) & TRB_TYPE_MASK);
    }
    auto EnableSlotCommandTRB::ControlDWord::set_slot_type(U8 val) -> void {
        m_register = (m_register & ~SLOT_TYPE_MASK)
                     | ((static_cast<U32>(val) << SHIFT_16) & SLOT_TYPE_MASK);
    }

    // ========================================================================================== //
    // DisableSlotCommandTRB — xHCI 2.0 §6.4.3.3
    // ========================================================================================== //

    [[nodiscard]] auto DisableSlotCommandTRB::ControlDWord::cycle() const -> bool {
        return bit_check(m_register, CYCLE_BIT_OFFSET);
    }
    [[nodiscard]] auto DisableSlotCommandTRB::ControlDWord::trb_type() const -> U8 {
        return static_cast<U8>((m_register & TRB_TYPE_MASK) >> 10);
    }
    [[nodiscard]] auto DisableSlotCommandTRB::ControlDWord::slot_id() const -> U8 {
        return static_cast<U8>((m_register & SLOT_ID_MASK) >> SHIFT_24);
    }
    auto DisableSlotCommandTRB::ControlDWord::set_cycle(bool v) -> void {
        m_register =
            v ? bit_set(m_register, CYCLE_BIT_OFFSET) : bit_clear(m_register, CYCLE_BIT_OFFSET);
    }
    auto DisableSlotCommandTRB::ControlDWord::set_trb_type(U8 val) -> void {
        m_register = (m_register & ~TRB_TYPE_MASK)
                     | ((static_cast<U32>(val) << 10) & TRB_TYPE_MASK);
    }
    auto DisableSlotCommandTRB::ControlDWord::set_slot_id(U8 val) -> void {
        m_register = (m_register & ~SLOT_ID_MASK)
                     | ((static_cast<U32>(val) << SHIFT_24) & SLOT_ID_MASK);
    }

    // ========================================================================================== //
    // AddressDeviceCommandTRB — xHCI 2.0 §6.4.3.4
    // ========================================================================================== //

    [[nodiscard]] auto AddressDeviceCommandTRB::InputContextPtrLoDWord::ptr() const -> U32 {
        return (m_register & PTR_MASK) >> 4;
    }
    auto AddressDeviceCommandTRB::InputContextPtrLoDWord::set_ptr(U32 val) -> void {
        m_register = (val << 4) & PTR_MASK;
    }

    [[nodiscard]] auto AddressDeviceCommandTRB::ControlDWord::cycle() const -> bool {
        return bit_check(m_register, CYCLE_BIT_OFFSET);
    }
    [[nodiscard]] auto AddressDeviceCommandTRB::ControlDWord::BSR() const -> bool {
        return bit_check(m_register, BSR_BIT_OFFSET);
    }
    [[nodiscard]] auto AddressDeviceCommandTRB::ControlDWord::trb_type() const -> U8 {
        return static_cast<U8>((m_register & TRB_TYPE_MASK) >> 10);
    }
    [[nodiscard]] auto AddressDeviceCommandTRB::ControlDWord::slot_id() const -> U8 {
        return static_cast<U8>((m_register & SLOT_ID_MASK) >> SHIFT_24);
    }
    auto AddressDeviceCommandTRB::ControlDWord::set_cycle(bool v) -> void {
        m_register =
            v ? bit_set(m_register, CYCLE_BIT_OFFSET) : bit_clear(m_register, CYCLE_BIT_OFFSET);
    }
    auto AddressDeviceCommandTRB::ControlDWord::set_BSR(bool v) -> void {
        m_register =
            v ? bit_set(m_register, BSR_BIT_OFFSET) : bit_clear(m_register, BSR_BIT_OFFSET);
    }
    auto AddressDeviceCommandTRB::ControlDWord::set_trb_type(U8 val) -> void {
        m_register = (m_register & ~TRB_TYPE_MASK)
                     | ((static_cast<U32>(val) << 10) & TRB_TYPE_MASK);
    }
    auto AddressDeviceCommandTRB::ControlDWord::set_slot_id(U8 val) -> void {
        m_register = (m_register & ~SLOT_ID_MASK)
                     | ((static_cast<U32>(val) << SHIFT_24) & SLOT_ID_MASK);
    }

    // ========================================================================================== //
    // ConfigureEndpointCommandTRB — xHCI 2.0 §6.4.3.5
    // ========================================================================================== //

    [[nodiscard]] auto ConfigureEndpointCommandTRB::InputContextPtrLoDWord::ptr() const -> U32 {
        return (m_register & PTR_MASK) >> 4;
    }
    auto ConfigureEndpointCommandTRB::InputContextPtrLoDWord::set_ptr(U32 val) -> void {
        m_register = (val << 4) & PTR_MASK;
    }

    [[nodiscard]] auto ConfigureEndpointCommandTRB::ControlDWord::cycle() const -> bool {
        return bit_check(m_register, CYCLE_BIT_OFFSET);
    }
    [[nodiscard]] auto ConfigureEndpointCommandTRB::ControlDWord::DC() const -> bool {
        return bit_check(m_register, DC_BIT_OFFSET);
    }
    [[nodiscard]] auto ConfigureEndpointCommandTRB::ControlDWord::trb_type() const -> U8 {
        return static_cast<U8>((m_register & TRB_TYPE_MASK) >> 10);
    }
    [[nodiscard]] auto ConfigureEndpointCommandTRB::ControlDWord::slot_id() const -> U8 {
        return static_cast<U8>((m_register & SLOT_ID_MASK) >> SHIFT_24);
    }
    auto ConfigureEndpointCommandTRB::ControlDWord::set_cycle(bool v) -> void {
        m_register =
            v ? bit_set(m_register, CYCLE_BIT_OFFSET) : bit_clear(m_register, CYCLE_BIT_OFFSET);
    }
    auto ConfigureEndpointCommandTRB::ControlDWord::set_DC(bool v) -> void {
        m_register =
            v ? bit_set(m_register, DC_BIT_OFFSET) : bit_clear(m_register, DC_BIT_OFFSET);
    }
    auto ConfigureEndpointCommandTRB::ControlDWord::set_trb_type(U8 val) -> void {
        m_register = (m_register & ~TRB_TYPE_MASK)
                     | ((static_cast<U32>(val) << 10) & TRB_TYPE_MASK);
    }
    auto ConfigureEndpointCommandTRB::ControlDWord::set_slot_id(U8 val) -> void {
        m_register = (m_register & ~SLOT_ID_MASK)
                     | ((static_cast<U32>(val) << SHIFT_24) & SLOT_ID_MASK);
    }

    // ========================================================================================== //
    // NoOpCommandTRB — xHCI 2.0 §6.4.3.1
    // ========================================================================================== //

    [[nodiscard]] auto NoOpCommandTRB::ControlDWord::cycle() const -> bool {
        return bit_check(m_register, CYCLE_BIT_OFFSET);
    }
    [[nodiscard]] auto NoOpCommandTRB::ControlDWord::trb_type() const -> U8 {
        return static_cast<U8>((m_register & TRB_TYPE_MASK) >> 10);
    }
    auto NoOpCommandTRB::ControlDWord::set_cycle(bool v) -> void {
        m_register =
            v ? bit_set(m_register, CYCLE_BIT_OFFSET) : bit_clear(m_register, CYCLE_BIT_OFFSET);
    }
    auto NoOpCommandTRB::ControlDWord::set_trb_type(U8 val) -> void {
        m_register = (m_register & ~TRB_TYPE_MASK)
                     | ((static_cast<U32>(val) << 10) & TRB_TYPE_MASK);
    }

    // ========================================================================================== //
    // TransferEventTRB — xHCI 2.0 §6.4.2.1
    // ========================================================================================== //

    [[nodiscard]] auto TransferEventTRB::StatusDWord::trb_transfer_length() const -> U32 {
        return m_register & TRB_TRANSFER_LENGTH_MASK;
    }
    [[nodiscard]] auto TransferEventTRB::StatusDWord::completion_code() const -> U8 {
        return static_cast<U8>((m_register & COMPLETION_CODE_MASK) >> SHIFT_24);
    }
    auto TransferEventTRB::StatusDWord::set_trb_transfer_length(U32 val) -> void {
        m_register = (m_register & ~TRB_TRANSFER_LENGTH_MASK) | (val & TRB_TRANSFER_LENGTH_MASK);
    }
    auto TransferEventTRB::StatusDWord::set_completion_code(U8 val) -> void {
        m_register = (m_register & ~COMPLETION_CODE_MASK)
                     | ((static_cast<U32>(val) << SHIFT_24) & COMPLETION_CODE_MASK);
    }

    [[nodiscard]] auto TransferEventTRB::ControlDWord::cycle() const -> bool {
        return bit_check(m_register, CYCLE_BIT_OFFSET);
    }
    [[nodiscard]] auto TransferEventTRB::ControlDWord::ED() const -> bool {
        return bit_check(m_register, ED_BIT_OFFSET);
    }
    [[nodiscard]] auto TransferEventTRB::ControlDWord::trb_type() const -> U8 {
        return static_cast<U8>((m_register & TRB_TYPE_MASK) >> 10);
    }
    [[nodiscard]] auto TransferEventTRB::ControlDWord::endpoint_id() const -> U8 {
        return static_cast<U8>((m_register & ENDPOINT_ID_MASK) >> SHIFT_16);
    }
    [[nodiscard]] auto TransferEventTRB::ControlDWord::slot_id() const -> U8 {
        return static_cast<U8>((m_register & SLOT_ID_MASK) >> SHIFT_24);
    }
    auto TransferEventTRB::ControlDWord::set_cycle(bool v) -> void {
        m_register =
            v ? bit_set(m_register, CYCLE_BIT_OFFSET) : bit_clear(m_register, CYCLE_BIT_OFFSET);
    }
    auto TransferEventTRB::ControlDWord::set_ED(bool v) -> void {
        m_register =
            v ? bit_set(m_register, ED_BIT_OFFSET) : bit_clear(m_register, ED_BIT_OFFSET);
    }
    auto TransferEventTRB::ControlDWord::set_trb_type(U8 val) -> void {
        m_register = (m_register & ~TRB_TYPE_MASK)
                     | ((static_cast<U32>(val) << 10) & TRB_TYPE_MASK);
    }
    auto TransferEventTRB::ControlDWord::set_endpoint_id(U8 val) -> void {
        m_register = (m_register & ~ENDPOINT_ID_MASK)
                     | ((static_cast<U32>(val) << SHIFT_16) & ENDPOINT_ID_MASK);
    }
    auto TransferEventTRB::ControlDWord::set_slot_id(U8 val) -> void {
        m_register = (m_register & ~SLOT_ID_MASK)
                     | ((static_cast<U32>(val) << SHIFT_24) & SLOT_ID_MASK);
    }

    // ========================================================================================== //
    // CommandCompletionEventTRB — xHCI 2.0 §6.4.2.2
    // ========================================================================================== //

    [[nodiscard]] auto CommandCompletionEventTRB::CommandTrbPtrLoDWord::ptr() const -> U32 {
        return (m_register & PTR_MASK) >> 4;
    }
    auto CommandCompletionEventTRB::CommandTrbPtrLoDWord::set_ptr(U32 val) -> void {
        m_register = (val << 4) & PTR_MASK;
    }

    [[nodiscard]] auto CommandCompletionEventTRB::StatusDWord::completion_parameter() const -> U32 {
        return m_register & COMPLETION_PARAMETER_MASK;
    }
    [[nodiscard]] auto CommandCompletionEventTRB::StatusDWord::completion_code() const -> U8 {
        return static_cast<U8>((m_register & COMPLETION_CODE_MASK) >> SHIFT_24);
    }
    auto CommandCompletionEventTRB::StatusDWord::set_completion_parameter(U32 val) -> void {
        m_register =
            (m_register & ~COMPLETION_PARAMETER_MASK) | (val & COMPLETION_PARAMETER_MASK);
    }
    auto CommandCompletionEventTRB::StatusDWord::set_completion_code(U8 val) -> void {
        m_register = (m_register & ~COMPLETION_CODE_MASK)
                     | ((static_cast<U32>(val) << SHIFT_24) & COMPLETION_CODE_MASK);
    }

    [[nodiscard]] auto CommandCompletionEventTRB::ControlDWord::cycle() const -> bool {
        return bit_check(m_register, CYCLE_BIT_OFFSET);
    }
    [[nodiscard]] auto CommandCompletionEventTRB::ControlDWord::trb_type() const -> U8 {
        return static_cast<U8>((m_register & TRB_TYPE_MASK) >> 10);
    }
    [[nodiscard]] auto CommandCompletionEventTRB::ControlDWord::vf_id() const -> U8 {
        return static_cast<U8>((m_register & VF_ID_MASK) >> SHIFT_16);
    }
    [[nodiscard]] auto CommandCompletionEventTRB::ControlDWord::slot_id() const -> U8 {
        return static_cast<U8>((m_register & SLOT_ID_MASK) >> SHIFT_24);
    }
    auto CommandCompletionEventTRB::ControlDWord::set_cycle(bool v) -> void {
        m_register =
            v ? bit_set(m_register, CYCLE_BIT_OFFSET) : bit_clear(m_register, CYCLE_BIT_OFFSET);
    }
    auto CommandCompletionEventTRB::ControlDWord::set_trb_type(U8 val) -> void {
        m_register = (m_register & ~TRB_TYPE_MASK)
                     | ((static_cast<U32>(val) << 10) & TRB_TYPE_MASK);
    }
    auto CommandCompletionEventTRB::ControlDWord::set_vf_id(U8 val) -> void {
        m_register = (m_register & ~VF_ID_MASK)
                     | ((static_cast<U32>(val) << SHIFT_16) & VF_ID_MASK);
    }
    auto CommandCompletionEventTRB::ControlDWord::set_slot_id(U8 val) -> void {
        m_register = (m_register & ~SLOT_ID_MASK)
                     | ((static_cast<U32>(val) << SHIFT_24) & SLOT_ID_MASK);
    }

    // ========================================================================================== //
    // PortStatusChangeEventTRB — xHCI 2.0 §6.4.2.3
    // ========================================================================================== //

    [[nodiscard]] auto PortStatusChangeEventTRB::PortIdDWord::port_id() const -> U8 {
        return static_cast<U8>((m_register & PORT_ID_MASK) >> SHIFT_24);
    }
    auto PortStatusChangeEventTRB::PortIdDWord::set_port_id(U8 val) -> void {
        m_register = (m_register & ~PORT_ID_MASK)
                     | ((static_cast<U32>(val) << SHIFT_24) & PORT_ID_MASK);
    }

    [[nodiscard]] auto PortStatusChangeEventTRB::StatusDWord::completion_code() const -> U8 {
        return static_cast<U8>((m_register & COMPLETION_CODE_MASK) >> SHIFT_24);
    }
    auto PortStatusChangeEventTRB::StatusDWord::set_completion_code(U8 val) -> void {
        m_register = (m_register & ~COMPLETION_CODE_MASK)
                     | ((static_cast<U32>(val) << SHIFT_24) & COMPLETION_CODE_MASK);
    }

    [[nodiscard]] auto PortStatusChangeEventTRB::ControlDWord::cycle() const -> bool {
        return bit_check(m_register, CYCLE_BIT_OFFSET);
    }
    [[nodiscard]] auto PortStatusChangeEventTRB::ControlDWord::trb_type() const -> U8 {
        return static_cast<U8>((m_register & TRB_TYPE_MASK) >> 10);
    }
    auto PortStatusChangeEventTRB::ControlDWord::set_cycle(bool v) -> void {
        m_register =
            v ? bit_set(m_register, CYCLE_BIT_OFFSET) : bit_clear(m_register, CYCLE_BIT_OFFSET);
    }
    auto PortStatusChangeEventTRB::ControlDWord::set_trb_type(U8 val) -> void {
        m_register = (m_register & ~TRB_TYPE_MASK)
                     | ((static_cast<U32>(val) << 10) & TRB_TYPE_MASK);
    }

} // namespace Rune::Device::USB