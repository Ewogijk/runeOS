
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

#ifndef RUNEOS_HID_H
#define RUNEOS_HID_H

#include <Ember/Ember.h>
#include <Ember/Enum.h>

#include <KRE/BitsAndBytes.h>
#include <KRE/Collections/Array.h>
#include <KRE/Collections/HashMap.h>
#include <KRE/Collections/LinkedList.h>
#include <KRE/String.h>

#include <Device/USB/HID/UsageTables.h>
#include <Device/USB/Request.h>

namespace Rune::Device::USB {

    // ========================================================================================== //
    // HID Class Descriptor Types — HID 1.11 §6.2, §7.1
    // ========================================================================================== //

#define HID_DESCRIPTOR_TYPES(X)                                                                    \
    X(HIDDescriptorType, HID, 0x21)                                                                \
    X(HIDDescriptorType, REPORT, 0x22)                                                             \
    X(HIDDescriptorType, PHYSICAL, 0x23)

    /// @brief The class-specific descriptor types of a HID device — HID 1.11 §7.1.
    ///
    /// Used in bDescriptorType of every HID class descriptor and in the high byte of wValue for
    /// GET_DESCRIPTOR. Bits 6..5 of these values are 01, marking them as Class descriptor types
    /// as opposed to the Standard types in DescriptorType; 0x24-0x2F are reserved.
    ///
    /// - HID (0x21): HID descriptor, names the subordinate descriptors of an interface.
    /// - REPORT (0x22): Report descriptor, always present.
    /// - PHYSICAL (0x23): Physical descriptor set, optional.
    DECLARE_TYPED_ENUM(HIDDescriptorType, U8, HID_DESCRIPTOR_TYPES, 0x00) // NOLINT

    // ========================================================================================== //
    // HID Descriptor — HID 1.11 §6.2.1
    // ========================================================================================== //

#define HID_COUNTRY_CODES(X)                                                                       \
    X(HIDCountryCode, NOT_SUPPORTED, 0)                                                            \
    X(HIDCountryCode, ARABIC, 1)                                                                   \
    X(HIDCountryCode, BELGIAN, 2)                                                                  \
    X(HIDCountryCode, CANADIAN_BILINGUAL, 3)                                                       \
    X(HIDCountryCode, CANADIAN_FRENCH, 4)                                                          \
    X(HIDCountryCode, CZECH_REPUBLIC, 5)                                                           \
    X(HIDCountryCode, DANISH, 6)                                                                   \
    X(HIDCountryCode, FINNISH, 7)                                                                  \
    X(HIDCountryCode, FRENCH, 8)                                                                   \
    X(HIDCountryCode, GERMAN, 9)                                                                   \
    X(HIDCountryCode, GREEK, 10)                                                                   \
    X(HIDCountryCode, HEBREW, 11)                                                                  \
    X(HIDCountryCode, HUNGARY, 12)                                                                 \
    X(HIDCountryCode, INTERNATIONAL_ISO, 13)                                                       \
    X(HIDCountryCode, ITALIAN, 14)                                                                 \
    X(HIDCountryCode, JAPAN_KATAKANA, 15)                                                          \
    X(HIDCountryCode, KOREAN, 16)                                                                  \
    X(HIDCountryCode, LATIN_AMERICAN, 17)                                                          \
    X(HIDCountryCode, NETHERLANDS_DUTCH, 18)                                                       \
    X(HIDCountryCode, NORWEGIAN, 19)                                                               \
    X(HIDCountryCode, PERSIAN_FARSI, 20)                                                           \
    X(HIDCountryCode, POLAND, 21)                                                                  \
    X(HIDCountryCode, PORTUGUESE, 22)                                                              \
    X(HIDCountryCode, RUSSIA, 23)                                                                  \
    X(HIDCountryCode, SLOVAKIA, 24)                                                                \
    X(HIDCountryCode, SPANISH, 25)                                                                 \
    X(HIDCountryCode, SWEDISH, 26)                                                                 \
    X(HIDCountryCode, SWISS_FRENCH, 27)                                                            \
    X(HIDCountryCode, SWISS_GERMAN, 28)                                                            \
    X(HIDCountryCode, SWITZERLAND, 29)                                                             \
    X(HIDCountryCode, TAIWAN, 30)                                                                  \
    X(HIDCountryCode, TURKISH_Q, 31)                                                               \
    X(HIDCountryCode, UK, 32)                                                                      \
    X(HIDCountryCode, US, 33)                                                                      \
    X(HIDCountryCode, YUGOSLAVIA, 34)                                                              \
    X(HIDCountryCode, TURKISH_F, 35)

    /// @brief bCountryCode of the HID descriptor: which country the hardware is localized for —
    ///         HID 1.11 §6.2.1.
    ///
    /// Most hardware is not localized and reports NOT_SUPPORTED (0). Keyboards may use the field
    /// to indicate the language of the key caps but are not required to. 36-255 are reserved, NONE
    /// (0xFF) marks an unrecognized code rather than a code the spec defines.
    DECLARE_TYPED_ENUM(HIDCountryCode, U8, HID_COUNTRY_CODES, 0xFF) // NOLINT

    /// @brief One subordinate class descriptor named by a HID descriptor — the (bDescriptorType,
    ///         wDescriptorLength) pair at offsets (3*n)+6 and (3*n)+7 of HID 1.11 §6.2.1.
    ///
    /// The first pair is mandatory and describes the Report descriptor. Optional pairs follow,
    /// one per additional class descriptor. Not a descriptor in its own right, so it has no
    /// bLength of its own.
    struct HIDSubordinateDescriptor {
        U8  m_descriptor_type; // a HIDDescriptorType
        U16 m_length;          // total size of the subordinate descriptor

