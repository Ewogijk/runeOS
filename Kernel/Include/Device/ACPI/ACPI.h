
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
    X(ACPIREQUEST, SHUTDOWN, 0x2)

    /// @brief TODO write doc
    ///
    /// - GET_ACPI_INFO: A_DESCRIPTION
    /// - SHUTDOWN: A_DESCRIPTION
    DECLARE_ENUM(ACPIREQUEST, ACPI_REQUESTS, 0x0) // NOLINT

    struct ACPIInfo {
        String m_oem;
        U32    m_revision;
    };

    class ACPIDriver : public BusDriver {
        bool _acpi_initialized = false;

        void handle_get_acpi_info_request(IOResponse& response);
        void handle_shutdown_request(IOResponse& response);

      public:
        ACPIDriver(DriverHandle handle, const String& name);

        auto start(void* context) -> bool override;
        auto stop() -> bool override;
        auto handle_request(IORequest request) -> IOResponse override;
        auto discover_devices() -> LinkedList<Device> override;
    };
} // namespace Rune::Device

#endif // RUNEOS_ACPI_H
