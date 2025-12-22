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

#include <Device/AHCI/GPT.h>

#include <KRE/BitsAndBytes.h>
#include <KRE/Utility.h>

#include <KRE/Memory.h>

namespace Rune::Device {
    const SharedPointer<Logger> LOGGER = LogContext::instance().get_logger("Device.GPT");

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      CRC32
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    auto reverse_bits(U32 data, U8 bit_count) -> U32 {
        U32 reflection = 0;
        for (U8 bit = 0; bit < bit_count; bit++) {
            if ((data & 0x01) != 0) {
                reflection |= (1 << ((bit_count - 1) - bit));
            }
            data = data >> 1;
        }
        return reflection;
    }

    // NOLINTBEGIN C-Style array is intentional as we can cast a GPTHeader instance to byte array,
    //  with the Array class we would need to memcpy in said array first
    auto compute_crc_32_checksum(const U8 data[], size_t size) -> U32 {
        U32           remainder  = MASK_DWORD;
        constexpr U32 POLYNOMIAL = 0x04C11DB7;
        constexpr U8  WIDTH      = 32;
        for (size_t i = 0; i < size; i++) {
            remainder ^= (reverse_bits(data[i], BIT_COUNT_BYTE) << (WIDTH - BIT_COUNT_BYTE));
            // Optimization: Use lookup table for all 256 possible byte values
            for (U8 bit = BIT_COUNT_BYTE; bit > 0; bit--) {
                if ((remainder & (1 << (WIDTH - 1))) != 0) {
                    remainder = (remainder << 1) ^ POLYNOMIAL;
                } else {
                    remainder = (remainder << 1);
                }
            }
        }
        return reverse_bits(remainder ^ MASK_DWORD, BIT_COUNT_DWORD);
    }

