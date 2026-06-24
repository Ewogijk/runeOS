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

#include <Device/USB/XHCI/DeviceContext.h>

#include <KRE/BitsAndBytes.h>

namespace Rune::Device::USB {

    // ========================================================================================== //
    // SlotContext::DW0 — xHCI 2.0 §6.2.2 Table 6-7
    // ========================================================================================== //

    [[nodiscard]] auto SlotContext::DW0::route_string() const -> U32 {
        return m_register & ROUTE_STRING_MASK;
    }

    [[nodiscard]] auto SlotContext::DW0::speed() const -> U8 {
        return static_cast<U8>((m_register & SPEED_MASK) >> SHIFT_20);
    }

    [[nodiscard]] auto SlotContext::DW0::MTT() const -> bool {
        return bit_check(m_register, MTT_BIT_OFFSET);
    }

    [[nodiscard]] auto SlotContext::DW0::hub() const -> bool {
        return bit_check(m_register, HUB_BIT_OFFSET);
    }

    [[nodiscard]] auto SlotContext::DW0::context_entries() const -> U8 {
        return static_cast<U8>((m_register & CONTEXT_ENTRIES_MASK) >> 27);
    }

    auto SlotContext::DW0::set_route_string(U32 val) -> void {
        m_register = (m_register & ~ROUTE_STRING_MASK) | (val & ROUTE_STRING_MASK);
    }

    auto SlotContext::DW0::set_speed(U8 val) -> void {
        m_register = (m_register & ~SPEED_MASK) | ((static_cast<U32>(val) << SHIFT_20) & SPEED_MASK);
    }

    auto SlotContext::DW0::set_MTT(bool v) -> void {
        m_register = v ? bit_set(m_register, MTT_BIT_OFFSET) : bit_clear(m_register, MTT_BIT_OFFSET);
    }

    auto SlotContext::DW0::set_hub(bool v) -> void {
        m_register = v ? bit_set(m_register, HUB_BIT_OFFSET) : bit_clear(m_register, HUB_BIT_OFFSET);
    }

    auto SlotContext::DW0::set_context_entries(U8 val) -> void {
        m_register = (m_register & ~CONTEXT_ENTRIES_MASK)
                     | ((static_cast<U32>(val) << 27) & CONTEXT_ENTRIES_MASK);
    }

    // ========================================================================================== //
    // SlotContext::DW1 — xHCI 2.0 §6.2.2 Table 6-7
    // ========================================================================================== //

    [[nodiscard]] auto SlotContext::DW1::max_exit_latency() const -> U16 {
        return static_cast<U16>(m_register & MAX_EXIT_LATENCY_MASK);
    }

    [[nodiscard]] auto SlotContext::DW1::root_hub_port_num() const -> U8 {
        return static_cast<U8>((m_register & ROOT_HUB_PORT_NUM_MASK) >> SHIFT_16);
    }

    [[nodiscard]] auto SlotContext::DW1::num_ports() const -> U8 {
        return static_cast<U8>((m_register & NUM_PORTS_MASK) >> SHIFT_24);
    }

    auto SlotContext::DW1::set_max_exit_latency(U16 val) -> void {
        m_register = (m_register & ~MAX_EXIT_LATENCY_MASK) | static_cast<U32>(val);
    }

    auto SlotContext::DW1::set_root_hub_port_num(U8 val) -> void {
        m_register = (m_register & ~ROOT_HUB_PORT_NUM_MASK)
                     | ((static_cast<U32>(val) << SHIFT_16) & ROOT_HUB_PORT_NUM_MASK);
    }

    auto SlotContext::DW1::set_num_ports(U8 val) -> void {
        m_register = (m_register & ~NUM_PORTS_MASK)
                     | ((static_cast<U32>(val) << SHIFT_24) & NUM_PORTS_MASK);
    }

    // ========================================================================================== //
    // SlotContext::DW2 — xHCI 2.0 §6.2.2 Table 6-7
    // ========================================================================================== //

