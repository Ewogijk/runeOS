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

#include <KRE/BitsAndBytes.h>
#include <KRE/Memory.h>

namespace Rune::VFS {
    DEFINE_ENUM(FATType, FAT_TYPES, 0x0)

    DEFINE_ENUM(FATFileAttribute, FAT_FILE_ATTRIBUTES, 0x0)

    const char* BIOSParameterBlock::RUNEOS_OEM = "runeOS  ";

    BIOSParameterBlock::BIOSParameterBlock() : jmpboot(), oemid() {
        jmpboot[0] = JMPBOOT0;
        jmpboot[1] = JMPBOOT1;
        jmpboot[2] = JMPBOOT2;
        memcpy(oemid, (void*) RUNEOS_OEM, OEMID_SIZE);
    }

    ExtendedBIOSParameterBlock1216::ExtendedBIOSParameterBlock1216()
        : volume_label(),
          boot_code(),
          signature_word(SIGNATURE_WORD) {}

    ExtendedBIOSParameterBlock32::ExtendedBIOSParameterBlock32()
        : reserved_0(),
          volume_label(),
          system_id(),
          boot_code(),
          signature_word(SIGNATURE_WORD) {}

    FileSystemInfo::FileSystemInfo()
        : lead_signature(LEAD_SIGNATURE),
          reserved_1(),
          struc_signature(STRUC_SIGNATURE),
          free_count(NO_INFO),
          next_free(NO_INFO),
          reserved_2(),
          trail_signature(TRAIL_SIGNATURE) {}

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                     FileEntry Implementation
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    auto FileEntry::validate_name(const String& name, U8 allowed_length) -> bool {
        if (name.size() > allowed_length) return false;
        if (name[0] == ' ') return false;
        for (U8 i = 0; i < allowed_length; i++) {
            char c = name[i];
            if (c == 0x0) continue;
            // File name can only contain code points > 127, letters, digits
            // and one of: $%'-_@~`!(){}^#&
            // NOLINTBEGIN
            bool is_digit           = 0x30 <= c && c <= 0x39;
            bool is_big_letter      = 0x41 <= c && c <= 0x5A;
            bool is_small_letter    = 0x61 <= c && c <= 0x7A;
            bool is_allowed_special = c == 0x24     // $
                                      || c == 0x25  // %
                                      || c == 0x27  // '
                                      || c == 0x2D  // -
                                      || c == 0x5F  // _
                                      || c == 0x40  // @
                                      || c == 0x7E  // ~
                                      || c == 0x60  // `
                                      || c == 0x21  // !
                                      || c == 0x28  // (
                                      || c == 0x29  // )
                                      || c == 0x7B  // {
                                      || c == 0x7D  // }
                                      || c == 0x5E  // ^
                                      || c == 0x23  // #
                                      || c == 0x26; // &
            // NOLINTEND

            if (!is_digit && !is_big_letter && !is_small_letter && !is_allowed_special)
                return false;
        }
        return true;
    }

    auto FileEntry::is_empty_end() const -> bool {
        return (unsigned char) short_name.as_array[0] == MARK_EMPTY_END;
    }

    auto FileEntry::is_empty_middle() const -> bool {
        return (unsigned char) short_name.as_array[0] == MARK_EMPTY_MIDDLE;
    }

    auto FileEntry::make_short_name() const -> String {
        String file_name =
            String(reinterpret_cast<const char*>(short_name.name), SHORT_NAME_MAIN_SIZE)
                .replace(' ', '\0');
        String ext =
            String(reinterpret_cast<const char*>(short_name.extension), SHORT_NAME_EXT_SIZE)
                .replace(' ', '\0');
        if (ext.size() > 0) file_name += "." + ext;
        return file_name;
    }

    auto FileEntry::compute_short_name_checksum() const -> U8 {
        U8 checksum = 0;
        for (unsigned char i : short_name.as_array)
            checksum = ((checksum & 1) ? 0x80 : 0) + (checksum >> 1) + i; // NOLINT
        return checksum;
    }

    auto FileEntry::cluster() const -> U32 {
        return first_cluster_high << SHIFT_16 | first_cluster_low;
    }

    auto FileEntry::has_attribute(VFS::FATFileAttribute attr) const -> bool {
        switch (attr) { // NOLINT All cases handled...
            case FATFileAttribute::READONLY:  return bit_check(attributes, 0);
            case FATFileAttribute::HIDDEN:    return bit_check(attributes, 1);
            case FATFileAttribute::SYSTEM:    return bit_check(attributes, 2);
            case FATFileAttribute::VOLUME_ID: return bit_check(attributes, 3);
            case FATFileAttribute::DIRECTORY: return bit_check(attributes, 4);
            case FATFileAttribute::ARCHIVE:   return bit_check(attributes, 5); // NOLINT
            case FATFileAttribute::LONG_FILE_NAME:
                return (attributes & FATFileAttribute::LONG_FILE_NAME)
                       == FATFileAttribute::LONG_FILE_NAME;
            case FATFileAttribute::NONE: return false;
        }
        return false;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                    Mounted Storage Ref Functions
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    StorageDevRef::StorageDevRef(U16 storage_dev, BIOSParameterBlock* bpb)
        : storage_dev(storage_dev),
          BPB(bpb) {}

    StorageDevRef::~StorageDevRef() { delete BPB; }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                     Long File Name Entry Functions
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    auto LongFileNameEntry::validate_name(const String& name) -> bool {
        if (name.size() > LongFileNameEntry::MAX_FILE_NAME_SIZE) return false;
        if (name[0] == ' ') return false;
        for (char c : name) {
            if (c == 0x0) continue;
            // File name can only contain code points > 127, letters, digits
            // and one of: $%'-_@~`!(){}^#&
            // additional special chars for long file names: . + , ; = [ ]
            // NOLINTBEGIN
            bool is_digit           = 0x30 <= c && c <= 0x39;
            bool is_big_letter      = 0x41 <= c && c <= 0x5A;
            bool is_small_letter    = 0x61 <= c && c <= 0x7A;
            bool is_allowed_special = c == 0x24     // $
                                      || c == 0x25  // %
                                      || c == 0x27  // '
                                      || c == 0x2D  // -
                                      || c == 0x5F  // _
                                      || c == 0x40  // @
                                      || c == 0x7E  // ~
                                      || c == 0x60  // `
                                      || c == 0x21  // !
                                      || c == 0x28  // (
                                      || c == 0x29  // )
                                      || c == 0x7B  // {
                                      || c == 0x7D  // }
                                      || c == 0x5E  // ^
                                      || c == 0x23  // #
                                      || c == 0x26  // &
                                      || c == 0x2E  // .
                                      || c == 0x2B  // +
                                      || c == 0x2C  // ,
                                      || c == 0x3B  // ;
                                      || c == 0x3D  // =
                                      || c == 0x5B  // [
                                      || c == 0x5D; // ]
            // NOLINTEND
            if (!is_digit && !is_big_letter && !is_small_letter && !is_allowed_special)
                return false;
        }
        return true;
    }

} // namespace Rune::VFS