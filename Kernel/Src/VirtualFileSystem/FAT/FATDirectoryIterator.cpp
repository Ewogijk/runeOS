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

#include <VirtualFileSystem/FAT/FATDirectoryIterator.h>

namespace Rune::VFS {
    DEFINE_ENUM(DirectoryIteratorState, DIRECTORY_ITERATOR_STATES, 0x0)

    void FATDirectoryIterator::get_next_cluster() {
        // If the iterator has not started yet read the cluster mentioned in the directory file
        // entry else get the next cluster from the FAT
        U32 next_cluster = _current_entry
                               ? _volume_manager.fat_read(_storage_dev, _bpb, _current_cluster)
                               : _current_cluster;
        if (next_cluster > _volume_manager.get_max_cluster_count()) {
            _state = DirectoryIteratorState::END_OF_DIRECTORY;
            return;
        }
        if (!_volume_manager.data_cluster_read(_storage_dev,
                                               _bpb,
                                               _cluster_buf.get(),
                                               next_cluster)) {
            _state = DirectoryIteratorState::DEV_ERROR;
            return;
        }
        _current_cluster = next_cluster;
        _entry_index     = 0;
        _current_entry   = (FileEntry*) _cluster_buf.get();
    }

    void FATDirectoryIterator::advance() {
        _entry_index++;
        _current_entry = &((FileEntry*) _cluster_buf.get())[_entry_index];
        if (_it_mode == DirectoryIterationMode::LIST_DIRECTORY && _current_entry->is_empty_end()) {
            // We reached the end of the directory and want to stop here
            _state = DirectoryIteratorState::END_OF_DIRECTORY;
            return;
        }
        _current_entry_as_laf.file_name       = "";
        _current_entry_as_laf.file            = *_current_entry;
        _current_entry_as_laf.location        = {_current_cluster, (U16) _entry_index};
        _current_entry_as_laf.first_lfn_entry = {};
    }

    void FATDirectoryIterator::parse_used_file_entry() {
        String            file_name = "";
        FileEntryLocation first_lfn_entry;
        // The current file entry is a used file or directory
        if (_current_entry->attributes == FATFileAttribute::LONG_FILE_NAME) {
            // Parse it's long file name entries
            if ((_current_entry->short_name.as_array[0] & 0xF0)
                != LongFileNameEntry::LAST_LFN_ENTRY) {
                _state = DirectoryIteratorState::CORRUPT_LFN_ENTRY;
                return;
            }
            first_lfn_entry = {_current_cluster, (U16) _entry_index};
            U8 lfne_count   = _current_entry->short_name.as_array[0] & 0x0F;
            U8 c_order      = lfne_count;
            while (c_order > 0) {
                auto* c_long_file_name_entry = (LongFileNameEntry*) _current_entry;
                if ((c_long_file_name_entry->order & 0x0F) != c_order) {
                    _state = DirectoryIteratorState::CORRUPT_LFN_ENTRY;
                    return;
                }
                // Copy file name to temp buffer
                // Encoding is UCS-2/UTF-16, but we only get the first byte of each char
                // because the kernel only supports ASCII (for ASCII the second byte is always 0
                // padding) Should a non ASCII character be part of the name we will embrace chaos
                char tmp[13];
                for (int i = 0; i < 5; i++)
                    tmp[i] = (char) (c_long_file_name_entry->file_name_1[i] & 0x00FF);
                for (int i = 0; i < 6; i++)
                    tmp[5 + i] = (char) (c_long_file_name_entry->file_name_2[i] & 0x00FF);
                for (int i = 0; i < 2; i++)
                    tmp[11 + i] = (char) (c_long_file_name_entry->file_name_3[i] & 0x00FF);

                int pStart = 0;
                int pEnd   = 13;
                if (c_order == lfne_count) {
                    // Skip trailing whitespace
                    while (tmp[pEnd - 1] == (char) 0xFF
                           || tmp[pEnd - 1] == '\0') // string will implicitly be null terminated
                        pEnd--;
                } else if (c_order == 1) {
                    // Skip leading whitespace
                    while (tmp[pStart] == ' ') pStart++;
                }

                // Long file name entries are in reverse order
                file_name = String(tmp, pStart, pEnd) + file_name;
                _current_entry++;
                _entry_index++;
                c_order--;
                if (_entry_index >= _max_entries_per_cluster) {
                    // The next long file name entry is located in the next directory cluster -> Get
                    // it!
                    get_next_cluster();
                    if (_state != DirectoryIteratorState::ITERATING)
                        return; // StorageError or EndOfDirectory
                }
            }
        } else {
            // It does not use a long file name entry
            file_name = _current_entry->make_short_name();
        }
        _current_entry_as_laf.file_name       = file_name;
        _current_entry_as_laf.file            = *_current_entry;
        _current_entry_as_laf.location        = {_current_cluster, (U16) _entry_index};
        _current_entry_as_laf.first_lfn_entry = first_lfn_entry;
    }