        [[nodiscard]] auto descriptor_type() const -> HIDDescriptorType {
            return HIDDescriptorType(m_descriptor_type);
        }
    } PACKED;
    static_assert(sizeof(HIDSubordinateDescriptor) == 3); // NOLINT

    /// @brief Identifies the length and type of the subordinate descriptors of a HID interface —
    ///         HID 1.11 §6.2.1.
    ///
    /// Returned interleaved between the Interface and Endpoint descriptors of a HID interface by
    /// GET_DESCRIPTOR(CONFIGURATION) (HID 1.11 §7.1); read it out of the configuration blob via
    /// AlternateSetting::m_class_descriptors rather than requesting it separately.
    ///
    /// Variable length: sizeof() covers only the fixed header, the bNumDescriptors subordinate
    /// pairs follow it. Use subordinate() to reach them, never member arithmetic.
    struct HIDDescriptor {
        /// @brief bLength when only the mandatory Report descriptor is declared, the common case.
        static constexpr U8 SIZE_SINGLE_SUBORDINATE = 9;

        U8  m_length;
        U8  m_descriptor_type; // = 0x21 (HID)
        U16 m_bcd_HID;         // HID class specification release (e.g. 0x0111 = HID 1.11)
        U8  m_country_code;    // a HIDCountryCode
        U8  m_num_descriptors; // number of subordinate class descriptors, always >= 1

        [[nodiscard]] auto country_code() const -> HIDCountryCode {
            return HIDCountryCode(m_country_code);
        }

        /// @brief
        /// @param index Zero-based subordinate descriptor index.
        /// @return The subordinate pair, nullptr if index is beyond bNumDescriptors or the pair
        ///          would lie outside the bLength the device reported.
        [[nodiscard]] auto subordinate(U8 index) const -> const HIDSubordinateDescriptor* {
            if (index >= m_num_descriptors) return nullptr;
            auto offset = static_cast<U16>(sizeof(HIDDescriptor)
                                           + (index * sizeof(HIDSubordinateDescriptor)));
            if (offset + sizeof(HIDSubordinateDescriptor) > m_length) return nullptr;
            return reinterpret_cast<const HIDSubordinateDescriptor*>(
                reinterpret_cast<const U8*>(this) + offset);
        }

        /// @brief wDescriptorLength of the Report descriptor, the length to request with
        ///         GET_DESCRIPTOR(REPORT).
        /// @return The length, 0 if the device declared no Report descriptor (malformed, HID 1.11
        ///          §6.2.1 requires one).
        [[nodiscard]] auto report_descriptor_length() const -> U16 {
            for (U8 i = 0; i < m_num_descriptors; i++) {
                const HIDSubordinateDescriptor* sub = subordinate(i);
                if (sub == nullptr) break;
                if (sub->descriptor_type() == HIDDescriptorType::REPORT) return sub->m_length;
            }
            return 0;
        }
    } PACKED;
    static_assert(sizeof(HIDDescriptor) == 6); // NOLINT

    // ========================================================================================== //
    // Report Descriptor Items — HID 1.11 §6.2.2
    // ========================================================================================== //

#define HID_ITEM_TYPES(X)                                                                          \
    X(HIDItemType, MAIN, 0)                                                                        \
    X(HIDItemType, GLOBAL, 1)                                                                      \
    X(HIDItemType, LOCAL, 2)                                                                       \
    X(HIDItemType, RESERVED, 3)

    /// @brief bType of an item prefix, bits 3..2 — HID 1.11 §6.2.2.2.
    ///
    /// - MAIN (0): Defines or groups data fields; snapshots the item state table.
    /// - GLOBAL (1): Describes subsequent items; persists until overridden.
    /// - LOCAL (2): Describes the next Main item only; cleared by it.
    /// - RESERVED (3): Only used by the long item format.
    DECLARE_TYPED_ENUM(HIDItemType, U8, HID_ITEM_TYPES, 0xFF) // NOLINT

#define HID_ITEM_TAGS(X)                                                                           \
    X(HIDItemTag, INPUT, 0x80)                                                                     \
    X(HIDItemTag, OUTPUT, 0x90)                                                                    \
    X(HIDItemTag, FEATURE, 0xB0)                                                                   \
    X(HIDItemTag, COLLECTION, 0xA0)                                                                \
    X(HIDItemTag, END_COLLECTION, 0xC0)                                                            \
    X(HIDItemTag, USAGE_PAGE, 0x04)                                                                \
    X(HIDItemTag, LOGICAL_MINIMUM, 0x14)                                                           \
    X(HIDItemTag, LOGICAL_MAXIMUM, 0x24)                                                           \
    X(HIDItemTag, PHYSICAL_MINIMUM, 0x34)                                                          \
    X(HIDItemTag, PHYSICAL_MAXIMUM, 0x44)                                                          \
    X(HIDItemTag, UNIT_EXPONENT, 0x54)                                                             \
    X(HIDItemTag, UNIT, 0x64)                                                                      \
    X(HIDItemTag, REPORT_SIZE, 0x74)                                                               \
    X(HIDItemTag, REPORT_ID, 0x84)                                                                 \
    X(HIDItemTag, REPORT_COUNT, 0x94)                                                              \
    X(HIDItemTag, PUSH, 0xA4)                                                                      \
    X(HIDItemTag, POP, 0xB4)                                                                       \
    X(HIDItemTag, USAGE, 0x08)                                                                     \
    X(HIDItemTag, USAGE_MINIMUM, 0x18)                                                             \
    X(HIDItemTag, USAGE_MAXIMUM, 0x28)                                                             \
    X(HIDItemTag, DESIGNATOR_INDEX, 0x38)                                                          \
    X(HIDItemTag, DESIGNATOR_MINIMUM, 0x48)                                                        \
    X(HIDItemTag, DESIGNATOR_MAXIMUM, 0x58)                                                        \
    X(HIDItemTag, STRING_INDEX, 0x78)                                                              \
    X(HIDItemTag, STRING_MINIMUM, 0x88)                                                            \
    X(HIDItemTag, STRING_MAXIMUM, 0x98)                                                            \
    X(HIDItemTag, DELIMITER, 0xA8)

