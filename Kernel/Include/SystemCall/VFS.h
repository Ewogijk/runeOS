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

#ifndef RUNEOS_VFS_H
#define RUNEOS_VFS_H


#include <Ember/Definitions.h>

#include <App/AppSubsystem.h>

#include <VirtualFileSystem/VFSSubsystem.h>

#include <SystemCall/KernelGuardian.h>


namespace Rune::SystemCall {

    /**
     * @brief A VFS node info that will be passed to user mode applications.
     */
    struct VFSNodeInfo {
        char   node_path[KernelGuardian::USER_STRING_LIMIT] = { };
        size_t size                                         = 0;
        U8     attributes                                   = 0;
    };


    /**
     * @brief The context for all virtual file system related system calls.
     */
    struct VFSContext {
        KernelGuardian* k_guard    = nullptr;
        VFS::Subsystem* vfs_subsys = nullptr;
        App::Subsystem* app_subsys = nullptr;
    };


    /**
     * @brief Search for a node at given path and return the node info for it.
     * @param sys_call_ctx A pointer to the VFS context.
     * @param node_path     Path to a node.
     * @param node_info_out Node info buffer.
     * @return 0:                  The node was found and the node_info_out buffer contains the node info.<br>
     *          BAD_ARG:           An argument is null, intersects kernel memory, exceeds the string size limit or the
     *                             node path contains illegal characters.<br>
     *          NODE_NOT_FOUND:    The node does not exist.<br>
     *          IO:                IO error.
     */
    S64 vfs_get_node_info(void* sys_call_ctx, U64 node_path, U64 node_info_out);


    /**
     * Note: Creating a node does not open it.
     *
     * @brief Try to create node at the requested path with the given node attributes.
     * @param sys_call_ctx A pointer to the VFS context.
     * @param node_path    Path to a VFS node.
     * @param node_attr    Initial attributes of the node.
     * @return 0:            The node has been created.<br>
     *          BAD_ARG:     An argument is null, intersects kernel memory, the node path exceeds the string size
     *                       limit, the node path contains an illegal character or an IO mode is invalid.<br>
     *          NODE_EXISTS: The node already exists.<br>
     *          IO:          An IO error happened.
     */
    S64 vfs_create(void* sys_call_ctx, U64 node_path, U64 node_attr);


    /**
     * @brief Search for a node with the given path and try to open it in the requested node IO mode.
     *
     * An opened node must be closed, otherwise the resource will leak until the calling application exits.
     *
     * @param sys_call_ctx A pointer to the VFS context.
     * @param node_path    Path to a VFS node.
     * @param io_mode      Node IO mode, 1: Read, 2: Write or 3: Append.
     * @return >0:              Node ID.<br>
     *          BAD_ARG:        An argument is null, intersects kernel memory, the node path exceeds the string size
     *                          limit, the node path contains an illegal character or an IO mode is invalid.<br>
     *          NODE_NOT_FOUND: The node path does not exist.<br>
     *          IO:             An IO error happened.
     */
    S64 vfs_open(void* sys_call_ctx, U64 node_path, U64 io_mode);


    /**
     * Note: Deleting a node does not open it.
     *
     * @brief Try to delete the node at given path.
     * @param sys_call_ctx A pointer to the VFS context.
     * @param node_path    Path to a VFS node.
     * @return 0:               The node got deleted.<br>
     *          BAD_ARG:        The node path is null, intersects kernel memory or contains an illegal character.<br>
     *          NODE_IN_USE:    The node cannot be deleted because it is used by another application.<br>
     *          NODE_NOT_FOUND: The node was not found.<br>
     *          IO:             An IO error happened.
     */
    S64 vfs_delete(void* sys_call_ctx, U64 node_path);


    /**
     * @brief Try to close the node identified by the given node ID.
     * @param sys_call_ctx A pointer to the VFS context.
     * @param ID           A node ID > 0.
     * @return 0:        The node has been closed.<br>
     *          BAD_ARG: The node ID is 0.<br>
     *          BAD_ID:  No node with the given ID was found.
     */
    S64 vfs_close(void* sys_call_ctx, U64 ID);


