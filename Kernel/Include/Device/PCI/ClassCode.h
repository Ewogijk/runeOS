
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

#ifndef RUNEOS_PCI_CLASSCODE_H
#define RUNEOS_PCI_CLASSCODE_H

#include <Ember/Enum.h>

#include <KRE/String.h>

namespace Rune::Device {
#define BASE_CLASS_CODES(X)                                                                        \
    X(ANCIENT, 0x0)                                                                                \
    X(MASS_STORAGE_CONTROLLER, 0x1)                                                                \
    X(NETWORK_CONTROLLER, 0x2)                                                                     \
    X(DISPLAY_CONTROLLER, 0x3)                                                                     \
    X(MULTIMEDIA_DEVICE, 0x4)                                                                      \
    X(MEMORY_CONTROLLER, 0x5)                                                                      \
    X(BRIDGE_DEVICE, 0x6)                                                                          \
    X(SIMPLE_COMMUNICATION_CONTROLLER, 0x7)                                                        \
    X(BASE_SYSTEM_PERIPHERAL, 0x8)                                                                 \
    X(INPUT_DEVICE, 0x9)                                                                           \
    X(DOCKING_STATION, 0xA)                                                                        \
    X(PROCESSOR, 0xB)                                                                              \
    X(SERIAL_BUS_CONTROLLER, 0xC)                                                                  \
    X(WIRELESS_CONTROLLER, 0xD)                                                                    \
    X(INTELLIGENT_IO_CONTROLLER, 0xE)                                                              \
    X(SATELLITE_COMMUNICATION_CONTROLLER, 0xF)                                                     \
    X(ENCRYPTION_DECRYPTION_CONTROLLER, 0x10)                                                      \
    X(DATA_ACQUISITION_AND_SIGNAL_PROCESSING_CONTROLLER, 0x11)                                     \
    X(PROCESSING_ACCELERATOR, 0x12)                                                                \
    X(NON_ESSENTIAL_INSTRUMENTATION, 0x13)

    /// @brief The PCI base class broadly classifies the type of functionality a device provides.
    ///
    /// NONE (0xFF): Device does not fit in any defined class.
    /// - ANCIENT (0x00): Device built before Class Code definitions were finalized.
    /// - MASS_STORAGE_CONTROLLER (0x01): Mass storage controller.
    /// - NETWORK_CONTROLLER (0x02): Network controller.
    /// - DISPLAY_CONTROLLER (0x03): Display controller.
    /// - MULTIMEDIA_DEVICE (0x04): Multimedia device.
    /// - MEMORY_CONTROLLER (0x05): Memory controller.
    /// - BRIDGE_DEVICE (0x06): Bridge device.
    /// - SIMPLE_COMMUNICATION_CONTROLLER (0x07): Simple communication controller.
    /// - BASE_SYSTEM_PERIPHERAL (0x08): Base system peripheral.
    /// - INPUT_DEVICE (0x09): Input device.
    /// - DOCKING_STATION (0x0A): Docking station.
    /// - PROCESSOR (0x0B): Processor.
    /// - SERIAL_BUS_CONTROLLER (0x0C): Serial bus controller.
    /// - WIRELESS_CONTROLLER (0x0D): Wireless controller.
    /// - INTELLIGENT_IO_CONTROLLER (0x0E): Intelligent I/O controller.
    /// - SATELLITE_COMMUNICATION_CONTROLLER (0x0F): Satellite communication controller.
    /// - ENCRYPTION_DECRYPTION_CONTROLLER (0x10): Encryption/decryption controller.
    /// - DATA_ACQUISITION_AND_SIGNAL_PROCESSING_CONTROLLER (0x11): Data acquisition and signal
    ///   processing controller.
    /// - PROCESSING_ACCELERATOR (0x12): Processing accelerator.
    /// - NON_ESSENTIAL_INSTRUMENTATION (0x13): Non-essential instrumentation function.
    EMBER_DEFINE_SMART_ENUM(BaseClass, BASE_CLASS_CODES, 0xFF, U8) // NOLINT

#define ANCIENT_SUB_CLASS_CODES(X)                                                                 \
    X(ALL_EXCEPT_VGA, 0x0)                                                                         \
    X(VGA_COMPATIBLE, 0x1)

    /// @brief Sub-classes for PCI base class ANCIENT (0x00).
    ///
    /// - ALL_EXCEPT_VGA (0x00): All devices except VGA-compatible.
    /// - VGA_COMPATIBLE (0x01): VGA-compatible device.
    EMBER_DEFINE_SMART_ENUM(AncientSubClass, ANCIENT_SUB_CLASS_CODES, 0xFF) // NOLINT

#define MASS_STORAGE_SUB_CLASS_CODES(X)                                                            \
    X(SCSI_CONTROLLER, 0x0)                                                                        \
    X(IDE_CONTROLLER, 0x1)                                                                         \
    X(FLOPPY_DISK_CONTROLLER, 0x2)                                                                 \
    X(IPI_BUS_CONTROLLER, 0x3)                                                                     \
    X(RAID_CONTROLLER, 0x4)                                                                        \
    X(ATA_CONTROLLER, 0x5)                                                                         \
    X(SERIAL_ATA_CONTROLLER, 0x6)                                                                  \
    X(SERIAL_ATTACHED_SCSI_CONTROLLER, 0x7)                                                        \
    X(NON_VOLATILE_MEMORY_CONTROLLER, 0x8)                                                         \
    X(UNIVERSAL_FLASH_STORAGE_CONTROLLER, 0x9)                                                     \
    X(OTHER, 0x80)

