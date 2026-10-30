
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
    X(USE_INTERFACE_DESCRIPTOR, 0x00)                                                              \
    X(AUDIO, 0x01)                                                                                 \
    X(CDC_CONTROL, 0x02)                                                                           \
    X(HID, 0x03)                                                                                   \
    X(PHYSICAL, 0x05)                                                                              \
    X(IMAGE, 0x06)                                                                                 \
    X(PRINTER, 0x07)                                                                               \
    X(MASS_STORAGE, 0x08)                                                                          \
    X(HUB, 0x09)                                                                                   \
    X(CDC_DATA, 0x0A)                                                                              \
    X(SMART_CARD, 0x0B)                                                                            \
    X(CONTENT_SECURITY, 0x0D)                                                                      \
    X(VIDEO, 0x0E)                                                                                 \
    X(PERSONAL_HEALTHCARE, 0x0F)                                                                   \
    X(AUDIO_VIDEO, 0x10)                                                                           \
    X(BILLBOARD, 0x11)                                                                             \
    X(USB_TYPE_C_BRIDGE, 0x12)                                                                     \
    X(I3C, 0x3C)                                                                                   \
    X(DIAGNOSTIC, 0xDC)                                                                            \
    X(WIRELESS_CONTROLLER, 0xE0)                                                                   \
    X(MISCELLANEOUS, 0xEF)                                                                         \
    X(APPLICATION_SPECIFIC, 0xFE)                                                                  \
    X(VENDOR_SPECIFIC, 0xFF)

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
    EMBER_DEFINE_SMART_ENUM(ClassCode, CLASS_CODES, 0x00, U8) // NOLINT

    // ========================================================================================== //
    // HID (0x03) — subclass and protocol codes
    // ========================================================================================== //

#define HID_SUBCLASS_CODES(X) X(BOOT_INTERFACE, 0x01)

    /// @brief Subclass codes for USB HID (class 0x03).
    ///
    /// - NONE (0x00): No subclass (auto-generated).
    /// - BOOT_INTERFACE (0x01): Boot Interface Subclass (keyboard / mouse).
    EMBER_DEFINE_SMART_ENUM(HIDSubClass, HID_SUBCLASS_CODES, 0x00, U8) // NOLINT

#define HID_PROTOCOL_CODES(X)                                                                      \
    X(KEYBOARD, 0x01)                                                                              \
    X(MOUSE, 0x02)

    /// @brief Protocol codes for USB HID Boot Interface (subclass 0x01).
    ///
    /// - NONE (0x00): None (auto-generated).
    /// - KEYBOARD (0x01): Keyboard.
    /// - MOUSE (0x02): Mouse.
    EMBER_DEFINE_SMART_ENUM(HIDProtocol, HID_PROTOCOL_CODES, 0x00, U8) // NOLINT

    // ========================================================================================== //
    // Mass Storage (0x08) — subclass and protocol codes
    // ========================================================================================== //

#define MASS_STORAGE_SUBCLASS_CODES(X)                                                             \
    X(RBC, 0x01)                                                                                   \
    X(ATAPI, 0x02)                                                                                 \
    X(UFI, 0x04)                                                                                   \
    X(SCSI, 0x06)                                                                                  \
    X(LSD_FS, 0x07)                                                                                \
    X(IEEE_1667, 0x08)                                                                             \
    X(VENDOR, 0xFF)

    /// @brief Subclass codes for USB Mass Storage (class 0x08).
    ///
    /// - RBC (0x01): Reduced Block Commands (flash drives).
    /// - ATAPI (0x02): CD/DVD drives.
    /// - UFI (0x04): Floppy drives.
    /// - SCSI (0x06): SCSI transparent command set (most common).
    /// - LSD_FS (0x07): LSD FS.
    /// - IEEE_1667 (0x08): IEEE 1667.
    /// - VENDOR (0xFF): Vendor Specific.
    EMBER_DEFINE_SMART_ENUM(MassStorageSubClass, MASS_STORAGE_SUBCLASS_CODES, 0x00, U8) // NOLINT

#define MASS_STORAGE_PROTOCOL_CODES(X)                                                             \
    X(CBI_WITH_INTERRUPT, 0x00)                                                                    \
    X(CBI_NO_INTERRUPT, 0x01)                                                                      \
    X(BULK_ONLY_TRANSPORT, 0x50)                                                                   \
    X(UAS, 0x62)                                                                                   \
    X(VENDOR, 0xFF)

    /// @brief Protocol codes for USB Mass Storage (class 0x08).
    ///
    /// - CBI_WITH_INTERRUPT (0x00): CBI Transport with completion interrupt.
    /// - CBI_NO_INTERRUPT (0x01): CBI Transport without completion interrupt.
    /// - BULK_ONLY_TRANSPORT (0x50): Bulk-Only Transport (BOT/BBB) — most common.
    /// - UAS (0x62): USB Attached SCSI.
    /// - VENDOR (0xFF): Vendor Specific.
    EMBER_DEFINE_SMART_ENUM(MassStorageProtocol, MASS_STORAGE_PROTOCOL_CODES, 0x00, U8) // NOLINT

    // ========================================================================================== //
    // Hub (0x09) — protocol codes (subclass is always 0x00)
    // ========================================================================================== //

