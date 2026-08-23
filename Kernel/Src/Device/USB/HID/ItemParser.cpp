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

#include <Device/USB/HID/ItemParser.h>

#include <KRE/Logging.h>

namespace Rune::Device::USB {
    auto HIDItemParser::ShortItem::value() const -> U32 {
        return LittleEndian::to_U32(m_data.data());
    }

    auto HIDItemParser::ShortItem::signed_value() const -> S32 {
        U8  data_size = m_prefix.data_size();
        U32 val       = value();
        if (data_size == 0 || data_size == 4) return static_cast<S32>(val);

        U32 sign_bit = 1U << ((data_size * BIT_COUNT_BYTE) - 1);
        return static_cast<S32>((val ^ sign_bit) - sign_bit);
    }

    auto HIDItemParser::LocalItem::extended_usage() const -> HIDExtendedUsage {
        constexpr U8 EXTENDED_USAGE_SIZE = 4;
        if (m_item.m_prefix.data_size() == EXTENDED_USAGE_SIZE)
            return {.m_extended_usage = m_item.value()};
        return HIDExtendedUsage::from(m_usage_page, static_cast<U16>(word_get(m_item.value(), 0)));
    }

    HIDItemParser::HIDItemParser(const U8* descriptor, size_t length)
        : m_descriptor(descriptor),
          m_length(length) {}

    auto HIDItemParser::skip_long_items() -> bool {
        if (m_cursor >= m_length) return false;
        // HID 1.11 defines no long item tags, so a long item can only be skipped. Its
        // size is 3 + bDataSize: prefix, bDataSize, bLongItemTag and the data bytes.
        const auto* prefix = reinterpret_cast<const USB::HIDItemPrefix*>(m_descriptor + m_cursor);
        while (prefix->is_long_item()) {
            if (m_cursor + 1 >= m_length) {
                WARN("Offset {}: Truncated long item", m_cursor);
                return false;
            }
            m_cursor += 3 + m_descriptor[m_cursor + 1];
            if (m_cursor >= m_length) {
                WARN("Long item size exceeds buffer")
                break;
            }
            prefix = reinterpret_cast<const USB::HIDItemPrefix*>(m_descriptor + m_cursor);
        }
        return m_cursor < m_length;
    }

    auto HIDItemParser::build_short_item() -> Optional<ShortItem> {
        if (!skip_long_items()) return {};
        const auto* prefix = reinterpret_cast<const USB::HIDItemPrefix*>(m_descriptor + m_cursor);
        U8          data_size = prefix->data_size();

        if (m_cursor + prefix->size() > m_length) {
            WARN("Offset {}: Truncated short item - Wants: {}, Bytes left: {}",
                 m_cursor,
                 prefix->size(),
                 m_length - m_cursor);
            return {};
        }
        Array<U8, 4> data{};
        memcpy(data.data(), m_descriptor + m_cursor + 1, data_size);

#if LOG_DEBUG_ENABLED
        String data_str("0x");
        for (U8 i = 0; i < data_size; i++) {
            data_str += String::format("{:0>2x}", data[i]);
        }
        DEBUG("{}({}, {}, {:0=#8b}, {})",
              prefix->type().to_string(),
              prefix->item_tag().to_string(),
              prefix->size(),
              prefix->m_prefix,
              data_str);
#endif

        return {
            ShortItem{.m_prefix = *prefix, .m_data = data}
        };
    }

