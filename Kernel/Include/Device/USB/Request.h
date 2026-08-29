
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

#ifndef RUNEOS_USB_REQUEST_H
#define RUNEOS_USB_REQUEST_H

#include <Ember/Ember.h>
#include <Ember/Enum.h>

namespace Rune::Device::USB {

    // ========================================================================================== //
    // Request Types (bmRequestType) - USB 3.2 §9.3, Table 9-3
    // ========================================================================================== //

#define REQUEST_TYPES(X)                                                                           \
    X(RequestType, DIRECTION_HOST_TO_DEVICE, 0b00000000)                                           \
    X(RequestType, DIRECTION_DEVICE_TO_HOST, 0b10000000)                                           \
    X(RequestType, TYPE_STANDARD, 0b00000000)                                                      \
    X(RequestType, TYPE_CLASS, 0b00100000)                                                         \
    X(RequestType, TYPE_VENDOR, 0b01000000)                                                        \
    X(RequestType, RECIPIENT_DEVICE, 0b00000000)                                                   \
    X(RequestType, RECIPIENT_INTERFACE, 0b00000001)                                                \
    X(RequestType, RECIPIENT_ENDPOINT, 0b00000010)                                                 \
    X(RequestType, RECIPIENT_OTHER, 0b00000011)                                                    \
    X(RequestType, RECIPIENT_VENDOR, 0b00011111)

    /// @brief Bit flags composing the bmRequestType field of a Setup packet — USB 3.2 Table 9-3.
    ///
    /// bmRequestType is a bitmap built by OR-ing one value from each of the three groups below:
    /// Direction (bit 7), Type (bits 6..5), and Recipient (bits 4..0).
    ///
    /// - DIRECTION_HOST_TO_DEVICE (bit 7 = 0): Data stage, if any, transfers host to device.
    /// - DIRECTION_DEVICE_TO_HOST (bit 7 = 1): Data stage, if any, transfers device to host.
    /// - TYPE_STANDARD (bits 6..5 = 0): Request defined by the USB core specification.
    /// - TYPE_CLASS (bits 6..5 = 1): Request defined by a USB device class specification.
    /// - TYPE_VENDOR (bits 6..5 = 2): Request defined by the device vendor.
    /// - RECIPIENT_DEVICE (bits 4..0 = 0): Request targets the device as a whole.
    /// - RECIPIENT_INTERFACE (bits 4..0 = 1): Request targets an interface, identified in wIndex.
    /// - RECIPIENT_ENDPOINT (bits 4..0 = 2): Request targets an endpoint, identified in wIndex.
    /// - RECIPIENT_OTHER (bits 4..0 = 3): Request targets an element not covered by the other
    /// recipients.
    /// - RECIPIENT_VENDOR (bits 4..0 = 31): Vendor specific recipient.
    DECLARE_TYPED_ENUM(RequestType, U8, REQUEST_TYPES, 0x0) // NOLINT

    /// @brief Bit mask to get the direction from bmRequestType.
    constexpr U8 REQUEST_TYPE_DIRECTION_MASK = 0b10000000;
    /// @brief Bit mask to get the type from bmRequestType.
    constexpr U8 REQUEST_TYPE_TYPE_MASK = 0b01100000;
    /// @brief Bit mask to get the recipient from bmRequestType.
    constexpr U8 REQUEST_TYPE_RECIPIENT_MASK = 0b00011111;

    // ========================================================================================== //
    // Standard Request Codes (bRequest) — USB 3.2 §9.4, Table 9-5
    // ========================================================================================== //

#define STANDARD_REQUEST_CODES(X)                                                                  \
    X(StandardRequestCode, GET_STATUS, 0x00)                                                       \
    X(StandardRequestCode, CLEAR_FEATURE, 0x01)                                                    \
    X(StandardRequestCode, SET_FEATURE, 0x03)                                                      \
    X(StandardRequestCode, SET_ADDRESS, 0x05)                                                      \
    X(StandardRequestCode, GET_DESCRIPTOR, 0x06)                                                   \
    X(StandardRequestCode, SET_DESCRIPTOR, 0x07)                                                   \
    X(StandardRequestCode, GET_CONFIGURATION, 0x08)                                                \
    X(StandardRequestCode, SET_CONFIGURATION, 0x09)                                                \
    X(StandardRequestCode, GET_INTERFACE, 0x0A)                                                    \
    X(StandardRequestCode, SET_INTERFACE, 0x0B)                                                    \
    X(StandardRequestCode, SYNCH_FRAME, 0x0C)                                                      \
    X(StandardRequestCode, SET_ENCRYPTION, 0x0D)                                                   \
    X(StandardRequestCode, GET_ENCRYPTION, 0x0E)                                                   \
    X(StandardRequestCode, SET_HANDSHAKE, 0x0F)                                                    \
    X(StandardRequestCode, GET_HANDSHAKE, 0x10)                                                    \
    X(StandardRequestCode, SET_CONNECTION, 0x11)                                                   \
    X(StandardRequestCode, SET_SECURITY_DATA, 0x12)                                                \
    X(StandardRequestCode, GET_SECURITY_DATA, 0x13)                                                \
    X(StandardRequestCode, SET_WUSB_DATA, 0x14)                                                    \
    X(StandardRequestCode, LOOPBACK_DATA_WRITE, 0x15)                                              \
    X(StandardRequestCode, LOOPBACK_DATA_READ, 0x16)                                               \
    X(StandardRequestCode, SET_INTERFACE_DS, 0x17)                                                 \
    X(StandardRequestCode, GET_FW_STATUS, 0x1A)                                                    \
    X(StandardRequestCode, SET_FW_STATUS, 0x1B)                                                    \
    X(StandardRequestCode, SET_SEL, 0x30)                                                          \
    X(StandardRequestCode, SET_ISOCH_DELAY, 0x31)

