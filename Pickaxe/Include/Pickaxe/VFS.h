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

#ifndef EWOGIJKOS_VFS_H
#define EWOGIJKOS_VFS_H


#include "SystemCall.h"


namespace Rune::Pickaxe {

    enum class NodeIOMode {
        READ   = 1,
        WRITE  = 2,
        APPEND = 3
    };

    enum class NodeAttribute {
        READONLY  = 0x01,
        HIDDEN    = 0x02,
        SYSTEM    = 0x04,
        DIRECTORY = 0x08,
        FILE      = 0x10
    };


    struct VFSNodeInfo {
        char   node_path[128] = { };
        size_t size           = 0;
        U8     attributes     = 0;


        [[nodiscard]] bool is_readonly() const;


        [[nodiscard]] bool is_hidden() const;


        [[nodiscard]] bool is_system_node() const;


        [[nodiscard]] bool is_directory() const;


        [[nodiscard]] bool is_file() const;
    };


    /**
     * @brief Search for a node at given path and return the node info for it.
     * @param node_path     Path to a node.
     * @param node_info_out Node info buffer.
     * @return 0: The node was found and the node_info_out buffer contains the node info.
     *          -1: The node path is null, the size exceeds the max allowed string size or intersects kernel memory.
     *          -2: The node path contains illegal characters.
     *          -3: The node info buffer is null or intersects kernel memory.
     *          -4: The node does not exist.
     *          -5: An intermediate node on the path is a file.
     *          -6: IO error.
     */
    S64 vfs_get_node_info(const char* node_path, VFSNodeInfo* node_info_out);


    /**
     * @brief Try to create node at the requested path with the given node attributes.
     * @param sysCallCtx A pointer to the VFS context.
     * @param node_path   Path to a VFS node.
     * @param node_attr   Initial attributes of the node.
     * @return 0: The node has been created.
     *          -1: The node path buffer is null or intersects kernel memory.
     *          -2: The node attributes contain unknown attributes.
     *          -3: The node path contains an illegal character.
     *          -4: The node already exists.
     *          -5: The Directory or File attributes are not set correctly.
     *          -6: An IO error happened.
     */
    S64 vfs_create(const char* node_path, U8 node_attr);


    /**
     * @brief Search for a node with the given path and try to open it in the requested node IO mode.
     *
     * An opened node handle must be closed, otherwise the resource will leak until the calling application exits.
     *
     * @param path  Path to a VFS node.
     * @param io_mode Node IO mode, 1: Read, 2: Write or 3: Append.
     * @return >0: A handle to a file node.
     *          -1: The node path buffer is null or intersects kernel memory.
     *          -2: The requested node IO mode is unknown.
     *          -3: The node path contains an illegal character.
     *          -4: The node path does not exist.
     *          -5: The kernel is out of node handles.
     *          -6: An IO error happened.
     */
    S64 vfs_open(const char* path, NodeIOMode io_mode);


    /**
     * @brief Try to delete the node at given path.

     * @param node_path   Path to a VFS node.
     * @return 0: The node got deleted.
     *          -1: The node path buffer is null or intersects kernel memory.
     *          -2: The node path contains an illegal character.
     *          -3: The node cannot be deleted because it is used by another application.
     *          -4: The node was not found.
     *          -5: An IO error happened.
     */
    S64 vfs_delete(const char* node_path);


    /**
     * @brief Try to close the node  identified by the given node handle.
     * @param node_handle A node handle > 0.
     * @return 0: The node has been closed.
     *          -1: The node handle is 0.
     *          -2: No node with the given node handle was found.
     */
    bool vfs_close(U16 node_handle);


    /**
     * @brief Try to read bufSize bytes into buf from the file referenced by the handle.
     * @param handle  Handle to a node.
     * @param buf     Pointer to a byte buffer.
     * @param buf_size Size of the buffer.
     * @return >=0: The number of bytes copied to the buffer.
     *          -1: The node handle is zero.
     *          -2: No node with the requested handle was found.
     *          -3: The node is a directory.
     *          -4: The user buffer buffer is null or intersects kernel memory.
     *          -5: Read is not supported.
     *          -6: The node handle is invalid, because the node has already been closed.
     *          -7: An IO error happened.
     *          -8: The bytes could not be copied to the user mode buffer.
     */
    S64 vfs_read(U16 handle, void* buf, size_t buf_size);


    /**
     * @brief Try to write bufSize bytes in the buf to the file referenced by the handle.
     * @param handle  Handle to a node.
     * @param buf     Pointer to a byte buffer.
     * @param buf_size Size of the buffer.
     * @return >=0: The number of bytes written to the file.
     *          -1: The node handle is zero.
     *          -2: No node with the requested handle was found.
     *          -3: The node is a directory.
     *          -4: The user buffer could not be copied to the kernel buffer.
     *          -5: Write is not supported.
     *          -6: The node is in read mode.
     *          -7: The node handle is invalid, because the node has already been closed.
     *          -8: An IO error happened.
     */
    S64 vfs_write(U16 handle, void* buf, size_t buf_size);


    /**
     * @brief Try to skip bytePos bytes from the beginning of the file with the requested handle.
     * @param handle     Handle to a node.
     * @param byte_pos    Amount of bytes to skip starting from the beginning of the file.
     * @return >=0: The number of bytes skipped.
     *          -1: The node handle is zero.
     *          -2: No node with the requested handle was found.
     *          -3: The node is a directory.
     *          -4: The node handle is invalid, because the node has already been closed.
     *          -5: An IO error occurred.
     */
    S64 vfs_seek(U16 handle, size_t byte_pos);


    /**
     * @brief Try to open a stream over the content of a directory.
     * @param dir_path     Path to a directory.
     * @return >=0: A handle to the opened directory stream.
     *          -1: The directory path buffer is null or intersects kernel memory.
     *          -2: The directory path contains an illegal character.
     *          -3: The kernel is out of directory stream handles.
     *          -4: The node was not found.
     *          -5: The node is not a directory.
     *          -6: An IO error happened.
     */
    S64 vfs_directory_stream_open(const char* dir_path);


    /**
     * @brief Try to get the next directory node.
     * @param dir_stream_handle Handle of a directory stream.
     * @param node_info_ptr     A pointer to a VFSNodeInfo struct.
     * @return 1: The directory contains at least one more node info.
     *          0: The directory has no more node infos left.
     *          -1: The handle is invalid.
     *          -2: The node info buffer is null or intersects kernel memory.
     *          -3: No directory stream with the requested handle exists.
     *          -4: An IO error occurred while getting the next node info.
     *          -5: The node info name could not be copied to the node info buffer.
     */
    S64 vfs_directory_stream_next(U16 dir_stream_handle, VFSNodeInfo* node_info_ptr);


    /**
     * @brief Try to close the directory stream with the given handle.
     * @param dir_stream_handle Handle to a directory stream.
     * @return 0: The directory stream is closed.
     *          -1: The handle is zero.
     *          -2: No directory stream with the requested handle exists.
     */
    S64 vfs_directory_stream_close(U16 dir_stream_handle);
}

#endif //EWOGIJKOS_VFS_H
