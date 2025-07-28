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

#include <SystemCall/VFSBundle.h>

#include <Ember/StatusCode.h>
#include <Ember/NodeDefinitions.h>


namespace Rune::SystemCall {
    S64 vfs_get_node_info(void* sys_call_ctx, const U64 node_path, const U64 node_info_out) {
        const auto* vfs_ctx                                   = static_cast<VFSContext*>(sys_call_ctx);
        auto*       u_node_path                               = reinterpret_cast<const char*>(node_path);
        char        k_node_path_buf[Ember::STRING_SIZE_LIMIT] = { };
        if (!vfs_ctx->k_guard->copy_string_user_to_kernel(u_node_path, -1, k_node_path_buf))
            return Ember::StatusCode::BAD_ARG;

        Path k_node_path(k_node_path_buf);
        // If the path is relative prepend the working directory of the active app
        if (!k_node_path.is_absolute())
            k_node_path = k_node_path.resolve(vfs_ctx->app_subsys->get_active_app()->working_directory);

        if (!vfs_ctx->vfs_subsys->is_valid_file_path(k_node_path))
            return Ember::StatusCode::BAD_ARG;

        VFS::NodeInfo k_node_info_buf;
        if (!vfs_ctx->k_guard
                    ->copy_byte_buffer_user_to_kernel(
                        (void*)node_info_out,
                        sizeof(VFS::NodeInfo),
                        (void*)&k_node_info_buf
                    ))
            return Ember::StatusCode::BAD_ARG;

        switch (VFS::IOStatus io_status = vfs_ctx->vfs_subsys->get_node_info(k_node_path, k_node_info_buf)) {
            case VFS::IOStatus::FOUND: {
                const auto u_node_info = LibK::memory_addr_to_pointer<Ember::NodeInfo>(node_info_out);
                // The node info will only contain the node name -> set it to the path the caller has provided
                if (!vfs_ctx->k_guard
                            ->copy_byte_buffer_kernel_to_user(
                                (void*)k_node_info_buf.node_path.to_cstr(),
                                (void*)u_node_info->node_path,
                                k_node_info_buf.node_path.size() + 1 // size() does not include null terminator
                            )) {
                    return Ember::StatusCode::BAD_ARG;
                }
                u_node_info->size       = k_node_info_buf.size;
                u_node_info->attributes = k_node_info_buf.attributes;
                return Ember::StatusCode::OKAY;
            }

            case VFS::IOStatus::NOT_FOUND:
                return Ember::StatusCode::NODE_NOT_FOUND;
            case VFS::IOStatus::BAD_PATH:
                return Ember::StatusCode::BAD_ARG;
            default:
                return Ember::StatusCode::IO_ERROR; // DEV_ERROR and DEV_UNKNOWN
        }
    }


    S64 vfs_create(void* sys_call_ctx, const U64 node_path, const U64 node_attr) {
        const auto* vfs_ctx                                   = static_cast<VFSContext*>(sys_call_ctx);
        auto*       u_node_path                               = reinterpret_cast<const char*>(node_path);
        char        k_node_path_buf[Ember::STRING_SIZE_LIMIT] = { };
        if (!vfs_ctx->k_guard->copy_string_user_to_kernel(u_node_path, -1, k_node_path_buf))
            return Ember::StatusCode::BAD_ARG;

        const U8     k_node_attr = node_attr;
        constexpr U8 verify_mask = Ember::NodeAttribute::READONLY
            | Ember::NodeAttribute::HIDDEN
            | Ember::NodeAttribute::SYSTEM
            | Ember::NodeAttribute::DIRECTORY
            | Ember::NodeAttribute::FILE;
        if ((k_node_attr & ~verify_mask) != 0)
            return Ember::StatusCode::BAD_ARG;

        Path k_node_path(k_node_path_buf);
        // If the path is relative prepend the working directory of the active app
        if (!k_node_path.is_absolute())
            k_node_path = k_node_path.resolve(vfs_ctx->app_subsys->get_active_app()->working_directory);

        if (!vfs_ctx->vfs_subsys->is_valid_file_path(k_node_path))
            return Ember::StatusCode::BAD_ARG;

        switch (VFS::IOStatus io_status = vfs_ctx->vfs_subsys->create(k_node_path, k_node_attr)) {
            case VFS::IOStatus::CREATED:
                return Ember::StatusCode::OKAY;
            case VFS::IOStatus::FOUND:
                return Ember::StatusCode::NODE_EXISTS;
            case VFS::IOStatus::BAD_ATTRIBUTE:
                return Ember::StatusCode::BAD_ARG;
            default:
                return Ember::StatusCode::IO_ERROR; // DEV_ERROR or StorageDevUnknown
        }
    }


