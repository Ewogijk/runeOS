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

#include <App/AppSubsystem.h>

#include <KernelRuntime/Lat15-Terminus16.h>

#include <App/TerminalStream.h>
#include <App/VoidStream.h>
#include <App/App.h>
#include <App/ELFLoader.h>

#include <VirtualFileSystem/FileStream.h>


namespace Rune::App {
    constexpr char const* FILE = "App";

    DEFINE_ENUM(StdStream, STD_STREAMS, 0x0)


    int AppSubsystem::schedule_for_start(
        const SharedPointer<Info>& app,
        const CPU::Stack&          user_stack,
        CPU::StartInfo*             start_info,
        const Path&                working_directory
    ) {
        app->working_directory = move(working_directory);
        _logger->info(
            FILE,
            R"(Starting App "{} v{}" (Vendor: {}) in "{}".)",
            app->name,
            app->version.to_string(),
            app->vendor,
            app->working_directory.to_string()
        );

        _cpu_subsys->get_scheduler()->lock();
        int t_id = _cpu_subsys->schedule_new_thread(
            "main",
            start_info,
            app->base_page_table_address,
            CPU::SchedulingPolicy::NORMAL,
            user_stack
        );
        app->handle = _app_handle_counter.acquire_handle();
        _app_table.put(app->handle, app);
        _cpu_subsys->find_thread(t_id)->app_handle = app->handle;
        app->thread_table.add_back(t_id);
        _cpu_subsys->get_scheduler()->unlock();
        return app->handle;
    }


    SharedPointer<TextStream> AppSubsystem::setup_std_stream(
        const SharedPointer<Info>& app,
        StdStream                  std_stream,
        const Rune::String&        target
    ) {
        LinkedList<String> t_split = target.split(':');
        if (t_split.is_empty() || t_split.size() > 2)
            return { };
        String t = *t_split[0];
        String arg;
        if (t_split.size() > 1)
            arg = *t_split[1];

        if (t == "inherit") {
            // Inherit the std stream from the calling app
            switch (std_stream) {
                case StdStream::IN:
                    return _active_app->std_in;
                case StdStream::OUT:
                    return _active_app->std_out;
                case StdStream::ERR:
                    return _active_app->std_err;
                default:
                    return { }; // NONE -> return nullptr
            }
        } else if (t == "void") {
            return SharedPointer<TextStream>(new VoidStream());
        } else if (t == "file") {
            if (arg.is_empty())
                return { }; // No file provided
            Path maybe_path = Path(arg).resolve(_active_app->working_directory);
            if (_vfs_subsys->is_valid_file_path(maybe_path)) {
                // Setup std stream with a file
                if (std_stream == StdStream::IN)
                    return { }; // Not supported

                SharedPointer<VFS::Node> node;
                VFS::IOStatus            st = _vfs_subsys->open(
                    maybe_path,
                    std_stream == StdStream::IN ? Ember::IOMode::READ : Ember::IOMode::WRITE,
                    node
                );
                if (st == VFS::IOStatus::NOT_FOUND) {
                    // File not found -> Create it
                    st = _vfs_subsys->create(maybe_path, (int)Ember::NodeAttribute::FILE);
                    if (st != VFS::IOStatus::CREATED)
                        return { };

                    // Try to open it again
                    st = _vfs_subsys->open(
                        maybe_path,
                        std_stream == StdStream::IN ? Ember::IOMode::READ : Ember::IOMode::WRITE,
                        node
                    );
                }
                if (st != VFS::IOStatus::OPENED)
                    // Cannot open file, even after possibly creating it
                    return { };

                // The opened file will be added to the active app but should be added to the app to be started
                _active_app->node_table.remove(node->handle);
                app->node_table.add_back(node->handle);
                return SharedPointer<TextStream>(new VFS::FileStream(node));
            }
        } else if (t == "pipe") {
            // TODO implement pipes
        }
        return { };
    }


    App::AppSubsystem::AppSubsystem() :
        Subsystem(),
        _memory_subsys(nullptr),
        _cpu_subsys(nullptr),
        _vfs_subsys(nullptr),
        _dev_subsys(nullptr),
        _app_table(),
        _app_table_fmt(),
        _app_handle_counter(),
        _active_app(nullptr) { }


    String AppSubsystem::get_name() const {
        return "App";
    }


