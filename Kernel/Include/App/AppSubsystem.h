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

#ifndef RUNEOS_APPSUBSYSTEM_H
#define RUNEOS_APPSUBSYSTEM_H


#include <Hammer/Path.h>

#include <LibK/Subsystem.h>

#include <App/App.h>

#include <CPU/CPUSubsystem.h>

#include <VirtualFileSystem/VFSSubsystem.h>

#include <Memory/MemorySubsystem.h>

#include <Device/DeviceSubsystem.h>


namespace Rune::App {

    /**
     * @brief The standard streams.
     * <ul>
     *  <li>IN: stdin</li>
     *  <li>OUT: stdout</li>
     *  <li>ERR: stderr</li>
     * </ul>
     */
#define STD_STREAMS(X)              \
             X(StdStream, IN, 0x1)  \
             X(StdStream, OUT, 0x2) \
             X(StdStream, ERR, 0x3) \



    DECLARE_ENUM(StdStream, STD_STREAMS, 0x0)  // NOLINT


    /**
     * The App subsystem is responsible for starting and managing Apps.
     *
     * <p>
     *  It maintains a currently running app determine by context switches and the App registry, a list of all currently
     *  running apps that will be updated whenever an App is started or the last thread of an app is terminated.
     * </p>
     */
    class Subsystem : public LibK::Subsystem {
        Memory::Subsystem* _memory_subsys;
        CPU::Subsystem   * _cpu_subsys;
        VFS::Subsystem   * _vfs_subsys;
        Device::Subsystem* _dev_subsys;
        LibK::FrameBuffer _frame_buffer;

        HashMap<U16, SharedPointer<Info>> _app_table;
        LibK::TableFormatter<Info>        _app_table_fmt;
        LibK::HandleCounter<U16>          _app_handle_counter;

        SharedPointer<Info> _active_app;


        /**
         * @brief Set the ID and working directory in the entry and schedule it's main thread for execution.
         * @return The assigned ID of the app.
         */
        int schedule_for_start(
            const SharedPointer<Info>& app,
            const CPU::Stack&          user_stack,
            CPU::StartInfo*             start_info,
            const Path&                working_directory
        );


        /**
         * @brief Setup a standard stream of the application.
         * @param std_stream stdin, stdout or stderr.
         * @param target     Stream target.
         * @return The standard stream or nullptr if setup failed.
         */
        SharedPointer<LibK::TextStream> setup_std_stream(
                const SharedPointer<Info>& app,
                StdStream std_stream,
                const String& target
        );

    public:
        Subsystem();


        ~Subsystem() override = default;


        /**
         *
         * @return Unique Kernel SubSystem name.
         */
        [[nodiscard]] String get_name() const override;


        bool start(const LibK::BootLoaderInfo& evt_ctx, const LibK::SubsystemRegistry& k_subsys_reg) override;


        void set_logger(SharedPointer<LibK::Logger> logger) override;


        /**
         *
         * @return A list with all running apps.
         */
        [[nodiscard]] LinkedList<Info*> get_app_table() const;


        /**
         *
         * @return The app that is currently executing code.
         */
        [[nodiscard]] Info* get_active_app() const;


        /**
         * @brief Dump the app table to the stream.
         * @param stream
         */
        void dump_app_table(const SharedPointer<LibK::TextStream>& stream) const;


        /**
         * @brief Load the OS and then schedule it's main thread.
         * @param os_exec           Path to the OS executable.
         * @param working_directory Working directory of the OS.
         * @return Final status of the OS start, the assigned ID will always be zero as the OS is always the first
         *          loaded app.
         */
        LoadStatus start_os(const Path& os_exec, const Path& working_directory);


        /**
         * <p>
         *  The start steps are:
         *  <ol>
         *   <li>ELF loading: Load the ELF executable into memory.</li>
         *   <li>App Argument loading: Copy the app arguments to the virtual address space of the app.</li>
         *   <li>App Start Allocation: Create/schedule a thread that will execute the "Main" function.</li>
         *  </ol>
         * </p>
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
         * @brief Load the ELF executable into memory and then schedule the "main" thread of the app.
         * @param executable        Path to an ELF executable.
         * @param argv              Arguments for the app.
         * @param working_directory Initial working directory.
         * @param stdin_target      Stdin stream target.
         * @param stdout_target     Stdout stream target.
         * @param stderr_target     Stderr stream target.
         *
         * @return The final start status of the app. If (LoadStatus == Loaded) then ID will contain the assigned app
         *          ID, otherwise the app ID is -1 and LoadStatus contains the error that happened.
         */
        StartStatus start_new_app(
                const Path& executable,
                char* argv[],
                const Path& working_directory,
                const String& stdin_target,
                const String& stdout_target,
                const String& stderr_target
        );


        /**
         * @brief free all app resources and exit the main thread with the provided exit code.
         *
         * The call will free all user mode memory, close all open files, terminate all threads except the main thread
         * and finally terminate the main thread using it's exit code.
         * @param exit_code Exit code of the apps main thread. >=0 indicates successful app exit and <0 indicates an
         *                  error.
         */
        void exit_running_app(int exit_code);


        /**
         * @brief Make the calling thread wait for an app with the given handle wait until it has exited. If no app with
         *          the requested handle exists a call to this function makes nothing.
         *
         * A call to this function will trigger a context switch, that is the function will only return after the
         * application has exited.
         *
         * @param handle ID of an application.
         * @return INT_MAX: No app with the handle was found, Else: The exit code of the application.
         */
        int join(int handle);
    };
}

#endif //RUNEOS_APPSUBSYSTEM_H