    S64 vfs_open(void* sys_call_ctx, const U64 node_path, const U64 io_mode) {
        const auto* vfs_ctx                                   = static_cast<VFSContext*>(sys_call_ctx);
        auto*       u_node_path                               = reinterpret_cast<const char*>(node_path);
        char        k_node_path_buf[Ember::STRING_SIZE_LIMIT] = { };
        if (!vfs_ctx->k_guard->copy_string_user_to_kernel(u_node_path, -1, k_node_path_buf))
            return Ember::StatusCode::BAD_ARG;

        const Ember::IOMode k_io_mode(io_mode);
        if (k_io_mode == Ember::IOMode::NONE)
            return Ember::StatusCode::BAD_ARG;

        Path k_node_path(k_node_path_buf);
        // If the path is relative prepend the working directory of the active app
        if (!k_node_path.is_absolute())
            k_node_path = k_node_path.resolve(vfs_ctx->app_subsys->get_active_app()->working_directory);

        if (!vfs_ctx->vfs_subsys->is_valid_file_path(k_node_path))
            return Ember::StatusCode::BAD_ARG;

        SharedPointer<VFS::Node> node;

        switch (VFS::IOStatus io_status = vfs_ctx->vfs_subsys->open(k_node_path, k_io_mode, node)) {
            case VFS::IOStatus::OPENED:
                return node->handle;
            case VFS::IOStatus::NOT_FOUND:
                return Ember::StatusCode::NODE_NOT_FOUND;
            case VFS::IOStatus::OUT_OF_HANDLES:
                return Ember::StatusCode::IO_ERROR;
            default:
                return Ember::StatusCode::IO_ERROR; // DEV_ERROR or StorageDevUnknown
        }
    }


    S64 vfs_delete(void* sys_call_ctx, const U64 node_path) {
        const auto* vfs_ctx = static_cast<VFSContext*>(sys_call_ctx);

        auto* u_node_path                               = reinterpret_cast<const char*>(node_path);
        char  k_node_path_buf[Ember::STRING_SIZE_LIMIT] = { };
        if (!vfs_ctx->k_guard->copy_string_user_to_kernel(u_node_path, -1, k_node_path_buf))
            return Ember::StatusCode::BAD_ARG;

        Path k_node_path(k_node_path_buf);
        // If the path is relative prepend the working directory of the active app
        if (!k_node_path.is_absolute())
            k_node_path = k_node_path.resolve(vfs_ctx->app_subsys->get_active_app()->working_directory);

        if (!vfs_ctx->vfs_subsys->is_valid_file_path(k_node_path))
            return Ember::StatusCode::BAD_ARG;

        switch (VFS::IOStatus io_status = vfs_ctx->vfs_subsys->delete_node(k_node_path)) {
            case VFS::IOStatus::DELETED:
                return Ember::StatusCode::OKAY;
            case VFS::IOStatus::ACCESS_DENIED:
                return Ember::StatusCode::NODE_IN_USE;
            case VFS::IOStatus::NOT_FOUND:
                return Ember::StatusCode::NODE_NOT_FOUND;
            default:
                return Ember::StatusCode::IO_ERROR; // DEV_ERROR or DEV_UNKNOWN
        }
    }


    S64 vfs_close(void* sys_call_ctx, const U64 ID) {
        const auto* vfs_ctx = static_cast<VFSContext*>(sys_call_ctx);
        const auto  node_ID = static_cast<U16>(ID);
        if (node_ID == 0)
            return Ember::StatusCode::BAD_ARG;

        const SharedPointer<VFS::Node> node = vfs_ctx->vfs_subsys->find_node(node_ID);
        if (!node)
            return Ember::StatusCode::UNKNOWN_ID;

        node->close();
        return Ember::StatusCode::OKAY;
    }


