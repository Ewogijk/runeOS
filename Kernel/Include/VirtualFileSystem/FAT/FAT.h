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

#ifndef RUNEOS_FAT_H
#define RUNEOS_FAT_H

#include <Ember/Ember.h>
#include <Ember/Enum.h>

#include <KRE/String.h>

namespace Rune::VFS {

    //////////////////////////////////////////////////////////////////////////////////
    //                                                                              //
    // FAT specification structures - Microsoft FAT Specification 30.08.2005        //
    //                                                                              //
    //////////////////////////////////////////////////////////////////////////////////

#define FAT_TYPES(X)                                                                               \
    X(FATType, FAT12, 0x1)                                                                         \
    X(FATType, FAT16, 0x2)                                                                         \
    X(FATType, FAT32, 0x3)

    DECLARE_ENUM(FATType, FAT_TYPES, 0x0) // NOLINT

    struct BIOSParameterBlock {
        static constexpr U8     ROOT_ENTRY_COUNT_FACTOR = 32;
        static constexpr size_t JMPBOOT_SIZE            = 3;
        static constexpr size_t OEMID_SIZE              = 8;
        static const char*      RUNEOS_OEM;

        static constexpr U8 JMPBOOT0 = 0xEB;
        static constexpr U8 JMPBOOT1 = 0x3C;
        static constexpr U8 JMPBOOT2 = 0x90;

        U8 jmpboot[JMPBOOT_SIZE]; // NOLINT need binary compatibility with FAT spec

        U8 oemid[OEMID_SIZE]; // NOLINT need binary compatibility with FAT spec

        U16 bytes_per_sector{0};    // 512, 1024, 2048 or 4096
        U8  sectors_per_cluster{0}; // 1-128, only power of 2's
        U16 reserved_sector_count{0};
        U8  fat_count{0};
        U16 root_entry_count{0};
        U16 total_sectors_16{0};
        U8  media_descriptor_type{0};
        U16 fat_size_16{0};
        U16 sectors_per_track{0};
        U16 head_count{0};
        U32 hidden_sector_count{0};
        U32 total_sectors_32{0};

        BIOSParameterBlock();
    } PACKED;

    struct ExtendedBIOSParameterBlock1216 {
        static constexpr size_t VOLUME_LABEL_SIZE = 11;
        static constexpr size_t BOOT_CODE_SIZE    = 448;

        static constexpr U16 SIGNATURE_WORD = 0x55AA;

        U8  drive_number{0};
        U8  reserved_1{0};
        U8  boot_signature{0};
        U32 volume_id{0};
        U8  volume_label[VOLUME_LABEL_SIZE]; // NOLINT need binary compatibility with FAT spec
        U8  file_system_type{0};
        U8  boot_code[BOOT_CODE_SIZE]; // NOLINT need binary compatibility with FAT spec
        U16 signature_word;

        ExtendedBIOSParameterBlock1216();
    } PACKED;

    struct ExtendedBIOSParameterBlock32 {
        static constexpr size_t RESERVED0_SIZE    = 12;
        static constexpr size_t VOLUME_LABEL_SIZE = 11;
        static constexpr size_t SYSTEM_ID_SIZE    = 8;
        static constexpr size_t BOOT_CODE_SIZE    = 420;

        static constexpr U16 SIGNATURE_WORD = 0x55AA;

        U32 fat_size_32{0};
        U16 flags{0};
        U16 fat_version{0};
        U32 root_cluster{0};
        U16 fs_info{0};
        U16 backup_bs_sector{0};
        U8  reserved_0[RESERVED0_SIZE]; // NOLINT need binary compatibility with FAT spec
        U8  drive_number{0};
        U8  reserved_1{0};
        U8  signature{0};
        U32 volume_id{0};
        U8  volume_label[VOLUME_LABEL_SIZE]; // NOLINT need binary compatibility with FAT spec
        U8  system_id[SYSTEM_ID_SIZE];       // NOLINT need binary compatibility with FAT spec
        U8  boot_code[BOOT_CODE_SIZE];       // NOLINT need binary compatibility with FAT spec
        U16 signature_word;

