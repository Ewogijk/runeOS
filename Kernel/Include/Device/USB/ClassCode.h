
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

#ifndef RUNEOS_USB_CLASSCODE_H
#define RUNEOS_USB_CLASSCODE_H

#include <Ember/Ember.h>
#include <Ember/Enum.h>

#include <KRE/String.h>

namespace Rune::Device::USB {

    // ========================================================================================== //
    // Base Class Codes (bDeviceClass / bInterfaceClass) — USB-IF Defined Class Codes
    // ========================================================================================== //

#define CLASS_CODES(X)                                                                             \
    X(ClassCode, USE_INTERFACE_DESCRIPTOR, 0x00)                                                   \
    X(ClassCode, AUDIO, 0x01)                                                                      \
    X(ClassCode, CDC_CONTROL, 0x02)                                                                \
    X(ClassCode, HID, 0x03)                                                                        \
    X(ClassCode, PHYSICAL, 0x05)                                                                   \
    X(ClassCode, IMAGE, 0x06)                                                                      \
    X(ClassCode, PRINTER, 0x07)                                                                    \
    X(ClassCode, MASS_STORAGE, 0x08)                                                               \
    X(ClassCode, HUB, 0x09)                                                                        \
    X(ClassCode, CDC_DATA, 0x0A)                                                                   \
    X(ClassCode, SMART_CARD, 0x0B)                                                                 \
    X(ClassCode, CONTENT_SECURITY, 0x0D)                                                           \
    X(ClassCode, VIDEO, 0x0E)                                                                      \
    X(ClassCode, PERSONAL_HEALTHCARE, 0x0F)                                                        \
    X(ClassCode, AUDIO_VIDEO, 0x10)                                                                \
    X(ClassCode, BILLBOARD, 0x11)                                                                  \
    X(ClassCode, USB_TYPE_C_BRIDGE, 0x12)                                                          \
    X(ClassCode, I3C, 0x3C)                                                                        \
    X(ClassCode, DIAGNOSTIC, 0xDC)                                                                 \
    X(ClassCode, WIRELESS_CONTROLLER, 0xE0)                                                        \
    X(ClassCode, MISCELLANEOUS, 0xEF)                                                              \
    X(ClassCode, APPLICATION_SPECIFIC, 0xFE)                                                       \
    X(ClassCode, VENDOR_SPECIFIC, 0xFF)

    /// @brief USB base class code (bDeviceClass / bInterfaceClass).
    ///
    /// - USE_INTERFACE_DESCRIPTOR (0x00): Class defined at the interface level.
    /// - AUDIO (0x01): Audio device.
    /// - CDC_CONTROL (0x02): Communications and CDC Control.
    /// - HID (0x03): Human Interface Device.
    /// - PHYSICAL (0x05): Physical device.
    /// - IMAGE (0x06): Still Imaging device.
    /// - PRINTER (0x07): Printer.
    /// - MASS_STORAGE (0x08): Mass Storage.
    /// - HUB (0x09): USB Hub.
    /// - CDC_DATA (0x0A): CDC-Data.
    /// - SMART_CARD (0x0B): Smart Card.
    /// - CONTENT_SECURITY (0x0D): Content Security.
    /// - VIDEO (0x0E): Video.
    /// - PERSONAL_HEALTHCARE (0x0F): Personal Healthcare.
    /// - AUDIO_VIDEO (0x10): Audio/Video Devices.
    /// - BILLBOARD (0x11): Billboard Device.
    /// - USB_TYPE_C_BRIDGE (0x12): USB Type-C Bridge.
    /// - I3C (0x3C): I3C Device.
    /// - DIAGNOSTIC (0xDC): Diagnostic Device.
    /// - WIRELESS_CONTROLLER (0xE0): Wireless Controller (Bluetooth, UWB).
    /// - MISCELLANEOUS (0xEF): Miscellaneous (IAD, RNDIS, …).
    /// - APPLICATION_SPECIFIC (0xFE): Application Specific (DFU, IrDA, …).
    /// - VENDOR_SPECIFIC (0xFF): Vendor Specific.
    DECLARE_TYPED_ENUM(ClassCode, U8, CLASS_CODES, 0x00) // NOLINT

