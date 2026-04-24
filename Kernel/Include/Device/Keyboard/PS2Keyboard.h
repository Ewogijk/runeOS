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

#include <Device/Keyboard/Keyboard.h>

#include <KRE/Collections/Array.h>

#include <CPU/Interrupt/IRQ.h>

namespace Rune::Device {
    /**
     * A PS2 keyboard driver converting the scancode set 2 to virtual keys.
     */
    class PS2Keyboard : public VirtualKeyboard {
        static constexpr size_t RING_BUFFER_SIZE = 256;
        static constexpr U8     EXTENDED_BYTE    = 0xE0;
        static constexpr U8     DATA_REGISTER    = 0x60;

        Array<U16, RING_BUFFER_SIZE> _key_code_cache;
        U8                           _start{0};
        U8                           _end{0};

        bool _wait_key_e0{false};

        CPU::IRQHandler _irq_handler;

      public:
        /// @brief Name of the PS2 Keyboard device.
        static const String PS2_KEYBOARD;

        PS2Keyboard(DriverHandle handle);

        auto read() -> int override;

        void flush() override;

        auto get_target_device_ID() -> SharedPointer<DeviceID> override;

        auto start(void* context) -> bool override;

        auto stop() -> bool override;

        auto handle_request(IORequest request) -> IOResponse override;

        void discover_devices(const DeviceMapper&          device_mapper,
                              HandleCounter<DeviceHandle>& dev_handle_counter) override;
    };
} // namespace Rune::Device

#endif // RUNEOS_PS2KEYBOARD_H
