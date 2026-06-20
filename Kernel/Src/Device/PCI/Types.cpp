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

#include <Device/PCI/Types.h>

#include <KRE/BitsAndBytes.h>

namespace Rune::Device {
    // ========================================================================================== //
    // PCI Header
    // ========================================================================================== //

    auto PCIConfigurationSpaceHeaderCommon::is_multi_function_device() const -> bool {
        return (header_type & MASK_MULTI_FUNCTION_DEVICE) != 0;
    }

    auto PCIConfigurationSpaceHeaderCommon::get_header_layout() const -> U8 {
        return header_type & MASK_HEADER_LAYOUT;
    }

    auto PCIConfigurationSpaceHeaderType0::is_64bit_bar(size_t bar_idx) const -> bool {
        switch (bar_idx) {
            case 0:  return (bar_0 & MASK_64BIT_ENCODING) >> 1 == BASE_REGISTER_64BIT;
            case 1:  return (bar_1 & MASK_64BIT_ENCODING) >> 1 == BASE_REGISTER_64BIT;
            case 2:  return (bar_2 & MASK_64BIT_ENCODING) >> 1 == BASE_REGISTER_64BIT;
            case 3:  return (bar_3 & MASK_64BIT_ENCODING) >> 1 == BASE_REGISTER_64BIT;
            case 4:  return (bar_4 & MASK_64BIT_ENCODING) >> 1 == BASE_REGISTER_64BIT;
            default: return false;
        }
    }

    auto PCIConfigurationSpaceHeaderType0::is_prefetchable_bar(size_t bar_idx) const -> bool {
        switch (bar_idx) {
            case 0:  return bit_check(bar_0, 4);
            case 1:  return bit_check(bar_1, 4);
            case 2:  return bit_check(bar_2, 4);
            case 3:  return bit_check(bar_3, 4);
            case 4:  return bit_check(bar_4, 4);
            default: return false;
        }
    }

    // ========================================================================================== //
    // PCIDeviceID
    // ========================================================================================== //

    PCIDeviceID::PCIDeviceID(U8 base_class_code, U8 subclass_code, U8 programming_interface)
        : m_base_class_code(base_class_code),
          m_subclass_code(subclass_code),
          m_programming_interface(programming_interface) {}

    auto PCIDeviceID::get_device_ID_type() const -> DeviceIDType { return DeviceIDType::PCI; }

    auto PCIDeviceID::equals(const DeviceID* d_ID) const -> bool {
        if (d_ID->get_device_ID_type() != DeviceIDType::PCI) return false;
        const auto* pci_d_ID = static_cast<const PCIDeviceID*>(d_ID);
        return m_base_class_code == pci_d_ID->m_base_class_code
               && m_subclass_code == pci_d_ID->m_subclass_code
               && m_programming_interface == pci_d_ID->m_programming_interface;
    }

    // ========================================================================================== //
    // PCIDevice
    // ========================================================================================== //

    PCIDevice::PCIDevice(Handle                                  handle,
                         const String&                           name,
                         const String&                           oem,
                         const String&                           revision,
                         const String&                           serial_number,
                         DeviceType                              device_type,
                         PCIDeviceID                             device_ID,
                         PCIConfigurationSpaceID                 config_space_ID,
                         const PCIConfigurationSpaceHeaderType0& pci_header)
        : Device(handle, name, oem, revision, serial_number, device_type),
          m_device_ID(move(device_ID)),
          m_config_space_ID(config_space_ID),
          m_pci_header(pci_header) {}

    auto PCIDevice::device_ID() const -> const DeviceID* { return &m_device_ID; }

    auto PCIDevice::config_space_ID() const -> const PCIConfigurationSpaceID& {
        return m_config_space_ID;
    }

    auto PCIDevice::pci_header() const -> const PCIConfigurationSpaceHeaderType0& {
        return m_pci_header;
    }

} // namespace Rune::Device