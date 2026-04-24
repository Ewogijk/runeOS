
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

#ifndef RUNEOS_CLASSCODE_H
#define RUNEOS_CLASSCODE_H

#include <Ember/Enum.h>

#include <KRE/String.h>

namespace Rune::Device {
#define BASE_CLASS_CODES(X)                                                                        \
    X(BaseClass, ANCIENT, 0x0)                                                                     \
    X(BaseClass, MASS_STORAGE_CONTROLLER, 0x1)                                                     \
    X(BaseClass, NETWORK_CONTROLLER, 0x2)                                                          \
    X(BaseClass, DISPLAY_CONTROLLER, 0x3)                                                          \
    X(BaseClass, MULTIMEDIA_DEVICE, 0x4)                                                           \
    X(BaseClass, MEMORY_CONTROLLER, 0x5)                                                           \
    X(BaseClass, BRIDGE_DEVICE, 0x6)                                                               \
    X(BaseClass, SIMPLE_COMMUNICATION_CONTROLLER, 0x7)                                             \
    X(BaseClass, BASE_SYSTEM_PERIPHERAL, 0x8)                                                      \
    X(BaseClass, INPUT_DEVICE, 0x9)                                                                \
    X(BaseClass, DOCKING_STATION, 0xA)                                                             \
    X(BaseClass, PROCESSOR, 0xB)                                                                   \
    X(BaseClass, SERIAL_BUS_CONTROLLER, 0xC)                                                       \
    X(BaseClass, WIRELESS_CONTROLLER, 0xD)                                                         \
    X(BaseClass, INTELLIGENT_IO_CONTROLLER, 0xE)                                                   \
    X(BaseClass, SATELLITE_COMMUNICATION_CONTROLLER, 0xF)                                          \
    X(BaseClass, ENCRYPTION_DECRYPTION_CONTROLLER, 0x10)                                           \
    X(BaseClass, DATA_ACQUISITION_AND_SIGNAL_PROCESSING_CONTROLLER, 0x11)                          \
    X(BaseClass, PROCESSING_ACCELERATOR, 0x12)                                                     \
    X(BaseClass, NON_ESSENTIAL_INSTRUMENTATION, 0x13)

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
    DECLARE_ENUM(BaseClass, BASE_CLASS_CODES, 0xFF) // NOLINT

#define ANCIENT_SUB_CLASS_CODES(X)                                                                 \
    X(AncientSubClass, ALL_EXCEPT_VGA, 0x0)                                                        \
    X(AncientSubClass, VGA_COMPATIBLE, 0x1)

    /// @brief Sub-classes for PCI base class ANCIENT (0x00).
    ///
    /// - ALL_EXCEPT_VGA (0x00): All devices except VGA-compatible.
    /// - VGA_COMPATIBLE (0x01): VGA-compatible device.
    DECLARE_ENUM(AncientSubClass, ANCIENT_SUB_CLASS_CODES, 0xFF) // NOLINT

#define MASS_STORAGE_SUB_CLASS_CODES(X)                                                            \
    X(MassStorageSubClass, SCSI_CONTROLLER, 0x0)                                                   \
    X(MassStorageSubClass, IDE_CONTROLLER, 0x1)                                                    \
    X(MassStorageSubClass, FLOPPY_DISK_CONTROLLER, 0x2)                                            \
    X(MassStorageSubClass, IPI_BUS_CONTROLLER, 0x3)                                                \
    X(MassStorageSubClass, RAID_CONTROLLER, 0x4)                                                   \
    X(MassStorageSubClass, ATA_CONTROLLER, 0x5)                                                    \
    X(MassStorageSubClass, SERIAL_ATA_CONTROLLER, 0x6)                                             \
    X(MassStorageSubClass, SERIAL_ATTACHED_SCSI_CONTROLLER, 0x7)                                   \
    X(MassStorageSubClass, NON_VOLATILE_MEMORY_CONTROLLER, 0x8)                                    \
    X(MassStorageSubClass, UNIVERSAL_FLASH_STORAGE_CONTROLLER, 0x9)                                \
    X(MassStorageSubClass, OTHER, 0x80)

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
    DECLARE_ENUM(MassStorageSubClass, MASS_STORAGE_SUB_CLASS_CODES, 0xFF) // NOLINT

#define SCSI_PROGRAMMING_INTERFACE_CODES(X)                                                        \
    X(SCSIProgrammingInterface, VENDOR_SPECIFIC, 0x0)                                              \
    X(SCSIProgrammingInterface, SOP_PQI_STORAGE_DEVICE, 0x11)                                      \
    X(SCSIProgrammingInterface, SOP_PQI_CONTROLLER, 0x12)                                          \
    X(SCSIProgrammingInterface, SOP_PQI_STORAGE_AND_CONTROLLER, 0x13)                              \
    X(SCSIProgrammingInterface, SOP_NVME_QUEUING, 0x21)

    /// @brief Programming interfaces for MassStorageSubClass::SCSI_CONTROLLER (0x01/0x00).
    ///
    /// - VENDOR_SPECIFIC (0x00): Vendor-specific interface.
    /// - SOP_PQI_STORAGE_DEVICE (0x11): SOP storage device using PQI.
    /// - SOP_PQI_CONTROLLER (0x12): SOP controller using PQI.
    /// - SOP_PQI_STORAGE_AND_CONTROLLER (0x13): SOP storage device and controller using PQI.
    /// - SOP_NVME_QUEUING (0x21): SOP storage device using NVMe queuing interface.
    DECLARE_ENUM(SCSIProgrammingInterface, SCSI_PROGRAMMING_INTERFACE_CODES, 0xFF) // NOLINT

#define ATA_PROGRAMMING_INTERFACE_CODES(X)                                                         \
    X(ATAProgrammingInterface, SINGLE_STEPPING, 0x20)                                              \
    X(ATAProgrammingInterface, CONTINUOUS_OPERATION, 0x30)

