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

#include <Device/ACPI/ACPI.h>

#include <KRE/Logging.h>
#include <KRE/Memory.h>
#include <KRE/System/System.h>

#include <Memory/Paging.h>

#include <Device/DeviceModule.h>
#include <Device/Keyboard/PS2Keyboard.h>
#include <Device/PCI/PCI.h>

CLINK {
#include <Device/ACPI/ACPICA/acpi.h>
}

namespace Rune::Device {
    const SharedPointer<Logger> LOGGER = LogContext::instance().get_logger("Device.ACPI");

    // ========================================================================================== //
    // ACPIDriver
    // ========================================================================================== //

    auto ACPIDriver::handle_get_acpi_info_request(IORequest& request) -> IORequestStatus {
        ACPI_TABLE_RSDP* rsdp = reinterpret_cast<ACPI_TABLE_RSDP*>(
            Memory::physical_to_virtual_address(System::instance().get_boot_info().rsdp_addr));
        auto* acpi_info       = reinterpret_cast<ACPIInfo*>(request.m_out_buffer);
        acpi_info->m_oem      = String(rsdp->OemId, ACPI_OEM_ID_SIZE);
        acpi_info->m_revision = rsdp->Revision;
        return IORequestStatus::HANDLED;
    }

    auto ACPIDriver::handle_shutdown_request(IORequest& request) -> IORequestStatus {
        ACPI_STATUS status = AcpiEnterSleepStatePrep(ACPI_STATE_S5);
        if (ACPI_FAILURE(status)) {
            LOGGER->error("AcpiEnterSleepStatePrep(S5) failed. Status={}", status);
            return IORequestStatus::FAILED;
        }

        status = AcpiEnterSleepState(ACPI_STATE_S5);
        if (ACPI_FAILURE(status)) {
            LOGGER->error("AcpiEnterSleepState(S5) failed. Status={}", status);
            return IORequestStatus::FAILED;
        }

        return IORequestStatus::HANDLED;
    }

    const BasicDeviceID ACPIDriver::ID_ACPI("ACPI");

    auto ACPIDriver::vendor() const -> String { return "Ewogjik"; };

    auto ACPIDriver::version() const -> Version {
        return {.major = 1, .minor = 0, .patch = 0};
    }

    auto ACPIDriver::target_device_ID() const -> const DeviceID* { return &ID_ACPI; }

    auto ACPIDriver::accept_device(const SharedPointer<Device>& device) -> bool {
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
        _acpi_initialized = true;

        auto* ds = System::instance().get_module<DeviceModule>(ModuleSelector::DEVICE);
        SharedPointer<Device> ps2_keyboard(
            new BasicDevice(ds->get_device_handle(),
                            PS2Keyboard::ID_PS2_KEYBOARD.get_string_ID(),
                            "",
                            "",
                            "",
                            DeviceType::KEYBOARD,
                            PS2Keyboard::ID_PS2_KEYBOARD));
        ds->register_device(device, ps2_keyboard);

        SharedPointer<Device> pci(new BasicDevice(ds->get_device_handle(),
                                                  PCIDriver::ID_PCI.get_string_ID(),
                                                  "",
                                                  "",
                                                  "",
                                                  DeviceType::GENERIC,
                                                  PCIDriver::ID_PCI));
        ds->register_device(device, pci);
        return true;
    }

    void ACPIDriver::remove_device(const SharedPointer<Device>& device) { SILENCE_UNUSED(device) }

    auto ACPIDriver::handle_request(const SharedPointer<Device>& device, IORequest request)
        -> IORequestStatus {
        if (!_acpi_initialized) return IORequestStatus::UNKNOWN_DEVICE;
        auto req = reinterpret_cast<ACPIRequest*>(request.m_in_buffer);

        switch (*req) {
            case ACPIRequest::GET_ACPI_INFO: return handle_get_acpi_info_request(request);
            case ACPIRequest::SHUTDOWN:      return handle_shutdown_request(request);
            default:                         return IORequestStatus::BAD_ARGUMENT;
        }
    }

} // namespace Rune::Device