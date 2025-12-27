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

#ifndef RUNEOS_FATDIRECTORYITERATOR_H
#define RUNEOS_FATDIRECTORYITERATOR_H

#include <Ember/Ember.h>
#include <VirtualFileSystem/Path.h>

#include <VirtualFileSystem/DirectoryStream.h>

#include <VirtualFileSystem/FAT/VolumeManager.h>

namespace Rune::VFS {

    /**
     * <ul>
     *  <li>ITERATING: Directory is still iterated. No Errors.</li>
     *  <li>END_OF_DIRECTORY: Last file entry has been returned. End is iteration mode dependant,
     * could be last used file entry or end of last allocated cluster.</li> <li>CORRUPT_LFN_ENTRY:
     * Corrupt long file name entry encountered. Iteration is stopped.</li> <li>DEV_ERROR: Error of
     * the underlying storage device. Iteration is stopped.</li>
     * </ul>
     */
#define DIRECTORY_ITERATOR_STATES(X)                                                               \
    X(DirectoryIteratorState, ITERATING, 0x1)                                                      \
    X(DirectoryIteratorState, END_OF_DIRECTORY, 0x2)                                               \
    X(DirectoryIteratorState, CORRUPT_LFN_ENTRY, 0x3)                                              \
    X(DirectoryIteratorState, DEV_ERROR, 0x4)

    DECLARE_ENUM(DirectoryIteratorState, DIRECTORY_ITERATOR_STATES, 0x0) // NOLINT

    /**
     * Modes of iteration define, how a directory is iterated:
     * <ul>
     *  <li>LIST_DIRECTORY: Iterate over all used file entries. Long file name and empty entries are
     * not returned. Iteration stops at the last used file entry.</li> <li>LIST_ALL: Iterate over
     * all file entries including empty entries. Long file name entries are not returned. Iteration
     * stops at the end of all allocated clusters.</li> <li>ATOMIC: Iterate atomically over all file
     * entries, that is each used, empty and long file name entry is returned. Iteration stops at
     * the end of all allocated clusters.</li>
     * </ul>
     */
#define DIRECTORY_ITERATION_MODES(X)                                                               \
    X(DirectoryIterationMode, LIST_DIRECTORY, 0x1)                                                 \
    X(DirectoryIterationMode, LIST_ALL, 0x2)                                                       \
    X(DirectoryIterationMode, ATOMIC, 0x3)

    DECLARE_ENUM(DirectoryIterationMode, DIRECTORY_ITERATION_MODES, 0x0) // NOLINT

    /**
     * Modes of iteration define, how a directory is iterated:
     * <ul>
     *  <li>FOUND: File/Directory found.</li>
     *  <li>NOT_FOUND: File/Directory not found.</li>
     *  <li>BAD_PATH: A file has been found in the middle of the path instead of a directory.</li>
     *  <li>DEV_ERROR: Error on the underlying storage device.</li>
     * </ul>
     */
#define NAVIGATION_STATUSES(X)                                                                     \
    X(NavigationStatus, FOUND, 0x1)                                                                \
    X(NavigationStatus, NOT_FOUND, 0x2)                                                            \
    X(NavigationStatus, BAD_PATH, 0x3)                                                             \
    X(NavigationStatus, DEV_ERROR, 0x3)

    DECLARE_ENUM(NavigationStatus, NAVIGATION_STATUSES, 0x0) // NOLINT

    /**
     * Status and possibly the target file (Status == Found).
     */
    struct NavigationResult {
        NavigationStatus       status = NavigationStatus::NONE;
        LocationAwareFileEntry file   = {};
    };

    class FATDirectoryIterator {
        U16                  _storage_dev;
        BIOSParameterBlock*  _bpb;
        const VolumeManager* _volume_manager;

        U32               _current_cluster{0};
        SharedPointer<U8> _cluster_buf;

        int                    _max_entries_per_cluster{0};
        FileEntry*             _current_entry{nullptr};
        int                    _entry_index{0};
        LocationAwareFileEntry _current_entry_as_laf;

        DirectoryIteratorState _state;
        DirectoryIterationMode _it_mode;

        /**
         * Read the next directory cluster.
         */
        void get_next_cluster();

        /**
         * Advance atomically by one file entry.
         */
        void advance();

        /**
         * Advance to the next used or unused file entry, if the file entry has a long file name,
         * all long file entries are also parsed in this step.
         */
        void parse_used_file_entry();

      public:
        FATDirectoryIterator(U16                    storage_dev,
                             BIOSParameterBlock*    bpb,
                             const VolumeManager*   volume_manager,
                             U32                    start_cluster,
                             DirectoryIterationMode it_mode);

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          Static Functions
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        static auto navigate_to(U16                         storage_dev,
                                BIOSParameterBlock*         bpb,
                                const VolumeManager*        volume_manager,
                                U32                         start_cluster,
                                LinkedListIterator<String>& path) -> NavigationResult;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          Iterator Functions
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        [[nodiscard]] auto has_next() const -> bool;

        auto operator*() -> LocationAwareFileEntry&;

        auto operator->() -> LocationAwareFileEntry*;

        // pre-increment
        auto operator++() -> FATDirectoryIterator&;

        // post-increment
        auto operator++(int) -> FATDirectoryIterator;

        auto operator==(const FATDirectoryIterator& o) const -> bool;

        auto operator!=(const FATDirectoryIterator& o) const -> bool;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                  Directory Iterator Specific Functions
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        /**
         *
         * @return Current state of the iterator.
         */
        [[nodiscard]] auto get_state() const -> DirectoryIteratorState;

        /**
         *
         * @return Current cluster that is currently being iterated.
         */
        [[nodiscard]] auto get_current_cluster() const -> U32;
    };

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      FAT Directory Stream
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    class FATDirectoryStream : public DirectoryStream {
        FATDirectoryIterator _fat_it;

        void update_state();

      public:
        explicit FATDirectoryStream(const Function<void()>& on_close,

                                    const FATDirectoryIterator& fat_it);

        auto next() -> Expected<NodeInfo, DirectoryStreamStatus> override;
    };
} // namespace Rune::VFS

#endif // RUNEOS_FATDIRECTORYITERATOR_H
