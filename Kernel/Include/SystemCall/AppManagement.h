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

#ifndef RUNEOS_APPMANAGEMENT_H
#define RUNEOS_APPMANAGEMENT_H


#include <Ember/Definitions.h>

#include <App/AppSubsystem.h>

#include <Device/DeviceSubsystem.h>

#include <CPU/CPUSubsystem.h>

#include <VirtualFileSystem/VFSSubsystem.h>

#include <SystemCall/KernelGuardian.h>


namespace Rune::SystemCall {
    /**
     * @brief The context for all app management system calls.
     */
    struct AppManagementContext {
        KernelGuardian*    k_guard       = nullptr;
        App::Subsystem*    app_subsys    = nullptr;
        Device::Subsystem* device_subsys = nullptr;
        CPU::Subsystem*    cpu_subsys    = nullptr;
        VFS::Subsystem*    vfs_subsys    = nullptr;
    };


    /**
     * @brief Maximum number of CLI arguments (including the null terminator) an app can receive.
     */
    constexpr U8 APP_ARG_LIMIT = 32;


    /**
     * @brief Wait until a key is available in the virtual keyboard buffer and write the keycode to given buffer.
     * @param sys_call_Ctx A pointer to the app management context.
     * @param key_code_out A pointer to a keycode buffer, U16*.
     * @return 0:        A key code is returned in the given key buffer.<br>
     *          BAD_ARG: The key code buffer is null or intersects kernel memory.
     */
    S64 read_std_in(void* sys_call_Ctx, U64 key_code_out);


    /**
     * @brief Write the msg to the stdout stream of the running app.
     * @param sys_call_Ctx A pointer to the app management context.
     * @param msg          A pointer to a c string.
     * @return >=0:      The number of written characters.<br>
     *          BAD_ARG: The msg is null, intersects kernel memory or exceeds the string size limit.
     */
    S64 write_std_out(void* sys_call_Ctx, U64 msg);


    /**
     * @brief Write the msg to the stderr stream of the running app.
     *
     * If the stream supports colors the msg will be written in red.
     *
     * @param sys_call_Ctx A pointer to the app management context.
     * @param msg          A pointer to a c string.
     * @return >=0:      The number of written characters.<br>
     *          BAD_ARG: The msg is null, intersects kernel memory or exceeds the string size limit.
     */
    S64 write_std_err(void* sys_call_Ctx, U64 msg);


    /**
     * @brief Start an application with the given arguments.
     *
     * <p>
     *  The standard stream targets define the source of the stdin and destinations of the stdout/stderr streams.
     *  One of the following targets can be chosen:
     *  <ul>
     *      <li>void - Connect the standard stream to the void stream.</li>
     *      <li>inherit - Connect the stdin/stdout/stderr to the std streams of the calling app.</li>
     *      <li>file:"path"- Only stdout/stderr: Redirect to the specified file, if it does not exist it will be
     *              created.</li>
     *      <li>pipe:"handle": Connect the standard stream to the requested pipe.</li>
     *  </ul>
     * </p>
     *
     * @param sys_call_ctx  A pointer to the app management context.
     * @param app_path      A path to the executable, either absolute or relative to the working directory of the calling
     *                       app.
     * @param argv          Pointer to the command line arguments, a null terminated array of c strings.
     * @param working_dir   The working directory of the app that will be started, either absolute or relative to the
     *                       working directory of the calling app.
     * @param stdin_target  stdin stream target.
     * @param stdout_target stdout stream target.
     * @param stderr_target stderr stream target.
     * @return >=0:                 The app has been started, the returned value is the assigned ID.<br>
     *          BAD_ARG:            One of the arguments is null, intersects kernel memory, exceeds the string size
     *                              limit or the number of arguments in argv exceeds the argument count limit.<br>
     *          NODE_NOT_FOUND: The path to the application or the working directory does not exist.<br>
     *          FAULT:              The app could not be started.
     */
    S64 app_start(
        void* sys_call_ctx,
        U64   app_path,
        U64   argv,
        U64   working_dir,
        U64   stdin_target,
        U64   stdout_target,
        U64   stderr_target
    );


    /**
     * @brief Exit the currently running app with the given exit code.
     * @param sys_call_Ctx A pointer to the app management context.
     * @param exit_code    Exit code of the apps main thread. >=0 indicates successful app exit and <0 indicates an
     *                     error.
     * @return 0: For the sake of the ABI but this function will never exit anyway and there is no app that can use
     *              the return value.
     */
    S64 app_exit(void* sys_call_Ctx, U64 exit_code);


    /**
     * @brief Wait until the application with the requested ID has exited.
     * @param sys_call_Ctx A pointer to the app management context.
     * @param ID           ID of an application.
     * @return BAD_ID: No application with the requested ID was running.<br>
     *          Else: The exit code of the application.
     */
    S64 app_join(void* sys_call_Ctx, U64 ID);


    /**
     * @brief Get the current working directory of the active app and copy it to the char buffer.
     * @param sys_call_Ctx A pointer to the app management context.
     * @param wd_out       Pointer to a char buffer.
     * @param wd_out_size  Size of the buffer.
     * @return 0: The working directory has been copied to the given buffer.<br>
     *          BAD_ARG: The char buffer is null, intersects kernel memory or is too small to fit the working directory.
     */
    S64 app_get_working_directory(void* sys_call_Ctx, U64 wd_out, U64 wd_out_size);


    /**
     * @brief Change the working directory of the active app to the requested working directory.
     * @param sys_call_Ctx A pointer to the app management context.
     * @param wd           A pointer to a c string containing the new working directory.
     * @return 0:                  The working directory of the active app has been changed.<br>
     *          BAD_ARG:           The working directory is null, intersects kernel memory or contains illegal
     *                             characters.<br>
     *          NODE_NOT_FOUND:    The working directory does not exist.<br>
     *          NODE_IS_DIRECTORY: The node is not a directory.<br>
     *          IO:                An IO error happened.
     */
    S64 app_change_working_directory(void* sys_call_Ctx, U64 wd);
}

#endif //RUNEOS_APPMANAGEMENT_H
