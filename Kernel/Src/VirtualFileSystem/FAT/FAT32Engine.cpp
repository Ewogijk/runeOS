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

#include <VirtualFileSystem/FAT/FAT32Engine.h>

#include <KRE/Math.h>
#include <KRE/Memory.h>

#include <VirtualFileSystem/FAT/FAT.h>

namespace Rune::VFS {
    auto FAT32Engine::get_name() const -> String { return "FAT32"; }

    auto FAT32Engine::make_new_boot_record(U8* buf, U32 sector_size, U32 sector_count) -> bool {
        constexpr U8 BOOT_CODE_OFFSET    = 0x5A;
        constexpr U8 RESERVED_SECTORS    = 32; // Usual value
        constexpr U8 NON_REMOVABLE_MEDIA = 0xF8;
        constexpr U8 BACKUP_BOOT_SECTOR  = 6;
        constexpr U8 DRIVE_NUMBER        = 0x80; // Hard drive

        auto* br32 = reinterpret_cast<BootRecord32*>(buf);
        // Define a jmp instruction to the boot code area
        br32->BPB.jmpboot[0]            = BIOSParameterBlock::JMPBOOT0;
        br32->BPB.jmpboot[1]            = BOOT_CODE_OFFSET;
        br32->BPB.jmpboot[2]            = BIOSParameterBlock::JMPBOOT2;
        br32->BPB.bytes_per_sector      = sector_size;
        br32->BPB.sectors_per_cluster   = 1;
        br32->BPB.reserved_sector_count = RESERVED_SECTORS;
        br32->BPB.fat_count             = 2;
        br32->BPB.root_entry_count      = 0;
        br32->BPB.total_sectors_16      = 0;
        br32->BPB.media_descriptor_type = NON_REMOVABLE_MEDIA;
        br32->BPB.fat_size_16           = 0;
        br32->BPB.sectors_per_track     = 0;
        br32->BPB.head_count            = 0;
        br32->BPB.hidden_sector_count   = 0;
        br32->BPB.total_sectors_32      = sector_count;

        // Extended BIOS parameter block
        U32 non_reserved_sector_count = sector_count - (br32->BPB.reserved_sector_count);
        // Total non reserved clusters / Cluster count per FAT sector
        U32 fat_size = div_round_up((non_reserved_sector_count / br32->BPB.sectors_per_cluster),
                                    (sector_size / 4));
        // Exclude clusters used by the FAT tables
        fat_size -= div_round_up(fat_size, sector_size) * br32->BPB.sectors_per_cluster
                    * br32->BPB.fat_count;

        br32->EBPB.fat_size_32      = fat_size;
        br32->EBPB.flags            = 0;
        br32->EBPB.fat_version      = 0;
        br32->EBPB.root_cluster     = 2;
        br32->EBPB.fs_info          = 0;
        br32->EBPB.backup_bs_sector = BACKUP_BOOT_SECTOR;
        br32->EBPB.drive_number     = DRIVE_NUMBER;
        br32->EBPB.signature        = 0;
        br32->EBPB.volume_id        = 0; // TODO combine current date and time into 32 bit value
        memcpy(br32->EBPB.volume_label,
               (void*) "NO NAME    ",
               ExtendedBIOSParameterBlock32::VOLUME_LABEL_SIZE);
        memcpy(br32->EBPB.system_id,
               (void*) "FAT32   ",
               ExtendedBIOSParameterBlock32::SYSTEM_ID_SIZE);
        memset(br32->EBPB.boot_code, 0, ExtendedBIOSParameterBlock32::BOOT_CODE_SIZE);
        br32->EBPB.signature_word = ExtendedBIOSParameterBlock32::SIGNATURE_WORD;
        return true;
    }

    auto FAT32Engine::can_mount(U32 total_clusters) -> bool {
        return total_clusters >= FAT_16_MAX_CLUSTERS;
    }

    auto FAT32Engine::get_backup_boot_record_sector(BIOSParameterBlock* bpb) -> U16 {
        return (reinterpret_cast<BootRecord32*>(bpb))->EBPB.backup_bs_sector;
    }

    auto FAT32Engine::get_root_directory_cluster(BIOSParameterBlock* bpb) -> U32 {
        return (reinterpret_cast<BootRecord32*>(bpb))->EBPB.root_cluster;
    }

    auto FAT32Engine::get_max_cluster_count() -> U32 { return MAX_CLUSTER_COUNT; }

    auto FAT32Engine::fat_get_size(BIOSParameterBlock* bpb) -> U32 {
        return (reinterpret_cast<BootRecord32*>(bpb))->EBPB.fat_size_32;
    }

    auto FAT32Engine::fat_get_eof_marker() -> U32 { return EOF; }

    auto FAT32Engine::fat_offset(U32 cluster) -> U32 { return cluster * 4; }

    auto FAT32Engine::fat_get_entry(U8* fat, U32 entry_offset) -> U32 {
        return (*(reinterpret_cast<U32*>(&fat[entry_offset]))) & ENTRY_MASK;
    }

    void FAT32Engine::fat_set_entry(U8* fat, U32 entry_offset, U32 new_entry) {
        new_entry = ((*(reinterpret_cast<U32*>(&fat[entry_offset]))) & RESERVED_BITS_MASK)
                    | (new_entry & ENTRY_MASK);
        *(reinterpret_cast<U32*>(&fat[entry_offset])) = new_entry;
    }

    auto FAT32Engine::fat_find_free_cluster(U8* fat, U32 fat_sector_idx) -> U32 {
        U16   cluster_count = 2 * CLUSTER_COUNT_PER_SECTOR;
        auto* fat32         = reinterpret_cast<U32*>(fat);
        // For the first FAT sector: Skip the first two entries -> they are reserved
        for (U16 i = fat_sector_idx == 0 ? 2 : 0; i < cluster_count; i++) {
            if (fat32[i] == 0) return (fat_sector_idx * (cluster_count / 2)) + i;
        }
        return get_max_cluster_count() + 1;
    }
} // namespace Rune::VFS