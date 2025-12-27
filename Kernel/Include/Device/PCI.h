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

#ifndef RUNEOS_PCI_H
#define RUNEOS_PCI_H

#include <CPU/IO.h>

#include <Memory/Paging.h>

#include <Device/AHCI/AHCI.h>

namespace Rune::Device {
    const SharedPointer<Logger> PCI_LOGGER = LogContext::instance().get_logger("Device.PCI");

    union CommandRegister {
        U16 AsUInt16 = 0;
        struct {
            U16 io_space                           : 1;
            U16 memory_space                       : 1;
            U16 bus_master                         : 1;
            U16 special_cycles                     : 1;
            U16 memory_write_and_invalidate_enable : 1;
            U16 vga_palette_snoop                  : 1;
            U16 parity_error_response              : 1;
            U16 reserved_0                         : 1;
            U16 serr_enable                        : 1;
            U16 fast_back_to_back_enable           : 1;
            U16 interrupt_disable                  : 1;
            U16 reserved_1                         : 5;
        };
    };

    struct PCIHeader {
        U16             vendor_id = 0;
        U16             device_id = 0;
        CommandRegister command;
        U16             status                = 0;
        U8              revision_id           = 0;
        U8              programming_interface = 0;
        U8              sub_class_code        = 0;
        U8              base_class_code       = 0;
        U8              cache_line_size       = 0;
        U8              latency_timer         = 0;
        U8              header_type           = 0;
        U8              bist                  = 0;
    };

    struct PCIHeaderType0 {
        PCIHeader header;
        U32       bar_0{};
        U32       bar_1{};
        U32       bar_2{};
        U32       bar_3{};
        U32       bar_4{};
        U32       bar_5{};
        U32       card_bus_cis_pointer{};
        U16       subsystem_vendor_id{};
        U16       subsystem_id{};
        U32       expansion_rom_address_base{};
        U8        capabilities_pointer{};
        U8        reserved_0{};
        U16       reserved_1{};
        U32       reserved_2{};
        U8        interrupt_line{};
        U8        interrupt_pin{};
        U8        min_grant{};
        U8        max_latency{};
    };

    class PCI {
        static constexpr U16 PCI_CONFIG = 0xCF8;
        static constexpr U16 PCI_DATA   = 0xCFC;

        static constexpr U8 ENABLE_BIT_SHIFT      = 31;
        static constexpr U8 BUS_NUMBER_SHIFT      = 16;
        static constexpr U8 DEVICE_NUMBER_SHIFT   = 11;
        static constexpr U8 FUNCTION_NUMBER_SHIFT = 8;
        static constexpr U8 REGISTER_OFFSET_MASK  = 0xFC;

      public:
        static auto read_byte(U8 bus, U8 device, U8 func, U8 offset) -> U8 {
            CPU::out_dw(PCI_CONFIG,
                        (1 << ENABLE_BIT_SHIFT | (bus << BUS_NUMBER_SHIFT)
                         | (device << DEVICE_NUMBER_SHIFT) | (func << FUNCTION_NUMBER_SHIFT)
                         | (offset & REGISTER_OFFSET_MASK)));
            return CPU::in_b(PCI_DATA + (offset & 0x03));
        }

        static void write_byte(U8 bus, U8 device, U8 func, U8 offset, U8 value) {
            CPU::out_dw(PCI_CONFIG,
                        (1 << ENABLE_BIT_SHIFT | (bus << BUS_NUMBER_SHIFT)
                         | (device << DEVICE_NUMBER_SHIFT) | (func << FUNCTION_NUMBER_SHIFT)
                         | (offset & REGISTER_OFFSET_MASK)));
            CPU::out_b(PCI_DATA + (offset & 0x03), value);
        }

        static auto read_word(U8 bus, U8 device, U8 func, U8 offset) -> U16 {
            if ((offset & 0x03) > 2) {
                return (read_byte(bus, device, func, offset + 1) << SHIFT_8)
                       | read_byte(bus, device, func, offset);
            }
            CPU::out_dw(PCI_CONFIG,
                        (1 << ENABLE_BIT_SHIFT | (bus << BUS_NUMBER_SHIFT)
                         | (device << DEVICE_NUMBER_SHIFT) | (func << FUNCTION_NUMBER_SHIFT)
                         | (offset & REGISTER_OFFSET_MASK)));
            return CPU::in_w(PCI_DATA + (offset & 0x03));
        }

