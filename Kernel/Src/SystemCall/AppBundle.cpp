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

#include <Ember/VirtualKey.h>
#include <Ember/StatusCode.h>


namespace Rune::SystemCall {
    S64 read_std_in(void* sys_call_ctx, U64 arg1) {
        auto* app_mng_ctx = (AppManagementContext*)sys_call_ctx;
        auto* timer       = app_mng_ctx->cpu_subsys->get_system_timer();

        Ember::VirtualKey key(app_mng_ctx->app_subsys->get_active_app()->std_in->read());
        while (key.is_none()) {
            timer->sleep_milli(2); // 1ms is too fast, dunno why but nothing happens
            key = Ember::VirtualKey(app_mng_ctx->app_subsys->get_active_app()->std_in->read());
        }
        U16   key_code        = key.get_key_code();
        auto* key_code_buffer = (U16*)arg1;
        return app_mng_ctx->k_guard->copy_byte_buffer_kernel_to_user((void*)&key_code, (void*)key_code_buffer, 2)
                   ? 0
                   : Ember::StatusCode::BAD_ARG;
    }


    S64 write_std_out(void* sys_call_ctx, U64 arg1) {
        auto* app_mng_ctx = (AppManagementContext*)sys_call_ctx;

        const char* k_buf_msg = new char[Ember::STRING_SIZE_LIMIT + 1];
        if (!app_mng_ctx->k_guard->copy_string_user_to_kernel((const char*)arg1, -1, k_buf_msg))
            return Ember::StatusCode::BAD_ARG;
        String k_msg(k_buf_msg);
        S64    byte_out = (S64)app_mng_ctx->app_subsys->get_active_app()->std_out->write((U8*)k_buf_msg, k_msg.size());
        delete[] k_buf_msg;
        return byte_out;
    }


    S64 write_std_err(void* sys_call_ctx, U64 arg1) {
        auto* app_mng_ctx = (AppManagementContext*)sys_call_ctx;

        const char* k_buf_msg = new char[Ember::STRING_SIZE_LIMIT + 1];
        if (!app_mng_ctx->k_guard->copy_string_user_to_kernel((const char*)arg1, -1, k_buf_msg))
            return Ember::StatusCode::BAD_ARG;
        String                          k_msg(k_buf_msg);
        SharedPointer<LibK::TextStream> std_err = app_mng_ctx->app_subsys->get_active_app()->std_err;

        std_err->set_foreground_color(LibK::Pixie::VSCODE_RED);
        S64 byte_out = (S64)app_mng_ctx->app_subsys->get_active_app()->std_err->write((U8*)k_buf_msg, k_msg.size());
        std_err->reset_style();
        delete[] k_buf_msg;
        return byte_out;
    }