    /// @brief Item tags of a Report descriptor — HID 1.11 §6.2.2.4 (Main), §6.2.2.7 (Global),
    ///         §6.2.2.8 (Local).
    ///
    /// Values are the item prefix byte with bSize masked off, i.e. bTag in bits 7..4 and bType in
    /// bits 3..2, matching the "One-Byte Prefix" column of the spec's tables. bType makes the
    /// three tag categories disjoint, so a parser can switch on
    /// HIDItemPrefix::item_tag() once instead of nesting a switch per item type.
    ///
    /// Main items — define or group data fields:
    /// - INPUT (0x80): Data field read by the host, via Get_Report or the Interrupt In pipe.
    /// - OUTPUT (0x90): Data field written by the host, via Set_Report or the Interrupt Out pipe.
    /// - FEATURE (0xB0): Device configuration field, not intended for the end user.
    /// - COLLECTION (0xA0): Opens a group of items; data is a HIDCollectionType.
    /// - END_COLLECTION (0xC0): Closes the innermost collection; carries no data.
    ///
    /// Global items — change the item state table for all subsequent items:
    /// - USAGE_PAGE (0x04): High 16 bits of subsequent 16-bit usages.
    /// - LOGICAL_MINIMUM (0x14) / LOGICAL_MAXIMUM (0x24): Extent of the reported value.
    /// - PHYSICAL_MINIMUM (0x34) / PHYSICAL_MAXIMUM (0x44): Extent in physical units; both
    ///   default to the logical extents until declared, and revert to that default when both are
    ///   declared as 0.
    /// - UNIT_EXPONENT (0x54): Base 10 exponent applied to the physical extents.
    /// - UNIT (0x64): Unit nibbles, see HIDUnitSystem.
    /// - REPORT_SIZE (0x74): Size of one data field in bits.
    /// - REPORT_ID (0x84): Prefixes every report of the device with a one byte ID. Must be
    ///   declared before the first Input/Output/Feature item; 0 is reserved.
    /// - REPORT_COUNT (0x94): Number of data fields the next Main item creates. For an array item
    ///   this is the maximum number of simultaneously reported controls.
    /// - PUSH (0xA4) / POP (0xB4): Save/restore the global item state table on a stack.
    ///
    /// Local items — describe the next Main item only and are cleared by it:
    /// - USAGE (0x08): Usage of one control; repeat for each control of a Main item.
    /// - USAGE_MINIMUM (0x18) / USAGE_MAXIMUM (0x28): Usage range for an array or bitmap.
    /// - DESIGNATOR_INDEX (0x38), DESIGNATOR_MINIMUM (0x48), DESIGNATOR_MAXIMUM (0x58): Index or
    ///   range into a Physical descriptor set.
    /// - STRING_INDEX (0x78), STRING_MINIMUM (0x88), STRING_MAXIMUM (0x98): Index or range into
    ///   the device's String descriptors.
    /// - DELIMITER (0xA8): Opens (1) or closes (0) a set of alternative usages for one control.
    ///   Parsers must handle delimiters; supporting usages beyond the first is optional.
    DECLARE_TYPED_ENUM(HIDItemTag, U8, HID_ITEM_TAGS, 0x00) // NOLINT

    /// @brief The one byte prefix every Report descriptor item starts with — HID 1.11 §6.2.2.2.
    ///
    /// Packs bSize (bits 1..0), bType (bits 3..2) and bTag (bits 7..4). A short item is the
    /// prefix plus 0, 1, 2 or 4 data bytes; the data is unsigned unless both Logical Minimum and
    /// Logical Maximum are negative-capable, in which case it is 2's complement (HID 1.11
    /// §6.2.2.7). This spec defines no long item tags, so is_long_item() means "skip me".
    struct HIDItemPrefix {
        /// @brief Prefix of a long item: bSize=2, bType=3, bTag=1111 (HID 1.11 §6.2.2.3). A long
        ///         item is followed by bDataSize, bLongItemTag and up to 255 data bytes.
        static constexpr U8 LONG_ITEM = 0xFE;
        /// @brief Masks bSize off the prefix, leaving a value comparable to a HIDItemTag.
        static constexpr U8 TAG_AND_TYPE_MASK = 0xFC;
        /// @brief bSize 3 encodes 4 data bytes, not 3.
        static constexpr U8 SIZE_FOUR_BYTES = 3;

        U8 m_prefix;

        /// @brief bSize decoded into a byte count: 0, 1, 2 or 4.
        [[nodiscard]] auto data_size() const -> U8 {
            U8 size = m_prefix & SIZE_MASK;
            return size == SIZE_FOUR_BYTES ? 4 : size;
        }

        [[nodiscard]] auto type() const -> HIDItemType {
            return HIDItemType(static_cast<U8>((m_prefix >> TYPE_SHIFT) & TYPE_MASK));
        }

        /// @brief The raw 4-bit bTag; only unique within one HIDItemType. Prefer item_tag().
        [[nodiscard]] auto tag() const -> U8 { return static_cast<U8>(m_prefix >> TAG_SHIFT); }

        /// @brief bTag and bType together, the value a HIDItemTag holds.
        [[nodiscard]] auto item_tag() const -> HIDItemTag {
            return HIDItemTag(static_cast<U8>(m_prefix & TAG_AND_TYPE_MASK));
        }

        [[nodiscard]] auto is_long_item() const -> bool { return m_prefix == LONG_ITEM; }

        /// @brief Total size of a short item in bytes, prefix included; advance by this to reach
        ///         the next item. Meaningless for a long item.
        [[nodiscard]] auto size() const -> U8 { return static_cast<U8>(1 + data_size()); }

