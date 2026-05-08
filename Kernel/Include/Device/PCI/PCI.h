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

#include <Device/Device.h>

#include <Device/PCI/Types.h>

namespace Rune::Device {
    // ========================================================================================== //
    // PCI Access
    // ========================================================================================== //

    /// @brief Read a byte from the configuration space.
    /// @param bus Bus.
    /// @param device Device.
    /// @param func Function.
    /// @param offset Offset.
    /// @return A byte read from the configuration space.
    auto pci_read_byte(U8 bus, U8 device, U8 func, U8 offset) -> U8;

    /// @brief Write a byte to the configuration space.
    /// @param bus Bus.
    /// @param device Device.
    /// @param func Function.
    /// @param offset Offset.
    void pci_write_byte(U8 bus, U8 device, U8 func, U8 offset, U8 value);

    /// @brief Read a word from the configuration space.
    /// @param bus Bus.
    /// @param device Device.
    /// @param func Function.
    /// @param offset Offset.
    /// @return A word read from the configuration space.
    auto pci_read_word(U8 bus, U8 device, U8 func, U8 offset) -> U16;

    /// @brief Write a word to the configuration space.
    /// @param bus Bus.
    /// @param device Device.
    /// @param func Function.
    /// @param offset Offset.
    void pci_write_word(U8 bus, U8 device, U8 func, U8 offset, U16 value);

    /// @brief Read a dword from the configuration space.
    /// @param bus Bus.
    /// @param device Device.
    /// @param func Function.
    /// @param offset Offset.
    /// @return A byte dword from the configuration space.
    auto pci_read_dword(U8 bus, U8 device, U8 func, U8 offset) -> U32;

    /// @brief Write a dword to the configuration space.
    /// @param bus Bus.
    /// @param device Device.
    /// @param func Function.
    /// @param offset Offset.
    void pci_write_dword(U8 bus, U8 device, U8 func, U8 offset, U32 value);

    /// @brief Read a common configuration space header.
    /// @param bus Bus
    /// @param device Device
    /// @param func Function
    /// @return A common configuration space header.
    auto pci_read_configuration_space_header_common(U8 bus, U8 device, U8 func)
        -> PCIConfigurationSpaceHeaderCommon;

    /// @brief Read a Type 0 configuration space header.
    /// @param csh_common Common PCI configuration space header.
    /// @param bus Bus.
    /// @param device Device.
    /// @param func Function.
    /// @return A common configuration space header.
    ///
    /// The given csh_common will be prepended to the Type 0 header-specific part.
    auto
    pci_read_configuration_space_header_type0(const PCIConfigurationSpaceHeaderCommon& csh_common,
                                              U8                                       bus,
                                              U8                                       device,
                                              U8 func) -> PCIConfigurationSpaceHeaderType0;

    // // Deprecated
    // void pci_check_device(AHCIDriver* ahci_driver, U8 bus, U8 device);
    //
    // // Deprecated
    // void pci_discover_devices(AHCIDriver* ahci_driver);

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
                        const SharedPointer<Device>& bus_device) -> bool;

      public:
        const static BasicDeviceID ID_PCI;

        PCIDriver() = default;

        [[nodiscard]] auto vendor() const -> String override;
        [[nodiscard]] auto version() const -> Version override;
        [[nodiscard]] auto target_device_ID() const -> const DeviceID* override;
        auto accept_device(const SharedPointer<Device>& device) -> bool override;
        void remove_device(const SharedPointer<Device>& device) override;
        auto handle_request(const SharedPointer<Device>& device, IORequest request)
            -> IORequestStatus override;
    };
} // namespace Rune::Device

#endif // RUNEOS_PCI_H
