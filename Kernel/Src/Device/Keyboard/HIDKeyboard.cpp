//  Copyright 2026 Ewogijk
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

#include <Device/Keyboard/HIDKeyboard.h>

#include <Memory/DMA.h>

#include <CPU/CPUModule.h>

#include <KRE/BitsAndBytes.h>
#include <KRE/Collections/Array.h>

#include <Device/DeviceModule.h>
#include <Device/KeyEventBuffer.h>
#include <Device/USB/ClassCode.h>
#include <Device/USB/HID/HID.h>
#include <Device/USB/HID/ItemParser.h>
#include <Device/USB/HID/UsageTables.h>

namespace Rune::Device {

    // ========================================================================================== //
    // HIDKeyCodeSet
    // ========================================================================================== //

    struct HIDKeyCodeSet {
        static constexpr U8 BIT_MAP_QUAD_SHIFT = 6;
        static constexpr U8 KEY_CODE_MASK      = 63;

        Array<U64, 4> m_key_bitmap;

        void set(U8 key_code) {
            m_key_bitmap[key_code >> BIT_MAP_QUAD_SHIFT] |= 1ULL << (key_code & KEY_CODE_MASK);
        }

        [[nodiscard]] auto is_set(U8 key_code) const -> bool {
            return (m_key_bitmap[key_code >> BIT_MAP_QUAD_SHIFT]
                    & (1ULL << (key_code & KEY_CODE_MASK)))
                   > 0;
        }

        friend auto operator==(const HIDKeyCodeSet& lhs, const HIDKeyCodeSet& rhs) -> bool {
            return lhs.m_key_bitmap[0] == rhs.m_key_bitmap[0]
                   && lhs.m_key_bitmap[1] == rhs.m_key_bitmap[1]
                   && lhs.m_key_bitmap[2] == rhs.m_key_bitmap[2]
                   && lhs.m_key_bitmap[3] == rhs.m_key_bitmap[3];
        }

        friend auto operator!=(const HIDKeyCodeSet& lhs, const HIDKeyCodeSet& rhs) -> bool {
            return !(lhs == rhs);
        }
    };

    // ========================================================================================== //
    // HIDKeyboardContext
    // ========================================================================================== //

    constexpr U8 HID_LCTRL_SHIFT  = 0;
    constexpr U8 HID_LSHIFT_SHIFT = 1;
    constexpr U8 HID_LALT_SHIFT   = 2;
    constexpr U8 HID_LGUI_SHIFT   = 3;
    constexpr U8 HID_RCTRL_SHIFT  = 4;
    constexpr U8 HID_RSHIFT_SHIFT = 5;
    constexpr U8 HID_RALT_SHIFT   = 6;
    constexpr U8 HID_RGUI_SHIFT   = 7;

    auto build_key_event(Ember::VirtualKey vk, U8 hid_modifiers, bool key_down) {
        return Ember::KeyEventBuilder()
            .with_virtual_key(vk)
            .with_lctrl(bit_check(hid_modifiers, HID_LCTRL_SHIFT))
            .with_lshift(bit_check(hid_modifiers, HID_LSHIFT_SHIFT))
            .with_lalt(bit_check(hid_modifiers, HID_LALT_SHIFT))
            .with_lgui(bit_check(hid_modifiers, HID_LGUI_SHIFT))
            .with_rctrl(bit_check(hid_modifiers, HID_RCTRL_SHIFT))
            .with_rshift(bit_check(hid_modifiers, HID_RSHIFT_SHIFT))
            .with_ralt(bit_check(hid_modifiers, HID_RALT_SHIFT))
            .with_rgui(bit_check(hid_modifiers, HID_RGUI_SHIFT))
            .with_key_down(key_down)
            .build();
    }

