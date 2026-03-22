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

#include "KRE/Logging.h"
#include "KRE/Memory.h"

CLINK {
#include <Device/ACPI/ACPICA/acpi.h>
}

namespace Rune::Device {
    const SharedPointer<Logger> LOGGER = LogContext::instance().get_logger("Device.ACPI");

    auto acpi_start() -> bool {
        ACPI_STATUS status = AcpiInitializeSubsystem();
        if (ACPI_FAILURE(status)) {
            LOGGER->error("AcpiInitializeSubsystem failed. Status={}", status);
            return false;
        }

        status = AcpiInitializeTables(nullptr, 0, TRUE);
        if (ACPI_FAILURE(status)) {
            LOGGER->error("AcpiInitializeTables failed. Status={}", status);
            return false;
        }

        status = AcpiLoadTables();
        if (ACPI_FAILURE(status)) {
            LOGGER->error("AcpiLoadTables failed. Status={}", status);
            return false;
        }

        status = AcpiEnableSubsystem(ACPI_FULL_INITIALIZATION);
        if (ACPI_FAILURE(status)) {
            LOGGER->error("AcpiEnableSubsystem failed. Status={}", status);
            return false;
        }

        status = AcpiInitializeObjects(ACPI_FULL_INITIALIZATION);
        if (ACPI_FAILURE(status)) {
            LOGGER->error("AcpiInitializeObjects failed. Status={}", status);
            return false;
        }

        return true;
    }
} // namespace Rune::Device