    /// @brief Programming interfaces for MassStorageSubClass::ATA_CONTROLLER (0x01/0x05).
    ///
    /// - SINGLE_STEPPING (0x20): ATA with ADMA interface, single stepping.
    /// - CONTINUOUS_OPERATION (0x30): ATA with ADMA interface, continuous operation.
    DECLARE_ENUM(ATAProgrammingInterface, ATA_PROGRAMMING_INTERFACE_CODES, 0xFF) // NOLINT

#define SERIAL_ATA_PROGRAMMING_INTERFACE_CODES(X)                                                  \
    X(SerialATAProgrammingInterface, VENDOR_SPECIFIC, 0x0)                                         \
    X(SerialATAProgrammingInterface, AHCI, 0x1)                                                    \
    X(SerialATAProgrammingInterface, SERIAL_STORAGE_BUS, 0x2)

    /// @brief Programming interfaces for MassStorageSubClass::SERIAL_ATA_CONTROLLER (0x01/0x06).
    ///
    /// - VENDOR_SPECIFIC (0x00): Vendor-specific interface.
    /// - AHCI (0x01): AHCI 1.0 interface.
    /// - SERIAL_STORAGE_BUS (0x02): Serial Storage Bus interface.
    DECLARE_ENUM(SerialATAProgrammingInterface,
                 SERIAL_ATA_PROGRAMMING_INTERFACE_CODES,
                 0xFF) // NOLINT

#define SERIAL_ATTACHED_SCSI_PROGRAMMING_INTERFACE_CODES(X)                                        \
    X(SerialAttachedSCSIProgrammingInterface, VENDOR_SPECIFIC, 0x0)                                \
    X(SerialAttachedSCSIProgrammingInterface, OBSOLETE, 0x1)

    /// @brief Programming interfaces for MassStorageSubClass::SERIAL_ATTACHED_SCSI_CONTROLLER
    ///        (0x01/0x07).
    ///
    /// - VENDOR_SPECIFIC (0x00): Vendor-specific interface.
    /// - OBSOLETE (0x01): Obsolete.
    DECLARE_ENUM(SerialAttachedSCSIProgrammingInterface,
                 SERIAL_ATTACHED_SCSI_PROGRAMMING_INTERFACE_CODES,
                 0xFF) // NOLINT

#define NVM_PROGRAMMING_INTERFACE_CODES(X)                                                         \
    X(NVMProgrammingInterface, VENDOR_SPECIFIC, 0x0)                                               \
    X(NVMProgrammingInterface, NVMHCI, 0x1)                                                        \
    X(NVMProgrammingInterface, NVME_IO_CONTROLLER, 0x2)                                            \
    X(NVMProgrammingInterface, NVME_ADMINISTRATIVE_CONTROLLER, 0x3)

    /// @brief Programming interfaces for MassStorageSubClass::NON_VOLATILE_MEMORY_CONTROLLER
    ///        (0x01/0x08).
    ///
    /// - VENDOR_SPECIFIC (0x00): Vendor-specific interface.
    /// - NVMHCI (0x01): NVMHCI interface.
    /// - NVME_IO_CONTROLLER (0x02): NVM Express I/O controller.
    /// - NVME_ADMINISTRATIVE_CONTROLLER (0x03): NVM Express administrative controller.
    DECLARE_ENUM(NVMProgrammingInterface, NVM_PROGRAMMING_INTERFACE_CODES, 0xFF) // NOLINT

#define UFS_PROGRAMMING_INTERFACE_CODES(X)                                                         \
    X(UFSProgrammingInterface, VENDOR_SPECIFIC, 0x0)                                               \
    X(UFSProgrammingInterface, UFSHCI, 0x1)

    /// @brief Programming interfaces for MassStorageSubClass::UNIVERSAL_FLASH_STORAGE_CONTROLLER
    ///        (0x01/0x09).
    ///
    /// - VENDOR_SPECIFIC (0x00): Vendor-specific interface.
    /// - UFSHCI (0x01): Universal Flash Storage Host Controller Interface.
    DECLARE_ENUM(UFSProgrammingInterface, UFS_PROGRAMMING_INTERFACE_CODES, 0xFF) // NOLINT

#define NETWORK_SUB_CLASS_CODES(X)                                                                 \
    X(NetworkSubClass, ETHERNET_CONTROLLER, 0x0)                                                   \
    X(NetworkSubClass, TOKEN_RING_CONTROLLER, 0x1)                                                 \
    X(NetworkSubClass, FDDI_CONTROLLER, 0x2)                                                       \
    X(NetworkSubClass, ATM_CONTROLLER, 0x3)                                                        \
    X(NetworkSubClass, ISDN_CONTROLLER, 0x4)                                                       \
    X(NetworkSubClass, WORLDFIP_CONTROLLER, 0x5)                                                   \
    X(NetworkSubClass, PICMG_MULTI_COMPUTING, 0x6)                                                 \
    X(NetworkSubClass, INFINIBAND_CONTROLLER, 0x7)                                                 \
    X(NetworkSubClass, HOST_FABRIC_CONTROLLER, 0x8)                                                \
    X(NetworkSubClass, OTHER, 0x80)

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
    DECLARE_ENUM(NetworkSubClass, NETWORK_SUB_CLASS_CODES, 0xFF) // NOLINT

#define DISPLAY_SUB_CLASS_CODES(X)                                                                 \
    X(DisplaySubClass, VGA_COMPATIBLE_CONTROLLER, 0x0)                                             \
    X(DisplaySubClass, XGA_CONTROLLER, 0x1)                                                        \
    X(DisplaySubClass, THREE_D_CONTROLLER, 0x2)                                                    \
    X(DisplaySubClass, OTHER, 0x80)