    void publish_key_event(Ember::VirtualKey key, U8 modifiers, bool key_down) {
        auto key_event = build_key_event(key, modifiers, key_down);
        g_key_event_buffer.append(key_event);
        DEBUG("KeyEvent[{},{},{}]",
              key_event.virtual_key().to_string(),
              byte_get(key_event.event_code(), 1),
              key_event.is_key_down())
    }

    void HIDKeyboardContext::publish_key_events(const HIDKeyboardFrame& new_frame) {
        HIDKeyCodeSet was_down{};
        HIDKeyCodeSet is_down{};
        for (size_t i = 0; i < HIDKeyboardFrame::KEYCODE_COUNT; i++) {
            was_down.set(m_last_keyboard_frame.m_key_codes[i]);
            is_down.set(new_frame.m_key_codes[i]);
        }

        Ember::VirtualKey last_pressed_key = Ember::VirtualKey::NONE;
        for (size_t i = 0; i < HIDKeyboardFrame::KEYCODE_COUNT; i++) {
            U8 released = m_last_keyboard_frame.m_key_codes[i];
            if (0x04 <= released && !is_down.is_set(released))
                publish_key_event(Ember::VirtualKey(released),
                                  m_last_keyboard_frame.m_modifiers,
                                  false);
        }
        for (size_t i = 0; i < HIDKeyboardFrame::KEYCODE_COUNT; i++) {
            U8 pressed = new_frame.m_key_codes[i];
            if (0x04 <= pressed && !was_down.is_set(pressed))
                publish_key_event(Ember::VirtualKey(pressed), new_frame.m_modifiers, true);

            if (0x04 <= pressed) last_pressed_key = Ember::VirtualKey(pressed);
        }

        if (last_pressed_key != Ember::VirtualKey::NONE && was_down == is_down
            && m_last_pressed == last_pressed_key)
            publish_key_event(last_pressed_key, new_frame.m_modifiers, true);
        m_last_keyboard_frame = new_frame;
        m_last_pressed        = last_pressed_key;
    }

    // ========================================================================================== //
    // HIDKeyboard
    // ========================================================================================== //

    auto make_hid_keyboard_frame(const USB::HIDReport& keyboard_report_def,
                                 U8*                   keyboard_report,
                                 U16                   report_size,
                                 bool uses_report_ID) -> Optional<HIDKeyboardFrame> {
        U8*              report_begin = uses_report_ID ? keyboard_report + 1 : keyboard_report;
        U16              size         = report_size - (uses_report_ID ? 1 : 0);
        HIDKeyboardFrame frame{};
        for (const auto& data : keyboard_report_def.m_data) {
            if (data.is_padding()) continue;
            if (data.m_usage_ranges[0].m_min.decode_usage_page()
                != USB::HIDUsagePage::KEYBOARD_KEYPAD)
                continue;

            if (data.m_flags.has(USB::HIDDataFlag::DATA)
                && data.m_flags.has(USB::HIDDataFlag::VARIABLE)) {
                for (U8 i = 0; i < min(data.m_report_count, static_cast<U32>(BIT_COUNT_BYTE));
                     i++) {
                    auto field = data.read_value(report_begin, size, i);
                    if (!field) return {};
                    if (field.value() != 0) frame.m_modifiers |= static_cast<U8>(1U << i);
                }
            }

            if (data.m_flags.has(USB::HIDDataFlag::DATA)
                && data.m_flags.has(USB::HIDDataFlag::ARRAY)) {
                for (size_t i = 0; i < min(data.m_report_count, HIDKeyboardFrame::KEYCODE_COUNT);
                     i++) {
                    auto field = data.read_value(report_begin, size, i);
                    if (!field) return {};
                    frame.m_key_codes[i] = static_cast<U8>(field.value());
                }
            }
        }
        return {frame};
    }

