
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

#ifndef RUNEOS_DESCRIPTOR_H
#define RUNEOS_DESCRIPTOR_H

#include <Ember/Ember.h>
#include <Ember/Enum.h>

#include <KRE/Collections/Array.h>
#include <KRE/String.h>

namespace Rune::Device::USB {

    // ========================================================================================== //
    // USB Device Descriptor — USB 3.2 §9.6.1
    // ========================================================================================== //

    /// @brief General information about a device.
    struct DeviceDescriptor {
        static constexpr U8 SIZE_PARTIAL = 8;
        static constexpr U8 SIZE_FULL    = 18;

        U8  m_length;          // = 18
        U8  m_descriptor_type; // = 1 (DEVICE)
        U16 m_bcd_USB;         // USB spec version (e.g. 0x0200 = USB 2.0)
        U8  m_device_class;
        U8  m_device_subclass;
        U8  m_device_protocol;
        U8  m_max_packet_size0; // EP0 max packet size: 8, 16, 32, or 64
        U16 m_id_vendor;
        U16 m_id_product;
        U16 m_bcd_device;
        U8  m_idx_manufacturer; // string descriptor index
        U8  m_idx_product;
        U8  m_idx_serial_number;
        U8  m_num_configurations;
    };
    // ========================================================================================== //
    // USB Configuration Descriptor — USB 3.2 §9.6.3
    // ========================================================================================== //

    /// @brief Describes one specific device configuration (Table 9-23).
    /// Returned as the header of a variable-length blob from GET_DESCRIPTOR(CONFIGURATION).
    /// The blob also contains all nested Interface, Endpoint, and companion descriptors;
    /// use wTotalLength to allocate the receive buffer.
    struct ConfigurationDescriptor {
        /// @brief Gen X speeds mA units (USB 3.2 §9.6.3).
        static constexpr U8 GEN_X_MAX_POWER_UNIT_mA = 8;
        /// @brief High-speed mode mA units (USB 3.2 §9.6.3).
        static constexpr U8 HS_MAX_POWER_UNIT_mA = 2;

        U8  m_length;          // = 9
        U8  m_descriptor_type; // = 2 (CONFIGURATION)
        U16 m_total_length;    // total bytes returned for this configuration (all descriptors)
        U8  m_num_interfaces;
        U8  m_configuration_value; // argument to SetConfiguration()
        U8  m_idx_configuration;   // string descriptor index
        U8  m_bm_attributes;       // D7=1(reserved), D6=self-powered, D5=remote-wakeup, D4..0=0
        U8  m_max_power;           // bus current in 2 mA units (HS) or 8 mA units (Gen X)

        [[nodiscard]] auto self_powered() const -> bool;
        [[nodiscard]] auto remote_wakeup() const -> bool;

      private:
        static constexpr U8 BM_ATTRIBUTES_SELF_POWERED  = 0x40; // D6
        static constexpr U8 BM_ATTRIBUTES_REMOTE_WAKEUP = 0x20; // D5
    };

    // ========================================================================================== //
    // USB Interface Descriptor — USB 3.2 §9.6.5
    // ========================================================================================== //

    /// @brief Describes one interface within a configuration (Table 9-25).
    /// Always returned as part of GET_DESCRIPTOR(CONFIGURATION); not directly accessible.
    /// bInterfaceClass/SubClass/Protocol hold the actual device class when the Device
    /// Descriptor's bDeviceClass is USE_INTERFACE_DESCRIPTOR (0x00).
    struct InterfaceDescriptor {
        U8 m_length;             // = 9
        U8 m_descriptor_type;    // = 4 (INTERFACE)
        U8 m_interface_number;   // zero-based index of this interface
        U8 m_alternate_setting;  // value for SetInterface(); 0 = default
        U8 m_num_endpoints;      // endpoints used, excluding EP0
        U8 m_interface_class;    // class code (USB-IF)
        U8 m_interface_subclass; // subclass code, qualified by m_interface_class
        U8 m_interface_protocol; // protocol code, qualified by class + subclass
        U8 m_idx_interface;      // string descriptor index
    };
    static_assert(sizeof(InterfaceDescriptor) == 9); // NOLINT

    // ========================================================================================== //
    // USB Interface Association Descriptor — USB 3.2 §9.6.4
    // ========================================================================================== //

    /// @brief Groups a contiguous range of interfaces into one logical function (Table 9-24).
    /// Present in the configuration blob immediately before the interfaces it groups; not
    /// directly accessible via GET_DESCRIPTOR.
    struct InterfaceAssociationDescriptor {
        U8 m_length;          // = 8
        U8 m_descriptor_type; // = 11 (INTERFACE_ASSOCIATION)
        U8 m_first_interface; // bFirstInterface
        U8 m_interface_count; // bInterfaceCount
        U8 m_function_class;
        U8 m_function_subclass;
        U8 m_function_protocol;
        U8 m_idx_function; // string descriptor index
    };
    static_assert(sizeof(InterfaceAssociationDescriptor) == 8); // NOLINT

