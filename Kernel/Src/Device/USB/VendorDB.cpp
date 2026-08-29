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

#include <Device/USB/VendorDB.h>

#include <KRE/Collections/HashMap.h>

namespace Rune::Device::USB {
    auto operator==(const VendorDBRequest& lhs, const VendorDBRequest& rhs) -> bool {
        return lhs.m_vendor_ID == rhs.m_vendor_ID && lhs.m_product_ID == rhs.m_product_ID;
    }
    auto operator!=(const VendorDBRequest& lhs, const VendorDBRequest& rhs) -> bool {
        return !(lhs == rhs);
    }

    // NOLINTBEGIN cppcoreguidelines-avoid-non-const-global-variables: must be mutable
    HashMap<VendorDBRequest, VendorDBResponse> VENDOR_DATABASE;
    // NOLINTEND

    void vendor_db_initialize() {
        // NOLINTBEGIN cppcoreguidelines-avoid-magic-numbers: not worth the work
        VENDOR_DATABASE.put({.m_vendor_ID = 0x46F4, .m_product_ID = 0x0001},
                            {.m_vendor_name = "QEMU", .m_product_name = "Unknown"});
        VENDOR_DATABASE.put({.m_vendor_ID = 0x46F4, .m_product_ID = 0x0002},
                            {.m_vendor_name = "QEMU", .m_product_name = "Unknown"});
        VENDOR_DATABASE.put(
            {.m_vendor_ID = 0x0627, .m_product_ID = 0x0001},
            {.m_vendor_name = "Adomax Technology Co., Ltd.", .m_product_name = "QEMU Tablet"});
        // NOLINTEND
    }

    auto vendor_db_resolve(VendorDBRequest vendor_db_request) -> VendorDBResponse {
        auto maybe_vendor_db_response = VENDOR_DATABASE.find(vendor_db_request);
        if (maybe_vendor_db_response == VENDOR_DATABASE.end())
            return {.m_vendor_name = "Unknown", .m_product_name = "Unknown"};
        return *maybe_vendor_db_response->value;
    }
} // namespace Rune::Device::USB