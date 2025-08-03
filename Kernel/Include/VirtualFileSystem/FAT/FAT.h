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
#include <Hammer/String.h>

#include <Ember/Enum.h>


namespace Rune::VFS {

    //////////////////////////////////////////////////////////////////////////////////
    //                                                                              //
    // FAT specification structures - Microsoft FAT Specification 30.08.2005        //
    //                                                                              //
    //////////////////////////////////////////////////////////////////////////////////

#define FAT_TYPES(X)         \
    X(FATType, FAT12, 0x1)  \
    X(FATType, FAT16, 0x2)  \
    X(FATType, FAT32, 0x3)  \



    DECLARE_ENUM(FATType, FAT_TYPES, 0x0) //NOLINT

    struct BIOSParameterBlock {
        U8 jmpboot[3];

        U8 oemid[8];

        U16 bytes_per_sector;            // 512, 1024, 2048 or 4096
        U8  sectors_per_cluster;         // 1-128, only power of 2's
        U16 reserved_sector_count;
        U8  fat_count;
        U16 root_entry_count;
        U16 total_sectors_16;
        U8  media_descriptor_type;
        U16 fat_size_16;
        U16 sectors_per_track;
        U16 head_count;
        U32 hidden_sector_count;
        U32 total_sectors_32;


        BIOSParameterBlock();
    } PACKED;


    struct ExtendedBIOSParameterBlock1216 {
        U8  drive_number;
        U8  reserved_1;
        U8  boot_signature;
        U32 volume_id;
        U8  volume_label[11];
        U8  file_system_type;
        U8  boot_code[448];
        U16 signature_word;


        ExtendedBIOSParameterBlock1216();
    } PACKED;


    struct ExtendedBIOSParameterBlock32 {
        U32 fat_size_32;
        U16 flags;
        U16 fat_version;
        U32 root_cluster;
        U16 fs_info;
        U16 backup_bs_sector;
        U8  reserved_0[12];
        U8  drive_number;
        U8  reserved_1;
        U8  signature;
        U32 volume_id;
        U8  volume_label[11];
        U8  system_id[8];
        U8  boot_code[420];
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
        U32 lead_signature;
        U8  reserved_1[480];
        U32 struc_signature;
        U32 free_count;
        U32 next_free;
        U8  reserved_2[12];
        U32 trail_signature;


        FileSystemInfo();
    };


#define FAT_FILE_ATTRIBUTES(X)                   \
    X(FATFileAttribute, READONLY, 0x01)        \
    X(FATFileAttribute, HIDDEN, 0x02)          \
    X(FATFileAttribute, SYSTEM, 0x04)          \
    X(FATFileAttribute, VOLUME_ID, 0x08)        \
    X(FATFileAttribute, DIRECTORY, 0x10)       \
    X(FATFileAttribute, ARCHIVE, 0x20)         \
    X(FATFileAttribute, LONG_FILE_NAME, 0x0F)    \



    DECLARE_ENUM(FATFileAttribute, FAT_FILE_ATTRIBUTES, 0x0) //NOLINT


    struct FileEntry {
        static constexpr U8 MARK_EMPTY_END       = 0x00;
        static constexpr U8 MARK_EMPTY_MIDDLE    = 0xE5;

        union {
            U8 as_array[11] = { };
            struct {
                U8 name[8];
                U8 extension[3];
            };
        }                   short_name;
        U8                  attributes           = 0;
        U8                  nt_reserved          = 0;
        U8                  creation_time_millis = 0;     // Hundreds of a second (100ms), 0 <= MS <= 199

        union {
            U16 as_U16 = 0;

            struct {
                U16 seconds: 5;
                U16 minutes: 6;
                U16 hour: 5;
            };
        }                   creation_time;

        union {
            U16 as_U16 = 0;

            struct {
                U16 day: 5;
                U16 month: 4;
                U16 year: 7;
            };
        }                   creation_date;

        union {
            U16 as_U16 = 0;

            struct {
                U16 day: 5;
                U16 month: 4;
                U16 year: 7;
            };
        }                   last_accessed_date;

        U16 first_cluster_high = 0;

        union {
            U16 as_U16 = 0;

            struct {
                U16 seconds: 5;
                U16 minutes: 6;
                U16 hour: 5;
            };
        }   last_modification_time;

        union {
            U16 as_U16 = 0;

            struct {
                U16 day: 5;
                U16 month: 4;
                U16 year: 7;
            };
        }   last_modification_date;

        U16 first_cluster_low = 0;
        U32 file_size         = 0;       // Bytes


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
        static bool validate_name(const String& name, U8 allowed_length);


        /**
         *
         * @return True if this entry is unused and no more used entries follow after this one.
         */
        [[nodiscard]] bool is_empty_end() const;


        /**
         *
         * @return True if this entry is unused but used entries will follow after this one.
         */
        [[nodiscard]] bool is_empty_middle() const;


        /**
         *
         * @return The dot separated short name and extension e.g. File.txt
         */
        [[nodiscard]] String make_short_name() const;


        /**
         *
         * @return
         */
        [[nodiscard]] U8 compute_short_name_checksum() const;


        /**
         *
         * @return cluster index of the file entry content.
         */
        [[nodiscard]] U32 cluster() const;


        [[nodiscard]] bool has_attribute(FATFileAttribute attr) const;
    };


    struct LongFileNameEntry {
        static constexpr U16 MAX_FILE_NAME_SIZE = 255;
        static constexpr U8  MAX_CHAR_PER_ENTRY = 13;
        static constexpr U8  LAST_LFN_ENTRY     = 0x40;

        U8  order;
        U16 file_name_1[5];
        U8  attributes;
        U8  long_entry_type;
        U8  short_file_name_checksum;
        U16 file_name_2[6];
        U16 reserved;
        U16 file_name_3[2];


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
        static bool validate_name(const String& name);
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
        U32 cluster   = 0;   // Cluster where the file entry is stored on the storage
        U16 entry_idx = 0;   // Index of the file entry on the cluster if interpreted as FileEntry[]
    };

    /**
     * A file entry with it's position on the storage.
     */
    struct LocationAwareFileEntry {
        String            file_name       = "";       // Cache the file name in case it is a long file name
        FileEntry         file            = { };      // Copy of the file entry on the storage
        FileEntryLocation location        = { 0, 0 }; // Location of the file entry on the volume
        FileEntryLocation first_lfn_entry = { 0, 0 }; // Location of the first LFN entry on the volume
    };


    /**
     * Mapping of a storage device ID to a BPB.
     */
    struct StorageDevRef {
        U16 storage_dev = -1;
        BIOSParameterBlock* BPB = nullptr;


        StorageDevRef(U16 storage_dev, BIOSParameterBlock* bpb);


        ~StorageDevRef();
    };
}

#endif //RUNEOS_FAT_H
