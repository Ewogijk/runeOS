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
#include <KRE/Utility.h>

#include <Device/PCI.h>

#include <Device/ACPI/ACPI.h>

namespace Rune::Device {
    const SharedPointer<Logger> LOGGER =
        LogContext::instance().get_logger("Device.DeviceSubsystem");

    const String DeviceModule::ROOT_DEVICE_NAME = "ACPI";

    void DeviceModule::setup_device_driver_registry() {
        auto acpi_driver = SharedPointer<Driver>(new ACPIDriver(_driver_handle_counter.acquire()));
        _device_driver_registry[acpi_driver->get_target_device()] = acpi_driver;
        auto ps2keyboard_driver =
            SharedPointer<Driver>(new PS2Keyboard(_driver_handle_counter.acquire()));
        _device_driver_registry[ps2keyboard_driver->get_target_device()] = ps2keyboard_driver;
    }

    bool DeviceModule::configure_root_device() {
        auto maybe_root_device_driver = _device_driver_registry.find(ROOT_DEVICE_NAME);
        if (maybe_root_device_driver == _device_driver_registry.end()) {
            LOGGER->error("The root device driver is missing.");
            return false;
        }
        auto root_device_driver = *maybe_root_device_driver->value;
        if (!root_device_driver->start(nullptr)) {
            LOGGER->error("Failed to start the root device driver.");
            return false;
        }

        auto       request  = ACPIREQUEST::GET_ACPI_INFO;
        IOResponse response = root_device_driver->handle_request(&request);
        if (response.m_status != IORequestStatus::HANDLED) {
            LOGGER->error("Failed to configure the root device.");
            return false;
        }
        auto* acpi_info              = reinterpret_cast<ACPIInfo*>(response.m_data);
        _root_device                 = Device(_device_handle_counter.acquire(), ACPIDriver::ACPI);
        _root_device.m_oem           = acpi_info->m_oem;
        _root_device.m_revision      = acpi_info->m_revision;
        _root_device.m_driver_handle = root_device_driver->get_handle();
        _root_device.m_is_bus_device = true;
        root_device_driver->set_operated_device(_root_device.get_handle());
        _device_registry.put(_root_device.get_handle(), _root_device);
        return true;
    }

    DeviceModule::DeviceModule()
        : _ahci_driver(nullptr),
          _root_device(Resource<DeviceHandle>::HANDLE_NONE, "") {}

    auto DeviceModule::get_name() const -> String { return "Device"; }

    auto DeviceModule::load(const BootInfo& boot_info) -> bool {
        SILENCE_UNUSED(boot_info)

        setup_device_driver_registry();

        if (!configure_root_device()) return false;

        Function<void(DeviceHandle, Device&, void*)> device_mapper =
            [this](DeviceHandle bus_device, Device& discovered_device, void* driver_ctx) {
                auto maybe_driver = _device_driver_registry.find(discovered_device.get_name());
                if (maybe_driver == _device_driver_registry.end()) {
                    LOGGER->warn(R"(Missing device driver: {})",
                                 discovered_device.get_unique_name());
                    return;
                }

                auto driver = *maybe_driver->value;
                if (driver->is_operating_device()) {
                    LOGGER->warn(R"(Device driver "{}" is already operating device {}.)",
                                 driver->get_unique_name(),
                                 driver->get_operated_device());
                    return;
                }

                if (!driver->start(driver_ctx)) {
                    LOGGER->warn(R"(Device driver start failure: {})", driver->get_unique_name());
                    return;
                }

                driver->set_operated_device(discovered_device.get_handle());
                discovered_device.m_driver_handle = driver->get_handle();

                _device_registry.put(discovered_device.get_handle(), discovered_device);
                _device_registry.find(bus_device)
                    ->value->m_child_devices.add_back(discovered_device.get_handle());
            };

        _device_driver_registry[_root_device.get_name()]->discover_devices(device_mapper,
                                                                           _device_handle_counter);

        PCI::discover_devices(*_ahci_driver);
        return true;
    }

    void DeviceModule::set_ahci_driver(UniquePointer<AHCIDriver> ahci_driver) {
        _ahci_driver = move(ahci_driver);
    }

    auto DeviceModule::get_ahci_driver() -> AHCIDriver* { return _ahci_driver.get(); }

    auto DeviceModule::get_keyboard() -> VirtualKeyboard* {
        return static_cast<VirtualKeyboard*>(
            _device_driver_registry.find(PS2Keyboard::PS2_KEYBOARD)->value->get());
    }
} // namespace Rune::Device