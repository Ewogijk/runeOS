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

#include <VirtualFileSystem/FAT/VolumeManager.h>

namespace Rune::VFS {
    size_t VolumeManager::data_cluster_to_lba(BIOSParameterBlock* bpb, size_t cluster) const {
        return bpb->reserved_sector_count + bpb->fat_count * _fat_engine->fat_get_size(bpb)
               + (cluster - 2) * bpb->sectors_per_cluster;
    }

    VolumeManager::VolumeManager(SharedPointer<FATEngine> fat_engine, Device::AHCIDriver& ahci_driver)
        : _fat_engine(move(fat_engine)),
          _ahci_driver(ahci_driver) {}

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          FAT Region Manipulation
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    U32 VolumeManager::fat_get_eof_marker() const { return _fat_engine->fat_get_eof_marker(); }

    U32 VolumeManager::fat_read(U16 storage_dev, BIOSParameterBlock* bpb, size_t cluster) const {
        U32 byte_offset       = _fat_engine->fat_offset(cluster);
        U32 fat_sector_number = bpb->reserved_sector_count + (byte_offset / bpb->bytes_per_sector);
        U8  fat[1024];
        if (_ahci_driver.read(storage_dev, fat, 1024, fat_sector_number) < 1024) return 0xFFFFFFFF;
        return _fat_engine->fat_get_entry(fat, byte_offset % bpb->bytes_per_sector);
    }

    bool VolumeManager::fat_write(U16 storage_dev, BIOSParameterBlock* bpb, size_t cluster, U32 fat_value) {
        U32 byte_offset       = _fat_engine->fat_offset(cluster);
        U32 fat_sector_number = bpb->reserved_sector_count + (byte_offset / bpb->bytes_per_sector);
        U8  fat[1024];
        if (_ahci_driver.read(storage_dev, fat, 1024, fat_sector_number) < 1024) return false;
        _fat_engine->fat_set_entry(fat, byte_offset % bpb->bytes_per_sector, fat_value);
        if (_ahci_driver.write(storage_dev, fat, 1024, fat_sector_number) < 1024) return false;

        memset(fat, 0, 1024);
        U32 fat_backup_sector_number = fat_sector_number + _fat_engine->fat_get_size(bpb);
        if (_ahci_driver.read(storage_dev, fat, 1024, fat_backup_sector_number) < 1024) return false;

        _fat_engine->fat_set_entry(fat, byte_offset % bpb->bytes_per_sector, fat_value);
        return _ahci_driver.write(storage_dev, fat, 1024, fat_backup_sector_number) == 1024;
    }

    U32 VolumeManager::fat_find_next_free_cluster(U16 storage_dev, BIOSParameterBlock* bpb) {
        size_t cluster_size = bpb->bytes_per_sector * bpb->sectors_per_cluster;
        for (U32 i = 0; i < _fat_engine->fat_get_size(bpb); i += 2) {
            U8 fat[2 * cluster_size];
            if (_ahci_driver.read(storage_dev, fat, 2 * cluster_size, bpb->reserved_sector_count + i)
                < 2 * cluster_size)
                return 0;

            U32 free_cluster = _fat_engine->fat_find_free_cluster(fat, i);
            if (free_cluster > 1 && free_cluster <= _fat_engine->get_max_cluster_count()) return free_cluster;
        }
        return 0;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Data Region Manipulation
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    U32 VolumeManager::get_max_cluster_count() const { return _fat_engine->get_max_cluster_count(); }

    bool
    VolumeManager::data_cluster_read(U16 storage_dev, VFS::BIOSParameterBlock* bpb, void* buf, size_t cluster) const {
        U32 cluster_size = bpb->bytes_per_sector * bpb->sectors_per_cluster;
        return _ahci_driver.read(storage_dev,
                                 buf,
                                 bpb->bytes_per_sector * bpb->sectors_per_cluster,
                                 data_cluster_to_lba(bpb, cluster))
               == cluster_size;
    }

    bool VolumeManager::data_cluster_write(U16 storage_dev, VFS::BIOSParameterBlock* bpb, void* buf, size_t cluster) {
        U32 cluster_size = bpb->bytes_per_sector * bpb->sectors_per_cluster;
        return _ahci_driver.write(storage_dev, buf, cluster_size, data_cluster_to_lba(bpb, cluster)) == cluster_size;
    }

} // namespace Rune::VFS