        static void write_word(U8 bus, U8 device, U8 func, U8 offset, U16 value) {
            if ((offset & 0x03) > 2) {
                write_byte(bus, device, func, offset, (U8) value);
                write_byte(bus, device, func, offset + 1, (U8) value >> SHIFT_8);
            }
            CPU::out_dw(PCI_CONFIG,
                        (1 << ENABLE_BIT_SHIFT | (bus << BUS_NUMBER_SHIFT)
                         | (device << DEVICE_NUMBER_SHIFT) | (func << FUNCTION_NUMBER_SHIFT)
                         | (offset & REGISTER_OFFSET_MASK)));
            CPU::out_w(PCI_DATA + (offset & 0x03), value);
        }

        static auto read_dword(U8 bus, U8 device, U8 func, U8 offset) -> U32 {
            if ((offset & 0x03) > 0) {
                return (read_word(bus, device, func, offset + 2) << SHIFT_16)
                       | read_word(bus, device, func, offset);
            }
            CPU::out_dw(PCI_CONFIG,
                        (1 << ENABLE_BIT_SHIFT | (bus << BUS_NUMBER_SHIFT)
                         | (device << DEVICE_NUMBER_SHIFT) | (func << FUNCTION_NUMBER_SHIFT)
                         | (offset & REGISTER_OFFSET_MASK)));
            return CPU::in_dw(PCI_DATA + (offset & 0x03));
        }

        static void write_dword(U8 bus, U8 device, U8 func, U8 offset, U32 value) {
            if ((offset & 0x03) > 2) {
                write_word(bus, device, func, offset, (uint16_t) value);
                write_word(bus, device, func, offset + 2, (uint16_t) value >> SHIFT_16);
            }
            CPU::out_dw(PCI_CONFIG,
                        (1 << ENABLE_BIT_SHIFT | (bus << BUS_NUMBER_SHIFT)
                         | (device << DEVICE_NUMBER_SHIFT) | (func << FUNCTION_NUMBER_SHIFT)
                         | (offset & REGISTER_OFFSET_MASK)));
            CPU::out_dw(PCI_DATA + (offset & 0x03), value);
        }

        static auto read_header(U8 bus, U8 device, U8 func) -> PCIHeader {
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
                .vendor_id             = read_word(bus, device, func, VENDOR_ID_OFFSET),
                .device_id             = read_word(bus, device, func, DEVICE_ID_OFFSET),
                .command               = {.AsUInt16 = read_word(bus, device, func, COMMAND_OFFSET)},
                .status                = read_word(bus, device, func, STATUS_OFFSET),
                .revision_id           = read_byte(bus, device, func, REVISION_ID_OFFSET),
                .programming_interface = read_byte(bus, device, func, PROGRAMMING_INTERFACE_OFFSET),
                .sub_class_code        = read_byte(bus, device, func, SUB_CLASS_CODE_OFFSET),
                .base_class_code       = read_byte(bus, device, func, BASE_CLASS_CODE_OFFSET),
                .cache_line_size       = read_byte(bus, device, func, CACHE_LINE_SIZE_OFFSET),
                .latency_timer         = read_byte(bus, device, func, LATENCY_TIMER_OFFSET),
                .header_type           = read_byte(bus, device, func, HEADER_TYPE_OFFSET),
                .bist                  = read_byte(bus, device, func, BIST_OFFSET),
            };
        }

