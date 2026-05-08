
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

#ifndef RUNEOS_PORTDRIVER_H
#define RUNEOS_PORTDRIVER_H

#include <Device/MassStorage/AHCI/PortEngine.h>
#include <Device/MassStorage/MassStorage.h>

namespace Rune::Device {
    class PortDriver : public Driver {
      public:
        PortDriver() = default;

        [[nodiscard]] auto vendor() const -> String override;
        [[nodiscard]] auto version() const -> Version override;
        [[nodiscard]] auto target_device_ID() const -> const DeviceID* override;
        auto               accept_device(const SharedPointer<Device>& device) -> bool override;
        void               remove_device(const SharedPointer<Device>& device) override;
        auto               handle_request(const SharedPointer<Device>& device, IORequest request)
            -> IORequestStatus override;
    };
} // namespace Rune::Device

#endif // RUNEOS_PORTDRIVER_H
