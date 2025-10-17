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

#ifndef RUNEOS_VFSMODULE_H
#define RUNEOS_VFSMODULE_H

#include <KRE/Stream.h>
#include <KRE/String.h>
#include <KRE/System/Module.h>
#include <KRE/System/Resource.h>

#include <KRE/Collections/HashMap.h>

#include <VirtualFileSystem/DirectoryStream.h>
#include <VirtualFileSystem/Driver.h>
#include <VirtualFileSystem/Node.h>
#include <VirtualFileSystem/Path.h>
#include <VirtualFileSystem/Status.h>

namespace Rune::VFS {

    /**
     * @brief All event hooks of the filesystem subsystem.
     * <ul>
     *  <li>NodeOpened: A node has been opened. Event Context: The handle of the opened node,
     * U16*.</li> <li>NodeClosed: A node has been closed. Event Context: The handle of the closed
     * node, U16*.</li> <li>DirectoryStreamOpened: A directory stream has been closed. Event
     * Context: The handle of the opened directory stream, U16*.</li> <li>DirectoryStreamClosed: A
     * directory stream has been opened. Event Context: The handle of the closed directory stream,
     * U16*.</li>
     * </ul>
     */
#define VFS_EVENT_HOOKS(X)                                                                         \
    X(EventHook, NODE_OPENED, 0x1)                                                                 \
    X(EventHook, NODE_CLOSED, 0x2)                                                                 \
    X(EventHook, DIRECTORY_STREAM_OPENED, 0x3)                                                     \
    X(EventHook, DIRECTORY_STREAM_CLOSED, 0x4)

    DECLARE_ENUM(EventHook, VFS_EVENT_HOOKS, 0x0) // NOLINT

    /**
     * @brief Mapping of a mount point (path) to a driver name and storage device ID.
     */
    struct MountPointInfo {
        Path   mount_point;
        String driver_name;
        U16    storage_device;
    };

    /**
     * @brief Counts all open node handles that point to the same path and if the node should be
     * deleted when the last file handles is closed.
     */
    struct NodeRefCount {
        Path node_path   = Path("");
        U16  ref_count   = 0;
        bool delete_this = false;
    };

    /**
     * @brief The Virtual Filesystem Subsystem (VFSS) is the main entry point to access
     * files/directories (Nodes) on possibly multiple file systems.
     *
     * <h1>Filesystem</h1>
     * <p>
     *  The filesystem is unix like, thus an example path is "/my/fancy/path". Same as unix it
     * contains a root filesystem mounted at "/" and further devices can be mounted at any path,
     * except if that path is already a mount point e.g. another drive could be mounted at
     * "/mnt/mydrive" but not at "/" since it is already taken.
     * </p>
     *
     * <h1>Mount Points</h1>
     * <p>
     *  A mount point simply encodes a path under which a logical storage device can be accessed
     * that it formatted with a filesystem, thus when accessing different node paths different
     * devices could be accessed. Each device may additionally use a different filesystem format,
     * therefore each mount point is associated with a specific filesystem driver.
     * </p>
     * <p>
     *  All mount points are registered in the mount point table with its associated filesystem
     * driver. The VFSS has exclusive memory ownership over mount points, only copies of the mount
     * point information is returned when requested.
     * </p>
     * <p>
     *  Mount point resolution (MPR) is the process where the best matching mount point is
     * determined for a given absolute path. MPR chooses the mount point that has the longest
     * matching common path with said absolute path. E.g. Given the mount points /a and /a/b/c and
     * some path /a/b/c/d, MPR will choose /a/b/c as the mount point for the path. The virtual file
     * system always has at least one device mounted under the root directory "/", therefore MPR
     * cannot fail, since all paths are at least mounted on the root directory if no other mount
     * point.
     * </p>
     * <h1>Driver</h1>
     * <p>
     *  Filesystem drivers are needed to be able to mount/unmount mount points, but also to format
     * logical storage devices. They are registered within the driver table. The VFSS has exclusive
     * memory ownership over drivers as long as they are registered, only when removing them from
     * the driver table will memory ownership transfer over to the caller.
     * </p>
     *
     * <h1>Node</h1>
     * <p>
     *  A node simply represents a file or directory in the filesystem. Each node can be identified
     * by a globally unique handle e.g. The node "/my/node" could have the handle 15. Whenever a
     * node is opened it will be registered in the node table that holds all currently open nodes in
     * the kernel across all applications. When a node is closed it will be removed from the node
     * table.
     * </p>
     * <p>
     *  Additionally, a node ref table is maintained that keeps track of how many times the same
     * node is opened, e.g. again using the node "/my/node" if two apps have opened it at the same
     * time an entry "/my/node" with a ref count of two is made in the node ref table. The node ref
     * table is required so that a node is not physically deleted while other apps are still using
     * it but rather physically deleted after the last app closed the node.
     * </p>
     * <p>
     *  Each node is dynamically allocated when it is first opened and memory ownership is shared
     * between the VFS and the calling code. As long as the node is open (registered in the node
     * table) the memory will not be freed, thus it is important to always close a node after it is
     * no longer needed. But also the calling code can obtain shared pointers to the node, thus must
     * make sure that those pointers are deleted at some point.
     * </p>
     */
    class VFSModule : public Module {
      private:
        // All registered file system drivers
        HashMap<String, UniquePointer<Driver>> _driver_table;