    S64 app_start(
        void*     sys_call_ctx,
        const U64 app_path,
        const U64 argv,
        const U64 working_dir,
        const U64 stdin_target,
        const U64 stdout_target,
        const U64 stderr_target
    ) {
        const auto* app_mng_ctx                         = static_cast<AppManagementContext*>(sys_call_ctx);
        auto*       u_app_path                          = reinterpret_cast<const char*>(app_path);
        auto*       u_app_argv                          = reinterpret_cast<const char**>(argv);
        auto*       u_app_wd                            = reinterpret_cast<const char*>(working_dir);
        auto*       u_stdin_target                      = reinterpret_cast<const char*>(stdin_target);
        auto*       u_stdout_target                     = reinterpret_cast<const char*>(stdout_target);
        auto*       u_stderr_target                     = reinterpret_cast<const char*>(stderr_target);
        char        k_str_buf[Ember::STRING_SIZE_LIMIT] = { };

        if (!app_mng_ctx->k_guard->copy_string_user_to_kernel(u_app_path, -1, k_str_buf))
            return Ember::StatusCode::BAD_ARG;
        Path k_app_path(k_str_buf);
        memset(k_str_buf, '\0', Ember::STRING_SIZE_LIMIT);

        if (!app_mng_ctx->k_guard->copy_string_user_to_kernel(u_app_wd, -1, k_str_buf))
            return Ember::StatusCode::BAD_ARG;
        Path k_app_wd(k_str_buf);
        memset(k_str_buf, '\0', Ember::STRING_SIZE_LIMIT);

        if (!app_mng_ctx->k_guard->copy_string_user_to_kernel(u_stdin_target, -1, k_str_buf))
            return Ember::StatusCode::BAD_ARG;
        const String k_stdin_target(k_str_buf);
        memset(k_str_buf, '\0', Ember::STRING_SIZE_LIMIT);

        if (!app_mng_ctx->k_guard->copy_string_user_to_kernel(u_stdout_target, -1, k_str_buf))
            return Ember::StatusCode::BAD_ARG;
        const String k_stdout_target(k_str_buf);
        memset(k_str_buf, '\0', Ember::STRING_SIZE_LIMIT);

        if (!app_mng_ctx->k_guard->copy_string_user_to_kernel(u_stderr_target, -1, k_str_buf))
            return Ember::StatusCode::BAD_ARG;
        const String k_stderr_target(k_str_buf);
        memset(k_str_buf, '\0', Ember::STRING_SIZE_LIMIT);

        // Copy uAppArgv to kernel memory
        char* k_app_argv[Ember::ARG_COUNT_LIMIT];
        memset(k_app_argv, '\0', Ember::ARG_COUNT_LIMIT);

        // We will just copy over the maximum amount of arguments uAppArgv can have
        // This will likely be way more than we need, but we really don't want to access user memory before it is
        // copied over to kernel memory
        if (!app_mng_ctx->k_guard->copy_byte_buffer_user_to_kernel(u_app_argv, sizeof(k_app_argv), k_app_argv))
            return Ember::StatusCode::BAD_ARG;

        int    argc     = 0;
        char** tmp_argv = k_app_argv;
        while (*tmp_argv) {
            ++tmp_argv;
            ++argc;
        }

        // argv is null terminated and the first argument is the app path -> The caller can pass
        // (Ember::ARG_COUNT_LIMIT - 2) arguments
        if (argc >= Ember::ARG_COUNT_LIMIT - 2)
            return Ember::StatusCode::BAD_ARG; // Too many arguments

        // The app args are still in user memory (we only copied the pointers to them over to kernel)
        // The array will hold the copies of the actual strings, which can hold the maximum number of allowed arguments
        // that have the maximum allowed size of a user memory string.
        char   k_app_args[Ember::ARG_COUNT_LIMIT * Ember::STRING_SIZE_LIMIT];
        size_t k_app_args_offset = 0;
        U8     i                 = 0;

        // Move all arguments up by one
        memmove(&k_app_argv[1], &k_app_argv[0], sizeof(char**) * (argc + 1));
        // Make k_app_argv[0] the app path
        memcpy(
            &k_app_args[k_app_args_offset],
            (void*)k_app_path.to_string().to_cstr(),
            k_app_path.to_string().size() + 1
        );
        k_app_argv[i] = &k_app_args[k_app_args_offset];
        k_app_args_offset += k_app_path.to_string().size() + 1;
        i++;
        while (k_app_argv[i] && i < Ember::ARG_COUNT_LIMIT) {
            if (!app_mng_ctx->k_guard->copy_string_user_to_kernel(k_app_argv[i], -1, k_str_buf))
                return Ember::StatusCode::BAD_ARG;
            String arg_tmp(k_str_buf);
            memcpy(&k_app_args[k_app_args_offset], (void*)arg_tmp.to_cstr(), arg_tmp.size() + 1);

            // The pointer to the c string still points to the user land string -> Update it to the kernel land string
            memset(k_str_buf, '\0', Ember::STRING_SIZE_LIMIT);
            k_app_argv[i] = &k_app_args[k_app_args_offset];
            k_app_args_offset += arg_tmp.size() + 1;
            i++;
        }

        VFS::NodeInfo dummy;
        if (!k_app_path.is_absolute())
            k_app_path = k_app_path.resolve(app_mng_ctx->app_subsys->get_active_app()->working_directory);
        if (app_mng_ctx->vfs_subsys->get_node_info(k_app_path, dummy) != VFS::IOStatus::FOUND)
            // Path to the app executable does not exist
            return Ember::StatusCode::NODE_NOT_FOUND;

        if (!k_app_wd.is_absolute())
            k_app_wd = k_app_wd.resolve(app_mng_ctx->app_subsys->get_active_app()->working_directory);
        if (app_mng_ctx->vfs_subsys->get_node_info(k_app_wd, dummy) != VFS::IOStatus::FOUND)
            // Path to the app working directory does not exist
            return Ember::StatusCode::NODE_NOT_FOUND;

        auto [load_result, ID] = app_mng_ctx->app_subsys
                                            ->start_new_app(
                                                k_app_path,
                                                k_app_argv,
                                                k_app_wd,
                                                k_stdin_target,
                                                k_stdout_target,
                                                k_stderr_target
                                            );
        if (load_result != App::LoadStatus::RUNNING)
            return Ember::StatusCode::FAULT;

        return ID;
    }


