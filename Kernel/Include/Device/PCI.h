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
        U32       bar_0;
        U32       bar_1;
        U32       bar_2;
        U32       bar_3;
        U32       bar_4;
        U32       bar_5;
        U32       card_bus_cis_pointer;
        U16       subsystem_vendor_id;
        U16       subsystem_id;
        U32       expansion_rom_address_base;
        U8        capabilities_pointer;
        U8        reserved_0;
        U16       reserved_1;
        U32       reserved_2;
        U8        interrupt_line;
        U8        interrupt_pin;
        U8        min_grant;
        U8        max_latency;
    };

    class PCI {
        static constexpr U16 PCI_CONFIG = 0xCF8;
        static constexpr U16 PCI_DATA   = 0xCFC;

      public:
        static auto read_byte(U8 bus, U8 device, U8 func, U8 offset) -> U8 {
            CPU::out_dw(PCI_CONFIG,
                        (1 << 31 | (bus << 16) | (device << 11) | (func << 8) | (offset & 0xFC)));
            return CPU::in_b(PCI_DATA + (offset & 0x03));
        }

        static void write_byte(U8 bus, U8 device, U8 func, U8 offset, U8 value) {
            CPU::out_dw(PCI_CONFIG,
                        (1 << 31 | (bus << 16) | (device << 11) | (func << 8) | (offset & 0xFC)));
            CPU::out_b(PCI_DATA + (offset & 0x03), value);
        }

        static auto read_word(U8 bus, U8 device, U8 func, U8 offset) -> U16 {
            if ((offset & 0x03) > 2) {
                return (read_byte(bus, device, func, offset + 1) << 8)
                       | read_byte(bus, device, func, offset);
            }
            CPU::out_dw(PCI_CONFIG,
                        (1 << 31 | (bus << 16) | (device << 11) | (func << 8) | (offset & 0xFC)));
            return CPU::in_w(PCI_DATA + (offset & 0x03));
        }

        static void write_word(U8 bus, U8 device, U8 func, U8 offset, U16 value) {
            if ((offset & 0x03) > 2) {
                write_byte(bus, device, func, offset, (U8) value);
                write_byte(bus, device, func, offset + 1, (U8) value >> 8);
            }
            CPU::out_dw(PCI_CONFIG,
                        (1 << 31 | (bus << 16) | (device << 11) | (func << 8) | (offset & 0xFC)));
            CPU::out_w(PCI_DATA + (offset & 0x03), value);
        }

        static auto read_dword(U8 bus, U8 device, U8 func, U8 offset) -> U32 {
            if ((offset & 0x03) > 0) {
                return (read_word(bus, device, func, offset + 2) << 16)
                       | read_word(bus, device, func, offset);
            }
            CPU::out_dw(PCI_CONFIG,
                        (1 << 31 | (bus << 16) | (device << 11) | (func << 8) | (offset & 0xFC)));
            return CPU::in_dw(PCI_DATA + (offset & 0x03));
        }

        static void write_dword(U8 bus, U8 device, U8 func, U8 offset, U32 value) {
            if ((offset & 0x03) > 2) {
                write_word(bus, device, func, offset, (uint16_t) value);
                write_word(bus, device, func, offset + 2, (uint16_t) value >> 16);
            }
            CPU::out_dw(PCI_CONFIG,
                        (1 << 31 | (bus << 16) | (device << 11) | (func << 8) | (offset & 0xFC)));
            CPU::out_dw(PCI_DATA + (offset & 0x03), value);
        }

        static auto read_header(U8 bus, U8 device, U8 func) -> PCIHeader {
            return {
                read_word(bus, device, func, 0x00),
                read_word(bus, device, func, 0x02),
                read_word(bus, device, func, 0x04),
                read_word(bus, device, func, 0x06),
                read_byte(bus, device, func, 0x08),
                read_byte(bus, device, func, 0x09),
                read_byte(bus, device, func, 0x0A),
                read_byte(bus, device, func, 0x0B),
                read_byte(bus, device, func, 0x0C),
                read_byte(bus, device, func, 0x0D),
                read_byte(bus, device, func, 0x0E),
                read_byte(bus, device, func, 0x0F),
            };
        }

        static void check_device(AHCIDriver& ahci_driver, U8 bus, U8 device) {
            PCIHeader header = read_header(bus, device, 0);
            if (header.vendor_id == 0xFFFF) return;

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

            if (header.base_class_code == 0x1 && header.sub_class_code == 0x6
                && header.vendor_id == 0x8086 && header.device_id == 0x2922) {
                write_word(bus, device, 0, 0x04, header.command.AsUInt16 | 0x4); // Enable DMA
                PCIHeaderType0 ahci_header = {
                    read_header(bus, device, 0),
                    read_dword(bus, device, 0, 0x10),
                    read_dword(bus, device, 0, 0x14),
                    read_dword(bus, device, 0, 0x18),
                    read_dword(bus, device, 0, 0x1C),
                    read_dword(bus, device, 0, 0x20),
                    read_dword(bus, device, 0, 0x24),
                    read_dword(bus, device, 0, 0x28),
                    read_word(bus, device, 0, 0x2C),
                    read_word(bus, device, 0, 0x2E),
                    read_dword(bus, device, 0, 0x30),
                    read_byte(bus, device, 0, 0x34),
                    0,
                    0,
                    0,
                    read_byte(bus, device, 0, 0x3C),
                    read_byte(bus, device, 0, 0x3D),
                    read_byte(bus, device, 0, 0x3E),
                    read_byte(bus, device, 0, 0x3F),
                };
                volatile auto* hba = reinterpret_cast<HBAMemory*>(
                    Memory::physical_to_virtual_address(ahci_header.bar_5));
                if (!ahci_driver.start(hba)) {
                    PCI_LOGGER->error("Failed to init AHCI");
                    while (true);
                }
            }

            if ((header.header_type & 0x80) != 0) {
                for (U8 func = 1; func < 8; func++) {
                    header = read_header(bus, device, 0);
                    if (header.vendor_id == 0xFFFF) continue;

                    PCI_LOGGER->debug("Bus: {}, Device: {}, Function: {} - {:#x}:{:#x} - Base Class "
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
            for (int bus = 0; bus < 255; bus++) {
                for (int device = 0; device < 32; device++) {
                    check_device(ahci_driver, bus, device);
                }
            }
        }
    };
} // namespace Rune::Device

#endif // RUNEOS_PCI_H
