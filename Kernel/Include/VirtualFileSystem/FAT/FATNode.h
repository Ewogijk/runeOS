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


#include <Hammer/Path.h>

#include <VirtualFileSystem/Node.h>

#include <VirtualFileSystem/FAT/FAT.h>
#include <VirtualFileSystem/FAT/VolumeManager.h>
#include <VirtualFileSystem/FAT/FileEntryManager.h>


namespace Rune::VFS {
    class FATNode : public Node {
        Path                   _path;
        IOMode             _node_io_mode;
        LocationAwareFileEntry _file_entry;
        VolumeManager   & _volume_manager;
        FileEntryManager& _file_entry_manager;
        SharedPointer<StorageDevRef> _mounted_storage;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          File Cursor
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        // Number of clusters the cursor has already pointed at
        U32 _processed_clusters;
        // Cluster the cursor is pointing at
        U32 _current_cluster;
        // Byte inside the current cluster the cursor is pointing at
        U32 _cluster_offset;


        void init_file_cursor();


        [[nodiscard]] U32 processed_bytes() const;


    public:
        FATNode(
                Function<void()> on_close,
                Path path,
                IOMode node_io_mode,
                LocationAwareFileEntry file_entry,
                VolumeManager& volume_manager,
                FileEntryManager& file_entry_manager,
                SharedPointer<StorageDevRef> mounted_storage
        );


        ~FATNode() override = default;


        [[nodiscard]] Path get_node_path() const override;


        [[nodiscard]] IOMode get_io_mode() const override;


        [[nodiscard]] size_t get_size() const override;


        [[nodiscard]] bool has_more() const override;


        NodeIOResult read(void* buf, size_t buf_size) override;


        NodeIOResult write(void* buf, size_t buf_size) override;


        NodeIOResult seek(SeekMode seek_mode, int offset) override;


        [[nodiscard]] bool has_attribute(NodeAttribute f_attr) const override;


        bool set_attribute(NodeAttribute n_attr, bool val) override;
    };
}

#endif //RUNEOS_FATNODE_H