        static void check_device(AHCIDriver& ahci_driver, U8 bus, U8 device) {
            constexpr U16 INVALID_VENDOR = 0xFFFF;
            PCIHeader     header         = read_header(bus, device, 0);
            if (header.vendor_id == INVALID_VENDOR) return;

            PCI_LOGGER->debug("Bus: {}, Device: {}, Function: {} - {:#x}:{:#x} - Base Class Code: "
                              "{:#x} - Sub Class: {:#x} "
                              "- Programming Interface: {:#x}",
                              bus,
                              device,
                              0,
                              header.vendor_id,
                              header.device_id,
                              header.base_class_code,
                              header.sub_class_code,
                              header.programming_interface);

            constexpr U8  AHCI_QEMU_BASE_CLASS     = 0x1;
            constexpr U8  AHCI_QEMU_SUB_CLASS_CODE = 0x6;
            constexpr U16 AHCI_QEMU_VENDOR_ID      = 0x8086;
            constexpr U16 AHCI_QEMU_DEVICE_ID      = 0x2922;
            if (header.base_class_code == AHCI_QEMU_BASE_CLASS
                && header.sub_class_code == AHCI_QEMU_SUB_CLASS_CODE
                && header.vendor_id == AHCI_QEMU_VENDOR_ID
                && header.device_id == AHCI_QEMU_DEVICE_ID) {
                write_word(bus, device, 0, 0x04, header.command.AsUInt16 | 0x4); // Enable DMA
                constexpr U8   BAR_0_OFFSET                      = 0x10;
                constexpr U8   BAR_1_OFFSET                      = 0x14;
                constexpr U8   BAR_2_OFFSET                      = 0x18;
                constexpr U8   BAR_3_OFFSET                      = 0x1C;
                constexpr U8   BAR_4_OFFSET                      = 0x20;
                constexpr U8   BAR_5_OFFSET                      = 0x24;
                constexpr U8   CARD_BUS_CIS_POINTER_OFFSET       = 0x28;
                constexpr U8   SUBSYSTEM_VENDOR_ID_OFFSET        = 0x2C;
                constexpr U8   SUBSYSTEM_ID_OFFSET               = 0x2E;
                constexpr U8   EXPANSION_ROM_ADDRESS_BASE_OFFSET = 0x30;
                constexpr U8   CAPABILITIES_POINTER_OFFSET       = 0x34;
                constexpr U8   INTERRUPT_LINE_OFFSET             = 0x3C;
                constexpr U8   INTERRUPT_PIN_OFFSET              = 0x3D;
                constexpr U8   MAX_GRANT_OFFSET                  = 0x3E;
                constexpr U8   MAX_LATENCY_OFFSET                = 0x3F;
                PCIHeaderType0 ahci_header                       = {
                                          .header               = read_header(bus, device, 0),
                                          .bar_0                = read_dword(bus, device, 0, BAR_0_OFFSET),
                                          .bar_1                = read_dword(bus, device, 0, BAR_1_OFFSET),
                                          .bar_2                = read_dword(bus, device, 0, BAR_2_OFFSET),
                                          .bar_3                = read_dword(bus, device, 0, BAR_3_OFFSET),
                                          .bar_4                = read_dword(bus, device, 0, BAR_4_OFFSET),
                                          .bar_5                = read_dword(bus, device, 0, BAR_5_OFFSET),
                                          .card_bus_cis_pointer = read_dword(bus, device, 0, CARD_BUS_CIS_POINTER_OFFSET),
                                          .subsystem_vendor_id  = read_word(bus, device, 0, SUBSYSTEM_VENDOR_ID_OFFSET),
                                          .subsystem_id         = read_word(bus, device, 0, SUBSYSTEM_ID_OFFSET),
                                          .expansion_rom_address_base =
                        read_dword(bus, device, 0, EXPANSION_ROM_ADDRESS_BASE_OFFSET),
                                          .capabilities_pointer = read_byte(bus, device, 0, CAPABILITIES_POINTER_OFFSET),
                                          .reserved_0           = 0,
                                          .reserved_1           = 0,
                                          .reserved_2           = 0,
                                          .interrupt_line       = read_byte(bus, device, 0, INTERRUPT_LINE_OFFSET),
                                          .interrupt_pin        = read_byte(bus, device, 0, INTERRUPT_PIN_OFFSET),
                                          .min_grant            = read_byte(bus, device, 0, MAX_GRANT_OFFSET),
                                          .max_latency          = read_byte(bus, device, 0, MAX_LATENCY_OFFSET),
                };
                volatile auto* hba = reinterpret_cast<HBAMemory*>(
                    Memory::physical_to_virtual_address(ahci_header.bar_5));
                if (!ahci_driver.start(hba)) {
                    PCI_LOGGER->error("Failed to init AHCI");
                    while (true);
                }
            }

            constexpr U8 FUNC_LIMIT            = 8;
            constexpr U8 MULTI_FUNCTION_DEVICE = 0x80;
            if ((header.header_type & MULTI_FUNCTION_DEVICE) != 0) {
                for (U8 func = 1; func < FUNC_LIMIT; func++) {
                    header = read_header(bus, device, 0);
                    if (header.vendor_id == INVALID_VENDOR) continue;

                    PCI_LOGGER->debug(
                        "Bus: {}, Device: {}, Function: {} - {:#x}:{:#x} - Base Class "
                        "Code: {:#x} - Sub "
                        "Class: {:#x} - Programming Interface: {:#x}",
                        bus,
                        device,
                        func,
                        header.vendor_id,
                        header.device_id,
                        header.base_class_code,
                        header.sub_class_code,
                        header.programming_interface);
                }
            }
        }

        static void discover_devices(AHCIDriver& ahci_driver) {
            constexpr U8 BUS_LIMIT    = 255;
            constexpr U8 DEVICE_LIMIT = 32;
            for (U8 bus = 0; bus < BUS_LIMIT; bus++) {
                for (U8 device = 0; device < DEVICE_LIMIT; device++) {
                    check_device(ahci_driver, bus, device);
                }
            }
        }
    };
} // namespace Rune::Device

#endif // RUNEOS_PCI_H
