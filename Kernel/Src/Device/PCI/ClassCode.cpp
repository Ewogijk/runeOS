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

#include <Device/PCI/ClassCode.h>

namespace Rune::Device {
    auto pci_resolve_subclass_code(BaseClass base_class, U8 subclass_code) -> String {
        switch (base_class) {
            case BaseClass::ANCIENT: return AncientSubClass(subclass_code).to_string();
            case BaseClass::MASS_STORAGE_CONTROLLER:
                return MassStorageSubClass(subclass_code).to_string();
            case BaseClass::NETWORK_CONTROLLER: return NetworkSubClass(subclass_code).to_string();
            case BaseClass::DISPLAY_CONTROLLER: return DisplaySubClass(subclass_code).to_string();
            case BaseClass::MULTIMEDIA_DEVICE: return MultimediaSubClass(subclass_code).to_string();
            case BaseClass::MEMORY_CONTROLLER: return MemorySubClass(subclass_code).to_string();
            case BaseClass::BRIDGE_DEVICE:     return BridgeSubClass(subclass_code).to_string();
            case BaseClass::SIMPLE_COMMUNICATION_CONTROLLER:
                return SimpleCommunicationSubClass(subclass_code).to_string();
            case BaseClass::BASE_SYSTEM_PERIPHERAL:
                return BaseSystemPeripheralSubClass(subclass_code).to_string();
            case BaseClass::INPUT_DEVICE: return InputDeviceSubClass(subclass_code).to_string();
            case BaseClass::DOCKING_STATION:
                return DockingStationSubClass(subclass_code).to_string();
            case BaseClass::PROCESSOR: return ProcessorSubClass(subclass_code).to_string();
            case BaseClass::SERIAL_BUS_CONTROLLER:
                return SerialBusSubClass(subclass_code).to_string();
            case BaseClass::WIRELESS_CONTROLLER: return WirelessSubClass(subclass_code).to_string();
            case BaseClass::INTELLIGENT_IO_CONTROLLER:
                return IntelligentIOSubClass(subclass_code).to_string();
            case BaseClass::SATELLITE_COMMUNICATION_CONTROLLER:
                return SatelliteCommunicationSubClass(subclass_code).to_string();
            case BaseClass::ENCRYPTION_DECRYPTION_CONTROLLER:
                return EncryptionDecryptionSubClass(subclass_code).to_string();
            case BaseClass::DATA_ACQUISITION_AND_SIGNAL_PROCESSING_CONTROLLER:
                return DataAcquisitionSubClass(subclass_code).to_string();
            case BaseClass::PROCESSING_ACCELERATOR:
                return ProcessingAcceleratorSubClass(subclass_code).to_string();
            case BaseClass::NON_ESSENTIAL_INSTRUMENTATION:
                return NonEssentialInstrumentationSubClass(subclass_code).to_string();
            default: return "NONE";
        }
    }

    auto pci_resolve_programming_interface(BaseClass base_class,
                                           U8        subclass_code,
                                           U8        programming_interface) -> String {
        switch (base_class) {
            case BaseClass::MASS_STORAGE_CONTROLLER:
                switch (subclass_code) {
                    case MassStorageSubClass::SCSI_CONTROLLER:
                        return SCSIProgrammingInterface(programming_interface).to_string();
                    case MassStorageSubClass::ATA_CONTROLLER:
                        return ATAProgrammingInterface(programming_interface).to_string();
                    case MassStorageSubClass::SERIAL_ATA_CONTROLLER:
                        return SerialATAProgrammingInterface(programming_interface).to_string();
                    case MassStorageSubClass::SERIAL_ATTACHED_SCSI_CONTROLLER:
                        return SerialAttachedSCSIProgrammingInterface(programming_interface)
                            .to_string();
                    case MassStorageSubClass::NON_VOLATILE_MEMORY_CONTROLLER:
                        return NVMProgrammingInterface(programming_interface).to_string();
                    case MassStorageSubClass::UNIVERSAL_FLASH_STORAGE_CONTROLLER:
                        return UFSProgrammingInterface(programming_interface).to_string();
                    default: return "NONE";
                }
            case BaseClass::DISPLAY_CONTROLLER:
                switch (subclass_code) {
                    case DisplaySubClass::VGA_COMPATIBLE_CONTROLLER:
                        return VGAProgrammingInterface(programming_interface).to_string();
                    default: return "NONE";
                }
            case BaseClass::MULTIMEDIA_DEVICE:
                switch (subclass_code) {
                    case MultimediaSubClass::HIGH_DEFINITION_AUDIO:
                        return HDAudioProgrammingInterface(programming_interface).to_string();
                    default: return "NONE";
                }
            case BaseClass::BRIDGE_DEVICE:
                switch (subclass_code) {
                    case BridgeSubClass::PCI_TO_PCI_BRIDGE:
                        return PCIBridgeProgrammingInterface(programming_interface).to_string();
                    case BridgeSubClass::SEMI_TRANSPARENT_PCI_TO_PCI_BRIDGE:
                        return SemiTransparentBridgeProgrammingInterface(programming_interface)
                            .to_string();
                    case BridgeSubClass::ADVANCED_SWITCHING_TO_PCI_HOST_BRIDGE:
                        return AdvancedSwitchingBridgeProgrammingInterface(programming_interface)
                            .to_string();
                    default: return "NONE";
                }
            case BaseClass::SIMPLE_COMMUNICATION_CONTROLLER:
                switch (subclass_code) {
                    case SimpleCommunicationSubClass::SERIAL_CONTROLLER:
                        return SerialControllerProgrammingInterface(programming_interface)
                            .to_string();
                    case SimpleCommunicationSubClass::PARALLEL_PORT:
                        return ParallelPortProgrammingInterface(programming_interface).to_string();
                    case SimpleCommunicationSubClass::MODEM:
                        return ModemProgrammingInterface(programming_interface).to_string();
                    default: return "NONE";
                }
            case BaseClass::BASE_SYSTEM_PERIPHERAL:
                switch (subclass_code) {
                    case BaseSystemPeripheralSubClass::PIC:
                        return PICProgrammingInterface(programming_interface).to_string();
                    case BaseSystemPeripheralSubClass::DMA_CONTROLLER:
                        return DMAProgrammingInterface(programming_interface).to_string();
                    case BaseSystemPeripheralSubClass::SYSTEM_TIMER:
                        return SystemTimerProgrammingInterface(programming_interface).to_string();
                    case BaseSystemPeripheralSubClass::RTC_CONTROLLER:
                        return RTCProgrammingInterface(programming_interface).to_string();
                    default: return "NONE";
                }
            case BaseClass::INPUT_DEVICE:
                switch (subclass_code) {
                    case InputDeviceSubClass::GAMEPORT_CONTROLLER:
                        return GameportProgrammingInterface(programming_interface).to_string();
                    default: return "NONE";
                }
            case BaseClass::SERIAL_BUS_CONTROLLER:
                switch (subclass_code) {
                    case SerialBusSubClass::FIREWIRE:
                        return FireWireProgrammingInterface(programming_interface).to_string();
                    case SerialBusSubClass::USB:
                        return USBProgrammingInterface(programming_interface).to_string();
                    case SerialBusSubClass::IPMI:
                        return IPMIProgrammingInterface(programming_interface).to_string();
                    default: return "NONE";
                }
            default: return "NONE";
        }
    }
} // namespace Rune::Device