    /// @brief Standard device request codes (bRequest field) — USB 3.2 Table 9-5.
    ///
    /// - GET_STATUS (0x00): Returns status for device, interface, or endpoint.
    /// - CLEAR_FEATURE (0x01): Clears or disables a feature.
    /// - SET_FEATURE (0x03): Sets or enables a feature.
    /// - SET_ADDRESS (0x05): Sets the device address.
    /// - GET_DESCRIPTOR (0x06): Returns the specified descriptor.
    /// - SET_DESCRIPTOR (0x07): Updates or adds a descriptor.
    /// - GET_CONFIGURATION (0x08): Returns the current configuration value.
    /// - SET_CONFIGURATION (0x09): Sets the device configuration.
    /// - GET_INTERFACE (0x0A): Returns the selected alternate setting.
    /// - SET_INTERFACE (0x0B): Selects an alternate setting for an interface.
    /// - SYNCH_FRAME (0x0C): Returns the frame number for isochronous sync.
    /// - SET_ENCRYPTION (0x0D): Wireless USB.
    /// - GET_ENCRYPTION (0x0E): Wireless USB.
    /// - SET_HANDSHAKE (0x0F): Wireless USB.
    /// - GET_HANDSHAKE (0x10): Wireless USB.
    /// - SET_CONNECTION (0x11): Wireless USB.
    /// - SET_SECURITY_DATA (0x12): Wireless USB.
    /// - GET_SECURITY_DATA (0x13): Wireless USB.
    /// - SET_WUSB_DATA (0x14): Wireless USB.
    /// - LOOPBACK_DATA_WRITE (0x15): Wireless USB.
    /// - LOOPBACK_DATA_READ (0x16): Wireless USB.
    /// - SET_INTERFACE_DS (0x17): Wireless USB.
    /// - GET_FW_STATUS (0x1A): Returns firmware status.
    /// - SET_FW_STATUS (0x1B): Sets firmware status.
    /// - SET_SEL (0x30): Sets system exit latency values (USB 3.x).
    /// - SET_ISOCH_DELAY (0x31): Sets isochronous delay in the host (USB 3.x).
    DECLARE_TYPED_ENUM(StandardRequestCode, U8, STANDARD_REQUEST_CODES, 0xFF) // NOLINT

    // ========================================================================================== //
    // Descriptor Types — USB 3.2 §9.4, Table 9-6
    // ========================================================================================== //

#define DESCRIPTOR_TYPES(X)                                                                        \
    X(DescriptorType, DEVICE, 0x01)                                                                \
    X(DescriptorType, CONFIGURATION, 0x02)                                                         \
    X(DescriptorType, STRING, 0x03)                                                                \
    X(DescriptorType, INTERFACE, 0x04)                                                             \
    X(DescriptorType, ENDPOINT, 0x05)                                                              \
    X(DescriptorType, INTERFACE_POWER, 0x08)                                                       \
    X(DescriptorType, OTG, 0x09)                                                                   \
    X(DescriptorType, DEBUG, 0x0A)                                                                 \
    X(DescriptorType, INTERFACE_ASSOCIATION, 0x0B)                                                 \
    X(DescriptorType, BOS, 0x0F)                                                                   \
    X(DescriptorType, DEVICE_CAPABILITY, 0x10)                                                     \
    X(DescriptorType, SUPERSPEED_USB_ENDPOINT_COMPANION, 0x30)                                     \
    X(DescriptorType, SUPERSPEEDPLUS_ISOCHRONOUS_ENDPOINT_COMPANION, 0x31)

