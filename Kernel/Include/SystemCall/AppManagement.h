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


#include <Hammer/Definitions.h>

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
        KernelGuardian   * k_guard       = nullptr;
        App::Subsystem   * app_subsys    = nullptr;
        Device::Subsystem* device_subsys = nullptr;
        CPU::Subsystem   * cpu_subsys    = nullptr;
        VFS::Subsystem   * vfs_subsys    = nullptr;
    };


    /**
     * @brief Maximum number of CLI arguments (including the null terminator) an app can receive.
     */
    constexpr U8 APP_ARG_LIMIT = 32;


    /**
     * @brief Wait until a key is available in the virtual keyboard buffer and write the keycode to given buffer.
     * @param sys_call_Ctx A pointer to the app management context.
     * @param arg1         A pointer to a keycode buffer, U16*.
     * @return 0: A key code is returned in the given key buffer.
     *          -1: The kernel guardian did not copy keycode from kernel memory to the given buffer.
     */
    S64 read_std_in(void* sys_call_Ctx, U64 arg1);


    /**
     * @brief Write a c string to the display in the white color of the currently active app.
     * @param sys_call_Ctx A pointer to the app management context.
     * @param arg1         A pointer to a c string.
     * @return >= 0: The number of written characters.
     *          -1: The kernel guardian did not copy the content of the c string over to kernel land.
     */
    S64 write_std_out(void* sys_call_Ctx, U64 arg1);


    /**
     * @brief Write a c string to the display in red color of the currently active app.
     * @param sys_call_Ctx A pointer to the app management context.
     * @param arg1         A pointer to a c string.
     * @return >= 0: The number of written characters.
     *          -1: The kernel guardian did not copy the content of the c string over to kernel land.
     */
    S64 write_std_err(void* sys_call_Ctx, U64 arg1);


    /**
     * @brief Start an application with the given arguments and working directory.
     *
     * See App::Subsystem::Start for a detailed description on starting new applications.
     *
     * @param sys_call_ctx  A pointer to the app management context.
     * @param app_path      A path to the executable, either absolute or relative to the working directory of the calling
     *                       app.
     * @param argv          Pointer to the command line arguments, a null terminated array of c strings.
     * @param working_dir   The working directory of the app that will be started, either absolute or relative to the
     *                       working directory of the calling app.
     * @param stdin_target  Stream target for stdin.
     * @param stdout_target Stream target for stdout.
     * @param stderr_target Stream target for stderr.
     * @return >= 0: The app has been started, the returned value is the assigned ID.
     *          -1: The kernel guardian found one or more bad arguments.
     *          -2: One or more command line arguments exceed the maximum length of KernelGuardian::UserStringLimit
     *              characters.
     *          -3: The number of app arguments exceeds the AppArgLimit.
     *          -4: The path to the application does not exist.
     *          -5: The working directory does not exist.
     *          -6: The app could not be started.
     */
    S64 app_start(
            void* sys_call_ctx,
            U64 app_path,
            U64 argv,
            U64 working_dir,
            U64 stdin_target,
            U64 stdout_target,
            U64 stderr_target
    );

    // "std"   -> display or keyboard
    // "afile" -> Open file stream
    // "3"     -> Configure pipe 3

    /**
     * @brief Exit the currently running app with the given exit code of its main thread.
     * @param sys_call_Ctx A pointer to the app management context.
     * @param arg1         Exit code of the main thread. See "App::AppSubsystem::ExitRunningApp" for conventions.
     * @return 0: For the sake of the ABI but this function will never exit anyway and there is no app that can use
     *              the return value.
     */
    S64 app_exit(void* sys_call_Ctx, U64 arg1);


    /**
     * @brief Wait until the application with the requested handle has exited.
     * @param sys_call_Ctx A pointer to the app management context.
     * @param arg1         Handle of an application.
     * @return INT_MAX: No application with the requested handle was running,
     *          Else: The exit code of the application.
     */
    S64 app_join(void* sys_call_Ctx, U64 arg1);


    /**
     * @brief Get the current working directory of the active app and copy it to the char buffer.
     * @param sys_call_Ctx A pointer to the app management context.
     * @param arg1         Pointer to a char buffer.
     * @param arg2         Size of the buffer.
     * @return 0: The working directory has been copied to the given buffer.
     *          -1: The char buffer is too small for the working directory.
     *          -2: The kernel guardian found one or more bad arguments.
     */
    S64 app_get_working_directory(void* sys_call_Ctx, U64 arg1, U64 arg2);


    /**
     * @brief Change the working directory of the active app to the requested working directory.
     * @param sys_call_Ctx A pointer to the app management context.
     * @param arg1         A pointer to a c string containing the new working directory.
     * @return 0: The working directory of the active app has been changed.
     *          -1: The kernel guardian found one or more bad arguments.
     *          -2: The path contains illegal characters.
     *          -3: The path does not exist.
     *          -4: The node is not a directory.
     *          -5: An IO error happened.
     */
    S64 app_change_working_directory(void* sys_call_Ctx, U64 arg1);
}

#endif //RUNEOS_APPMANAGEMENT_H