    auto HIDItemParser::handle_global(const USB::HIDItemPrefix* prefix, const ShortItem& item)
        -> bool {
        if (prefix->item_tag() == HIDItemTag::PUSH) {
            m_global_item_state_table_stack.add_back(m_global_item_state_table);
            return true;
        }
        if (prefix->item_tag() == HIDItemTag::POP) {
            bool is_pop_successful =
                m_global_item_state_table_stack.remove_back()
                    .transform<bool>([this](LinkedList<ShortItem> table) -> bool {
                        m_global_item_state_table.clear();
                        m_global_item_state_table = move(table);
                        return true;
                    })
                    .value_or(false);
            if (!is_pop_successful) {
                WARN("Failed to pop global item state table");
                return false;
            }
            return true;
        }

        // Overwrite GLOBAL if tag is already present
        bool tag_found = false;
        for (auto& g_item : m_global_item_state_table) {
            if (g_item.m_prefix.item_tag() == prefix->item_tag()) {
                g_item    = item;
                tag_found = true;
                break;
            }
        }
        if (!tag_found) {
            // No global with same tag found -> add it
            m_global_item_state_table.add_back(item);
        }
        return true;
    }

    auto HIDItemParser::current_usage_page() const -> U16 {
        for (auto& g_item : m_global_item_state_table) {
            if (g_item.m_prefix.item_tag() == HIDItemTag::USAGE_PAGE)
                return static_cast<U16>(word_get(g_item.value(), 0));
        }
        return 0;
    }

    auto HIDItemParser::handle_local(const USB::HIDItemPrefix* prefix, const ShortItem& item)
        -> bool {
        if (prefix->item_tag() == HIDItemTag::DELIMITER) {
            m_in_delimiter = item.value() == 1;
            m_alias_taken  = false;
            return true;
        }

        if (m_in_delimiter && m_alias_taken) return true;

        m_local_item_state_table.add_back({.m_item = item, .m_usage_page = current_usage_page()});

        if (m_in_delimiter
            && (prefix->item_tag() == HIDItemTag::USAGE
                || prefix->item_tag() == HIDItemTag::USAGE_MAXIMUM))
            m_alias_taken = true;
        return true;
    }

    auto HIDItemParser::advance_until_main_item() -> bool {
        while (m_cursor < m_length) {
            if (!skip_long_items()) return false;

            const auto* prefix =
                reinterpret_cast<const USB::HIDItemPrefix*>(m_descriptor + m_cursor);

            if (prefix->type() == USB::HIDItemType::MAIN) return true;

            Optional<ShortItem> maybe_item = build_short_item();
            if (!maybe_item) return false;
            const auto& item = maybe_item.value();
            if (prefix->type() == USB::HIDItemType::GLOBAL) {
                if (!handle_global(prefix, item)) return false;
            } else if (prefix->type() == USB::HIDItemType::LOCAL) {
                if (!handle_local(prefix, item)) return false;
            }
            m_cursor += prefix->size();
        }
        return false;
    }

