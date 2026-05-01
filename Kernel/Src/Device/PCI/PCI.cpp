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

#include <Device/PCI/PCI.h>

#include <KRE/BitsAndBytes.h>
#include <KRE/Logging.h>

#include <CPU/IO.h>

#include <Device/PCI/ClassCode.h>
#include <Device/PCI/VendorDB.h>

namespace Rune::Device {
    const SharedPointer<Logger> LOGGER = LogContext::instance().get_logger("Device.PCI");

    // ========================================================================================== //
    // PCI Access
    // ========================================================================================== //

    constexpr U16 PCI_CONFIG = 0xCF8;
    constexpr U16 PCI_DATA   = 0xCFC;

    constexpr U8 ENABLE_BIT_SHIFT      = 31;
    constexpr U8 BUS_NUMBER_SHIFT      = 16;
    constexpr U8 DEVICE_NUMBER_SHIFT   = 11;
    constexpr U8 FUNCTION_NUMBER_SHIFT = 8;
    constexpr U8 REGISTER_OFFSET_MASK  = 0xFC;

    constexpr U16 INVALID_VENDOR = 0xFFFF;
    constexpr U16 BUS_LIMIT      = 256;
    constexpr U8  DEVICE_LIMIT   = 32;
    constexpr U8  FUNCTION_LIMIT = 8;

    auto pci_read_byte(U8 bus, U8 device, U8 func, U8 offset) -> U8 {
        CPU::out_dw(PCI_CONFIG,
                    (1 << ENABLE_BIT_SHIFT | (bus << BUS_NUMBER_SHIFT)
                     | (device << DEVICE_NUMBER_SHIFT) | (func << FUNCTION_NUMBER_SHIFT)
                     | (offset & REGISTER_OFFSET_MASK)));
        return CPU::in_b(PCI_DATA + (offset & 0x03));
    }

    void pci_write_byte(U8 bus, U8 device, U8 func, U8 offset, U8 value) {
        CPU::out_dw(PCI_CONFIG,
                    (1 << ENABLE_BIT_SHIFT | (bus << BUS_NUMBER_SHIFT)
                     | (device << DEVICE_NUMBER_SHIFT) | (func << FUNCTION_NUMBER_SHIFT)
                     | (offset & REGISTER_OFFSET_MASK)));
        CPU::out_b(PCI_DATA + (offset & 0x03), value);
    }

    auto pci_read_word(U8 bus, U8 device, U8 func, U8 offset) -> U16 {
        if ((offset & 0x03) > 2) {
            return (pci_read_byte(bus, device, func, offset + 1) << SHIFT_8)
                   | pci_read_byte(bus, device, func, offset);
        }
        CPU::out_dw(PCI_CONFIG,
                    (1 << ENABLE_BIT_SHIFT | (bus << BUS_NUMBER_SHIFT)
                     | (device << DEVICE_NUMBER_SHIFT) | (func << FUNCTION_NUMBER_SHIFT)
                     | (offset & REGISTER_OFFSET_MASK)));
        return CPU::in_w(PCI_DATA + (offset & 0x03));
    }

    void pci_write_word(U8 bus, U8 device, U8 func, U8 offset, U16 value) {
        if ((offset & 0x03) > 2) {
            pci_write_byte(bus, device, func, offset, (U8) value);
            pci_write_byte(bus, device, func, offset + 1, (U8) value >> SHIFT_8);
        }
        CPU::out_dw(PCI_CONFIG,
                    (1 << ENABLE_BIT_SHIFT | (bus << BUS_NUMBER_SHIFT)
                     | (device << DEVICE_NUMBER_SHIFT) | (func << FUNCTION_NUMBER_SHIFT)
                     | (offset & REGISTER_OFFSET_MASK)));
        CPU::out_w(PCI_DATA + (offset & 0x03), value);
    }

    auto pci_read_dword(U8 bus, U8 device, U8 func, U8 offset) -> U32 {
        if ((offset & 0x03) > 0) {
            return (pci_read_word(bus, device, func, offset + 2) << SHIFT_16)
                   | pci_read_word(bus, device, func, offset);
        }
        CPU::out_dw(PCI_CONFIG,
                    (1 << ENABLE_BIT_SHIFT | (bus << BUS_NUMBER_SHIFT)
                     | (device << DEVICE_NUMBER_SHIFT) | (func << FUNCTION_NUMBER_SHIFT)
                     | (offset & REGISTER_OFFSET_MASK)));
        return CPU::in_dw(PCI_DATA + (offset & 0x03));
    }