        // All mount points and their devices
        HashMap<Path, MountPointInfo> _mount_point_table;

        // Counts all open node handles that point to a single path
        HashMap<Path, NodeRefCount> _node_ref_table;

        // All currently opened nodes
        HashMap<U16, SharedPointer<Node>> _node_table;
        IDCounter<U16>                    _node_handle_counter;

        // All currently opened directory streams
        HashMap<U16, SharedPointer<DirectoryStream>> _dir_stream_table;
        IDCounter<U16>                               _dir_stream_handle_counter;

        [[nodiscard]] auto resolve(const Path& path) const -> MountPointInfo;

        auto create_system_directory(const Path& path) -> bool;

      public:
        explicit VFSModule();

        ~VFSModule() override = default;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                      KernelSubsystem Overrides
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        [[nodiscard]] auto get_name() const -> String override;

        auto load(const BootInfo& boot_info) -> bool override;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                      Filesystem Driver Registration
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        /**
         *
         * @return A list of the names of all registered filesystem drivers.
         */
        [[nodiscard]] auto get_driver_table() const -> LinkedList<String>;

        /**
         * Add a new filesystem driver.
         *
         * @param driver
         *
         * @return True: The filesystem driver is added, False: It is not.
         */
        auto install_driver(UniquePointer<Driver> driver) -> bool;

        /**
         * Remove the filesystem driver.
         *
         * @param driver
         *
         * @return True: The filesystem driver is removed, False: It is not.
         */
        auto uninstall_driver(UniquePointer<Driver> driver) -> bool;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          Node Table Access
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        /**
         * @brief
         * @return The node table with all currently open nodes.
         */
        [[nodiscard]] auto get_node_table() const -> LinkedList<Node*>;

        /**
         * @brief Dump the node table to the stream.
         * @stream
         */
        void dump_node_table(const SharedPointer<TextStream>& stream) const;

        /**
         * @brief Dump the node ref table to the stream.
         * @stream
         */
        void dump_node_ref_table(const SharedPointer<TextStream>& stream) const;

        /**
         * @brief Get a node from the node table.
         * @param handle Node handle.
         * @return The node if the node handle is present else a nullptr.
         */
        [[nodiscard]] auto find_node(U16 handle) const -> SharedPointer<Node>;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                      Directory Stream Table Access
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        /**
         * @brief
         * @return The directory stream table with all currently open directory streams.
         */
        [[nodiscard]] auto get_directory_stream_table() const -> LinkedList<DirectoryStream*>;

        /**
         * @brief Dump the directory stream table to the stream.
         * @stream
         */
        void dump_directory_stream_table(const SharedPointer<TextStream>& stream) const;