    /// @brief Sub-classes for PCI base class MASS_STORAGE_CONTROLLER (0x01).
    ///
    /// - SCSI_CONTROLLER (0x00): SCSI controller.
    /// - IDE_CONTROLLER (0x01): IDE controller.
    /// - FLOPPY_DISK_CONTROLLER (0x02): Floppy disk controller.
    /// - IPI_BUS_CONTROLLER (0x03): IPI bus controller.
    /// - RAID_CONTROLLER (0x04): RAID controller.
    /// - ATA_CONTROLLER (0x05): ATA controller with ADMA interface.
    /// - SERIAL_ATA_CONTROLLER (0x06): Serial ATA controller.
    /// - SERIAL_ATTACHED_SCSI_CONTROLLER (0x07): Serial Attached SCSI controller.
    /// - NON_VOLATILE_MEMORY_CONTROLLER (0x08): Non-volatile memory controller.
    /// - UNIVERSAL_FLASH_STORAGE_CONTROLLER (0x09): Universal Flash Storage controller.
    /// - OTHER (0x80): Other mass storage controller.
    EMBER_DEFINE_SMART_ENUM(MassStorageSubClass, MASS_STORAGE_SUB_CLASS_CODES, 0xFF) // NOLINT

#define SCSI_PROGRAMMING_INTERFACE_CODES(X)                                                        \
    X(VENDOR_SPECIFIC, 0x0)                                                                        \
    X(SOP_PQI_STORAGE_DEVICE, 0x11)                                                                \
    X(SOP_PQI_CONTROLLER, 0x12)                                                                    \
    X(SOP_PQI_STORAGE_AND_CONTROLLER, 0x13)                                                        \
    X(SOP_NVME_QUEUING, 0x21)

    /// @brief Programming interfaces for MassStorageSubClass::SCSI_CONTROLLER (0x01/0x00).
    ///
    /// - VENDOR_SPECIFIC (0x00): Vendor-specific interface.
    /// - SOP_PQI_STORAGE_DEVICE (0x11): SOP storage device using PQI.
    /// - SOP_PQI_CONTROLLER (0x12): SOP controller using PQI.
    /// - SOP_PQI_STORAGE_AND_CONTROLLER (0x13): SOP storage device and controller using PQI.
    /// - SOP_NVME_QUEUING (0x21): SOP storage device using NVMe queuing interface.
    EMBER_DEFINE_SMART_ENUM(SCSIProgrammingInterface, SCSI_PROGRAMMING_INTERFACE_CODES, 0xFF)

#define ATA_PROGRAMMING_INTERFACE_CODES(X)                                                         \
    X(SINGLE_STEPPING, 0x20)                                                                       \
    X(CONTINUOUS_OPERATION, 0x30)

    /// @brief Programming interfaces for MassStorageSubClass::ATA_CONTROLLER (0x01/0x05).
    ///
    /// - SINGLE_STEPPING (0x20): ATA with ADMA interface, single stepping.
    /// - CONTINUOUS_OPERATION (0x30): ATA with ADMA interface, continuous operation.
    EMBER_DEFINE_SMART_ENUM(ATAProgrammingInterface, ATA_PROGRAMMING_INTERFACE_CODES, 0xFF)

#define SERIAL_ATA_PROGRAMMING_INTERFACE_CODES(X)                                                  \
    X(VENDOR_SPECIFIC, 0x0)                                                                        \
    X(AHCI, 0x1)                                                                                   \
    X(SERIAL_STORAGE_BUS, 0x2)

    /// @brief Programming interfaces for MassStorageSubClass::SERIAL_ATA_CONTROLLER (0x01/0x06).
    ///
    /// - VENDOR_SPECIFIC (0x00): Vendor-specific interface.
    /// - AHCI (0x01): AHCI 1.0 interface.
    /// - SERIAL_STORAGE_BUS (0x02): Serial Storage Bus interface.
    EMBER_DEFINE_SMART_ENUM(SerialATAProgrammingInterface,
                            SERIAL_ATA_PROGRAMMING_INTERFACE_CODES,
                            0xFF)

#define SERIAL_ATTACHED_SCSI_PROGRAMMING_INTERFACE_CODES(X)                                        \
    X(VENDOR_SPECIFIC, 0x0)                                                                        \
    X(OBSOLETE, 0x1)

    /// @brief Programming interfaces for MassStorageSubClass::SERIAL_ATTACHED_SCSI_CONTROLLER
    ///        (0x01/0x07).
    ///
    /// - VENDOR_SPECIFIC (0x00): Vendor-specific interface.
    /// - OBSOLETE (0x01): Obsolete.
    EMBER_DEFINE_SMART_ENUM(SerialAttachedSCSIProgrammingInterface,
                            SERIAL_ATTACHED_SCSI_PROGRAMMING_INTERFACE_CODES,
                            0xFF)

#define NVM_PROGRAMMING_INTERFACE_CODES(X)                                                         \
    X(VENDOR_SPECIFIC, 0x0)                                                                        \
    X(NVMHCI, 0x1)                                                                                 \
    X(NVME_IO_CONTROLLER, 0x2)                                                                     \
    X(NVME_ADMINISTRATIVE_CONTROLLER, 0x3)

    /// @brief Programming interfaces for MassStorageSubClass::NON_VOLATILE_MEMORY_CONTROLLER
    ///        (0x01/0x08).
    ///
    /// - VENDOR_SPECIFIC (0x00): Vendor-specific interface.
    /// - NVMHCI (0x01): NVMHCI interface.
    /// - NVME_IO_CONTROLLER (0x02): NVM Express I/O controller.
    /// - NVME_ADMINISTRATIVE_CONTROLLER (0x03): NVM Express administrative controller.
    EMBER_DEFINE_SMART_ENUM(NVMProgrammingInterface, NVM_PROGRAMMING_INTERFACE_CODES, 0xFF)

#define UFS_PROGRAMMING_INTERFACE_CODES(X)                                                         \
    X(VENDOR_SPECIFIC, 0x0)                                                                        \
    X(UFSHCI, 0x1)

