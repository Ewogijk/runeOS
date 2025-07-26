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

#include <SystemCall/VFS.h>


namespace Rune::SystemCall {

    S64 vfs_get_node_info(void* sys_call_ctx, U64 node_path, U64 node_info_out) {
        auto* vfs_ctx     = (VFSContext*) sys_call_ctx;
        auto* u_node_path = (const char*) node_path;
        char k_node_path_buf[KernelGuardian::USER_STRING_LIMIT];
        memset(k_node_path_buf, 0, KernelGuardian::USER_STRING_LIMIT);
        if (!vfs_ctx->k_guard->copy_string_user_to_kernel(u_node_path, -1, k_node_path_buf))
            return -1;

        Path k_node_path(k_node_path_buf);
        // If the path is relative prepend the working directory of the active app
        if (!k_node_path.is_absolute())
            k_node_path = k_node_path.resolve(vfs_ctx->app_subsys->get_active_app()->working_directory);

        if (!vfs_ctx->vfs_subsys->is_valid_file_path(k_node_path))
            return -2;

        VFS::NodeInfo k_node_info_buf;
        if (!vfs_ctx->k_guard
                    ->copy_byte_buffer_user_to_kernel(
                            (void*) node_info_out,
                            sizeof(VFS::NodeInfo),
                            (void*) &k_node_info_buf
                    ))
            return -3;

        VFS::IOStatus io_status = vfs_ctx->vfs_subsys->get_node_info(k_node_path, k_node_info_buf);
        switch (io_status) {
            case VFS::IOStatus::FOUND: {
                auto u_node_info = LibK::memory_addr_to_pointer<VFSNodeInfo>(node_info_out);
                // The node info will only contain the node name -> set it to the path the caller has provided
                if (!vfs_ctx->k_guard
                            ->copy_byte_buffer_kernel_to_user(
                                    (void*) k_node_info_buf.node_path.to_cstr(),
                                    (void*) u_node_info->node_path,
                                    k_node_info_buf.node_path.size() + 1    // size() does not include null terminator
                            )) {
                    return -3;
                }
                u_node_info->size = k_node_info_buf.size;
                u_node_info->attributes = k_node_info_buf.attributes;
                return 0;
            }

            case VFS::IOStatus::NOT_FOUND:
                return -4;
            case VFS::IOStatus::BAD_PATH:
                return -5;
            default:
                return -6;  // DEV_ERROR and DEV_UNKNOWN
        }
    }


    S64 vfs_create(void* sys_call_ctx, U64 node_path, U64 node_attr) {
        auto* vfs_ctx     = (VFSContext*) sys_call_ctx;
        auto* u_node_path = (const char*) node_path;
        char k_node_path_buf[KernelGuardian::USER_STRING_LIMIT];
        memset(k_node_path_buf, 0, KernelGuardian::USER_STRING_LIMIT);
        if (!vfs_ctx->k_guard->copy_string_user_to_kernel(u_node_path, -1, k_node_path_buf))
            return -1;

        U8 k_node_attr = node_attr;
        U8 verify_mask = Ember::NodeAttribute::READONLY
                         | Ember::NodeAttribute::HIDDEN
                         | Ember::NodeAttribute::SYSTEM
                         | Ember::NodeAttribute::DIRECTORY
                         | Ember::NodeAttribute::FILE;
        if ((k_node_attr & ~verify_mask) != 0)
            return -2;

        Path k_node_path(k_node_path_buf);
        // If the path is relative prepend the working directory of the active app
        if (!k_node_path.is_absolute())
            k_node_path = k_node_path.resolve(vfs_ctx->app_subsys->get_active_app()->working_directory);

        if (!vfs_ctx->vfs_subsys->is_valid_file_path(k_node_path))
            return -3;

        VFS::IOStatus io_status = vfs_ctx->vfs_subsys->create(k_node_path, k_node_attr);
        switch (io_status) {
            case VFS::IOStatus::CREATED:
                return 0;
            case VFS::IOStatus::FOUND:
                return -4;
            case VFS::IOStatus::BAD_ATTRIBUTE:
                return -5;
            default:
                return -6; // DEV_ERROR or StorageDevUnknown
        }
    }


