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

#include <SystemCall/AppBundle.h>

#include <Ember/AppBits.h>
#include <Ember/Ember.h>

namespace Rune::SystemCall {
    Ember::StatusCode read_stdin(void* sys_call_ctx, const U64 key_code_out) {
        const auto* app_syscall_ctx = static_cast<AppSystemCallContext*>(sys_call_ctx);
        auto*       timer           = app_syscall_ctx->cpu_module->get_system_timer();

        Ember::VirtualKey key(app_syscall_ctx->app_module->get_active_app()->std_in->read());
        while (key.is_none()) {
            timer->sleep_milli(2); // 1ms is too fast, dunno why but nothing happens
            key = Ember::VirtualKey(app_syscall_ctx->app_module->get_active_app()->std_in->read());
        }
        U16   key_code        = key.get_key_code();
        auto* key_code_buffer = reinterpret_cast<U16*>(key_code_out);
        return app_syscall_ctx->k_guard->copy_byte_buffer_kernel_to_user((void*) &key_code,
                                                                         (void*) key_code_buffer,
                                                                         2)
                   ? 0
                   : Ember::Status::BAD_ARG;
    }

    Ember::StatusCode write_stdout(void* sys_call_ctx, const U64 msg, const U64 msg_size) {
        const auto* app_syscall_ctx = static_cast<AppSystemCallContext*>(sys_call_ctx);

        const char* k_buf_msg = new char[msg_size + 1];
        if (!app_syscall_ctx->k_guard->copy_string_user_to_kernel(
                reinterpret_cast<const char*>(msg),
                msg_size,
                k_buf_msg))
            return Ember::Status::BAD_ARG;
        const String            k_msg(k_buf_msg);
        const Ember::StatusCode byte_out = static_cast<Ember::StatusCode>(
            app_syscall_ctx->app_module->get_active_app()->std_out->write((U8*) k_buf_msg,
                                                                          k_msg.size()));
        delete[] k_buf_msg;
        return byte_out;
    }

    Ember::StatusCode write_stderr(void* sys_call_ctx, const U64 msg, const U64 msg_size) {
        const auto* app_syscall_ctx = static_cast<AppSystemCallContext*>(sys_call_ctx);

        const char* k_buf_msg = new char[msg_size + 1];
        if (!app_syscall_ctx->k_guard->copy_string_user_to_kernel(
                reinterpret_cast<const char*>(msg),
                msg_size,
                k_buf_msg))
            return Ember::Status::BAD_ARG;
        const String                    k_msg(k_buf_msg);
        const SharedPointer<TextStream> std_err =
            app_syscall_ctx->app_module->get_active_app()->std_err;

        std_err->set_foreground_color(Pixie::VSCODE_RED);
        const Ember::StatusCode byte_out = static_cast<Ember::StatusCode>(
            app_syscall_ctx->app_module->get_active_app()->std_err->write((U8*) k_buf_msg,
                                                                          k_msg.size()));
        std_err->reset_style();
        delete[] k_buf_msg;
        return byte_out;
    }

    Ember::StatusCode get_ID(void* sys_call_ctx) {
        const auto* app_syscall_ctx = static_cast<AppSystemCallContext*>(sys_call_ctx);
        return app_syscall_ctx->app_module->get_active_app()->handle;
    }