    [[nodiscard]] auto SlotContext::DW2::parent_hub_slot_id() const -> U8 {
        return static_cast<U8>(m_register & PARENT_HUB_SLOT_ID_MASK);
    }

    [[nodiscard]] auto SlotContext::DW2::parent_port_num() const -> U8 {
        return static_cast<U8>((m_register & PARENT_PORT_NUM_MASK) >> SHIFT_8);
    }

    [[nodiscard]] auto SlotContext::DW2::TTT() const -> U8 {
        return static_cast<U8>((m_register & TTT_MASK) >> SHIFT_16);
    }

    [[nodiscard]] auto SlotContext::DW2::interrupter_target() const -> U16 {
        return static_cast<U16>((m_register & INTERRUPTER_TARGET_MASK) >> 22);
    }

    auto SlotContext::DW2::set_parent_hub_slot_id(U8 val) -> void {
        m_register = (m_register & ~PARENT_HUB_SLOT_ID_MASK) | static_cast<U32>(val);
    }

    auto SlotContext::DW2::set_parent_port_num(U8 val) -> void {
        m_register = (m_register & ~PARENT_PORT_NUM_MASK)
                     | ((static_cast<U32>(val) << SHIFT_8) & PARENT_PORT_NUM_MASK);
    }

    auto SlotContext::DW2::set_TTT(U8 val) -> void {
        m_register = (m_register & ~TTT_MASK)
                     | ((static_cast<U32>(val) << SHIFT_16) & TTT_MASK);
    }

    auto SlotContext::DW2::set_interrupter_target(U16 val) -> void {
        m_register = (m_register & ~INTERRUPTER_TARGET_MASK)
                     | ((static_cast<U32>(val) << 22) & INTERRUPTER_TARGET_MASK);
    }

    // ========================================================================================== //
    // SlotContext::DW3 — xHCI 2.0 §6.2.2 Table 6-7
    // ========================================================================================== //

    [[nodiscard]] auto SlotContext::DW3::usb_device_address() const -> U8 {
        return static_cast<U8>(m_register & USB_DEVICE_ADDRESS_MASK);
    }

    [[nodiscard]] auto SlotContext::DW3::slot_state() const -> U8 {
        return static_cast<U8>((m_register & SLOT_STATE_MASK) >> 27);
    }

    auto SlotContext::DW3::set_usb_device_address(U8 val) -> void {
        m_register = (m_register & ~USB_DEVICE_ADDRESS_MASK) | static_cast<U32>(val);
    }

    auto SlotContext::DW3::set_slot_state(U8 val) -> void {
        m_register = (m_register & ~SLOT_STATE_MASK)
                     | ((static_cast<U32>(val) << 27) & SLOT_STATE_MASK);
    }

    // ========================================================================================== //
    // EndpointContext::DW0 — xHCI 2.0 §6.2.3 Table 6-9
    // ========================================================================================== //

    [[nodiscard]] auto EndpointContext::DW0::ep_state() const -> U8 {
        return static_cast<U8>(m_register & EP_STATE_MASK);
    }

    [[nodiscard]] auto EndpointContext::DW0::mult() const -> U8 {
        return static_cast<U8>((m_register & MULT_MASK) >> SHIFT_8);
    }

    [[nodiscard]] auto EndpointContext::DW0::max_p_streams() const -> U8 {
        return static_cast<U8>((m_register & MAX_P_STREAMS_MASK) >> 10);
    }

    [[nodiscard]] auto EndpointContext::DW0::LSA() const -> bool {
        return bit_check(m_register, LSA_BIT_OFFSET);
    }

    [[nodiscard]] auto EndpointContext::DW0::interval() const -> U8 {
        return static_cast<U8>((m_register & INTERVAL_MASK) >> SHIFT_16);
    }

    [[nodiscard]] auto EndpointContext::DW0::max_esit_hi() const -> U8 {
        return static_cast<U8>((m_register & MAX_ESIT_HI_MASK) >> SHIFT_24);
    }