    auto poll_keyboard(ThreadStartupPacket* tsp) -> int {
        if (tsp->argc != 2) {
            ERROR("HIDKeyboard driver address is missing.")
            return -1;
        }
        uintptr_t ptr = 0;
        if (!parse_int<uintptr_t>(tsp->argv[0], Radix::HEX, ptr)) return -1;
        Ember::Handle device_handle = 0;
        if (!parse_int<Ember::Handle>(tsp->argv[1], Radix::DECIMAL, device_handle)) return -1;
        auto* driver = reinterpret_cast<HIDKeyboardDriver*>(ptr);

        SharedPointer<HIDKeyboardContext> kb_ctx;
        {
            CriticalSection<SpinlockIRQSafe> _(driver->m_lock);
            auto kb_ctx_it = driver->m_keyboard_contexts.find(device_handle);
            if (kb_ctx_it == driver->m_keyboard_contexts.end()) return -1;
            kb_ctx = *kb_ctx_it->value;
        }
        auto*             dm = System::instance().get_module<DeviceModule>(ModuleSelector::DEVICE);
        UniquePointer<U8> keyboard_report(reinterpret_cast<U8*>(
            Memory::DMA::allocate_zeroed(kb_ctx->m_ep_in_interrupt_max_packet_size)));
        while (true) {
            USB::DataTransferRequest dtr{
                .m_header          = {.m_transfer_type = USB::TransferRequestType::INTERRUPT,
                                      .m_device_handle = kb_ctx->m_keyboard->get_handle()},
                .m_endpoint_number = kb_ctx->m_ep_in_interrupt,
                .m_direction       = USB::Direction::IN,
                .m_length          = kb_ctx->m_ep_in_interrupt_max_packet_size,
                .m_data_buffer     = keyboard_report.get()
            };
            USB::TransferResponse tr;
            IORequest             io_req = {.m_in_data = &dtr, .m_out_data = &tr};
            auto                  st =
                dm->control_device(kb_ctx->m_keyboard->bus_device()->get_handle(), io_req).get();
            if (st != IORequestStatus::HANDLED || tr.m_residual_bytes != 0) {
                // A host controller error occurred, or a short packet was detected -> Skip the
                // frame
                WARN("Data request failed: {}", st.to_string())
                CriticalSection<SpinlockIRQSafe> _(kb_ctx->m_lock);
                if (!kb_ctx->m_run_polling_thread) break;
                continue;
            }
            DEBUG("{}",
                  String::join(", ",
                               keyboard_report.get(),
                               kb_ctx->m_ep_in_interrupt_max_packet_size))
            make_hid_keyboard_frame(kb_ctx->m_key_code_input_report,
                                    keyboard_report.get(),
                                    kb_ctx->m_ep_in_interrupt_max_packet_size,
                                    kb_ctx->m_uses_report_ID)
                .and_then<int>([&kb_ctx](const HIDKeyboardFrame& frame) -> auto {
                    kb_ctx->publish_key_events(frame);
                    return 0;
                });

            CriticalSection<SpinlockIRQSafe> _(kb_ctx->m_lock);
            if (!kb_ctx->m_run_polling_thread) break;
        };
        return 0;
    }

    const USB::USBDeviceID HIDKeyboardDriver::ID_HID_KEYBOARD_BOOT(USB::ClassCode::HID,
                                                                   USB::HIDSubClass::BOOT_INTERFACE,
                                                                   USB::HIDProtocol::KEYBOARD);

    const USB::USBDeviceID HIDKeyboardDriver::ID_HID_KEYBOARD_ANY(USB::ClassCode::HID,
                                                                  USB::HIDSubClass::NONE,
                                                                  USB::HIDProtocol::NONE);

    HIDKeyboardDriver::HIDKeyboardDriver()
        : m_bind_mutex(Ember::HANDLE_NONE, "HID-KB Mutex") {

          };

    auto HIDKeyboardDriver::vendor() const -> String { return "Ewogijk"; }

    auto HIDKeyboardDriver::version() const -> Version {
        return {.major = 1, .minor = 0, .patch = 0, .pre_release = ""};
    }