    Ember::StatusCode app_start(void*     sys_call_ctx,
                                const U64 app_path,
                                const U64 argv,
                                const U64 working_dir,
                                const U64 stdin_config,
                                const U64 stdout_config,
                                const U64 stderr_config) {
        const auto* app_syscall_ctx = static_cast<AppSystemCallContext*>(sys_call_ctx);
        auto*       u_app_path      = reinterpret_cast<const char*>(app_path);
        auto*       u_app_argv      = reinterpret_cast<const char**>(argv);
        auto*       u_app_wd        = reinterpret_cast<const char*>(working_dir);
        auto*       u_stdin_config  = reinterpret_cast<void*>(stdin_config);
        auto*       u_stdout_config = reinterpret_cast<void*>(stdout_config);
        auto*       u_stderr_config = reinterpret_cast<void*>(stderr_config);
        char        k_str_buf[Ember::STRING_SIZE_LIMIT] = {};

        if (!app_syscall_ctx->k_guard->copy_string_user_to_kernel(u_app_path, -1, k_str_buf))
            return Ember::Status::BAD_ARG;
        Path k_app_path(k_str_buf);
        memset(k_str_buf, '\0', Ember::STRING_SIZE_LIMIT);

        if (!app_syscall_ctx->k_guard->copy_string_user_to_kernel(u_app_wd, -1, k_str_buf))
            return Ember::Status::BAD_ARG;
        Path k_app_wd(k_str_buf);
        memset(k_str_buf, '\0', Ember::STRING_SIZE_LIMIT);

        Ember::StdIOConfig k_stdin_config;
        if (!app_syscall_ctx->k_guard->copy_byte_buffer_user_to_kernel(u_stdin_config,
                                                                       sizeof(Ember::StdIOConfig),
                                                                       &k_stdin_config))
            return Ember::Status::BAD_ARG;

        Ember::StdIOConfig k_stdout_config;
        if (!app_syscall_ctx->k_guard->copy_byte_buffer_user_to_kernel(u_stdout_config,
                                                                       sizeof(Ember::StdIOConfig),
                                                                       &k_stdout_config))
            return Ember::Status::BAD_ARG;

        Ember::StdIOConfig k_stderr_config;
        if (!app_syscall_ctx->k_guard->copy_byte_buffer_user_to_kernel(u_stderr_config,
                                                                       sizeof(Ember::StdIOConfig),
                                                                       &k_stderr_config))
            return Ember::Status::BAD_ARG;

        // Copy uAppArgv to kernel memory
        char* k_app_argv[Ember::ARG_COUNT_LIMIT];
        memset(k_app_argv, '\0', Ember::ARG_COUNT_LIMIT);

        // We will just copy over the maximum amount of arguments uAppArgv can have
        // This will likely be way more than we need, but we really don't want to access user memory
        // before it is copied over to kernel memory
        if (!app_syscall_ctx->k_guard->copy_byte_buffer_user_to_kernel(u_app_argv,
                                                                       sizeof(k_app_argv),
                                                                       k_app_argv))
            return Ember::Status::BAD_ARG;

        int    argc     = 0;
        char** tmp_argv = k_app_argv;
        while (*tmp_argv) {
            ++tmp_argv;
            ++argc;
        }

        // argv is null terminated and the first argument is the app path -> The caller can pass
        // (Ember::ARG_COUNT_LIMIT - 2) arguments
        if (argc >= Ember::ARG_COUNT_LIMIT - 2) return Ember::Status::BAD_ARG; // Too many arguments

        // The app args are still in user memory (we only copied the pointers to them over to
        // kernel) The array will hold the copies of the actual strings, which can hold the maximum
        // number of allowed arguments that have the maximum allowed size of a user memory string.
        char   k_app_args[Ember::ARG_COUNT_LIMIT * Ember::STRING_SIZE_LIMIT];
        size_t k_app_args_offset = 0;
        U8     i                 = 0;

        // Move all arguments up by one
        memmove(&k_app_argv[1], &k_app_argv[0], sizeof(char**) * (argc + 1));
        // Make k_app_argv[0] the app path
        memcpy(&k_app_args[k_app_args_offset],
               (void*) k_app_path.to_string().to_cstr(),
               k_app_path.to_string().size() + 1);
        k_app_argv[i]      = &k_app_args[k_app_args_offset];
        k_app_args_offset += k_app_path.to_string().size() + 1;
        i++;
        while (k_app_argv[i] && i < Ember::ARG_COUNT_LIMIT) {
            if (!app_syscall_ctx->k_guard->copy_string_user_to_kernel(k_app_argv[i], -1, k_str_buf))
                return Ember::Status::BAD_ARG;
            String arg_tmp(k_str_buf);
            memcpy(&k_app_args[k_app_args_offset], (void*) arg_tmp.to_cstr(), arg_tmp.size() + 1);

            // The pointer to the c string still points to the user land string -> Update it to the
            // kernel land string
            memset(k_str_buf, '\0', Ember::STRING_SIZE_LIMIT);
            k_app_argv[i]      = &k_app_args[k_app_args_offset];
            k_app_args_offset += arg_tmp.size() + 1;
            i++;
        }

        VFS::NodeInfo dummy;
        if (!k_app_path.is_absolute())
            k_app_path = k_app_path.resolve(
                app_syscall_ctx->app_module->get_active_app()->working_directory);
        if (app_syscall_ctx->vfs_module->get_node_info(k_app_path, dummy) != VFS::IOStatus::FOUND)
            // Path to the app executable does not exist
            return Ember::Status::NODE_NOT_FOUND;

        if (!k_app_wd.is_absolute())
            k_app_wd =
                k_app_wd.resolve(app_syscall_ctx->app_module->get_active_app()->working_directory);
        if (app_syscall_ctx->vfs_module->get_node_info(k_app_wd, dummy) != VFS::IOStatus::FOUND)
            // Path to the app working directory does not exist
            return Ember::Status::NODE_NOT_FOUND;

        auto [load_result, ID] = app_syscall_ctx->app_module->start_new_app(k_app_path,
                                                                            k_app_argv,
                                                                            k_app_wd,
                                                                            k_stdin_config,
                                                                            k_stdout_config,
                                                                            k_stderr_config);
        if (load_result != App::LoadStatus::RUNNING) return Ember::Status::FAULT;

        return ID;
    }

