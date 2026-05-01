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
                        DeviceHandle                 bus_device,
                        const DeviceMapper&          device_mapper,
                        HandleCounter<DeviceHandle>& dev_handle_counter) -> bool;

      public:
        const static BasicDeviceID ID_PCI;

        PCIDriver(DriverHandle handle);

        [[nodiscard]] auto get_target_device_ID() const -> const DeviceID* override;
        auto               start(DeviceHandle dev_handle, void* context) -> bool override;
        auto               stop(DeviceHandle dev_handle) -> bool override;
        auto handle_request(DeviceHandle dev_handle, IORequest request) -> IORequestStatus override;
        void discover_devices(DeviceHandle                 bus_device,
                              const DeviceMapper&          device_mapper,
                              HandleCounter<DeviceHandle>& dev_handle_counter) override;
    };
} // namespace Rune::Device

#endif // RUNEOS_PCI_H