    S64 vfs_open(void* sys_call_ctx, U64 arg1, U64 arg2) {
        auto* vfs_ctx     = (VFSContext*) sys_call_ctx;
        auto* u_node_path = (const char*) arg1;
        char k_node_path_buf[KernelGuardian::USER_STRING_LIMIT];
        memset(k_node_path_buf, 0, KernelGuardian::USER_STRING_LIMIT);
        if (!vfs_ctx->k_guard->copy_string_user_to_kernel(u_node_path, -1, k_node_path_buf))
            return -1;

        Ember::IOMode io_mode(arg2);
        if (io_mode == Ember::IOMode::NONE)
            return -2;

        Path k_node_path(k_node_path_buf);
        // If the path is relative prepend the working directory of the active app
        if (!k_node_path.is_absolute())
            k_node_path = k_node_path.resolve(vfs_ctx->app_subsys->get_active_app()->working_directory);

        if (!vfs_ctx->vfs_subsys->is_valid_file_path(k_node_path))
            return -3;

        SharedPointer<VFS::Node> node;
        VFS::IOStatus            io_status = vfs_ctx->vfs_subsys->open(k_node_path, io_mode, node);

        switch (io_status) {
            case VFS::IOStatus::OPENED:
                return node->handle;
            case VFS::IOStatus::NOT_FOUND:
                return -4;
            case VFS::IOStatus::OUT_OF_HANDLES:
                return -5;
            default:
                return -6; // DEV_ERROR or StorageDevUnknown
        }
    }


    S64 vfs_delete(void* sys_call_ctx, U64 node_path) {
        auto* vfs_ctx = (VFSContext*) sys_call_ctx;

        auto* u_node_path = (const char*) node_path;
        char k_node_path_buf[KernelGuardian::USER_STRING_LIMIT];
        memset(k_node_path_buf, 0, KernelGuardian::USER_STRING_LIMIT);
        if (!vfs_ctx->k_guard->copy_string_user_to_kernel(u_node_path, -1, k_node_path_buf))
            return -1;

        Path k_node_path(k_node_path_buf);
        // If the path is relative prepend the working directory of the active app
        if (!k_node_path.is_absolute())
            k_node_path = k_node_path.resolve(vfs_ctx->app_subsys->get_active_app()->working_directory);

        if (!vfs_ctx->vfs_subsys->is_valid_file_path(k_node_path))
            return -2;

        VFS::IOStatus io_status = vfs_ctx->vfs_subsys->delete_node(k_node_path);
        switch (io_status) {
            case VFS::IOStatus::DELETED:
                return 0;
            case VFS::IOStatus::ACCESS_DENIED:
                return -3;
            case VFS::IOStatus::NOT_FOUND:
                return -4;
            default:
                return -5; // DEV_ERROR or DEV_UNKNOWN
        }
    }


    S64 vfs_close(void* sys_call_ctx, U64 arg1) {
        auto* vfs_ctx = (VFSContext*) sys_call_ctx;
        auto node_handle = (U16) arg1;
        if (node_handle == 0)
            return -1;

        SharedPointer<VFS::Node> node = vfs_ctx->vfs_subsys->find_node(node_handle);
        if (!node)
            return -2;

        node->close();
        return 0;
    }


    S64 vfs_read(void* sys_call_ctx, U64 handle, U64 u_buf, U64 u_buf_size) {
        auto* vfs_ctx = (VFSContext*) sys_call_ctx;
        U16 node_handle = handle;
        if (node_handle == 0)
            return -1;

        SharedPointer<VFS::Node> node = vfs_ctx->vfs_subsys->find_node(node_handle);
        if (!node)
            return -2;

        if (!node->has_attribute(Ember::NodeAttribute::FILE))
            return -3;

        void* u_buff = (void*) u_buf;
        size_t u_buff_size = u_buf_size;
        if (!vfs_ctx->k_guard->verify_user_buffer(u_buff, u_buff_size))
            return -4;

        U8 k_buf[u_buff_size];
        memset(k_buf, 0, u_buff_size);
        VFS::NodeIOResult io_res = node->read(k_buf, u_buff_size);
        if (io_res.status == VFS::NodeIOStatus::NOT_SUPPORTED)
            return -5;
        else if (io_res.status == VFS::NodeIOStatus::CLOSED)
            return -6;
        else if (io_res.status == VFS::NodeIOStatus::DEV_ERROR)
            return -7;

        if (!vfs_ctx->k_guard->copy_byte_buffer_kernel_to_user(k_buf, u_buff, u_buff_size))
            return -8;

        return (S64) io_res.byte_count;
    }


    S64 vfs_write(void* sys_call_ctx, U64 handle, U64 u_buf, U64 u_buf_size) {
        auto* vfs_ctx = (VFSContext*) sys_call_ctx;
        U16 node_handle = handle;
        if (node_handle == 0)
            return -1;

        SharedPointer<VFS::Node> node = vfs_ctx->vfs_subsys->find_node(node_handle);
        if (!node)
            return -2;

        if (!node->has_attribute(Ember::NodeAttribute::FILE))
            return -3;

        void* u_buff = (void*) u_buf;
        size_t u_buff_size = u_buf_size;
        U8     k_buf[u_buff_size];
        memset(k_buf, 0, u_buff_size);
        if (!vfs_ctx->k_guard->copy_byte_buffer_user_to_kernel(u_buff, u_buff_size, k_buf))
            return -4;

        VFS::NodeIOResult io_res = node->write(k_buf, u_buff_size);

        switch (io_res.status) {
            case VFS::NodeIOStatus::OKAY:
                return (S64) io_res.byte_count;
            case VFS::NodeIOStatus::NOT_SUPPORTED:
                return -5;
            case VFS::NodeIOStatus::NOT_ALLOWED:
                return -6;
            case VFS::NodeIOStatus::CLOSED:
                return -7;
            default:
                return -8; // DEV_ERROR
        }
    }


