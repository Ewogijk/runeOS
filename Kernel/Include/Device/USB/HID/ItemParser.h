
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

#ifndef RUNEOS_ITEMPARSER_H
#define RUNEOS_ITEMPARSER_H

#include <KRE/Utility.h>

#include <Device/USB/HID/HID.h>

namespace Rune::Device::USB {
    /// @brief A one-shot HID item parser.
    class HIDItemParser {

        // ====================================================================================== //
        // Intermediate parsing structs
        // ====================================================================================== //

        /// @brief A copy of a short item.
        struct ShortItem {
            USB::HIDItemPrefix m_prefix;
            Array<U8, 4>       m_data;

            /// @brief
            /// @return The item data as an unsigned 32-bit integer.
            [[nodiscard]] auto value() const -> U32;

            /// @brief
            /// @return The sign extended item data as a signed 32-bit integer.
            [[nodiscard]] auto signed_value() const -> S32;
        };
        /// @brief A local item together with the Usage Page that was in effect when it was
        ///         declared.
        struct LocalItem {
            ShortItem m_item{};
            U16       m_usage_page = 0;

            /// @brief
            /// @return Create an extended usage with the usage page and short item value.
            [[nodiscard]] auto extended_usage() const -> HIDExtendedUsage;
        };

        /// @brief A snapshot of the values of all global items defined at the time of encountering
        ///         a MAIN item.
        struct CompositeGlobalItems {
            /// @brief Usage Page.
            U16 m_usage_page = 0;

            /// @brief Logical extents.
            S32 m_logical_minimum = 0;
            S32 m_logical_maximum = 0;

            /// @brief Physical extents.
            S32 m_physical_minimum = 0;
            S32 m_physical_maximum = 0;

            /// @brief The packed Unit item: nibble 0 is the unit system, nibbles 1-6 hold a 4-bit
            ///         signed exponent per base unit. HID 1.11 §6.2.2.7.
            U32 m_unit = 0;
            /// @brief Base 10 exponent, decoded from a 4-bit signed code, so -8 to 7.
            S8 m_unit_exponent = 0;

            /// @brief Report Size and Report Count.
            U32 m_report_size  = 0;
            U32 m_report_count = 0;
            /// @brief Report ID, always a single byte. HID 1.11 §6.2.2.7.
            U8 m_report_ID = 0;
        };

        /// @brief A snapshot of the values of all local items defined at the time of encountering a
        ///         MAIN item.
        struct CompositeLocalItems {
            LinkedList<HIDExtendedUsageRange> m_usage_ranges;

            U32 m_designator_index   = 0;
            U32 m_designator_minimum = 0;
            U32 m_designator_maximum = 0;

            U32 m_string_index   = 0;
            U32 m_string_minimum = 0;
            U32 m_string_maximum = 0;
        };

        /// @brief A MAIN item with a snapshot of the global and local item values defined at parse
        ///         time.
        struct MainItem {
            ShortItem            m_short_item{};
            CompositeGlobalItems m_global;
            CompositeLocalItems  m_local;
        };

        // ====================================================================================== //
        // Class properties
        // ====================================================================================== //

        const U8*             m_descriptor;
        size_t                m_length;
        size_t                m_cursor{};
        LinkedList<ShortItem> m_global_item_state_table;
        LinkedList<LocalItem> m_local_item_state_table;

        LinkedList<LinkedList<ShortItem>> m_global_item_state_table_stack;

        /// @brief True while the cursor is inside an open Delimiter, HID 1.11 §6.2.2.8.
        bool m_in_delimiter = false;

        /// @brief True once the first alias of the open Delimiter is complete.
        bool m_alias_taken = false;

        /// @brief The Usage Page currently held by the global item state table.
        /// @return The low word of the Usage Page item, 0 if none was declared yet.
        [[nodiscard]] auto current_usage_page() const -> U16;

        /// @brief Check if the cursor points to a long item and try to skip it.
        /// @return True:  Valid cursor - No long item detected or skipped without invalidating the
        ///                 cursor.
        ///         False: Invalid cursor - Long item detected, but it is truncated.
        auto skip_long_items() -> bool;

        auto build_short_item() -> Optional<ShortItem>;

        auto handle_global(const USB::HIDItemPrefix* prefix, const ShortItem& item) -> bool;

        auto handle_local(const USB::HIDItemPrefix* prefix, const ShortItem& item) -> bool;

        /// @brief Advance the cursor until a main item is detected.
        /// @return True: A main item was found.
        ///         False: No more items are left in the stream or a malformed item was detected.
        ///
        /// Global items will be added to the global item state table. If a global item with the
        /// same tag already exists, it will be overwritten.
        ///
        /// Local items will be added to the local item state table.
        auto advance_until_main_item() -> bool;

        /// @brief Handle a single global item.
        /// @return
        auto build_composite_global_items() -> CompositeGlobalItems;

        auto build_composite_local_items() -> CompositeLocalItems;

        auto build_main_item() -> Optional<MainItem>;

        auto parse_main_items() -> LinkedList<MainItem>;

      public:
        HIDItemParser(const U8* descriptor, size_t length);

        /// @brief Parse the report descriptor.
        /// @return All HID reports published by the device.
        ///
        /// The parser state is not reset after parsing, thus this function can only parse the
        /// descriptor once.
        auto parse_hid_reports() -> HIDReports;
    };
} // namespace Rune::Device::USB

#endif // RUNEOS_ITEMPARSER_H
