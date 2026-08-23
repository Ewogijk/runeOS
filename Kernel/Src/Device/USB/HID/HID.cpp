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

#include <../../../Include/Device/USB/HID/HID.h>

#include <KRE/BitsAndBytes.h>
#include <KRE/Logging.h>

namespace Rune::Device::USB {
    // ========================================================================================== //
    // Descriptors — HID 1.11 §6
    // ========================================================================================== //

    DEFINE_TYPED_ENUM(HIDDescriptorType, U8, HID_DESCRIPTOR_TYPES, 0x00)
    DEFINE_TYPED_ENUM(HIDCountryCode, U8, HID_COUNTRY_CODES, 0xFF)
    DEFINE_TYPED_ENUM(HIDItemType, U8, HID_ITEM_TYPES, 0xFF)
    DEFINE_TYPED_ENUM(HIDItemTag, U8, HID_ITEM_TAGS, 0x00)
    DEFINE_TYPED_ENUM(HIDDataFlag, U32, HID_DATA_FLAGS, 0x0)
    DEFINE_TYPED_ENUM(HIDCollectionType, U8, HID_COLLECTION_TYPES, 0xFF)
    DEFINE_TYPED_ENUM(HIDPhysicalBias, U8, HID_PHYSICAL_BIASES, 0)
    DEFINE_TYPED_ENUM(HIDPhysicalQualifier, U8, HID_PHYSICAL_QUALIFIERS, 0)
    DEFINE_TYPED_ENUM(HIDPhysicalDesignator, U8, HID_PHYSICAL_DESIGNATORS, 0x00)

    // ========================================================================================== //
    // Class-Specific Requests — HID 1.11 §7.2
    // ========================================================================================== //

    DEFINE_TYPED_ENUM(HIDClassRequest, U8, HID_CLASS_REQUESTS, 0x00)
    DEFINE_TYPED_ENUM(HIDReportType, U8, HID_REPORT_TYPES, 0x00)
    DEFINE_TYPED_ENUM(HIDProtocolMode, U8, HID_PROTOCOL_MODES, 0xFF)

    auto hid_build_set_protocol_request(const SharedPointer<FunctionDevice>& hid_device,
                                        U16                                  interface_number,
                                        HIDProtocolMode protocol_mode) -> ControlTransferRequest {
        return USB::ControlTransferRequest::of(
            hid_device->get_handle(),
            USB::RequestType::DIRECTION_HOST_TO_DEVICE | USB::RequestType::TYPE_CLASS
                | USB::RequestType::RECIPIENT_INTERFACE,
            USB::HIDClassRequest::SET_PROTOCOL,
            protocol_mode,
            interface_number,
            0,
            nullptr);
    }

    auto hid_build_set_idle_request(const SharedPointer<FunctionDevice>& hid_device,
                                    U16                                  interface_number,
                                    U8                                   report_ID,
                                    U16 duration_ms) -> ControlTransferRequest {
        constexpr U8  HID_IDLE_DURATION_UNIT_MS = 4;
        constexpr U16 HID_IDLE_DURATION_MAX_MS  = 1020;
        U16           clamped =
            duration_ms > HID_IDLE_DURATION_MAX_MS ? HID_IDLE_DURATION_MAX_MS : duration_ms;
        auto                        duration = static_cast<U8>(clamped / HID_IDLE_DURATION_UNIT_MS);
        return USB::ControlTransferRequest::of(
            hid_device->get_handle(),
            USB::RequestType::DIRECTION_HOST_TO_DEVICE | USB::RequestType::TYPE_CLASS
                | USB::RequestType::RECIPIENT_INTERFACE,
            USB::HIDClassRequest::SET_IDLE,
            static_cast<U16>(static_cast<U16>(duration) << SHIFT_8 | report_ID),
            interface_number,
            0,
            nullptr);
    }

    // ========================================================================================== //
    // HID Item Model
    // ========================================================================================== //

    // ====================================================================================== //
    // HIDDataFlags
    // ====================================================================================== //

    auto HIDDataFlags::has(HIDDataFlag flag) const -> bool {
        return (m_flags & flag.to_value()) != 0;
    }

    auto HIDDataFlags::decode_flags() const -> String {
        constexpr U8 DATA_FLAG_COUNT = 9;
        String       flags;
        for (size_t i = 0; i < DATA_FLAG_COUNT; i++) {
            if (bit_check(m_flags, i)) {
                if (!flags.is_empty() && i != 0) flags += "|";
                U16 bla  = bit_set(0, i);
                flags   += HIDDataFlag(bla).to_string();
            }
        }
        return flags;
    }

