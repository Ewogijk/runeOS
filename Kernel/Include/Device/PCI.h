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
        static auto read_byte(U8 bus, U8 device, U8 func, U8 offset) -> U8;

        static void write_byte(U8 bus, U8 device, U8 func, U8 offset, U8 value);

        static auto read_word(U8 bus, U8 device, U8 func, U8 offset) -> U16;

        static void write_word(U8 bus, U8 device, U8 func, U8 offset, U16 value);

        static auto read_dword(U8 bus, U8 device, U8 func, U8 offset) -> U32;

        static void write_dword(U8 bus, U8 device, U8 func, U8 offset, U32 value);

        static auto read_header(U8 bus, U8 device, U8 func) -> PCIHeader;

        static void check_device(AHCIDriver& ahci_driver, U8 bus, U8 device);

        static void discover_devices(AHCIDriver& ahci_driver);
    };
} // namespace Rune::Device

#endif // RUNEOS_PCI_H