    /// @brief Sub-classes for PCI base class DISPLAY_CONTROLLER (0x03).
    ///
    /// - VGA_COMPATIBLE_CONTROLLER (0x00): VGA-compatible controller.
    /// - XGA_CONTROLLER (0x01): XGA controller.
    /// - THREE_D_CONTROLLER (0x02): 3D controller.
    /// - OTHER (0x80): Other display controller.
    DECLARE_ENUM(DisplaySubClass, DISPLAY_SUB_CLASS_CODES, 0xFF) // NOLINT

#define VGA_PROGRAMMING_INTERFACE_CODES(X)                                                         \
    X(VGAProgrammingInterface, VGA_COMPATIBLE, 0x0)                                                \
    X(VGAProgrammingInterface, COMPATIBLE_8514, 0x1)

    /// @brief Programming interfaces for DisplaySubClass::VGA_COMPATIBLE_CONTROLLER (0x03/0x00).
    ///
    /// - VGA_COMPATIBLE (0x00): VGA-compatible controller.
    /// - COMPATIBLE_8514 (0x01): 8514-compatible controller.
    DECLARE_ENUM(VGAProgrammingInterface, VGA_PROGRAMMING_INTERFACE_CODES, 0xFF) // NOLINT

#define MULTIMEDIA_SUB_CLASS_CODES(X)                                                              \
    X(MultimediaSubClass, VIDEO_DEVICE, 0x0)                                                       \
    X(MultimediaSubClass, AUDIO_DEVICE, 0x1)                                                       \
    X(MultimediaSubClass, COMPUTER_TELEPHONY_DEVICE, 0x2)                                          \
    X(MultimediaSubClass, HIGH_DEFINITION_AUDIO, 0x3)                                              \
    X(MultimediaSubClass, OTHER, 0x80)

    /// @brief Sub-classes for PCI base class MULTIMEDIA_DEVICE (0x04).
    ///
    /// - VIDEO_DEVICE (0x00): Video device.
    /// - AUDIO_DEVICE (0x01): Audio device.
    /// - COMPUTER_TELEPHONY_DEVICE (0x02): Computer telephony device.
    /// - HIGH_DEFINITION_AUDIO (0x03): High Definition Audio device.
    /// - OTHER (0x80): Other multimedia device.
    DECLARE_ENUM(MultimediaSubClass, MULTIMEDIA_SUB_CLASS_CODES, 0xFF) // NOLINT

#define HD_AUDIO_PROGRAMMING_INTERFACE_CODES(X)                                                    \
    X(HDAudioProgrammingInterface, HD_AUDIO_1_0, 0x0)                                              \
    X(HDAudioProgrammingInterface, HD_AUDIO_1_0_VENDOR_EXTENSIONS, 0x80)

    /// @brief Programming interfaces for MultimediaSubClass::HIGH_DEFINITION_AUDIO (0x04/0x03).
    ///
    /// - HD_AUDIO_1_0 (0x00): High Definition Audio 1.0 compatible.
    /// - HD_AUDIO_1_0_VENDOR_EXTENSIONS (0x80): HD-A 1.0 compatible with vendor extensions.
    DECLARE_ENUM(HDAudioProgrammingInterface, HD_AUDIO_PROGRAMMING_INTERFACE_CODES, 0xFF) // NOLINT

#define MEMORY_SUB_CLASS_CODES(X)                                                                  \
    X(MemorySubClass, RAM, 0x0)                                                                    \
    X(MemorySubClass, FLASH, 0x1)                                                                  \
    X(MemorySubClass, OTHER, 0x80)

    /// @brief Sub-classes for PCI base class MEMORY_CONTROLLER (0x05).
    ///
    /// - RAM (0x00): RAM controller.
    /// - FLASH (0x01): Flash memory controller.
    /// - OTHER (0x80): Other memory controller.
    DECLARE_ENUM(MemorySubClass, MEMORY_SUB_CLASS_CODES, 0xFF) // NOLINT

#define BRIDGE_SUB_CLASS_CODES(X)                                                                  \
    X(BridgeSubClass, HOST_BRIDGE, 0x0)                                                            \
    X(BridgeSubClass, ISA_BRIDGE, 0x1)                                                             \
    X(BridgeSubClass, EISA_BRIDGE, 0x2)                                                            \
    X(BridgeSubClass, MCA_BRIDGE, 0x3)                                                             \
    X(BridgeSubClass, PCI_TO_PCI_BRIDGE, 0x4)                                                      \
    X(BridgeSubClass, PCMCIA_BRIDGE, 0x5)                                                          \
    X(BridgeSubClass, NUBUS_BRIDGE, 0x6)                                                           \
    X(BridgeSubClass, CARDBUS_BRIDGE, 0x7)                                                         \
    X(BridgeSubClass, RACEWAY_BRIDGE, 0x8)                                                         \
    X(BridgeSubClass, SEMI_TRANSPARENT_PCI_TO_PCI_BRIDGE, 0x9)                                     \
    X(BridgeSubClass, INFINIBAND_TO_PCI_HOST_BRIDGE, 0xA)                                          \
    X(BridgeSubClass, ADVANCED_SWITCHING_TO_PCI_HOST_BRIDGE, 0xB)                                  \
    X(BridgeSubClass, OTHER, 0x80)

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
    DECLARE_ENUM(BridgeSubClass, BRIDGE_SUB_CLASS_CODES, 0xFF) // NOLINT

#define PCI_BRIDGE_PROGRAMMING_INTERFACE_CODES(X)                                                  \
    X(PCIBridgeProgrammingInterface, STANDARD, 0x0)                                                \
    X(PCIBridgeProgrammingInterface, SUBTRACTIVE_DECODE, 0x1)

