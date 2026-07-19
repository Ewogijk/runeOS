
//  Copyright 2025 Ewogijk
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

#ifndef RUNEOS_USB_H
#define RUNEOS_USB_H

#include <Device/Device.h>

#include <Device/USB/Descriptor.h>
#include <Device/USB/Request.h>

namespace Rune::Device::USB {
    // ========================================================================================== //
    // USB Device ID
    // ========================================================================================== //

    class USBDeviceID : public DeviceID {
        U8 m_device_class;
        U8 m_subclass;
        U8 m_protocol;

      public:
        USBDeviceID(U8 device_class, U8 subclass, U8 protocol);

        [[nodiscard]] auto get_device_ID_type() const -> DeviceIDType override;
        [[nodiscard]] auto equals(const DeviceID* d_ID) const -> bool override;

        [[nodiscard]] auto device_class() const -> U8;
        [[nodiscard]] auto subclass() const -> U8;
        [[nodiscard]] auto protocol() const -> U8;
    };

    // ========================================================================================== //
    // USB Configuration Model — USB 3.2 §9.6.3-§9.6.6
    // ========================================================================================== //

    /// @brief One endpoint of an alternate setting, decoded from an EndpointDescriptor
    ///         (USB 3.2 §9.6.6, Table 9-26).
    struct EndPoint {
        U8           m_endpoint_number = 0;                  // bEndpointAddress bits 3..0
        Direction    m_direction       = Direction::NONE;    // bEndpointAddress bit 7
        TransferType m_transfer_type   = TransferType::NONE; // bmAttributes bits 1..0
        /// @brief bmAttributes bits 3..2, only meaningful when m_transfer_type ==
        ///         TransferType::ISOCHRONOUS; see sync_type().
        U8 m_synchronization = 0;
        /// @brief bmAttributes bits 5..4, only meaningful when m_transfer_type is
        ///         ISOCHRONOUS or INTERRUPT; see isochronous_usage_type()/interrupt_usage_type().
        U8  m_usage           = 0;
        U16 m_max_packet_size = 0; // wMaxPacketSize
        U8  m_interval        = 0; // bInterval, polling interval in 125 us units

        /// @brief bMaxBurst from the SuperSpeed Endpoint Companion descriptor (USB 3.2 §9.6.7):
        ///         maximum number of packets per burst minus one, 0..15. Zero for control
        ///         endpoints and for endpoints not operating at Gen X speed (no companion).
        U8 m_max_burst = 0;
        /// @brief Mult from the SuperSpeed Endpoint Companion descriptor: zero-based number of
        ///         additional packets per service interval. Only meaningful for isochronous
        ///         endpoints; zero otherwise.
        U8 m_mult = 0;
        /// @brief wBytesPerInterval from the SuperSpeed Endpoint Companion descriptor: total
        ///         bytes transferred per service interval. Only valid for periodic endpoints;
        ///         zero for control/bulk and for endpoints without a companion.
        U16 m_bytes_per_interval = 0;

        /// @brief
        /// @return m_synchronization decoded as a SyncType.
        ///         Valid only when m_transfer_type == TransferType::ISOCHRONOUS.
        [[nodiscard]] auto sync_type() const -> SyncType;

        /// @brief
        /// @return m_usage decoded as an InterruptUsageType.
        ///         Valid only when m_transfer_type == TransferType::INTERRUPT.
        [[nodiscard]] auto interrupt_usage_type() const -> InterruptUsageType;

        /// @brief
        /// @return m_usage decoded as an IsochronousUsageType.
        ///         Valid only when m_transfer_type == TransferType::ISOCHRONOUS.
        [[nodiscard]] auto isochronous_usage_type() const -> IsochronousUsageType;
    };

    /// @brief One alternate setting of an Interface, USB 3.2 §9.6.5.
    ///
    /// Alternate settings of the same interface share the same interface number but do not
    /// share endpoints; each alternate setting owns a fully independent endpoint list. The
    /// default alternate setting is always 0; SetInterface() selects among the others.
    struct AlternateSetting {
        U8                   m_setting_number     = 0; // bAlternateSetting
        U8                   m_interface_class    = 0;
        U8                   m_interface_subclass = 0;
        U8                   m_interface_protocol = 0;
        U8                   m_string_index       = 0; // iInterface
        LinkedList<EndPoint> m_endpoints;
    };

