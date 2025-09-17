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

#ifndef RUNEOS_FATDRIVER_H
#define RUNEOS_FATDRIVER_H

#include <Device/AHCI/AHCI.h>

#include <VirtualFileSystem/Driver.h>

#include <VirtualFileSystem/FAT/FAT.h>
#include <VirtualFileSystem/FAT/FATEngine.h>
#include <VirtualFileSystem/FAT/FileEntryManager.h>

namespace Rune::VFS {

    class FATDriver : public Driver {
        LinkedList<SharedPointer<StorageDevRef>> _storage_dev_ref_table;
        SharedPointer<FATEngine>                 _fat_engine;

        VolumeManager    _volume_manager;
        FileEntryManager _file_entry_manager;

        Device::AHCIDriver& _ahci_driver;

        [[nodiscard]]
        SharedPointer<StorageDevRef> find_storage_dev_ref(U16 storage_dev) const;

        static U8 node_attributes_to_fat_file_attributes(U8 node_attr);

        [[nodiscard]]
        IOStatus exists(const SharedPointer<StorageDevRef>& md, const Path& path) const;

        IOStatus make_long_file_name_entries(const SharedPointer<StorageDevRef>& md,
                                             const Path&                         path,
                                             LinkedList<LocationAwareFileEntry>& out);

        IOStatus
        create_file(const SharedPointer<StorageDevRef>& md, const Path& path, U8 attributes);

        IOStatus
        create_directory(const SharedPointer<StorageDevRef>& md, const Path& path, U8 attributes);

        IOStatus delete_file(const SharedPointer<StorageDevRef>& md, LocationAwareFileEntry& file);

        IOStatus delete_directory(const SharedPointer<StorageDevRef>& md,
                                  LocationAwareFileEntry&             dir,
                                  const Path&                         path);

      public:
        explicit FATDriver(SharedPointer<FATEngine> fat_engine, Device::AHCIDriver& ahci_driver);

        ~FATDriver() override = default;

        [[nodiscard]]
        String get_name() const override;

        FormatStatus format(U16 storage_dev) override;

        MountStatus mount(U16 storage_dev) override;

        MountStatus unmount(U16 storage_dev) override;

        [[nodiscard]]
        bool is_valid_file_path(const Path& path) const override;

        IOStatus create(U16 storage_dev, const Path& path, U8 attributes) override;

        IOStatus open(U16                  storage_dev,
                      const Path&          mount_point,
                      const Path&          path,
                      Ember::IOMode        node_io_mode,
                      Function<void()>     on_close,
                      SharedPointer<Node>& out) override;

        IOStatus find_node(U16 storage_dev, const Path& path, NodeInfo& out) override;

        IOStatus delete_node(U16 storage_dev, const Path& path) override;

        IOStatus open_directory_stream(U16                             storage_dev,
                                       const Path&                     path,
                                       const Function<void()>&         on_close,
                                       SharedPointer<DirectoryStream>& out) override;
    };
} // namespace Rune::VFS

#endif // RUNEOS_FATDRIVER_H