    S64 vfs_seek(void* sys_call_ctx, const U64 handle, const U64 seek_mode, const U64 offset) {
        const auto* vfs_ctx     = static_cast<VFSContext*>(sys_call_ctx);
        const U16   node_handle = handle;
        if (node_handle == 0)
            return -1;

        const SharedPointer<VFS::Node> node = vfs_ctx->vfs_subsys->find_node(node_handle);
        if (!node)
            return -2;

        const Ember::SeekMode k_seek_mode = Ember::SeekMode(seek_mode);
        if (k_seek_mode == Ember::SeekMode::NONE)
            return -3;

        switch (auto [status, byte_count] = node->seek(k_seek_mode, static_cast<int>(offset)); status) {
            case VFS::NodeIOStatus::OKAY:
                return static_cast<S64>(byte_count);
            case VFS::NodeIOStatus::NOT_SUPPORTED:
                return -4;
            case VFS::NodeIOStatus::CLOSED:
                return -5;
            default:
                return -6; // DEV_ERROR
        }
    }


    S64 vfs_directory_stream_open(void* sys_call_ctx, U64 dir_path) {
        auto* vfs_ctx = (VFSContext*) sys_call_ctx;

        auto* u_node_path = (const char*) dir_path;
        char k_node_path_buf[KernelGuardian::USER_STRING_LIMIT];
        memset(k_node_path_buf, 0, KernelGuardian::USER_STRING_LIMIT);
        if (!vfs_ctx->k_guard->copy_string_user_to_kernel(u_node_path, -1, k_node_path_buf))
            return -1;

        Path k_node_path(k_node_path_buf);
        // If the path is relative prepend the working directory of the active app
        if (!k_node_path.is_absolute())
            k_node_path = k_node_path.resolve(vfs_ctx->app_subsys->get_active_app()->working_directory);


        if (!vfs_ctx->vfs_subsys->is_valid_file_path(k_node_path))
            return -2;

        SharedPointer<VFS::DirectoryStream> dir_stream;
        VFS::IOStatus                       io_status = vfs_ctx->vfs_subsys
                                                               ->open_directory_stream(k_node_path, dir_stream);
        switch (io_status) {
            case VFS::IOStatus::OPENED:
                return dir_stream->handle;
            case VFS::IOStatus::BAD_PATH:
                return -5; // Path is made absolute if not -> Path must be a file
            case VFS::IOStatus::OUT_OF_HANDLES:
                return -3;
            case VFS::IOStatus::NOT_FOUND:
                return -4;
            default:
                return -6; //DEV_ERROR or DEV_UNKNOWN
        }
    }


    S64 vfs_directory_stream_next(void* sys_call_ctx, U64 dir_stream_handle, U64 node_info_ptr) {
        auto* vfs_ctx = (VFSContext*) sys_call_ctx;
        U16 k_handle = (U16) dir_stream_handle;
        if (k_handle == 0)
            return -1;

        if (!vfs_ctx->k_guard->verify_user_buffer((void*) node_info_ptr, sizeof(VFSNodeInfo)))
            return -2;

        SharedPointer<VFS::DirectoryStream> dir_stream = vfs_ctx->vfs_subsys->find_directory_stream(k_handle);
        if (!dir_stream)
            return -3;

        VFS::NodeInfo node_info = dir_stream->get_next();
        if (dir_stream->get_state() == VFS::DirectoryStreamState::IO_ERROR)
            return -4;

        auto* u_node_info = (VFSNodeInfo*) node_info_ptr;
        if (!vfs_ctx->k_guard->copy_byte_buffer_kernel_to_user(
                (void*) node_info.node_path.to_cstr(),
                &u_node_info->node_path,
                node_info.node_path.size() + 1 // size() does not include null terminator
        ))
            return -5;

        u_node_info->size       = node_info.size;
        u_node_info->attributes = node_info.attributes;
        return dir_stream->get_state() == VFS::DirectoryStreamState::HAS_MORE ? 1 : 0;
    }


    S64 vfs_directory_stream_close(void* sys_call_ctx, U64 dir_stream_handle) {
        auto* vfs_ctx = (VFSContext*) sys_call_ctx;
        U16 k_handle = (U16) dir_stream_handle;
        if (k_handle == 0)
            return -1;

        SharedPointer<VFS::DirectoryStream> dir_stream = vfs_ctx->vfs_subsys->find_directory_stream(k_handle);
        if (!dir_stream)
            return -2;

        dir_stream->close();
        return 0;
    }
}