    // ========================================================================================== //
    // HID (0x03) — subclass and protocol codes
    // ========================================================================================== //

#define HID_SUBCLASS_CODES(X) X(HIDSubClass, BOOT_INTERFACE, 0x01)

    /// @brief Subclass codes for USB HID (class 0x03).
    ///
    /// - NONE (0x00): No subclass (auto-generated).
    /// - BOOT_INTERFACE (0x01): Boot Interface Subclass (keyboard / mouse).
    DECLARE_TYPED_ENUM(HIDSubClass, U8, HID_SUBCLASS_CODES, 0x00) // NOLINT

#define HID_PROTOCOL_CODES(X)                                                                      \
    X(HIDProtocol, KEYBOARD, 0x01)                                                                 \
    X(HIDProtocol, MOUSE, 0x02)

    /// @brief Protocol codes for USB HID Boot Interface (subclass 0x01).
    ///
    /// - NONE (0x00): None (auto-generated).
    /// - KEYBOARD (0x01): Keyboard.
    /// - MOUSE (0x02): Mouse.
    DECLARE_TYPED_ENUM(HIDProtocol, U8, HID_PROTOCOL_CODES, 0x00) // NOLINT

    // ========================================================================================== //
    // Mass Storage (0x08) — subclass and protocol codes
    // ========================================================================================== //

#define MASS_STORAGE_SUBCLASS_CODES(X)                                                             \
    X(MassStorageSubClass, RBC, 0x01)                                                              \
    X(MassStorageSubClass, ATAPI, 0x02)                                                            \
    X(MassStorageSubClass, UFI, 0x04)                                                              \
    X(MassStorageSubClass, SCSI, 0x06)                                                             \
    X(MassStorageSubClass, LSD_FS, 0x07)                                                           \
    X(MassStorageSubClass, IEEE_1667, 0x08)                                                        \
    X(MassStorageSubClass, VENDOR, 0xFF)

    /// @brief Subclass codes for USB Mass Storage (class 0x08).
    ///
    /// - RBC (0x01): Reduced Block Commands (flash drives).
    /// - ATAPI (0x02): CD/DVD drives.
    /// - UFI (0x04): Floppy drives.
    /// - SCSI (0x06): SCSI transparent command set (most common).
    /// - LSD_FS (0x07): LSD FS.
    /// - IEEE_1667 (0x08): IEEE 1667.
    /// - VENDOR (0xFF): Vendor Specific.
    DECLARE_TYPED_ENUM(MassStorageSubClass, U8, MASS_STORAGE_SUBCLASS_CODES, 0x00) // NOLINT

#define MASS_STORAGE_PROTOCOL_CODES(X)                                                             \
    X(MassStorageProtocol, CBI_WITH_INTERRUPT, 0x00)                                               \
    X(MassStorageProtocol, CBI_NO_INTERRUPT, 0x01)                                                 \
    X(MassStorageProtocol, BULK_ONLY_TRANSPORT, 0x50)                                              \
    X(MassStorageProtocol, UAS, 0x62)                                                              \
    X(MassStorageProtocol, VENDOR, 0xFF)

    /// @brief Protocol codes for USB Mass Storage (class 0x08).
    ///
    /// - CBI_WITH_INTERRUPT (0x00): CBI Transport with completion interrupt.
    /// - CBI_NO_INTERRUPT (0x01): CBI Transport without completion interrupt.
    /// - BULK_ONLY_TRANSPORT (0x50): Bulk-Only Transport (BOT/BBB) — most common.
    /// - UAS (0x62): USB Attached SCSI.
    /// - VENDOR (0xFF): Vendor Specific.
    DECLARE_TYPED_ENUM(MassStorageProtocol, U8, MASS_STORAGE_PROTOCOL_CODES, 0x00) // NOLINT