    /**
     * @brief Try to read u_buf_size bytes into uBuf from the node referenced by the ID.
     * @param sys_call_ctx A pointer to the VFS context.
     * @param ID           Node ID.
     * @param u_buf        Pointer to a byte buffer.
     * @param u_buf_size   Size of the buffer.
     * @return >=0:                The number of bytes copied to the buffer.<br>
     *          BAD_ARG:           The buffer is null or intersects kernel memory. Or the node ID is zero.<br>
     *          BAD_ID:            No node with the requested ID was found.<br>
     *          NODE_IS_DIRECTORY: The node is a directory.<br>
     *          NODE_CLOSED:       The node ID is invalid, because the node has already been closed.<br>
     *          IO:                An IO error happened.<br>
     *          FAULT:             The bytes could not be copied to the user mode buffer.
     */
    S64 vfs_read(void* sys_call_ctx, U64 ID, U64 u_buf, U64 u_buf_size);


    /**
     * @brief Try to write uBufSize bytes in the uBuf to the file referenced by the ID.
     * @param sys_call_ctx A pointer to the VFS context.
     * @param ID           Node ID.
     * @param u_buf        Pointer to a byte buffer.
     * @param u_buf_size   Size of the buffer.
     * @return >=0:                The number of bytes written to the file.<br>
     *          BAD_ARG:           The buffer is null or intersects kernel memory. Or the node ID is zero.<br>
     *          BAD_ID:            No node with the requested ID was found.<br>
     *          NODE_IS_DIRECTORY: The node is a directory.<br>
     *          NODE_CLOSED:       The node ID is invalid, because the node has already been closed.<br>
     *          ACCESS_DENIED:     The node is in read mode.<br>
     *          IO:                An IO error happened.
     */
    S64 vfs_write(void* sys_call_ctx, U64 ID, U64 u_buf, U64 u_buf_size);


    /**
     * @brief Try to skip 'offset' bytes in a file depending on the requested 'seek_mode'.
     * @param sys_call_ctx A pointer to the VFS context.
     * @param ID           Node ID.
     * @param seek_mode    Determines how the new file cursor position is calculated.
     * @param offset       Number of bytes to skip.
     * @return >=0:                The number of bytes skipped.<br>
     *          BAD_ARG:           The node ID is zero, the seek mode is invalid or the offset is bad.<br>
     *          BAD_ID:            No node with the requested ID was found.<br>
     *          NODE_IS_DIRECTORY: The node is a directory.<br>
     *          NODE_CLOSED:       The node ID is invalid, because the node has already been closed.<br>
     *          IO:                An IO error happened.
     */
    S64 vfs_seek(void* sys_call_ctx, U64 ID, U64 seek_mode, U64 offset);


    /**
     * @brief Try to open a stream over the content of a directory.
     * @param sys_call_ctx A pointer to the VFS context.
     * @param dir_path     Path to a directory.
     * @return >=0:             An ID to the opened directory stream.<br>
     *          BAD_ARGS:       The directory path buffer is null, intersects kernel memory or contains an illegal
     *                          character.<br>
     *          NODE_NOT_FOUND: The node was not found.<br>
     *          NODE_IS_FILE:   The node is a file<br>
     *          IO:             An IO error happened.
     */
    S64 vfs_directory_stream_open(void* sys_call_ctx, U64 dir_path);


    /**
     * @brief Try to get the next directory node.
     * @param sys_call_ctx  A pointer to the VFS context.
     * @param dir_stream_ID Directory stream ID.
     * @param node_info_ptr A pointer to a VFSNodeInfo struct.
     * @return 1:        The directory contains at least one more node info.<br>
     *          0:       The directory has no more node infos left.<br>
     *          BAD_ARG: The ID is invalid, the node info buffer is null or intersects kernel memory.<br>
     *          BAD_ID:  No directory stream with the requested ID exists.<br>
     *          IO:      An IO error occurred while getting the next node info.
     */
    S64 vfs_directory_stream_next(void* sys_call_ctx, U64 dir_stream_ID, U64 node_info_ptr);


    /**
     * @brief Try to close the directory stream with the given ID.
     * @param sys_call_ctx  A pointer to the VFS context.
     * @param dir_stream_ID Directory stream ID.
     * @return 0: The directory stream is closed.
     *          BAD_ARG: The ID is zero.
     *          BAD_ID: No directory stream with the requested ID exists.
     */
    S64 vfs_directory_stream_close(void* sys_call_ctx, U64 dir_stream_ID);
}

#endif //RUNEOS_VFS_H
