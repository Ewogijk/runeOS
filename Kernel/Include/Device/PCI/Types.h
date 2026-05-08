
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

#ifndef RUNEOS_PCIDEVICE_H
#define RUNEOS_PCIDEVICE_H

#include <Device/Device.h>

namespace Rune::Device {
    // ========================================================================================== //
    // Configuration Space
    // ========================================================================================== //

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

    /// @brief Common part of all PCI configuration space headers.
    struct PCIConfigurationSpaceHeaderCommon {
        static constexpr U8 MASK_HEADER_LAYOUT         = 0x7F;
        static constexpr U8 MASK_MULTI_FUNCTION_DEVICE = 0x80;

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

        /// @brief
        /// @return True: The device may contain more than one function.
        ///         False: Otherwise.
        [[nodiscard]] auto is_multi_function_device() const -> bool;

        /// @brief Get the layout of the second part of the header.
        /// @return 0: Header type 0 - General Device
        ///         1: Header type 1 - PCI-to-PCI Bridge
        [[nodiscard]] auto get_header_layout() const -> U8;
    };

    /// @brief Type 0 configuration space header of generic devices.
    struct PCIConfigurationSpaceHeaderType0 {
        PCIConfigurationSpaceHeaderCommon header;
        U32                               bar_0{};
        U32                               bar_1{};
        U32                               bar_2{};
        U32                               bar_3{};
        U32                               bar_4{};
        U32                               bar_5{};
        U32                               card_bus_cis_pointer{};
        U16                               subsystem_vendor_id{};
        U16                               subsystem_id{};
        U32                               expansion_rom_address_base{};
        U8                                capabilities_pointer{};
        U8                                reserved_0{};
        U16                               reserved_1{};
        U32                               reserved_2{};
        U8                                interrupt_line{};
        U8                                interrupt_pin{};
        U8                                min_grant{};
        U8                                max_latency{};
    };

    // ========================================================================================== //
    // PCIDeviceID
    // ========================================================================================== //
    /// @brief PCI-specific device ID containing base class code, subclass code and programming
    ///         interface.
    class PCIDeviceID : public DeviceID {
        U8 m_base_class_code;
        U8 m_subclass_code;
        U8 m_programming_interface;

      public:
        PCIDeviceID(U8 base_class_code, U8 subclass_code, U8 programming_interface);
        [[nodiscard]] auto get_device_ID_type() const -> DeviceIDType override;
        auto               equals(const DeviceID* d_ID) const -> bool override;
    };

    // ========================================================================================== //
    // PCIDevice
    // ========================================================================================== //

    class PCIDevice : public Device {
        PCIDeviceID                      m_device_ID;
        PCIConfigurationSpaceHeaderType0 m_pci_header;

      public:
        PCIDevice(Handle                                  handle,
                  const String&                           name,
                  const String&                           oem,
                  const String&                           revision,
                  const String&                           serial_number,
                  DeviceType                              device_type,
                  PCIDeviceID                             device_ID,
                  const PCIConfigurationSpaceHeaderType0& pci_header);

        [[nodiscard]] auto device_ID() const -> const DeviceID* override;

        /// @brief
        /// @return The PCI configuration space header type 0.
        [[nodiscard]] auto pci_header() const -> const PCIConfigurationSpaceHeaderType0&;
    };
} // namespace Rune::Device

#endif // RUNEOS_PCIDEVICE_H