        ExtendedBIOSParameterBlock32();
    } PACKED;

    struct BootRecord1216 {
        BIOSParameterBlock             BPB;
        ExtendedBIOSParameterBlock1216 EBPB;
    };

    struct BootRecord32 {
        BIOSParameterBlock           BPB;
        ExtendedBIOSParameterBlock32 EBPB;
    };

    struct FileSystemInfo {
        static constexpr size_t RESERVED1_SIZE = 480;
        static constexpr size_t RESERVED2_SIZE = 12;

        static constexpr U32 NO_INFO         = 0xFFFFFFFF;
        static constexpr U32 LEAD_SIGNATURE  = 0x41615252;
        static constexpr U32 STRUC_SIGNATURE = 0x61417272;
        static constexpr U32 TRAIL_SIGNATURE = 0xAA550000;

        U32 lead_signature;
        U8  reserved_1[RESERVED1_SIZE]; // NOLINT need binary compatibility with FAT spec
        U32 struc_signature;
        U32 free_count;
        U32 next_free;
        U8  reserved_2[RESERVED2_SIZE]; // NOLINT need binary compatibility with FAT spec
        U32 trail_signature;

        FileSystemInfo();
    };

#define FAT_FILE_ATTRIBUTES(X)                                                                     \
    X(FATFileAttribute, READONLY, 0x01)                                                            \
    X(FATFileAttribute, HIDDEN, 0x02)                                                              \
    X(FATFileAttribute, SYSTEM, 0x04)                                                              \
    X(FATFileAttribute, VOLUME_ID, 0x08)                                                           \
    X(FATFileAttribute, DIRECTORY, 0x10)                                                           \
    X(FATFileAttribute, ARCHIVE, 0x20)                                                             \
    X(FATFileAttribute, LONG_FILE_NAME, 0x0F)

    DECLARE_ENUM(FATFileAttribute, FAT_FILE_ATTRIBUTES, 0x0) // NOLINT

    struct FileEntry {
        static constexpr U8 SHORT_NAME_SIZE      = 11;
        static constexpr U8 SHORT_NAME_MAIN_SIZE = 8;
        static constexpr U8 SHORT_NAME_EXT_SIZE  = 3;

        static constexpr U8 MARK_EMPTY_END    = 0x00;
        static constexpr U8 MARK_EMPTY_MIDDLE = 0xE5;
        static constexpr U8 TRAILING_SPACE    = 0x20;

        // NOLINTBEGIN need binary compatibility with FAT spec
        union {
            U8 as_array[SHORT_NAME_SIZE] = {};
            struct {
                U8 name[SHORT_NAME_MAIN_SIZE];
                U8 extension[SHORT_NAME_EXT_SIZE];
            };
        } short_name;
        // NOLINTEND
        U8 attributes           = 0;
        U8 nt_reserved          = 0;
        U8 creation_time_millis = 0; // Hundreds of a second (100ms), 0 <= MS <= 199

        union {
            U16 as_U16 = 0;

            struct {
                U16 seconds : 5;
                U16 minutes : 6;
                U16 hour    : 5;
            };
        } creation_time;

        union {
            U16 as_U16 = 0;

            struct {
                U16 day   : 5;
                U16 month : 4;
                U16 year  : 7;
            };
        } creation_date;

        union {
            U16 as_U16 = 0;

            struct {
                U16 day   : 5;
                U16 month : 4;
                U16 year  : 7;
            };
        } last_accessed_date;

        U16 first_cluster_high = 0;

        union {
            U16 as_U16 = 0;

            struct {
                U16 seconds : 5;
                U16 minutes : 6;
                U16 hour    : 5;
            };
        } last_modification_time;

        union {
            U16 as_U16 = 0;

            struct {
                U16 day   : 5;
                U16 month : 4;
                U16 year  : 7;
            };
        } last_modification_date;

        U16 first_cluster_low = 0;
        U32 file_size         = 0; // Bytes