    /// @brief Programming interfaces for MassStorageSubClass::UNIVERSAL_FLASH_STORAGE_CONTROLLER
    ///        (0x01/0x09).
    ///
    /// - VENDOR_SPECIFIC (0x00): Vendor-specific interface.
    /// - UFSHCI (0x01): Universal Flash Storage Host Controller Interface.
    EMBER_DEFINE_SMART_ENUM(UFSProgrammingInterface, UFS_PROGRAMMING_INTERFACE_CODES, 0xFF)

#define NETWORK_SUB_CLASS_CODES(X)                                                                 \
    X(ETHERNET_CONTROLLER, 0x0)                                                                    \
    X(TOKEN_RING_CONTROLLER, 0x1)                                                                  \
    X(FDDI_CONTROLLER, 0x2)                                                                        \
    X(ATM_CONTROLLER, 0x3)                                                                         \
    X(ISDN_CONTROLLER, 0x4)                                                                        \
    X(WORLDFIP_CONTROLLER, 0x5)                                                                    \
    X(PICMG_MULTI_COMPUTING, 0x6)                                                                  \
    X(INFINIBAND_CONTROLLER, 0x7)                                                                  \
    X(HOST_FABRIC_CONTROLLER, 0x8)                                                                 \
    X(OTHER, 0x80)

    /// @brief Sub-classes for PCI base class NETWORK_CONTROLLER (0x02).
    ///
    /// - ETHERNET_CONTROLLER (0x00): Ethernet controller.
    /// - TOKEN_RING_CONTROLLER (0x01): Token Ring controller.
    /// - FDDI_CONTROLLER (0x02): FDDI controller.
    /// - ATM_CONTROLLER (0x03): ATM controller.
    /// - ISDN_CONTROLLER (0x04): ISDN controller.
    /// - WORLDFIP_CONTROLLER (0x05): WorldFip controller.
    /// - PICMG_MULTI_COMPUTING (0x06): PICMG 2.14 Multi Computing controller.
    /// - INFINIBAND_CONTROLLER (0x07): InfiniBand controller.
    /// - HOST_FABRIC_CONTROLLER (0x08): Host fabric controller (vendor-specific interface).
    /// - OTHER (0x80): Other network controller.
    EMBER_DEFINE_SMART_ENUM(NetworkSubClass, NETWORK_SUB_CLASS_CODES, 0xFF) // NOLINT

#define DISPLAY_SUB_CLASS_CODES(X)                                                                 \
    X(VGA_COMPATIBLE_CONTROLLER, 0x0)                                                              \
    X(XGA_CONTROLLER, 0x1)                                                                         \
    X(THREE_D_CONTROLLER, 0x2)                                                                     \
    X(OTHER, 0x80)

    /// @brief Sub-classes for PCI base class DISPLAY_CONTROLLER (0x03).
    ///
    /// - VGA_COMPATIBLE_CONTROLLER (0x00): VGA-compatible controller.
    /// - XGA_CONTROLLER (0x01): XGA controller.
    /// - THREE_D_CONTROLLER (0x02): 3D controller.
    /// - OTHER (0x80): Other display controller.
    EMBER_DEFINE_SMART_ENUM(DisplaySubClass, DISPLAY_SUB_CLASS_CODES, 0xFF) // NOLINT

#define VGA_PROGRAMMING_INTERFACE_CODES(X)                                                         \
    X(VGA_COMPATIBLE, 0x0)                                                                         \
    X(COMPATIBLE_8514, 0x1)

    /// @brief Programming interfaces for DisplaySubClass::VGA_COMPATIBLE_CONTROLLER (0x03/0x00).
    ///
    /// - VGA_COMPATIBLE (0x00): VGA-compatible controller.
    /// - COMPATIBLE_8514 (0x01): 8514-compatible controller.
    EMBER_DEFINE_SMART_ENUM(VGAProgrammingInterface, VGA_PROGRAMMING_INTERFACE_CODES, 0xFF)

#define MULTIMEDIA_SUB_CLASS_CODES(X)                                                              \
    X(VIDEO_DEVICE, 0x0)                                                                           \
    X(AUDIO_DEVICE, 0x1)                                                                           \
    X(COMPUTER_TELEPHONY_DEVICE, 0x2)                                                              \
    X(HIGH_DEFINITION_AUDIO, 0x3)                                                                  \
    X(OTHER, 0x80)

    /// @brief Sub-classes for PCI base class MULTIMEDIA_DEVICE (0x04).
    ///
    /// - VIDEO_DEVICE (0x00): Video device.
    /// - AUDIO_DEVICE (0x01): Audio device.
    /// - COMPUTER_TELEPHONY_DEVICE (0x02): Computer telephony device.
    /// - HIGH_DEFINITION_AUDIO (0x03): High Definition Audio device.
    /// - OTHER (0x80): Other multimedia device.
    EMBER_DEFINE_SMART_ENUM(MultimediaSubClass, MULTIMEDIA_SUB_CLASS_CODES, 0xFF) // NOLINT

#define HD_AUDIO_PROGRAMMING_INTERFACE_CODES(X)                                                    \
    X(HD_AUDIO_1_0, 0x0)                                                                           \
    X(HD_AUDIO_1_0_VENDOR_EXTENSIONS, 0x80)

    /// @brief Programming interfaces for MultimediaSubClass::HIGH_DEFINITION_AUDIO (0x04/0x03).
    ///
    /// - HD_AUDIO_1_0 (0x00): High Definition Audio 1.0 compatible.
    /// - HD_AUDIO_1_0_VENDOR_EXTENSIONS (0x80): HD-A 1.0 compatible with vendor extensions.
    EMBER_DEFINE_SMART_ENUM(HDAudioProgrammingInterface, HD_AUDIO_PROGRAMMING_INTERFACE_CODES, 0xFF)

#define MEMORY_SUB_CLASS_CODES(X)                                                                  \
    X(RAM, 0x0)                                                                                    \
    X(FLASH, 0x1)                                                                                  \
    X(OTHER, 0x80)