    auto EndpointContext::DW0::set_ep_state(U8 val) -> void {
        m_register = (m_register & ~EP_STATE_MASK) | (static_cast<U32>(val) & EP_STATE_MASK);
    }

    auto EndpointContext::DW0::set_mult(U8 val) -> void {
        m_register = (m_register & ~MULT_MASK)
                     | ((static_cast<U32>(val) << SHIFT_8) & MULT_MASK);
    }

    auto EndpointContext::DW0::set_max_p_streams(U8 val) -> void {
        m_register = (m_register & ~MAX_P_STREAMS_MASK)
                     | ((static_cast<U32>(val) << 10) & MAX_P_STREAMS_MASK);
    }

    auto EndpointContext::DW0::set_LSA(bool v) -> void {
        m_register = v ? bit_set(m_register, LSA_BIT_OFFSET) : bit_clear(m_register, LSA_BIT_OFFSET);
    }

    auto EndpointContext::DW0::set_interval(U8 val) -> void {
        m_register = (m_register & ~INTERVAL_MASK)
                     | ((static_cast<U32>(val) << SHIFT_16) & INTERVAL_MASK);
    }

    auto EndpointContext::DW0::set_max_esit_hi(U8 val) -> void {
        m_register = (m_register & ~MAX_ESIT_HI_MASK)
                     | ((static_cast<U32>(val) << SHIFT_24) & MAX_ESIT_HI_MASK);
    }

    // ========================================================================================== //
    // EndpointContext::DW1 — xHCI 2.0 §6.2.3 Table 6-9
    // ========================================================================================== //

    [[nodiscard]] auto EndpointContext::DW1::CERR() const -> U8 {
        return static_cast<U8>((m_register & CERR_MASK) >> 1);
    }

    [[nodiscard]] auto EndpointContext::DW1::ep_type() const -> U8 {
        return static_cast<U8>((m_register & EP_TYPE_MASK) >> 3);
    }

    [[nodiscard]] auto EndpointContext::DW1::HID() const -> bool {
        return bit_check(m_register, HID_BIT_OFFSET);
    }

    [[nodiscard]] auto EndpointContext::DW1::max_burst_size() const -> U8 {
        return static_cast<U8>((m_register & MAX_BURST_SIZE_MASK) >> SHIFT_8);
    }

    [[nodiscard]] auto EndpointContext::DW1::max_packet_size() const -> U16 {
        return static_cast<U16>((m_register & MAX_PACKET_SIZE_MASK) >> SHIFT_16);
    }

    auto EndpointContext::DW1::set_CERR(U8 val) -> void {
        m_register = (m_register & ~CERR_MASK)
                     | ((static_cast<U32>(val) << 1) & CERR_MASK);
    }

    auto EndpointContext::DW1::set_ep_type(U8 val) -> void {
        m_register = (m_register & ~EP_TYPE_MASK)
                     | ((static_cast<U32>(val) << 3) & EP_TYPE_MASK);
    }

    auto EndpointContext::DW1::set_HID(bool v) -> void {
        m_register = v ? bit_set(m_register, HID_BIT_OFFSET) : bit_clear(m_register, HID_BIT_OFFSET);
    }

    auto EndpointContext::DW1::set_max_burst_size(U8 val) -> void {
        m_register = (m_register & ~MAX_BURST_SIZE_MASK)
                     | ((static_cast<U32>(val) << SHIFT_8) & MAX_BURST_SIZE_MASK);
    }

    auto EndpointContext::DW1::set_max_packet_size(U16 val) -> void {
        m_register = (m_register & ~MAX_PACKET_SIZE_MASK)
                     | ((static_cast<U32>(val) << SHIFT_16) & MAX_PACKET_SIZE_MASK);
    }

    // ========================================================================================== //
    // EndpointContext::TRDequeuePtr — xHCI 2.0 §6.2.3 Table 6-9
    // ========================================================================================== //

