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

#include <Device/Device.h>

#include <Device/Keyboard/Keyboard.h>

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

        HandleCounter<Handle> m_device_handle_counter;

        SharedPointer<Device> m_device_tree;

        LinkedList<SharedPointer<Driver>> m_driver_store;

        /// @brief Recursively get all devices in the device tree that have the same device type as
        ///         device_type and cast them to the DeviceInterface type.
        /// @tparam DeviceInterface Type that the devices will be cast to.
        /// @param out_devices Matching devices will be added to this list.
        /// @param current_device The current device that is matched with device_type.
        /// @param device_type Device type that devices have to match against.
        template <class DeviceInterface>
        void get_devices_by_type(LinkedList<SharedPointer<DeviceInterface>>& out_devices,
                                 const SharedPointer<Device>&                current_device,
                                 DeviceType                                  device_type) {
            if (current_device->device_type() == device_type)
                out_devices.add_back(SharedPointer<DeviceInterface>(current_device));

            for (const auto& dev : current_device->child_devices())
                get_devices_by_type<DeviceInterface>(out_devices, dev, device_type);
        }

        /// @brief Iterate the device tree to find the device matching with dev_handle.
        /// @param current_device
        /// @param dev_handle
        /// @return A pointer to the device matching with dev_handle, Otherwise: null.
        auto find_device(const SharedPointer<Device>& current_device, Handle dev_handle)
            -> SharedPointer<Device>;

        /// @brief Find the driver with the matching device ID.
        /// @param device_ID
        /// @return A pointer to the device driver if found, otherwise null.
        auto find_device_driver(const DeviceID* device_ID) -> SharedPointer<Driver>;

        /// @brief Iterate the device tree and find all devices matching with the device_id.
        /// @param out_devices Matching devices will be added to this list.
        /// @param current_device
        /// @param device_id Device ID that devices have to match with.
        void match_devices(LinkedList<SharedPointer<Device>>& out_devices,
                           const SharedPointer<Device>&       current_device,
                           const DeviceID*                    device_id);

        /// @brief Iterate the device tree and unbind all devices from the driver.
        /// @param current_device
        /// @param driver Driver that will be removed.
        void remove_device_driver(const SharedPointer<Device>& current_device,
                                  const SharedPointer<Driver>& driver);

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
        // Device Driver API
        // ====================================================================================== //

        /// @brief
        /// @return A reference to the device driver store.
        [[nodiscard]] auto device_driver_store() const -> const LinkedList<SharedPointer<Driver>>&;

        /// @brief Add the driver to the driver store and bind it to all devices with matching
        ///         device IDs.
        /// @param driver Pointer to a device driver.
        /// @return True: The device driver was registered,
        ///         False: Driver is null or the driver was already registered.
        auto register_device_driver(const SharedPointer<Driver>& driver) -> bool;

        /// @brief Remove the driver from the driver store and unbind it from all bound devices.
        /// @param driver
        /// @return True: The driver was removed,
        ///         False: Driver is null or the driver was not registered.
        auto unregister_device_driver(const SharedPointer<Driver>& driver) -> bool;

        // ====================================================================================== //
        // Device API
        // ====================================================================================== //

        auto get_keyboard() -> VirtualKeyboard*;

        /// @brief
        /// @return A pointer to the device tree.
        [[nodiscard]] auto device_tree() const -> const SharedPointer<Device>&;

        auto get_device_handle() -> Handle;

        /// @brief Get all devices that have the given device_type casted to DeviceInterface*.
        /// @tparam DeviceInterface Type to which all device pointers will be cast.
        /// @param device_type Type of the devices that will be returned.
        /// @return A list of pointers to devices casted to DeviceInterface.
        template <class DeviceInterface>
        auto get_devices(DeviceType device_type) -> LinkedList<SharedPointer<DeviceInterface>> {
            LinkedList<SharedPointer<DeviceInterface>> devices;
            get_devices_by_type(devices, m_device_tree, device_type);
            return devices;
        }

        /// @brief Get the device with dev_handle, ensure it has the requested device type.
        /// @tparam DeviceInterface Type to which the device will be cast.
        /// @param device_type Type of the desired device.
        /// @param dev_handle Handle of the desired device.
        /// @return The requested device cast to DeviceInterface. Nullptr if no device with the
        ///         handle was found or the device type does not match with device_type.
        template <class DeviceInterface>
        auto get_device(Handle dev_handle)
            -> SharedPointer<DeviceInterface> {
            return SharedPointer<DeviceInterface>(find_device(m_device_tree, dev_handle));
        }

        /// @brief Add the device to the given bus_device and try to bind it to a registered driver.
        /// @param bus_device Bus device.
        /// @param device Device to add to the bus device.
        /// @return True: The device was added to bus_device.
        ///         False: The device or bus_device are null, device has the device type NONE,
        ///         bus_device is not in the device tree, device is already in the device tree.
        auto register_device(const SharedPointer<Device>& bus_device,
                             const SharedPointer<Device>& device) -> bool;

        /// @brief Remove the device from the device tree and unbind it from its device driver.
        /// @param device
        /// @return True: The device is removed from the device tree.
        ///         False: The device is null.
        auto unregister_device(const SharedPointer<Device>& device) -> bool;

        /// @brief Try to find the device with dev_handle and forward the io_request to its device
        ///         driver.
        /// @param dev_handle Handle of the device that should handle the IO request.
        /// @param io_request IO request.
        /// @return On successfully forwarded IO request: The status returned by the driver.
        ///         Otherwise:<br>
        ///         UNKNOWN_DEVICE: No device with dev_handle exists.
        ///         DEVICE_NOT_OPERATIONAL: The device is not operated by a driver.
        auto control_device(Handle dev_handle, const IORequest& io_request) -> IORequestStatus;
    };
} // namespace Rune::Device

#endif // RUNEOS_DEVICEMODULE_H
