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

#include <VirtualFileSystem/FAT/FileEntryManager.h>


namespace Rune::VFS {
    IMPLEMENT_ENUM(VolumeAccessStatus, VOLUME_ACCESS_STATUSES, 0x0)


    FileEntryManager::FileEntryManager(
            SharedPointer<FATEngine> fat_engine,
            Device::AHCIDriver& ahci_driver,
            VolumeManager& volume_manager
    )
            : _fat_engine(move(fat_engine)),
              _ahci_driver(ahci_driver),
              _volume_manager(volume_manager) {

    }


    VolumeAccessStatus FileEntryManager::search(
            U16 storage_dev,
            BIOSParameterBlock* bpb,
            const Path& path,
            LocationAwareFileEntry& out
    ) const {
        U32       root_cluster = _fat_engine->get_root_directory_cluster(bpb);
        FileEntry root_dummy;
        root_dummy.attributes       = FATFileAttribute::DIRECTORY;
        root_dummy.first_cluster_low  = root_cluster & 0xFFFF;
        root_dummy.first_cluster_high = (root_cluster >> 16) & 0xFFFF;

        LinkedList<String> p_split = path.split();
        if (p_split.is_empty() || (p_split.size() == 1 && (*p_split.head() == "." || *p_split.head() == ".."))) {
            out = { "", root_dummy, root_cluster, 0, 0 };
            return VolumeAccessStatus::OKAY;
        } else {
            LinkedListIterator<String> p_it    = p_split.begin();
            NavigationResult           nav_res = FATDirectoryIterator::navigate_to(
                    storage_dev,
                    bpb,
                    _volume_manager,
                    root_dummy.cluster(),
                    p_it
            );
            out = nav_res.file;
            if (nav_res.status == NavigationStatus::FOUND)
                return VolumeAccessStatus::OKAY;
            else if (nav_res.status == NavigationStatus::NOT_FOUND)
                return VolumeAccessStatus::NOT_FOUND;
            else
                return VolumeAccessStatus::DEV_ERROR;
        }
    }


    VolumeAccessStatus FileEntryManager::find_empty_file_entries(
            U16 storage_dev,
            BIOSParameterBlock* bpb,
            const Path& path,
            U16 range,
            LinkedList<LocationAwareFileEntry>& out
    ) {
        U32       root_cluster = _fat_engine->get_root_directory_cluster(bpb);
        FileEntry root_dummy;
        root_dummy.attributes         = FATFileAttribute::DIRECTORY;
        root_dummy.first_cluster_low  = root_cluster & 0xFFFF;
        root_dummy.first_cluster_high = (root_cluster >> 16) & 0xFFFF;

        LinkedList<String>     p_split = path.split();
        LocationAwareFileEntry dir;
        if (p_split.is_empty()) {
            dir = { "", root_dummy, root_cluster, 0, 0 };
        } else {
            // Get the directory file entry
            LinkedListIterator<String> p_it    = p_split.begin();
            NavigationResult           nav_res = FATDirectoryIterator::navigate_to(
                    storage_dev,
                    bpb,
                    _volume_manager,
                    root_dummy.cluster(),
                    p_it
            );
            dir = nav_res.file;

            if (nav_res.status != NavigationStatus::FOUND) {
                if (nav_res.status == NavigationStatus::NOT_FOUND)
                    return VolumeAccessStatus::NOT_FOUND;
                else
                    return VolumeAccessStatus::DEV_ERROR;
            }
        }

        FATDirectoryIterator dIt(
                storage_dev,
                bpb,
                _volume_manager,
                dir.file.first_cluster_high << 16 | dir.file.first_cluster_low,
                DirectoryIterationMode::LIST_ALL
        );
        bool last_e_5    = false;
        U16  range_found = 0;
        while (dIt.has_next() && range_found < range) {
            LocationAwareFileEntry cEntry = *dIt;
            if (!cEntry.file.is_empty_end()) {
                if (!cEntry.file.is_empty_middle()) {
                    // Skip used file entry -> Reset the E5 range search
                    last_e_5    = false;
                    range_found = 0;
                    out.clear();
                    ++dIt;
                    continue;
                }
                if (!last_e_5)
                    last_e_5 = true;

                range_found++;
                out.add_back(cEntry);
            } else {
                // We are at the end of the directories used entries, all following entries will always be unused,
                // the caller must ensure that there is enough space for the required amount of empty entries
                // -> Just gather all the needed entries without checking anything
                range_found++;
                out.add_back(cEntry);
            }
            ++dIt;
        }

        if (range_found < range) {
            size_t required_space  = range - range_found;
            size_t cluster_size      = bpb->bytes_per_sector * bpb->sectors_per_cluster;
            U32 first_new_cluster = 0;
            U32 current_cluster   = dIt.get_current_cluster();
            // Allocate new clusters until we have enough free space for the rest of the file entries
            while (required_space > 0) {
                U32 next_cluster = allocate_cluster(storage_dev, bpb, dir, current_cluster);
                if (next_cluster >= _fat_engine->get_max_cluster_count())
                    return VolumeAccessStatus::DEV_ERROR; // Cannot allocate enough clusters

                if (first_new_cluster == 0)
                    // Save first allocated cluster, so we can start iteration from here
                    first_new_cluster = next_cluster;
                current_cluster = next_cluster;
                // Underflow protection
                required_space      = required_space < cluster_size / sizeof(FileEntry)
                                      ? 0
                                      : required_space - cluster_size / sizeof(FileEntry);
            }

            // Add more free entries to "out"
            FATDirectoryIterator after_end(
                    storage_dev,
                    bpb,
                    _volume_manager,
                    first_new_cluster,
                    DirectoryIterationMode::LIST_ALL
            );
            while (after_end.has_next() && range_found < range) {
                out.add_back(*after_end);
                range_found++;
                ++after_end;
            }
        }

        return VolumeAccessStatus::OKAY;
    }


