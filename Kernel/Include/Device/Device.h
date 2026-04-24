
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
        virtual ~DeviceID()                                              = default;
        virtual auto get_device_ID_type() -> DeviceIDType                = 0;
        virtual auto equals(const SharedPointer<DeviceID>& d_ID) -> bool = 0;
    };

    class StringDeviceID : public DeviceID {
        String m_string_ID;

      public:
        StringDeviceID(const String& device_id);

        auto get_device_ID_type() -> DeviceIDType override;
        auto equals(const SharedPointer<DeviceID>& d_ID) -> bool override;
    };

    // ========================================================================================== //
    // Device
    // ========================================================================================== //

    class Driver;

    /// @brief Handle type of device.
    using DeviceHandle = U16;

    /// @brief Handle type of driver.
    using DriverHandle = U16;

    /// @brief A device connected to the system e.g., a keyboard.
    ///
    /// Each device has a unique handle and a unique name. The name will be used to map devices to
    /// device drivers. Since handles are dynamically assigned during runtime, they cannot be used
    /// to identify specific devices for a device driver; hence a static name is used.
    ///
    /// Devices are organized in a tree structure. Generally speaking, devices can be divided into
    /// two types of devices: Bus Devices and Leaf Devices. Bus devices like PCIe can contain
    /// other devices, while Leaf Devices do not, e.g., a keyboard.
    struct Device : public Resource<DeviceHandle> {
        /// @brief Name of the original equipment manufacturer
        String m_oem;

        /// @brief The revision of the device.
        U32 m_revision = 0;

        /// @brief The handle of the driver operating this device.
        DriverHandle m_driver_handle = Resource<DriverHandle>::HANDLE_NONE;

        /// @brief If this device is a bus device, this list contains all devices detected on the
        ///         bus.
        LinkedList<DeviceHandle> m_child_devices;

        /// @brief Determines if this device will be treated as a bus device. True: Bus Device,
        /// False: Leaf Device.
        bool m_is_bus_device = false;

        SharedPointer<DeviceID> m_device_ID;

        Device(DeviceHandle handle, const String& name);
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
    /// DeviceHandle: The handle of the bus device that has discovered the given device.
    /// Device: A reference to a device discovered by a bus device.
    /// void*: A pointer to data required for device driver initialization.
    using DeviceMapper = Function<void(DeviceHandle, Device&, void*)>;

    /// @brief A device driver operates a device in the system.
    ///
    /// Device drivers are mapped to devices automatically. Device drivers provide the name of the
    /// supported, and the kernel will map it to the matching device once it is discovered.
    class Driver : public Resource<DriverHandle> {
        DeviceHandle m_device_handle;

      public:
        Driver(DriverHandle handle, const String& name);
        virtual ~Driver() = default;

        /// @brief Check if the driver is operating a device.
        /// @return True: A device handle was assigned to this driver, False: Otherwise.
        [[nodiscard]] auto is_operating_device() const -> bool {
            return m_device_handle != Resource<DeviceHandle>::HANDLE_NONE;
        }

        /// @brief
        /// @return The handle of the device that is operated by this driver.
        [[nodiscard]] auto get_operated_device() const -> DeviceHandle { return m_device_handle; }

        /// @brief
        /// @param device_handle Handle of the device to operate.
        void set_operated_device(DeviceHandle device_handle) {
            if (!is_operating_device()) {
                m_device_handle = device_handle;
            }
        }

        /// @brief
        /// @return CAD ID of the device this driver intends to operate.
        virtual auto get_target_device_ID() -> SharedPointer<DeviceID> = 0;

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
        virtual void discover_devices(const DeviceMapper&          device_mapper,
                                      HandleCounter<DeviceHandle>& dev_handle_counter) = 0;
    };
} // namespace Rune::Device

#endif // RUNEOS_DEVICE_H