    void pci_write_dword(U8 bus, U8 device, U8 func, U8 offset, U32 value) {
        if ((offset & 0x03) > 2) {
            pci_write_word(bus, device, func, offset, (uint16_t) value);
            pci_write_word(bus, device, func, offset + 2, (uint16_t) value >> SHIFT_16);
        }
        CPU::out_dw(PCI_CONFIG,
                    (1 << ENABLE_BIT_SHIFT | (bus << BUS_NUMBER_SHIFT)
                     | (device << DEVICE_NUMBER_SHIFT) | (func << FUNCTION_NUMBER_SHIFT)
                     | (offset & REGISTER_OFFSET_MASK)));
        CPU::out_dw(PCI_DATA + (offset & 0x03), value);
    }

    auto pci_read_configuration_space_header_common(U8 bus, U8 device, U8 func)
        -> PCIConfigurationSpaceHeaderCommon {
        constexpr U8 VENDOR_ID_OFFSET             = 0x00;
        constexpr U8 DEVICE_ID_OFFSET             = 0x02;
        constexpr U8 COMMAND_OFFSET               = 0x04;
        constexpr U8 STATUS_OFFSET                = 0x06;
        constexpr U8 REVISION_ID_OFFSET           = 0x08;
        constexpr U8 PROGRAMMING_INTERFACE_OFFSET = 0x09;
        constexpr U8 SUB_CLASS_CODE_OFFSET        = 0x0A;
        constexpr U8 BASE_CLASS_CODE_OFFSET       = 0x0B;
        constexpr U8 CACHE_LINE_SIZE_OFFSET       = 0x0C;
        constexpr U8 LATENCY_TIMER_OFFSET         = 0x0D;
        constexpr U8 HEADER_TYPE_OFFSET           = 0x0E;
        constexpr U8 BIST_OFFSET                  = 0x0F;
        return {
            .vendor_id             = pci_read_word(bus, device, func, VENDOR_ID_OFFSET),
            .device_id             = pci_read_word(bus, device, func, DEVICE_ID_OFFSET),
            .command               = {.AsUInt16 = pci_read_word(bus, device, func, COMMAND_OFFSET)},
            .status                = pci_read_word(bus, device, func, STATUS_OFFSET),
            .revision_id           = pci_read_byte(bus, device, func, REVISION_ID_OFFSET),
            .programming_interface = pci_read_byte(bus, device, func, PROGRAMMING_INTERFACE_OFFSET),
            .sub_class_code        = pci_read_byte(bus, device, func, SUB_CLASS_CODE_OFFSET),
            .base_class_code       = pci_read_byte(bus, device, func, BASE_CLASS_CODE_OFFSET),
            .cache_line_size       = pci_read_byte(bus, device, func, CACHE_LINE_SIZE_OFFSET),
            .latency_timer         = pci_read_byte(bus, device, func, LATENCY_TIMER_OFFSET),
            .header_type           = pci_read_byte(bus, device, func, HEADER_TYPE_OFFSET),
            .bist                  = pci_read_byte(bus, device, func, BIST_OFFSET),
        };
    }