      private:
        static constexpr U8 SIZE_MASK  = 0x03;
        static constexpr U8 TYPE_SHIFT = 2;
        static constexpr U8 TYPE_MASK  = 0x03;
        static constexpr U8 TAG_SHIFT  = 4;
    } PACKED;
    static_assert(sizeof(HIDItemPrefix) == 1); // NOLINT

#define HID_DATA_FLAGS(X)                                                                          \
    X(HIDDataFlag, CONSTANT, 0x0001)                                                               \
    X(HIDDataFlag, VARIABLE, 0x0002)                                                               \
    X(HIDDataFlag, RELATIVE, 0x0004)                                                               \
    X(HIDDataFlag, WRAP, 0x0008)                                                                   \
    X(HIDDataFlag, NON_LINEAR, 0x0010)                                                             \
    X(HIDDataFlag, NO_PREFERRED, 0x0020)                                                           \
    X(HIDDataFlag, NULL_STATE, 0x0040)                                                             \
    X(HIDDataFlag, VOLATILE, 0x0080)                                                               \
    X(HIDDataFlag, BUFFERED_BYTES, 0x0100)

    /// @brief The data of an Input, Output or Feature Main item, a bitmap — HID 1.11 §6.2.2.5.
    ///
    /// - CONSTANT (bit 0): Static read-only field the host cannot modify, e.g. report padding.
    /// - VARIABLE (bit 1): One field per control. Cleared means an array, where each field holds
    ///   the index of an asserted control (keyboard scan code style) and Report Count bounds how
    ///   many controls can be reported at once.
    /// - RELATIVE (bit 2): Value is a change since the last report rather than an absolute one.
    /// - WRAP (bit 3): Value rolls over at the logical extents.
    /// - NON_LINEAR (bit 4): Raw data has been processed, e.g. an acceleration curve.
    /// - NO_PREFERRED (bit 5): Control does not return to a rest state on its own.
    /// - NULL_STATE (bit 6): Control has a state where it reports a value outside the logical
    ///   extents to mean "no meaningful data".
    /// - VOLATILE (bit 7): Output/Feature only; the value can change without host interaction.
    ///   Reserved and zero for Input items.
    /// - BUFFERED_BYTES (bit 8): Field is a fixed-size byte stream, not a numeric value, and must
    ///   be aligned on a byte boundary.
    DECLARE_TYPED_ENUM(HIDDataFlag, U32, HID_DATA_FLAGS, 0x0) // NOLINT

    /// @brief The data bytes of an Input, Output or Feature Main item as the bitmap they are —
    ///         HID 1.11 §6.2.2.5.
    struct HIDDataFlags {
        /// @brief The raw data of the Main item.
        U32 m_flags = 0;

        /// @brief
        /// @param flag The flag to test for.
        /// @return True: The bit of the flag is set, the item is in the state the flag names.
        ///         False: The bit is cleared, the item is in the complementary state.
        [[nodiscard]] auto has(HIDDataFlag flag) const -> bool;

        /// @brief The set flags as a "|" separated list of their names, e.g. "VARIABLE|RELATIVE".
        [[nodiscard]] auto decode_flags() const -> String;
    };

#define HID_COLLECTION_TYPES(X)                                                                    \
    X(HIDCollectionType, PHYSICAL, 0x00)                                                           \
    X(HIDCollectionType, APPLICATION, 0x01)                                                        \
    X(HIDCollectionType, LOGICAL, 0x02)                                                            \
    X(HIDCollectionType, REPORT, 0x03)                                                             \
    X(HIDCollectionType, NAMED_ARRAY, 0x04)                                                        \
    X(HIDCollectionType, USAGE_SWITCH, 0x05)                                                       \
    X(HIDCollectionType, USAGE_MODIFIER, 0x06)

    /// @brief The data of a Collection Main item — HID 1.11 §6.2.2.6.
    ///
    /// A Usage must be associated with every collection. Collections nest and are optional except
    /// for the top-level Application collection. Main items inside a collection of an unknown
    /// vendor-defined type (0x80-0xFF) must be ignored, though its Global items still affect the
    /// item state table; the same applies to a known type carrying an unknown usage. 0x07-0x7F are
    /// reserved.
    ///
    /// - PHYSICAL (0x00): Data collected at one geometric point, e.g. one sensor of many.
    /// - APPLICATION (0x01): A group of Main items an application recognizes, e.g. a keyboard.
    ///   Reports are usually associated with one of these, at least one Report ID each.
    /// - LOGICAL (0x02): A composite data structure, e.g. a buffer paired with its byte count.
    /// - REPORT (0x03): Wraps all fields of one report; carries a unique Report ID.
    /// - NAMED_ARRAY (0x04): An array of selector usages, so the array field itself can be named.
    /// - USAGE_SWITCH (0x05): Special-cases the meaning of the usages it contains.
    /// - USAGE_MODIFIER (0x06): Modifies the usage of the encompassing collection.
    DECLARE_TYPED_ENUM(HIDCollectionType, U8, HID_COLLECTION_TYPES, 0xFF) // NOLINT

    // ========================================================================================== //
    // Physical Descriptors — HID 1.11 §6.2.3
    // ========================================================================================== //
    //
    // Entirely optional and rarely implemented: they map controls to the body part that actuates
    // them, for devices with many identical controls. Similar Physical descriptors are grouped
    // into sets; a Designator Index Local item in the Report descriptor selects one descriptor of
    // a set. Descriptor set 0 is special and reports how many sets exist and how long each is,
    // requested with GET_DESCRIPTOR(PHYSICAL) and a Descriptor Index of 0.

#define HID_PHYSICAL_BIASES(X)                                                                     \
    X(HIDPhysicalBias, RIGHT_HAND, 1)                                                              \
    X(HIDPhysicalBias, LEFT_HAND, 2)                                                               \
    X(HIDPhysicalBias, BOTH_HANDS, 3)                                                              \
    X(HIDPhysicalBias, EITHER_HAND, 4)

