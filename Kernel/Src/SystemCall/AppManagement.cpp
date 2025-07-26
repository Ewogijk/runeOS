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

#include <SystemCall/AppManagement.h>

#include <Ember/VirtualKey.h>

namespace Rune::SystemCall {
    S64 read_std_in(void* sys_call_ctx, U64 arg1) {
        auto* app_mng_ctx = (AppManagementContext*) sys_call_ctx;
        auto* timer       = app_mng_ctx->cpu_subsys->get_system_timer();

        Ember::VirtualKey key(app_mng_ctx->app_subsys->get_active_app()->std_in->read());
        while (key.is_none()) {
            timer->sleep_milli(2); // 1ms is too fast, dunno why but nothing happens
            key = Ember::VirtualKey(app_mng_ctx->app_subsys->get_active_app()->std_in->read());
        }
        U16 key_code = key.get_key_code();
        auto* key_code_buffer = (U16*) arg1;
        return app_mng_ctx->k_guard->copy_byte_buffer_kernel_to_user((void*) &key_code, (void*) key_code_buffer, 2)
               ? 0
               : -1;
    }


    S64 write_std_out(void* sys_call_ctx, U64 arg1) {
        auto* app_mng_ctx = (AppManagementContext*) sys_call_ctx;

        const char* k_buf_msg = new char[KernelGuardian::USER_STRING_LIMIT + 1];
        if (!app_mng_ctx->k_guard->copy_string_user_to_kernel((const char*) arg1, -1, k_buf_msg))
            return -1;
        String k_msg(k_buf_msg);
        S64 byte_out = (S64) app_mng_ctx->app_subsys->get_active_app()->std_out->write((U8*) k_buf_msg, k_msg.size());
        delete[] k_buf_msg;
        return byte_out;
    }


    S64 write_std_err(void* sys_call_ctx, U64 arg1) {
        auto* app_mng_ctx = (AppManagementContext*) sys_call_ctx;

        const char* k_buf_msg = new char[KernelGuardian::USER_STRING_LIMIT + 1];
        if (!app_mng_ctx->k_guard->copy_string_user_to_kernel((const char*) arg1, -1, k_buf_msg))
            return -1;
        String k_msg(k_buf_msg);
        SharedPointer<LibK::TextStream> std_err = app_mng_ctx->app_subsys->get_active_app()->std_err;

        std_err->set_foreground_color(LibK::Pixie::VSCODE_RED);
        S64 byte_out = (S64) app_mng_ctx->app_subsys->get_active_app()->std_err->write((U8*) k_buf_msg, k_msg.size());
        std_err->reset_style();
        delete[] k_buf_msg;
        return byte_out;
    }


    S64 app_start(
            void* sys_call_ctx,
            U64 app_path,
            U64 argv,
            U64 working_dir,
            U64 stdin_target,
            U64 stdout_target,
            U64 stderr_target
    ) {
        auto* app_mng_ctx     = (AppManagementContext*) sys_call_ctx;
        auto* u_app_path      = (const char*) app_path;
        auto* u_app_argv      = (const char**) argv;
        auto* u_app_wd        = (const char*) working_dir;
        auto* u_stdin_target  = (const char*) stdin_target;
        auto* u_stdout_target = (const char*) stdout_target;
        auto* u_stderr_target = (const char*) stderr_target;
        char k_str_buf[KernelGuardian::USER_STRING_LIMIT];
        memset(k_str_buf, 0, KernelGuardian::USER_STRING_LIMIT);

        if (!app_mng_ctx->k_guard->copy_string_user_to_kernel(u_app_path, -1, k_str_buf))
            return -1;
        Path k_app_path(k_str_buf);
        memset(k_str_buf, '\0', KernelGuardian::USER_STRING_LIMIT);

        if (!app_mng_ctx->k_guard->copy_string_user_to_kernel(u_app_wd, -1, k_str_buf))
            return -1;
        Path k_app_wd(k_str_buf);
        memset(k_str_buf, '\0', KernelGuardian::USER_STRING_LIMIT);

        if (!app_mng_ctx->k_guard->copy_string_user_to_kernel(u_stdin_target, -1, k_str_buf))
            return -1;
        String k_stdin_target(k_str_buf);
        memset(k_str_buf, '\0', KernelGuardian::USER_STRING_LIMIT);

        if (!app_mng_ctx->k_guard->copy_string_user_to_kernel(u_stdout_target, -1, k_str_buf))
            return -1;
        String k_stdout_target(k_str_buf);
        memset(k_str_buf, '\0', KernelGuardian::USER_STRING_LIMIT);

        if (!app_mng_ctx->k_guard->copy_string_user_to_kernel(u_stderr_target, -1, k_str_buf))
            return -1;
        String k_stderr_target(k_str_buf);
        memset(k_str_buf, '\0', KernelGuardian::USER_STRING_LIMIT);


        // Copy uAppArgv to kernel land memory
        char* k_app_argv[APP_ARG_LIMIT];
        // We will just copy over the maximum amount of arguments uAppArgv can have
        // This will likely be way more than we need, but we really don't want to access user land memory before it is
        // copied over to kernel land memory
        if (!app_mng_ctx->k_guard->copy_byte_buffer_user_to_kernel(u_app_argv, sizeof(k_app_argv), k_app_argv))
            return -1;

        // The app args are still in user land memory (we only copied the pointers to them over to kernel land)
        // The array will hold the copies of the actual strings, which can hold the maximum number of allowed arguments
        // that have the maximum allowed size of a user land string.
        char   k_app_args[APP_ARG_LIMIT * KernelGuardian::USER_STRING_LIMIT];
        size_t k_app_args_offset = 0;
        U8     i                 = 0;
        while (k_app_argv[i] && i < APP_ARG_LIMIT) {
            if (!app_mng_ctx->k_guard->copy_string_user_to_kernel(k_app_argv[i], -1, k_str_buf))
                return -2;
            String arg_tmp(k_str_buf);
            memcpy(&k_app_args[k_app_args_offset], (void*) arg_tmp.to_cstr(), arg_tmp.size() + 1);

            // The pointer to the c string still points to the user land string -> Update it to the kernel land string
            memset(k_str_buf, '\0', KernelGuardian::USER_STRING_LIMIT);
            k_app_argv[i] = &k_app_args[k_app_args_offset];
            k_app_args_offset += arg_tmp.size() + 1;
            i++;
        }

        if (i >= APP_ARG_LIMIT)
            // uAppArgv is not null terminated!
            return -3;

        VFS::NodeInfo dummy;
        if (!k_app_path.is_absolute())
            k_app_path = k_app_path.resolve(app_mng_ctx->app_subsys->get_active_app()->working_directory);
        if (app_mng_ctx->vfs_subsys->get_node_info(k_app_path, dummy) != VFS::IOStatus::FOUND)
            // Path to the app executable does not exist
            return -4;

        if (!k_app_wd.is_absolute())
            k_app_wd = k_app_wd.resolve(app_mng_ctx->app_subsys->get_active_app()->working_directory);
        if (app_mng_ctx->vfs_subsys->get_node_info(k_app_wd, dummy) != VFS::IOStatus::FOUND)
            // Path to the app working directory does not exist
            return -5;

        App::StartStatus st = app_mng_ctx->app_subsys
                                         ->start_new_app(
                                                 k_app_path,
                                                 k_app_argv,
                                                 k_app_wd,
                                                 k_stdin_target,
                                                 k_stdout_target,
                                                 k_stderr_target
                                         );
        if (st.load_result != App::LoadStatus::RUNNING)
            return -6;

        return st.handle;
    }