    auto
    pci_read_configuration_space_header_type0(const PCIConfigurationSpaceHeaderCommon& csh_common,
                                              U8                                       bus,
                                              U8                                       device,
                                              U8 func) -> PCIConfigurationSpaceHeaderType0 {
        constexpr U8 BAR_0_OFFSET                      = 0x10;
        constexpr U8 BAR_1_OFFSET                      = 0x14;
        constexpr U8 BAR_2_OFFSET                      = 0x18;
        constexpr U8 BAR_3_OFFSET                      = 0x1C;
        constexpr U8 BAR_4_OFFSET                      = 0x20;
        constexpr U8 BAR_5_OFFSET                      = 0x24;
        constexpr U8 CARD_BUS_CIS_POINTER_OFFSET       = 0x28;
        constexpr U8 SUBSYSTEM_VENDOR_ID_OFFSET        = 0x2C;
        constexpr U8 SUBSYSTEM_ID_OFFSET               = 0x2E;
        constexpr U8 EXPANSION_ROM_ADDRESS_BASE_OFFSET = 0x30;
        constexpr U8 CAPABILITIES_POINTER_OFFSET       = 0x34;
        constexpr U8 INTERRUPT_LINE_OFFSET             = 0x3C;
        constexpr U8 INTERRUPT_PIN_OFFSET              = 0x3D;
        constexpr U8 MAX_GRANT_OFFSET                  = 0x3E;
        constexpr U8 MAX_LATENCY_OFFSET                = 0x3F;
        return {
            .header               = csh_common,
            .bar_0                = pci_read_dword(bus, device, func, BAR_0_OFFSET),
            .bar_1                = pci_read_dword(bus, device, func, BAR_1_OFFSET),
            .bar_2                = pci_read_dword(bus, device, func, BAR_2_OFFSET),
            .bar_3                = pci_read_dword(bus, device, func, BAR_3_OFFSET),
            .bar_4                = pci_read_dword(bus, device, func, BAR_4_OFFSET),
            .bar_5                = pci_read_dword(bus, device, func, BAR_5_OFFSET),
            .card_bus_cis_pointer = pci_read_dword(bus, device, func, CARD_BUS_CIS_POINTER_OFFSET),
            .subsystem_vendor_id  = pci_read_word(bus, device, func, SUBSYSTEM_VENDOR_ID_OFFSET),
            .subsystem_id         = pci_read_word(bus, device, func, SUBSYSTEM_ID_OFFSET),
            .expansion_rom_address_base =
                pci_read_dword(bus, device, func, EXPANSION_ROM_ADDRESS_BASE_OFFSET),
            .capabilities_pointer = pci_read_byte(bus, device, func, CAPABILITIES_POINTER_OFFSET),
            .reserved_0           = 0,
            .reserved_1           = 0,
            .reserved_2           = 0,
            .interrupt_line       = pci_read_byte(bus, device, func, INTERRUPT_LINE_OFFSET),
            .interrupt_pin        = pci_read_byte(bus, device, func, INTERRUPT_PIN_OFFSET),
            .min_grant            = pci_read_byte(bus, device, func, MAX_GRANT_OFFSET),
            .max_latency          = pci_read_byte(bus, device, func, MAX_LATENCY_OFFSET),
        };
    }