    /// @brief Sub-classes for PCI base class MEMORY_CONTROLLER (0x05).
    ///
    /// - RAM (0x00): RAM controller.
    /// - FLASH (0x01): Flash memory controller.
    /// - OTHER (0x80): Other memory controller.
    EMBER_DEFINE_SMART_ENUM(MemorySubClass, MEMORY_SUB_CLASS_CODES, 0xFF) // NOLINT

#define BRIDGE_SUB_CLASS_CODES(X)                                                                  \
    X(HOST_BRIDGE, 0x0)                                                                            \
    X(ISA_BRIDGE, 0x1)                                                                             \
    X(EISA_BRIDGE, 0x2)                                                                            \
    X(MCA_BRIDGE, 0x3)                                                                             \
    X(PCI_TO_PCI_BRIDGE, 0x4)                                                                      \
    X(PCMCIA_BRIDGE, 0x5)                                                                          \
    X(NUBUS_BRIDGE, 0x6)                                                                           \
    X(CARDBUS_BRIDGE, 0x7)                                                                         \
    X(RACEWAY_BRIDGE, 0x8)                                                                         \
    X(SEMI_TRANSPARENT_PCI_TO_PCI_BRIDGE, 0x9)                                                     \
    X(INFINIBAND_TO_PCI_HOST_BRIDGE, 0xA)                                                          \
    X(ADVANCED_SWITCHING_TO_PCI_HOST_BRIDGE, 0xB)                                                  \
    X(OTHER, 0x80)

    /// @brief Sub-classes for PCI base class BRIDGE_DEVICE (0x06).
    ///
    /// - HOST_BRIDGE (0x00): Host bridge.
    /// - ISA_BRIDGE (0x01): ISA bridge.
    /// - EISA_BRIDGE (0x02): EISA bridge.
    /// - MCA_BRIDGE (0x03): MCA bridge.
    /// - PCI_TO_PCI_BRIDGE (0x04): PCI-to-PCI bridge.
    /// - PCMCIA_BRIDGE (0x05): PCMCIA bridge.
    /// - NUBUS_BRIDGE (0x06): NuBus bridge.
    /// - CARDBUS_BRIDGE (0x07): CardBus bridge.
    /// - RACEWAY_BRIDGE (0x08): RACEway bridge.
    /// - SEMI_TRANSPARENT_PCI_TO_PCI_BRIDGE (0x09): Semi-transparent PCI-to-PCI bridge.
    /// - INFINIBAND_TO_PCI_HOST_BRIDGE (0x0A): InfiniBand-to-PCI host bridge.
    /// - ADVANCED_SWITCHING_TO_PCI_HOST_BRIDGE (0x0B): Advanced Switching to PCI host bridge.
    /// - OTHER (0x80): Other bridge device.
    EMBER_DEFINE_SMART_ENUM(BridgeSubClass, BRIDGE_SUB_CLASS_CODES, 0xFF) // NOLINT

#define PCI_BRIDGE_PROGRAMMING_INTERFACE_CODES(X)                                                  \
    X(STANDARD, 0x0)                                                                               \
    X(SUBTRACTIVE_DECODE, 0x1)

    /// @brief Programming interfaces for BridgeSubClass::PCI_TO_PCI_BRIDGE (0x06/0x04).
    ///
    /// - STANDARD (0x00): Standard PCI-to-PCI bridge.
    /// - SUBTRACTIVE_DECODE (0x01): Subtractive decode PCI-to-PCI bridge.
    EMBER_DEFINE_SMART_ENUM(PCIBridgeProgrammingInterface,
                            PCI_BRIDGE_PROGRAMMING_INTERFACE_CODES,
                            0xFF)

#define SEMI_TRANSPARENT_BRIDGE_PROGRAMMING_INTERFACE_CODES(X)                                     \
    X(PRIMARY_FACING_HOST, 0x40)                                                                   \
    X(SECONDARY_FACING_HOST, 0x80)

    /// @brief Programming interfaces for BridgeSubClass::SEMI_TRANSPARENT_PCI_TO_PCI_BRIDGE
    ///        (0x06/0x09).
    ///
    /// - PRIMARY_FACING_HOST (0x40): Primary PCI bus side facing the system host processor.
    /// - SECONDARY_FACING_HOST (0x80): Secondary PCI bus side facing the system host processor.
    EMBER_DEFINE_SMART_ENUM(SemiTransparentBridgeProgrammingInterface,
                            SEMI_TRANSPARENT_BRIDGE_PROGRAMMING_INTERFACE_CODES,
                            0xFF)

#define ADVANCED_SWITCHING_BRIDGE_PROGRAMMING_INTERFACE_CODES(X)                                   \
    X(CUSTOM, 0x0)                                                                                 \
    X(ASI_SIG_PORTAL, 0x1)

    /// @brief Programming interfaces for BridgeSubClass::ADVANCED_SWITCHING_TO_PCI_HOST_BRIDGE
    ///        (0x06/0x0B).
    ///
    /// - CUSTOM (0x00): Custom interface.
    /// - ASI_SIG_PORTAL (0x01): ASI-SIG defined portal interface.
    EMBER_DEFINE_SMART_ENUM(AdvancedSwitchingBridgeProgrammingInterface,
                            ADVANCED_SWITCHING_BRIDGE_PROGRAMMING_INTERFACE_CODES,
                            0xFF)

#define SIMPLE_COMMUNICATION_SUB_CLASS_CODES(X)                                                    \
    X(SERIAL_CONTROLLER, 0x0)                                                                      \
    X(PARALLEL_PORT, 0x1)                                                                          \
    X(MULTIPORT_SERIAL_CONTROLLER, 0x2)                                                            \
    X(MODEM, 0x3)                                                                                  \
    X(GPIB_CONTROLLER, 0x4)                                                                        \
    X(SMART_CARD, 0x5)                                                                             \
    X(OTHER, 0x80)

    /// @brief Sub-classes for PCI base class SIMPLE_COMMUNICATION_CONTROLLER (0x07).
    ///
    /// - SERIAL_CONTROLLER (0x00): Serial controller.
    /// - PARALLEL_PORT (0x01): Parallel port.
    /// - MULTIPORT_SERIAL_CONTROLLER (0x02): Multiport serial controller.
    /// - MODEM (0x03): Modem.
    /// - GPIB_CONTROLLER (0x04): GPIB (IEEE 488.1/2) controller.
    /// - SMART_CARD (0x05): Smart Card controller.
    /// - OTHER (0x80): Other communications device.
    EMBER_DEFINE_SMART_ENUM(SimpleCommunicationSubClass, SIMPLE_COMMUNICATION_SUB_CLASS_CODES, 0xFF)

#define SERIAL_CONTROLLER_PROGRAMMING_INTERFACE_CODES(X)                                           \
    X(GENERIC_XT_COMPATIBLE, 0x0)                                                                  \
    X(COMPATIBLE_16450, 0x1)                                                                       \
    X(COMPATIBLE_16550, 0x2)                                                                       \
    X(COMPATIBLE_16650, 0x3)                                                                       \
    X(COMPATIBLE_16750, 0x4)                                                                       \
    X(COMPATIBLE_16850, 0x5)                                                                       \
    X(COMPATIBLE_16950, 0x6)

