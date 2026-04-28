
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

#ifndef RUNEOS_DEVICE_H
#define RUNEOS_DEVICE_H

#include <KRE/Memory.h>
#include <KRE/String.h>
#include <KRE/System/Resource.h>

namespace Rune::Device {
    // ========================================================================================== //
    // DeviceID
    // ========================================================================================== //

#define DEVICE_ID_TYPES(X)                                                                         \
    X(DeviceIDType, STRING, 0x1)                                                                   \
    X(DeviceIDType, PCI, 0x2)

    /// @brief Helper type to differentiate between DeviceID subclasses.
    ///
    /// This is needed because dynamic_cast is unsupported.
    ///
    /// - STRING: StringDeviceID class
    /// - PCI: PCIDeviceID class
    DECLARE_ENUM(DeviceIDType, DEVICE_ID_TYPES, 0x0) // NOLINT

    /// @brief Base type for a context-aware device ID (CAD ID) that is used to match devices and
    /// device drivers.
    ///
    /// Example: A PCI Device ID could consist of the Device's base class, subclass and programming
    ///             interface.
    class DeviceID {
      public:
        virtual ~DeviceID()                                                   = default;
        [[nodiscard]] virtual auto get_device_ID_type() const -> DeviceIDType = 0;
        [[nodiscard]] virtual auto equals(const DeviceID* d_ID) const -> bool = 0;
    };

    // ========================================================================================== //
    // BasicDeviceID
    // ========================================================================================== //

    /// @brief A DeviceID implementation for devices with a simple string ID.
    class BasicDeviceID : public DeviceID {
        String m_string_ID;

      public:
        BasicDeviceID(const String& device_id);

        [[nodiscard]] auto get_device_ID_type() const -> DeviceIDType override;
        [[nodiscard]] auto equals(const DeviceID* d_ID) const -> bool override;

        [[nodiscard]] auto get_string_ID() const -> String;
    };

    // ========================================================================================== //
    // Device
    // ========================================================================================== //

    class Driver;

    /// @brief Handle type of device.
    using DeviceHandle = U16;

    /// @brief Handle type of driver.
    using DriverHandle = U16;

#define DEVICE_TYPES(X)                                                                            \
    X(DeviceType, MASS_STORAGE_DEVICE, 0x1)                                                        \
    X(DeviceType, KEYBOARD, 0x2)                                                                   \
    X(DeviceType, GENERIC, 0x3)

    /// @brief Describes general functionality of a device and to which interface type it can be
    ///         safely cast.
    ///
    /// - MASS_STORAGE_DEVICE: A device that stores large amounts of data, e.g., HDD, SSD, USB, etc.
    ///     - Interface: TBD
    /// - KEYBOARD: A computer keyboard.
    ///     - Interface: VirtualKeyboard - Device/Keyboard/Keyboard.h
    /// - GENERIC: A generic device, the catch-all category of devices.
    ///     - Interface: Device
    DECLARE_ENUM(DeviceType, DEVICE_TYPES, 0x0) // NOLINT

    /// @brief A device connected to the system e.g., a keyboard.
    ///
    /// Each device has a unique handle and a unique name. The name will be used to map devices to
    /// device drivers. Since handles are dynamically assigned during runtime, they cannot be used
    /// to identify specific devices for a device driver; hence a static name is used.
    ///
    /// Devices are organized in a tree structure. Generally speaking, devices can be divided into
    /// two types of devices: Bus Devices and Leaf Devices. Bus devices like PCIe can contain
    /// other devices, while Leaf Devices do not, e.g., a keyboard.
    class Device : public Resource<DeviceHandle> {
        String m_oem;

        String m_revision;

        String m_serial_number;

        DeviceType m_device_type;

        /// @brief The handle of the driver operating this device.
        DriverHandle m_driver_handle = Resource<DriverHandle>::HANDLE_NONE;

        /// @brief If this device is a bus device, this list contains all devices detected on the
        ///         bus.
        LinkedList<DeviceHandle> m_child_devices;

      public:
        Device(DeviceHandle  handle,
               const String& name,
               const String& oem,
               const String& revision,
               const String& serial_number,
               DeviceType    device_type);
        virtual ~Device() = default;

        /// @brief
        /// @return The OEM name of the device manufacturer.
        [[nodiscard]] auto get_oem() const -> const String&;

        /// @brief
        /// @return The hardware revision identifier of the device.
        [[nodiscard]] auto get_revision() const -> const String&;

        /// @brief
        /// @return The serial number uniquely identifying this device unit.
        [[nodiscard]] auto get_serial_number() const -> const String&;

        /// @brief
        /// @return The general category of this device.
        [[nodiscard]] auto get_device_type() const -> DeviceType;

        /// @brief
        /// @return The handle to the driver that operates this device.
        ///         Resource<DriverHandle>::HANDLE_NONE if no driver is mapped to the device.
        [[nodiscard]] auto get_driver_handle() const -> DriverHandle;

        /// @brief Set the handle of the driver that is operating this device.
        /// @param driver_handle
        void set_driver_handle(DriverHandle driver_handle);

        /// @brief
        /// @return A reference to the list of child devices.
        [[nodiscard]] auto get_child_devices() -> LinkedList<DeviceHandle>&;