    /// @brief One interface slot within a Configuration, owning all of its alternate settings.
    struct Interface {
        U8                           m_interface_number = 0; // bInterfaceNumber
        U8                           m_active_setting = 0; // currently selected via SetInterface()
        LinkedList<AlternateSetting> m_alternate_settings;

        /// @brief
        /// @return The alternate setting matching m_active_setting.
        [[nodiscard]] auto active() const -> const AlternateSetting&;
    };

    /// @brief One logical function of a Configuration, USB 3.2 §9.6.4 — either an Interface
    ///         Association Descriptor group (e.g. a CDC-ACM modem's Communications and Data
    ///         interfaces) or a single interface not covered by an IAD.
    ///
    /// Owns every Interface (and their alternate settings/endpoints) that make up the function.
    /// A Configuration's interfaces are partitioned disjointly among its functions.
    struct Function {
        U8                    m_function_class    = 0;
        U8                    m_function_subclass = 0;
        U8                    m_function_protocol = 0;
        U8                    m_string_index      = 0; // iFunction
        LinkedList<Interface> m_interfaces;
    };

    /// @brief One of the device's configurations, USB 3.2 §9.6.3.
    ///
    /// A device has one or more configurations, only one active at a time via
    /// SetConfiguration(bConfigurationValue). Configurations are fully independent of one
    /// another: the number of interfaces, their functions, and their endpoints may differ
    /// completely from one configuration to the next.
    struct Configuration {
        U8   m_configuration_value = 0;     // argument to SetConfiguration()
        U8   m_string_index        = 0;     // iConfiguration
        bool m_self_powered        = false; // bmAttributes D6
        bool m_remote_wakeup       = false; // bmAttributes D5
        U16  m_max_power_mA        = 0;     // bMaxPower, normalized to mA
        /// @brief One entry per logical function of this configuration. Always populated: an
        ///         interface not covered by an Interface Association Descriptor is its own
        ///         single-interface function. Every interface of the configuration lives in
        ///         exactly one of these functions.
        LinkedList<Function> m_functions;
    };

    // ========================================================================================== //
    // USB Composite Device
    // ========================================================================================== //

    /// @brief The physical USB device attached to a port.
    ///
    /// Owns the device-level identity and every configuration reported by the device. It is a
    /// Bus Device: once a configuration is selected via SetConfiguration(), one child
    /// USBFunctionDevice is registered per Function of that configuration.
    class CompositeDevice : public Device {
        USBDeviceID m_device_ID; // from the Device Descriptor

        U16 m_vendor_ID;

        U16 m_product_ID;

        /// @brief All configurations reported by the device (bNumConfigurations).
        LinkedList<Configuration> m_configurations;
        /// @brief bConfigurationValue of the currently active configuration, absent while the
        ///         device has not been configured yet.
        Optional<U8> m_active_configuration;

      public:
        CompositeDevice(Handle        handle,
                        const String& name,
                        const String& oem,
                        const String& revision,
                        const String& serial_number,
                        USBDeviceID   usb_device_id,
                        U16           vendor_ID,
                        U16           product_ID);

        [[nodiscard]] auto device_ID() const -> const DeviceID* override;

        [[nodiscard]] auto vendor_ID() const -> U16;

        [[nodiscard]] auto product_ID() const -> U16;

        /// @brief
        /// @return All configurations reported by this device.
        [[nodiscard]] auto configurations() const -> const LinkedList<Configuration>&;

        /// @brief
        /// @return All configurations reported by this device (mutable, e.g. to update an
        ///         interface's active alternate setting).
        [[nodiscard]] auto configurations() -> LinkedList<Configuration>&;

        /// @brief Add a configuration parsed from a GET_DESCRIPTOR(CONFIGURATION) response.
        void add_configuration(Configuration configuration);

        /// @brief
        /// @return bConfigurationValue of the active configuration, absent if not yet
        ///         configured.
        [[nodiscard]] auto active_configuration() const -> Optional<U8>;

        /// @brief Mark configuration_value as active after a successful SetConfiguration().
        void set_active_configuration(U8 configuration_value);
    };

    // ========================================================================================== //
    // USB Function Device
    // ========================================================================================== //

