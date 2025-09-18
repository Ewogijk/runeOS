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

#ifndef RUNEOS_FILEENTRYMANAGER_H
#define RUNEOS_FILEENTRYMANAGER_H

#include <Ember/Enum.h>
#include <KernelRuntime/Path.h>

#include <VirtualFileSystem/FAT/FAT.h>
#include <VirtualFileSystem/FAT/FATDirectoryIterator.h>
#include <VirtualFileSystem/FAT/VolumeManager.h>

namespace Rune::VFS {

#define VOLUME_ACCESS_STATUSES(X)                                                                  \
    X(VolumeAccessStatus, OKAY, 0x1)                                                               \
    X(VolumeAccessStatus, NOT_FOUND, 0x2)                                                          \
    X(VolumeAccessStatus, BAD_PATH, 0x3)                                                           \
    X(VolumeAccessStatus, DEV_ERROR, 0x4)

    DECLARE_ENUM(VolumeAccessStatus, VOLUME_ACCESS_STATUSES, 0x0) // NOLINT

    /**
     * High level search and manipulations of FAT file entries.
     */
    class FileEntryManager {
        SharedPointer<FATEngine> _fat_engine;
        Device::AHCIDriver&      _ahci_driver;
        VolumeManager&           _volume_manager;

      public:
        explicit FileEntryManager(SharedPointer<FATEngine> fat_engine,
                                  Device::AHCIDriver&      ahci_driver,
                                  VolumeManager&           volume_manager);

        /**
         * Search for the file entry at the given path.
         *
         * @param storage_dev
         * @param bpb
         * @param path      Path to a file entry.
         * @param out       If the VolumeAccessStatus is Okay, the file entry will be placed in this
         * variable. If not the variable is not modified.
         *
         * @return Status of the volume access.
         */
        VolumeAccessStatus search(U16                     storage_dev,
                                  BIOSParameterBlock*     bpb,
                                  const Path&             path,
                                  LocationAwareFileEntry& out) const;

        /**
         * Search for the file entry at the given path, check if it is a directory. If yes a
         * coherent range with requested size of empty file entries in the directory will be placed
         * in the out list.
         *
         * <p>
         *  If the directory does not any coherent range of file entries that is big enough, new
         * clusters will be allocated until the requirement is satisfied.
         * </p>
         *
         * @param storage_dev
         * @param bpb
         * @param path      Path to a directory file entry.
         * @param rangeSize Size of the range.
         * @param out       Will contain empty file entries that are next to each other oor will not
         * be modified if an error occurs.
         *
         * @return Status of the volume access.
         */
        VolumeAccessStatus find_empty_file_entries(U16                                 storage_dev,
                                                   BIOSParameterBlock*                 bpb,
                                                   const Path&                         path,
                                                   U16                                 range,
                                                   LinkedList<LocationAwareFileEntry>& out);

        /**
         * Get the file entry from the storage device that the given entry points to and update it
         * with the content of given entry.
         *
         * @param storage_dev
         * @param bpb
         * @param entry     An file entry with the same path as this one will be searched and
         * updated to exactly match this entry.
         *
         * @return True: The file entry is updated. False: It is not.
         */
        bool update(U16 storage_dev, BIOSParameterBlock* bpb, const LocationAwareFileEntry& entry);

        /**
         * allocate a new cluster for the given file.
         *
         * @param storage_dev
         * @param bpb
         * @param file
         * @param last_file_cluster Currently last cluster of the file. Is zero for empty files.
         *
         * @return Index of the allocated cluster. 0 if no free cluster was found.
         */
        U32 allocate_cluster(U16                     storage_dev,
                             BIOSParameterBlock*     bpb,
                             LocationAwareFileEntry& file,
                             U32                     last_file_cluster);
    };
} // namespace Rune::VFS

#endif // RUNEOS_FILEENTRYMANAGER_H
