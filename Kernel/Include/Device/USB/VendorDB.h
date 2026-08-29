
//  Copyright 2026 Ewogijk
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#ifndef RUNEOS_VENDORDB_H
#define RUNEOS_VENDORDB_H

#include <Ember/Ember.h>

#include <KRE/String.h>

namespace Rune::Device::USB {
    /// @brief A pair of vendor ID and device ID.
    struct VendorDBRequest {
        U16 m_vendor_ID;
        U16 m_product_ID;

        friend auto operator==(const VendorDBRequest& lhs, const VendorDBRequest& rhs) -> bool;
        friend auto operator!=(const VendorDBRequest& lhs, const VendorDBRequest& rhs) -> bool;
    };

    /// @brief A pair of vendor name and device name.
    struct VendorDBResponse {
        String m_vendor_name;
        String m_product_name;
    };

    /// @brief Set up the vendor database information.
    void vendor_db_initialize();

    /// @brief Try to resolve the given vendor ID and device ID to human-readable vendor and device
    ///         names.
    /// @param vendor_db_request Vendor ID and device ID that should be resolved.
    /// @return A vendor db response containing a vendor name and device name.
    ///
    /// If the given vendor ID and device ID are unknown, the response will contain "Unknown" for
    /// both vendor name and device name.
    auto vendor_db_resolve(VendorDBRequest vendor_db_request) -> VendorDBResponse;
} // namespace Rune::Device::USB

namespace Rune {
    template <>
    struct Hash<Device::USB::VendorDBRequest> {
        auto operator()(const Device::USB::VendorDBRequest& key) const -> size_t {
            U16 arr[2] = {key.m_vendor_ID, key.m_product_ID}; // NOLINT
            return FNV::do_hash<U16>(arr, 2);
        }
    };
} // namespace Rune

#endif // RUNEOS_VENDORDB_H
