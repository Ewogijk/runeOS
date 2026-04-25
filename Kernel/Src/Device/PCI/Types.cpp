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

namespace Rune::Device {
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
        auto* pci_d_ID = static_cast<const PCIDeviceID*>(d_ID);
        return m_base_class_code == pci_d_ID->m_base_class_code
               && m_subclass_code == pci_d_ID->m_subclass_code
               && m_programming_interface == pci_d_ID->m_programming_interface;
    }

    // ========================================================================================== //
    // PCIDevice
    // ========================================================================================== //

    PCIDevice::PCIDevice(DeviceHandle       handle,
                         const String&      name,
                         const String&      oem,
                         U32                revision,
                         const PCIDeviceID& device_ID)
        : Device(handle, name, oem, revision),
          m_device_ID(device_ID) {}

    auto PCIDevice::get_device_ID() const -> const DeviceID* { return &m_device_ID; }

    // ========================================================================================== //
    // PCI Header
    // ========================================================================================== //

    auto PCIConfigurationSpaceHeaderCommon::is_multi_function_device() -> bool {
        return (header_type & MASK_MULTI_FUNCTION_DEVICE) != 0;
    }

    auto PCIConfigurationSpaceHeaderCommon::get_header_layout() -> U8 {
        return header_type & MASK_HEADER_LAYOUT;
    }
} // namespace Rune::Device