    /// @brief Bits 7..5 of bPhysicalInfo: which hand a descriptor set characterizes — HID 1.11
    ///         §6.2.3. NONE (0) is the spec's "Not applicable"; 5-7 are reserved. A device that
    ///         only fits one hand does not return sets biased to the other.
    DECLARE_TYPED_ENUM(HIDPhysicalBias, U8, HID_PHYSICAL_BIASES, 0) // NOLINT

#define HID_PHYSICAL_QUALIFIERS(X)                                                                 \
    X(HIDPhysicalQualifier, RIGHT, 1)                                                              \
    X(HIDPhysicalQualifier, LEFT, 2)                                                               \
    X(HIDPhysicalQualifier, BOTH, 3)                                                               \
    X(HIDPhysicalQualifier, EITHER, 4)                                                             \
    X(HIDPhysicalQualifier, CENTER, 5)

    /// @brief Bits 7..5 of bFlags: which side of the body the designator refers to — HID 1.11
    ///         §6.2.3. NONE (0) is the spec's "Not applicable"; 6 and 7 are reserved.
    DECLARE_TYPED_ENUM(HIDPhysicalQualifier, U8, HID_PHYSICAL_QUALIFIERS, 0) // NOLINT

#define HID_PHYSICAL_DESIGNATORS(X)                                                                \
    X(HIDPhysicalDesignator, HAND, 0x01)                                                           \
    X(HIDPhysicalDesignator, EYEBALL, 0x02)                                                        \
    X(HIDPhysicalDesignator, EYEBROW, 0x03)                                                        \
    X(HIDPhysicalDesignator, EYELID, 0x04)                                                         \
    X(HIDPhysicalDesignator, EAR, 0x05)                                                            \
    X(HIDPhysicalDesignator, NOSE, 0x06)                                                           \
    X(HIDPhysicalDesignator, MOUTH, 0x07)                                                          \
    X(HIDPhysicalDesignator, UPPER_LIP, 0x08)                                                      \
    X(HIDPhysicalDesignator, LOWER_LIP, 0x09)                                                      \
    X(HIDPhysicalDesignator, JAW, 0x0A)                                                            \
    X(HIDPhysicalDesignator, NECK, 0x0B)                                                           \
    X(HIDPhysicalDesignator, UPPER_ARM, 0x0C)                                                      \
    X(HIDPhysicalDesignator, ELBOW, 0x0D)                                                          \
    X(HIDPhysicalDesignator, FOREARM, 0x0E)                                                        \
    X(HIDPhysicalDesignator, WRIST, 0x0F)                                                          \
    X(HIDPhysicalDesignator, PALM, 0x10)                                                           \
    X(HIDPhysicalDesignator, THUMB, 0x11)                                                          \
    X(HIDPhysicalDesignator, INDEX_FINGER, 0x12)                                                   \
    X(HIDPhysicalDesignator, MIDDLE_FINGER, 0x13)                                                  \
    X(HIDPhysicalDesignator, RING_FINGER, 0x14)                                                    \
    X(HIDPhysicalDesignator, LITTLE_FINGER, 0x15)                                                  \
    X(HIDPhysicalDesignator, HEAD, 0x16)                                                           \
    X(HIDPhysicalDesignator, SHOULDER, 0x17)                                                       \
    X(HIDPhysicalDesignator, HIP, 0x18)                                                            \
    X(HIDPhysicalDesignator, WAIST, 0x19)                                                          \
    X(HIDPhysicalDesignator, THIGH, 0x1A)                                                          \
    X(HIDPhysicalDesignator, KNEE, 0x1B)                                                           \
    X(HIDPhysicalDesignator, CALF, 0x1C)                                                           \
    X(HIDPhysicalDesignator, ANKLE, 0x1D)                                                          \
    X(HIDPhysicalDesignator, FOOT, 0x1E)                                                           \
    X(HIDPhysicalDesignator, HEEL, 0x1F)                                                           \
    X(HIDPhysicalDesignator, BALL_OF_FOOT, 0x20)                                                   \
    X(HIDPhysicalDesignator, BIG_TOE, 0x21)                                                        \
    X(HIDPhysicalDesignator, SECOND_TOE, 0x22)                                                     \
    X(HIDPhysicalDesignator, THIRD_TOE, 0x23)                                                      \
    X(HIDPhysicalDesignator, FOURTH_TOE, 0x24)                                                     \
    X(HIDPhysicalDesignator, LITTLE_TOE, 0x25)                                                     \
    X(HIDPhysicalDesignator, BROW, 0x26)                                                           \
    X(HIDPhysicalDesignator, CHEEK, 0x27)

    /// @brief bDesignator of a Physical descriptor: the body part that actuates an item — HID
    ///         1.11 §6.2.3. NONE (0x00) is the spec's "None"; 0x28-0xFF are reserved.
    DECLARE_TYPED_ENUM(HIDPhysicalDesignator, U8, HID_PHYSICAL_DESIGNATORS, 0x00) // NOLINT

    /// @brief Physical descriptor set 0 — HID 1.11 §6.2.3: how many descriptor sets the device
    ///         has and how long each one is.
    ///
    /// Returned by GET_DESCRIPTOR(PHYSICAL) with a Descriptor Index of 0. Set 0 itself is not
    /// counted in bNumber, so the sets are requested with indices 1..bNumber.
    struct HIDPhysicalDescriptorSet0 {
        U8  m_number; // bNumber: number of Physical descriptor sets, set 0 excluded
        U16 m_length; // bLength: length of each Physical descriptor set
    } PACKED;
    static_assert(sizeof(HIDPhysicalDescriptorSet0) == 3); // NOLINT