    [[nodiscard]] auto EndpointContext::TRDequeuePtr::DCS() const -> bool {
        return bit_check(m_register, DCS_BIT_OFFSET);
    }

    [[nodiscard]] auto EndpointContext::TRDequeuePtr::ptr() const -> U64 {
        return (m_register & PTR_MASK) >> 4;
    }

    auto EndpointContext::TRDequeuePtr::set_DCS(bool v) -> void {
        m_register =
            v ? bit_set(m_register, DCS_BIT_OFFSET) : bit_clear(m_register, DCS_BIT_OFFSET);
    }

    auto EndpointContext::TRDequeuePtr::set_ptr(U64 val) -> void {
        m_register = (m_register & ~PTR_MASK) | ((val << 4) & PTR_MASK);
    }

    // ========================================================================================== //
    // EndpointContext::DW4 — xHCI 2.0 §6.2.3 Table 6-9
    // ========================================================================================== //

    [[nodiscard]] auto EndpointContext::DW4::average_trb_length() const -> U16 {
        return static_cast<U16>(m_register & AVERAGE_TRB_LENGTH_MASK);
    }

    [[nodiscard]] auto EndpointContext::DW4::max_esit_lo() const -> U16 {
        return static_cast<U16>((m_register & MAX_ESIT_LO_MASK) >> SHIFT_16);
    }

    auto EndpointContext::DW4::set_average_trb_length(U16 val) -> void {
        m_register = (m_register & ~AVERAGE_TRB_LENGTH_MASK) | static_cast<U32>(val);
    }

    auto EndpointContext::DW4::set_max_esit_lo(U16 val) -> void {
        m_register = (m_register & ~MAX_ESIT_LO_MASK)
                     | ((static_cast<U32>(val) << SHIFT_16) & MAX_ESIT_LO_MASK);
    }

    // ========================================================================================== //
    // InputControlContext::DropContextFlags — xHCI 2.0 §6.2.5.1 Table 6-13
    // ========================================================================================== //

    [[nodiscard]] auto InputControlContext::DropContextFlags::D() const -> U32 {
        return (m_register & D_MASK) >> 2;
    }

    auto InputControlContext::DropContextFlags::set_D(U32 val) -> void {
        m_register = (m_register & ~D_MASK) | ((val << 2) & D_MASK);
    }

    // ========================================================================================== //
    // InputControlContext::InputControlContextFieldDefs — xHCI 2.0 §6.2.5.1 Table 6-13
    // ========================================================================================== //

    [[nodiscard]] auto InputControlContext::InputControlContextFieldDefs::configuration_value()
        const -> U8 {
        return static_cast<U8>(m_register & CONFIGURATION_VALUE_MASK);
    }

    [[nodiscard]] auto InputControlContext::InputControlContextFieldDefs::interface_number()
        const -> U8 {
        return static_cast<U8>((m_register & INTERFACE_NUMBER_MASK) >> SHIFT_8);
    }

    [[nodiscard]] auto InputControlContext::InputControlContextFieldDefs::alternate_setting()
        const -> U8 {
        return static_cast<U8>((m_register & ALTERNATE_SETTING_MASK) >> SHIFT_16);
    }

    auto InputControlContext::InputControlContextFieldDefs::set_configuration_value(U8 val) -> void {
        m_register = (m_register & ~CONFIGURATION_VALUE_MASK) | static_cast<U32>(val);
    }

    auto InputControlContext::InputControlContextFieldDefs::set_interface_number(U8 val) -> void {
        m_register = (m_register & ~INTERFACE_NUMBER_MASK)
                     | ((static_cast<U32>(val) << SHIFT_8) & INTERFACE_NUMBER_MASK);
    }

    auto InputControlContext::InputControlContextFieldDefs::set_alternate_setting(U8 val) -> void {
        m_register = (m_register & ~ALTERNATE_SETTING_MASK)
                     | ((static_cast<U32>(val) << SHIFT_16) & ALTERNATE_SETTING_MASK);
    }

} // namespace Rune::Device::USB