    bool FileEntryManager::update(U16 storage_dev, BIOSParameterBlock* bpb, const LocationAwareFileEntry& entry) {
        U8 buf[bpb->bytes_per_sector * bpb->sectors_per_cluster];
        if (!_volume_manager.data_cluster_read(storage_dev, bpb, buf, entry.location.cluster))
            return false;
        auto* file_cluster = (FileEntry*) buf;
        file_cluster[entry.location.entry_idx] = entry.file;
        return _volume_manager.data_cluster_write(storage_dev, bpb, buf, entry.location.cluster);
    }


    U32 FileEntryManager::allocate_cluster(
            U16 storage_dev,
            BIOSParameterBlock* bpb,
            LocationAwareFileEntry& file,
            U32 last_file_cluster
    ) {
        U32 free_cluster = _volume_manager.fat_find_next_free_cluster(storage_dev, bpb);
        if (free_cluster == 0)
            return 0;

        if (last_file_cluster == 0) {
            // File of length zero -> Update the file entry and the FAT
            // Note: Regarding the root cluster it is never empty as the root sector is always implicitly allocated
            //          in the BPB, therefore this case can never occur, and we will never accidentally update the
            //          non-existing root file entry, thus provoking the end of the world
            file.file.first_cluster_low  = free_cluster & 0xFFFF;
            file.file.first_cluster_high = (free_cluster >> 16) & 0xFFFF;
            if (!update(storage_dev, bpb, file))
                return 0;
            if (!_volume_manager.fat_write(storage_dev, bpb, free_cluster, _fat_engine->fat_get_eof_marker())) {
                file.file.first_cluster_low  = 0;
                file.file.first_cluster_high = 0;
                update(storage_dev, bpb, file);
                return 0;
            }
        } else {
            // File has at least one cluster -> Update the last file cluster in the FAT to point to the new cluster
            // and update the new cluster in the FAT to the EOF marker
            if (!_volume_manager.fat_write(storage_dev, bpb, last_file_cluster, free_cluster))
                return 0;
            if (!_volume_manager.fat_write(storage_dev, bpb, free_cluster, _fat_engine->fat_get_eof_marker())) {
                _volume_manager.fat_write(storage_dev, bpb, last_file_cluster, _fat_engine->fat_get_eof_marker());
                return 0;
            }
        }
        return free_cluster;
    }

}