    auto verify_crc_32_checksum(const U8 data[], size_t size, U32 expected_crc_32) -> bool {
        return compute_crc_32_checksum(data, size) == expected_crc_32;
    }
    // NOLINTEND

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          GUID
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    auto GUID::to_string() -> String {
        // Node must be converted in two steps as it is a 6 byte value
        return String::format("{:0=8x}-{:0=4x}-{:0=4x}-{:0=4x}-{:0=8x}{:0=4x}",
                              LittleEndian::to_U32(buf.data()),
                              LittleEndian::to_U16(&buf[GUID::TIME_MID_OFFSET]),
                              LittleEndian::to_U16(&buf[GUID::TIME_HIGH_AND_VERSION_OFFSET]),
                              BigEndian::to_U16(&buf[GUID::CLOCK_SEQ_OFFSET]),
                              BigEndian::to_U32(&buf[GUID::NODE_OFFSET]),
                              BigEndian::to_U16(&buf[GUID::NODE_OFFSET + 4]));
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      GPTPartitionTableEntry
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    DEFINE_ENUM(GPTScanStatus, GPT_SCAN_STATUSES, 0x0)

    auto GPTPartitionTableEntry::get_name() -> String {
        Array<U8, GPTPartitionTableEntry::PARTITION_NAME_SIZE> buf{};
        for (U8 i = 0; i < GPTPartitionTableEntry::PARTITION_NAME_SIZE; i++) {
            buf[i] = name_buf[i] & MASK_BYTE;
            if (buf[i] == 0) break;
        }
        return {reinterpret_cast<const char*>(buf.data())};
    }

    // NOLINTBEGIN U8[] of sector_buf and sector_reader is required by PortEngine
    auto parse_header(Function<size_t(U8[], size_t, U64)>& sector_reader,
                      size_t                               sector_size,
                      U64                                  lba,
                      GPTHeader&                           out) -> GPTScanStatus {
        U8     sector_buf[sector_size];
        size_t read = sector_reader(sector_buf, forward<size_t>(sector_size), forward<U64>(lba));
        if (read != sector_size) return GPTScanStatus::STORAGE_DEV_ERROR;
        out.signature     = LittleEndian::to_U64(&sector_buf[GPTHeader::SIGNATURE_OFFSET]);
        out.revision      = LittleEndian::to_U32(&sector_buf[GPTHeader::REVISION_OFFSET]);
        out.header_size   = LittleEndian::to_U32(&sector_buf[GPTHeader::HEADER_SIZE_OFFSET]);
        out.header_crc_32 = LittleEndian::to_U32(&sector_buf[GPTHeader::HEADER_CRC32_OFFSET]);
        out.reserved[0]   = 0;
        out.reserved[1]   = 0;
        out.reserved[2]   = 0;
        out.reserved[3]   = 0;
        out.my_lba        = LittleEndian::to_U64(&sector_buf[GPTHeader::MY_LBA_OFFSET]);
        out.alternate_lba = LittleEndian::to_U64(&sector_buf[GPTHeader::ALTERNATE_LBA_OFFSET]);
        out.first_usable_lba =
            LittleEndian::to_U64(&sector_buf[GPTHeader::FIRST_USABLE_LBA_OFFSET]);
        out.last_usable_lba = LittleEndian::to_U64(&sector_buf[GPTHeader::LAST_USABLE_LBA_OFFSET]);
        memcpy(out.disk_guid.buf.data(),
               &sector_buf[GPTHeader::DISK_GUID_OFFSET],
               GPTHeader::DISK_GUID_SIZE);
        out.partition_entry_lba =
            LittleEndian::to_U64(&sector_buf[GPTHeader::PARTITION_ENTRY_LBA_OFFSET]);
        out.number_of_partition_entries =
            LittleEndian::to_U32(&sector_buf[GPTHeader::NUMBER_OF_PARTITION_ENTRIES_OFFSET]);
        out.size_of_partition_entry =
            LittleEndian::to_U32(&sector_buf[GPTHeader::SIZE_OF_PARTITION_ENTRY_OFFSET]);
        out.partition_entry_array_crc_32 =
            LittleEndian::to_U32(&sector_buf[GPTHeader::PARTITION_ENTRY_ARRAY_CRC32_OFFSET]);

        // 0x5452415020494645 -> ASCII: "EFI PART"
        if (out.signature != GPTHeader::SIGNATURE_HEX) return GPTScanStatus::NOT_DETECTED;

        U32 crc_back_up   = out.header_crc_32;
        out.header_crc_32 = 0;
        bool result =
            verify_crc_32_checksum(reinterpret_cast<U8*>(&out), out.header_size, crc_back_up);
        out.header_crc_32 = crc_back_up;
        return result ? GPTScanStatus::DETECTED : GPTScanStatus::CORRUPT_HEADER;
    }

    auto gpt_scan_device(Function<size_t(U8[], size_t, U64)>& sector_reader, size_t sector_size)
        -> GPTScanResult {

        GPTHeader     header;
        GPTScanStatus gpt_scan_status = parse_header(sector_reader, sector_size, 1, header);
        if (gpt_scan_status != GPTScanStatus::DETECTED) {
            if (gpt_scan_status != GPTScanStatus::NOT_DETECTED)
                LOGGER->warn("Failed to parse GPT header: {} (LBA 1)", gpt_scan_status.to_string());
            return {.status          = gpt_scan_status,
                    .header          = {},
                    .partition_table = LinkedList<GPTPartitionTableEntry>()};
        }

        // Read whole partition table
        auto buf_size = header.size_of_partition_entry * header.number_of_partition_entries;
        if (!memory_is_aligned(buf_size, sector_size))
            buf_size = memory_align(buf_size, sector_size, true);
        U8  partition_table_buf[buf_size]; // NOLINT size is dynamic -> cannot use Array
        U32 b_pos = 0;
        U64 c_lba = header.partition_entry_lba;
        while (b_pos < buf_size) {
            size_t bytes_read = sector_reader(&partition_table_buf[b_pos],
                                              forward<size_t>(sector_size),
                                              forward<U64>(c_lba));
            if (bytes_read != sector_size) {
                LOGGER->warn("Failed to read partition table entry at sector {}.", c_lba);
                return {.status          = GPTScanStatus::STORAGE_DEV_ERROR,
                        .header          = {},
                        .partition_table = LinkedList<GPTPartitionTableEntry>()};
            }
            b_pos += sector_size;
            c_lba++;
        }

        // Compute partition table CRC
        U32 p_t_crc_32 = compute_crc_32_checksum(partition_table_buf,
                                                 static_cast<size_t>(header.size_of_partition_entry)
                                                     * header.number_of_partition_entries);
        if (p_t_crc_32 != header.partition_entry_array_crc_32) {
            LOGGER->warn("Wrong partition table CRC detected.");
            return {.status          = GPTScanStatus::CORRUPT_PARTITION_TABLE,
                    .header          = {},
                    .partition_table = LinkedList<GPTPartitionTableEntry>()};
        }

        GPTHeader back_up_header;
        if (header.my_lba == 1) {
            // Check the backup header
            GPTScanStatus bu_gpt_scan_status =
                parse_header(sector_reader, sector_size, header.alternate_lba, back_up_header);
            if (bu_gpt_scan_status != GPTScanStatus::DETECTED) {
                if (bu_gpt_scan_status != GPTScanStatus::NOT_DETECTED)
                    LOGGER->warn("Failed to parse backup GPT header: {} (LBA 1)",
                                 bu_gpt_scan_status.to_string());
                return {.status          = bu_gpt_scan_status,
                        .header          = {},
                        .partition_table = LinkedList<GPTPartitionTableEntry>()};
            }
        }

        // Parse partition table
        Array<U8, GUID::SIZE> zeroes{};
        memset(zeroes.data(), 0, GUID::SIZE);
        LinkedList<GPTPartitionTableEntry> p_e_table;
        U32                                p_e_buf_idx = 0;
        while (p_e_buf_idx < header.number_of_partition_entries) {
            size_t byte_offset = p_e_buf_idx * sizeof(GPTPartitionTableEntry);
            if (memcmp(zeroes.data(), &partition_table_buf[byte_offset], GUID::SIZE) == 0) {
                // Unused entry
                p_e_buf_idx++;
                continue;
            }
            GPTPartitionTableEntry pt_e;
            size_t                 pte_offset = 0;
            memcpy(pt_e.partition_type_guid.buf.data(),
                   &partition_table_buf[byte_offset],
                   GUID::SIZE);

            pte_offset += GUID::SIZE;
            memcpy(pt_e.unique_partition_guid.buf.data(),
                   &partition_table_buf[byte_offset + pte_offset],
                   GUID::SIZE);

            pte_offset += GUID::SIZE;
            pt_e.starting_lba =
                LittleEndian::to_U64(&partition_table_buf[byte_offset + pte_offset]);

            pte_offset      += GPTPartitionTableEntry::LBA_AND_ATTRIBUTES_SIZE;
            pt_e.ending_lba  = LittleEndian::to_U64(&partition_table_buf[byte_offset + pte_offset]);

            pte_offset      += GPTPartitionTableEntry::LBA_AND_ATTRIBUTES_SIZE;
            pt_e.attributes  = LittleEndian::to_U64(&partition_table_buf[byte_offset + pte_offset]);

            pte_offset += GPTPartitionTableEntry::LBA_AND_ATTRIBUTES_SIZE;
            memcpy(pt_e.name_buf.data(),
                   &partition_table_buf[byte_offset + pte_offset],
                   static_cast<size_t>(GPTPartitionTableEntry::PARTITION_NAME_SIZE) * 2);

            p_e_table.add_back(pt_e);
            p_e_buf_idx++;
        }

        return {.status = GPTScanStatus::DETECTED, .header = header, .partition_table = p_e_table};
    }
    // NOLINTEND

} // namespace Rune::Device