    /// @brief Programming interfaces for BridgeSubClass::PCI_TO_PCI_BRIDGE (0x06/0x04).
    ///
    /// - STANDARD (0x00): Standard PCI-to-PCI bridge.
    /// - SUBTRACTIVE_DECODE (0x01): Subtractive decode PCI-to-PCI bridge.
    DECLARE_ENUM(PCIBridgeProgrammingInterface,
                 PCI_BRIDGE_PROGRAMMING_INTERFACE_CODES,
                 0xFF) // NOLINT

#define SEMI_TRANSPARENT_BRIDGE_PROGRAMMING_INTERFACE_CODES(X)                                     \
    X(SemiTransparentBridgeProgrammingInterface, PRIMARY_FACING_HOST, 0x40)                        \
    X(SemiTransparentBridgeProgrammingInterface, SECONDARY_FACING_HOST, 0x80)

    /// @brief Programming interfaces for BridgeSubClass::SEMI_TRANSPARENT_PCI_TO_PCI_BRIDGE
    ///        (0x06/0x09).
    ///
    /// - PRIMARY_FACING_HOST (0x40): Primary PCI bus side facing the system host processor.
    /// - SECONDARY_FACING_HOST (0x80): Secondary PCI bus side facing the system host processor.
    DECLARE_ENUM(SemiTransparentBridgeProgrammingInterface,
                 SEMI_TRANSPARENT_BRIDGE_PROGRAMMING_INTERFACE_CODES,
                 0xFF) // NOLINT

#define ADVANCED_SWITCHING_BRIDGE_PROGRAMMING_INTERFACE_CODES(X)                                   \
    X(AdvancedSwitchingBridgeProgrammingInterface, CUSTOM, 0x0)                                    \
    X(AdvancedSwitchingBridgeProgrammingInterface, ASI_SIG_PORTAL, 0x1)

    /// @brief Programming interfaces for BridgeSubClass::ADVANCED_SWITCHING_TO_PCI_HOST_BRIDGE
    ///        (0x06/0x0B).
    ///
    /// - CUSTOM (0x00): Custom interface.
    /// - ASI_SIG_PORTAL (0x01): ASI-SIG defined portal interface.
    DECLARE_ENUM(AdvancedSwitchingBridgeProgrammingInterface,
                 ADVANCED_SWITCHING_BRIDGE_PROGRAMMING_INTERFACE_CODES,
                 0xFF) // NOLINT

#define SIMPLE_COMMUNICATION_SUB_CLASS_CODES(X)                                                    \
    X(SimpleCommunicationSubClass, SERIAL_CONTROLLER, 0x0)                                         \
    X(SimpleCommunicationSubClass, PARALLEL_PORT, 0x1)                                             \
    X(SimpleCommunicationSubClass, MULTIPORT_SERIAL_CONTROLLER, 0x2)                               \
    X(SimpleCommunicationSubClass, MODEM, 0x3)                                                     \
    X(SimpleCommunicationSubClass, GPIB_CONTROLLER, 0x4)                                           \
    X(SimpleCommunicationSubClass, SMART_CARD, 0x5)                                                \
    X(SimpleCommunicationSubClass, OTHER, 0x80)

    /// @brief Sub-classes for PCI base class SIMPLE_COMMUNICATION_CONTROLLER (0x07).
    ///
    /// - SERIAL_CONTROLLER (0x00): Serial controller.
    /// - PARALLEL_PORT (0x01): Parallel port.
    /// - MULTIPORT_SERIAL_CONTROLLER (0x02): Multiport serial controller.
    /// - MODEM (0x03): Modem.
    /// - GPIB_CONTROLLER (0x04): GPIB (IEEE 488.1/2) controller.
    /// - SMART_CARD (0x05): Smart Card controller.
    /// - OTHER (0x80): Other communications device.
    DECLARE_ENUM(SimpleCommunicationSubClass, SIMPLE_COMMUNICATION_SUB_CLASS_CODES, 0xFF) // NOLINT

#define SERIAL_CONTROLLER_PROGRAMMING_INTERFACE_CODES(X)                                           \
    X(SerialControllerProgrammingInterface, GENERIC_XT_COMPATIBLE, 0x0)                            \
    X(SerialControllerProgrammingInterface, COMPATIBLE_16450, 0x1)                                 \
    X(SerialControllerProgrammingInterface, COMPATIBLE_16550, 0x2)                                 \
    X(SerialControllerProgrammingInterface, COMPATIBLE_16650, 0x3)                                 \
    X(SerialControllerProgrammingInterface, COMPATIBLE_16750, 0x4)                                 \
    X(SerialControllerProgrammingInterface, COMPATIBLE_16850, 0x5)                                 \
    X(SerialControllerProgrammingInterface, COMPATIBLE_16950, 0x6)

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
    DECLARE_ENUM(SerialControllerProgrammingInterface,
                 SERIAL_CONTROLLER_PROGRAMMING_INTERFACE_CODES,
                 0xFF) // NOLINT

#define PARALLEL_PORT_PROGRAMMING_INTERFACE_CODES(X)                                               \
    X(ParallelPortProgrammingInterface, STANDARD, 0x0)                                             \
    X(ParallelPortProgrammingInterface, BIDIRECTIONAL, 0x1)                                        \
    X(ParallelPortProgrammingInterface, ECP_1_X, 0x2)                                              \
    X(ParallelPortProgrammingInterface, IEEE1284_CONTROLLER, 0x3)                                  \
    X(ParallelPortProgrammingInterface, IEEE1284_TARGET, 0xFE)

