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

#ifndef RUNEOS_FAT32ENGINE_H
#define RUNEOS_FAT32ENGINE_H

#include <VirtualFileSystem/FAT/FATEngine.h>

namespace Rune::VFS {

    class FAT32Engine : public FATEngine {
        static constexpr U16 FAT_16_MAX_CLUSTERS = 65525;
        static constexpr U32 EOF                 = 0xFFFFFFFF;
        ///@brief The 4 high bits are reserved, they must be preserved on modification.
        static constexpr U32 RESERVED_BITS_MASK = 0xF0000000;
        ///@brief Mask to get non-reserved bits of an entry.
        static constexpr U32 ENTRY_MASK = 0x0FFFFFFF;
        ///@brief Max cluster count per spec is 0x0FFFFFF7, don't remember why this was chosen.
        static constexpr U32 MAX_CLUSTER_COUNT        = 0x0FFFFFF0;
        static constexpr U8  CLUSTER_COUNT_PER_SECTOR = 128;

      public:
        ~FAT32Engine() override = default;

        [[nodiscard]] auto get_name() const -> String override;

        //////////////////////////////////////////////////////////////////////////////////
        //                                                                              //
        // BootRecord functions                                                         //
        //                                                                              //
        //////////////////////////////////////////////////////////////////////////////////

        auto make_new_boot_record(U8* buf, U32 sector_size, U32 sector_count) -> bool override;

        auto can_mount(U32 total_clusters) -> bool override;

        auto get_backup_boot_record_sector(BIOSParameterBlock* bpb) -> U16 override;

        auto get_root_directory_cluster(BIOSParameterBlock* bpb) -> U32 override;

        auto get_max_cluster_count() -> U32 override;

        //////////////////////////////////////////////////////////////////////////////////
        //                                                                              //
        // FAT functions                                                                //
        //                                                                              //
        //////////////////////////////////////////////////////////////////////////////////

        auto fat_get_size(BIOSParameterBlock* bpb) -> U32 override;

        auto fat_get_eof_marker() -> U32 override;

        auto fat_offset(U32 cluster) -> U32 override;

        auto fat_get_entry(U8* fat, U32 entry_offset) -> U32 override;

        void fat_set_entry(U8* fat, U32 entry_offset, U32 new_entry) override;

        auto fat_find_free_cluster(U8* fat, U32 fat_sector_idx) -> U32 override;
    };

} // namespace Rune::VFS

#endif // RUNEOS_FAT32ENGINE_H