    // ========================================================================================== //
    // Hub (0x09) — protocol codes (subclass is always 0x00)
    // ========================================================================================== //

#define HUB_PROTOCOL_CODES(X)                                                                      \
    X(HubProtocol, FULL_SPEED, 0x00)                                                               \
    X(HubProtocol, HI_SPEED_SINGLE_TT, 0x01)                                                       \
    X(HubProtocol, HI_SPEED_MULTI_TT, 0x02)                                                        \
    X(HubProtocol, SUPERSPEED, 0x03)

    /// @brief Protocol codes for USB Hub (class 0x09, subclass 0x00).
    ///
    /// - FULL_SPEED (0x00): Full-speed / Low-speed Hub.
    /// - HI_SPEED_SINGLE_TT (0x01): Hi-Speed Hub with single Transaction Translator.
    /// - HI_SPEED_MULTI_TT (0x02): Hi-Speed Hub with multiple Transaction Translators.
    /// - SUPERSPEED (0x03): SuperSpeed Hub.
    DECLARE_TYPED_ENUM(HubProtocol, U8, HUB_PROTOCOL_CODES, 0x00) // NOLINT

    // ========================================================================================== //
    // CDC Control (0x02) — subclass codes
    // ========================================================================================== //

#define CDC_SUBCLASS_CODES(X)                                                                      \
    X(CDCSubClass, DIRECT_LINE_CONTROL, 0x01)                                                      \
    X(CDCSubClass, ABSTRACT_CONTROL, 0x02)                                                         \
    X(CDCSubClass, TELEPHONE_CONTROL, 0x03)                                                        \
    X(CDCSubClass, MULTI_CHANNEL_CONTROL, 0x04)                                                    \
    X(CDCSubClass, CAPI_CONTROL, 0x05)                                                             \
    X(CDCSubClass, ETHERNET_NETWORKING, 0x06)                                                      \
    X(CDCSubClass, ATM_NETWORKING, 0x07)                                                           \
    X(CDCSubClass, WIRELESS_HANDSET_CONTROL, 0x08)                                                 \
    X(CDCSubClass, DEVICE_MANAGEMENT, 0x09)                                                        \
    X(CDCSubClass, MOBILE_DIRECT_LINE, 0x0A)                                                       \
    X(CDCSubClass, OBEX, 0x0B)                                                                     \
    X(CDCSubClass, ETHERNET_EMULATION, 0x0C)                                                       \
    X(CDCSubClass, NETWORK_CONTROL, 0x0D)

    /// @brief Subclass codes for CDC Control (class 0x02).
    ///
    /// - DIRECT_LINE_CONTROL (0x01): Direct Line Control Model.
    /// - ABSTRACT_CONTROL (0x02): Abstract Control Model (virtual serial port / ACM).
    /// - TELEPHONE_CONTROL (0x03): Telephone Control Model.
    /// - MULTI_CHANNEL_CONTROL (0x04): Multi-Channel Control Model.
    /// - CAPI_CONTROL (0x05): CAPI Control Model.
    /// - ETHERNET_NETWORKING (0x06): Ethernet Networking Control Model.
    /// - ATM_NETWORKING (0x07): ATM Networking Control Model.
    /// - WIRELESS_HANDSET_CONTROL (0x08): Wireless Handset Control Model.
    /// - DEVICE_MANAGEMENT (0x09): Device Management.
    /// - MOBILE_DIRECT_LINE (0x0A): Mobile Direct Line Model.
    /// - OBEX (0x0B): OBEX.
    /// - ETHERNET_EMULATION (0x0C): Ethernet Emulation Model.
    /// - NETWORK_CONTROL (0x0D): Network Control Model.
    DECLARE_TYPED_ENUM(CDCSubClass, U8, CDC_SUBCLASS_CODES, 0x00) // NOLINT