    // ========================================================================================== //
    // USB Endpoint Descriptor — USB 3.2 §9.6.6
    // ========================================================================================== //

#define ENDPOINT_DIRECTIONS(X)                                                                     \
    X(Direction, OUT, 0)                                                                           \
    X(Direction, IN, 1)

    /// Direction encoded in bEndpointAddress bit 7 (Table 9-26).
    DECLARE_TYPED_ENUM(Direction, U8, ENDPOINT_DIRECTIONS, 0xFF) // NOLINT

#define ENDPOINT_TRANSFER_TYPES(X)                                                                 \
    X(TransferType, CONTROL, 0)                                                                    \
    X(TransferType, ISOCHRONOUS, 1)                                                                \
    X(TransferType, BULK, 2)                                                                       \
    X(TransferType, INTERRUPT, 3)

    /// Transfer type encoded in bmAttributes bits 1..0 (Table 9-26).
    DECLARE_TYPED_ENUM(TransferType, U8, ENDPOINT_TRANSFER_TYPES, 0xFF) // NOLINT

#define ENDPOINT_SYNC_TYPES(X)                                                                     \
    X(SyncType, NO_SYNC, 0)                                                                        \
    X(SyncType, ASYNC, 1)                                                                          \
    X(SyncType, ADAPTIVE, 2)                                                                       \
    X(SyncType, SYNC, 3)

    /// Synchronization type encoded in bmAttributes bits 3..2 (isochronous endpoints only, Table
    /// 9-26).
    DECLARE_TYPED_ENUM(SyncType, U8, ENDPOINT_SYNC_TYPES, 0xFF) // NOLINT

#define ISOCHRONOUS_ENDPOINT_USAGE_TYPES(X)                                                        \
    X(IsochronousUsageType, DATA, 0)                                                               \
    X(IsochronousUsageType, FEEDBACK, 1)                                                           \
    X(IsochronousUsageType, IMPLICIT_FEEDBACK_DATA, 2)

    /// Usage type encoded in bmAttributes bits 5..4 (isochronous endpoints only, Table 9-26).
    DECLARE_TYPED_ENUM(IsochronousUsageType, U8, ISOCHRONOUS_ENDPOINT_USAGE_TYPES, 0xFF) // NOLINT

#define INTERRUPT_ENDPOINT_USAGE_TYPES(X)                                                          \
    X(InterruptUsageType, PERIODDIC, 0)                                                            \
    X(InterruptUsageType, NOTIFICATION, 1)

    /// Usage type encoded in bmAttributes bits 5..4 (interrupt endpoints only, Table 9-26).
    DECLARE_TYPED_ENUM(InterruptUsageType, U8, INTERRUPT_ENDPOINT_USAGE_TYPES, 0xFF) // NOLINT

    /// @brief Describes one endpoint within an interface (Table 9-26).
    /// Always returned as part of GET_DESCRIPTOR(CONFIGURATION); not directly accessible.
    /// Never present for endpoint zero (the default control pipe).
    struct EndpointDescriptor {
        U8 m_length;           // = 7
        U8 m_descriptor_type;  // = 5 (ENDPOINT)
        U8 m_endpoint_address; // bits 3..0 = endpoint number; bit 7 = direction (1=IN, 0=OUT;
                               // ignored for control)
        U8 m_bm_attributes;    // bits 1..0 = transfer type; bits 3..2 = sync type (iso only);
                               // bits 5..4 = usage type (iso/interrupt)
        U16 m_max_packet_size; // max packet size this endpoint can send/receive for this
                               // configuration
        U8 m_interval; // polling interval in 125 μs units; reserved for SuperSpeed bulk/control

        [[nodiscard]] auto endpoint_number() const -> U8 {
            return m_endpoint_address & ENDPOINT_NUMBER_MASK;
        }
        [[nodiscard]] auto direction() const -> Direction {
            return Direction(static_cast<U8>((m_endpoint_address >> DIRECTION_SHIFT) & 0x01));
        }
        [[nodiscard]] auto transfer_type() const -> TransferType {
            return TransferType(static_cast<U8>(m_bm_attributes & TRANSFER_TYPE_MASK));
        }
        /// Valid only when transfer_type() == TransferType::ISOCHRONOUS.
        [[nodiscard]] auto sync_type() const -> SyncType {
            return SyncType(static_cast<U8>((m_bm_attributes >> SYNC_TYPE_SHIFT) & 0x03));
        }
        /// Valid only when transfer_type() == TransferType::ISOCHRONOUS.
        [[nodiscard]] auto isochronous_usage_type() const -> IsochronousUsageType {
            return IsochronousUsageType(
                static_cast<U8>((m_bm_attributes >> USAGE_TYPE_SHIFT) & 0x03));
        }
        /// Valid only when transfer_type() == TransferType::INTERRUPT.
        [[nodiscard]] auto interrupt_usage_type() const -> InterruptUsageType {
            return InterruptUsageType(
                static_cast<U8>((m_bm_attributes >> USAGE_TYPE_SHIFT) & 0x03));
        }

