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

#include <../../../../Include/Device/MassStorage/GPT.h>

#include <KRE/BitsAndBytes.h>
#include <KRE/Utility.h>

#include <KRE/Memory.h>

namespace Rune::Device {
    // ========================================================================================== //
    // CRC32
    // ========================================================================================== //

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

    // ========================================================================================== //
    // GUID
    // ========================================================================================== //

    auto GUID::from_string(const String& guid) -> Optional<GUID> {
        constexpr U8 GUID_STR_SIZE = 36;
        if (guid.size() != GUID_STR_SIZE) return Optional<GUID>::empty();
        // NOLINTBEGIN cppcoreguidelines-avoid-magic-numbers
        //  Expected format: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
        for (size_t i = 0; i < GUID_STR_SIZE; i++) {
            char ch      = guid[i];
            bool is_dash = i == 8 || i == 13 || i == 18 || i == 23;
            if (is_dash) {
                if (ch != '-') return Optional<GUID>::empty();
            } else if (('0' > ch || ch > '9') && ('a' > ch || ch > 'f') && ('A' > ch || ch > 'F')) {
                return Optional<GUID>::empty();
            }
        }

        U32 time_low              = 0;
        U16 time_mid              = 0;
        U16 time_high_and_version = 0;
        U16 clock_seq             = 0;
        // Node must be converted in two steps as it is a 6 byte value
        U16 node_high = 0;
        U32 node_low  = 0;
        if (!parse_int(guid.substring(0, 8), Radix::HEX, time_low)
            || !parse_int(guid.substring(9, 4), Radix::HEX, time_mid)
            || !parse_int(guid.substring(14, 4), Radix::HEX, time_high_and_version)
            || !parse_int(guid.substring(19, 4), Radix::HEX, clock_seq)
            || !parse_int(guid.substring(24, 4), Radix::HEX, node_high)
            || !parse_int(guid.substring(28, 8), Radix::HEX, node_low))
            return Optional<GUID>::empty();
        // NOLINTEND

        GUID out{};
        LittleEndian::to_bytes(time_low, &out.m_buf[TIME_LOW_OFFSET]);
        LittleEndian::to_bytes(time_mid, &out.m_buf[TIME_MID_OFFSET]);
        LittleEndian::to_bytes(time_high_and_version, &out.m_buf[TIME_HIGH_AND_VERSION_OFFSET]);
        BigEndian::to_bytes(clock_seq, &out.m_buf[CLOCK_SEQ_OFFSET]);
        BigEndian::to_bytes(node_high, &out.m_buf[NODE_OFFSET]);
        BigEndian::to_bytes(node_low, &out.m_buf[NODE_OFFSET + 2]);
        return make_optional(move(out));
    }

    auto GUID::to_string() -> String {
        // Node must be converted in two steps as it is a 6 byte value
        return String::format("{:0=8x}-{:0=4x}-{:0=4x}-{:0=4x}-{:0=8x}{:0=4x}",
                              LittleEndian::to_U32(m_buf.data()),
                              LittleEndian::to_U16(&m_buf[GUID::TIME_MID_OFFSET]),
                              LittleEndian::to_U16(&m_buf[GUID::TIME_HIGH_AND_VERSION_OFFSET]),
                              BigEndian::to_U16(&m_buf[GUID::CLOCK_SEQ_OFFSET]),
                              BigEndian::to_U32(&m_buf[GUID::NODE_OFFSET]),
                              BigEndian::to_U16(&m_buf[GUID::NODE_OFFSET + 4]));
    }

    // ========================================================================================== //
    // GPTPartitionTableEntry
    // ========================================================================================== //