    /// @brief Programming interfaces for SimpleCommunicationSubClass::SERIAL_CONTROLLER
    ///        (0x07/0x00).
    ///
    /// - GENERIC_XT_COMPATIBLE (0x00): Generic XT-compatible serial controller.
    /// - COMPATIBLE_16450 (0x01): 16450-compatible serial controller.
    /// - COMPATIBLE_16550 (0x02): 16550-compatible serial controller.
    /// - COMPATIBLE_16650 (0x03): 16650-compatible serial controller.
    /// - COMPATIBLE_16750 (0x04): 16750-compatible serial controller.
    /// - COMPATIBLE_16850 (0x05): 16850-compatible serial controller.
    /// - COMPATIBLE_16950 (0x06): 16950-compatible serial controller.
    EMBER_DEFINE_SMART_ENUM(SerialControllerProgrammingInterface,
                            SERIAL_CONTROLLER_PROGRAMMING_INTERFACE_CODES,
                            0xFF)

#define PARALLEL_PORT_PROGRAMMING_INTERFACE_CODES(X)                                               \
    X(STANDARD, 0x0)                                                                               \
    X(BIDIRECTIONAL, 0x1)                                                                          \
    X(ECP_1_X, 0x2)                                                                                \
    X(IEEE1284_CONTROLLER, 0x3)                                                                    \
    X(IEEE1284_TARGET, 0xFE)

    /// @brief Programming interfaces for SimpleCommunicationSubClass::PARALLEL_PORT (0x07/0x01).
    ///
    /// - STANDARD (0x00): Standard parallel port.
    /// - BIDIRECTIONAL (0x01): Bidirectional parallel port.
    /// - ECP_1_X (0x02): ECP 1.X compliant parallel port.
    /// - IEEE1284_CONTROLLER (0x03): IEEE1284 controller.
    /// - IEEE1284_TARGET (0xFE): IEEE1284 target device (not a controller).
    EMBER_DEFINE_SMART_ENUM(ParallelPortProgrammingInterface,
                            PARALLEL_PORT_PROGRAMMING_INTERFACE_CODES,
                            0xFF)

#define MODEM_PROGRAMMING_INTERFACE_CODES(X)                                                       \
    X(GENERIC, 0x0)                                                                                \
    X(HAYES_16450_COMPATIBLE, 0x1)                                                                 \
    X(HAYES_16550_COMPATIBLE, 0x2)                                                                 \
    X(HAYES_16650_COMPATIBLE, 0x3)                                                                 \
    X(HAYES_16750_COMPATIBLE, 0x4)

    /// @brief Programming interfaces for SimpleCommunicationSubClass::MODEM (0x07/0x03).
    ///
    /// - GENERIC (0x00): Generic modem.
    /// - HAYES_16450_COMPATIBLE (0x01): Hayes compatible modem, 16450-compatible interface.
    /// - HAYES_16550_COMPATIBLE (0x02): Hayes compatible modem, 16550-compatible interface.
    /// - HAYES_16650_COMPATIBLE (0x03): Hayes compatible modem, 16650-compatible interface.
    /// - HAYES_16750_COMPATIBLE (0x04): Hayes compatible modem, 16750-compatible interface.
    EMBER_DEFINE_SMART_ENUM(ModemProgrammingInterface, MODEM_PROGRAMMING_INTERFACE_CODES, 0xFF)

#define BASE_SYSTEM_PERIPHERAL_SUB_CLASS_CODES(X)                                                  \
    X(PIC, 0x0)                                                                                    \
    X(DMA_CONTROLLER, 0x1)                                                                         \
    X(SYSTEM_TIMER, 0x2)                                                                           \
    X(RTC_CONTROLLER, 0x3)                                                                         \
    X(PCI_HOT_PLUG_CONTROLLER, 0x4)                                                                \
    X(SD_HOST_CONTROLLER, 0x5)                                                                     \
    X(IOMMU, 0x6)                                                                                  \
    X(ROOT_COMPLEX_EVENT_COLLECTOR, 0x7)                                                           \
    X(OTHER, 0x80)

    /// @brief Sub-classes for PCI base class BASE_SYSTEM_PERIPHERAL (0x08).
    ///
    /// - PIC (0x00): Programmable interrupt controller.
    /// - DMA_CONTROLLER (0x01): DMA controller.
    /// - SYSTEM_TIMER (0x02): System timer.
    /// - RTC_CONTROLLER (0x03): RTC controller.
    /// - PCI_HOT_PLUG_CONTROLLER (0x04): Generic PCI Hot-Plug controller.
    /// - SD_HOST_CONTROLLER (0x05): SD Host controller.
    /// - IOMMU (0x06): IOMMU.
    /// - ROOT_COMPLEX_EVENT_COLLECTOR (0x07): Root Complex Event Collector.
    /// - OTHER (0x80): Other system peripheral.
    EMBER_DEFINE_SMART_ENUM(BaseSystemPeripheralSubClass,
                            BASE_SYSTEM_PERIPHERAL_SUB_CLASS_CODES,
                            0xFF)

#define PIC_PROGRAMMING_INTERFACE_CODES(X)                                                         \
    X(GENERIC_8259, 0x0)                                                                           \
    X(ISA, 0x1)                                                                                    \
    X(EISA, 0x2)                                                                                   \
    X(IO_APIC, 0x10)                                                                               \
    X(IOX_APIC, 0x20)