    auto HIDItemParser::build_composite_global_items() -> CompositeGlobalItems {
        U16  usage_page           = 0;
        S32  logical_min          = 0;
        S32  logical_max          = 0;
        S32  physical_min         = 0;
        S32  physical_max         = 0;
        bool physical_min_defined = false;
        bool physical_max_defined = false;
        U32  unit                 = 0;
        S8   unit_exponent        = 0;
        U32  report_size          = 0;
        U32  report_count         = 0;
        U8   report_id            = 0;

        // The minima decide whether the maxima are read as signed, so they must be resolved first:
        // HID 1.11 §6.2.2.7 does not require a minimum to be declared before its maximum.
        for (auto& item : m_global_item_state_table) {
            switch (item.m_prefix.item_tag()) {
                case HIDItemTag::LOGICAL_MINIMUM: {
                    // The minimum is always signed, HID 1.11 §6.2.2.7.
                    logical_min = item.signed_value();
                    break;
                }
                case HIDItemTag::PHYSICAL_MINIMUM: {
                    physical_min         = item.signed_value();
                    physical_min_defined = true;
                    break;
                }
                default: break;
            }
        }

        for (auto& item : m_global_item_state_table) {
            switch (item.m_prefix.item_tag()) {
                case HIDItemTag::USAGE_PAGE: {
                    usage_page = static_cast<U16>(item.value());
                    break;
                }
                case HIDItemTag::LOGICAL_MAXIMUM: {
                    // The extents are only signed if the minimum is negative, otherwise both are
                    // unsigned, e.g. "Logical Minimum (0), Logical Maximum (0xFF)" is 0 to 255 and
                    // not 0 to -1. HID 1.11 §6.2.2.7 defines the unsigned/signed rule but not how
                    // to disambiguate the maximum, keying off the minimum is the common convention.
                    logical_max =
                        logical_min < 0 ? item.signed_value() : static_cast<S32>(item.value());
                    break;
                }
                case HIDItemTag::PHYSICAL_MAXIMUM: {
                    physical_max =
                        physical_min < 0 ? item.signed_value() : static_cast<S32>(item.value());
                    physical_max_defined = true;
                    break;
                }
                case HIDItemTag::UNIT: {
                    unit = item.value();
                    break;
                }
                case HIDItemTag::UNIT_EXPONENT: {
                    // A 4 bit signed code: 0x0-0x7 are 0 to 7 and 0x8-0xF are -8 to -1.
                    U8 val        = item.m_data[0] & 0xF;
                    unit_exponent = val < 0x8 ? val : val - 0x10;
                    break;
                }
                case HIDItemTag::REPORT_SIZE: {
                    // Clamped so that report_count * report_size cannot overflow downstream.
                    report_size = item.value();
                    break;
                }
                case HIDItemTag::REPORT_COUNT: {
                    report_count = item.value();
                    break;
                }
                case HIDItemTag::REPORT_ID: {
                    report_id = item.m_data[0];
                    break;
                }
                default: break;
            }
        }
        // The physical extents default to the logical extents while undefined and revert back to
        // them when both are declared as zero, HID 1.11 §6.2.2.7.
        bool physical_defined = (physical_min_defined && physical_max_defined)
                                && (physical_min != 0 || physical_max != 0);
        if (!physical_defined) {
            physical_min = logical_min;
            physical_max = logical_max;
        }
        return {
            .m_usage_page       = usage_page,
            .m_logical_minimum  = logical_min,
            .m_logical_maximum  = logical_max,
            .m_physical_minimum = physical_min,
            .m_physical_maximum = physical_max,
            .m_unit             = unit,
            .m_unit_exponent    = unit_exponent,
            .m_report_size      = report_size,
            .m_report_count     = report_count,
            .m_report_ID        = report_id,
        };
    }

    auto HIDItemParser::build_composite_local_items() -> CompositeLocalItems {
        LinkedList<HIDExtendedUsageRange> usage_ranges;
        HIDExtendedUsage                  usage_minimum{};
        bool                              usage_minimum_defined = false;
        U32                               designator_index      = 0;
        U32                               designator_minimum    = 0;
        U32                               designator_maximum    = 0;
        U32                               string_index          = 0;
        U32                               string_minimum        = 0;
        U32                               string_maximum        = 0;

        for (auto& local : m_local_item_state_table) {
            const ShortItem& item = local.m_item;
            switch (item.m_prefix.item_tag()) {
                case HIDItemTag::USAGE: {
                    HIDExtendedUsage usage = local.extended_usage();
                    usage_ranges.add_back({.m_min = usage, .m_max = usage});
                    break;
                }
                case HIDItemTag::USAGE_MINIMUM: {
                    usage_minimum         = local.extended_usage();
                    usage_minimum_defined = true;
                    break;
                }
                case HIDItemTag::USAGE_MAXIMUM: {
                    if (!usage_minimum_defined) {
                        WARN("Missing USAGE_MINIMUM item detected. Will ignore")
                        break;
                    }
                    if (local.extended_usage().usage() < usage_minimum.usage()) {
                        WARN("USAGE_MAXIMUM < USAGE_MINIMUM: {} < {}. Will ignore",
                             local.extended_usage().usage(),
                             usage_minimum.usage())
                        break;
                    }
                    usage_ranges.add_back(
                        {.m_min = usage_minimum, .m_max = local.extended_usage()});
                    usage_minimum         = {};
                    usage_minimum_defined = false;
                    break;
                }
                case HIDItemTag::DESIGNATOR_INDEX: {
                    designator_index = item.value();
                    break;
                }
                case HIDItemTag::DESIGNATOR_MINIMUM: {
                    designator_minimum = item.value();
                    break;
                }
                case HIDItemTag::DESIGNATOR_MAXIMUM: {
                    designator_maximum = item.value();
                    break;
                }
                case HIDItemTag::STRING_INDEX: {
                    string_index = item.value();
                    break;
                }
                case HIDItemTag::STRING_MINIMUM: {
                    string_minimum = item.value();
                    break;
                }
                case HIDItemTag::STRING_MAXIMUM: {
                    string_maximum = item.value();
                    break;
                }
                default: break;
            }
        }
        return {
            .m_usage_ranges       = move(usage_ranges),
            .m_designator_index   = designator_index,
            .m_designator_minimum = designator_minimum,
            .m_designator_maximum = designator_maximum,
            .m_string_index       = string_index,
            .m_string_minimum     = string_minimum,
            .m_string_maximum     = string_maximum,
        };
    }