    auto decode_gpt_string(Array<U16, GPTPartitionTableEntry::PARTITION_NAME_SIZE> name_buf)
        -> String {
        // name_buf is UTF-16LE encoded
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
                      GPTHeader&                           out) -> GPTError {
        U8     sector_buf[sector_size];
        size_t read = sector_reader(sector_buf, forward<size_t>(sector_size), forward<U64>(lba));
        if (read != sector_size) return GPTError::STORAGE_DEV_ERROR;
        out.m_signature     = LittleEndian::to_U64(&sector_buf[GPTHeader::SIGNATURE_OFFSET]);
        out.m_revision      = LittleEndian::to_U32(&sector_buf[GPTHeader::REVISION_OFFSET]);
        out.m_header_size   = LittleEndian::to_U32(&sector_buf[GPTHeader::HEADER_SIZE_OFFSET]);
        out.m_header_crc_32 = LittleEndian::to_U32(&sector_buf[GPTHeader::HEADER_CRC32_OFFSET]);
        out.m_reserved[0]   = 0;
        out.m_reserved[1]   = 0;
        out.m_reserved[2]   = 0;
        out.m_reserved[3]   = 0;
        out.m_my_lba        = LittleEndian::to_U64(&sector_buf[GPTHeader::MY_LBA_OFFSET]);
        out.m_alternate_lba = LittleEndian::to_U64(&sector_buf[GPTHeader::ALTERNATE_LBA_OFFSET]);
        out.m_first_usable_lba =
            LittleEndian::to_U64(&sector_buf[GPTHeader::FIRST_USABLE_LBA_OFFSET]);
        out.m_last_usable_lba =
            LittleEndian::to_U64(&sector_buf[GPTHeader::LAST_USABLE_LBA_OFFSET]);
        memcpy(out.m_disk_guid.m_buf.data(),
               &sector_buf[GPTHeader::DISK_GUID_OFFSET],
               GPTHeader::DISK_GUID_SIZE);
        out.m_partition_entry_lba =
            LittleEndian::to_U64(&sector_buf[GPTHeader::PARTITION_ENTRY_LBA_OFFSET]);
        out.m_number_of_partition_entries =
            LittleEndian::to_U32(&sector_buf[GPTHeader::NUMBER_OF_PARTITION_ENTRIES_OFFSET]);
        out.m_size_of_partition_entry =
            LittleEndian::to_U32(&sector_buf[GPTHeader::SIZE_OF_PARTITION_ENTRY_OFFSET]);
        out.m_partition_entry_array_crc_32 =
            LittleEndian::to_U32(&sector_buf[GPTHeader::PARTITION_ENTRY_ARRAY_CRC32_OFFSET]);

        // 0x5452415020494645 -> ASCII: "EFI PART"
        if (out.m_signature != GPTHeader::SIGNATURE_HEX) return GPTError::NOT_DETECTED;

        U32 crc_back_up     = out.m_header_crc_32;
        out.m_header_crc_32 = 0;
        bool result =
            verify_crc_32_checksum(reinterpret_cast<U8*>(&out), out.m_header_size, crc_back_up);
        out.m_header_crc_32 = crc_back_up;
        return result ? GPTError::NONE : GPTError::CORRUPT_HEADER;
    }

