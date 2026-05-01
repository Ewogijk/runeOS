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

#ifndef RUNEOS_KEYBOARD_H
#define RUNEOS_KEYBOARD_H

#include <Ember/Ember.h>

#include <KRE/Stream.h>

#include <Device/Device.h>

namespace Rune::Device {
    /**
     * @brief The virtual keyboard maps physical keyboard scancodes to virtual keycodes and provides
     * them to the system as a stream.
     */
    class VirtualKeyboard : public TextStream, public Driver {
      public:
        VirtualKeyboard(DriverHandle handle, const String& name);

        // ====================================================================================== //
        // TextStream API
        // ====================================================================================== //

        auto is_read_supported() -> bool override;

        /**
         * @brief Read a single virtual key press from the buffer.
         * @return NoneKeyCode: The buffer is empty, Else: A virtual key code of the oldest pressed
         * key.
         */
        auto read() -> int override = 0;

        auto is_write_supported() -> bool override;

        auto write(U8 value) -> bool override;

        /**
         * @brief Flush a virtual keys in the buffer.
         */
        void flush() override = 0;

        void close() override;

        auto is_ansi_supported() -> bool override;

        // ====================================================================================== //
        // Driver API
        // ====================================================================================== //

        [[nodiscard]] auto get_target_device_ID() const -> const DeviceID* override = 0;

        auto start(DeviceHandle dev_handle, void* context) -> bool override = 0;

        auto stop(DeviceHandle dev_handle) -> bool override = 0;

        auto handle_request(DeviceHandle dev_handle, IORequest request)
            -> IORequestStatus override = 0;

        void discover_devices(DeviceHandle                 bus_device,
                              const DeviceMapper&          device_mapper,
                              HandleCounter<DeviceHandle>& dev_handle_counter) override = 0;
    };
} // namespace Rune::Device

#endif // RUNEOS_KEYBOARD_H