    /// @brief One Physical descriptor of a set — HID 1.11 §6.2.3.
    ///
    /// Designator Index items in the Report descriptor select one of these by position within its
    /// set. Two controls only share a Designator/Qualifier/Effort triple when they are physically
    /// connected, e.g. the two halves of one long key cap.
    struct HIDPhysicalDescriptor {
        U8 m_designator; // bDesignator, a HIDPhysicalDesignator
        U8 m_flags;      // bFlags: bits 7..5 Qualifier, bits 4..0 Effort

        [[nodiscard]] auto designator() const -> HIDPhysicalDesignator {
            return HIDPhysicalDesignator(m_designator);
        }

        [[nodiscard]] auto qualifier() const -> HIDPhysicalQualifier {
            return HIDPhysicalQualifier(
                static_cast<U8>((m_flags >> QUALIFIER_SHIFT) & QUALIFIER_MASK));
        }

        /// @brief How hard the control is to reach: 0 means the body part rests on it, higher
        ///         values mean the user has to stretch further.
        [[nodiscard]] auto effort() const -> U8 { return m_flags & EFFORT_MASK; }

      private:
        static constexpr U8 QUALIFIER_SHIFT = 5;
        static constexpr U8 QUALIFIER_MASK  = 0x07;
        static constexpr U8 EFFORT_MASK     = 0x1F;
    } PACKED;
    static_assert(sizeof(HIDPhysicalDescriptor) == 2); // NOLINT

    /// @brief Header of a Physical descriptor set — HID 1.11 §6.2.3.
    ///
    /// Variable length: sizeof() covers only bPhysicalInfo, the Physical descriptors follow it.
    /// The set carries no length of its own, so descriptor() cannot bound-check the index; bound
    /// it by (HIDPhysicalDescriptorSet0::m_length - 1) / sizeof(HIDPhysicalDescriptor).
    struct HIDPhysicalDescriptorSet {
        U8 m_physical_info; // bits 7..5 Bias, bits 4..0 Preference

        [[nodiscard]] auto bias() const -> HIDPhysicalBias {
            return HIDPhysicalBias(static_cast<U8>((m_physical_info >> BIAS_SHIFT) & BIAS_MASK));
        }

        /// @brief How preferred this set is for its Bias; 0 is the most preferred or most typical
        ///         set, higher values are alternatives.
        [[nodiscard]] auto preference() const -> U8 { return m_physical_info & PREFERENCE_MASK; }

        /// @brief
        /// @param index Zero-based Physical descriptor index within this set.
        /// @return The Physical descriptor at that index. The caller must keep index within the
        ///          set length reported by descriptor set 0.
        [[nodiscard]] auto descriptor(U8 index) const -> const HIDPhysicalDescriptor* {
            return reinterpret_cast<const HIDPhysicalDescriptor*>(
                reinterpret_cast<const U8*>(this) + sizeof(HIDPhysicalDescriptorSet)
                + (index * sizeof(HIDPhysicalDescriptor)));
        }

      private:
        static constexpr U8 BIAS_SHIFT      = 5;
        static constexpr U8 BIAS_MASK       = 0x07;
        static constexpr U8 PREFERENCE_MASK = 0x1F;
    } PACKED;
    static_assert(sizeof(HIDPhysicalDescriptorSet) == 1); // NOLINT

    // ========================================================================================== //
    // Class-Specific Requests — HID 1.11 §7.2
    // ========================================================================================== //

#define HID_CLASS_REQUESTS(X)                                                                      \
    X(HIDClassRequest, GET_REPORT, 0x01)                                                           \
    X(HIDClassRequest, GET_IDLE, 0x02)                                                             \
    X(HIDClassRequest, GET_PROTOCOL, 0x03)                                                         \
    X(HIDClassRequest, SET_REPORT, 0x09)                                                           \
    X(HIDClassRequest, SET_IDLE, 0x0A)                                                             \
    X(HIDClassRequest, SET_PROTOCOL, 0x0B)

    /// @brief bRequest of a HID class-specific request — HID 1.11 §7.2. All of them go over the
    ///         Default (control) pipe with the interface as recipient; 0x04-0x08 are reserved.
    ///
    /// - GET_REPORT (0x01): Read a report over the Control pipe. Mandatory for every HID device.
    ///   Meant for initialization and for reading absolute or Feature state, not for polling —
    ///   use the Interrupt In pipe for recurring Input reports.
    /// - GET_IDLE (0x02): Read the current idle rate of an Input report.
    /// - GET_PROTOCOL (0x03): Read whether the boot or the report protocol is active. Required
    ///   only for devices in the Boot Interface subclass.
    /// - SET_REPORT (0x09): Send an Output or Feature report over the Control pipe. Required
    ///   whenever the device declares an Output report, and the only route for them when no
    ///   Interrupt Out endpoint is declared.
    /// - SET_IDLE (0x0A): Silence duplicate Input reports on the Interrupt In pipe.
    /// - SET_PROTOCOL (0x0B): Switch between boot and report protocol. Required only for devices
    ///   in the Boot Interface subclass.
    DECLARE_TYPED_ENUM(HIDClassRequest, U8, HID_CLASS_REQUESTS, 0x00) // NOLINT

    /// @brief bmRequestType of a device to host class request — HID 1.11 §7.2 (10100001): Get
    ///         Report, Get Idle and Get Protocol.
    static constexpr U8 HID_CLASS_REQUEST_TYPE_IN =
        static_cast<U8>(RequestType::DIRECTION_DEVICE_TO_HOST)
        | static_cast<U8>(RequestType::TYPE_CLASS)
        | static_cast<U8>(RequestType::RECIPIENT_INTERFACE);