    /// @brief One logical function of a USBCompositeDevice's active configuration, registered as
    ///         a child device of its USBCompositeDevice.
    ///
    /// This is the unit device drivers bind to: it represents every Interface (and their
    /// alternate settings/endpoints) that make up the function, whether declared through a single
    /// interface or grouped by an Interface Association Descriptor. The interfaces themselves are
    /// owned by the parent CompositeDevice (reachable via bus_device()); this device only stores
    /// which Configuration and Function they belong to.
    class FunctionDevice : public Device {
        USBDeviceID m_device_ID; // Function's class/subclass/protocol

        U8  m_configuration_value = 0; // owning Configuration::m_configuration_value
        U16 m_function_value      = 0; // index into the owning Configuration's m_functions

        /// @brief Resolve the owning Function from the parent CompositeDevice (bus_device()).
        [[nodiscard]] auto owning_function() const -> const Function&;

      public:
        FunctionDevice(Handle        handle,
                       const String& name,
                       const String& oem,
                       const String& revision,
                       const String& serial_number,
                       USBDeviceID   usb_device_id,
                       U8            configuration_value,
                       U16           function_idx);

        [[nodiscard]] auto device_ID() const -> const DeviceID* override;

        [[nodiscard]] auto configuration_value() const -> U8;

        [[nodiscard]] auto interfaces() const -> const LinkedList<Interface>&;

        /// @brief
        /// @return The interface with the given number, nullptr if it is not part of this
        ///         function.
        [[nodiscard]] auto find_interface(U8 interface_number) const -> const Interface*;

        /// @brief Select an alternate setting for one of this function's interfaces after a
        ///         successful SetInterface(). Does nothing if interface_number is not part of
        ///         this function.
        void set_active_setting(U8 interface_number, U8 setting_number);
    };

    // ========================================================================================== //
    // Host Controller IO Requests
    //
    // The IORequest struct shall be set up as followed:
    //      io_request.m_in_buffer  = &my_transfer_request; // e.g. ControlTransferRequest
    //      io_request.m_out_buffer = &my_data_buffer;      // An buffer depending on the transfer
    //                                                      // request
    // ========================================================================================== //

#define TRANSFER_REQUEST_TYPES(X)                                                                  \
    X(TransferRequestType, CONTROL, 0x1)                                                           \
    X(TransferRequestType, BULK, 0x2)                                                              \
    X(TransferRequestType, INTERRUPT, 0x3)                                                         \
    X(TransferRequestType, ISOCHRONOUS, 0x4)

    /// @brief The supported general transfer types.
    ///
    /// - CONTROL:     Control transfer (default control pipe, EP0).
    /// - BULK:        Bulk transfer on a bulk endpoint.
    /// - INTERRUPT:   Interrupt transfer on an interrupt endpoint.
    /// - ISOCHRONOUS: Isochronous transfer on an isochronous endpoint.
    DECLARE_ENUM(TransferRequestType, TRANSFER_REQUEST_TYPES, 0x0) // NOLINT

    /// @brief The header of a transfer request identifies the calling function device and the
    ///         request type.
    struct TransferRequestHeader {
        TransferRequestType m_transfer_type = TransferRequestType::NONE;
        Handle              m_device_handle = 0;
    };

    /// @brief An IO request for USB control transfers, USB 3.2 §4.4.5.
    struct ControlTransferRequest {
        TransferRequestHeader m_header;
        U8                    m_request_type = 0; // bmRequestType
        U8                    m_request      = 0; // bRequest
        U16                   m_value        = 0; // wValue
        U16                   m_index        = 0; // wIndex
        U16                   m_length       = 0; // wLength
    };

    /// @brief An IO request for USB Bulk, Interrupt and Isochronous transfers.
    struct DataTransferRequest {
        TransferRequestHeader m_header;
        U8                    m_endpoint_number = 0;               // bEndpointAddress bits 3..0
        Direction             m_direction       = Direction::NONE; // bEndpointAddress bit 7
        U32                   m_length          = 0;               // bytes to transfer

        // Isochronous only:
        /// @brief SIA — schedule the TD on the next available Isoch service interval. When true,
        ///         m_isoch_frame_id is ignored.
        bool m_isoch_start_asap = true;
        /// @brief Target (micro)frame for the isochronous TD when m_isoch_start_asap == false.
        U16 m_isoch_frame_id = 0;
    };
} // namespace Rune::Device::USB

#endif // RUNEOS_USB_H