    /// @brief Programming interfaces for SimpleCommunicationSubClass::PARALLEL_PORT (0x07/0x01).
    ///
    /// - STANDARD (0x00): Standard parallel port.
    /// - BIDIRECTIONAL (0x01): Bidirectional parallel port.
    /// - ECP_1_X (0x02): ECP 1.X compliant parallel port.
    /// - IEEE1284_CONTROLLER (0x03): IEEE1284 controller.
    /// - IEEE1284_TARGET (0xFE): IEEE1284 target device (not a controller).
    DECLARE_ENUM(ParallelPortProgrammingInterface,
                 PARALLEL_PORT_PROGRAMMING_INTERFACE_CODES,
                 0xFF) // NOLINT

#define MODEM_PROGRAMMING_INTERFACE_CODES(X)                                                       \
    X(ModemProgrammingInterface, GENERIC, 0x0)                                                     \
    X(ModemProgrammingInterface, HAYES_16450_COMPATIBLE, 0x1)                                      \
    X(ModemProgrammingInterface, HAYES_16550_COMPATIBLE, 0x2)                                      \
    X(ModemProgrammingInterface, HAYES_16650_COMPATIBLE, 0x3)                                      \
    X(ModemProgrammingInterface, HAYES_16750_COMPATIBLE, 0x4)

    /// @brief Programming interfaces for SimpleCommunicationSubClass::MODEM (0x07/0x03).
    ///
    /// - GENERIC (0x00): Generic modem.
    /// - HAYES_16450_COMPATIBLE (0x01): Hayes compatible modem, 16450-compatible interface.
    /// - HAYES_16550_COMPATIBLE (0x02): Hayes compatible modem, 16550-compatible interface.
    /// - HAYES_16650_COMPATIBLE (0x03): Hayes compatible modem, 16650-compatible interface.
    /// - HAYES_16750_COMPATIBLE (0x04): Hayes compatible modem, 16750-compatible interface.
    DECLARE_ENUM(ModemProgrammingInterface, MODEM_PROGRAMMING_INTERFACE_CODES, 0xFF) // NOLINT

#define BASE_SYSTEM_PERIPHERAL_SUB_CLASS_CODES(X)                                                  \
    X(BaseSystemPeripheralSubClass, PIC, 0x0)                                                      \
    X(BaseSystemPeripheralSubClass, DMA_CONTROLLER, 0x1)                                           \
    X(BaseSystemPeripheralSubClass, SYSTEM_TIMER, 0x2)                                             \
    X(BaseSystemPeripheralSubClass, RTC_CONTROLLER, 0x3)                                           \
    X(BaseSystemPeripheralSubClass, PCI_HOT_PLUG_CONTROLLER, 0x4)                                  \
    X(BaseSystemPeripheralSubClass, SD_HOST_CONTROLLER, 0x5)                                       \
    X(BaseSystemPeripheralSubClass, IOMMU, 0x6)                                                    \
    X(BaseSystemPeripheralSubClass, ROOT_COMPLEX_EVENT_COLLECTOR, 0x7)                             \
    X(BaseSystemPeripheralSubClass, OTHER, 0x80)

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
    DECLARE_ENUM(BaseSystemPeripheralSubClass,
                 BASE_SYSTEM_PERIPHERAL_SUB_CLASS_CODES,
                 0xFF) // NOLINT

#define PIC_PROGRAMMING_INTERFACE_CODES(X)                                                         \
    X(PICProgrammingInterface, GENERIC_8259, 0x0)                                                  \
    X(PICProgrammingInterface, ISA, 0x1)                                                           \
    X(PICProgrammingInterface, EISA, 0x2)                                                          \
    X(PICProgrammingInterface, IO_APIC, 0x10)                                                      \
    X(PICProgrammingInterface, IOX_APIC, 0x20)

    /// @brief Programming interfaces for BaseSystemPeripheralSubClass::PIC (0x08/0x00).
    ///
    /// - GENERIC_8259 (0x00): Generic 8259 PIC.
    /// - ISA (0x01): ISA PIC.
    /// - EISA (0x02): EISA PIC.
    /// - IO_APIC (0x10): I/O APIC interrupt controller.
    /// - IOX_APIC (0x20): I/O(x) APIC interrupt controller.
    DECLARE_ENUM(PICProgrammingInterface, PIC_PROGRAMMING_INTERFACE_CODES, 0xFF) // NOLINT

#define DMA_PROGRAMMING_INTERFACE_CODES(X)                                                         \
    X(DMAProgrammingInterface, GENERIC_8237, 0x0)                                                  \
    X(DMAProgrammingInterface, ISA, 0x1)                                                           \
    X(DMAProgrammingInterface, EISA, 0x2)

