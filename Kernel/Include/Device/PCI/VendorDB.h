
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

#ifndef RUNEOS_VENDORLOOKUP_H
#define RUNEOS_VENDORLOOKUP_H

#include <Ember/Ember.h>

#include <KRE/String.h>

namespace Rune::Device {
    /// @brief A pair of vendor ID and device ID.
    struct PCIVendorDBRequest {
        U16 m_vendor_ID;
        U16 m_device_ID;

        friend auto operator==(const PCIVendorDBRequest& lhs, const PCIVendorDBRequest& rhs)
            -> bool;
        friend auto operator!=(const PCIVendorDBRequest& lhs, const PCIVendorDBRequest& rhs) -> bool;
    };

    /// @brief A pair of vendor name and device name.
    struct PCIVendorDBResponse {
        String m_vendor_name;
        String m_device_name;
    };

    /// @brief Setup the vendor database information.
    void pci_vendor_db_initialize();

    /// @brief Try to resolve the given vendor ID and device ID to human-readable vendor and device
    ///         names.
    /// @param vendor_db_request Vendor ID and device ID that should be resolved.
    /// @return A vendor db response containing a vendor name and device name.
    ///
    /// If the given vendor ID and device ID are unknown, the response will contain "Unknown" for
    /// both vendor name and device name.
    auto pci_vendor_db_resolve(PCIVendorDBRequest vendor_db_request) -> PCIVendorDBResponse;
} // namespace Rune::Device

namespace Rune {
    template <>
    struct Hash<Device::PCIVendorDBRequest> {
        auto operator()(const Device::PCIVendorDBRequest& key) const -> size_t {
            U16 arr[2] = {key.m_vendor_ID, key.m_device_ID};
            return FNV::do_hash<U16>(arr, 2);
        }
    };
} // namespace Rune

#endif // RUNEOS_VENDORLOOKUP_H
