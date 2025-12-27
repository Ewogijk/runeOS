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

#ifndef RUNEOS_FATENGINE_H
#define RUNEOS_FATENGINE_H

#include <VirtualFileSystem/FAT/FAT.h>

namespace Rune::VFS {

    class FATEngine {
      public:
        virtual ~FATEngine() = default;

        [[nodiscard]] virtual auto get_name() const -> String = 0;

        //////////////////////////////////////////////////////////////////////////////////
        //                                                                              //
        // BootRecord functions                                                         //
        //                                                                              //
        //////////////////////////////////////////////////////////////////////////////////

        /**
         * Create a new boot record for a storage device and write it to the buffer which must have
         * the size of a sector on said storage device.
         *
         * @param buf          Output buffer for the boot record.
         * @param sector_size  Sector size of a storage.
         * @param sector_count Sector count of a storage.
         *
         * @return True: The boot record is created. False: It is not.
         */
        virtual auto make_new_boot_record(U8* buf, U32 sector_size, U32 sector_count) -> bool = 0;

        /**
         *
         * @param total_clusters Total clusters on a storage device.
         *
         * @return True: The storage can be mounted by the FAT driver
         */
        virtual auto can_mount(U32 total_clusters) -> bool = 0;

        /**
         *
         * @param bpb
         *
         * @return The backup boot record sector.
         */
        virtual auto get_backup_boot_record_sector(BIOSParameterBlock* bpb) -> U16 = 0;

        /**
         *
         * @param bpb
         *
         * @return The root directory cluster.
         */
        virtual auto get_root_directory_cluster(BIOSParameterBlock* bpb) -> uint32_t = 0;

        /**
         *
         * @return The maximum number of clusters a storage device can have.
         */
        virtual auto get_max_cluster_count() -> uint32_t = 0;

        //////////////////////////////////////////////////////////////////////////////////
        //                                                                              //
        // FAT functions                                                                //
        //                                                                              //
        //////////////////////////////////////////////////////////////////////////////////

        /**
         *
         * @param bpb
         *
         * @return The size of a single FAT in sectors.
         */
        virtual auto fat_get_size(BIOSParameterBlock* bpb) -> uint32_t = 0;

        /**
         *
         * @return The EOF marker in the FAT.
         */
        virtual auto fat_get_eof_marker() -> uint32_t = 0;

        /**
         *
         * @param cluster cluster index.
         *
         * @return Offset into the FAT for a cluster, that is the number of bytes that need to be
         * skipped to read the FAT entry for the cluster.
         */
        virtual auto fat_offset(uint32_t cluster) -> uint32_t = 0;

        /**
         *
         * @param fat          A buffer for two FAT sectors.
         * @param entry_offset Offset into the FAT buffer.
         *
         * @return The FAT entry in the given FAT buffer.
         */
        virtual auto fat_get_entry(U8* fat, uint32_t entry_offset) -> uint32_t = 0;

        /**
         * Set the FAT entry at the given offset in the FAT buffer to the new value.
         *
         * @param fat          A buffer for two FAT sectors.
         * @param entry_offset Offset into the FAT buffer.
         * @param new_entry    New FAT entry.
         */
        virtual void fat_set_entry(U8* fat, uint32_t entry_offset, uint32_t new_entry) = 0;

        /**
         * Try to find a free cluster in the given FAT buffer.
         *
         * @param fat            A buffer for two FAT sectors.
         * @param fat_sector_idx Sector index of the FAT buffer, e.g. if the buffer starts at the
         * 11th FAT sector the sector index is 11.
         *
         * @return A free cluster or MaxClusterCount() + 1 if no free cluster was found.
         */
        virtual auto fat_find_free_cluster(U8* fat, uint32_t fat_sector_idx) -> uint32_t = 0;
    };
} // namespace Rune::VFS

#endif // RUNEOS_FATENGINE_H
