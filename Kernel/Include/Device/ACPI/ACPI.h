
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

#ifndef RUNEOS_ACPI_H
#define RUNEOS_ACPI_H

#include <Device/Device.h>

namespace Rune::Device {

#define ACPI_REQUESTS(X)                                                                           \
    X(ACPIREQUEST, GET_ACPI_INFO, 0x1)                                                             \
    X(ACPIREQUEST, SHUTDOWN, 0x2)                                                                  \
    X(ACPIREQUEST, REBOOT, 0x3)

    /// @brief All supported ACPI requests.
    ///
    /// - GET_ACPI_INFO: Get the OEM and Revision of ACPI.
    /// - SHUTDOWN: Shut the system down.
    /// - REBOOT: Reboot the system.
    DECLARE_ENUM(ACPIRequest, ACPI_REQUESTS, 0x0) // NOLINT

    struct ACPIInfo {
        String m_oem;
        U32    m_revision;
    };

    class ACPIDriver : public Driver {
        bool _acpi_initialized = false;

      public:
        const static BasicDeviceID ID_ACPI;

        ACPIDriver() = default;

        [[nodiscard]] auto vendor() const -> String override;
        [[nodiscard]] auto version() const -> Version override;
        [[nodiscard]] auto target_device_ID() const -> const DeviceID* override;
        auto               accept_device(const SharedPointer<Device>& device) -> bool override;
        void               remove_device(const SharedPointer<Device>& device) override;
        auto               handle_request(const SharedPointer<Device>& device, IORequest request)
            -> IORequestStatus override;
    };
} // namespace Rune::Device

#endif // RUNEOS_ACPI_H
