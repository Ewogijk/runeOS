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

    void ACPIDriver::handle_get_acpi_info_request(IOResponse& response) {
        ACPI_TABLE_RSDP* rsdp = reinterpret_cast<ACPI_TABLE_RSDP*>(
            Memory::physical_to_virtual_address(System::instance().get_boot_info().rsdp_addr));
        response.m_status = IORequestStatus::HANDLED;
        response.m_data   = new ACPIInfo{.m_oem      = String(rsdp->OemId, ACPI_OEM_ID_SIZE),
                                         .m_revision = rsdp->Revision};
    }

    void ACPIDriver::handle_shutdown_request(IOResponse& response) {
        ACPI_STATUS status = AcpiEnterSleepStatePrep(ACPI_STATE_S5);
        if (ACPI_FAILURE(status)) {
            LOGGER->error("AcpiEnterSleepStatePrep(S5) failed. Status={}", status);
            response.m_status = IORequestStatus::FAILED;
            return;
        }

        status = AcpiEnterSleepState(ACPI_STATE_S5);
        if (ACPI_FAILURE(status)) {
            LOGGER->error("AcpiEnterSleepState(S5) failed. Status={}", status);
            response.m_status = IORequestStatus::FAILED;
            return;
        }

        response.m_status = IORequestStatus::HANDLED;
        response.m_data   = nullptr;
    }

    const String ACPIDriver::ACPI = "ACPI";

    ACPIDriver::ACPIDriver(DriverHandle handle) : Driver(handle, ACPI) {}

    auto ACPIDriver::get_target_device_ID() -> SharedPointer<DeviceID> {
        return SharedPointer<DeviceID>(new StringDeviceID(ACPI));
    }

    auto ACPIDriver::start(void* context) -> bool {
        SILENCE_UNUSED(context)
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
        return true;
    }

    auto ACPIDriver::stop() -> bool { return true; }

    auto ACPIDriver::handle_request(IORequest request) -> IOResponse {
        if (!_acpi_initialized)
            return IOResponse{.m_status = IORequestStatus::FAILED, .m_data = nullptr};
        auto       req = reinterpret_cast<ACPIREQUEST*>(request);
        IOResponse io_response;

        switch (*req) {
            case ACPIREQUEST::GET_ACPI_INFO: handle_get_acpi_info_request(io_response); break;
            case ACPIREQUEST::SHUTDOWN:      handle_shutdown_request(io_response); break;
            default:                         io_response.m_status = IORequestStatus::BAD_ARGUMENT;
        }

        return io_response;
    }

    void ACPIDriver::discover_devices(const DeviceMapper&          device_mapper,
                                      HandleCounter<DeviceHandle>& dev_handle_counter) {
        Device ps2_keyboard     = Device(dev_handle_counter.acquire(), PS2Keyboard::PS2_KEYBOARD);
        ps2_keyboard.m_oem      = "";
        ps2_keyboard.m_revision = 0;
        ps2_keyboard.m_is_bus_device = false;
        ps2_keyboard.m_device_ID =
            SharedPointer<DeviceID>(new StringDeviceID(PS2Keyboard::PS2_KEYBOARD));
        device_mapper(get_handle(), ps2_keyboard, nullptr);

        Device pci          = Device(dev_handle_counter.acquire(), PCIDriver::PCI);
        pci.m_oem           = "";
        pci.m_revision      = 0;
        pci.m_is_bus_device = true;
        pci.m_device_ID     = SharedPointer<DeviceID>(new StringDeviceID(PCIDriver::PCI));
        device_mapper(get_handle(), pci, nullptr);
    }

} // namespace Rune::Device