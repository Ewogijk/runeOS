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

#include <KernelRuntime/ByteOrder.h>

#include <KernelRuntime/Memory.h>

namespace Rune::Device {
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          CRC32 Implementation
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    U32 reverse_bits(U32 data, U8 bit_count) {
        U32 reflection = 0;
        for (U8 bit = 0; bit < bit_count; bit++) {
            if (data & 0x01) {
                reflection |= (1 << ((bit_count - 1) - bit));
            }
            data = data >> 1;
        }
        return reflection;
    }

    U32 compute_crc_32_checksum(const U8 data[], size_t size) {
        U32 remainder  = 0xFFFFFFFF;
        U32 polynomial = 0x04C11DB7;
        U8  width      = 32;
        for (size_t i = 0; i < size; i++) {
            remainder ^= (reverse_bits(data[i], 8) << (width - 8));
            // Optimization: Use lookup table for all 256 possible byte values
            for (U8 bit = 8; bit > 0; bit--) {
                if (remainder & (1 << (width - 1))) {
                    remainder = (remainder << 1) ^ polynomial;
                } else {
                    remainder = (remainder << 1);
                }
            }
        }
        return reverse_bits(remainder ^ 0xFFFFFFFF, 32);
    }

    bool verify_crc_32_checksum(const U8 data[], size_t size, U32 expected_crc_32) {
        return compute_crc_32_checksum(data, size) == expected_crc_32;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          GUID Impl
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    String GUID::to_string() {
        return String::format("{:0=8x}-{:0=4x}-{:0=4x}-{:0=4x}-{:0=8x}{:0=4x}",
                              LittleEndian::to_U32(buf),
                              LittleEndian::to_U16(&buf[4]),
                              LittleEndian::to_U16(&buf[6]),
                              BigEndian::to_U16(&buf[8]),
                              BigEndian::to_U32(&buf[10]),
                              BigEndian::to_U16(&buf[14]));
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      GPTPartitionTableEntry Impl
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    DEFINE_ENUM(GPTScanStatus, GPT_SCAN_STATUSES, 0x0)

    String GPTPartitionTableEntry::get_name() {
        U8 buf[18];
        for (int i = 0; i < 18; i++) {
            buf[i] = name_buf[i] & 0x00FF;
            if (buf[i] == 0) break;
        }
        return {(const char*) buf};
    }

    GPTScanStatus
    parse_header(Function<size_t(U8[], size_t, U64)>& sector_reader, size_t sector_size, U64 lba, GPTHeader& out) {
        U8     sector_buf[sector_size];
        size_t read = sector_reader(sector_buf, forward<size_t>(sector_size), forward<U64>(lba));
        if (read != sector_size) return GPTScanStatus::STORAGE_DEV_ERROR;
        out.signature        = LittleEndian::to_U64(sector_buf);
        out.revision         = LittleEndian::to_U32(&sector_buf[8]);
        out.header_size      = LittleEndian::to_U32(&sector_buf[12]);
        out.header_crc_32    = LittleEndian::to_U32(&sector_buf[16]);
        out.reserved[0]      = 0;
        out.reserved[1]      = 0;
        out.reserved[2]      = 0;
        out.reserved[3]      = 0;
        out.my_lba           = LittleEndian::to_U64(&sector_buf[24]);
        out.alternate_lba    = LittleEndian::to_U64(&sector_buf[32]);
        out.first_usable_lba = LittleEndian::to_U64(&sector_buf[40]);
        out.last_usable_lba  = LittleEndian::to_U64(&sector_buf[48]);
        memcpy(out.disk_guid.buf, &sector_buf[56], 16);
        out.partition_entry_lba          = LittleEndian::to_U64(&sector_buf[72]);
        out.number_of_partition_entries  = LittleEndian::to_U32(&sector_buf[80]);
        out.size_of_partition_entry      = LittleEndian::to_U32(&sector_buf[84]);
        out.partition_entry_array_crc_32 = LittleEndian::to_U32(&sector_buf[88]);

        // 0x5452415020494645 -> ASCII: "EFI PART"
        if (out.signature != 0x5452415020494645) return GPTScanStatus::NOT_DETECTED;

        U32 crc_back_up   = out.header_crc_32;
        out.header_crc_32 = 0;
        bool result       = verify_crc_32_checksum((U8*) &out, out.header_size, crc_back_up);
        out.header_crc_32 = crc_back_up;
        return result ? GPTScanStatus::DETECTED : GPTScanStatus::CORRUPT_HEADER;
    }

    GPTScanResult gpt_scan_device(const SharedPointer<Logger>&         logger,
                                  Function<size_t(U8[], size_t, U64)>& sector_reader,
                                  size_t                               sector_size) {

        GPTHeader     header;
        GPTScanStatus gpt_scan_status = parse_header(sector_reader, sector_size, 1, header);
        if (gpt_scan_status != GPTScanStatus::DETECTED) {
            if (gpt_scan_status != GPTScanStatus::NOT_DETECTED)
                logger->warn("GPT", "Failed to parse GPT header: {} (LBA 1)", gpt_scan_status.to_string());
            return {gpt_scan_status, {}, LinkedList<GPTPartitionTableEntry>()};
        }

        // Read whole partition table
        size_t buf_size = header.size_of_partition_entry * header.number_of_partition_entries;
        if (!memory_is_aligned(buf_size, sector_size)) buf_size = memory_align(buf_size, sector_size, true);
        U8  partition_table_buf[buf_size];
        U32 b_pos = 0;
        U64 c_lba = header.partition_entry_lba;
        while (b_pos < buf_size) {
            size_t bytes_read =
                sector_reader(&partition_table_buf[b_pos], forward<size_t>(sector_size), forward<U64>(c_lba));
            if (bytes_read != sector_size) {
                logger->warn("GPT", "Failed to read partition table entry at sector {}.", c_lba);
                return {GPTScanStatus::STORAGE_DEV_ERROR, {}, LinkedList<GPTPartitionTableEntry>()};
            }
            b_pos += sector_size;
            c_lba++;
        }

        // Compute partition table CRC
        U32 p_t_crc_32 = compute_crc_32_checksum(partition_table_buf,
                                                 header.size_of_partition_entry * header.number_of_partition_entries);
        if (p_t_crc_32 != header.partition_entry_array_crc_32) {
            logger->warn("GPT", "Wrong partition table CRC detected.");
            return {GPTScanStatus::CORRUPT_PARTITION_TABLE, {}, LinkedList<GPTPartitionTableEntry>()};
        }

        GPTHeader back_up_header;
        if (header.my_lba == 1) {
            // Check the backup header
            GPTScanStatus bu_gpt_scan_status =
                parse_header(sector_reader, sector_size, header.alternate_lba, back_up_header);
            if (bu_gpt_scan_status != GPTScanStatus::DETECTED) {
                if (bu_gpt_scan_status != GPTScanStatus::NOT_DETECTED)
                    logger->warn("GPT",
                                 "Failed to parse backup GPT header: {} (LBA 1)",
                                 bu_gpt_scan_status.to_string());
                return {bu_gpt_scan_status, {}, LinkedList<GPTPartitionTableEntry>()};
            }
        }

        // Parse partition table
        U8 zeroes[16];
        memset(zeroes, 0, 16);
        LinkedList<GPTPartitionTableEntry> p_e_table;
        U32                                p_e_buf_idx = 0;
        while (p_e_buf_idx < header.number_of_partition_entries) {
            size_t byte_offset = p_e_buf_idx * sizeof(GPTPartitionTableEntry);
            if (memcmp(zeroes, &partition_table_buf[byte_offset], 16) == 0) {
                // Unused entry
                p_e_buf_idx++;
                continue;
            }
            GPTPartitionTableEntry pt_e;
            memcpy(pt_e.partition_type_guid.buf, &partition_table_buf[byte_offset], 16);
            memcpy(pt_e.unique_partition_guid.buf, &partition_table_buf[byte_offset + 16], 16);
            pt_e.starting_lba = LittleEndian::to_U64(&partition_table_buf[byte_offset + 32]);
            pt_e.ending_lba   = LittleEndian::to_U64(&partition_table_buf[byte_offset + 40]);
            pt_e.attributes   = LittleEndian::to_U64(&partition_table_buf[byte_offset + 48]);
            memcpy(pt_e.name_buf, &partition_table_buf[byte_offset + 56], 72);
            p_e_table.add_back(pt_e);

            p_e_buf_idx++;
        }

        return {GPTScanStatus::DETECTED, header, p_e_table};
    }

} // namespace Rune::Device