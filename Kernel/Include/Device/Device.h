
//  Copyright 2026 Ewogijk
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#ifndef RUNEOS_DEVICE_H
#define RUNEOS_DEVICE_H

#include <KRE/Memory.h>
#include <KRE/Resource.h>
#include <KRE/String.h>
#include <KRE/System/Module.h>

#include <CPU/Threading/Future.h>

namespace Rune::Device {
    // ========================================================================================== //
    // DeviceID
    // ========================================================================================== //

#define DEVICE_ID_TYPES(X)                                                                         \
    X(DeviceIDType, STRING, 0x1)                                                                   \
    X(DeviceIDType, PCI, 0x2)                                                                      \
    X(DeviceIDType, USB, 0x3)

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
    using Handle = U16;

#define DEVICE_TYPES(X)                                                                            \
    X(DeviceType, GENERIC, 0x1)                                                                    \
    X(DeviceType, KEYBOARD, 0x2)                                                                   \
    X(DeviceType, MASS_STORAGE_DEVICE, 0x3)                                                        \
    X(DeviceType, USB_COMPOSITE_DEVICE, 0x4)                                                       \
    X(DeviceType, USB_FUNCTION_DEVICE, 0x5)

    /// @brief Describes the general functionality of a device and to which interface type it can be
    ///         safely cast.
    ///
    /// - GENERIC: A generic device, the catch-all category of devices.
    ///     - Interface: Device
    /// - KEYBOARD: A computer keyboard.
    ///     - Interface: BasicDevice
    /// - MASS_STORAGE_DEVICE: A device that stores large amounts of data, e.g., HDD, SSD, etc.
    ///     - Interface: MassStorageDevice
    /// - USB_COMPOSITE_DEVICE: A USB composite device.
    ///     - Interface: CompositeDevice
    /// - USB_FUNCTION_DEVICE: A USB device representing a USB function.
    ///     - Interface: FunctionDevice
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
    class Device : public Resource<Handle> {
        String m_oem;

        String m_revision;

        String m_serial_number;

        DeviceType m_device_type;

        /// @brief The handle of the driver operating this device.
        SharedPointer<Driver> m_driver;

        /// @brief
        SharedPointer<Device> m_bus_device;

        /// @brief If this device is a bus device, this list contains all devices detected on the
        ///         bus.
        LinkedList<SharedPointer<Device>> m_child_devices;

      public:
        Device(Handle        handle,
               const String& name,
               const String& oem,
               const String& revision,
               const String& serial_number,
               DeviceType    device_type);
        virtual ~Device() = default;

        /// @brief
        /// @return The OEM name.
        [[nodiscard]] auto oem() const -> const String&;

        /// @brief
        /// @return The hardware revision identifier of the device.
        [[nodiscard]] auto revision() const -> const String&;

        /// @brief
        /// @return The serial number of the device.
        [[nodiscard]] auto serial_number() const -> const String&;

        /// @brief
        /// @return The general category of this device.
        [[nodiscard]] auto device_type() const -> DeviceType;

        /// @brief
        /// @return The handle to the driver that operates this device.
        ///         nullptr if no driver is mapped to the device.
        [[nodiscard]] auto driver() const -> const SharedPointer<Driver>&;

        /// @brief
        /// @return The handle to the driver that operates this device.
        ///         nullptr if no driver is mapped to the device.
        [[nodiscard]] auto driver() -> SharedPointer<Driver>&;

        /// @brief
        /// @return The bus device of this device.
        ///         nullptr for the root device.
        ///
        /// Every device except the root device always has to have a bus device.
        [[nodiscard]] auto bus_device() const -> const SharedPointer<Device>&;

        /// @brief
        /// @return The bus device of this device.
        ///         nullptr for the root device.
        ///
        /// Every device except the root device always has to have a bus device.
        [[nodiscard]] auto bus_device() -> SharedPointer<Device>&;

        /// @brief
        /// @return A reference to the list of child devices.
        [[nodiscard]] auto child_devices() const -> const LinkedList<SharedPointer<Device>>&;

        /// @brief
        /// @return A reference to the list of child devices.
        [[nodiscard]] auto child_devices() -> LinkedList<SharedPointer<Device>>&;

        /// @brief
        /// @return A pointer to the Device ID.
        ///
        /// The device always owns the pointer and must guarantee a valid pointer is returned at all
        /// times.
        [[nodiscard]] virtual auto device_ID() const -> const DeviceID* = 0;
    };