    /// @brief Programming interfaces for BaseSystemPeripheralSubClass::DMA_CONTROLLER (0x08/0x01).
    ///
    /// - GENERIC_8237 (0x00): Generic 8237 DMA controller.
    /// - ISA (0x01): ISA DMA controller.
    /// - EISA (0x02): EISA DMA controller.
    DECLARE_ENUM(DMAProgrammingInterface, DMA_PROGRAMMING_INTERFACE_CODES, 0xFF) // NOLINT

#define SYSTEM_TIMER_PROGRAMMING_INTERFACE_CODES(X)                                                \
    X(SystemTimerProgrammingInterface, GENERIC_8254, 0x0)                                          \
    X(SystemTimerProgrammingInterface, ISA, 0x1)                                                   \
    X(SystemTimerProgrammingInterface, EISA, 0x2)                                                  \
    X(SystemTimerProgrammingInterface, HIGH_PERFORMANCE_EVENT_TIMER, 0x3)

    /// @brief Programming interfaces for BaseSystemPeripheralSubClass::SYSTEM_TIMER (0x08/0x02).
    ///
    /// - GENERIC_8254 (0x00): Generic 8254 system timer.
    /// - ISA (0x01): ISA system timer.
    /// - EISA (0x02): EISA system timers (two timers).
    /// - HIGH_PERFORMANCE_EVENT_TIMER (0x03): High Performance Event Timer.
    DECLARE_ENUM(SystemTimerProgrammingInterface,
                 SYSTEM_TIMER_PROGRAMMING_INTERFACE_CODES,
                 0xFF) // NOLINT

#define RTC_PROGRAMMING_INTERFACE_CODES(X)                                                         \
    X(RTCProgrammingInterface, GENERIC, 0x0)                                                       \
    X(RTCProgrammingInterface, ISA, 0x1)

    /// @brief Programming interfaces for BaseSystemPeripheralSubClass::RTC_CONTROLLER (0x08/0x03).
    ///
    /// - GENERIC (0x00): Generic RTC controller.
    /// - ISA (0x01): ISA RTC controller.
    DECLARE_ENUM(RTCProgrammingInterface, RTC_PROGRAMMING_INTERFACE_CODES, 0xFF) // NOLINT

#define INPUT_DEVICE_SUB_CLASS_CODES(X)                                                            \
    X(InputDeviceSubClass, KEYBOARD_CONTROLLER, 0x0)                                               \
    X(InputDeviceSubClass, DIGITIZER, 0x1)                                                         \
    X(InputDeviceSubClass, MOUSE_CONTROLLER, 0x2)                                                  \
    X(InputDeviceSubClass, SCANNER_CONTROLLER, 0x3)                                                \
    X(InputDeviceSubClass, GAMEPORT_CONTROLLER, 0x4)                                               \
    X(InputDeviceSubClass, OTHER, 0x80)

    /// @brief Sub-classes for PCI base class INPUT_DEVICE (0x09).
    ///
    /// - KEYBOARD_CONTROLLER (0x00): Keyboard controller.
    /// - DIGITIZER (0x01): Digitizer (pen).
    /// - MOUSE_CONTROLLER (0x02): Mouse controller.
    /// - SCANNER_CONTROLLER (0x03): Scanner controller.
    /// - GAMEPORT_CONTROLLER (0x04): Gameport controller.
    /// - OTHER (0x80): Other input controller.
    DECLARE_ENUM(InputDeviceSubClass, INPUT_DEVICE_SUB_CLASS_CODES, 0xFF) // NOLINT

#define GAMEPORT_PROGRAMMING_INTERFACE_CODES(X)                                                    \
    X(GameportProgrammingInterface, GENERIC, 0x0)                                                  \
    X(GameportProgrammingInterface, LEGACY, 0x10)

    /// @brief Programming interfaces for InputDeviceSubClass::GAMEPORT_CONTROLLER (0x09/0x04).
    ///
    /// - GENERIC (0x00): Generic gameport controller.
    /// - LEGACY (0x10): Legacy gameport interface.
    DECLARE_ENUM(GameportProgrammingInterface, GAMEPORT_PROGRAMMING_INTERFACE_CODES, 0xFF) // NOLINT

#define DOCKING_STATION_SUB_CLASS_CODES(X)                                                         \
    X(DockingStationSubClass, GENERIC, 0x0)                                                        \
    X(DockingStationSubClass, OTHER, 0x80)

