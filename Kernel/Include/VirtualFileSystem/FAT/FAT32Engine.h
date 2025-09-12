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

      public:
        ~FAT32Engine() override = default;

        [[nodiscard]]
        String get_name() const override;

        //////////////////////////////////////////////////////////////////////////////////
        //                                                                              //
        // BootRecord functions                                                         //
        //                                                                              //
        //////////////////////////////////////////////////////////////////////////////////

        bool make_new_boot_record(U8* buf, U32 sector_size, U32 sector_count) override;

        bool can_mount(U32 total_clusters) override;

        U16 get_backup_boot_record_sector(BIOSParameterBlock* bpb) override;

        U32 get_root_directory_cluster(BIOSParameterBlock* bpb) override;

        U32 get_max_cluster_count() override;

        //////////////////////////////////////////////////////////////////////////////////
        //                                                                              //
        // FAT functions                                                                //
        //                                                                              //
        //////////////////////////////////////////////////////////////////////////////////

        U32 fat_get_size(BIOSParameterBlock* bpb) override;

        U32 fat_get_eof_marker() override;

        U32 fat_offset(U32 cluster) override;

        U32 fat_get_entry(U8* fat, U32 entry_offset) override;

        void fat_set_entry(U8* fat, U32 entry_offset, U32 new_entry) override;

        U32 fat_find_free_cluster(U8* fat, U32 fat_sector_idx) override;
    };

} // namespace Rune::VFS

#endif // RUNEOS_FAT32ENGINE_H
