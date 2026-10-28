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

#ifndef RUNEOS_GPT_H
#define RUNEOS_GPT_H

#include <KRE/String.h>

#include <Ember/Ember.h>
#include <Ember/Enum.h>

#include <KRE/Collections/Array.h>
#include <KRE/Logging.h>

namespace Rune::Device {
    /// @brief GUID as defined in RFC 4122.
    struct GUID {
        static constexpr char const* UNUSED_ENTRY         = "00000000-0000-0000-0000-000000000000";
        static constexpr char const* EFI_SYSTEM_PARTITION = "C12A7328-F81F-11D2-BA4B-00A0C93EC93B";

        static constexpr U8 SIZE = 16;

        static constexpr U8 TIME_LOW_OFFSET              = 0;
        static constexpr U8 TIME_MID_OFFSET              = 4;
        static constexpr U8 TIME_HIGH_AND_VERSION_OFFSET = 6;
        static constexpr U8 CLOCK_SEQ_OFFSET             = 8;
        static constexpr U8 NODE_OFFSET                  = 10;

        Array<U8, SIZE> m_buf;

        /// @brief Parse a GUID from its string representation, e.g.
        ///         "8fa4455d-2d55-45ba-8bca-cbcedf48bdf6".
        ///
        /// @param guid String representation of a GUID.
        /// @return The parsed GUID or an empty optional if the string is malformed.
        static auto from_string(const String& guid) -> Optional<GUID>;

        /// @brief
        /// @return The string representation of the GUID.
        auto to_string() -> String;
    };

    /// @brief The GPT header contains information about all partitions on a drive.
    /// Source: <a
    /// href="https://uefi.org/specs/UEFI/2.10/05_GUID_Partition_Table_Format.html#gpt-header">
    /// UEFI 2.10 - GPT Header</a>
    struct GPTHeader {
        /// @brief "EFI PART" signature in ASCII hex representation.
        static constexpr U64 SIGNATURE_HEX                      = 0x5452415020494645;
        static constexpr U8  SIGNATURE_OFFSET                   = 0;
        static constexpr U8  REVISION_OFFSET                    = 8;
        static constexpr U8  HEADER_SIZE_OFFSET                 = 12;
        static constexpr U8  HEADER_CRC32_OFFSET                = 16;
        static constexpr U8  RESERVED_OFFSET                    = 20;
        static constexpr U8  MY_LBA_OFFSET                      = 24;
        static constexpr U8  ALTERNATE_LBA_OFFSET               = 32;
        static constexpr U8  FIRST_USABLE_LBA_OFFSET            = 40;
        static constexpr U8  LAST_USABLE_LBA_OFFSET             = 48;
        static constexpr U8  DISK_GUID_OFFSET                   = 56;
        static constexpr U8  PARTITION_ENTRY_LBA_OFFSET         = 72;
        static constexpr U8  NUMBER_OF_PARTITION_ENTRIES_OFFSET = 80;
        static constexpr U8  SIZE_OF_PARTITION_ENTRY_OFFSET     = 84;
        static constexpr U8  PARTITION_ENTRY_ARRAY_CRC32_OFFSET = 88;
        static constexpr U8  DISK_GUID_SIZE                     = 16;

        U64          m_signature     = 0;
        U32          m_revision      = 0;
        U32          m_header_size   = 0;
        U32          m_header_crc_32 = 0;
        Array<U8, 4> m_reserved{};
        U64          m_my_lba           = 0;
        U64          m_alternate_lba    = 0;
        U64          m_first_usable_lba = 0; // Little-endian
        U64          m_last_usable_lba  = 0; // Little-endian
        GUID         m_disk_guid{};
        U64          m_partition_entry_lba          = 0; // Little-endian
        U32          m_number_of_partition_entries  = 0;
        U32          m_size_of_partition_entry      = 0;
        U32          m_partition_entry_array_crc_32 = 0;
    };

    /// @brief An entry in the GPT partition table represents a single partition on a drive.
    /// Source: <a
    /// href="https://uefi.org/specs/UEFI/2.10/05_GUID_Partition_Table_Format.html#gpt-partition-entry-array">
    /// UEFI 2.10 - GPT Partition Entry Array</a>
    struct GPTPartitionTableEntry {

        static constexpr U8 PARTITION_NAME_SIZE     = 36;
        static constexpr U8 LBA_AND_ATTRIBUTES_SIZE = 8;

        static constexpr U8 PARTITION_TYPE_GUID_OFFSET   = 0;
        static constexpr U8 UNIQUE_PARTITION_GUID_OFFSET = 16;
        static constexpr U8 FIRST_LBA_OFFSET             = 32;
        static constexpr U8 LAST_LBA_OFFSET              = 40;
        static constexpr U8 ATTRIBUTES_OFFSET            = 48;
        static constexpr U8 PARTITION_NAME_OFFSET        = 56;

        GUID   m_partition_type_guid{};
        GUID   m_unique_partition_guid{};
        U64    m_starting_lba = 0; // Little-endian
        U64    m_ending_lba   = 0; // Inclusive, Little-endian
        U64    m_attributes   = 0;
        String m_name;
    };

#define GPT_SCAN_STATUSES(X)                                                                       \
    X(NOT_DETECTED, 0x1)                                                                           \
    X(CORRUPT_HEADER, 0x2)                                                                         \
    X(CORRUPT_PARTITION_TABLE, 0x3)                                                                \
    X(STORAGE_DEV_ERROR, 0x4)

    /// @brief Possible errors when parsing a GPT.
    ///
    /// - NOT_DETECTED: No GPT was found.
    /// - CORRUPT_HEADER: The CRC of the header is wrong.
    /// - CORRUPT_PARTITION_TABLE: The CRC of the partition table is wrong.
    /// - STORAGE_DEV_ERROR: The storage device had an error.
    EMBER_DEFINE_SMART_ENUM(GPTError, GPT_SCAN_STATUSES, 0x0) // NOLINT

    /// @brief End result of trying to detect a GPT on a storage device.
    ///
    /// If Status == GPTScanStatus::Detected the Header and PartitionTable variables contain valid
    /// data else they should be ignored.
    struct GPT {
        GPTHeader                          m_header;
        LinkedList<GPTPartitionTableEntry> m_partition_table;
    };

    // NOLINTBEGIN Array size is dynamic
    /// @brief Scan a mass storage device for a GPT.
    /// @param sector_reader Function to read a specific amount of bytes at an LBA.
    /// @param sector_size Size of a sector.
    /// @return No error: The GPT header and partition table.
    ///         Otherwise: An error code with the reason of failure.
    auto gpt_scan_device(Function<size_t(U8[], size_t, U64)>& sector_reader, size_t sector_size)
        -> Expected<GPT, GPTError>;
    // NOLINTEND
} // namespace Rune::Device

#endif // RUNEOS_GPT_H