    auto HIDKeyboardDriver::can_bind(const DeviceID* device_ID) -> bool {
        return ID_HID_KEYBOARD_BOOT.equals(device_ID) || ID_HID_KEYBOARD_ANY.equals(device_ID);
    }

    auto HIDKeyboardDriver::bind(const SharedPointer<Device>& device) -> bool {
        CriticalSection<CPU::Mutex> _(m_bind_mutex);
        auto                        keyboard = SharedPointer<USB::FunctionDevice>(device);
        auto* dm = System::instance().get_module<DeviceModule>(ModuleSelector::DEVICE);

        {
            CriticalSection<SpinlockIRQSafe> _(m_lock);
            if (m_keyboard_contexts.find(device->get_handle()) != m_keyboard_contexts.end()) {
                WARN("{}: HID Keyboard is already bound", device->get_unique_name())
                return false;
            }
        }

        USB::HIDReports reports{};
        U8              interface_number                = 0;
        bool            interface_found                 = false;
        U8              ep_in_interrupt                 = 0;
        U16             ep_in_interrupt_max_packet_size = 0;
        for (auto& iface : keyboard->interfaces()) {
            for (auto& alt : iface.m_alternate_settings) {
                USB::USBDeviceID ifID(alt.m_interface_class,
                                      alt.m_interface_subclass,
                                      alt.m_interface_protocol);
                if (!(ifID.equals(&ID_HID_KEYBOARD_BOOT) || ifID.equals(&ID_HID_KEYBOARD_ANY)))
                    continue;

                // Find the Interrupt IN endpoint
                for (auto& ep : alt.m_endpoints) {
                    if (ep.m_direction == USB::Direction::IN
                        && ep.m_transfer_type == USB::TransferType::INTERRUPT) {
                        ep_in_interrupt                 = ep.m_endpoint_number;
                        ep_in_interrupt_max_packet_size = ep.m_max_packet_size;
                    }
                }
                if (ep_in_interrupt == 0) {
                    WARN("{}: Device declares no IN INTERRUPT endpoint", device->get_unique_name())
                    return false;
                }
                if (ep_in_interrupt_max_packet_size == 0) {
                    WARN("{}: IN INTERRUPT endpoint declares", device->get_unique_name())
                    return false;
                }

                const USB::HIDDescriptor* hid_desc = nullptr;
                for (const auto& class_descriptor :
                     keyboard->configuration().m_descriptor_blob.descriptors(
                         alt.m_class_descriptors)) {
                    if (class_descriptor.type() != USB::HIDDescriptorType::HID) continue;

                    hid_desc = class_descriptor.as<USB::HIDDescriptor>();
                }
                if (!hid_desc) {
                    WARN("{}: No HID keyboard descriptor found", device->get_unique_name());
                    return false;
                }

                U16 report_length = hid_desc->report_descriptor_length();
                if (report_length == 0) {
                    WARN("{}: HID descriptor declares no Report descriptor", device->get_handle())
                    return false;
                }

                auto* report_desc = static_cast<U8*>(Memory::DMA::allocate_zeroed(report_length));
                USB::ControlTransferRequest get_report_desc = USB::ControlTransferRequest::of(
                    keyboard->get_handle(),
                    USB::RequestType::DIRECTION_DEVICE_TO_HOST | USB::RequestType::TYPE_STANDARD
                        | USB::RequestType::RECIPIENT_INTERFACE,
                    USB::StandardRequestCode::GET_DESCRIPTOR,
                    USB::HIDDescriptorType::REPORT << SHIFT_8 | 0,
                    iface.m_interface_number,
                    report_length,
                    report_desc);
                IORequest io_req{.m_in_data = &get_report_desc, .m_out_data = nullptr};
                auto      st = dm->control_device(device->bus_device()->get_handle(), io_req).get();
                if (st != IORequestStatus::HANDLED) {
                    Memory::DMA::free(report_desc);
                    WARN("{}: GET_DESCRIPTOR(REPORT) failed - {}",
                         device->get_unique_name(),
                         st.to_string());
                    return false;
                }
                reports = USB::HIDItemParser(report_desc, report_length).parse_hid_reports();
                Memory::DMA::free(report_desc);

                if (alt.m_setting_number > 0) {
                    // Alt setting zero
                    USB::ControlTransferRequest set_interface = USB::ControlTransferRequest::of(
                        keyboard->get_handle(),
                        USB::RequestType::DIRECTION_HOST_TO_DEVICE | USB::RequestType::TYPE_STANDARD
                            | USB::RequestType::RECIPIENT_INTERFACE,
                        USB::StandardRequestCode::SET_INTERFACE,
                        alt.m_setting_number,
                        iface.m_interface_number,
                        0,
                        nullptr);

                    io_req = {.m_in_data = &set_interface, .m_out_data = nullptr};
                    st     = dm->control_device(device->bus_device()->get_handle(), io_req).get();
                    if (st != IORequestStatus::HANDLED) {
                        WARN("{}: SET_INTERFACE() failed - {}",
                             device->get_unique_name(),
                             st.to_string());
                        return false;
                    }
                }
                interface_number = iface.m_interface_number;
                interface_found  = true;
            }
            if (interface_found) break;
        }
        if (!interface_found) {
            WARN("{}: No HID keyboard interface found", device->get_unique_name());
            return false;
        }

        USB::ControlTransferRequest set_protocol =
            USB::hid_build_set_protocol_request(keyboard,
                                                interface_number,
                                                USB::HIDProtocolMode::REPORT);
        IORequest io_req = {.m_in_data = &set_protocol, .m_out_data = nullptr};
        auto      st     = dm->control_device(device->bus_device()->get_handle(), io_req).get();
        if (st != IORequestStatus::HANDLED) {
            WARN("{}: SET_PROTOCOL(REPORT) failed - {}", device->get_unique_name(), st.to_string());
            return false;
        }
        USB::HIDReport key_code_input_report;
        USB::HIDReport led_output_report;
        for (const auto& report : reports.m_reports.values()) {
            if (report.m_tag.m_type == USB::HIDReportType::INPUT
                && report.m_usage.decode_usage_page() == USB::HIDUsagePage::GENERIC_DESKTOP
                && USB::HIDGenericDesktopPage(report.m_usage.usage())
                       == USB::HIDGenericDesktopPage::KEYBOARD) {
                key_code_input_report = report;
            }

            if (report.m_tag.m_type == USB::HIDReportType::OUTPUT
                && report.m_usage.decode_usage_page() == USB::HIDUsagePage::GENERIC_DESKTOP
                && USB::HIDGenericDesktopPage(report.m_usage.usage())
                       == USB::HIDGenericDesktopPage::KEYBOARD) {
                led_output_report = report;
            }
        }

        if (key_code_input_report.m_tag.m_type == USB::HIDReportType::NONE) {
            WARN("{}: Devices declares no KEYBOARD INPUT report", device->get_unique_name())
            return false;
        }

        USB::ControlTransferRequest set_idle =
            USB::hid_build_set_idle_request(keyboard,
                                            interface_number,
                                            key_code_input_report.m_tag.m_report_ID,
                                            0);
        io_req = {.m_in_data = &set_idle, .m_out_data = nullptr};
        st     = dm->control_device(device->bus_device()->get_handle(), io_req).get();
        if (st != IORequestStatus::HANDLED) {
            WARN("{}: SET_IDLE({}) failed - {}",
                 device->get_unique_name(),
                 key_code_input_report.m_tag.m_report_ID,
                 st.to_string());
            return false;
        }
        SharedPointer<HIDKeyboardContext> kb_ctx(new HIDKeyboardContext{
            .m_keyboard                        = keyboard,
            .m_uses_report_ID                  = reports.m_uses_report_IDs,
            .m_key_code_input_report           = key_code_input_report,
            .m_ep_in_interrupt                 = ep_in_interrupt,
            .m_ep_in_interrupt_max_packet_size = ep_in_interrupt_max_packet_size,
            .m_led_output_report               = led_output_report,
            .m_this_addr           = int_to_string(reinterpret_cast<uintptr_t>(this), Radix::HEX),
            .m_device_handle       = int_to_string(keyboard->get_handle(), Radix::DECIMAL),
            .m_argv                = {},
            .m_tsp                 = {},
            .m_run_polling_thread  = true,
            .m_lock                = SpinlockIRQSafe(),
            .m_thread_handle       = Ember::HANDLE_NONE,
            .m_last_keyboard_frame = HIDKeyboardFrame{},
            .m_last_pressed        = Ember::VirtualKey::NONE});

        // NOLINTBEGIN
        kb_ctx->m_argv[0] = const_cast<char*>(kb_ctx->m_this_addr.to_cstr());
        kb_ctx->m_argv[1] = const_cast<char*>(kb_ctx->m_device_handle.to_cstr());
        // NOLINTEND
        kb_ctx->m_argv[2]  = nullptr;
        kb_ctx->m_tsp.argc = 2;
        kb_ctx->m_tsp.argv = kb_ctx->m_argv;
        kb_ctx->m_tsp.main = &poll_keyboard;

        {
            CriticalSection<SpinlockIRQSafe> _(m_lock);
            m_keyboard_contexts.put(kb_ctx->m_keyboard->get_handle(), kb_ctx);
        }
        auto* cpu_module = System::instance().get_module<CPU::CPUModule>(ModuleSelector::CPU);
        kb_ctx->m_thread_handle = cpu_module->schedule_new_thread(
            String::format("HID-KB#{}", device->get_handle()),
            &kb_ctx->m_tsp,
            Memory::get_base_page_table_address(),
            CPU::SchedulingPolicy::LOW_LATENCY,
            CPU::Stack{.stack_bottom = nullptr, .stack_top = 0x0, .stack_size = 0x0});

        if (kb_ctx->m_thread_handle == Ember::HANDLE_NONE) {
            WARN("{}: Failed to schedule polling thread", device->get_unique_name())
            CriticalSection<SpinlockIRQSafe> _(m_lock);
            m_keyboard_contexts.remove(device->get_handle());
            return false;
        }

        return true;
    }