    // ====================================================================================== //
    // HIDReportTag
    // ====================================================================================== //

    constexpr auto HIDReportTag::as_w_value() const -> U16 {
        return static_cast<U16>(m_type.to_value() << SHIFT_8 | m_report_ID);
    }

    auto operator==(const HIDReportTag& lhs, const HIDReportTag& rhs) -> bool {
        return lhs.m_type == rhs.m_type && lhs.m_report_ID == rhs.m_report_ID;
    }

    auto operator!=(const HIDReportTag& lhs, const HIDReportTag& rhs) -> bool {
        return !(lhs == rhs);
    }

    // ====================================================================================== //
    // HIDExtendedUsage
    // ====================================================================================== //

    auto HIDExtendedUsage::usage_page() const -> U16 { return word_get(m_extended_usage, 1); };

    auto HIDExtendedUsage::usage() const -> U16 { return word_get(m_extended_usage, 0); }

    auto HIDExtendedUsage::decode_usage_page() const -> HIDUsagePage {
        return HIDUsagePage(word_get(m_extended_usage, 1));
    }

    auto HIDExtendedUsage::from(U16 usage_page, U16 usage) -> HIDExtendedUsage {
        return {.m_extended_usage = static_cast<U32>(usage_page << SHIFT_16 | usage)};
    }

    // ====================================================================================== //
    // HIDUsageRange
    // ====================================================================================== //

    auto HIDExtendedUsageRange::count() const -> U32 { return m_max.usage() - m_min.usage() + 1; }

    // ====================================================================================== //
    // HIDCollectionPathEntry
    // ====================================================================================== //

    auto HIDCollectionPathEntry::type() const -> HIDCollectionType {
        return HIDCollectionType(m_type);
    }

    // ====================================================================================== //
    // HIDData
    // ====================================================================================== //

    auto to_unit_symbol(U8 unit_system, U8 unit, U8 exponent) -> String {
        String exp_str = exponent == 0 ? "" : String::format("^{}", exponent);
        // NOLINTBEGIN
        switch (unit_system) {
            case 1: {
                switch (unit) {
                    case 1:  return "cm" + exp_str;
                    case 2:  return "g" + exp_str;
                    case 3:  return "s" + exp_str;
                    case 4:  return "K" + exp_str;
                    case 5:  return "A" + exp_str;
                    case 6:  return "cd" + exp_str;
                    default: return HIDData::UNIT_NONE; ;
                }
            }
            case 2: {
                switch (unit) {
                    case 1:  return "rad" + exp_str;
                    case 2:  return "g" + exp_str;
                    case 3:  return "s" + exp_str;
                    case 4:  return "K" + exp_str;
                    case 5:  return "A" + exp_str;
                    case 6:  return "cd" + exp_str;
                    default: return HIDData::UNIT_NONE; ;
                }
            }
            case 3: {
                switch (unit) {
                    case 1:  return "in" + exp_str;
                    case 2:  return "slug" + exp_str;
                    case 3:  return "s" + exp_str;
                    case 4:  return "F" + exp_str;
                    case 5:  return "A" + exp_str;
                    case 6:  return "cd" + exp_str;
                    default: return HIDData::UNIT_NONE; ;
                }
            }
            case 4: {
                switch (unit) {
                    case 1:  return "deg" + exp_str;
                    case 2:  return "slug" + exp_str;
                    case 3:  return "s" + exp_str;
                    case 4:  return "F" + exp_str;
                    case 5:  return "A" + exp_str;
                    case 6:  return "cd" + exp_str;
                    default: return HIDData::UNIT_NONE; ;
                }
            }
            default: return HIDData::UNIT_NONE;
        }
        // NOLINTEND
    }

    auto HIDData::is_padding() const -> bool { return m_usage_ranges.empty(); }

    auto HIDData::unit_as_string() const -> String {
        U8 unit_system = nibble_get(m_unit, 0);
        if (unit_system == 0) return UNIT_NONE;
        String unit_str;
        for (U8 i = 1; i < BIT_COUNT_BYTE; i++) {
            U8     unit_exponent = nibble_get(m_unit, i);
            String unit_symbol   = to_unit_symbol(unit_system, i, unit_exponent);
            if (unit_symbol != UNIT_NONE) {
                unit_str += String::format("{}{}", i > 1 ? "*" : "", unit_symbol);
            }
        }
        return unit_str;
    }
} // namespace Rune::Device::USB