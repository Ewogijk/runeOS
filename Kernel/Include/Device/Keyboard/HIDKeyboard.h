
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

#ifndef RUNEOS_HIDKEYBOARD_H
#define RUNEOS_HIDKEYBOARD_H

#include <Ember/AppBits.h>

#include <KRE/CPU.h>

#include <CPU/Threading/Mutex.h>

#include <Device/Device.h>
#include <Device/USB/HID/HID.h>
#include <Device/USB/USB.h>

namespace Rune::Device {

    /// @brief Keyboard state, currently pressed modifiers and keys.
    struct HIDKeyboardFrame {
        static constexpr U32     KEYCODE_COUNT = 6;
        U8                       m_modifiers   = 0;
        Array<U8, KEYCODE_COUNT> m_key_codes;
    };

    struct HIDKeyboardContext {
        SharedPointer<USB::FunctionDevice> m_keyboard;
        bool                               m_uses_report_ID = false;
        USB::HIDReport                     m_key_code_input_report{};
        U8                                 m_ep_in_interrupt                 = 0;
        U16                                m_ep_in_interrupt_max_packet_size = 0;
        USB::HIDReport                     m_led_output_report{};

        bool            m_run_polling_thread = false;
        SpinlockIRQSafe m_lock;
        Ember::Handle   m_thread_handle;

        HIDKeyboardFrame  m_last_keyboard_frame{};
        Ember::VirtualKey m_last_pressed = Ember::VirtualKey::NONE;

        void publish_key_events(const HIDKeyboardFrame& new_frame);
    };

    /// @brief
    class HIDKeyboardDriver : public Driver {
        HashMap<Ember::Handle, SharedPointer<HIDKeyboardContext>> m_keyboard_contexts;
        /// @brief Guards m_keyboard_contexts access.
        SpinlockIRQSafe m_lock;
        /// @brief Synchronizes bind/unbind calls to prevent interleaving calls.
        CPU::Mutex m_bind_mutex;

        friend auto poll_keyboard(Ember::ThreadLaunchPacket* tsp) -> int;

      public:
        static const USB::USBDeviceID ID_HID_KEYBOARD_BOOT;
        static const USB::USBDeviceID ID_HID_KEYBOARD_ANY;

        HIDKeyboardDriver();

        [[nodiscard]] auto vendor() const -> String override;
        [[nodiscard]] auto version() const -> Version override;
        auto               can_bind(const DeviceID* device_ID) -> bool override;
        auto               bind(const SharedPointer<Device>& device) -> bool override;
        void               unbind(const SharedPointer<Device>& device) override;
        auto               handle_request(const SharedPointer<Device>& device, IORequest request)
            -> CPU::Future<IORequestStatus> override;
    };

} // namespace Rune::Device

#endif // RUNEOS_HIDKEYBOARD_H