    /// @brief Sub-classes for PCI base class DOCKING_STATION (0x0A).
    ///
    /// - GENERIC (0x00): Generic docking station.
    /// - OTHER (0x80): Other type of docking station.
    DECLARE_ENUM(DockingStationSubClass, DOCKING_STATION_SUB_CLASS_CODES, 0xFF) // NOLINT

#define PROCESSOR_SUB_CLASS_CODES(X)                                                               \
    X(ProcessorSubClass, I386, 0x0)                                                                \
    X(ProcessorSubClass, I486, 0x1)                                                                \
    X(ProcessorSubClass, PENTIUM, 0x2)                                                             \
    X(ProcessorSubClass, ALPHA, 0x10)                                                              \
    X(ProcessorSubClass, POWERPC, 0x20)                                                            \
    X(ProcessorSubClass, MIPS, 0x30)                                                               \
    X(ProcessorSubClass, CO_PROCESSOR, 0x40)                                                       \
    X(ProcessorSubClass, OTHER, 0x80)

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
    DECLARE_ENUM(ProcessorSubClass, PROCESSOR_SUB_CLASS_CODES, 0xFF) // NOLINT

#define SERIAL_BUS_SUB_CLASS_CODES(X)                                                              \
    X(SerialBusSubClass, FIREWIRE, 0x0)                                                            \
    X(SerialBusSubClass, ACCESS_BUS, 0x1)                                                          \
    X(SerialBusSubClass, SSA, 0x2)                                                                 \
    X(SerialBusSubClass, USB, 0x3)                                                                 \
    X(SerialBusSubClass, FIBRE_CHANNEL, 0x4)                                                       \
    X(SerialBusSubClass, SMBUS, 0x5)                                                               \
    X(SerialBusSubClass, INFINIBAND, 0x6)                                                          \
    X(SerialBusSubClass, IPMI, 0x7)                                                                \
    X(SerialBusSubClass, SERCOS, 0x8)                                                              \
    X(SerialBusSubClass, CANBUS, 0x9)                                                              \
    X(SerialBusSubClass, MIPI_I3C, 0xA)                                                            \
    X(SerialBusSubClass, OTHER, 0x80)

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
    DECLARE_ENUM(SerialBusSubClass, SERIAL_BUS_SUB_CLASS_CODES, 0xFF) // NOLINT

#define FIREWIRE_PROGRAMMING_INTERFACE_CODES(X)                                                    \
    X(FireWireProgrammingInterface, GENERIC, 0x0)                                                  \
    X(FireWireProgrammingInterface, OPEN_HCI, 0x10)

    /// @brief Programming interfaces for SerialBusSubClass::FIREWIRE (0x0C/0x00).
    ///
    /// - GENERIC (0x00): Generic IEEE 1394 interface.
    /// - OPEN_HCI (0x10): IEEE 1394 OpenHCI specification.
    DECLARE_ENUM(FireWireProgrammingInterface, FIREWIRE_PROGRAMMING_INTERFACE_CODES, 0xFF) // NOLINT

#define USB_PROGRAMMING_INTERFACE_CODES(X)                                                         \
    X(USBProgrammingInterface, UHCI, 0x0)                                                          \
    X(USBProgrammingInterface, OHCI, 0x10)                                                         \
    X(USBProgrammingInterface, EHCI, 0x20)                                                         \
    X(USBProgrammingInterface, XHCI, 0x30)                                                         \
    X(USBProgrammingInterface, NO_SPECIFIC_INTERFACE, 0x80)                                        \
    X(USBProgrammingInterface, USB_DEVICE, 0xFE)

    /// @brief Programming interfaces for SerialBusSubClass::USB (0x0C/0x03).
    ///
    /// - UHCI (0x00): Universal Host Controller Interface (USB 1.x).
    /// - OHCI (0x10): Open Host Controller Interface.
    /// - EHCI (0x20): Enhanced Host Controller Interface (USB 2.0).
    /// - XHCI (0x30): eXtensible Host Controller Interface (USB 3.x).
    /// - NO_SPECIFIC_INTERFACE (0x80): No specific programming interface.
    /// - USB_DEVICE (0xFE): USB device (not a host controller).
    DECLARE_ENUM(USBProgrammingInterface, USB_PROGRAMMING_INTERFACE_CODES, 0xFF) // NOLINT

#define IPMI_PROGRAMMING_INTERFACE_CODES(X)                                                        \
    X(IPMIProgrammingInterface, SMIC, 0x0)                                                         \
    X(IPMIProgrammingInterface, KEYBOARD_CONTROLLER_STYLE, 0x1)                                    \
    X(IPMIProgrammingInterface, BLOCK_TRANSFER, 0x2)

    /// @brief Programming interfaces for SerialBusSubClass::IPMI (0x0C/0x07).
    ///
    /// - SMIC (0x00): IPMI SMIC interface.
    /// - KEYBOARD_CONTROLLER_STYLE (0x01): IPMI Keyboard Controller Style interface.
    /// - BLOCK_TRANSFER (0x02): IPMI Block Transfer interface.
    DECLARE_ENUM(IPMIProgrammingInterface, IPMI_PROGRAMMING_INTERFACE_CODES, 0xFF) // NOLINT

#define WIRELESS_SUB_CLASS_CODES(X)                                                                \
    X(WirelessSubClass, IRDA, 0x0)                                                                 \
    X(WirelessSubClass, CONSUMER_IR, 0x1)                                                          \
    X(WirelessSubClass, UWB_RADIO, 0x10)                                                           \
    X(WirelessSubClass, RF_CONTROLLER, 0x11)                                                       \
    X(WirelessSubClass, BLUETOOTH, 0x12)                                                           \
    X(WirelessSubClass, BROADBAND, 0x20)                                                           \
    X(WirelessSubClass, ETHERNET_802_11A, 0x21)                                                    \
    X(WirelessSubClass, CELLULAR, 0x40)                                                            \
    X(WirelessSubClass, CELLULAR_WITH_ETHERNET, 0x41)                                              \
    X(WirelessSubClass, OTHER, 0x80)

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
    DECLARE_ENUM(WirelessSubClass, WIRELESS_SUB_CLASS_CODES, 0xFF) // NOLINT

#define INTELLIGENT_IO_SUB_CLASS_CODES(X) X(IntelligentIOSubClass, I2O, 0x0)