      private:
        static constexpr U8 ENDPOINT_NUMBER_MASK = 0x0F;
        static constexpr U8 DIRECTION_SHIFT      = 7;
        static constexpr U8 TRANSFER_TYPE_MASK   = 0x03;
        static constexpr U8 SYNC_TYPE_SHIFT      = 2;
        static constexpr U8 USAGE_TYPE_SHIFT     = 4;
    } PACKED;
    static_assert(sizeof(EndpointDescriptor) == 7); // NOLINT

    // ========================================================================================== //
    // SuperSpeed Endpoint Companion Descriptor — USB 3.2 §9.6.7
    // ========================================================================================== //

    /// @brief Additional endpoint characteristics returned only by Enhanced SuperSpeed devices
    ///         operating at Gen X speed (Table 9-28). Immediately follows each endpoint
    ///         descriptor (except the default control pipe) in the configuration blob; not
    ///         directly accessible via GET_DESCRIPTOR.
    struct SuperSpeedEndpointCompanionDescriptor {
        U8 m_length;          // = 6
        U8 m_descriptor_type; // = 0x30 (SUPERSPEED_USB_ENDPOINT_COMPANION)
        U8 m_max_burst;       // bMaxBurst: max packets per burst minus one, 0..15; 0 for control
        U8 m_bm_attributes;   // bulk: bits 4..0 MaxStreams; iso: bits 1..0 Mult, bit 7 SSP ISO
                              // Companion; reserved for control/interrupt
        U16 m_bytes_per_interval; // wBytesPerInterval: total bytes per service interval (periodic)

        /// Valid only for bulk endpoints; 0 means the endpoint defines no streams.
        [[nodiscard]] auto max_streams() const -> U8 { return m_bm_attributes & MAX_STREAMS_MASK; }
        /// Zero-based additional packets per service interval; valid only for isochronous
        /// endpoints and only when ssp_iso_companion() is false.
        [[nodiscard]] auto mult() const -> U8 { return m_bm_attributes & MULT_MASK; }
        /// When true a SuperSpeedPlus Isochronous Endpoint Companion follows and mult() must be
        /// ignored. Valid only for isochronous endpoints.
        [[nodiscard]] auto ssp_iso_companion() const -> bool {
            return (m_bm_attributes & SSP_ISO_COMPANION_MASK) != 0;
        }

      private:
        static constexpr U8 MAX_STREAMS_MASK       = 0x1F;
        static constexpr U8 MULT_MASK              = 0x03;
        static constexpr U8 SSP_ISO_COMPANION_MASK = 0x80;
    } PACKED;
    static_assert(sizeof(SuperSpeedEndpointCompanionDescriptor) == 6); // NOLINT

    // ========================================================================================== //
    // USB String Descriptor — USB 3.2 §9.6.9
    // ========================================================================================== //

    /// @brief String index zero for all languages (Table 9-30): returns the array of 2-byte
    ///         LANGID codes the device supports. Not NULL-terminated; a device may omit all
    ///         string descriptors, in which case it shall not return an array of LANGID codes.
    struct StringDescriptorZero {
        static constexpr U8  SIZE_HEADER     = 2;
        static constexpr U16 MAX_BUFFER_SIZE = 256;

        U8                          m_length;          // = LangIdCount * 2 + 2
        U8                          m_descriptor_type; // = 3 (STRING)
        Array<U16, MAX_BUFFER_SIZE> m_lang_id{};       // LANGID codes

        /// @brief Number of LANGID codes actually reported by the device.
        [[nodiscard]] auto lang_id_count() const -> U8;
    };

    /// @brief UNICODE string descriptor (Table 9-31): a UTF16LE encoded string as defined by the
    ///         Unicode Standard, requested for a non-zero string index using a LANGID reported by
    ///         StringDescriptorZero. Not NULL-terminated.
    struct StringDescriptor {
        static constexpr U8  SIZE_HEADER           = 2;
        static constexpr U16 MAX_BUFFER_SIZE       = 256;
        static constexpr U16 ASCII_CODE_UNIT_LIMIT = 0x80;

        U8                          m_length;          // = StringLength + 2
        U8                          m_descriptor_type; // = 3 (STRING)
        Array<U16, MAX_BUFFER_SIZE> m_string{};        // UTF16LE encoded string

        /// @brief Length of the encoded string in bytes (not UTF16 code units).
        [[nodiscard]] auto string_length() const -> U8;

        [[nodiscard]] auto string() const -> String;
    };
} // namespace Rune::Device::USB

#endif // RUNEOS_DESCRIPTOR_H