    S64 app_exit(void* sys_call_ctx, const U64 exit_code) {
        const auto* app_mng_ctx = static_cast<AppManagementContext*>(sys_call_ctx);
        const int   k_exit_code = static_cast<int>(exit_code);
        app_mng_ctx->app_subsys->exit_running_app(k_exit_code);
        return Ember::StatusCode::OKAY;
    }


    S64 app_join(void* sys_call_ctx, U64 ID) {
        const auto* app_mng_ctx   = static_cast<AppManagementContext*>(sys_call_ctx);
        const int   app_ID        = static_cast<int>(ID);
        const int   app_exit_code = app_mng_ctx->app_subsys->join(app_ID);
        return app_exit_code == INT_MAX ? Ember::StatusCode::UNKNOWN_ID : app_exit_code;
    }


    S64 app_get_working_directory(void* sys_call_ctx, const U64 wd_out, const U64 wd_out_size) {
        const auto* app_mng_ctx = static_cast<AppManagementContext*>(sys_call_ctx);
        auto*       u_wd_out    = reinterpret_cast<char*>(wd_out);
        const auto  b_size      = static_cast<size_t>(wd_out_size);

        const auto wd = app_mng_ctx->app_subsys->get_active_app()->working_directory.to_string();
        if (b_size < wd.size() + 1) return Ember::StatusCode::BAD_ARG;

        if (!app_mng_ctx->k_guard->copy_byte_buffer_kernel_to_user((void*)wd.to_cstr(), u_wd_out, wd.size() + 1))
            return Ember::StatusCode::BAD_ARG;
        return Ember::StatusCode::OKAY;
    }


    S64 app_change_working_directory(void* sys_call_ctx, const U64 wd) {
        const auto* app_mng_ctx                     = static_cast<AppManagementContext*>(sys_call_ctx);
        auto*       u_str                           = reinterpret_cast<const char*>(wd);
        char        k_str[Ember::STRING_SIZE_LIMIT] = { };
        if (!app_mng_ctx->k_guard->copy_string_user_to_kernel(u_str, -1, k_str)) {
            return Ember::StatusCode::BAD_ARG;
        }

        App::Info* app = app_mng_ctx->app_subsys->get_active_app();
        Path       path(k_str);
        if (!path.is_absolute())
            path = path.resolve(app->working_directory);
        if (path == app->working_directory)
            // The resolved path is the current working directory (e.g. path==".") -> No need to do anything
            return Ember::StatusCode::OKAY;
        if (!app_mng_ctx->vfs_subsys->is_valid_file_path(path))
            return Ember::StatusCode::BAD_ARG;

        SharedPointer<VFS::Node> node;
        switch (VFS::IOStatus st = app_mng_ctx->vfs_subsys->open(path, Ember::IOMode::READ, node)) {
            case VFS::IOStatus::OPENED:
                if (node->has_attribute(Ember::NodeAttribute::FILE)) {
                    // Not a directory
                    node->close();
                    return Ember::StatusCode::NODE_IS_FILE;
                }
                app->working_directory = path;
                node->close();
                return Ember::StatusCode::OKAY; // Working directory is changed.
            case VFS::IOStatus::BAD_PATH:
            case VFS::IOStatus::NOT_FOUND:
                return Ember::StatusCode::NODE_NOT_FOUND; // Node does not exist or the path is bad.
            default:
                return Ember::StatusCode::IO_ERROR; // DEV_UNKNOWN, DEV_ERROR, OUT_OF_HANDLES
        }
    }
}