    /// @brief Programming interfaces for BaseSystemPeripheralSubClass::PIC (0x08/0x00).
    ///
    /// - GENERIC_8259 (0x00): Generic 8259 PIC.
    /// - ISA (0x01): ISA PIC.
    /// - EISA (0x02): EISA PIC.
    /// - IO_APIC (0x10): I/O APIC interrupt controller.
    /// - IOX_APIC (0x20): I/O(x) APIC interrupt controller.
    EMBER_DEFINE_SMART_ENUM(PICProgrammingInterface, PIC_PROGRAMMING_INTERFACE_CODES, 0xFF)

#define DMA_PROGRAMMING_INTERFACE_CODES(X)                                                         \
    X(GENERIC_8237, 0x0)                                                                           \
    X(ISA, 0x1)                                                                                    \
    X(EISA, 0x2)

    /// @brief Programming interfaces for BaseSystemPeripheralSubClass::DMA_CONTROLLER (0x08/0x01).
    ///
    /// - GENERIC_8237 (0x00): Generic 8237 DMA controller.
    /// - ISA (0x01): ISA DMA controller.
    /// - EISA (0x02): EISA DMA controller.
    EMBER_DEFINE_SMART_ENUM(DMAProgrammingInterface, DMA_PROGRAMMING_INTERFACE_CODES, 0xFF)

#define SYSTEM_TIMER_PROGRAMMING_INTERFACE_CODES(X)                                                \
    X(GENERIC_8254, 0x0)                                                                           \
    X(ISA, 0x1)                                                                                    \
    X(EISA, 0x2)                                                                                   \
    X(HIGH_PERFORMANCE_EVENT_TIMER, 0x3)

    /// @brief Programming interfaces for BaseSystemPeripheralSubClass::SYSTEM_TIMER (0x08/0x02).
    ///
    /// - GENERIC_8254 (0x00): Generic 8254 system timer.
    /// - ISA (0x01): ISA system timer.
    /// - EISA (0x02): EISA system timers (two timers).
    /// - HIGH_PERFORMANCE_EVENT_TIMER (0x03): High Performance Event Timer.
    EMBER_DEFINE_SMART_ENUM(SystemTimerProgrammingInterface,
                            SYSTEM_TIMER_PROGRAMMING_INTERFACE_CODES,
                            0xFF)

#define RTC_PROGRAMMING_INTERFACE_CODES(X)                                                         \
    X(GENERIC, 0x0)                                                                                \
    X(ISA, 0x1)

    /// @brief Programming interfaces for BaseSystemPeripheralSubClass::RTC_CONTROLLER (0x08/0x03).
    ///
    /// - GENERIC (0x00): Generic RTC controller.
    /// - ISA (0x01): ISA RTC controller.
    EMBER_DEFINE_SMART_ENUM(RTCProgrammingInterface, RTC_PROGRAMMING_INTERFACE_CODES, 0xFF)

#define INPUT_DEVICE_SUB_CLASS_CODES(X)                                                            \
    X(KEYBOARD_CONTROLLER, 0x0)                                                                    \
    X(DIGITIZER, 0x1)                                                                              \
    X(MOUSE_CONTROLLER, 0x2)                                                                       \
    X(SCANNER_CONTROLLER, 0x3)                                                                     \
    X(GAMEPORT_CONTROLLER, 0x4)                                                                    \
    X(OTHER, 0x80)

    /// @brief Sub-classes for PCI base class INPUT_DEVICE (0x09).
    ///
    /// - KEYBOARD_CONTROLLER (0x00): Keyboard controller.
    /// - DIGITIZER (0x01): Digitizer (pen).
    /// - MOUSE_CONTROLLER (0x02): Mouse controller.
    /// - SCANNER_CONTROLLER (0x03): Scanner controller.
    /// - GAMEPORT_CONTROLLER (0x04): Gameport controller.
    /// - OTHER (0x80): Other input controller.
    EMBER_DEFINE_SMART_ENUM(InputDeviceSubClass, INPUT_DEVICE_SUB_CLASS_CODES, 0xFF) // NOLINT

#define GAMEPORT_PROGRAMMING_INTERFACE_CODES(X)                                                    \
    X(GENERIC, 0x0)                                                                                \
    X(LEGACY, 0x10)

    /// @brief Programming interfaces for InputDeviceSubClass::GAMEPORT_CONTROLLER (0x09/0x04).
    ///
    /// - GENERIC (0x00): Generic gameport controller.
    /// - LEGACY (0x10): Legacy gameport interface.
    EMBER_DEFINE_SMART_ENUM(GameportProgrammingInterface,
                            GAMEPORT_PROGRAMMING_INTERFACE_CODES,
                            0xFF)

#define DOCKING_STATION_SUB_CLASS_CODES(X)                                                         \
    X(GENERIC, 0x0)                                                                                \
    X(OTHER, 0x80)

    /// @brief Sub-classes for PCI base class DOCKING_STATION (0x0A).
    ///
    /// - GENERIC (0x00): Generic docking station.
    /// - OTHER (0x80): Other type of docking station.
    EMBER_DEFINE_SMART_ENUM(DockingStationSubClass, DOCKING_STATION_SUB_CLASS_CODES, 0xFF) // NOLINT

#define PROCESSOR_SUB_CLASS_CODES(X)                                                               \
    X(I386, 0x0)                                                                                   \
    X(I486, 0x1)                                                                                   \
    X(PENTIUM, 0x2)                                                                                \
    X(ALPHA, 0x10)                                                                                 \
    X(POWERPC, 0x20)                                                                               \
    X(MIPS, 0x30)                                                                                  \
    X(CO_PROCESSOR, 0x40)                                                                          \
    X(OTHER, 0x80)

