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

#include <Ember/Ember.h>
#include <Ember/VFSBits.h>

namespace Rune::SystemCall {
    auto vfs_get_node_info(void* sys_call_ctx, const U64 node_path, const U64 node_info_out)
        -> Ember::StatusCode {
        const auto* vfs_ctx     = static_cast<VFSSystemCallContext*>(sys_call_ctx);
        const auto* u_node_path = reinterpret_cast<const char*>(node_path);
        char        k_node_path_buf[Ember::STRING_SIZE_LIMIT] = {}; // NOLINT Is Kernel ABI
        if (!vfs_ctx->k_guard->copy_string_user_to_kernel(u_node_path, -1, k_node_path_buf))
            return Ember::Status::BAD_ARG;

        Path k_node_path(k_node_path_buf);
        // If the path is relative prepend the working directory of the active app
        if (!k_node_path.is_absolute())
            k_node_path =
                k_node_path.resolve(vfs_ctx->app_module->get_active_app()->working_directory);

        if (!vfs_ctx->vfs_module->is_valid_file_path(k_node_path)) return Ember::Status::BAD_ARG;

        VFS::NodeInfo k_node_info_buf;
        if (!vfs_ctx->k_guard->copy_byte_buffer_user_to_kernel(
                reinterpret_cast<void*>(node_info_out),
                sizeof(VFS::NodeInfo),
                (void*) &k_node_info_buf))
            return Ember::Status::BAD_ARG;

        switch (VFS::IOStatus io_status =
                    vfs_ctx->vfs_module->get_node_info(k_node_path, k_node_info_buf)) {
            case VFS::IOStatus::FOUND: {
                auto* const u_node_info = memory_addr_to_pointer<Ember::NodeInfo>(node_info_out);
                // The node info will only contain the node name -> set it to the path the caller
                // has provided
                if (!vfs_ctx->k_guard->copy_byte_buffer_kernel_to_user(
                        (void*) k_node_info_buf.node_path.to_cstr(),
                        (void*) u_node_info->node_path,
                        k_node_info_buf.node_path.size()
                            + 1 // size() does not include null terminator
                        )) {
                    return Ember::Status::BAD_ARG;
                }
                u_node_info->size       = k_node_info_buf.size;
                u_node_info->attributes = k_node_info_buf.attributes;
                return Ember::Status::OKAY;
            }

            case VFS::IOStatus::NOT_FOUND: return Ember::Status::NODE_NOT_FOUND;
            case VFS::IOStatus::BAD_PATH:  return Ember::Status::BAD_ARG;
            default:                       return Ember::Status::IO_ERROR; // DEV_ERROR and DEV_UNKNOWN
        }
    }

    auto vfs_get_node_info_by_ID(void* sys_call_ctx, U64 node_ID, U64 node_info_out)
        -> Ember::StatusCode {
        const auto* vfs_ctx = static_cast<VFSSystemCallContext*>(sys_call_ctx);

        if (node_ID == 0) return Ember::Status::BAD_ARG;

        VFS::NodeInfo k_node_info_buf;
        if (!vfs_ctx->k_guard->copy_byte_buffer_user_to_kernel(
                reinterpret_cast<void*>(node_info_out),
                sizeof(VFS::NodeInfo),
                (void*) &k_node_info_buf))
            return Ember::Status::BAD_ARG;

        VFS::IOStatus io_status = vfs_ctx->vfs_module->get_node_info(node_ID, k_node_info_buf);
        if (io_status == VFS::IOStatus::NOT_FOUND) return Ember::Status::NODE_NOT_FOUND;

        auto* const u_node_info = memory_addr_to_pointer<Ember::NodeInfo>(node_info_out);
        // The node info will only contain the node name -> set it to the path the caller
        // has provided
        if (!vfs_ctx->k_guard->copy_byte_buffer_kernel_to_user(
                (void*) k_node_info_buf.node_path.to_cstr(),
                (void*) u_node_info->node_path,
                k_node_info_buf.node_path.size() + 1 // size() does not include null terminator
                )) {
            return Ember::Status::BAD_ARG;
        }
        u_node_info->size       = k_node_info_buf.size;
        u_node_info->attributes = k_node_info_buf.attributes;
        return Ember::Status::OKAY;
    }