    // ========================================================================================== //
    // Wireless Controller (0xE0) — subclass and protocol codes
    // ========================================================================================== //

#define WIRELESS_SUBCLASS_CODES(X)                                                                 \
    X(WirelessSubClass, RADIO_FREQUENCY, 0x01)                                                     \
    X(WirelessSubClass, USB_WIRE_ADAPTER, 0x02)

    /// @brief Subclass codes for Wireless Controller (class 0xE0).
    ///
    /// - RADIO_FREQUENCY (0x01): RF Controller (Bluetooth, UWB, …).
    /// - USB_WIRE_ADAPTER (0x02): USB Wire Adapter.
    DECLARE_TYPED_ENUM(WirelessSubClass, U8, WIRELESS_SUBCLASS_CODES, 0x00) // NOLINT

#define WIRELESS_RF_PROTOCOL_CODES(X)                                                              \
    X(WirelessRFProtocol, BLUETOOTH, 0x01)                                                         \
    X(WirelessRFProtocol, UWB_RADIO_CONTROL, 0x02)                                                 \
    X(WirelessRFProtocol, REMOTE_NDIS, 0x03)                                                       \
    X(WirelessRFProtocol, BLUETOOTH_AMP, 0x04)

    /// @brief Protocol codes for Wireless RF Controller (class 0xE0, subclass 0x01).
    ///
    /// - BLUETOOTH (0x01): Bluetooth Programming Interface.
    /// - UWB_RADIO_CONTROL (0x02): UWB Radio Control Interface.
    /// - REMOTE_NDIS (0x03): Remote NDIS.
    /// - BLUETOOTH_AMP (0x04): Bluetooth AMP Controller.
    DECLARE_TYPED_ENUM(WirelessRFProtocol, U8, WIRELESS_RF_PROTOCOL_CODES, 0x00) // NOLINT

    // ========================================================================================== //
    // Application Specific (0xFE) — subclass codes
    // ========================================================================================== //

#define APPLICATION_SPECIFIC_SUBCLASS_CODES(X)                                                     \
    X(AppSpecificSubClass, DFU, 0x01)                                                              \
    X(AppSpecificSubClass, IRDA_BRIDGE, 0x02)                                                      \
    X(AppSpecificSubClass, TEST_MEASURE, 0x03)

    /// @brief Subclass codes for Application Specific (class 0xFE).
    ///
    /// - DFU (0x01): Device Firmware Upgrade.
    /// - IRDA_BRIDGE (0x02): IrDA Bridge Device.
    /// - TEST_MEASURE (0x03): USB Test and Measurement Device.
    DECLARE_TYPED_ENUM(AppSpecificSubClass, U8, APPLICATION_SPECIFIC_SUBCLASS_CODES, 0x00) // NOLINT

    /// @brief Try to resolve the subclass_code to the subclass matching with the given class_code.
    /// @param class_code    Base class code (bDeviceClass / bInterfaceClass).
    /// @param subclass_code Subclass code.
    /// @return Name of the subclass code or "NONE" if subclass_code does not encode a valid
    ///         subclass.
    auto subclass_code_resolve(ClassCode class_code, U8 subclass_code) -> String;

    /// @brief Try to resolve the protocol_code to the protocol matching with the given class_code
    ///         and subclass_code combination.
    /// @param class_code    Base class code (bDeviceClass / bInterfaceClass).
    /// @param subclass_code Subclass code.
    /// @param protocol_code Protocol code.
    /// @return Name of the protocol code or "NONE" if protocol_code does not encode a valid
    ///         protocol.
    auto protocol_code_resolve(ClassCode class_code, U8 subclass_code, U8 protocol_code)
        -> String;

} // namespace Rune::Device::USB

#endif // RUNEOS_USB_CLASSCODE_H