    Ember::StatusCode app_exit(void* sys_call_ctx, const U64 exit_code) {
        const auto* app_syscall_ctx = static_cast<AppSystemCallContext*>(sys_call_ctx);
        const int   k_exit_code     = static_cast<int>(exit_code);
        app_syscall_ctx->app_module->exit_running_app(k_exit_code);
        return Ember::Status::OKAY;
    }

    Ember::StatusCode app_join(void* sys_call_ctx, U64 ID) {
        const auto* app_syscall_ctx = static_cast<AppSystemCallContext*>(sys_call_ctx);
        const int   app_ID          = static_cast<int>(ID);
        const int   app_exit_code   = app_syscall_ctx->app_module->join(app_ID);
        return app_exit_code == INT_MAX ? Ember::Status::UNKNOWN_ID : app_exit_code;
    }

    Ember::StatusCode
    app_current_directory(void* sys_call_ctx, const U64 wd_out, const U64 wd_out_size) {
        const auto* app_syscall_ctx = static_cast<AppSystemCallContext*>(sys_call_ctx);
        auto*       u_wd_out        = reinterpret_cast<char*>(wd_out);
        const auto  b_size          = static_cast<size_t>(wd_out_size);

        const auto wd =
            app_syscall_ctx->app_module->get_active_app()->working_directory.to_string();
        if (b_size < wd.size() + 1) return Ember::Status::BAD_ARG;

        if (!app_syscall_ctx->k_guard->copy_byte_buffer_kernel_to_user((void*) wd.to_cstr(),
                                                                       u_wd_out,
                                                                       wd.size() + 1))
            return Ember::Status::BAD_ARG;
        return Ember::Status::OKAY;
    }

    Ember::StatusCode app_change_directory(void* sys_call_ctx, const U64 wd) {
        const auto* app_syscall_ctx = static_cast<AppSystemCallContext*>(sys_call_ctx);
        auto*       u_str           = reinterpret_cast<const char*>(wd);
        char        k_str[Ember::STRING_SIZE_LIMIT] = {};
        if (!app_syscall_ctx->k_guard->copy_string_user_to_kernel(u_str, -1, k_str)) {
            return Ember::Status::BAD_ARG;
        }

        App::Info* app = app_syscall_ctx->app_module->get_active_app();
        Path       path(k_str);
        if (!path.is_absolute()) path = path.resolve(app->working_directory);
        if (path == app->working_directory)
            // The resolved path is the current working directory (e.g. path==".") -> No need to do
            // anything
            return Ember::Status::OKAY;
        if (!app_syscall_ctx->vfs_module->is_valid_file_path(path)) return Ember::Status::BAD_ARG;

        SharedPointer<VFS::Node> node;
        switch (VFS::IOStatus st =
                    app_syscall_ctx->vfs_module->open(path, Ember::IOMode::READ, node)) {
            case VFS::IOStatus::OPENED:
                if (node->has_attribute(Ember::NodeAttribute::FILE)) {
                    // Not a directory
                    node->close();
                    return Ember::Status::NODE_IS_FILE;
                }
                app->working_directory = path;
                node->close();
                return Ember::Status::OKAY; // Working directory is changed.
            case VFS::IOStatus::BAD_PATH:
            case VFS::IOStatus::NOT_FOUND:
                return Ember::Status::NODE_NOT_FOUND; // Node does not exist or the path is bad.
            default: return Ember::Status::IO_ERROR;  // DEV_UNKNOWN, DEV_ERROR, OUT_OF_HANDLES
        }
    }
} // namespace Rune::SystemCall
