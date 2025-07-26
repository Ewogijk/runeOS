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

#include <VirtualFileSystem/FAT/FATNode.h>

#include <Hammer/Math.h>

#include <VirtualFileSystem/FAT/FATDirectoryIterator.h>


namespace Rune::VFS {
    void FATNode::init_file_cursor() {
        U32 i                 = 0;
        U32 last_file_cluster = _file_entry.file.first_cluster_high << 16 | _file_entry.file.first_cluster_low;
        U32 cluster           = last_file_cluster;
        while (cluster != 0 && cluster < _volume_manager.get_max_cluster_count() + 1) {
            last_file_cluster = cluster;
            cluster           = _volume_manager.fat_read(_mounted_storage->storage_dev, _mounted_storage->BPB, cluster);
            i++;
        }

        // Move the cluster cursor
        if (_node_io_mode == Ember::IOMode::APPEND) {
            U32 cluster_size    = _mounted_storage->BPB->bytes_per_sector * _mounted_storage->BPB->sectors_per_cluster;
            _processed_clusters = i;
            if (_file_entry.file.file_size % cluster_size != 0)
                _processed_clusters--;
            _current_cluster = last_file_cluster;
            _cluster_offset  = _file_entry.file.file_size - _processed_clusters * cluster_size;
        } else {
            _processed_clusters = 0;
            _current_cluster    = _file_entry.file.first_cluster_high << 16 | _file_entry.file.first_cluster_low;
            _cluster_offset     = 0;
        }
    }


    U32 FATNode::processed_bytes() const {
        return _processed_clusters * (_mounted_storage->BPB->bytes_per_sector * _mounted_storage->BPB
                ->sectors_per_cluster)
            + _cluster_offset;
    }


    FATNode::FATNode(
        Function<void()>             on_close,
        Path                         path,
        Ember::IOMode                node_io_mode,
        LocationAwareFileEntry       file_entry,
        VolumeManager&               volume_manager,
        FileEntryManager&            file_entry_manager,
        SharedPointer<StorageDevRef> mounted_storage
    ) : Node(move(on_close)),
        _path(move(path)),
        _node_io_mode(node_io_mode),
        _file_entry(move(file_entry)),
        _volume_manager(volume_manager),
        _file_entry_manager(file_entry_manager),
        _mounted_storage(move(mounted_storage)),
        _processed_clusters(0),
        _current_cluster(0),
        _cluster_offset(0) {
        init_file_cursor();
    }


    Path FATNode::get_node_path() const {
        return _path;
    }


    Ember::IOMode FATNode::get_io_mode() const {
        return _node_io_mode;
    }


    size_t FATNode::get_size() const {
        return _file_entry.file.file_size;
    }


    bool FATNode::has_more() const {
        if (_closed)
            return false;

        if (!has_attribute(Ember::NodeAttribute::FILE))
            return false;
        return processed_bytes() < _file_entry.file.file_size;
    }


    NodeIOResult FATNode::read(void* buf, size_t buf_size) {
        if (_closed)
            return {NodeIOStatus::CLOSED, 0};

        if (!has_attribute(Ember::NodeAttribute::FILE))
            return {NodeIOStatus::NOT_SUPPORTED, 0};

        if (_file_entry.file.file_size == 0)
            return {NodeIOStatus::OKAY, 0};

        if (!buf)
            return {NodeIOStatus::BAD_ARGS, 0};

        if (buf_size == 0)
            return {NodeIOStatus::OKAY, 0};

        size_t cluster_size = _mounted_storage->BPB->bytes_per_sector * _mounted_storage->BPB->sectors_per_cluster;
        U8     tmp_buf[cluster_size];
        size_t buf_pos = 0;
        while (has_more() && buf_pos < buf_size) {
            if (!_volume_manager.data_cluster_read(
                _mounted_storage->storage_dev,
                _mounted_storage->BPB,
                tmp_buf,
                _current_cluster
            ))
                return {NodeIOStatus::DEV_ERROR, buf_pos};

            // 1. Min: Copy no more bytes than in the file or buffer left
            // 2. Min: Copy no more bytes than in the cluster or "1. Min" left
            size_t b_to_copy = min(
                min((size_t)_file_entry.file.file_size - processed_bytes(), buf_size - buf_pos),
                cluster_size - _cluster_offset
            );
            memcpy(&((U8*)buf)[buf_pos], &tmp_buf[_cluster_offset], b_to_copy);
            _cluster_offset += b_to_copy;
            buf_pos += b_to_copy;

            if (_cluster_offset >= cluster_size) {
                U32 next_cluster = _volume_manager.fat_read(
                    _mounted_storage->storage_dev,
                    _mounted_storage->BPB,
                    _current_cluster
                );
                if (next_cluster >= _volume_manager.get_max_cluster_count() + 1)
                    break;
                _processed_clusters++;
                _current_cluster = next_cluster;
                _cluster_offset  = 0;
            }
        }
        return {NodeIOStatus::OKAY, buf_pos};
    }