    FATDirectoryIterator::FATDirectoryIterator(U16                    storage_dev,
                                               BIOSParameterBlock*    bpb,
                                               const VolumeManager&   volume_manager,
                                               U32                    start_cluster,
                                               DirectoryIterationMode it_mode)
        : _storage_dev(storage_dev),
          _bpb(bpb),
          _volume_manager(volume_manager),
          _current_cluster(0),
          _cluster_buf(nullptr),
          _max_entries_per_cluster(0),
          _current_entry(nullptr),
          _entry_index(0),
          _current_entry_as_laf(),
          _state(DirectoryIteratorState::ITERATING),
          _it_mode(it_mode) {

        if (_it_mode == DirectoryIterationMode::NONE) {
            // None iteration mode is illegal!
            _state = DirectoryIteratorState::END_OF_DIRECTORY;
        } else {
            // Configure the iterator and move to the first file entry to be returned
            U32 cluster_size         = bpb->bytes_per_sector * bpb->sectors_per_cluster;
            _current_cluster         = start_cluster;
            _cluster_buf             = SharedPointer<U8>(new U8[cluster_size]);
            _max_entries_per_cluster = (int) (cluster_size / sizeof(FileEntry));
            // Load the very first cluster -> Can transition only to "StorageError" state because
            // there must always be a "dot" and "dotdot"
            get_next_cluster();
            if (_state != DirectoryIteratorState::DEV_ERROR) {
                _entry_index = -1;
                ++(*this);
            }
        }
    }

    NavigationResult FATDirectoryIterator::navigate_to(U16                         storage_dev,
                                                       BIOSParameterBlock*         bpb,
                                                       const VolumeManager&        volume_manager,
                                                       U32                         start_cluster,
                                                       LinkedListIterator<String>& path) {
        String wanted_file_entry = *path;
        ++path;
        FATDirectoryIterator d_it(storage_dev,
                                  bpb,
                                  volume_manager,
                                  start_cluster,
                                  DirectoryIterationMode::LIST_DIRECTORY);
        while (d_it.has_next()) {
            LocationAwareFileEntry c_entry = *d_it;
            if (c_entry.file_name == wanted_file_entry) {
                if (!path.has_next()) {
                    // Reached end of patch and file entry found -> return file entry
                    return {NavigationStatus::FOUND, c_entry};
                } else {
                    // More path left -> Recursively search for the next part in the directory found
                    // here
                    if (FATFileAttribute(c_entry.file.attributes & FATFileAttribute::DIRECTORY)
                        == FATFileAttribute::DIRECTORY) {
                        return navigate_to(storage_dev,
                                           bpb,
                                           volume_manager,
                                           c_entry.file.cluster(),
                                           path);
                    }
                    return {NavigationStatus::BAD_PATH, {}};
                }
            }
            ++d_it;
        }

        if (d_it.get_state() == DirectoryIteratorState::END_OF_DIRECTORY)
            return {NavigationStatus::NOT_FOUND, {}};

        return {NavigationStatus::DEV_ERROR, {}};
    }

    bool FATDirectoryIterator::has_next() const {
        return _state == DirectoryIteratorState::ITERATING;
    }

