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

#include <Device/PCI/VendorDB.h>

#include <KRE/Collections/HashMap.h>

namespace Rune::Device {

    auto operator==(const PCIVendorDBRequest& lhs, const PCIVendorDBRequest& rhs) -> bool {
        return lhs.m_vendor_ID == rhs.m_vendor_ID && lhs.m_device_ID == rhs.m_device_ID;
    }
    auto operator!=(const PCIVendorDBRequest& lhs, const PCIVendorDBRequest& rhs) -> bool {
        return !(lhs == rhs);
    }

    // NOLINTBEGIN cppcoreguidelines-avoid-non-const-global-variables: must be mutable
    HashMap<PCIVendorDBRequest, PCIVendorDBResponse> VENDOR_DATABASE;
    // NOLINTEND

    void pci_vendor_db_initialize() {
        // NOLINTBEGIN cppcoreguidelines-avoid-magic-numbers: not worth the work
        VENDOR_DATABASE.put({.m_vendor_ID = 0x8086, .m_device_ID = 0x1237},
                            {.m_vendor_name = "Intel Corporation",
                             .m_device_name = "440FX - 82441FX PMC [Natoma]"});
        VENDOR_DATABASE.put({.m_vendor_ID = 0x8086, .m_device_ID = 0x7000},
                            {.m_vendor_name = "Intel Corporation",
                             .m_device_name = "82371SB PIIX3 ISA [Natoma/Triton II]"});
        VENDOR_DATABASE.put({.m_vendor_ID = 0x8086, .m_device_ID = 0x7010},
                            {.m_vendor_name = "Intel Corporation",
                             .m_device_name = "82371SB PIIX3 IDE [Natoma/Triton II]"});
        VENDOR_DATABASE.put(
            {.m_vendor_ID = 0x8086, .m_device_ID = 0x7113},
            {.m_vendor_name = "Intel Corporation", .m_device_name = "82371AB/EB/MB PIIX4 ACPI"});
        VENDOR_DATABASE.put(
            {.m_vendor_ID = 0x8086, .m_device_ID = 0x2922},
            {.m_vendor_name = "Intel Corporation",
             .m_device_name = "82801IR/IO/IH (ICH9R/DO/DH) 6 port SATA Controller [AHCI mode]"});
        VENDOR_DATABASE.put(
            {.m_vendor_ID = 0x1B36, .m_device_ID = 0x000D},
            {.m_vendor_name = "Red Hat, Inc.", .m_device_name = "QEMU XHCI Host Controller"});
        // NOLINTEND
    }

    auto pci_vendor_db_resolve(PCIVendorDBRequest vendor_db_request) -> PCIVendorDBResponse {
        auto maybe_vendor_db_response = VENDOR_DATABASE.find(vendor_db_request);
        if (maybe_vendor_db_response == VENDOR_DATABASE.end())
            return {.m_vendor_name = "Unknown", .m_device_name = "Unknown"};
        return *maybe_vendor_db_response->value;
    }
} // namespace Rune::Device