    bool AppSubsystem::start(
        const BootLoaderInfo&    boot_info,
        const SubsystemRegistry& k_subsys_reg
    ) {
        _memory_subsys = k_subsys_reg.get_as<Memory::MemorySubsystem>(KernelSubsystem::MEMORY);
        _cpu_subsys    = k_subsys_reg.get_as<CPU::CPUSubsystem>(KernelSubsystem::CPU);
        _vfs_subsys    = k_subsys_reg.get_as<VFS::VFSSubsystem>(KernelSubsystem::VFS);
        _dev_subsys    = k_subsys_reg.get_as<Device::DeviceSubsystem>(KernelSubsystem::DEVICE);
        _frame_buffer  = boot_info.framebuffer;

        // Setup app table
        LinkedList<Column<Info>> at_cols;
        at_cols.add_back(Column<Info>::make_handle_column_table(26));
        at_cols.add_back(
            {
                "Version",
                12,
                [](Info* app) {
                    return app->version.to_string();
                }
            }
        );
        at_cols.add_back(
            {
                "Vendor",
                12,
                [](Info* app) {
                    return app->vendor;
                }
            }
        );
        at_cols.add_back(
            {
                "Location",
                20,
                [](Info* app) {
                    return app->location.to_string();
                }
            }
        );
        at_cols.add_back(
            {
                "Thread Table",
                50,
                [](Info* app) {
                    String threads = "";
                    for (auto handle : app->thread_table)
                        threads += String::format("{}, ", handle);

                    if (threads.is_empty())
                        threads = "-";
                    return threads;
                }
            }
        );
        at_cols.add_back(
            {
                "Node Table",
                50,
                [](Info* app) {
                    String files = "";
                    for (auto handle : app->node_table)
                        files += String::format("{}, ", handle);

                    if (files.is_empty())
                        files = "-";
                    return files;
                }
            }
        );
        at_cols.add_back(
            {
                "Directory Stream Table",
                50,
                [](Info* app) {
                    String files = "";
                    for (auto handle : app->directory_stream_table)
                        files += String::format("{}, ", handle);

                    if (files.is_empty())
                        files = "-";
                    return files;
                }
            }
        );
        _app_table_fmt.configure("App", at_cols);

        // Register event hooks
        _logger->debug(FILE, "Registering eventhooks...");
        _cpu_subsys->install_event_handler(
            CPU::EventHook(CPU::EventHook::THREAD_CREATED).to_string(),
            "App Thread Table Manager - ThreadCreated",
            [this](void* evt_ctx) {
                auto t        = (CPU::Thread*)evt_ctx;
                t->app_handle = _active_app->handle;
            }
        );
        _cpu_subsys->install_event_handler(
            CPU::EventHook(CPU::EventHook::THREAD_TERMINATED).to_string(),
            "App Thread Table Manager - ThreadTerminated",
            [this](void* evt_ctx) {
                // Find the app this thread belongs to
                auto*               tt_ctx = (CPU::ThreadTerminatedContext*)evt_ctx;
                SharedPointer<Info> finished_app(nullptr);
                for (auto& app_entry : _app_table) {
                    auto& app = *app_entry.value;
                    if (app->handle == tt_ctx->terminated->app_handle) {
                        app->thread_table.remove(tt_ctx->terminated->handle);
                        if (app->thread_table.is_empty())
                            finished_app = app;
                        break;
                    }
                }

                // Finish app clean up -> Free base page table and app info struct
                if (finished_app) {
                    _logger->trace(FILE, R"(Terminating app: "{}-{}"!)", finished_app->handle, finished_app->name);

                    Memory::PhysicalMemoryManager* pmm = _memory_subsys->get_physical_memory_manager();
                    _logger->trace(
                        FILE,
                        "Freeing base page table at {:0=#16x}",
                        finished_app->base_page_table_address
                    );
                    if (!pmm->free(finished_app->base_page_table_address))
                        _logger->warn(
                            FILE,
                            R"(Failed to free base page table of "{}-{}.")",
                            finished_app->handle,
                            finished_app->name
                        );

                    _app_table.remove(finished_app->handle);
                    // We currently have two refs to the finished app: 1. finishedApp and 2. _active_app
                    // Both will be freed when this event handler finishes
                    if (finished_app.get_ref_count() > 2) {
                        _logger->warn(
                            FILE,
                            R"(>> Memory Leak << - "{}-{}" has {} references but expected 2.
                                    App info struct will not be freed.)",
                            finished_app->handle,
                            finished_app->name,
                            finished_app.get_ref_count()
                        );
                    }
                }

                // Switch the active app if the next thread does belong to another app
                if (_active_app->handle != tt_ctx->next_scheduled->app_handle) {
                    SharedPointer<Info> next_active(nullptr);
                    for (auto& app_entry : _app_table) {
                        auto& app = *app_entry.value;
                        if (app->handle == tt_ctx->next_scheduled->app_handle)
                            next_active = app;
                    }
                    _logger->trace(
                        FILE,
                        R"(Switching running app: "{}" -> "{}")",
                        _active_app->name,
                        next_active ? next_active->name : ""
                    );
                    _active_app = next_active;
                }
            }
        );
        _cpu_subsys->install_event_handler(
            CPU::EventHook(CPU::EventHook::CONTEXT_SWITCH).to_string(),
            "App Thread Table Manager - ContextSwitch",
            [this](void* evt_ctx) {
                auto* next = (CPU::Thread*)evt_ctx;
                // Switch the active app if the next thead belongs to another app
                if (next->app_handle != _active_app->handle) {
                    for (auto& app_entry : _app_table) {
                        auto& app = *app_entry.value;
                        if (app->handle == next->app_handle) {
                            _logger->trace(
                                FILE,
                                R"(Switching running app: "{}-{}" -> "{}-{}")",
                                _active_app->handle,
                                _active_app->name,
                                app->handle,
                                app->name
                            );
                            _active_app = app;
                            break;
                        }
                    }
                }
            }
        );