        /**
         * Verify that the given name is valid.
         *
         * A name is valid when it only contains:
         * <ol>
         *  <li>Upper or lower case letters</li>
         *  <li>Digits</li>
         *  <li>ASCII code points > 127</li>
         *  <li>Special characters: $%'-_@~`!(){}^#&</li>
         *  <li>It's length is either 8 (file name) or 3 (file extension)</li>
         * <ol>
         *
         * @param name           Name
         * @param allowed_length Allowed length of the name
         *
         * @return True if the name is valid.
         */
        static auto validate_name(const String& name, U8 allowed_length) -> bool;

        /**
         *
         * @return True if this entry is unused and no more used entries follow after this one.
         */
        [[nodiscard]] auto is_empty_end() const -> bool;

        /**
         *
         * @return True if this entry is unused but used entries will follow after this one.
         */
        [[nodiscard]] auto is_empty_middle() const -> bool;

        /**
         *
         * @return The dot separated short name and extension e.g. File.txt
         */
        [[nodiscard]] auto make_short_name() const -> String;

        /**
         *
         * @return
         */
        [[nodiscard]] auto compute_short_name_checksum() const -> U8;

        /**
         *
         * @return cluster index of the file entry content.
         */
        [[nodiscard]] auto cluster() const -> U32;

        [[nodiscard]] auto has_attribute(FATFileAttribute attr) const -> bool;
    };

    struct LongFileNameEntry {
        static constexpr U8 MASK_LAST_LFN_ENTRY = 0xF0;

        static constexpr U8  FN1_SIZE           = 5;
        static constexpr U8  FN2_SIZE           = 6;
        static constexpr U8  FN3_SIZE           = 2;
        static constexpr U16 MAX_FILE_NAME_SIZE = 255;
        static constexpr U8  MAX_CHAR_PER_ENTRY = 13;
        static constexpr U8  LAST_LFN_ENTRY     = 0x40;

        U8  order;
        U16 file_name_1[FN1_SIZE]; // NOLINT need binary compatibility with FAT spec
        U8  attributes;
        U8  long_entry_type;
        U8  short_file_name_checksum;
        U16 file_name_2[FN2_SIZE]; // NOLINT need binary compatibility with FAT spec
        U16 reserved;
        U16 file_name_3[FN3_SIZE]; // NOLINT need binary compatibility with FAT spec

        /**
         * Verify that the given name is valid.
         *
         * A name is valid when it only contains:
         * <ol>
         *  <li>Upper or lower case letters</li>
         *  <li>Digits</li>
         *  <li>ASCII code points > 127</li>
         *  <li>Special characters: $%'-_@~`!(){}^#&.+,;=[]</li>
         *  <li>It's length is either 8 (file name) or 3 (file extension)</li>
         * <ol>
         *
         * @param name          Name
         * @param allowedLength Allowed length of the name
         *
         * @return True if the name is valid.
         */
        static auto validate_name(const String& name) -> bool;
    } PACKED;

    //////////////////////////////////////////////////////////////////////////////////
    //                                                                              //
    // Own FAT structures                                                           //
    //                                                                              //
    //////////////////////////////////////////////////////////////////////////////////

    /**
     * @brief Location of a file entry on the volume.
     */
    struct FileEntryLocation {
        U32 cluster   = 0; // Cluster where the file entry is stored on the storage
        U16 entry_idx = 0; // Index of the file entry on the cluster if interpreted as FileEntry[]
    };

    /**
     * A file entry with it's position on the storage.
     */
    struct LocationAwareFileEntry {
        String            file_name = ""; // Cache the file name in case it is a long file name
        FileEntry         file      = {}; // Copy of the file entry on the storage
        FileEntryLocation location  = {.cluster   = 0,
                                       .entry_idx = 0}; // Location of the file entry on the volume
        FileEntryLocation first_lfn_entry = {
            .cluster   = 0,
            .entry_idx = 0}; // Location of the first LFN entry on the volume
    };

    /**
     * Mapping of a storage device ID to a BPB.
     */
    struct StorageDevRef {
        U16                 storage_dev = -1;
        BIOSParameterBlock* BPB         = nullptr;

        StorageDevRef(U16 storage_dev, BIOSParameterBlock* bpb);

        ~StorageDevRef();
    };
} // namespace Rune::VFS

#endif // RUNEOS_FAT_H