    /// @brief Sub-classes for PCI base class PROCESSOR (0x0B).
    ///
    /// - I386 (0x00): 386 processor.
    /// - I486 (0x01): 486 processor.
    /// - PENTIUM (0x02): Pentium processor.
    /// - ALPHA (0x10): Alpha processor.
    /// - POWERPC (0x20): PowerPC processor.
    /// - MIPS (0x30): MIPS processor.
    /// - CO_PROCESSOR (0x40): Co-processor.
    /// - OTHER (0x80): Other processor.
    EMBER_DEFINE_SMART_ENUM(ProcessorSubClass, PROCESSOR_SUB_CLASS_CODES, 0xFF) // NOLINT

#define SERIAL_BUS_SUB_CLASS_CODES(X)                                                              \
    X(FIREWIRE, 0x0)                                                                               \
    X(ACCESS_BUS, 0x1)                                                                             \
    X(SSA, 0x2)                                                                                    \
    X(USB, 0x3)                                                                                    \
    X(FIBRE_CHANNEL, 0x4)                                                                          \
    X(SMBUS, 0x5)                                                                                  \
    X(INFINIBAND, 0x6)                                                                             \
    X(IPMI, 0x7)                                                                                   \
    X(SERCOS, 0x8)                                                                                 \
    X(CANBUS, 0x9)                                                                                 \
    X(MIPI_I3C, 0xA)                                                                               \
    X(OTHER, 0x80)

    /// @brief Sub-classes for PCI base class SERIAL_BUS_CONTROLLER (0x0C).
    ///
    /// - FIREWIRE (0x00): IEEE 1394 (FireWire) controller.
    /// - ACCESS_BUS (0x01): ACCESS.bus controller.
    /// - SSA (0x02): SSA controller.
    /// - USB (0x03): Universal Serial Bus controller.
    /// - FIBRE_CHANNEL (0x04): Fibre Channel controller.
    /// - SMBUS (0x05): SMBus (System Management Bus) controller.
    /// - INFINIBAND (0x06): InfiniBand controller (deprecated; use base class 0x02/0x07).
    /// - IPMI (0x07): IPMI controller.
    /// - SERCOS (0x08): SERCOS Interface.
    /// - CANBUS (0x09): CANbus controller.
    /// - MIPI_I3C (0x0A): MIPI I3C host controller.
    /// - OTHER (0x80): Other serial bus controller.
    EMBER_DEFINE_SMART_ENUM(SerialBusSubClass, SERIAL_BUS_SUB_CLASS_CODES, 0xFF, U8) // NOLINT

#define FIREWIRE_PROGRAMMING_INTERFACE_CODES(X)                                                    \
    X(GENERIC, 0x0)                                                                                \
    X(OPEN_HCI, 0x10)

    /// @brief Programming interfaces for SerialBusSubClass::FIREWIRE (0x0C/0x00).
    ///
    /// - GENERIC (0x00): Generic IEEE 1394 interface.
    /// - OPEN_HCI (0x10): IEEE 1394 OpenHCI specification.
    EMBER_DEFINE_SMART_ENUM(FireWireProgrammingInterface,
                            FIREWIRE_PROGRAMMING_INTERFACE_CODES,
                            0xFF)

#define USB_PROGRAMMING_INTERFACE_CODES(X)                                                         \
    X(UHCI, 0x0)                                                                                   \
    X(OHCI, 0x10)                                                                                  \
    X(EHCI, 0x20)                                                                                  \
    X(XHCI, 0x30)                                                                                  \
    X(NO_SPECIFIC_INTERFACE, 0x80)                                                                 \
    X(USB_DEVICE, 0xFE)

    /// @brief Programming interfaces for SerialBusSubClass::USB (0x0C/0x03).
    ///
    /// - UHCI (0x00): Universal Host Controller Interface (USB 1.x).
    /// - OHCI (0x10): Open Host Controller Interface.
    /// - EHCI (0x20): Enhanced Host Controller Interface (USB 2.0).
    /// - XHCI (0x30): eXtensible Host Controller Interface (USB 3.x).
    /// - NO_SPECIFIC_INTERFACE (0x80): No specific programming interface.
    /// - USB_DEVICE (0xFE): USB device (not a host controller).
    EMBER_DEFINE_SMART_ENUM(USBProgrammingInterface, USB_PROGRAMMING_INTERFACE_CODES, 0xFF, U8)

#define IPMI_PROGRAMMING_INTERFACE_CODES(X)                                                        \
    X(SMIC, 0x0)                                                                                   \
    X(KEYBOARD_CONTROLLER_STYLE, 0x1)                                                              \
    X(BLOCK_TRANSFER, 0x2)

    /// @brief Programming interfaces for SerialBusSubClass::IPMI (0x0C/0x07).
    ///
    /// - SMIC (0x00): IPMI SMIC interface.
    /// - KEYBOARD_CONTROLLER_STYLE (0x01): IPMI Keyboard Controller Style interface.
    /// - BLOCK_TRANSFER (0x02): IPMI Block Transfer interface.
    EMBER_DEFINE_SMART_ENUM(IPMIProgrammingInterface, IPMI_PROGRAMMING_INTERFACE_CODES, 0xFF)

#define WIRELESS_SUB_CLASS_CODES(X)                                                                \
    X(IRDA, 0x0)                                                                                   \
    X(CONSUMER_IR, 0x1)                                                                            \
    X(UWB_RADIO, 0x10)                                                                             \
    X(RF_CONTROLLER, 0x11)                                                                         \
    X(BLUETOOTH, 0x12)                                                                             \
    X(BROADBAND, 0x20)                                                                             \
    X(ETHERNET_802_11A, 0x21)                                                                      \
    X(CELLULAR, 0x40)                                                                              \
    X(CELLULAR_WITH_ETHERNET, 0x41)                                                                \
    X(OTHER, 0x80)

    /// @brief Sub-classes for PCI base class WIRELESS_CONTROLLER (0x0D).
    ///
    /// - IRDA (0x00): iRDA compatible controller.
    /// - CONSUMER_IR (0x01): Consumer IR controller.
    /// - UWB_RADIO (0x10): UWB Radio controller.
    /// - RF_CONTROLLER (0x11): RF controller.
    /// - BLUETOOTH (0x12): Bluetooth controller.
    /// - BROADBAND (0x20): Broadband controller.
    /// - ETHERNET_802_11A (0x21): IEEE 802.11a/b Ethernet controller.
    /// - CELLULAR (0x40): Cellular controller/modem.
    /// - CELLULAR_WITH_ETHERNET (0x41): Cellular controller/modem with Ethernet (802.11).
    /// - OTHER (0x80): Other wireless controller.
    EMBER_DEFINE_SMART_ENUM(WirelessSubClass, WIRELESS_SUB_CLASS_CODES, 0xFF) // NOLINT

#define INTELLIGENT_IO_SUB_CLASS_CODES(X) X(I2O, 0x0)