        _vfs_subsys->install_event_handler(
            VFS::EventHook(VFS::EventHook::NODE_OPENED).to_string(),
            "App Node Table Manager - On Open",
            [this](void* evt_ctx) {
                U16 handle = *((U16*)evt_ctx);
                _logger->trace(
                    FILE,
                    R"(Add node handle {} to node table of app "{}-{}".)",
                    handle,
                    _active_app->handle,
                    _active_app->name
                );
                _active_app->node_table.add_back(handle);
            }
        );
        _vfs_subsys->install_event_handler(
            VFS::EventHook(VFS::EventHook::NODE_CLOSED).to_string(),
            "App Node Table Manager - On Close",
            [this](void* evt_ctx) {
                U16 handle = *((U16*)evt_ctx);
                _logger->trace(
                    FILE,
                    R"(Remove node handle {} from the node table of app "{}-{}".)",
                    handle,
                    _active_app->handle,
                    _active_app->name
                );
                _active_app->node_table.remove(handle);
            }
        );

        _vfs_subsys->install_event_handler(
            VFS::EventHook(VFS::EventHook::DIRECTORY_STREAM_OPENED).to_string(),
            "App Directory Stream Table Manager - On Open",
            [this](void* evt_ctx) {
                U16 handle = *((U16*)evt_ctx);
                _logger->trace(
                    FILE,
                    R"(Add directory stream handle {} to directory stream table of app "{}-{}".)",
                    handle,
                    _active_app->handle,
                    _active_app->name
                );
                _active_app->directory_stream_table.add_back(handle);
            }
        );
        _vfs_subsys->install_event_handler(
            VFS::EventHook(VFS::EventHook::DIRECTORY_STREAM_CLOSED).to_string(),
            "App Directory Stream Table Manager - On Close",
            [this](void* evt_ctx) {
                U16 handle = *((U16*)evt_ctx);
                _logger->trace(
                    FILE,
                    R"(Remove directory stream handle {} from the directory stream table of app "{}-{}".)",
                    handle,
                    _active_app->handle,
                    _active_app->name
                );
                _active_app->directory_stream_table.remove(handle);
            }
        );

        // A dummy app that belongs to the kernel itself, which owns the kernel logs files and all threads running at
        // this moment (idle, terminator and boot)
        auto kernel_app     = SharedPointer<Info>(new Info());
        kernel_app->name    = "KApp";
        kernel_app->vendor  = "Ewogijk";
        kernel_app->version = {MAJOR, MINOR, PATCH, PRERELEASE};
        kernel_app->handle  = _app_handle_counter.acquire_handle();

        // This is a dummy app that will be removed hence the standard IO streams are attached to nothing
        kernel_app->std_out = SharedPointer<TextStream>(new VoidStream());
        kernel_app->std_err = kernel_app->std_out;
        kernel_app->std_in  = kernel_app->std_in;