        /// @brief
        /// @return A pointer to the Device ID.
        ///
        /// The device always owns the pointer and must guarantue a valid pointer is returned at all
        /// times.
        [[nodiscard]] virtual auto get_device_ID() const -> const DeviceID* = 0;
    };

    // ========================================================================================== //
    // BasicDevice
    // ========================================================================================== //

    class BasicDevice : public Device {
        BasicDeviceID m_device_ID;

      public:
        BasicDevice(DeviceHandle         handle,
                    const String&        name,
                    const String&        oem,
                    const String&        revision,
                    const String&        serial_number,
                    DeviceType           device_type,
                    const BasicDeviceID& device_ID);

        [[nodiscard]] auto get_device_ID() const -> const DeviceID* override;
    };

    // ========================================================================================== //
    // Driver
    // ========================================================================================== //

    /// @brief An IO request is a driver-specific context that contains information about an action
    ///         a driver should perform.
    using IORequest = void*;

#define IO_REQUEST_STATES(X)                                                                       \
    X(IORequestStatus, NO_DEVICE, 0x1)                                                             \
    X(IORequestStatus, BAD_ARGUMENT, 0x2)                                                          \
    X(IORequestStatus, FAILED, 0x3)                                                                \
    X(IORequestStatus, HANDLED, 0x4)

    /// @brief An IO Request status encodes the status of a request after the action was performed
    ///         by a driver.
    ///
    /// - NO_DEVICE: The request could not be handled because the driver is not mapped to a device.
    /// - BAD_ARGUMENT: The IO request contains an invalid argument.
    /// - FAILED: The IO request could not be handled due to an error.
    /// - HANDLED: The IO request was handled and the response contains valid data.
    DECLARE_ENUM(IORequestStatus, IO_REQUEST_STATES, 0x0) // NOLINT

    /// @brief An IO response is associated with an IO request and consists of the status of the
    ///         request and driver-specific data containing a return value.
    ///
    /// If the response has the status IORequestStatus::HANDLED, it must contain valid data as
    /// specified by the driver.
    struct IOResponse {
        IORequestStatus m_status = IORequestStatus::NONE;
        void*           m_data   = nullptr;
    };

    /// @brief A function that maps a device to a device driver.
    ///
    /// - DeviceHandle: The handle of the bus device that has discovered the given device.
    /// - Device: A pointer to a device discovered by a bus device.
    /// - void*: A pointer to data required for device driver initialization.
    using DeviceMapper = Function<void(DeviceHandle, SharedPointer<Device>, void*)>;

    /// @brief A device driver operates a device in the system.
    ///
    /// Device drivers are mapped to devices automatically. Device drivers provide the name of the
    /// supported, and the kernel will map it to the matching device once it is discovered.
    class Driver : public Resource<DriverHandle> {
        LinkedList<DeviceHandle> m_operated_devices;

      public:
        Driver(DriverHandle handle, const String& name);
        virtual ~Driver() = default;

        /// @brief
        /// @return A list handles of all operated devices.
        [[nodiscard]] auto get_operated_devices() const -> LinkedList<DeviceHandle> {
            return m_operated_devices;
        }

        /// @brief
        /// @param device_handle Handle of the device to operate.
        void add_operated_device(DeviceHandle device_handle) {
            m_operated_devices.add_back(device_handle);
        }

        /// @brief
        /// @return DeviceID of the device this driver intends to operate.
        /// The device driver always owns the pointer and must guarantue a valid pointer is
        /// returned at all times.
        [[nodiscard]] virtual auto get_target_device_ID() const -> const DeviceID* = 0;

        /// @brief Initialize the device to a working state so that it is able to handle IO
        ///         requests.
        /// @param context A device driver specific context required to start it.
        /// @return True: Device initialization was successful, False: Otherwise.
        ///
        /// The calling function has to guarantue that lifetime of context outlives the call to
        /// start, otherwise the behavior is not defined.
        virtual auto start(void* context) -> bool = 0;

        /// @brief Finish any ongoing IO requests, flush data to the device, if any, and shutdown
        /// the
        ///         device.
        /// @return True: Device shutdown was successful, False: Otherwise.
        ///
        /// The device will not be able to handle IO requests anymore after it has been shutdown.
        virtual auto stop() -> bool = 0;

        /// @brief Perform a device driver-specific action as requested.
        /// @param request An IO request.
        /// @return IO response containing the result of the action.
        virtual auto handle_request(IORequest request) -> IOResponse = 0;

        /// @brief Scan the bus for devices.
        /// @return A list of devices connected to the bus.
        ///
        /// If the operated device is a leaf device, an empty list must be returned.
        virtual void discover_devices(DeviceHandle                 bus_device,
                                      const DeviceMapper&          device_mapper,
                                      HandleCounter<DeviceHandle>& dev_handle_counter) = 0;
    };
} // namespace Rune::Device

namespace Rune {
    template <>
    struct Hash<Device::DeviceType> {
        auto operator()(const Device::DeviceType& key) const -> size_t { return FNV::do_hash(key.to_value()); }
    };
} // namespace Rune

#endif // RUNEOS_DEVICE_H
