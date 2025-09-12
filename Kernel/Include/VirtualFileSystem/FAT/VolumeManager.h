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

#ifndef RUNEOS_VOLUMEMANAGER_H
#define RUNEOS_VOLUMEMANAGER_H

#include <Device/AHCI/AHCI.h>

#include <VirtualFileSystem/FAT/FAT.h>
#include <VirtualFileSystem/FAT/FATEngine.h>

namespace Rune::VFS {

    /**
     * Manages low level read/writes on the FAT and Data region of a volume.
     */
    class VolumeManager {
        SharedPointer<FATEngine> _fat_engine;
        Device::AHCIDriver&      _ahci_driver;

        /**
         *
         * @param bpb
         * @param cluster
         *
         * @return Convert a cluster to LBA.
         */
        size_t data_cluster_to_lba(BIOSParameterBlock* bpb, size_t cluster) const;

      public:
        explicit VolumeManager(SharedPointer<FATEngine> fat_engine, Device::AHCIDriver& ahci_driver);

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          FAT Region Manipulation
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        /**
         *
         * @return The FAT EOF marker.
         */
        [[nodiscard]]
        U32 fat_get_eof_marker() const;

        /**
         * Read the FAT entry of a cluster.
         *
         * @param storage_id
         * @param bpb
         * @param cluster
         *
         * @return FAT entry for the given cluster.
         */
        [[nodiscard]]
        U32 fat_read(U16 storage_id, BIOSParameterBlock* bpb, size_t cluster) const;

        /**
         * Update the FAT entry of a cluster.
         *
         * @param storage_dev
         * @param bpb
         * @param cluster  cluster for which the FAT entry should be updated.
         * @param fat_value New FAT value.
         *
         * @return True: The FAT entry is updated, False: It is not.
         */
        bool fat_write(U16 storage_dev, BIOSParameterBlock* bpb, size_t cluster, U32 fat_value);

        /**
         * Search the FAT for a free cluster. The first free encountered cluster will be returned.
         *
         * @param storage_dev
         * @param bpb
         *
         * @return The next free cluster in the FAT. 0 if no cluster is free or a storage error happened.
         */
        U32 fat_find_next_free_cluster(U16 storage_dev, BIOSParameterBlock* bpb);

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          Data Region Manipulation
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        /**
         *
         * @return Maximum number of cluster a storage can have.
         */
        [[nodiscard]]
        U32 get_max_cluster_count() const;

        /**
         * Read a single data cluster, that is a cluster after the reserved and FAT parts of the storage.
         *
         * @param storage_dev
         * @param bpb
         * @param buf        Buffer to read the cluster into.
         * @param cluster    cluster index.
         *
         * @return True: The cluster was read. False: It was not.
         */
        bool data_cluster_read(U16 storage_dev, BIOSParameterBlock* bpb, void* buf, size_t cluster) const;

        /**
         * Overwrite a single data cluster, that is a cluster after the reserved and FAT parts of the storage.
         *
         * @param storage_dev
         * @param bpb
         * @param buf       Buffer with data to write.
         * @param cluster   cluster index.
         *
         * @return True: The cluster was written. False: It was not.
         */
        bool data_cluster_write(U16 storage_dev, BIOSParameterBlock* bpb, void* buf, size_t cluster);
    };
} // namespace Rune::VFS

#endif // RUNEOS_VOLUMEMANAGER_H