        kernel_app->base_page_table_address = Memory::get_base_page_table_address();
        _app_table.put(kernel_app->handle, kernel_app);

        for (auto& t : _cpu_subsys->get_thread_table()) {
            kernel_app->thread_table.add_back(t->handle);
            t->app_handle = kernel_app->handle;
        }

        for (auto& f_e : _vfs_subsys->get_node_table())
            kernel_app->node_table.add_back(f_e->handle);

        _active_app = kernel_app;
        _logger->debug(
            FILE,
            R"(Initialize the kernel app "v{} " by {}.)",
            kernel_app->name,
            kernel_app->version.to_string(),
            kernel_app->vendor
        );
        return true;
    }


    void AppSubsystem::set_logger(SharedPointer<Logger> logger) {
        _logger = logger;
    }


    LinkedList<Info*> AppSubsystem::get_app_table() const {
        LinkedList<Info*> apps;
        for (auto& app_entry : _app_table)
            apps.add_back(app_entry.value->get());
        return apps;
    }


    Info* AppSubsystem::get_active_app() const {
        return _active_app.get();
    }


    void AppSubsystem::dump_app_table(const SharedPointer<TextStream>& stream) const {
        auto it = _app_table.begin();
        _app_table_fmt.dump(
            stream,
            [&it] {
                Info* i = nullptr;
                if (it.has_next()) {
                    i = it->value->get();
                    ++it;
                }
                return i;
            }
        );
    }


    LoadStatus AppSubsystem::start_os(const Path& os_exec, const Path& working_directory) {
        if (!_app_handle_counter.has_more_handles())
            return LoadStatus::LOAD_ERROR;
        ELFLoader         loader(_memory_subsys, _vfs_subsys, _logger);
        auto              app = SharedPointer<Info>(new Info());
        CPU::Stack        user_stack;
        VirtualAddr start_info_addr;
        _logger->info(FILE, "Loading OS: {}", os_exec.to_string());
        char*       dummy_args[1] = {
            nullptr
        };
        LoadStatus load_status = loader.load(os_exec, dummy_args, app, user_stack, start_info_addr, true);
        if (load_status != LoadStatus::LOADED) {
            _logger->warn(FILE, "Failed to load OS. Status: {}", load_status.to_string());
            return load_status;
        }

        // Hook up the OS stdin/stderr to the terminal stream that renders on the display
        app->std_out = SharedPointer<TextStream>(
            new TerminalStream(
                _cpu_subsys,
                &_frame_buffer,
                &LAT15TERMINUS16,
                Pixie::BLACK,
                Pixie::VSCODE_WHITE
            )
        );
        // Set the error stream also to the terminal stream, just print text in red
        app->std_err = app->std_out;
        // Hook up the stdin to the keyboard
        app->std_in = SharedPointer<TextStream>(_dev_subsys->get_keyboard().get());

        schedule_for_start(
            app,
            user_stack,
            memory_addr_to_pointer<CPU::StartInfo>(start_info_addr),
            move(working_directory)
        );
        return LoadStatus::RUNNING;
    }


    StartStatus AppSubsystem::start_new_app(
        const Path&   executable,
        char**        argv,
        const Path&   working_directory,
        const String& stdin_target,
        const String& stdout_target,
        const String& stderr_target
    ) {
        if (!_app_handle_counter.has_more_handles())
            return {LoadStatus::LOAD_ERROR, -1};
        ELFLoader         loader(_memory_subsys, _vfs_subsys, _logger);
        auto              app = SharedPointer<Info>(new Info());
        CPU::Stack        user_stack;
        VirtualAddr start_info_addr;
        _logger->info(FILE, "Loading executable: {}", executable.to_string());
        LoadStatus load_status = loader.load(executable, argv, app, user_stack, start_info_addr, false);
        if (load_status != LoadStatus::LOADED) {
            _logger->warn(FILE, "Failed to load executable. Status: {}", load_status.to_string());
            return {load_status, -1};
        }

        auto std_in = setup_std_stream(app, StdStream::IN, stdin_target);
        if (!std_in) {
            _logger->warn(FILE, "{}: Unknown stdin target. Got: {}", executable.to_string(), stdin_target);
            return {LoadStatus::BAD_STDIO, -1};
        }

        auto std_out = setup_std_stream(app, StdStream::OUT, stdout_target);
        if (!std_out) {
            _logger->warn(FILE, "{}: Unknown std_out target. Got: {}", executable.to_string(), stdout_target);
            return {LoadStatus::BAD_STDIO, -1};
        }

        SharedPointer<TextStream> std_err;
        if (stdout_target == stderr_target) {
            // Point stderr to stdout
            std_err = std_out;
        } else {
            // Open new stream for stderr
            std_err = setup_std_stream(app, StdStream::ERR, stderr_target);
            if (!std_err) {
                _logger->warn(FILE, "{}: Unknown std_err target. Got: {}", executable.to_string(), stderr_target);
                return {LoadStatus::BAD_STDIO, -1};
            }
        }
        app->std_in  = move(std_in);
        app->std_out = move(std_out);
        app->std_err = move(std_err);
        int app_id   = schedule_for_start(
            app,
            user_stack,
            memory_addr_to_pointer<CPU::StartInfo>(start_info_addr),
            move(working_directory)
        );
        return {LoadStatus::RUNNING, app_id};
    }


    void AppSubsystem::exit_running_app(int exit_code) {
        _active_app->exit_code = exit_code;

        // Close std io streams
        _active_app->std_in->close();
        _active_app->std_out->close();
        _active_app->std_err->close();

        _logger->debug(FILE, R"(App "{}-{}" has exited.)", _active_app->handle, _active_app->name);
        _logger->debug(FILE, "Freeing user mode memory...");
        if (!_memory_subsys->get_virtual_memory_manager()
                           ->free_virtual_address_space(_active_app->base_page_table_address)) {
            _logger->warn(
                FILE,
                R"(Failed to free virtual address space of app "{}-{}")",
                _active_app->handle,
                _active_app->name
            );
        }

        _logger->debug(FILE, "Terminating all app threads...");
        for (auto r_t : _active_app->thread_table) {
            if (!_cpu_subsys->terminate_thread(r_t)
                && r_t != _cpu_subsys->get_scheduler()->get_running_thread()->handle) {
                _logger->warn(FILE, R"(Failed to terminate thread with ID {}.)", r_t);
            }
        }
        _active_app->thread_table.clear();

        _logger->debug(FILE, "Closing all open nodes of the app...");
        for (auto handle : _active_app->node_table) {
            auto node = _vfs_subsys->find_node(handle);
            if (node)
                node->close();
            else
                _logger->warn(FILE, R"(Failed to close node with handle {}.)", handle);
        }
        _active_app->node_table.clear();

        // Schedule all threads joining with this app
        auto* scheduler = _cpu_subsys->get_scheduler();
        scheduler->lock();
        _logger->debug(FILE, "Scheduling all joining threads...");
        for (auto& j_t : _active_app->joining_thread_table) {
            j_t->join_app_id = 0;
            scheduler->schedule(j_t);
        }
        _active_app->joining_thread_table.clear();
        scheduler->unlock();

        CPU::thread_exit(exit_code);
    }


    int AppSubsystem::join(int handle) {
        // Important: We need to keep a copy of the shared pointer here, so that the app info does not get freed
        //              when the final context switch from its main thread to the next thread happens after it has
        //              exited, otherwise the info gets freed and it is no longer possible to access its exit code.
        SharedPointer<Info> app;
        for (auto& app_entry : _app_table) {
            auto& a = *app_entry.value;
            if (a->handle == handle)
                app = a;
        }
        if (!app) {
            _logger->debug(FILE, R"(No app with ID {} was found.)", handle);
            return INT_MAX;
        }

        auto* scheduler = _cpu_subsys->get_scheduler();
        scheduler->lock();
        auto r_t = scheduler->get_running_thread();
        _logger->debug(
            FILE,
            R"(Thread "{}-{}" is joining with app "{}-{}")",
            r_t->handle,
            r_t->name,
            app->handle,
            app->name
        );
        r_t->join_app_id = app->handle;
        r_t->state       = CPU::ThreadState::WAITING;
        app->joining_thread_table.add_back(r_t);
        scheduler->execute_next_thread();
        // The "unlock" call will trigger a context switch to whatever next thread will be run and this thread
        // will wait until it is scheduled again in the "exit_running_app" function.
        scheduler->unlock();
        // The application has exited here, meaning this thread was rescheduled in "exit_running_app" at some point
        // thus the exit_code of the app is now set
        return app->exit_code;
    }
}