    auto HIDItemParser::build_main_item() -> Optional<MainItem> {
        const auto* prefix = reinterpret_cast<const USB::HIDItemPrefix*>(m_descriptor + m_cursor);
        if (prefix->type() != HIDItemType::MAIN) return {};
        auto short_item = build_short_item();
        if (!short_item) return {};
        auto global_items = build_composite_global_items();
        auto local_items  = build_composite_local_items();
        m_local_item_state_table.clear();
        // Safeguard against a DELIMITER(CLOSE): This would lead to local items being dropped for
        // the next main items
        m_in_delimiter  = false;
        m_alias_taken   = false;
        m_cursor       += prefix->size();
        return {
            MainItem{.m_short_item = short_item.value(),
                     .m_global     = global_items,
                     .m_local      = local_items}
        };
    }

    auto HIDItemParser::parse_main_items() -> LinkedList<MainItem> {
        LinkedList<MainItem> main_items;
        while (m_cursor < m_length) {
            if (!advance_until_main_item()) break;
            auto maybe_main_item = build_main_item();
            if (!maybe_main_item) {
                WARN("Failed to parse MAIN item");
                break;
            }
            main_items.add_back(maybe_main_item.value());
        }
        return main_items;
    }

    auto HIDItemParser::log_reports(const HIDReports& reports) {
        DEBUG("HID Reports - Uses Report IDs={}", reports.m_uses_report_IDs)
        for (const auto& kv : reports.m_reports) {
            auto*                                       report = kv.value;
            StringRepresentation<USB::HIDExtendedUsage> ext_usage_str_repr;
            DEBUG("    REPORT({}, {}) - {}, S{}",
                  report->m_tag.m_report_ID,
                  report->m_tag.m_type.to_string(),
                  ext_usage_str_repr(report->m_usage),
                  report->m_bit_size)

            for (auto& data : report->m_data) {
                USB::HIDCollectionPathEntry c_entries[data.m_collection_path.size()];
                data.m_collection_path.as_array(c_entries);
                USB::HIDExtendedUsageRange ranges[data.m_usage_ranges.size()];
                data.m_usage_ranges.as_array(ranges);

                String unit = data.unit_as_string();
                if (unit != USB::HIDData::UNIT_NONE && data.m_unit_exponent != 0) {
                    unit = String::format("{}E*{}", data.m_unit_exponent, unit);
                }
                DEBUG("        DATA({}, {}) - L{}-{} P{}-{} D{}x{} O{} V?{}",
                      data.m_flags.decode_flags(),
                      unit,
                      data.m_logical_minimum,
                      data.m_logical_maximum,
                      data.m_physical_minimum,
                      data.m_physical_maximum,
                      data.m_report_count,
                      data.m_report_size,
                      data.m_bit_offset,
                      data.m_is_vendor_defined_data)
                DEBUG("            C: {} ",
                      String::join(", ", c_entries, data.m_collection_path.size()))
                DEBUG("            U: {} ", String::join(", ", ranges, data.m_usage_ranges.size()))
            }
        }
    }