    /// @brief Sub-classes for PCI base class INTELLIGENT_IO_CONTROLLER (0x0E).
    ///
    /// - I2O (0x00): Intelligent I/O (I2O) Architecture Specification 1.0.
    EMBER_DEFINE_SMART_ENUM(IntelligentIOSubClass, INTELLIGENT_IO_SUB_CLASS_CODES, 0xFF) // NOLINT

#define SATELLITE_COMMUNICATION_SUB_CLASS_CODES(X)                                                 \
    X(TV, 0x1)                                                                                     \
    X(AUDIO, 0x2)                                                                                  \
    X(VOICE, 0x3)                                                                                  \
    X(DATA, 0x4)                                                                                   \
    X(OTHER, 0x80)

    /// @brief Sub-classes for PCI base class SATELLITE_COMMUNICATION_CONTROLLER (0x0F).
    ///
    /// - TV (0x01): TV satellite controller.
    /// - AUDIO (0x02): Audio satellite controller.
    /// - VOICE (0x03): Voice satellite controller.
    /// - DATA (0x04): Data satellite controller.
    /// - OTHER (0x80): Other satellite communication controller.
    EMBER_DEFINE_SMART_ENUM(SatelliteCommunicationSubClass,
                            SATELLITE_COMMUNICATION_SUB_CLASS_CODES,
                            0xFF)

#define ENCRYPTION_DECRYPTION_SUB_CLASS_CODES(X)                                                   \
    X(NETWORK_AND_COMPUTING, 0x0)                                                                  \
    X(ENTERTAINMENT, 0x10)                                                                         \
    X(OTHER, 0x80)

    /// @brief Sub-classes for PCI base class ENCRYPTION_DECRYPTION_CONTROLLER (0x10).
    ///
    /// - NETWORK_AND_COMPUTING (0x00): Network and computing encryption/decryption controller.
    /// - ENTERTAINMENT (0x10): Entertainment encryption/decryption controller.
    /// - OTHER (0x80): Other encryption/decryption controller.
    EMBER_DEFINE_SMART_ENUM(EncryptionDecryptionSubClass,
                            ENCRYPTION_DECRYPTION_SUB_CLASS_CODES,
                            0xFF)

#define DATA_ACQUISITION_SUB_CLASS_CODES(X)                                                        \
    X(DPIO_MODULES, 0x0)                                                                           \
    X(PERFORMANCE_COUNTERS, 0x1)                                                                   \
    X(COMMUNICATIONS_SYNCHRONIZATION, 0x10)                                                        \
    X(MANAGEMENT_CARD, 0x20)                                                                       \
    X(OTHER, 0x80)

    /// @brief Sub-classes for PCI base class DATA_ACQUISITION_AND_SIGNAL_PROCESSING_CONTROLLER
    ///        (0x11).
    ///
    /// - DPIO_MODULES (0x00): DPIO modules.
    /// - PERFORMANCE_COUNTERS (0x01): Performance counters.
    /// - COMMUNICATIONS_SYNCHRONIZATION (0x10): Communications synchronization, time and
    ///   frequency test/measurement.
    /// - MANAGEMENT_CARD (0x20): Management card.
    /// - OTHER (0x80): Other data acquisition/signal processing controller.
    EMBER_DEFINE_SMART_ENUM(DataAcquisitionSubClass, DATA_ACQUISITION_SUB_CLASS_CODES, 0xFF)

#define PROCESSING_ACCELERATOR_SUB_CLASS_CODES(X) X(PROCESSING_ACCELERATOR, 0x0)

    /// @brief Sub-classes for PCI base class PROCESSING_ACCELERATOR (0x12).
    ///
    /// - PROCESSING_ACCELERATOR (0x00): Processing accelerator (vendor-specific interface).
    EMBER_DEFINE_SMART_ENUM(ProcessingAcceleratorSubClass,
                            PROCESSING_ACCELERATOR_SUB_CLASS_CODES,
                            0xFF)

#define NON_ESSENTIAL_INSTRUMENTATION_SUB_CLASS_CODES(X) X(NON_ESSENTIAL_INSTRUMENTATION, 0x0)

    /// @brief Sub-classes for PCI base class NON_ESSENTIAL_INSTRUMENTATION (0x13).
    ///
    /// - NON_ESSENTIAL_INSTRUMENTATION (0x00): Non-essential instrumentation function
    ///   (vendor-specific interface).
    EMBER_DEFINE_SMART_ENUM(NonEssentialInstrumentationSubClass,
                            NON_ESSENTIAL_INSTRUMENTATION_SUB_CLASS_CODES,
                            0xFF)

    /// @brief Try to resolve the subclass_code to the subclass matching with the given base_class.
    /// @param base_class Base class.
    /// @param subclass_code Subclass code.
    /// @return Name of the subclass code or "NONE" if subclass_code does not encode a valid
    ///         subclass.
    auto pci_resolve_subclass_code(BaseClass base_class, U8 subclass_code) -> String;

    /// @brief Try to resolve the programming_interface to the programming interface matching with
    ///         the given base_class and subclass_code combination.
    /// @param base_class Base class.
    /// @param subclass_code Subclass code.
    /// @param programming_interface Programming interface.
    /// @return Name of the programming interface code or "NONE" if programming_interface does not
    ///         encode a valid programming interface.
    auto pci_resolve_programming_interface(BaseClass base_class,
                                           U8        subclass_code,
                                           U8        programming_interface) -> String;
} // namespace Rune::Device

#endif // RUNEOS_PCI_CLASSCODE_H