#define HUB_PROTOCOL_CODES(X)                                                                      \
    X(FULL_SPEED, 0x00)                                                                            \
    X(HI_SPEED_SINGLE_TT, 0x01)                                                                    \
    X(HI_SPEED_MULTI_TT, 0x02)                                                                     \
    X(SUPERSPEED, 0x03)

    /// @brief Protocol codes for USB Hub (class 0x09, subclass 0x00).
    ///
    /// - FULL_SPEED (0x00): Full-speed / Low-speed Hub.
    /// - HI_SPEED_SINGLE_TT (0x01): Hi-Speed Hub with single Transaction Translator.
    /// - HI_SPEED_MULTI_TT (0x02): Hi-Speed Hub with multiple Transaction Translators.
    /// - SUPERSPEED (0x03): SuperSpeed Hub.
    EMBER_DEFINE_SMART_ENUM(HubProtocol, HUB_PROTOCOL_CODES, 0x00, U8) // NOLINT

    // ========================================================================================== //
    // CDC Control (0x02) — subclass codes
    // ========================================================================================== //

#define CDC_SUBCLASS_CODES(X)                                                                      \
    X(DIRECT_LINE_CONTROL, 0x01)                                                                   \
    X(ABSTRACT_CONTROL, 0x02)                                                                      \
    X(TELEPHONE_CONTROL, 0x03)                                                                     \
    X(MULTI_CHANNEL_CONTROL, 0x04)                                                                 \
    X(CAPI_CONTROL, 0x05)                                                                          \
    X(ETHERNET_NETWORKING, 0x06)                                                                   \
    X(ATM_NETWORKING, 0x07)                                                                        \
    X(WIRELESS_HANDSET_CONTROL, 0x08)                                                              \
    X(DEVICE_MANAGEMENT, 0x09)                                                                     \
    X(MOBILE_DIRECT_LINE, 0x0A)                                                                    \
    X(OBEX, 0x0B)                                                                                  \
    X(ETHERNET_EMULATION, 0x0C)                                                                    \
    X(NETWORK_CONTROL, 0x0D)

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
    EMBER_DEFINE_SMART_ENUM(CDCSubClass, CDC_SUBCLASS_CODES, 0x00, U8) // NOLINT

    // ========================================================================================== //
    // Wireless Controller (0xE0) — subclass and protocol codes
    // ========================================================================================== //

#define WIRELESS_SUBCLASS_CODES(X)                                                                 \
    X(RADIO_FREQUENCY, 0x01)                                                                       \
    X(USB_WIRE_ADAPTER, 0x02)

    /// @brief Subclass codes for Wireless Controller (class 0xE0).
    ///
    /// - RADIO_FREQUENCY (0x01): RF Controller (Bluetooth, UWB, …).
    /// - USB_WIRE_ADAPTER (0x02): USB Wire Adapter.
    EMBER_DEFINE_SMART_ENUM(WirelessSubClass, WIRELESS_SUBCLASS_CODES, 0x00, U8) // NOLINT

#define WIRELESS_RF_PROTOCOL_CODES(X)                                                              \
    X(BLUETOOTH, 0x01)                                                                             \
    X(UWB_RADIO_CONTROL, 0x02)                                                                     \
    X(REMOTE_NDIS, 0x03)                                                                           \
    X(BLUETOOTH_AMP, 0x04)

    /// @brief Protocol codes for Wireless RF Controller (class 0xE0, subclass 0x01).
    ///
    /// - BLUETOOTH (0x01): Bluetooth Programming Interface.
    /// - UWB_RADIO_CONTROL (0x02): UWB Radio Control Interface.
    /// - REMOTE_NDIS (0x03): Remote NDIS.
    /// - BLUETOOTH_AMP (0x04): Bluetooth AMP Controller.
    EMBER_DEFINE_SMART_ENUM(WirelessRFProtocol, WIRELESS_RF_PROTOCOL_CODES, 0x00, U8) // NOLINT

    // ========================================================================================== //
    // Application Specific (0xFE) — subclass codes
    // ========================================================================================== //

#define APPLICATION_SPECIFIC_SUBCLASS_CODES(X)                                                     \
    X(DFU, 0x01)                                                                                   \
    X(IRDA_BRIDGE, 0x02)                                                                           \
    X(TEST_MEASURE, 0x03)

    /// @brief Subclass codes for Application Specific (class 0xFE).
    ///
    /// - DFU (0x01): Device Firmware Upgrade.
    /// - IRDA_BRIDGE (0x02): IrDA Bridge Device.
    /// - TEST_MEASURE (0x03): USB Test and Measurement Device.
    EMBER_DEFINE_SMART_ENUM(AppSpecificSubClass,
                            APPLICATION_SPECIFIC_SUBCLASS_CODES,
                            0x00,
                            U8)

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
    auto protocol_code_resolve(ClassCode class_code, U8 subclass_code, U8 protocol_code) -> String;

} // namespace Rune::Device::USB

#endif // RUNEOS_USB_CLASSCODE_H