    auto HIDItemParser::parse_hid_reports() -> HIDReports {
        LinkedList<MainItem> main_items = parse_main_items();

        bool                               uses_report_IDs = false;
        HashMap<HIDReportTag, HIDReport>   reports;
        LinkedList<HIDCollectionPathEntry> path;
        // counter > 0: The parser is inside a vendor-defined collection.
        int vendor_defined_coll_depth = 0;

        for (auto& main_item : main_items) {
            switch (main_item.m_short_item.m_prefix.item_tag()) {
                case HIDItemTag::COLLECTION: {
                    if (main_item.m_global.m_report_ID > 0) uses_report_IDs = true;

                    HIDExtendedUsage usage{};
                    if (!main_item.m_local.m_usage_ranges.empty())
                        usage = main_item.m_local.m_usage_ranges[0].m_min;
                    path.add_back({.m_type = main_item.m_short_item.m_data[0], .m_usage = usage});
                    if (path.last().m_type == HIDCollectionType::NONE) vendor_defined_coll_depth++;
                    break;
                }
                case HIDItemTag::INPUT:
                case HIDItemTag::OUTPUT:
                case HIDItemTag::FEATURE: {
                    if (main_item.m_global.m_report_ID > 0) uses_report_IDs = true;

                    HIDReportType type = HIDReportType::NONE;
                    if (main_item.m_short_item.m_prefix.item_tag() == HIDItemTag::INPUT)
                        type = HIDReportType::INPUT;
                    else if (main_item.m_short_item.m_prefix.item_tag() == HIDItemTag::OUTPUT)
                        type = HIDReportType::OUTPUT;
                    else
                        type = HIDReportType::FEATURE;
                    HIDReportTag tag{.m_type = type, .m_report_ID = main_item.m_global.m_report_ID};

                    HIDData data{.m_flags                  = {main_item.m_short_item.value()},
                                 .m_collection_path        = path,
                                 .m_usage_ranges           = move(main_item.m_local.m_usage_ranges),
                                 .m_logical_minimum        = main_item.m_global.m_logical_minimum,
                                 .m_logical_maximum        = main_item.m_global.m_logical_maximum,
                                 .m_physical_minimum       = main_item.m_global.m_physical_minimum,
                                 .m_physical_maximum       = main_item.m_global.m_physical_maximum,
                                 .m_unit                   = main_item.m_global.m_unit,
                                 .m_unit_exponent          = main_item.m_global.m_unit_exponent,
                                 .m_report_size            = main_item.m_global.m_report_size,
                                 .m_report_count           = main_item.m_global.m_report_count,
                                 .m_bit_offset             = 0,
                                 .m_is_vendor_defined_data = vendor_defined_coll_depth > 0};
                    auto&   report = reports[tag];
                    if (report.m_tag.m_type == HIDReportType::NONE) report.m_tag = tag;
                    if (report.m_usage.m_extended_usage == 0) {
                        for (auto& entry : path) {
                            if (entry.m_type != HIDCollectionType::APPLICATION) continue;
                            report.m_usage = entry.m_usage;
                            break;
                        }
                    }
                    data.m_bit_offset  = report.m_bit_size;
                    report.m_bit_size += data.m_report_count * data.m_report_size;
                    report.m_data.add_back(data);
                    break;
                }
                case HIDItemTag::END_COLLECTION: {
                    auto c = path.remove_back();
                    if (c.has_value() && c.value().m_type == HIDCollectionType::NONE)
                        vendor_defined_coll_depth--;
                    break;
                }
                default: break; // Global/Local item tags are already parsed
            }
        }

        HIDReports r = {.m_uses_report_IDs = uses_report_IDs, .m_reports = move(reports)};
#if LOG_DEBUG_ENABLED
        log_reports(r);
#endif
        return r;
    }
} // namespace Rune::Device::USB