    void HIDKeyboardDriver::unbind(const SharedPointer<Device>& device) {
        CriticalSection<CPU::Mutex>       _(m_bind_mutex);
        SharedPointer<HIDKeyboardContext> kb_ctx;
        {
            CriticalSection<SpinlockIRQSafe> _(m_lock);
            auto kb_ctx_it = m_keyboard_contexts.find(device->get_handle());
            if (kb_ctx_it == m_keyboard_contexts.end()) {
                WARN("{}: Unknown device, cannot unbind", device->get_unique_name())
                return;
            }
            kb_ctx = *kb_ctx_it->value;
        }
        auto* cpu_module = System::instance().get_module<CPU::CPUModule>(ModuleSelector::CPU);
        {
            CriticalSection<SpinlockIRQSafe> _(kb_ctx->m_lock);
            kb_ctx->m_run_polling_thread = false;
        }
        cpu_module->sync_with_thread_stop(kb_ctx->m_thread_handle);
        CriticalSection<SpinlockIRQSafe> _(m_lock);
        m_keyboard_contexts.remove(device->get_handle());
    }

    auto HIDKeyboardDriver::handle_request(const SharedPointer<Device>& device, IORequest request)
        -> CPU::Future<IORequestStatus> {
        SILENCE_UNUSED(device)
        SILENCE_UNUSED(request)
        return CPU::Promise<IORequestStatus>::make_completed_future(IORequestStatus::UNSUPPORTED);
    }
} // namespace Rune::Device