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

#ifndef RUNEOS_DEVICEMODULE_H
#define RUNEOS_DEVICEMODULE_H

#include <KRE/System/Module.h>

#include <Memory/MemoryModule.h>

#include <CPU/CPUModule.h>

#include <Device/AHCI/AHCI.h>
#include <Device/Device.h>
#include <Device/Keyboard/PS2Keyboard.h>

namespace Rune::Device {

    /// @brief The device module handles device tree configuration and access to devices and device
    ///         drivers.
    ///
    /// # Device Tree
    ///
    /// Devices are organized in a tree structure, the Device Tree, where Bus Devices will contain
    /// child devices and so on, until a Leaf Device is reached, meaning a Device that does not
    /// contain any child devices. Bus Devices are used to discover devices connected to a bus,
    /// PCIe is a bus device, while a Leaf Device could be an RTC.
    ///
    /// The Device Tree is accessed through the Root Device.
    ///
    /// # Root Device
    ///
    /// The Root Device has access to the Root Bus, which is the very first bus accessed in the
    /// system, and it contains every other device.
    ///
    /// # Device Driver Mapping
    ///
    /// Once a Device has been discovered, it must be mapped to a device driver that operates it,
    /// this process is called Device Driver Mapping and happens automatically when the Device
    /// Module is loaded.
    ///
    /// Once a device is discovered, the device module will try to find the matching device driver.
    /// Devices provide a unique device ID, while a device driver will request the device ID of the
    /// device that it wants to operate.<br>
    /// Device IDs are context-aware, that means the structure of a Device ID depends on the bus.
    class DeviceModule : public Module {
        DeviceHandle m_root_device_handle;

        HandleCounter<DeviceHandle> m_device_handle_counter;
        HandleCounter<DriverHandle> m_driver_handle_counter;

        /// @brief Contains all devices.
        HashMap<DeviceHandle, SharedPointer<Device>> m_device_registry;

        /// @brief Contains all device drivers.
        HashMap<DriverHandle, SharedPointer<Driver>> m_device_driver_registry;

        /// @brief Fast lookup for devices of a specific type.
        HashMap<DeviceType, LinkedList<SharedPointer<Device>>> m_device_registry_by_type;

        /// @brief Register basic device drivers.
        void setup_device_driver_registry();

        /// @brief Find the driver with the matching device ID.
        /// @param device_ID
        /// @return A pointer to the device driver if found, otherwise null.
        auto search_device_driver(const DeviceID* device_ID) -> SharedPointer<Driver>;

        /// @brief Map the root device to the root device driver.
        /// @return True: The root device was mapped to the root device and initialized.
        ///         False: Otherwise.
        auto configure_root_device() -> bool;

        /// @brief Discover and start devices on the bus of the given device, then recursively call
        ///         this function on the discovered devices.
        /// @param device
        /// @param device_mapper
        void build_device_tree(SharedPointer<Device> device, const DeviceMapper& device_mapper);

      public:
        explicit DeviceModule();

        ~DeviceModule() override = default;

        // ====================================================================================== //
        // Module API
        // ====================================================================================== //

        [[nodiscard]] auto get_name() const -> String override;

        /// @brief Configure the root device and then build the device tree.
        /// @param boot_info
        /// @return True: The device tree has been build, False: Otherwise.
        auto load(const BootInfo& boot_info) -> bool override;

        // ====================================================================================== //
        // Device Module API
        // ====================================================================================== //

        auto get_ahci_driver() -> AHCIDriver*;

        auto get_keyboard() -> VirtualKeyboard*;

        // auto get_device_driver_handle_counter() -> HandleCounter<DriverHandle>&;
        // auto register_device_driver(const SharedPointer<Driver>& driver) -> bool;

        template <class DeviceInterface>
        auto get_devices(DeviceType device_type) -> LinkedList<DeviceInterface*> {
            LinkedList<Device*> devices;
            for (auto& device : m_device_registry_by_type[device_type])
                devices.add_back(static_cast<DeviceInterface*>(device.get()));
            return devices;
        }
    };
} // namespace Rune::Device

#endif // RUNEOS_DEVICEMODULE_H