    auto vfs_create(void* sys_call_ctx, const U64 node_path, const U64 node_attr)
        -> Ember::StatusCode {
        const auto* vfs_ctx     = static_cast<VFSSystemCallContext*>(sys_call_ctx);
        const auto* u_node_path = reinterpret_cast<const char*>(node_path);
        char        k_node_path_buf[Ember::STRING_SIZE_LIMIT] = {}; // NOLINT Is Kernel ABI
        if (!vfs_ctx->k_guard->copy_string_user_to_kernel(u_node_path, -1, k_node_path_buf))
            return Ember::Status::BAD_ARG;

        const U8     k_node_attr = node_attr;
        constexpr U8 verify_mask = Ember::NodeAttribute::READONLY | Ember::NodeAttribute::HIDDEN
                                   | Ember::NodeAttribute::SYSTEM | Ember::NodeAttribute::DIRECTORY
                                   | Ember::NodeAttribute::FILE;
        if ((k_node_attr & ~verify_mask) != 0) return Ember::Status::BAD_ARG;

        Path k_node_path(k_node_path_buf);
        // If the path is relative prepend the working directory of the active app
        if (!k_node_path.is_absolute())
            k_node_path =
                k_node_path.resolve(vfs_ctx->app_module->get_active_app()->working_directory);

        if (!vfs_ctx->vfs_module->is_valid_file_path(k_node_path)) return Ember::Status::BAD_ARG;

        switch (VFS::IOStatus io_status = vfs_ctx->vfs_module->create(k_node_path, k_node_attr)) {
            case VFS::IOStatus::CREATED:       return Ember::Status::OKAY;
            case VFS::IOStatus::FOUND:         return Ember::Status::NODE_EXISTS;
            case VFS::IOStatus::BAD_ATTRIBUTE: return Ember::Status::BAD_ARG;
            default:                           return Ember::Status::IO_ERROR; // DEV_ERROR or StorageDevUnknown
        }
    }

    auto vfs_open(void* sys_call_ctx, const U64 node_path, const U64 io_mode) -> Ember::StatusCode {
        const auto* vfs_ctx     = static_cast<VFSSystemCallContext*>(sys_call_ctx);
        const auto* u_node_path = reinterpret_cast<const char*>(node_path);
        char        k_node_path_buf[Ember::STRING_SIZE_LIMIT] = {}; // NOLINT Is Kernel ABI
        if (!vfs_ctx->k_guard->copy_string_user_to_kernel(u_node_path, -1, k_node_path_buf))
            return Ember::Status::BAD_ARG;

        const Ember::IOMode k_io_mode(io_mode);
        if (k_io_mode == Ember::IOMode::NONE) return Ember::Status::BAD_ARG;

        Path k_node_path(k_node_path_buf);
        // If the path is relative prepend the working directory of the active app
        if (!k_node_path.is_absolute())
            k_node_path =
                k_node_path.resolve(vfs_ctx->app_module->get_active_app()->working_directory);

        if (!vfs_ctx->vfs_module->is_valid_file_path(k_node_path)) return Ember::Status::BAD_ARG;

        SharedPointer<VFS::Node> node;

        switch (VFS::IOStatus io_status = vfs_ctx->vfs_module->open(k_node_path, k_io_mode, node)) {
            case VFS::IOStatus::OPENED:    return node->handle;
            case VFS::IOStatus::NOT_FOUND: return Ember::Status::NODE_NOT_FOUND;
            default:
                return Ember::Status::IO_ERROR; // OUT_OF_HANDLES, DEV_ERROR or StorageDevUnknown
        }
    }

    auto vfs_delete(void* sys_call_ctx, const U64 node_path) -> Ember::StatusCode {
        const auto* vfs_ctx = static_cast<VFSSystemCallContext*>(sys_call_ctx);

        const auto* u_node_path = reinterpret_cast<const char*>(node_path);
        char        k_node_path_buf[Ember::STRING_SIZE_LIMIT] = {}; // NOLINT Is Kernel ABI
        if (!vfs_ctx->k_guard->copy_string_user_to_kernel(u_node_path, -1, k_node_path_buf))
            return Ember::Status::BAD_ARG;

        Path k_node_path(k_node_path_buf);
        // If the path is relative prepend the working directory of the active app
        if (!k_node_path.is_absolute())
            k_node_path =
                k_node_path.resolve(vfs_ctx->app_module->get_active_app()->working_directory);

        if (!vfs_ctx->vfs_module->is_valid_file_path(k_node_path)) return Ember::Status::BAD_ARG;

        switch (VFS::IOStatus io_status = vfs_ctx->vfs_module->delete_node(k_node_path)) {
            case VFS::IOStatus::DELETED:       return Ember::Status::OKAY;
            case VFS::IOStatus::ACCESS_DENIED: return Ember::Status::NODE_IN_USE;
            case VFS::IOStatus::NOT_FOUND:     return Ember::Status::NODE_NOT_FOUND;
            default:                           return Ember::Status::IO_ERROR; // DEV_ERROR or DEV_UNKNOWN
        }
    }

