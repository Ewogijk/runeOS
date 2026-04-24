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

#include <Device/Device.h>

#include <Device/AHCI/AHCI.h>

namespace Rune::Device {

    class PCIDeviceID : public DeviceID {
        U8 m_base_class_code;
        U8 m_subclass_code;
        U8 m_programming_interface;

      public:
        PCIDeviceID(U8 base_class_code, U8 subclass_code, U8 programming_interface);
        auto get_device_ID_type() -> DeviceIDType override;
        auto equals(const SharedPointer<DeviceID>& d_ID) -> bool override;
    };

    // ========================================================================================== //
    // PCI Access
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
        auto is_multi_function_device() -> bool;

        /// @brief Get the layout of the second part of the header.
        /// @return 0: Header type 0 - General Device
        ///         1: Header type 1 - PCI-to-PCI Bridge
        auto get_header_layout() -> U8;
    };

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

    auto pci_read_byte(U8 bus, U8 device, U8 func, U8 offset) -> U8;

    void pci_write_byte(U8 bus, U8 device, U8 func, U8 offset, U8 value);

    auto pci_read_word(U8 bus, U8 device, U8 func, U8 offset) -> U16;

    void pci_write_word(U8 bus, U8 device, U8 func, U8 offset, U16 value);

    auto pci_read_dword(U8 bus, U8 device, U8 func, U8 offset) -> U32;

    void pci_write_dword(U8 bus, U8 device, U8 func, U8 offset, U32 value);

    auto pci_read_configuration_space_header_common(U8 bus, U8 device, U8 func)
        -> PCIConfigurationSpaceHeaderCommon;

    auto
    pci_read_configuration_space_header_type0(const PCIConfigurationSpaceHeaderCommon& csh_common,
                                              U8                                       bus,
                                              U8                                       device,
                                              U8 func) -> PCIConfigurationSpaceHeaderType0;

    void pci_check_device(AHCIDriver* ahci_driver, U8 bus, U8 device);

    void pci_discover_devices(AHCIDriver* ahci_driver);

    // ========================================================================================== //
    // PCIDriver
    // ========================================================================================== //

    class PCIDriver : public Driver {

        /// @brief Try to map a PCI device to device driver.
        /// @param bus
        /// @param device
        /// @param func
        /// @param device_mapper
        /// @param dev_handle_counter
        /// @return True: The device is a multifunction device, False: Otherwise.
        auto map_device(U16                          bus,
                        U8                           device,
                        U8                           func,
                        const DeviceMapper&          device_mapper,
                        HandleCounter<DeviceHandle>& dev_handle_counter) -> bool;

      public:
        const static String PCI;

        PCIDriver(DriverHandle handle);

        auto get_target_device_ID() -> SharedPointer<DeviceID> override;
        auto start(void* context) -> bool override;
        auto stop() -> bool override;
        auto handle_request(IORequest request) -> IOResponse override;
        void discover_devices(const DeviceMapper&          device_mapper,
                              HandleCounter<DeviceHandle>& dev_handle_counter) override;
    };
} // namespace Rune::Device

#endif // RUNEOS_PCI_H