    /// @brief bmRequestType of a host to device class request — HID 1.11 §7.2 (00100001): Set
    ///         Report, Set Idle and Set Protocol.
    static constexpr U8 HID_CLASS_REQUEST_TYPE_OUT =
        static_cast<U8>(RequestType::DIRECTION_HOST_TO_DEVICE)
        | static_cast<U8>(RequestType::TYPE_CLASS)
        | static_cast<U8>(RequestType::RECIPIENT_INTERFACE);

#define HID_REPORT_TYPES(X)                                                                        \
    X(HIDReportType, INPUT, 0x01)                                                                  \
    X(HIDReportType, OUTPUT, 0x02)                                                                 \
    X(HIDReportType, FEATURE, 0x03)

    /// @brief The Report Type in the high byte of wValue for Get_Report and Set_Report — HID 1.11
    ///         §7.2.1. 0x04-0xFF are reserved.
    ///
    /// - INPUT (0x01): Data flowing device to host, defined by Input Main items.
    /// - OUTPUT (0x02): Data flowing host to device, defined by Output Main items.
    /// - FEATURE (0x03): Configuration data, defined by Feature Main items.
    DECLARE_TYPED_ENUM(HIDReportType, U8, HID_REPORT_TYPES, 0x00) // NOLINT

    // ========================================================================================== //
    // Idle Rate — HID 1.11 §7.2.3, §7.2.4
    // ========================================================================================== //

    /// @brief One unit of the Duration in the high byte of wValue for Set_Idle, in milliseconds —
    ///         HID 1.11 §7.2.4.
    static constexpr U8 HID_IDLE_DURATION_UNIT_MS = 4;

    /// @brief Duration 0: report only when the report data changes, indefinitely — HID 1.11
    ///         §7.2.4.
    static constexpr U8 HID_IDLE_DURATION_INDEFINITE = 0;

    /// @brief Longest Duration Set_Idle can express, 255 * 4 ms — HID 1.11 §7.2.4. Together with
    ///         one unit this gives a range of 0.004 to 1.020 s, accurate to +/-(10% + 2 ms).
    static constexpr U16 HID_IDLE_DURATION_MAX_MS = 1020;

    /// @brief Report ID 0 in the low byte of wValue for Set_Idle and Get_Idle: the idle rate
    ///         applies to every Input report of the device — HID 1.11 §7.2.4.
    static constexpr U8 HID_IDLE_ALL_REPORTS = 0;

    /// @brief Recommended idle rate a keyboard is initialized to, the delay before the first key
    ///         repeat — HID 1.11 §7.2.4. Mice and joysticks default to indefinite instead.
    static constexpr U16 HID_IDLE_KEYBOARD_DEFAULT_MS = 500;

    /// @brief Compose wValue for Set_Idle — HID 1.11 §7.2.4.
    /// @param duration_ms Idle duration in milliseconds, rounded down to a multiple of
    ///                     HID_IDLE_DURATION_UNIT_MS and clamped to HID_IDLE_DURATION_MAX_MS.
    ///                     HID_IDLE_DURATION_INDEFINITE reports on change only. A duration below
    ///                     the endpoint's polling interval reports at the polling rate.
    /// @param report_id    Report the rate applies to; HID_IDLE_ALL_REPORTS for all of them.
    [[nodiscard]] constexpr auto hid_idle_request_value(U16 duration_ms,
                                                        U8  report_id = HID_IDLE_ALL_REPORTS)
        -> U16 {
        U16 clamped =
            duration_ms > HID_IDLE_DURATION_MAX_MS ? HID_IDLE_DURATION_MAX_MS : duration_ms;
        auto duration = static_cast<U8>(clamped / HID_IDLE_DURATION_UNIT_MS);
        return static_cast<U16>(static_cast<U16>(duration) << SHIFT_8 | report_id);
    }

    // ========================================================================================== //
    // Protocol — HID 1.11 §7.2.5, §7.2.6
    // ========================================================================================== //

#define HID_PROTOCOL_MODES(X)                                                                      \
    X(HIDProtocolMode, BOOT, 0x00)                                                                 \
    X(HIDProtocolMode, REPORT, 0x01)

    /// @brief wValue of Set_Protocol and the one data byte of Get_Protocol — HID 1.11 §7.2.6.
    ///
    /// Not to be confused with HIDProtocol in ClassCode.h, which is the bInterfaceProtocol of a
    /// Boot Interface subclass interface (keyboard or mouse).
    ///
    /// Devices come up in REPORT protocol, but the host must not assume that and should set the
    /// protocol explicitly while initializing a device.
    ///
    /// - BOOT (0x00): The fixed report format of the device's boot protocol, which needs no
    ///   Report descriptor parser.
    /// - REPORT (0x01): The report format the device's own Report descriptor declares.
    DECLARE_TYPED_ENUM(HIDProtocolMode, U8, HID_PROTOCOL_MODES, 0xFF) // NOLINT

    // ========================================================================================== //
    // HID Item Model
    // ========================================================================================== //

    /// @brief A pair of HIDReportType and report ID to uniquely identify a HID report.
    struct HIDReportTag {
        /// @brief Report ID 0 is reserved and must not be used — HID 1.11 §6.2.2.7.
        static constexpr U8 ID_NONE = 0;

        HIDReportType m_type      = HIDReportType::NONE;
        U8            m_report_ID = ID_NONE;

        /// @brief Compose wValue for Get_Report and Set_Report — HID 1.11 §7.2.1.
        [[nodiscard]] constexpr auto as_w_value() const -> U16;

        friend auto operator==(const HIDReportTag& lhs, const HIDReportTag& rhs) -> bool;
        friend auto operator!=(const HIDReportTag& lhs, const HIDReportTag& rhs) -> bool;
    };
} // namespace Rune::Device::USB

