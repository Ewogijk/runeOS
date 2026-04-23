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
    /// Devices must have unique names, while device drivers must request the name of the device
    /// they are intended to operate, their target device. This way the device module knows which
    /// device driver should be mapped to which device.
    class DeviceModule : public Module {
        /// @brief ACPI provides the root bus.
        static const String ROOT_DEVICE_NAME;

        UniquePointer<AHCIDriver> _ahci_driver;

        Device _root_device;

        HandleCounter<DeviceHandle> _device_handle_counter;
        HandleCounter<DriverHandle> _driver_handle_counter;

        /// @brief Contains all devices.
        HashMap<DeviceHandle, Device> _device_registry;

        /// @brief Contains all device drivers.
        HashMap<String, SharedPointer<Driver>> _device_driver_registry;

        /// @brief Register basic device drivers.
        void setup_device_driver_registry();

        /// @brief Map the root device to the root device driver.
        /// @return True: The root device was mapped to the root device and initialized.
        ///         False: Otherwise.
        auto configure_root_device() -> bool;

      public:
        explicit DeviceModule();

        ~DeviceModule() override = default;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                      KernelSubsystem Overrides
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        [[nodiscard]] auto get_name() const -> String override;

        auto load(const BootInfo& boot_info) -> bool override;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                  Device specific functions
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        void set_ahci_driver(UniquePointer<AHCIDriver> ahci_driver);

        auto get_ahci_driver() -> AHCIDriver*;

        auto get_keyboard() -> VirtualKeyboard*;
    };
} // namespace Rune::Device

#endif // RUNEOS_DEVICEMODULE_H
