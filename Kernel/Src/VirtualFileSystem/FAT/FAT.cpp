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

#include <VirtualFileSystem/FAT/FAT.h>

#include <Hammer/Memory.h>
#include <Hammer/Algorithm.h>


namespace Rune::VFS {
    DEFINE_ENUM(FATType, FAT_TYPES, 0x0)


    DEFINE_ENUM(FATFileAttribute, FAT_FILE_ATTRIBUTES, 0x0)


    BIOSParameterBlock::BIOSParameterBlock()
            : jmpboot(),
              oemid(),
              bytes_per_sector(0),
              sectors_per_cluster(0),
              reserved_sector_count(0),
              fat_count(0),
              root_entry_count(0),
              total_sectors_16(0),
              media_descriptor_type(0),
              fat_size_16(0),
              sectors_per_track(0),
              head_count(0),
              hidden_sector_count(0),
              total_sectors_32(0) {
        jmpboot[0] = 0xEB;
        jmpboot[1] = 0x3C;
        jmpboot[2] = 0x90;
        const char* ewo05 = "Ewo0.5  ";
        memcpy(oemid, (void*) ewo05, 8);
    }


    ExtendedBIOSParameterBlock1216::ExtendedBIOSParameterBlock1216()
            : drive_number(0),
              reserved_1(0),
              boot_signature(0),
              volume_id(0),
              volume_label(),
              file_system_type(0),
              boot_code(),
              signature_word(0x55AA) {

    }


    ExtendedBIOSParameterBlock32::ExtendedBIOSParameterBlock32()
            : fat_size_32(0),
              flags(0),
              fat_version(0),
              root_cluster(0),
              fs_info(0),
              backup_bs_sector(0),
              reserved_0(),
              drive_number(0),
              reserved_1(0),
              signature(0),
              volume_id(0),
              volume_label(),
              system_id(),
              boot_code(),
              signature_word(0) {
    }


    FileSystemInfo::FileSystemInfo()
            : lead_signature(0x41615252),
              reserved_1(),
              struc_signature(0x61417272),
              free_count(0xFFFFFFFF),
              next_free(0xFFFFFFFF),
              reserved_2(),
              trail_signature(0xAA550000) {

    }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                     FileEntry Implementation
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    bool FileEntry::validate_name(const String& name, U8 allowed_length) {
        if (name.size() > allowed_length)
            return false;
        if (name[0] == ' ' || name[0] == 0x20)
            return false;
        for (int i = 0; i < allowed_length; i++) {
            char c = name[i];
            if (c == 0x0)
                continue;
            // File name can only contain code points > 127, letters, digits
            // and one of: $%'-_@~`!(){}^#&
            bool is_digit           = 0x30 <= c && c <= 0x39;
            bool is_big_letter      = 0x41 <= c && c <= 0x5A;
            bool is_small_letter    = 0x61 <= c && c <= 0x7A;
            bool is_allowed_special = c == 0x24       // $
                                      || c == 0x25    // %
                                      || c == 0x27    // '
                                      || c == 0x2D    // -
                                      || c == 0x5F    // _
                                      || c == 0x40    // @
                                      || c == 0x7E    // ~
                                      || c == 0x60    // `
                                      || c == 0x21    // !
                                      || c == 0x28    // (
                                      || c == 0x29    // )
                                      || c == 0x7B    // {
                                      || c == 0x7D    // }
                                      || c == 0x5E    // ^
                                      || c == 0x23    // #
                                      || c == 0x26;   // &

            if (!is_digit && !is_big_letter && !is_small_letter && !is_allowed_special)
                return false;
        }
        return true;
    }


    bool FileEntry::is_empty_end() const {
        return (unsigned char) short_name.as_array[0] == MARK_EMPTY_END;
    }


    bool FileEntry::is_empty_middle() const {
        return (unsigned char) short_name.as_array[0] == MARK_EMPTY_MIDDLE;
    }


    String FileEntry::make_short_name() const {
        String file_name = String((const char*) short_name.name, 8).replace(0x20, '\0');
        String ext       = String((const char*) short_name.extension, 3).replace(0x20, '\0');
        if (ext.size() > 0)
            file_name += "." + ext;
        return file_name;
    }


    U8 FileEntry::compute_short_name_checksum() const {
        U8                 checksum = 0;
        for (unsigned char i: short_name.as_array)
            checksum = ((checksum & 1) ? 0x80 : 0) + (checksum >> 1) + i;
        return checksum;
    }


    U32 FileEntry::cluster() const {
        return first_cluster_high << 16 | first_cluster_low;
    }


    bool FileEntry::has_attribute(VFS::FATFileAttribute attr) const {
        switch (attr) {
            case FATFileAttribute::READONLY:
                return check_bit(attributes, 0);
            case FATFileAttribute::HIDDEN:
                return check_bit(attributes, 1);
            case FATFileAttribute::SYSTEM:
                return check_bit(attributes, 2);
            case FATFileAttribute::VOLUME_ID:
                return check_bit(attributes, 3);
            case FATFileAttribute::DIRECTORY:
                return check_bit(attributes, 4);
            case FATFileAttribute::ARCHIVE:
                return check_bit(attributes, 5);
            case FATFileAttribute::LONG_FILE_NAME:
                return (attributes & FATFileAttribute::LONG_FILE_NAME) == FATFileAttribute::LONG_FILE_NAME;
            case FATFileAttribute::NONE:
                return false;
        }
        return false;
    }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                    Mounted Storage Ref Functions
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    StorageDevRef::StorageDevRef(U16 storage_dev, BIOSParameterBlock* bpb) :
            storage_dev(storage_dev), BPB(bpb) {

    }


    StorageDevRef::~StorageDevRef() {
        delete BPB;
    }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                     Long File Name Entry Functions
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    bool LongFileNameEntry::validate_name(const String& name) {
        if (name.size() > LongFileNameEntry::MAX_FILE_NAME_SIZE)
            return false;
        if (name[0] == ' ' || name[0] == 0x20)
            return false;
        for (size_t i = 0; i < name.size(); i++) {
            char c = name[i];
            if (c == 0x0)
                continue;
            // File name can only contain code points > 127, letters, digits
            // and one of: $%'-_@~`!(){}^#&
            // additional special chars for long file names: . + , ; = [ ]
            bool is_digit           = 0x30 <= c && c <= 0x39;
            bool is_big_letter      = 0x41 <= c && c <= 0x5A;
            bool is_small_letter    = 0x61 <= c && c <= 0x7A;
            bool is_allowed_special = c == 0x24       // $
                                      || c == 0x25    // %
                                      || c == 0x27    // '
                                      || c == 0x2D    // -
                                      || c == 0x5F    // _
                                      || c == 0x40    // @
                                      || c == 0x7E    // ~
                                      || c == 0x60    // `
                                      || c == 0x21    // !
                                      || c == 0x28    // (
                                      || c == 0x29    // )
                                      || c == 0x7B    // {
                                      || c == 0x7D    // }
                                      || c == 0x5E    // ^
                                      || c == 0x23    // #
                                      || c == 0x26    // &
                                      || c == 0x2E    // .
                                      || c == 0x2B    // +
                                      || c == 0x2C    // ,
                                      || c == 0x3B    // ;
                                      || c == 0x3D    // =
                                      || c == 0x5B    // [
                                      || c == 0x5D;   // ]

            if (!is_digit && !is_big_letter && !is_small_letter && !is_allowed_special)
                return false;
        }
        return true;
    }

}