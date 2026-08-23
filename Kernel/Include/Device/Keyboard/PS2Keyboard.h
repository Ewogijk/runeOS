/*
 *  Copyright 2025 Ewogijk
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef RUNEOS_PS2KEYBOARD_H
#define RUNEOS_PS2KEYBOARD_H

#include <CPU/Interrupt/IRQ.h>

#include <Device/Device.h>

namespace Rune::Device {
    /**
     * A PS2 keyboard driver converting the scancode set 1 to key events.
     *
     * Decoded key events are published to the kernel wide key event buffer.
     */
    class PS2Keyboard : public Driver {
        static constexpr U8 EXTENDED_BYTE = 0xE0;
        static constexpr U8 PAUSE_BYTE    = 0xE1;
        static constexpr U8 DATA_REGISTER = 0x60;

        /// @brief Number of scan codes that follow the pause byte in a pause key sequence.
        static constexpr U8 PAUSE_SEQUENCE_LENGTH = 2;

        /// @brief True: The next scan code must be decoded with the E0 scan code table.
        bool _wait_key_e0{false};

        /// @brief Number of scan codes of the pause key sequence that are left to be consumed.
        U8 _pause_bytes_left{0};

        /// @brief True: The currently decoded pause key sequence is a key press.
        bool _pause_key_down{false};

        /// @brief Currently pressed modifier keys, uses the HID keyboard modifier bit layout.
        U8 _modifiers{0};

        CPU::FastInterruptHandler _irq_handler;

        /// @brief Decode a scan code and publish a key event to the key event buffer.
        /// @param scan_code A scan code set 1 byte as read from the data register.
        void handle_scan_code(U8 scan_code);

      public:
        static const BasicDeviceID ID_PS2_KEYBOARD;

        PS2Keyboard();

        [[nodiscard]] auto vendor() const -> String override;
        [[nodiscard]] auto version() const -> Version override;
        auto               can_bind(const DeviceID* device_ID) -> bool override;
        auto               bind(const SharedPointer<Device>& device) -> bool override;
        void               unbind(const SharedPointer<Device>& device) override;
        auto               handle_request(const SharedPointer<Device>& device, IORequest request)
            -> CPU::Future<IORequestStatus> override;
    };
} // namespace Rune::Device

#endif // RUNEOS_PS2KEYBOARD_H
