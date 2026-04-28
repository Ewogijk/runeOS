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

#include <Device/AHCI/AHCI.h>

namespace Rune::Device {
    const SharedPointer<Logger> LOGGER = LogContext::instance().get_logger("Device.DeviceModule");

    void DeviceModule::setup_device_driver_registry() {
        auto mem_module =
            System::instance().get_module<Memory::MemoryModule>(ModuleSelector::MEMORY);
        auto cpu_module = System::instance().get_module<CPU::CPUModule>(ModuleSelector::CPU);

        auto acpi_driver = SharedPointer<Driver>(new ACPIDriver(m_driver_handle_counter.acquire()));
        m_device_driver_registry[acpi_driver->get_handle()] = acpi_driver;

        auto ps2keyboard_driver =
            SharedPointer<Driver>(new PS2Keyboard(m_driver_handle_counter.acquire()));
        m_device_driver_registry[ps2keyboard_driver->get_handle()] = ps2keyboard_driver;

        auto pci_driver = SharedPointer<Driver>(new PCIDriver(m_driver_handle_counter.acquire()));
        m_device_driver_registry[pci_driver->get_handle()] = pci_driver;

        auto ahci_driver = SharedPointer<Driver>(new AHCIDriver(m_driver_handle_counter.acquire(),
                                                                mem_module->get_heap(),
                                                                cpu_module->get_system_timer()));
        m_device_driver_registry[ahci_driver->get_handle()] = ahci_driver;
    }

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
        auto*                 acpi_info = reinterpret_cast<ACPIInfo*>(response.m_data);
        SharedPointer<Device> root_device(new BasicDevice(m_device_handle_counter.acquire(),
                                                          ACPIDriver::ID_ACPI.get_string_ID(),
                                                          acpi_info->m_oem,
                                                          int_to_string(acpi_info->m_revision, 10),
                                                          "",
                                                          DeviceType::GENERIC,
                                                          ACPIDriver::ID_ACPI));
        root_device->set_driver_handle(root_device_driver->get_handle());
        root_device_driver->add_operated_device(root_device->get_handle());
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

            if (!driver->start(driver_ctx)) {
                LOGGER->warn(R"(Device driver start failure: {})", driver->get_unique_name());
                return;
            }

            driver->add_operated_device(discovered_device->get_handle());
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

    auto DeviceModule::get_ahci_driver() -> AHCIDriver* {
        return static_cast<AHCIDriver*>(search_device_driver(&AHCIDriver::ID_AHCI).get());
    }

    auto DeviceModule::get_keyboard() -> VirtualKeyboard* {
        return static_cast<VirtualKeyboard*>(
            search_device_driver(&PS2Keyboard::ID_PS2_KEYBOARD).get());
    }
} // namespace Rune::Device