    NodeIOResult FATNode::write(void* buf, size_t buf_size) {
        if (_closed)
            return {NodeIOStatus::CLOSED, 0};

        if (!has_attribute(Ember::NodeAttribute::FILE))
            return {NodeIOStatus::NOT_SUPPORTED, 0};

        if (_node_io_mode == Ember::IOMode::READ)
            return {NodeIOStatus::NOT_ALLOWED, 0};

        if (!buf)
            return {NodeIOStatus::BAD_ARGS, 0};

        if (buf_size == 0)
            return {NodeIOStatus::OKAY, 0};

        size_t cluster_size   = _mounted_storage->BPB->bytes_per_sector * _mounted_storage->BPB->sectors_per_cluster;
        size_t buf_pos        = 0;
        bool   is_first_write = _processed_clusters == 0 && _cluster_offset == 0;
        while (buf_pos < buf_size) {
            if (_current_cluster == 0 || _processed_clusters >= div_round_up(
                _file_entry.file.file_size,
                (U32)cluster_size
            )) {
                // End of file reached -> Allocate new cluster
                _current_cluster = _file_entry_manager.allocate_cluster(
                    _mounted_storage->storage_dev,
                    _mounted_storage->BPB,
                    _file_entry,
                    _current_cluster
                );
                if (_current_cluster == 0)
                    return {NodeIOStatus::DEV_ERROR, 0};
            }

            // Read current cluster
            U8 w_buf[cluster_size];
            if (!_volume_manager.data_cluster_read(
                _mounted_storage->storage_dev,
                _mounted_storage->BPB,
                w_buf,
                _current_cluster
            ))
                return {NodeIOStatus::DEV_ERROR, buf_pos};
            // Copy buffer bytes to the cluster buffer
            size_t b_to_copy = min(buf_size - buf_pos, cluster_size - _cluster_offset);
            memcpy(&w_buf[_cluster_offset], &((U8*)buf)[buf_pos], b_to_copy); // _cluster_offset needs update
            if (_node_io_mode == Ember::IOMode::WRITE && is_first_write)
                memset(&w_buf[b_to_copy], 0, cluster_size - b_to_copy);

            // Write updated cluster to volume
            if (!_volume_manager.data_cluster_write(
                _mounted_storage->storage_dev,
                _mounted_storage->BPB,
                w_buf,
                _current_cluster
            ))
                return {NodeIOStatus::DEV_ERROR, 0};
            buf_pos += b_to_copy;
            _cluster_offset += b_to_copy;

            if (_cluster_offset >= cluster_size) {
                // All bytes in the cluster have been updated -> Move to next cluster
                _processed_clusters++;
                _cluster_offset = 0;

                U32 next_cluster = _volume_manager.fat_read(
                    _mounted_storage->storage_dev,
                    _mounted_storage->BPB,
                    _current_cluster
                );
                if (next_cluster < _volume_manager.get_max_cluster_count() + 1)
                    _current_cluster = next_cluster;
            }
        };

        // Update file size
        U32 old_size               = _file_entry.file.file_size;
        _file_entry.file.file_size = _node_io_mode == Ember::IOMode::WRITE && is_first_write
                                         ? buf_pos
                                         : _file_entry.file.file_size + buf_pos;

        if (_file_entry.file.file_size < old_size) {
            // The file shrunk -> Free excess FAT clusters
            U32 total_clusters = div_round_up(_file_entry.file.file_size, (U32)cluster_size);
            U32 cluster        = _file_entry.file.first_cluster_high << 16 | _file_entry.file.first_cluster_low;
            U32 eof_cluster    = 0;
            U32 i              = 0;
            while (cluster != 0 && cluster < _volume_manager.get_max_cluster_count() + 1) {
                U32 next_cluster = _volume_manager.fat_read(
                    _mounted_storage->storage_dev,
                    _mounted_storage->BPB,
                    cluster
                );
                if (i == total_clusters - 1)
                    eof_cluster = cluster;
                else if (i > total_clusters - 1)
                    _volume_manager.fat_write(_mounted_storage->storage_dev, _mounted_storage->BPB, cluster, 0);
                cluster = next_cluster;
                i++;
            }

            if (eof_cluster > 0)
                _volume_manager.fat_write(
                    _mounted_storage->storage_dev,
                    _mounted_storage->BPB,
                    eof_cluster,
                    _volume_manager.fat_get_eof_marker()
                );
        }

        // Update the file entry on the volume
        if (!_file_entry_manager.update(_mounted_storage->storage_dev, _mounted_storage->BPB, _file_entry))
            return {NodeIOStatus::DEV_ERROR, 0};
        return {NodeIOStatus::OKAY, buf_pos};
    }