    S64 vfs_read(void* sys_call_ctx, const U64 ID, const U64 u_buf, const U64 u_buf_size) {
        const auto* vfs_ctx = static_cast<VFSContext*>(sys_call_ctx);
        const U16   node_ID = ID;
        if (node_ID == 0)
            return Ember::StatusCode::BAD_ARG;

        const SharedPointer<VFS::Node> node = vfs_ctx->vfs_subsys->find_node(node_ID);
        if (!node)
            return Ember::StatusCode::UNKNOWN_ID;

        if (!node->has_attribute(Ember::NodeAttribute::FILE))
            return Ember::StatusCode::NODE_IS_DIRECTORY;

        const auto   u_buff      = reinterpret_cast<void*>(u_buf);
        const size_t u_buff_size = u_buf_size;
        if (!vfs_ctx->k_guard->verify_user_buffer(u_buff, u_buff_size))
            return Ember::StatusCode::BAD_ARG;

        U8 k_buf[u_buff_size];
        memset(k_buf, 0, u_buff_size);
        auto [status, byte_count] = node->read(k_buf, u_buff_size);
        if (status == VFS::NodeIOStatus::NOT_SUPPORTED)
            return Ember::StatusCode::ACCESS_DENIED;
        if (status == VFS::NodeIOStatus::CLOSED)
            return Ember::StatusCode::NODE_CLOSED;
        if (status == VFS::NodeIOStatus::DEV_ERROR)
            return Ember::StatusCode::IO_ERROR;

        if (!vfs_ctx->k_guard->copy_byte_buffer_kernel_to_user(k_buf, u_buff, u_buff_size))
            return -Ember::StatusCode::IO_ERROR;

        return static_cast<S64>(byte_count);
    }


    S64 vfs_write(void* sys_call_ctx, const U64 ID, const U64 u_buf, const U64 u_buf_size) {
        const auto* vfs_ctx = static_cast<VFSContext*>(sys_call_ctx);
        const U16   node_ID = ID;
        if (node_ID == 0)
            return Ember::StatusCode::BAD_ARG;

        const SharedPointer<VFS::Node> node = vfs_ctx->vfs_subsys->find_node(node_ID);
        if (!node)
            return Ember::StatusCode::UNKNOWN_ID;

        if (!node->has_attribute(Ember::NodeAttribute::FILE))
            return Ember::StatusCode::NODE_IS_DIRECTORY;

        const auto   u_buff      = reinterpret_cast<void*>(u_buf);
        const size_t u_buff_size = u_buf_size;
        U8           k_buf[u_buff_size];
        memset(k_buf, 0, u_buff_size);
        if (!vfs_ctx->k_guard->copy_byte_buffer_user_to_kernel(u_buff, u_buff_size, k_buf))
            return Ember::StatusCode::BAD_ARG;

        switch (auto [status, byte_count] = node->write(k_buf, u_buff_size); status) {
            case VFS::NodeIOStatus::OKAY:
                return static_cast<S64>(byte_count);
            case VFS::NodeIOStatus::NOT_SUPPORTED:
                return Ember::StatusCode::NODE_IS_DIRECTORY;
            case VFS::NodeIOStatus::NOT_ALLOWED:
                return Ember::StatusCode::ACCESS_DENIED;
            case VFS::NodeIOStatus::CLOSED:
                return Ember::StatusCode::NODE_CLOSED;
            default:
                return Ember::StatusCode::IO_ERROR; // DEV_ERROR
        }
    }


    S64 vfs_seek(void* sys_call_ctx, const U64 ID, const U64 seek_mode, const U64 offset) {
        const auto* vfs_ctx     = static_cast<VFSContext*>(sys_call_ctx);
        const U16   node_handle = ID;
        if (node_handle == 0)
            return Ember::StatusCode::BAD_ARG;

        const SharedPointer<VFS::Node> node = vfs_ctx->vfs_subsys->find_node(node_handle);
        if (!node)
            return Ember::StatusCode::UNKNOWN_ID;

        if (!node->has_attribute(Ember::NodeAttribute::FILE))
            return Ember::StatusCode::NODE_IS_DIRECTORY;

        const auto k_seek_mode = Ember::SeekMode(seek_mode);
        if (k_seek_mode == Ember::SeekMode::NONE)
            return Ember::StatusCode::BAD_ARG;

        switch (auto [status, byte_count] = node->seek(k_seek_mode, static_cast<int>(offset)); status) {
            case VFS::NodeIOStatus::OKAY:
                return static_cast<S64>(byte_count);
            case VFS::NodeIOStatus::NOT_SUPPORTED:
                return Ember::StatusCode::NODE_IS_DIRECTORY;
            case VFS::NodeIOStatus::CLOSED:
                return Ember::StatusCode::NODE_CLOSED;
            default:
                return Ember::StatusCode::IO_ERROR; // DEV_ERROR
        }
    }