    auto vfs_close(void* sys_call_ctx, const U64 ID) -> Ember::StatusCode {
        const auto* vfs_ctx = static_cast<VFSSystemCallContext*>(sys_call_ctx);
        const auto  node_ID = static_cast<U16>(ID);
        if (node_ID == 0) return Ember::Status::BAD_ARG;

        const SharedPointer<VFS::Node> node = vfs_ctx->vfs_module->find_node(node_ID);
        if (!node) return Ember::Status::UNKNOWN_ID;

        node->close();
        return Ember::Status::OKAY;
    }

    auto vfs_read(void* sys_call_ctx, const U64 ID, const U64 buf, const U64 buf_size)
        -> Ember::StatusCode {
        const auto* vfs_ctx = static_cast<VFSSystemCallContext*>(sys_call_ctx);
        const U16   node_ID = ID;
        if (node_ID == 0) return Ember::Status::BAD_ARG;

        const SharedPointer<VFS::Node> node = vfs_ctx->vfs_module->find_node(node_ID);
        if (!node) return Ember::Status::UNKNOWN_ID;

        if (!node->has_attribute(Ember::NodeAttribute::FILE))
            return Ember::Status::NODE_IS_DIRECTORY;

        auto* const  u_buf      = reinterpret_cast<void*>(buf);
        const size_t u_buf_size = buf_size;
        if (!vfs_ctx->k_guard->verify_user_buffer(u_buf, u_buf_size)) return Ember::Status::BAD_ARG;

        U8 k_buf[u_buf_size]; // NOLINT Is Kernel ABI
        memset(k_buf, 0, u_buf_size);
        auto [status, byte_count] = node->read(k_buf, u_buf_size);
        if (status == VFS::NodeIOStatus::NOT_SUPPORTED) return Ember::Status::ACCESS_DENIED;
        if (status == VFS::NodeIOStatus::CLOSED) return Ember::Status::NODE_CLOSED;
        if (status == VFS::NodeIOStatus::DEV_ERROR) return Ember::Status::IO_ERROR;

        if (!vfs_ctx->k_guard->copy_byte_buffer_kernel_to_user(k_buf, u_buf, u_buf_size))
            return -Ember::Status::IO_ERROR;

        return static_cast<Ember::StatusCode>(byte_count);
    }

    auto vfs_write(void* sys_call_ctx, const U64 ID, const U64 buf, const U64 buf_size)
        -> Ember::StatusCode {
        const auto* vfs_ctx = static_cast<VFSSystemCallContext*>(sys_call_ctx);
        const U16   node_ID = ID;
        if (node_ID == 0) return Ember::Status::BAD_ARG;

        const SharedPointer<VFS::Node> node = vfs_ctx->vfs_module->find_node(node_ID);
        if (!node) return Ember::Status::UNKNOWN_ID;

        if (!node->has_attribute(Ember::NodeAttribute::FILE))
            return Ember::Status::NODE_IS_DIRECTORY;

        auto* const  u_buf      = reinterpret_cast<void*>(buf);
        const size_t u_buf_size = buf_size;
        U8           k_buf[u_buf_size]; // NOLINT Is Kernel ABI
        memset(k_buf, 0, u_buf_size);
        if (!vfs_ctx->k_guard->copy_byte_buffer_user_to_kernel(u_buf, u_buf_size, k_buf))
            return Ember::Status::BAD_ARG;

        switch (auto [status, byte_count] = node->write(k_buf, u_buf_size); status) {
            case VFS::NodeIOStatus::OKAY:          return static_cast<Ember::StatusCode>(byte_count);
            case VFS::NodeIOStatus::NOT_SUPPORTED: return Ember::Status::NODE_IS_DIRECTORY;
            case VFS::NodeIOStatus::NOT_ALLOWED:   return Ember::Status::ACCESS_DENIED;
            case VFS::NodeIOStatus::CLOSED:        return Ember::Status::NODE_CLOSED;
            default:                               return Ember::Status::IO_ERROR; // DEV_ERROR
        }
    }

    auto vfs_seek(void* sys_call_ctx, const U64 ID, const U64 seek_mode, const U64 offset)
        -> Ember::StatusCode {
        const auto* vfs_ctx     = static_cast<VFSSystemCallContext*>(sys_call_ctx);
        const U16   node_handle = ID;
        if (node_handle == 0) return Ember::Status::BAD_ARG;

        const SharedPointer<VFS::Node> node = vfs_ctx->vfs_module->find_node(node_handle);
        if (!node) return Ember::Status::UNKNOWN_ID;

        if (!node->has_attribute(Ember::NodeAttribute::FILE))
            return Ember::Status::NODE_IS_DIRECTORY;

        const auto k_seek_mode = Ember::SeekMode(seek_mode);
        if (k_seek_mode == Ember::SeekMode::NONE) return Ember::Status::BAD_ARG;

        switch (auto [status, byte_count] = node->seek(k_seek_mode, static_cast<int>(offset));
                status) {
            case VFS::NodeIOStatus::OKAY:          return static_cast<Ember::StatusCode>(byte_count);
            case VFS::NodeIOStatus::NOT_SUPPORTED: return Ember::Status::NODE_IS_DIRECTORY;
            case VFS::NodeIOStatus::CLOSED:        return Ember::Status::NODE_CLOSED;
            default:                               return Ember::Status::IO_ERROR; // DEV_ERROR
        }
    }

