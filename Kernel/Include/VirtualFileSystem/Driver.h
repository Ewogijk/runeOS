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

#ifndef RUNEOS_DRIVER_H
#define RUNEOS_DRIVER_H

#include <Ember/Enum.h>
#include <KernelRuntime/Path.h>
#include <KernelRuntime/String.h>

#include <KernelRuntime/Memory.h>
#include <VirtualFileSystem/DirectoryStream.h>
#include <VirtualFileSystem/Node.h>
#include <VirtualFileSystem/Status.h>

namespace Rune::VFS {

    /**
     * A filesystem driver allows access to a disk formatted according to some filesystem
     * specification.
     */
    class Driver {
      public:
        virtual ~Driver() = default;

        /**
         * @return Name of the filesystem specification.
         */
        [[nodiscard]]
        virtual String get_name() const = 0;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                     Storage Device Functions
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        /**
         * Important: The old filesystem (if any) will be overriden by this function. All data will
         * be lost.
         *
         * @brief Create a new empty filesystem on the storage device.
         *
         * @see VFS::VFSSubsystem::format(const Lib::Path&, U16)
         *
         * @param storage_dev ID of a storage device.
         *
         * @return Formatted:           The storage device is formatted.
         *          FormatError:        An error happened while formatting the storage device.
         * Failure reason is specific to the file system implementation, check the logs.
         *          StorageDevError:    An IO error happened.
         */
        virtual FormatStatus format(U16 storage_dev) = 0;

        /**
         * A filesystem driver implementation shall be able to mount multiple storage devices at
         * once.
         *
         * @brief Make the storage device known to this filesystem driver.
         *
         * @see VFS::VFSSubsystem::mount(const Lib::Path&, U16)
         *
         * @param storage_dev ID of a storage device.
         *
         * @return Mounted:            The storage device is mounted.
         *          AlreadyMounted:    The storage device is already mounted.
         *          NotSupported:      The storage device is not formatted or uses an unknown
         * filesystem. StorageDevError:   An IO error happened.
         */
        virtual MountStatus mount(U16 storage_dev) = 0;

        /**
         *
         * @brief Remove the storage device from the known devices of this filesystem driver.
         *
         * @see VFS::VFSSubsystem::unmount(const Lib::Path&)
         *
         * @param storage_dev ID of a storage device.
         *
         * @return Unmounted:          The storage device is unmounted.
         *          NotMounted:        The storage device is not known.
         *          MountError:        The storage device could not be unmounted.
         *          StorageDevError:   An IO error happened.
         */
        virtual MountStatus unmount(U16 storage_dev) = 0;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          File Manipulations
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        /**
         * @brief Check if the given path contains any illegal characters.
         * @param path
         * @return True: The path does not contain any illegal character, False: The path contains
         * at least one illegal character.
         */
        [[nodiscard]]
        virtual bool is_valid_file_path(const Path& path) const = 0;

        /**
         * @brief Create a new file or directory on the storage device.
         * @see VFS::VFSSubsystem::create(const Lib::Path&, U8)
         * @param storage_dev ID of a storage device.
         * @param path           A path relative to the mount point of the storage device.
         * @param attributes     Initial attributes of the file.
         * @return Created:             The node has been created.
         *          Exists:             The node already exists.
         *          BadAttribute:       The Directory or File node attributes are not set correctly.
         *          BadName:            The node path contains illegal characters.
         *          StorageDevUnknown:  The storage device is unknown.
         *          StorageDevError:    An IO error happened.
         */
        virtual IOStatus create(U16 storage_dev, const Path& path, U8 attributes) = 0;

        /**
         * If the path is empty the root node of the filesystem shall be returned.
         *
         * @brief Try to find the node at the given path.
         * @see VFS::VFSSubsystem::open(const Lib::Path&, FileSystem::FileModeAttribute)
         * @param storage_dev ID of a storage device.
         * @param path           A path relative to the mount point of the storage device.
         * @param fileMode       Mode in which the file will be accessed e.g. read, write, etc.
         * @param path           Callback to be executed when the node is closed.
         * @param out            Pointer that will be set to the open node on success.
         * @return Opened:              The node was found, a pointer to it has been placed in out.
         *          NotFound:           The node path does not exist.
         *          StorageDevUnknown:  The storage device is unknown.
         *          StorageDevError:    An IO error happened.
         */
        virtual IOStatus open(U16                  storage_dev,
                              const Path&          mount_point,
                              const Path&          path,
                              Ember::IOMode        io_mode,
                              Function<void()>     on_close,
                              SharedPointer<Node>& out) = 0;

        /**
         * This operation will not create a node handle.
         *
         * @brief Search for the node with given path and get the node info if found.
         * @see VFS::VFSSubsystem::find_node(const Path& path, NodeInfo& out)
         * @param storage_dev Handle of a storage device.
         * @param path  Absolute path to a node.
         * @param out   If a node was found the info will be put in this object.
         * @return FOUND:       The node was found.
         *          NOT_FOUND:   The node was not found.
         *          BAD_PATH:    The path is not absolute.
         *          DEV_UNKNOWN: The storage device is unknown to the driver.
         *          DEV_ERROR:   An IO error happened.
         */
        virtual IOStatus find_node(U16 storage_dev, const Path& path, NodeInfo& out) = 0;

        /**
         * @brief Delete a file on the storage device.
         * @see VFS::VFSSubsystem::delete(const Lib::Path&)
         * @param storage_dev ID of a storage device.
         * @param path        A path relative to the mount point of the storage device.
         * @return Deleted:             The node has been deleted.
         *          NotFound:           The node path does not exist.
         *          StorageDevUnknown:  The storage device is unknown.
         *          StorageDevError:    An IO error happened.
         */
        virtual IOStatus delete_node(U16 storage_dev, const Path& path) = 0;

        /**
         * @brief Open a stream over the content of the directory.
         * @param storage_dev ID of a storage device.
         * @param path        A path relative to the mount point of the storage device.
         * @param onClose     Function to execute when the stream is closed.
         * @param out         Pointer that will be set to the open directory stream on success.
         * @return Opened:              The directory is opened, a pointer to it is placed in out.
         *          NotFound:           The node path does not exist.
         *          StorageDevUnknown:  The storage device is unknown.
         *          StorageDevError:    An IO error happened.
         */
        virtual IOStatus open_directory_stream(U16                             storage_dev,
                                               const Path&                     path,
                                               const Function<void()>&         on_close,
                                               SharedPointer<DirectoryStream>& out) = 0;
    };
} // namespace Rune::VFS

#endif // RUNEOS_DRIVER_H