    // void pci_check_device(AHCIDriver* ahci_driver, U8 bus, U8 device) {
    //     PCIConfigurationSpaceHeaderCommon header =
    //         pci_read_configuration_space_header_common(bus, device, 0);
    //     if (header.vendor_id == INVALID_VENDOR) return;
    //
    //     PCIVendorDBResponse resp = pci_vendor_db_resolve(
    //         {.m_vendor_ID = header.vendor_id, .m_device_ID = header.device_id});
    //     BaseClass bc(header.base_class_code);
    //     String    subclass = pci_resolve_subclass_code(bc, header.sub_class_code);
    //     String    prog_int = pci_resolve_programming_interface(bc,
    //                                                         header.sub_class_code,
    //                                                         header.programming_interface);
    //
    //     LOGGER->debug("B{}-D{}-F{}: {}-{} ({:#x}:{:#x}) - {} ({:#x}), {} ({:#x}), {} ({:#x}),",
    //                   bus,
    //                   device,
    //                   0,
    //                   resp.m_vendor_name,
    //                   resp.m_device_name,
    //                   header.vendor_id,
    //                   header.device_id,
    //                   bc.to_string(),
    //                   header.base_class_code,
    //                   subclass,
    //                   header.sub_class_code,
    //                   prog_int,
    //                   header.programming_interface);
    //
    //     constexpr U8  AHCI_QEMU_BASE_CLASS     = 0x1;
    //     constexpr U8  AHCI_QEMU_SUB_CLASS_CODE = 0x6;
    //     constexpr U16 AHCI_QEMU_VENDOR_ID      = 0x8086;
    //     constexpr U16 AHCI_QEMU_DEVICE_ID      = 0x2922;
    //     if (header.base_class_code == AHCI_QEMU_BASE_CLASS
    //         && header.sub_class_code == AHCI_QEMU_SUB_CLASS_CODE
    //         && header.vendor_id == AHCI_QEMU_VENDOR_ID && header.device_id ==
    //         AHCI_QEMU_DEVICE_ID) { pci_write_word(bus, device, 0, 0x04, header.command.AsUInt16 |
    //         0x4); // Enable DMA constexpr U8                     BAR_0_OFFSET = 0x10; constexpr
    //         U8                     BAR_1_OFFSET                      = 0x14; constexpr U8
    //         BAR_2_OFFSET                      = 0x18; constexpr U8 BAR_3_OFFSET = 0x1C; constexpr
    //         U8                     BAR_4_OFFSET                      = 0x20; constexpr U8
    //         BAR_5_OFFSET                      = 0x24; constexpr U8 CARD_BUS_CIS_POINTER_OFFSET =
    //         0x28; constexpr U8                     SUBSYSTEM_VENDOR_ID_OFFSET        = 0x2C;
    //         constexpr U8                     SUBSYSTEM_ID_OFFSET               = 0x2E;
    //         constexpr U8                     EXPANSION_ROM_ADDRESS_BASE_OFFSET = 0x30;
    //         constexpr U8                     CAPABILITIES_POINTER_OFFSET       = 0x34;
    //         constexpr U8                     INTERRUPT_LINE_OFFSET             = 0x3C;
    //         constexpr U8                     INTERRUPT_PIN_OFFSET              = 0x3D;
    //         constexpr U8                     MAX_GRANT_OFFSET                  = 0x3E;
    //         constexpr U8                     MAX_LATENCY_OFFSET                = 0x3F;
    //         PCIConfigurationSpaceHeaderType0 ahci_header                       = {
    //                                   .header               =
    //                                   pci_read_configuration_space_header_common(bus, device, 0),
    //                                   .bar_0                = pci_read_dword(bus, device, 0,
    //                                   BAR_0_OFFSET), .bar_1                = pci_read_dword(bus,
    //                                   device, 0, BAR_1_OFFSET), .bar_2                =
    //                                   pci_read_dword(bus, device, 0, BAR_2_OFFSET), .bar_3 =
    //                                   pci_read_dword(bus, device, 0, BAR_3_OFFSET), .bar_4 =
    //                                   pci_read_dword(bus, device, 0, BAR_4_OFFSET), .bar_5 =
    //                                   pci_read_dword(bus, device, 0, BAR_5_OFFSET),
    //                                   .card_bus_cis_pointer = pci_read_dword(bus, device, 0,
    //                                   CARD_BUS_CIS_POINTER_OFFSET), .subsystem_vendor_id  =
    //                                   pci_read_word(bus, device, 0, SUBSYSTEM_VENDOR_ID_OFFSET),
    //                                   .subsystem_id         = pci_read_word(bus, device, 0,
    //                                   SUBSYSTEM_ID_OFFSET), .expansion_rom_address_base =
    //                 pci_read_dword(bus, device, 0, EXPANSION_ROM_ADDRESS_BASE_OFFSET),
    //                                   .capabilities_pointer = pci_read_byte(bus, device, 0,
    //                                   CAPABILITIES_POINTER_OFFSET), .reserved_0           = 0,
    //                                   .reserved_1           = 0,
    //                                   .reserved_2           = 0,
    //                                   .interrupt_line       = pci_read_byte(bus, device, 0,
    //                                   INTERRUPT_LINE_OFFSET), .interrupt_pin        =
    //                                   pci_read_byte(bus, device, 0, INTERRUPT_PIN_OFFSET),
    //                                   .min_grant            = pci_read_byte(bus, device, 0,
    //                                   MAX_GRANT_OFFSET), .max_latency          =
    //                                   pci_read_byte(bus, device, 0, MAX_LATENCY_OFFSET),
    //         };
    //         volatile auto* hba = reinterpret_cast<HBAMemory*>(
    //             Memory::physical_to_virtual_address(ahci_header.bar_5));
    //         if (!ahci_driver->start(hba)) {
    //             LOGGER->error("Failed to init AHCI");
    //             while (true);
    //         }
    //     }
    //
    //     constexpr U8 FUNC_LIMIT            = 8;
    //     constexpr U8 MULTI_FUNCTION_DEVICE = 0x80;
    //     if ((header.header_type & MULTI_FUNCTION_DEVICE) != 0) {
    //         for (U8 func = 1; func < FUNC_LIMIT; func++) {
    //             header = pci_read_configuration_space_header_common(bus, device, func);
    //             if (header.vendor_id == INVALID_VENDOR) continue;
    //             PCIVendorDBResponse resp = pci_vendor_db_resolve(
    //                 {.m_vendor_ID = header.vendor_id, .m_device_ID = header.device_id});
    //             BaseClass bc(header.base_class_code);
    //             String    subclass = pci_resolve_subclass_code(bc, header.sub_class_code);
    //             String    prog_int = pci_resolve_programming_interface(bc,
    //                                                                 header.sub_class_code,
    //                                                                 header.programming_interface);
    //             LOGGER->debug(
    //                 "B{}-D{}-F{}: {}-{} ({:#x}:{:#x}) - {} ({:#x}), {} ({:#x}), {} ({:#x}),",
    //                 bus,
    //                 device,
    //                 0,
    //                 resp.m_vendor_name,
    //                 resp.m_device_name,
    //                 header.vendor_id,
    //                 header.device_id,
    //                 bc.to_string(),
    //                 header.base_class_code,
    //                 subclass,
    //                 header.sub_class_code,
    //                 prog_int,
    //                 header.programming_interface);
    //         }
    //     }
    // }
    //
    // void pci_discover_devices(AHCIDriver* ahci_driver) {
    //     pci_vendor_db_initialize();
    //     for (U16 bus = 0; bus < BUS_LIMIT; bus++) {
    //         for (U8 device = 0; device < DEVICE_LIMIT; device++) {
    //             pci_check_device(ahci_driver, bus, device);
    //         }
    //     }
    // }