    S64 app_exit(void* sys_call_ctx, U64 arg1) {
        auto* app_mng_ctx = (AppManagementContext*) sys_call_ctx;
        int exit_code = (int) arg1;
        app_mng_ctx->app_subsys->exit_running_app(exit_code);
        return 0;
    }


    S64 app_join(void* sys_call_ctx, U64 arg1) {
        auto* app_mng_ctx = (AppManagementContext*) sys_call_ctx;
        int app_handle = (int) arg1;
        return app_mng_ctx->app_subsys->join(app_handle);
    }


    S64 app_get_working_directory(void* sys_call_ctx, U64 arg1, U64 arg2) {
        auto* app_mng_ctx = (AppManagementContext*) sys_call_ctx;
        auto* wd_out      = (char*) arg1;
        auto b_size = (size_t) arg2;

        auto wd = app_mng_ctx->app_subsys->get_active_app()->working_directory.to_string();
        if (b_size < wd.size() + 1)
            return -1;

        if (!app_mng_ctx->k_guard->copy_byte_buffer_kernel_to_user((void*) wd.to_cstr(), wd_out, wd.size() + 1))
            return -2;

        return 0;
    }


    S64 app_change_working_directory(void* sys_call_ctx, U64 arg1) {
        auto* app_mng_ctx = (AppManagementContext*) sys_call_ctx;
        auto* u_str       = (const char*) arg1;
        char k_str[KernelGuardian::USER_STRING_LIMIT];
        memset(k_str, 0, KernelGuardian::USER_STRING_LIMIT);
        if (!app_mng_ctx->k_guard->copy_string_user_to_kernel(u_str, -1, k_str)) {
            return -1;
        }

        App::Info* app = app_mng_ctx->app_subsys->get_active_app();
        Path path(k_str);
        if (!path.is_absolute())
            path = path.resolve(app->working_directory);
        if (path == app->working_directory)
            // The resolved path is the current working directory (e.g. path==".") -> No need to do anything
            return 0;
        if (!app_mng_ctx->vfs_subsys->is_valid_file_path(path))
            return -2;

        SharedPointer<VFS::Node> node;
        VFS::IOStatus            st = app_mng_ctx->vfs_subsys->open(path, Ember::IOMode::READ, node);
        switch (st) {
            case VFS::IOStatus::OPENED:
                if (node->has_attribute(Ember::NodeAttribute::FILE)) {
                    // Not a directory
                    node->close();
                    return -4;
                }
                app->working_directory = path;
                node->close();
                return 0;   // Working directory is changed.
            case VFS::IOStatus::BAD_PATH:
            case VFS::IOStatus::NOT_FOUND:
                return -3;  // Node does not exist or the path is bad.
            default:
                return -5;  // DEV_UNKNOWN, DEV_ERROR, OUT_OF_HANDLES
        }
    }

}