    // ========================================================================================== //
    // BasicDevice
    // ========================================================================================== //

    class BasicDevice : public Device {
        BasicDeviceID m_device_ID;

      public:
        BasicDevice(Handle        handle,
                    const String& name,
                    const String& oem,
                    const String& revision,
                    const String& serial_number,
                    DeviceType    device_type,
                    BasicDeviceID device_ID);

        [[nodiscard]] auto device_ID() const -> const DeviceID* override;
    };

    // ========================================================================================== //
    // Driver
    // ========================================================================================== //

    /// @brief An IO request contains the driver-specific in and out buffers.
    ///
    /// - In Data: Contains information about an action a driver should perform.
    /// - Out Data: If the requested action was performed successfully, the out buffer contains
    ///                 the driver response, otherwise the buffer content is undefined.
    struct IORequest {
        void* m_in_data;
        void* m_out_data;
    };

#define IO_REQUEST_STATES(X)                                                                       \
    X(IORequestStatus, UNSUPPORTED, 0x1)                                                           \
    X(IORequestStatus, UNKNOWN_DEVICE, 0x2)                                                        \
    X(IORequestStatus, DEVICE_NOT_OPERATIONAL, 0x3)                                                \
    X(IORequestStatus, UNKNOWN_DRIVER, 0x4)                                                        \
    X(IORequestStatus, BAD_ARGUMENT, 0x5)                                                          \
    X(IORequestStatus, FAILED, 0x6)                                                                \
    X(IORequestStatus, HANDLED, 0x7)

    /// @brief An IO Request status encodes the status of a request after the action was performed
    ///         by a driver.
    ///
    /// - UNSUPPORTED: IO requests are not supported.
    /// - UNKNOWN_DEVICE: The requested device does not exist.
    /// - DEVICE_NOT_OPERATIONAL: The IO request could not be handled because the device is not
    ///                             operated by a driver.
    /// - UNKNOWN_DRIVER: The IO request could not be handled because the device's driver was not
    ///                     found.
    /// - BAD_ARGUMENT: The IO request In buffer contains invalid arguments.
    /// - FAILED: An error occurred handling the IO request.
    /// - HANDLED: The IO request was handled, and the response contains valid data.
    DECLARE_ENUM(IORequestStatus, IO_REQUEST_STATES, 0x0) // NOLINT

    /// @brief A device driver operates devices in the system.
    class Driver {
      public:
        Driver()          = default;
        virtual ~Driver() = default;

        /// @brief
        /// @return Driver vendor.
        [[nodiscard]] virtual auto vendor() const -> String = 0;

        /// @brief
        /// @return Driver version.
        [[nodiscard]] virtual auto version() const -> Version = 0;

        /// @brief
        /// @param device_ID ID of a device.
        /// @return True: This driver can bind to the device.
        ///         False: Otherwise.
        virtual auto can_bind(const DeviceID* device_ID) -> bool = 0;

        /// @brief Initialize the device to a state that allows it to handle IO requests.
        /// @param device Handle of the device to initialize.
        /// @return True: The device is initialized.
        ///         False: Otherwise.
        ///
        /// If device initialization succeeds, the driver will be bound to the device which means
        /// IO requests for the device will be forwarded to this driver.
        ///
        /// This function will be called when the driver returned true after a call the can_bind().
        virtual auto bind(const SharedPointer<Device>& device) -> bool = 0;

        /// @brief Cancel any ongoing IO requests and release any resources associated with the
        ///         device.
        /// @param dev_handle Handle of the device to stop.
        ///
        /// The device driver will be unbound from the device after this function has been called.
        ///
        /// This function is called before a device will be removed from the device tree.
        virtual void unbind(const SharedPointer<Device>& device) = 0;

        /// @brief Perform a device driver-specific action on the device with the given handle.
        /// @param device Reference to a bound device.
        /// @param request An IO request.
        /// @return The status of the finished IO request.
        virtual auto handle_request(const SharedPointer<Device>& device, IORequest request)
            -> CPU::Future<IORequestStatus> = 0;
    };
} // namespace Rune::Device

namespace Rune {
    template <>
    struct Hash<Device::DeviceType> {
        auto operator()(const Device::DeviceType& key) const -> size_t {
            return FNV::do_hash(key.to_value());
        }
    };
} // namespace Rune

#endif // RUNEOS_DEVICE_H