    S64 vfs_directory_stream_open(void* sys_call_ctx, const U64 dir_path) {
        const auto* vfs_ctx = static_cast<VFSContext*>(sys_call_ctx);

        auto* u_node_path                               = reinterpret_cast<const char*>(dir_path);
        char  k_node_path_buf[Ember::STRING_SIZE_LIMIT] = { };
        if (!vfs_ctx->k_guard->copy_string_user_to_kernel(u_node_path, -1, k_node_path_buf))
            return Ember::StatusCode::BAD_ARG;

        Path k_node_path(k_node_path_buf);
        // If the path is relative prepend the working directory of the active app
        if (!k_node_path.is_absolute())
            k_node_path = k_node_path.resolve(vfs_ctx->app_subsys->get_active_app()->working_directory);

        if (!vfs_ctx->vfs_subsys->is_valid_file_path(k_node_path))
            return Ember::StatusCode::BAD_ARG;

        SharedPointer<VFS::DirectoryStream> dir_stream;
        const VFS::IOStatus                 io_status = vfs_ctx->vfs_subsys
                                               ->open_directory_stream(k_node_path, dir_stream);
        switch (io_status) {
            case VFS::IOStatus::OPENED:
                return dir_stream->handle;
            case VFS::IOStatus::BAD_PATH:
                return Ember::StatusCode::NODE_IS_FILE; // Path is made absolute -> Path must be a file
            case VFS::IOStatus::OUT_OF_HANDLES:
                return Ember::StatusCode::IO_ERROR;
            case VFS::IOStatus::NOT_FOUND:
                return Ember::StatusCode::NODE_NOT_FOUND;
            default:
                return Ember::StatusCode::IO_ERROR; //DEV_ERROR or DEV_UNKNOWN
        }
    }


    S64 vfs_directory_stream_next(void* sys_call_ctx, const U64 dir_stream_ID, U64 node_info_ptr) {
        const auto* vfs_ctx = static_cast<VFSContext*>(sys_call_ctx);
        const U16   k_ID    = static_cast<U16>(dir_stream_ID);
        if (k_ID == 0)
            return Ember::StatusCode::BAD_ARG;

        if (!vfs_ctx->k_guard->verify_user_buffer((void*)node_info_ptr, sizeof(Ember::NodeInfo)))
            return Ember::StatusCode::BAD_ARG;

        const SharedPointer<VFS::DirectoryStream> dir_stream = vfs_ctx->vfs_subsys->find_directory_stream(k_ID);
        if (!dir_stream)
            return Ember::StatusCode::UNKNOWN_ID;

        auto [node_path, size, attributes] = dir_stream->get_next();
        if (dir_stream->get_state() == VFS::DirectoryStreamState::IO_ERROR)
            return Ember::StatusCode::IO_ERROR;

        auto* u_node_info = reinterpret_cast<Ember::NodeInfo*>(node_info_ptr);
        if (!vfs_ctx->k_guard->copy_byte_buffer_kernel_to_user(
            (void*)node_path.to_cstr(),
            &u_node_info->node_path,
            node_path.size() + 1 // size() does not include null terminator
        ))
            return Ember::StatusCode::BAD_ARG;

        u_node_info->size       = size;
        u_node_info->attributes = attributes;
        return dir_stream->get_state() == VFS::DirectoryStreamState::HAS_MORE
                   ? Ember::StatusCode::DIRECTORY_STREAM_HAS_MORE
                   : Ember::StatusCode::DIRECTORY_STREAM_EOD;
    }


    S64 vfs_directory_stream_close(void* sys_call_ctx, const U64 dir_stream_ID) {
        const auto* vfs_ctx = static_cast<VFSContext*>(sys_call_ctx);
        const U16   k_ID    = static_cast<U16>(dir_stream_ID);
        if (k_ID == 0)
            return Ember::StatusCode::BAD_ARG;

        const SharedPointer<VFS::DirectoryStream> dir_stream = vfs_ctx->vfs_subsys->find_directory_stream(k_ID);
        if (!dir_stream)
            return Ember::StatusCode::UNKNOWN_ID;

        dir_stream->close();
        return Ember::StatusCode::OKAY;
    }
}