namespace Rune {
    /// @brief Hash support for HIDReportTag.
    ///
    /// Workaround because the "m_reports" member of HIDReports needs the hash support, so this
    /// must be defined before HIDReports, but Hash has the restriction that it needs to be put
    /// in the Rune namespace.
    ///
    /// Hence, the ugly split of the Rune::Device::USB namespace.
    template <>
    struct Hash<Device::USB::HIDReportTag> {
        auto operator()(const Device::USB::HIDReportTag& key) const -> size_t {
            Array<U8, 2> data{key.m_type.to_value(), key.m_report_ID};
            return FNV::do_hash(data.data(), 2);
        }
    };
} // namespace Rune

namespace Rune::Device::USB {

    /// @brief Extended Usage, HID 1.11 §6.2.2.8. High 16 bits encode the usage page, and the low 16
    ///         bit encode the usage.
    struct HIDExtendedUsage {

        U32 m_extended_usage = 0;

        [[nodiscard]] auto usage_page() const -> U16;

        [[nodiscard]] auto usage() const -> U16;

        [[nodiscard]] auto decode_usage_page() const -> HIDUsagePage;

        static auto from(U16 usage_page, U16 usage) -> HIDExtendedUsage;
    };

    /// @brief A range of extended usages also used to define a single extended usage if
    ///         m_min==m_max.
    struct HIDExtendedUsageRange {
        HIDExtendedUsage m_min;
        HIDExtendedUsage m_max;

        /// @brief
        /// @return Number of usages defined by the range.
        [[nodiscard]] auto count() const -> U32;
    };

    /// @brief One enclosing collection of a data item — HID 1.11 §6.2.2.6.
    struct HIDCollectionPathEntry {
        U8               m_type  = 0;
        HIDExtendedUsage m_usage = {};

        /// @brief
        /// @return The collection type decoded as enum.
        [[nodiscard]] auto type() const -> HIDCollectionType;
    };

    /// @brief Data of an Input, Output, or Feature report.
    struct HIDData {
        static constexpr char const* UNIT_NONE = "-";

        HIDDataFlags                       m_flags;
        LinkedList<HIDCollectionPathEntry> m_collection_path;
        LinkedList<HIDExtendedUsageRange>  m_usage_ranges;

        S32 m_logical_minimum = 0;
        S32 m_logical_maximum = 0;

        S32 m_physical_minimum = 0;
        S32 m_physical_maximum = 0;

        /// @brief The packed Unit item: nibble 0 is the unit system, nibbles 1-6 hold a 4-bit
        ///         signed exponent per base unit. HID 1.11 §6.2.2.7.
        U32 m_unit          = 0;
        S8  m_unit_exponent = 0;

        U32 m_report_size  = 0;
        U32 m_report_count = 0;

        U32 m_bit_offset = 0;

        /// @brief True: The data is part of an unknown vendor-defined collection type.
        ///        False: Otherwise
        bool m_is_vendor_defined_data = false;

        /// @brief
        /// @return True: The data item is padding.
        ///         False: Otherwise.
        [[nodiscard]] auto is_padding() const -> bool;

        /// @brief The unit decoded as a string of unit symbols e.g. m*s^-1.
        [[nodiscard]] auto unit_as_string() const -> String;
    };

    /// @brief A HID report defines the layout and usage of data sent to or received from a HID
    ///         device.
    struct HIDReport {
        HIDReportTag        m_tag;
        HIDExtendedUsage    m_usage;
        LinkedList<HIDData> m_data;
        U32                 m_bit_size = 0;
    };

    /// @brief A collection of all reports published by a HID device.
    struct HIDReports {
        /// @brief True: At least one report uses a report ID > 0, thus all reports must be prefixed
        ///                 by the 1-byte report ID.
        ///        False: All reports use report ID = 0, thus reports drop the 1-byte report ID
        ///                 prefix.
        bool                             m_uses_report_IDs = false;
        HashMap<HIDReportTag, HIDReport> m_reports;
    };
} // namespace Rune::Device::USB

namespace Rune {
    template <>
    struct StringRepresentation<Device::USB::HIDExtendedUsage> {
        static auto operator()(const Device::USB::HIDExtendedUsage& ext_usage) -> String {
            Device::USB::HIDUsagePage up(ext_usage.usage_page());
            String                    up_str = up.to_string();
            if (up_str == g_ENUM_NONE) up_str = String::format("{}", ext_usage.usage_page());
            return String::format("{}:{}",
                                  up_str,
                                  Device::USB::hid_decode_usage(up, ext_usage.usage()));
        }
    };

    template <>
    struct StringRepresentation<Device::USB::HIDExtendedUsageRange> {
        static auto operator()(const Device::USB::HIDExtendedUsageRange& range) -> String {
            if (range.count() > 1) {
                // The range decodes multiple values -> do not decode the usages because the string
                // may not make sense for a human-readable string e.g., The keyboard keycode range
                // EQUAL-KEYPAD_EIGHT does not convey useful information; 46-96 is more helpful in
                // this case
                Device::USB::HIDUsagePage up(range.m_min.usage_page());
                String                    up_str = up.to_string();
                if (up_str == g_ENUM_NONE) up_str = String::format("{}", range.m_min.usage_page());
                return String::format("{}:{}-{}", up_str, range.m_min.usage(), range.m_max.usage());
            }
            StringRepresentation<Device::USB::HIDExtendedUsage> ext_usage_str_repr;
            return ext_usage_str_repr(range.m_min);
        }
    };

    template <>
    struct StringRepresentation<Device::USB::HIDCollectionPathEntry> {
        static auto operator()(const Device::USB::HIDCollectionPathEntry& path) -> String {
            StringRepresentation<Device::USB::HIDExtendedUsage> ext_usage_str_repr;
            return String::format("{}:{}",
                                  path.type().to_string(),
                                  ext_usage_str_repr(path.m_usage));
        }
    };
} // namespace Rune

#endif // RUNEOS_HID_H