        /**
         * @brief Get a directory stream from the directory stream table.
         * @param handle Directory stream handle.
         * @return The directory stream if the directory stream handle is present else a nullptr.
         */
        [[nodiscard]] auto find_directory_stream(U16 handle) const
            -> SharedPointer<DirectoryStream>;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          Mounting and Formatting
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        /**
         *
         * @return A list of all mount points.
         */
        [[nodiscard]] auto get_mount_point_table() const -> LinkedList<MountPointInfo>;

        /**
         * @brief Dump the mount point table to the stream.
         * @param stream
         */
        void dump_mount_point_table(const SharedPointer<TextStream>& stream) const;

        /**
         * Try to format the storage device with the given ID with the filesystem implemented by the
         * requested driver.
         *
         * <p>
         *  Formatting a storage device will make the old filesystem unusable, therefore erasing the
         * current content of the device.
         * </p>
         *
         * @param driver         Driver that will be used for formatting.
         * @param storage_device ID of the storage device.
         *
         * @return FORMATTED:     The storage device is formatted.
         *          FORMAT_ERROR: An error happened while formatting the storage device. Failure
         * reason is specific to the file system implementation, check the logs. DEV_ERROR:    An IO
         * error happened.
         */
        [[nodiscard]] auto format(const String& driver_name, uint16_t storage_device) const
            -> FormatStatus;

        /**
         * Try to mount the storage with the given ID to the mount point.
         * <p>
         *  The very first mount point must always be the root directory of the file system, an
         * attempt to mount another path will fail until the root directory is mounted.
         * </p>
         *
         * <p>
         *  A mount point must have the following properties:
         *  <ol>
         *      <li>A mount point must be relative to another mount point.</li>
         *      <li>A mount point must be an existing directory.</li>
         *      <li>A mount point cannot be equal to an existing mount point.</li>
         *  </ol>
         * </p>
         *
         * @param mount_point    Absolute path to an existing directory.
         * @param storage_device_id ID of the storage device.
         *
         * @return MOUNTED:          The storage device is mounted.
         *          ALREADY_MOUNTED: The storage device is already mounted.
         *          MOUNT_ERROR:     No mount points are registered and this mount point is not the
         * directory or the directory does not exists. NOT_SUPPORTED:   No driver supports the
         * filesystem of the storage device. DEV_ERROR:       An IO error happened.
         */
        auto mount(const Path& mount_point, U16 storage_device_id) -> MountStatus;

        /**
         * Try to unmount the given mount point.
         *
         * <p>
         *  The root directory cannot be unmounted.
         * </p>
         *
         * @param mount_point Absolute path to an existing directory.
         *
         * @return UNMOUNTED:    The storage device is unmounted.
         *          BAD_PATH:    The mount is not absolute or the root directory.
         *          NOT_MOUNTED: The mount point is not known.
         *          MOUNT_ERROR: The mount point could not be removed from the mount point table or
         * the driver failed to unmount the storage device. DEV_ERROR:   An IO error happened.
         */
        auto unmount(const Path& mount_point) -> MountStatus;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          Filesystem Access
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        /**
         * @brief Check if the absolute path contains any illegal characters.
         *
         * <p>
         *  Checking the path involves going through each mount point along the whole path and
         * finding the corresponding driver that validates the portion of the path it is responsible
         * for.
         * </p>
         * <p>
         *  For example: /path/to/some/file
         *               |-----------|-----|
         *                   FAT32    EXT4
         *  Assuming we have EXT4 support (which is not the case) and the mount point "/" which is a
         * FAT32 filesystem and the mount point "/path/to/some" an EXT4 filesystem.<br> First the
         * "/file" part is validated by the EXT4 filesystem driver and then "/path/to/some" is
         * validated by the FAT32 driver.
         * </p>
         * @param path An absolute path.
         * @return True: The path does not contain any illegal character, False: The path contains
         * at least one illegal character.
         */
        [[nodiscard]] auto is_valid_file_path(const Path& path) const -> bool;

        /**
         * This operation will not create a node handle.
         *
         * @brief Search for the node with given path and get the node info if found.
         * @param path  Absolute path to a node.
         * @param out   If a node was found the info will be put in this object.
         * @return FOUND:       The node was found.
         *          NOT_FOUND:   The node was not found.
         *          BAD_PATH:    The path is not absolute.
         *          DEV_UNKNOWN: The storage device is unknown to the driver.
         *          DEV_ERROR:   An IO error happened.
         */
        auto get_node_info(const Path& path, NodeInfo& out) -> IOStatus;

