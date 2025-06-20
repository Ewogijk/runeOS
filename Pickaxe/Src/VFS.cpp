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

#include <Pickaxe/VFS.h>


namespace Rune::Pickaxe {

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          VFSNodeInfo
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    bool VFSNodeInfo::is_readonly() const {
        return attributes & (int) NodeAttribute::READONLY;
    }


    bool VFSNodeInfo::is_hidden() const {
        return attributes & (int) NodeAttribute::HIDDEN;
    }


    bool VFSNodeInfo::is_system_node() const {
        return attributes & (int) NodeAttribute::SYSTEM;
    }


    bool VFSNodeInfo::is_directory() const {
        return attributes & (int) NodeAttribute::DIRECTORY;
    }


    bool VFSNodeInfo::is_file() const {
        return attributes & (int) NodeAttribute::FILE;
    }


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          System Calls
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    S64 vfs_get_node_info(const char* node_path, VFSNodeInfo* node_info_out) {
        SystemCallPayload payload = create_payload2(100, (uintptr_t) node_path, (uintptr_t) node_info_out);
        return make_system_call(&payload);
    }


    S64 vfs_create(const char* node_path, U8 node_attr) {
        SystemCallPayload payload = create_payload2(101, (uintptr_t) node_path, node_attr);
        return make_system_call(&payload);
    }


    S64 vfs_open(const char* path, NodeIOMode io_mode) {
        SystemCallPayload payload = create_payload2(102, (uintptr_t) path, (U64) io_mode);
        return make_system_call(&payload);
    }


    S64 vfs_delete(const char* node_path) {
        SystemCallPayload payload = create_payload1(103, (uintptr_t) node_path);
        return make_system_call(&payload);
    }


    bool vfs_close(U16 node_handle) {
        SystemCallPayload payload = create_payload1(104, node_handle);
        return make_system_call(&payload) >= 0;
    }


    S64 vfs_read(U16 handle, void* buf, size_t buf_size) {
        SystemCallPayload payload = create_payload3(105, handle, (uintptr_t) buf, buf_size);
        return make_system_call(&payload);
    }


    S64 vfs_write(U16 handle, void* buf, size_t buf_size) {
        SystemCallPayload payload = create_payload3(106, handle, (uintptr_t) buf, buf_size);
        return make_system_call(&payload);
    }


    S64 vfs_seek(U16 handle, size_t byte_pos) {
        SystemCallPayload payload = create_payload2(107, handle, byte_pos);
        return make_system_call(&payload);
    }


    S64 vfs_directory_stream_open(const char* dir_path) {
        SystemCallPayload payload = create_payload1(108, (uintptr_t) dir_path);
        return make_system_call(&payload);
    }


    S64 vfs_directory_stream_next(U16 dir_stream_handle, VFSNodeInfo* node_info_ptr) {
        SystemCallPayload payload = create_payload2(109, dir_stream_handle, (uintptr_t) node_info_ptr);
        return make_system_call(&payload);
    }


    S64 vfs_directory_stream_close(U16 dir_stream_handle) {
        SystemCallPayload payload = create_payload1(110, dir_stream_handle);
        return make_system_call(&payload);
    }
}