    // ========================================================================================== //
    // PCIDriver
    // ========================================================================================== //

    const BasicDeviceID PCIDriver::ID_PCI("PCI");

    auto PCIDriver::map_device(U16                          bus,
                               U8                           device,
                               U8                           func,
                               DeviceHandle                 bus_device,
                               const DeviceMapper&          device_mapper,
                               HandleCounter<DeviceHandle>& dev_handle_counter) -> bool {
        PCIConfigurationSpaceHeaderCommon header =
            pci_read_configuration_space_header_common(bus, device, func);
        if (header.vendor_id == INVALID_VENDOR) return false;

        PCIVendorDBResponse resp = pci_vendor_db_resolve(
            {.m_vendor_ID = header.vendor_id, .m_device_ID = header.device_id});
        BaseClass bc(header.base_class_code);
        String    subclass = pci_resolve_subclass_code(bc, header.sub_class_code);
        String    prog_int = pci_resolve_programming_interface(bc,
                                                            header.sub_class_code,
                                                            header.programming_interface);

        LOGGER->debug("B{}-D{}-F{}: {}-{} ({:#x}:{:#x}) - {} ({:#x}), {} ({:#x}), {} ({:#x}),",
                      bus,
                      device,
                      0,
                      resp.m_vendor_name,
                      resp.m_device_name,
                      header.vendor_id,
                      header.device_id,
                      bc.to_string(),
                      header.base_class_code,
                      subclass,
                      header.sub_class_code,
                      prog_int,
                      header.programming_interface);

        SharedPointer<Device> dev(new PCIDevice(dev_handle_counter.acquire(),
                                                resp.m_device_name,
                                                resp.m_vendor_name,
                                                int_to_string(header.revision_id, 10),
                                                "",
                                                DeviceType::GENERIC,
                                                PCIDeviceID(header.base_class_code,
                                                            header.sub_class_code,
                                                            header.programming_interface)));
        if (header.get_header_layout() == 0x0) {
            PCIConfigurationSpaceHeaderType0 csh_type0 =
                pci_read_configuration_space_header_type0(header, bus, device, 0);
            device_mapper(bus_device, dev, &csh_type0);
        } else {
            LOGGER->warn("PCI Header Type{} detected but it is not supported yet!",
                         header.get_header_layout());
        }
        return header.is_multi_function_device();
    }

    PCIDriver::PCIDriver(DriverHandle handle) : Driver(handle, ID_PCI.get_string_ID()) {}

    auto PCIDriver::get_target_device_ID() const -> const DeviceID* { return &ID_PCI; }

    auto PCIDriver::start(DeviceHandle dev_handle, void* context) -> bool {
        SILENCE_UNUSED(context)
        pci_vendor_db_initialize();
        add_operated_device(dev_handle);
        return true;
    }

    auto PCIDriver::stop(DeviceHandle dev_handle) -> bool { return false; }

    auto PCIDriver::handle_request(DeviceHandle dev_handle, IORequest request) -> IORequestStatus {
        SILENCE_UNUSED(dev_handle)
        SILENCE_UNUSED(request)
        return IORequestStatus::UNSUPPORTED;
    }

    void PCIDriver::discover_devices(DeviceHandle                 bus_device,
                                     const DeviceMapper&          device_mapper,
                                     HandleCounter<DeviceHandle>& dev_handle_counter) {
        for (U16 bus = 0; bus < BUS_LIMIT; bus++) {
            for (U8 device = 0; device < DEVICE_LIMIT; device++) {
                if (map_device(bus, device, 0, bus_device, device_mapper, dev_handle_counter)) {
                    for (U8 func = 1; func < FUNCTION_LIMIT; func++) {
                        map_device(bus,
                                   device,
                                   func,
                                   bus_device,
                                   device_mapper,
                                   dev_handle_counter);
                    }
                }
            }
        }
    }

} // namespace Rune::Device