    /// @brief Descriptor type codes — USB 3.2 Table 9-6.
    ///
    /// Used in the high byte of wValue for GET_DESCRIPTOR / SET_DESCRIPTOR,
    /// and in bDescriptorType of every descriptor header.
    ///
    /// - DEVICE (0x01): Device Descriptor.
    /// - CONFIGURATION (0x02): Configuration Descriptor.
    /// - STRING (0x03): String Descriptor.
    /// - INTERFACE (0x04): Interface Descriptor.
    /// - ENDPOINT (0x05): Endpoint Descriptor.
    /// - INTERFACE_POWER (0x08): Interface Power Descriptor.
    /// - OTG (0x09): OTG Descriptor.
    /// - DEBUG (0x0A): Debug Descriptor.
    /// - INTERFACE_ASSOCIATION (0x0B): Interface Association Descriptor.
    /// - BOS (0x0F): Binary Object Store Descriptor.
    /// - DEVICE_CAPABILITY (0x10): Device Capability Descriptor.
    /// - SUPERSPEED_USB_ENDPOINT_COMPANION (0x30): SuperSpeed Endpoint Companion.
    /// - SUPERSPEEDPLUS_ISOCHRONOUS_ENDPOINT_COMPANION (0x31): SuperSpeedPlus Isochronous
    /// Companion.
    DECLARE_TYPED_ENUM(DescriptorType, U8, DESCRIPTOR_TYPES, 0xFF) // NOLINT

    // ========================================================================================== //
    // Standard Feature Selectors — USB 3.2 §9.4, Table 9-7
    // ========================================================================================== //

#define STANDARD_FEATURE_SELECTORS(X)                                                              \
    X(StandardFeatureSelector, ENDPOINT_HALT, 0x00)                                                \
    X(StandardFeatureSelector, FUNCTION_SUSPEND, 0x00)                                             \
    X(StandardFeatureSelector, DEVICE_REMOTE_WAKEUP, 0x01)                                         \
    X(StandardFeatureSelector, TEST_MODE, 0x02)                                                    \
    X(StandardFeatureSelector, B_HNP_ENABLE, 0x03)                                                 \
    X(StandardFeatureSelector, A_HNP_SUPPORT, 0x04)                                                \
    X(StandardFeatureSelector, A_ALT_HNP_SUPPORT, 0x05)                                            \
    X(StandardFeatureSelector, WUSB_DEVICE, 0x06)                                                  \
    X(StandardFeatureSelector, U1_ENABLE, 0x30)                                                    \
    X(StandardFeatureSelector, U2_ENABLE, 0x31)                                                    \
    X(StandardFeatureSelector, LTM_ENABLE, 0x32)                                                   \
    X(StandardFeatureSelector, B3_NTF_HOST_REL, 0x33)                                              \
    X(StandardFeatureSelector, B3_RSP_ENABLE, 0x34)                                                \
    X(StandardFeatureSelector, LDM_ENABLE, 0x35)

    /// @brief Standard feature selector values (wValue for SET_FEATURE / CLEAR_FEATURE) —
    ///        USB 3.2 Table 9-7.
    ///
    /// Note: ENDPOINT_HALT and FUNCTION_SUSPEND share value 0x00 but target different
    ///       recipients — Endpoint and Interface, respectively.
    ///
    /// - ENDPOINT_HALT (0x00): Halt an endpoint (Endpoint recipient).
    /// - FUNCTION_SUSPEND (0x00): Suspend a function (Interface recipient).
    /// - DEVICE_REMOTE_WAKEUP (0x01): Enable remote wakeup (Device recipient).
    /// - TEST_MODE (0x02): Enter test mode (Device recipient).
    /// - B_HNP_ENABLE (0x03): OTG HNP enable (Device recipient).
    /// - A_HNP_SUPPORT (0x04): OTG A-device HNP support (Device recipient).
    /// - A_ALT_HNP_SUPPORT (0x05): OTG alternate HNP support (Device recipient).
    /// - WUSB_DEVICE (0x06): Wireless USB device (Device recipient).
    /// - U1_ENABLE (0x30): Enable U1 power state (Device recipient).
    /// - U2_ENABLE (0x31): Enable U2 power state (Device recipient).
    /// - LTM_ENABLE (0x32): Enable Latency Tolerance Messaging (Device recipient).
    /// - B3_NTF_HOST_REL (0x33): OTG use only (Device recipient).
    /// - B3_RSP_ENABLE (0x34): OTG use only (Device recipient).
    /// - LDM_ENABLE (0x35): Enable Precision Time Measurement LDM (Device recipient).
    DECLARE_TYPED_ENUM(StandardFeatureSelector, U8, STANDARD_FEATURE_SELECTORS, 0xFF) // NOLINT

} // namespace Rune::Device::USB

#endif // RUNEOS_USB_REQUEST_H