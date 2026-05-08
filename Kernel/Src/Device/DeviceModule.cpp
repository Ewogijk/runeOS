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

    // ========================================================================================== //
    // Private Functions
    // ========================================================================================== //

    auto DeviceModule::find_device(const SharedPointer<Device>& current_device, Handle dev_handle)
        -> SharedPointer<Device> {
        if (dev_handle == Resource<Handle>::HANDLE_NONE) return {};
        if (dev_handle == current_device->get_handle()) return current_device;
        for (auto& child : current_device->child_devices()) {
            auto maybe_device = find_device(child, dev_handle);
            if (maybe_device) return maybe_device;
        }
        return {};
    }

    auto DeviceModule::find_device_driver(const DeviceID* device_ID) -> SharedPointer<Driver> {
        if (device_ID == nullptr) return {};
        for (auto& driver : m_driver_store)
            if (driver->target_device_ID()->equals(device_ID)) return driver;
        return {};
    }

    void DeviceModule::match_devices(LinkedList<SharedPointer<Device>>& out_devices,
                                     const SharedPointer<Device>&       current_device,
                                     const DeviceID*                    device_id) {
        if (current_device->device_ID()->equals(device_id)) out_devices.add_back(current_device);
        for (auto& child : current_device->child_devices())
            match_devices(out_devices, child, device_id);
    }

    void DeviceModule::remove_device_driver(const SharedPointer<Device>& current_device,
                                            const SharedPointer<Driver>& driver) {
        if (current_device->driver() == driver) {
            LOGGER->debug(R"({}: Unbind device from driver by {} v{})",
                          current_device->get_unique_name(),
                          driver->vendor(),
                          driver->version().to_string());
            current_device->driver()->remove_device(current_device);
            current_device->driver() = SharedPointer<Driver>();
        }
        for (auto& child : current_device->child_devices()) remove_device_driver(child, driver);
    }

    // ========================================================================================== //
    // Public Functions
    // ========================================================================================== //

    DeviceModule::DeviceModule() = default;

    auto DeviceModule::get_name() const -> String { return "Device"; }

    auto DeviceModule::load(const BootInfo& boot_info) -> bool {
        SILENCE_UNUSED(boot_info)

        auto root_device_driver = find_device_driver(&ACPIDriver::ID_ACPI);
        if (!root_device_driver) {
            LOGGER->error("The root device driver is missing.");
            return false;
        }
        auto root_device_handle = m_device_handle_counter.acquire();
        // ACPI needs to be initialized to query OEM and revision; hence a dummy device is created
        // that is required to start the ACPI driver
        // After initialization the dummy device is replaced with the full ACPI device object
        SharedPointer<Device> root_device_dummy(new BasicDevice(root_device_handle,
                                                                ACPIDriver::ID_ACPI.get_string_ID(),
                                                                "",
                                                                "",
                                                                "",
                                                                DeviceType::GENERIC,
                                                                ACPIDriver::ID_ACPI));
        m_device_tree = root_device_dummy;
        if (!root_device_driver->accept_device(root_device_dummy)) {
            LOGGER->error("Failed to bind the root device");
            return false;
        }

        IORequest request{};
        auto      acpi_req = ACPIRequest::GET_ACPI_INFO;
        ACPIInfo  acpi_info;
        request.m_in_buffer  = &acpi_req;
        request.m_out_buffer = &acpi_info;
        auto req_status      = root_device_driver->handle_request(root_device_dummy, request);
        if (req_status != IORequestStatus::HANDLED) {
            LOGGER->error("Failed to configure the root device.");
            return false;
        }
        constexpr int         RADIX_DECIMAL = 10;
        SharedPointer<Device> root_device(
            new BasicDevice(root_device_handle,
                            ACPIDriver::ID_ACPI.get_string_ID(),
                            acpi_info.m_oem,
                            int_to_string(acpi_info.m_revision, RADIX_DECIMAL),
                            "",
                            DeviceType::GENERIC,
                            ACPIDriver::ID_ACPI));

        // Replace the ACPI dummy device with the correct ACPI device object.
        m_device_tree = root_device;
        root_device->child_devices().add_all(root_device_dummy->child_devices());
        for (auto& child : root_device->child_devices()) child->bus_device() = root_device;
        root_device->driver() = root_device_driver;
        return true;
    }

    // ========================================================================================== //
    // Device Driver API
    // ========================================================================================== //

    auto DeviceModule::device_driver_store() const -> const LinkedList<SharedPointer<Driver>>& {
        return m_driver_store;
    }

    auto DeviceModule::register_device_driver(const SharedPointer<Driver>& driver) -> bool {
        if (!driver) return false;
        for (auto& d : m_driver_store)
            if (d == driver) return false;

        m_driver_store.add_back(driver);

        if (m_device_tree) {
            LinkedList<SharedPointer<Device>> matching_devices;
            match_devices(matching_devices, m_device_tree, driver->target_device_ID());

            for (auto& dev : matching_devices) {
                if (driver->accept_device(dev)) {
                    LOGGER->debug(R"({}: Bind device to driver by {} v{})",
                                  dev->get_unique_name(),
                                  driver->vendor(),
                                  driver->version().to_string());
                    dev->driver() = driver;
                }
            }
        }
        return true;
    }

    auto DeviceModule::unregister_device_driver(const SharedPointer<Driver>& driver) -> bool {
        if (!driver) return false;
        if (!m_driver_store.remove(driver)) return false;
        remove_device_driver(m_device_tree, driver);
        return true;
    }

    // ========================================================================================== //
    // Device API
    // ========================================================================================== //

    auto DeviceModule::get_keyboard() -> VirtualKeyboard* {
        return static_cast<VirtualKeyboard*>(
            find_device_driver(&PS2Keyboard::ID_PS2_KEYBOARD).get());
    }

    auto DeviceModule::get_device_handle() -> Handle { return m_device_handle_counter.acquire(); }

    auto DeviceModule::device_tree() const -> const SharedPointer<Device>& { return m_device_tree; }

    auto DeviceModule::register_device(const SharedPointer<Device>& bus_device,
                                       const SharedPointer<Device>& device) -> bool {

        if (!bus_device || !device) return false;
        if (bus_device == device) return false;
        if (device->device_type() == DeviceType::NONE) return false;
        if (!find_device(m_device_tree, bus_device->get_handle())) return false;
        if (find_device(m_device_tree, device->get_handle())) return false;

        bus_device->child_devices().add_back(device);
        device->bus_device() = bus_device;
        LOGGER->debug("New device {}: OEM: {}, Rev: {}, SN: {}",
                      device->get_unique_name(),
                      device->oem(),
                      device->revision(),
                      device->serial_number());

        auto driver = find_device_driver(device->device_ID());
        if (!driver) return true;

        if (driver->accept_device(device)) {
            LOGGER->debug(R"({}: Bind device to driver by {} v{})",
                          device->get_unique_name(),
                          driver->vendor(),
                          driver->version().to_string());
            device->driver() = driver;
        }
        return true;
    }

    // NOLINTBEGIN readability-convert-member-functions-to-static: dont care
    auto DeviceModule::unregister_device(const SharedPointer<Device>& device) -> bool {
        if (!device) return false;
        if (!device->bus_device())
            // The device is either not in the device tree or is the root device
            return false;

        auto bus_device = device->bus_device();
        if (bus_device) bus_device->child_devices().remove(device);
        device->bus_device() = SharedPointer<Device>();

        if (device->driver()) {
            device->driver()->remove_device(device);
            device->driver() = SharedPointer<Driver>();
        }
        return true;
    }
    // NOLINTEND

    auto DeviceModule::control_device(Handle dev_handle, const IORequest& io_request)
        -> IORequestStatus {
        auto device = find_device(m_device_tree, dev_handle);
        if (!device) return IORequestStatus::UNKNOWN_DEVICE;
        if (!device->driver()) return IORequestStatus::DEVICE_NOT_OPERATIONAL;

        return device->driver()->handle_request(device, io_request);
    }

} // namespace Rune::Device