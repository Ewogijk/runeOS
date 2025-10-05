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

#ifndef RUNEOS_GPT_H
#define RUNEOS_GPT_H

#include <KRE/String.h>

#include <Ember/Ember.h>
#include <Ember/Enum.h>

#include <KRE/Logging.h>

namespace Rune::Device {
    /**
     * @brief GUID as defined in RFC 4122.
     */
    struct GUID {
        static constexpr U8 SIZE = 16;

        U8 buf[SIZE];

        /**
         * @brief Print string representation the GUID
         * @return
         */
        String to_string();
    };

    /**
     *
     * Source: <a
     * href="https://uefi.org/specs/UEFI/2.10/05_GUID_Partition_Table_Format.html#gpt-header">
     * UEFI 2.10 - GPT Header</a>
     * @brief The GPT header contains information about all partitions on a drive.
     */
    struct GPTHeader {
        U64  signature        = 0;
        U32  revision         = 0;
        U32  header_size      = 0;
        U32  header_crc_32    = 0;
        U8   reserved[4]      = {};
        U64  my_lba           = 0;
        U64  alternate_lba    = 0;
        U64  first_usable_lba = 0; // Little-endian
        U64  last_usable_lba  = 0; // Little-endian
        GUID disk_guid;
        U64  partition_entry_lba          = 0; // Little-endian
        U32  number_of_partition_entries  = 0;
        U32  size_of_partition_entry      = 0;
        U32  partition_entry_array_crc_32 = 0;
    };

    /**
     * Source: <a
     * href="https://uefi.org/specs/UEFI/2.10/05_GUID_Partition_Table_Format.html#gpt-partition-entry-array">
     * UEFI 2.10 - GPT Partition Entry Array</a>
     * @brief An entry in the GPT partition table represents a single partition on a drive.
     */
    struct GPTPartitionTableEntry {
        GUID partition_type_guid;
        GUID unique_partition_guid;
        U64  starting_lba = 0; // Little-endian
        U64  ending_lba   = 0; // Inclusive, Little-endian
        U64  attributes   = 0;
        U16  name_buf[36] = {}; // UTF-16LE

        /**
         * @brief Note: Only ASCII characters are supported.
         * @return Name of the partition.
         */
        String get_name();
    };

#define GPT_SCAN_STATUSES(X)                                                                       \
    X(GPTScanStatus, DETECTED, 0x1)                                                                \
    X(GPTScanStatus, NOT_DETECTED, 0x2)                                                            \
    X(GPTScanStatus, CORRUPT_HEADER, 0x3)                                                          \
    X(GPTScanStatus, CORRUPT_PARTITION_TABLE, 0x4)                                                 \
    X(GPTScanStatus, STORAGE_DEV_ERROR, 0x5)

    /**
     * <ul>
     *  <li>Detected: A GPT was found.</li>
     *  <li>NotDetected: No GPT was found.</li>
     *  <li>CorruptHeader: The CRC of the header is wrong.</li>
     *  <li>CorruptPartitionTable: The CRC of the partition table is wrong.</li>
     *  <li>StorageDevError: The storage device had an error.</li>
     * </ul>
     *
     * @brief Final status after a drive was scanned for a GPT.
     */
    DECLARE_ENUM(GPTScanStatus, GPT_SCAN_STATUSES, 0x0) // NOLINT

    /**
     * If Status == GPTScanStatus::Detected the Header and PartitionTable variables contain valid
     * data else they should be ignored.
     *
     * @brief End result of trying to detect a GPT on a storage device.
     */
    struct GPTScanResult {
        GPTScanStatus                      status;
        GPTHeader                          header;
        LinkedList<GPTPartitionTableEntry> partition_table;
    };

    auto gpt_scan_device(Function<size_t(U8[], size_t, U64)>& sector_reader, size_t sector_size)
        -> GPTScanResult;
} // namespace Rune::Device

#endif // RUNEOS_GPT_H