    NodeIOResult FATNode::seek(const Ember::SeekMode seek_mode, const int offset) {
        if (_closed) return {NodeIOStatus::CLOSED, 0};
        if (!has_attribute(Ember::NodeAttribute::FILE)) return {NodeIOStatus::NOT_SUPPORTED, 0};

        const size_t cluster_size = _mounted_storage->BPB->sectors_per_cluster
            * _mounted_storage->BPB->bytes_per_sector;
        const size_t file_cursor = (_processed_clusters * cluster_size) + _cluster_offset;
        bool         bad_offset  = false;
        switch (seek_mode) {
            case Ember::SeekMode::BEGIN:
                bad_offset = offset < 0 || static_cast<U32>(offset) >= _file_entry.file.file_size;
                break;
            case Ember::SeekMode::CURSOR:
                bad_offset = file_cursor + offset >= _file_entry.file.file_size;
                break;
            case Ember::SeekMode::END:
                bad_offset = offset > 0 || static_cast<U32>(abs(offset)) > _file_entry.file.file_size;
                break;
            default: break;
        }
        if (bad_offset) return {NodeIOStatus::BAD_ARGS, 0};

        size_t to_seek = offset;
        switch (seek_mode) {
            case Ember::SeekMode::BEGIN:
                to_seek = offset;
                break;
            case Ember::SeekMode::CURSOR:
                to_seek = file_cursor + offset;
                break;
            case Ember::SeekMode::END:
                to_seek = _file_entry.file.file_size + offset;
                break;
            default: break;
        }
        // Hyper Optimization: Buffer the FAT clusters -> These vars can be instantly computed
        // _processed_clusters = bytePos / clusterSize
        // _current_cluster = clusterBuffer[_processed_clusters]
        // _cluster_offset = bytePos % clusterSize
        _processed_clusters = 0;
        _current_cluster    = 0;
        _cluster_offset     = 0;
        U32 cluster         = _file_entry.file.first_cluster_high << 16 | _file_entry.file.first_cluster_low;
        _current_cluster    = cluster;
        while (to_seek > 0) {
            if (to_seek < cluster_size) {
                _cluster_offset = to_seek;
                to_seek         = 0;
            } else {
                cluster = _volume_manager.fat_read(_mounted_storage->storage_dev, _mounted_storage->BPB, cluster);
                if (cluster == 0 && cluster >= _volume_manager.get_max_cluster_count() + 1)
                    return {NodeIOStatus::DEV_ERROR, offset - to_seek};

                _processed_clusters++;
                _current_cluster = cluster;
                _cluster_offset  = 0;
                to_seek -= cluster_size;
            }
        }
        return {NodeIOStatus::OKAY, abs(offset) - to_seek};
    }


    bool FATNode::has_attribute(Ember::NodeAttribute f_attr) const {
        if (_closed)
            return false;
        FATFileAttribute fat_attr;
        switch (f_attr) {
            case Ember::NodeAttribute::READONLY:
                fat_attr = FATFileAttribute::READONLY;
                break;
            case Ember::NodeAttribute::HIDDEN:
                fat_attr = FATFileAttribute::HIDDEN;
                break;
            case Ember::NodeAttribute::SYSTEM:
                fat_attr = FATFileAttribute::SYSTEM;
                break;
            case Ember::NodeAttribute::DIRECTORY:
                fat_attr = FATFileAttribute::DIRECTORY;
                break;
            case Ember::NodeAttribute::FILE:
                fat_attr = FATFileAttribute::ARCHIVE;
                break;
            default:
                fat_attr = FATFileAttribute::NONE;
        }
        return _file_entry.file.has_attribute(fat_attr);
    }


    bool FATNode::set_attribute(Ember::NodeAttribute n_attr, bool val) {
        if (_closed)
            return false;
        if (n_attr == Ember::NodeAttribute::FILE || n_attr == Ember::NodeAttribute::DIRECTORY)
            return false;
        if (val)
            _file_entry.file.attributes |= n_attr;
        else
            _file_entry.file.attributes &= ~n_attr;
        return _file_entry_manager.update(_mounted_storage->storage_dev, _mounted_storage->BPB, _file_entry);
    }
}