    LocationAwareFileEntry& FATDirectoryIterator::operator*() { return _current_entry_as_laf; }

    LocationAwareFileEntry* FATDirectoryIterator::operator->() { return &_current_entry_as_laf; }

    FATDirectoryIterator& FATDirectoryIterator::operator++() {
        if (_state != DirectoryIteratorState::ITERATING) return *this;

        if (_entry_index >= _max_entries_per_cluster - 1)
            // We are at the end of a directory cluster but not end of the directory -> Read next
            // cluster
            get_next_cluster();
        else
            // advance the file cursor by one
            advance();

        if (_state != DirectoryIteratorState::ITERATING)
            // We reached the end of directory or an error happened
            return *this;

        switch (_it_mode) {
            case DirectoryIterationMode::LIST_DIRECTORY:
                // Skip all empty file entries in between used entries
                while (_current_entry->is_empty_middle()) {
                    advance();
                    if (_state != DirectoryIteratorState::ITERATING) return *this;
                }

                // A used file entry is reached
                parse_used_file_entry();
                break;
            case DirectoryIterationMode::LIST_ALL:
                if (!_current_entry->is_empty_middle() && !_current_entry->is_empty_end())
                    parse_used_file_entry();
                // else file entry is empty -> LocationAwareFileEntry already set by "advance"
                break;
            case DirectoryIterationMode::ATOMIC:
                // We return each file entry individually (no LFN entry parsing) ->
                // LocationAwareFileEntry already set by "advance"
            case DirectoryIterationMode::NONE: break;
        }

        return *this;
    }

    FATDirectoryIterator FATDirectoryIterator::operator++(int) {
        FATDirectoryIterator tmp = *this;
        ++(*this);
        return tmp;
    }

    bool FATDirectoryIterator::operator==(const FATDirectoryIterator& o) const {
        return _current_cluster == o._current_cluster && _entry_index == o._entry_index;
    }

    bool FATDirectoryIterator::operator!=(const FATDirectoryIterator& o) const {
        return _current_cluster != o._current_cluster || _entry_index != o._entry_index;
    }

    DirectoryIteratorState FATDirectoryIterator::get_state() const { return _state; }

    U32 FATDirectoryIterator::get_current_cluster() const { return _current_cluster; }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      FAT Directory Stream
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    FATDirectoryStream::FATDirectoryStream(const Function<void()>&     on_close,
                                           const FATDirectoryIterator& fat_it)
        : DirectoryStream(move(on_close)),
          _fat_it(move(fat_it)) {
    }

    Expected<NodeInfo, DirectoryStreamStatus> FATDirectoryStream::next() {
        switch (_fat_it.get_state()) {
            case DirectoryIteratorState::END_OF_DIRECTORY:
                return Unexpected<DirectoryStreamStatus>(DirectoryStreamStatus::END_OF_DIRECTORY);
            case DirectoryIteratorState::CORRUPT_LFN_ENTRY:
            case DirectoryIteratorState::DEV_ERROR:
                return Unexpected<DirectoryStreamStatus>(DirectoryStreamStatus::IO_ERROR);
            default:
                break; // ITERATING -> continue get node info
        }

        U8 node_attr = 0;
        if (_fat_it->file.has_attribute(FATFileAttribute::READONLY))
            node_attr |= Ember::NodeAttribute::READONLY;
        if (_fat_it->file.has_attribute(FATFileAttribute::HIDDEN))
            node_attr |= Ember::NodeAttribute::HIDDEN;
        if (_fat_it->file.has_attribute(FATFileAttribute::SYSTEM))
            node_attr |= Ember::NodeAttribute::SYSTEM;
        if (_fat_it->file.has_attribute(FATFileAttribute::DIRECTORY))
            node_attr |= Ember::NodeAttribute::DIRECTORY;
        if (_fat_it->file.has_attribute(FATFileAttribute::ARCHIVE))
            node_attr |= Ember::NodeAttribute::FILE;
        NodeInfo node_info = NodeInfo{_fat_it->file_name, _fat_it->file.file_size, node_attr};
        ++_fat_it;
        return node_info;
    }
} // namespace Rune::VFS