    auto vfs_directory_stream_open(void* sys_call_ctx, const U64 dir_path) -> Ember::StatusCode {
        const auto* vfs_ctx = static_cast<VFSSystemCallContext*>(sys_call_ctx);

        const auto* u_node_path = reinterpret_cast<const char*>(dir_path);
        char        k_node_path_buf[Ember::STRING_SIZE_LIMIT] = {}; // NOLINT Is Kernel ABI
        if (!vfs_ctx->k_guard->copy_string_user_to_kernel(u_node_path, -1, k_node_path_buf))
            return Ember::Status::BAD_ARG;

        Path k_node_path(k_node_path_buf);
        // If the path is relative prepend the working directory of the active app
        if (!k_node_path.is_absolute())
            k_node_path =
                k_node_path.resolve(vfs_ctx->app_module->get_active_app()->working_directory);

        if (!vfs_ctx->vfs_module->is_valid_file_path(k_node_path)) return Ember::Status::BAD_ARG;

        SharedPointer<VFS::DirectoryStream> dir_stream;
        const VFS::IOStatus                 io_status =
            vfs_ctx->vfs_module->open_directory_stream(k_node_path, dir_stream);
        switch (io_status) {
            case VFS::IOStatus::OPENED: return dir_stream->handle;
            case VFS::IOStatus::BAD_PATH:
                return Ember::Status::NODE_IS_FILE; // Path is made absolute -> Path must be a file
            case VFS::IOStatus::OUT_OF_HANDLES: return Ember::Status::IO_ERROR;
            case VFS::IOStatus::NOT_FOUND:      return Ember::Status::NODE_NOT_FOUND;
            default:                            return Ember::Status::IO_ERROR; // DEV_ERROR or DEV_UNKNOWN
        }
    }

    auto vfs_directory_stream_next(void* sys_call_ctx, const U64 dir_stream_ID, U64 node_info_ptr)
        -> Ember::StatusCode {
        const auto* vfs_ctx = static_cast<VFSSystemCallContext*>(sys_call_ctx);
        const U16   k_ID    = static_cast<U16>(dir_stream_ID);
        if (k_ID == 0) return Ember::Status::BAD_ARG;

        if (!vfs_ctx->k_guard->verify_user_buffer(reinterpret_cast<void*>(node_info_ptr),
                                                  sizeof(Ember::NodeInfo)))
            return Ember::Status::BAD_ARG;

        const SharedPointer<VFS::DirectoryStream> dir_stream =
            vfs_ctx->vfs_module->find_directory_stream(k_ID);
        if (!dir_stream) return Ember::Status::UNKNOWN_ID;

        auto maybe_node_info = dir_stream->next();
        if (!maybe_node_info) {
            return maybe_node_info.error() == VFS::DirectoryStreamStatus::END_OF_DIRECTORY
                       ? Ember::Status::DIRECTORY_STREAM_EOD
                       : Ember::Status::IO_ERROR;
        }

        const auto& k_node_info = maybe_node_info.value();
        auto*       u_node_info = reinterpret_cast<Ember::NodeInfo*>(node_info_ptr);
        if (!vfs_ctx->k_guard->copy_byte_buffer_kernel_to_user(
                (void*) k_node_info.node_path.to_cstr(),
                &u_node_info->node_path,
                k_node_info.node_path.size() + 1 // size() does not include null terminator
                ))
            return Ember::Status::BAD_ARG;
        u_node_info->size       = k_node_info.size;
        u_node_info->attributes = k_node_info.attributes;
        return Ember::Status::DIRECTORY_STREAM_HAS_MORE;
    }

    auto vfs_directory_stream_close(void* sys_call_ctx, const U64 dir_stream_ID)
        -> Ember::StatusCode {
        const auto* vfs_ctx = static_cast<VFSSystemCallContext*>(sys_call_ctx);
        const U16   k_ID    = static_cast<U16>(dir_stream_ID);
        if (k_ID == 0) return Ember::Status::BAD_ARG;

        const SharedPointer<VFS::DirectoryStream> dir_stream =
            vfs_ctx->vfs_module->find_directory_stream(k_ID);
        if (!dir_stream) return Ember::Status::UNKNOWN_ID;

        dir_stream->close();
        return Ember::Status::OKAY;
    }
} // namespace Rune::SystemCall
