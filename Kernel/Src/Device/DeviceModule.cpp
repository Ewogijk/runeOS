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

#include <Device/DeviceModule.h>

#include <KRE/System/System.h>

#include <Device/PCI.h>

#include <Device/ACPI/ACPI.h>

namespace Rune::Device {
    const SharedPointer<Logger> LOGGER =
        LogContext::instance().get_logger("Device.DeviceSubsystem");

    bool DeviceModule::configure_root_device() {
        if (!_acpi_driver->start(nullptr)) {
            LOGGER->error("Failed to start ACPI driver.");
            return false;
        }
        auto       request  = ACPIREQUEST::GET_ACPI_INFO;
        IOResponse response = _acpi_driver->handle_request(&request);
        if (response.m_status != IORequestStatus::HANDLED) {
            LOGGER->error("Failed to configure root device.");
            return false;
        }
        auto* acpi_info              = reinterpret_cast<ACPIInfo*>(response.m_data);
        _root_device                 = Device(1, "ACPI");
        _root_device.m_oem           = acpi_info->m_oem;
        _root_device.m_revision      = acpi_info->m_revision;
        _root_device.m_driver_handle = _acpi_driver->get_handle();
        return true;
    }

    DeviceModule::DeviceModule()
        : _ahci_driver(nullptr),
          _keyboard(new PS2Keyboard()),
          _acpi_driver(new ACPIDriver(1, "ACPI")),
          _root_device(Resource<DeviceHandle>::HANDLE_NONE, "") {}

    auto DeviceModule::get_name() const -> String { return "Device"; }

    auto DeviceModule::load(const BootInfo& boot_info) -> bool {
        SILENCE_UNUSED(boot_info)
        if (!configure_root_device()) return false;

        PCI::discover_devices(*_ahci_driver);

        _keyboard->start();
        return true;
    }

    void DeviceModule::set_ahci_driver(UniquePointer<AHCIDriver> ahci_driver) {
        _ahci_driver = move(ahci_driver);
    }

    auto DeviceModule::get_ahci_driver() -> AHCIDriver* { return _ahci_driver.get(); }

    auto DeviceModule::get_keyboard() -> SharedPointer<VirtualKeyboard> { return _keyboard; }
} // namespace Rune::Device