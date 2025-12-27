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

#ifndef RUNEOS_FATNODE_H
#define RUNEOS_FATNODE_H

#include <VirtualFileSystem/Path.h>

#include <VirtualFileSystem/Node.h>

#include <VirtualFileSystem/FAT/FAT.h>
#include <VirtualFileSystem/FAT/FileEntryManager.h>
#include <VirtualFileSystem/FAT/VolumeManager.h>

namespace Rune::VFS {
    class FATNode : public Node {
        Path                         _path;
        Ember::IOMode                _node_io_mode;
        LocationAwareFileEntry       _file_entry;
        VolumeManager*               _volume_manager;
        FileEntryManager*            _file_entry_manager;
        SharedPointer<StorageDevRef> _mounted_storage;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          File Cursor
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        // Number of clusters the cursor has already pointed at
        U32 _processed_clusters{0};
        // Cluster the cursor is pointing at
        U32 _current_cluster{0};
        // Byte inside the current cluster the cursor is pointing at
        U32 _cluster_offset{0};

        void init_file_cursor();

        [[nodiscard]] auto processed_bytes() const -> U32;

      public:
        FATNode(Function<void()>             on_close,
                Path                         path,
                Ember::IOMode                node_io_mode,
                LocationAwareFileEntry       file_entry,
                VolumeManager*               volume_manager,
                FileEntryManager*            file_entry_manager,
                SharedPointer<StorageDevRef> mounted_storage);

        ~FATNode() override = default;

        [[nodiscard]] auto get_node_path() const -> Path override;

        [[nodiscard]] auto get_io_mode() const -> Ember::IOMode override;

        [[nodiscard]] auto get_size() const -> size_t override;

        [[nodiscard]] auto has_more() const -> bool override;

        auto read(void* buf, size_t buf_size) -> NodeIOResult override;

        auto write(void* buf, size_t buf_size) -> NodeIOResult override;

        auto seek(Ember::SeekMode seek_mode, int offset) -> NodeIOResult override;

        [[nodiscard]] auto has_attribute(Ember::NodeAttribute f_attr) const -> bool override;

        auto set_attribute(Ember::NodeAttribute n_attr, bool val) -> bool override;
    };
} // namespace Rune::VFS

#endif // RUNEOS_FATNODE_H
