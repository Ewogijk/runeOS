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
    auto VolumeManager::data_cluster_to_lba(BIOSParameterBlock* bpb, size_t cluster) const
        -> size_t {
        return bpb->reserved_sector_count + (bpb->fat_count * _fat_engine->fat_get_size(bpb))
               + ((cluster - 2) * bpb->sectors_per_cluster);
    }

    VolumeManager::VolumeManager(SharedPointer<FATEngine> fat_engine,
                                 Device::AHCIDriver*      ahci_driver)
        : _fat_engine(move(fat_engine)),
          _ahci_driver(ahci_driver) {}

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          FAT Region Manipulation
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    auto VolumeManager::fat_get_eof_marker() const -> U32 {
        return _fat_engine->fat_get_eof_marker();
    }

    auto VolumeManager::fat_read(U16 storage_id, BIOSParameterBlock* bpb, size_t cluster) const
        -> U32 {
        U32 two_sector_size   = bpb->bytes_per_sector * 2;
        U32 byte_offset       = _fat_engine->fat_offset(cluster);
        U32 fat_sector_number = bpb->reserved_sector_count + (byte_offset / bpb->bytes_per_sector);
        U8  fat[two_sector_size]; // NOLINT
        if (_ahci_driver->read(storage_id, fat, two_sector_size, fat_sector_number)
            < two_sector_size)
            return _fat_engine->fat_get_eof_marker();
        return _fat_engine->fat_get_entry(fat, byte_offset % bpb->bytes_per_sector);
    }

    auto VolumeManager::fat_write(U16                 storage_dev,
                                  BIOSParameterBlock* bpb,
                                  size_t              cluster,
                                  U32                 fat_value) -> bool {
        U32 two_sector_size   = bpb->bytes_per_sector * 2;
        U32 byte_offset       = _fat_engine->fat_offset(cluster);
        U32 fat_sector_number = bpb->reserved_sector_count + (byte_offset / bpb->bytes_per_sector);
        U8  fat[two_sector_size]; // NOLINT
        if (_ahci_driver->read(storage_dev, fat, two_sector_size, fat_sector_number)
            < two_sector_size)
            return false;
        _fat_engine->fat_set_entry(fat, byte_offset % bpb->bytes_per_sector, fat_value);
        if (_ahci_driver->write(storage_dev, fat, two_sector_size, fat_sector_number)
            < two_sector_size)
            return false;

        memset(fat, 0, two_sector_size);
        U32 fat_backup_sector_number = fat_sector_number + _fat_engine->fat_get_size(bpb);
        if (_ahci_driver->read(storage_dev, fat, two_sector_size, fat_backup_sector_number)
            < two_sector_size)
            return false;

        _fat_engine->fat_set_entry(fat, byte_offset % bpb->bytes_per_sector, fat_value);
        return _ahci_driver->write(storage_dev, fat, two_sector_size, fat_backup_sector_number)
               == two_sector_size;
    }

    auto VolumeManager::fat_find_next_free_cluster(U16 storage_dev, BIOSParameterBlock* bpb)
        -> U32 {
        auto cluster_size = static_cast<size_t>(bpb->bytes_per_sector * bpb->sectors_per_cluster);
        for (U32 i = 0; i < _fat_engine->fat_get_size(bpb); i += 2) {
            U8 fat[2 * cluster_size]; // NOLINT
            if (_ahci_driver->read(storage_dev,
                                   fat,
                                   2 * cluster_size,
                                   bpb->reserved_sector_count + i)
                < 2 * cluster_size)
                return 0;

            U32 free_cluster = _fat_engine->fat_find_free_cluster(fat, i);
            if (free_cluster > 1 && free_cluster <= _fat_engine->get_max_cluster_count())
                return free_cluster;
        }
        return 0;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Data Region Manipulation
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    auto VolumeManager::get_max_cluster_count() const -> U32 {
        return _fat_engine->get_max_cluster_count();
    }

    auto VolumeManager::data_cluster_read(U16                      storage_dev,
                                          VFS::BIOSParameterBlock* bpb,
                                          void*                    buf,
                                          size_t                   cluster) const -> bool {
        U32 cluster_size = bpb->bytes_per_sector * bpb->sectors_per_cluster;
        return _ahci_driver->read(
                   storage_dev,
                   buf,
                   static_cast<size_t>(bpb->bytes_per_sector * bpb->sectors_per_cluster),
                   data_cluster_to_lba(bpb, cluster))
               == cluster_size;
    }

    auto VolumeManager::data_cluster_write(U16                      storage_dev,
                                           VFS::BIOSParameterBlock* bpb,
                                           void*                    buf,
                                           size_t                   cluster) -> bool {
        U32 cluster_size = bpb->bytes_per_sector * bpb->sectors_per_cluster;
        return _ahci_driver->write(storage_dev,
                                   buf,
                                   cluster_size,
                                   data_cluster_to_lba(bpb, cluster))
               == cluster_size;
    }

} // namespace Rune::VFS