    auto gpt_scan_device(Function<size_t(U8[], size_t, U64)>& sector_reader, size_t sector_size)
        -> Expected<GPT, GPTError> {

        GPTHeader header;
        auto      gpt_error = parse_header(sector_reader, sector_size, 1, header);
        if (gpt_error != GPTError::NONE) {
            if (gpt_error != GPTError::NOT_DETECTED)
                WARN("Failed to parse GPT header: {} (LBA 1)", gpt_error.to_string());

            return Unexpected<GPTError>(move(gpt_error));
        }

        // Read whole partition table
        auto buf_size = header.m_size_of_partition_entry * header.m_number_of_partition_entries;
        if (!memory_is_aligned(buf_size, sector_size))
            buf_size = memory_align(buf_size, sector_size, true);

        /// buf_size can be huge (~16KiB), so it is best to heap allocate the buffer
        auto partition_table_buf = UniquePointer<U8>(new U8[buf_size]);
        U32  b_pos               = 0;
        U64  c_lba               = header.m_partition_entry_lba;
        while (b_pos < buf_size) {
            size_t bytes_read = sector_reader(&partition_table_buf.get()[b_pos],
                                              forward<size_t>(sector_size),
                                              forward<U64>(c_lba));
            if (bytes_read != sector_size) {
                WARN("Failed to read partition table entry at sector {}.", c_lba);
                return Unexpected<GPTError>(move(GPTError::STORAGE_DEV_ERROR));
            }
            b_pos += sector_size;
            c_lba++;
        }

        // Compute partition table CRC
        U32 p_t_crc_32 =
            compute_crc_32_checksum(partition_table_buf.get(),
                                    static_cast<size_t>(header.m_size_of_partition_entry)
                                        * header.m_number_of_partition_entries);
        if (p_t_crc_32 != header.m_partition_entry_array_crc_32) {
            WARN("Wrong partition table CRC detected.");
            return Unexpected<GPTError>(move(GPTError::CORRUPT_PARTITION_TABLE));
        }

        GPTHeader back_up_header;
        if (header.m_my_lba == 1) {
            // Check the backup header
            GPTError bu_gpt_error =
                parse_header(sector_reader, sector_size, header.m_alternate_lba, back_up_header);
            if (bu_gpt_error != GPTError::NONE) {
                if (bu_gpt_error != GPTError::NOT_DETECTED)
                    WARN("Failed to parse backup GPT header: {} (LBA 1)", bu_gpt_error.to_string());
                return Unexpected<GPTError>(move(move(bu_gpt_error)));
            }
        }

        // Parse partition table
        Array<U8, GUID::SIZE> zeroes{};
        memset(zeroes.data(), 0, GUID::SIZE);
        LinkedList<GPTPartitionTableEntry> p_e_table;
        U32                                p_e_buf_idx = 0;
        while (p_e_buf_idx < header.m_number_of_partition_entries) {
            size_t byte_offset = p_e_buf_idx * header.m_size_of_partition_entry;
            if (memcmp(zeroes.data(), &partition_table_buf.get()[byte_offset], GUID::SIZE) == 0) {
                // Unused entry
                p_e_buf_idx++;
                continue;
            }
            GPTPartitionTableEntry pt_e;
            size_t                 pte_offset = 0;
            memcpy(pt_e.m_partition_type_guid.m_buf.data(),
                   &partition_table_buf.get()[byte_offset],
                   GUID::SIZE);

            pte_offset += GUID::SIZE;
            memcpy(pt_e.m_unique_partition_guid.m_buf.data(),
                   &partition_table_buf.get()[byte_offset + pte_offset],
                   GUID::SIZE);

            pte_offset += GUID::SIZE;
            pt_e.m_starting_lba =
                LittleEndian::to_U64(&partition_table_buf.get()[byte_offset + pte_offset]);

            pte_offset += GPTPartitionTableEntry::LBA_AND_ATTRIBUTES_SIZE;
            pt_e.m_ending_lba =
                LittleEndian::to_U64(&partition_table_buf.get()[byte_offset + pte_offset]);

            pte_offset += GPTPartitionTableEntry::LBA_AND_ATTRIBUTES_SIZE;
            pt_e.m_attributes =
                LittleEndian::to_U64(&partition_table_buf.get()[byte_offset + pte_offset]);

            pte_offset += GPTPartitionTableEntry::LBA_AND_ATTRIBUTES_SIZE;
            Array<U16, GPTPartitionTableEntry::PARTITION_NAME_SIZE> name_buf;
            memcpy(name_buf.data(),
                   &partition_table_buf.get()[byte_offset + pte_offset],
                   static_cast<size_t>(GPTPartitionTableEntry::PARTITION_NAME_SIZE) * 2);
            pt_e.m_name = decode_gpt_string(name_buf);

            p_e_table.add_back(pt_e);
            p_e_buf_idx++;
        }

        GPT gpt = {.m_header = header, .m_partition_table = p_e_table};
        return {gpt};
    }
    // NOLINTEND

} // namespace Rune::Device