        /**
         * Try to create a file/directory at the path with the given attributes. Either the
         * FileAttribute::Directory or FileAttribute::File attribute must be set otherwise the
         * creation will fail.
         *
         * <p>
         *  If the given path is a mount point, the attempt to resolve the path to a mount point
         * fails or the driver runs into an error, then the path is not created.
         * </p>
         *
         * @param path       Absolute path.
         * @param attributes File attributes of the path.
         *
         * @return CREATED:        The node has been created.
         *          EXISTS:        The node already exists.
         *          BAD_PATH:      The path is not absolute or the root directory.
         *          BAD_ATTRIBUTE: The Directory or File node attributes are not set correctly.
         *          BAD_NAME:      The node path contains illegal characters.
         *          DEV_UNKNOWN:   The storage device is unknown to the driver.
         *          DEV_ERROR:     An IO error happened.
         */
        auto create(const Path& path, U8 attributes) -> IOStatus;

        /**
         * @brief Try to open the file/directory at the requested path. If the path is found the
         * node will be added to the node table and a pointer to it is placed in "out".
         *
         * <p>
         *  If the attempt to resolve the path to a mount point fails or the driver runs into an
         * error, then the path is not opened.
         * </p>
         *
         * @param path    Absolute path to an existing file/directory.
         * @param io_mode IO mode.
         * @param out     In case of success a pointer to the node will be placed here.
         *
         * @return OPENED:          A handle to the node path is created.
         *          BAD_PATH:       The path is not absolute or the root directory.
         *          OUT_OF_HANDLES: No more free node handles are left.
         *          NOT_FOUND:      The node path does not exist.
         *          DEV_UNKNOWN:    The storage device is unknown to the driver.
         *          DEV_ERROR:      An IO error happened.
         *
         */
        [[nodiscard]] auto
        open(const Path& path, Ember::IOMode node_io_mode, SharedPointer<Node>& out) -> IOStatus;

        /**
         * Try to delete the file/directory at the given path. Note that deleting a file does not
         close the file.
         *
         * <p>
         *  If the attempt to resolve the path to a mount point fails or the driver runs into an
         error, then the path
         *  is not opened.
         * </p>
         *
         * <p>
         *  If multiple node handles point to the same file, the deletion will be postponed until
         the last node handle
         *  is closed.
         * </p>
         *
         * @param path Absolute path to an existing file/directory.

         *
         * @return DELETED:        The node has been deleted or is marked for deletion and will be
         deleted later
         *                         because another application has also opened the file.
         *          BAD_PATH:      The path is not absolute or the root directory.
         *          ACCESS_DENIED: The path is a mount point or a directory and another node path in
         it is open.
         *          NOT_FOUND:     The node path does not exist.
         *          DEV_UNKNOWN:   The storage device is unknown to the driver.
         *          DEV_ERROR:     An IO error happened.
         */
        auto delete_node(const Path& path) -> IOStatus;

        /**
         * @brief Try to open a stream over the content of the directory.
         *
         * <p>
         *  If the attempt to resolve the path to a mount point fails or the driver runs into an
         * error, then the path is not opened.
         * </p>
         * @param path Absolute path to an existing directory.
         * @param out  In case of success a pointer to the directory stream will be placed here.
         * @return OPENED:          A handle for the directory stream has been created.
         *          BAD_PATH:       The path is not absolute or a file.
         *          OUT_OF_HANDLES: No more free directory stream handles are left.
         *          NOT_FOUND:      The node path does not exist.
         *          DEV_UNKNOWN:    The storage device is unknown to the driver.
         *          DEV_ERROR:      An IO error happened.
         */
        auto open_directory_stream(const Path& path, SharedPointer<DirectoryStream>& out)
            -> IOStatus;
    };
} // namespace Rune::VFS

#endif // RUNEOS_VFSMODULE_H
