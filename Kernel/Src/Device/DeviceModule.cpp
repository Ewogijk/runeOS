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

#include <Device/PCI/PCI.h>

#include <Device/ACPI/ACPI.h>

#include <Device/Keyboard/PS2Keyboard.h>

namespace Rune::Device {
    const SharedPointer<Logger> LOGGER = LogContext::instance().get_logger("Device.DeviceModule");

    auto DeviceModule::search_device_driver(const DeviceID* device_ID) -> SharedPointer<Driver> {
        for (auto maybe_device_driver : m_device_driver_registry) {
            auto device_driver = *maybe_device_driver.value;
            if (device_driver->get_target_device_ID()->equals(device_ID)) return device_driver;
        }
        return SharedPointer<Driver>();
    }

    bool DeviceModule::configure_root_device() {
        auto root_device_driver = search_device_driver(&ACPIDriver::ID_ACPI);
        if (!root_device_driver) {
            LOGGER->error("The root device driver is missing.");
            return false;
        }
        auto root_device_handle = m_device_handle_counter.acquire();
        if (!root_device_driver->start(root_device_handle, nullptr)) {
            LOGGER->error("Failed to start the root device driver.");
            return false;
        }

        IORequest request;
        auto      acpi_req = ACPIREQUEST::GET_ACPI_INFO;
        ACPIInfo  acpi_info;
        request.m_in_buffer  = &acpi_req;
        request.m_out_buffer = &acpi_info;
        auto req_status      = root_device_driver->handle_request(root_device_handle, request);
        if (req_status != IORequestStatus::HANDLED) {
            LOGGER->error("Failed to configure the root device.");
            return false;
        }
        SharedPointer<Device> root_device(new BasicDevice(root_device_handle,
                                                          ACPIDriver::ID_ACPI.get_string_ID(),
                                                          acpi_info.m_oem,
                                                          int_to_string(acpi_info.m_revision, 10),
                                                          "",
                                                          DeviceType::GENERIC,
                                                          ACPIDriver::ID_ACPI));
        root_device->set_driver_handle(root_device_driver->get_handle());
        m_device_registry.put(root_device->get_handle(), root_device);
        m_root_device_handle = root_device->get_handle();
        return true;
    }

    void DeviceModule::build_device_tree(SharedPointer<Device> device,
                                         const DeviceMapper&   device_mapper) {
        if (device->get_driver_handle() == Resource<DriverHandle>::HANDLE_NONE) return;

        auto maybe_driver = m_device_driver_registry.find(device->get_driver_handle());
        if (maybe_driver == m_device_driver_registry.end()) return;

        auto driver = *maybe_driver->value;
        if (!driver->get_operated_devices().contains(device->get_handle())) {
            LOGGER->error("Wrong device mapping detected. Driver {} does not operate device {}.",
                          driver->get_unique_name(),
                          device->get_unique_name());
            return;
        }
        driver->discover_devices(device->get_handle(), device_mapper, m_device_handle_counter);

        for (auto child_device_handle : device->get_child_devices()) {
            auto maybe_child_device = m_device_registry.find(child_device_handle);
            if (maybe_child_device == m_device_registry.end()) continue;
            build_device_tree(*maybe_child_device->value, device_mapper);
        }
    }

    DeviceModule::DeviceModule() : m_root_device_handle(Resource<DeviceHandle>::HANDLE_NONE) {}

    auto DeviceModule::get_name() const -> String { return "Device"; }

    auto DeviceModule::load(const BootInfo& boot_info) -> bool {
        SILENCE_UNUSED(boot_info)

        setup_device_driver_registry();

        if (!configure_root_device()) return false;

        DeviceMapper device_mapper = [this](DeviceHandle bus_device,
                                            SharedPointer<Device> discovered_device,
                                            void* driver_ctx) {
            auto driver = search_device_driver(discovered_device->get_device_ID());
            if (!driver) {
                LOGGER->warn(R"(Missing device driver: {})", discovered_device->get_unique_name());
                return;
            }

            if (!driver->start(discovered_device->get_handle(), driver_ctx)) {
                LOGGER->warn(R"(Device driver start failure: {})", driver->get_unique_name());
                return;
            }

            discovered_device->set_driver_handle(driver->get_handle());
            m_device_registry.put(discovered_device->get_handle(), discovered_device);
            m_device_registry_by_type[discovered_device->get_device_type()].add_back(
                discovered_device);
            (*m_device_registry.find(bus_device)->value)
                ->get_child_devices()
                .add_back(discovered_device->get_handle());
        };

        auto root_device = m_device_registry.find(m_root_device_handle);
        if (root_device == m_device_registry.end()) {
            LOGGER->error("Failed to find root device.");
            return false;
        };
        build_device_tree(*root_device->value, device_mapper);
        return true;
    }

    // ========================================================================================== //
    // Device Driver API
    // ========================================================================================== //

    auto DeviceModule::get_device_driver_handle() -> DriverHandle {
        return m_driver_handle_counter.acquire();
    }

    auto DeviceModule::register_device_driver(SharedPointer<Driver> driver) -> bool {
        auto maybe_device_driver = m_device_driver_registry.find(driver->get_handle());
        if (maybe_device_driver != m_device_driver_registry.end()) return false;
        m_device_driver_registry.put(driver->get_handle(), driver);
        return true;
    }

    // ========================================================================================== //
    // Device API
    // ========================================================================================== //

    auto DeviceModule::get_keyboard() -> VirtualKeyboard* {
        return static_cast<VirtualKeyboard*>(
            search_device_driver(&PS2Keyboard::ID_PS2_KEYBOARD).get());
    }

    auto DeviceModule::control_device(DeviceHandle dev_handle, const IORequest& io_request)
        -> IORequestStatus {
        auto maybe_dev = m_device_registry.find(dev_handle);
        if (maybe_dev == m_device_registry.end()) return IORequestStatus::UNKNOWN_DEVICE;

        auto dev = *maybe_dev->value;
        if (dev->get_driver_handle() == Resource<DriverHandle>::HANDLE_NONE)
            return IORequestStatus::DEVICE_NOT_OPERATIONAL;

        auto maybe_driver = m_device_driver_registry.find(dev->get_driver_handle());
        if (maybe_driver == m_device_driver_registry.end()) return IORequestStatus::UNKNOWN_DRIVER;

        return (*maybe_driver->value)->handle_request(dev_handle, io_request);
    }

} // namespace Rune::Device