    /// @brief Sub-classes for PCI base class INTELLIGENT_IO_CONTROLLER (0x0E).
    ///
    /// - I2O (0x00): Intelligent I/O (I2O) Architecture Specification 1.0.
    DECLARE_ENUM(IntelligentIOSubClass, INTELLIGENT_IO_SUB_CLASS_CODES, 0xFF) // NOLINT

#define SATELLITE_COMMUNICATION_SUB_CLASS_CODES(X)                                                 \
    X(SatelliteCommunicationSubClass, TV, 0x1)                                                     \
    X(SatelliteCommunicationSubClass, AUDIO, 0x2)                                                  \
    X(SatelliteCommunicationSubClass, VOICE, 0x3)                                                  \
    X(SatelliteCommunicationSubClass, DATA, 0x4)                                                   \
    X(SatelliteCommunicationSubClass, OTHER, 0x80)

    /// @brief Sub-classes for PCI base class SATELLITE_COMMUNICATION_CONTROLLER (0x0F).
    ///
    /// - TV (0x01): TV satellite controller.
    /// - AUDIO (0x02): Audio satellite controller.
    /// - VOICE (0x03): Voice satellite controller.
    /// - DATA (0x04): Data satellite controller.
    /// - OTHER (0x80): Other satellite communication controller.
    DECLARE_ENUM(SatelliteCommunicationSubClass,
                 SATELLITE_COMMUNICATION_SUB_CLASS_CODES,
                 0xFF) // NOLINT

#define ENCRYPTION_DECRYPTION_SUB_CLASS_CODES(X)                                                   \
    X(EncryptionDecryptionSubClass, NETWORK_AND_COMPUTING, 0x0)                                    \
    X(EncryptionDecryptionSubClass, ENTERTAINMENT, 0x10)                                           \
    X(EncryptionDecryptionSubClass, OTHER, 0x80)

    /// @brief Sub-classes for PCI base class ENCRYPTION_DECRYPTION_CONTROLLER (0x10).
    ///
    /// - NETWORK_AND_COMPUTING (0x00): Network and computing encryption/decryption controller.
    /// - ENTERTAINMENT (0x10): Entertainment encryption/decryption controller.
    /// - OTHER (0x80): Other encryption/decryption controller.
    DECLARE_ENUM(EncryptionDecryptionSubClass,
                 ENCRYPTION_DECRYPTION_SUB_CLASS_CODES,
                 0xFF) // NOLINT

#define DATA_ACQUISITION_SUB_CLASS_CODES(X)                                                        \
    X(DataAcquisitionSubClass, DPIO_MODULES, 0x0)                                                  \
    X(DataAcquisitionSubClass, PERFORMANCE_COUNTERS, 0x1)                                          \
    X(DataAcquisitionSubClass, COMMUNICATIONS_SYNCHRONIZATION, 0x10)                               \
    X(DataAcquisitionSubClass, MANAGEMENT_CARD, 0x20)                                              \
    X(DataAcquisitionSubClass, OTHER, 0x80)

    /// @brief Sub-classes for PCI base class DATA_ACQUISITION_AND_SIGNAL_PROCESSING_CONTROLLER
    ///        (0x11).
    ///
    /// - DPIO_MODULES (0x00): DPIO modules.
    /// - PERFORMANCE_COUNTERS (0x01): Performance counters.
    /// - COMMUNICATIONS_SYNCHRONIZATION (0x10): Communications synchronization, time and
    ///   frequency test/measurement.
    /// - MANAGEMENT_CARD (0x20): Management card.
    /// - OTHER (0x80): Other data acquisition/signal processing controller.
    DECLARE_ENUM(DataAcquisitionSubClass, DATA_ACQUISITION_SUB_CLASS_CODES, 0xFF) // NOLINT

#define PROCESSING_ACCELERATOR_SUB_CLASS_CODES(X)                                                  \
    X(ProcessingAcceleratorSubClass, PROCESSING_ACCELERATOR, 0x0)

    /// @brief Sub-classes for PCI base class PROCESSING_ACCELERATOR (0x12).
    ///
    /// - PROCESSING_ACCELERATOR (0x00): Processing accelerator (vendor-specific interface).
    DECLARE_ENUM(ProcessingAcceleratorSubClass,
                 PROCESSING_ACCELERATOR_SUB_CLASS_CODES,
                 0xFF) // NOLINT

#define NON_ESSENTIAL_INSTRUMENTATION_SUB_CLASS_CODES(X)                                           \
    X(NonEssentialInstrumentationSubClass, NON_ESSENTIAL_INSTRUMENTATION, 0x0)

    /// @brief Sub-classes for PCI base class NON_ESSENTIAL_INSTRUMENTATION (0x13).
    ///
    /// - NON_ESSENTIAL_INSTRUMENTATION (0x00): Non-essential instrumentation function
    ///   (vendor-specific interface).
    DECLARE_ENUM(NonEssentialInstrumentationSubClass,
                 NON_ESSENTIAL_INSTRUMENTATION_SUB_CLASS_CODES,
                 0xFF) // NOLINT

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

#endif // RUNEOS_CLASSCODE_H
