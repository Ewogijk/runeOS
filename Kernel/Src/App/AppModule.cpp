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

#include <App/AppModule.h>

#include <KRE/System/Lat15-Terminus16.h>
#include <KRE/System/System.h>

#include <App/App.h>
#include <App/ELFLoader.h>
#include <App/TerminalStream.h>
#include <App/VoidStream.h>

#include <VirtualFileSystem/FileStream.h>

namespace Rune::App {
    const SharedPointer<Logger> LOGGER = LogContext::instance().get_logger("App.AppSubsystem");

    DEFINE_ENUM(StdStream, STD_STREAMS, 0x0)

    auto AppModule::schedule_for_start(const SharedPointer<Info>& app,
                                       const CPU::Stack&          user_stack,
                                       CPU::StartInfo*            start_info,
                                       const Path&                working_directory) -> int {
        app->working_directory = move(working_directory);
        LOGGER->info(R"(Starting App "{} v{}" (Vendor: {}) in "{}".)",
                     app->name,
                     app->version.to_string(),
                     app->vendor,
                     app->working_directory.to_string());

        _cpu_module->get_scheduler()->lock();
        int t_id    = _cpu_module->schedule_new_thread("main",
                                                    start_info,
                                                    app->base_page_table_address,
                                                    CPU::SchedulingPolicy::NORMAL,
                                                    user_stack);
        app->handle = _app_handle_counter.acquire();
        _app_table.put(app->handle, app);
        _cpu_module->find_thread(t_id)->app_handle = app->handle;
        app->thread_table.add_back(t_id);
        _cpu_module->get_scheduler()->unlock();
        return app->handle;
    }

    auto AppModule::setup_file_stream(const SharedPointer<Info>& app,
                                      StdStream                  std_stream,
                                      const Path& file_path) -> SharedPointer<TextStream> {
        if (file_path.to_string().is_empty()) return {}; // No file provided
        Path resolved_path = file_path.resolve(_active_app->working_directory);
        if (_vfs_module->is_valid_file_path(resolved_path)) {
            // Setup std stream with a file
            if (std_stream == StdStream::IN) return {}; // Not supported

            SharedPointer<VFS::Node> node;
            VFS::IOStatus            st = _vfs_module->open(resolved_path,
                                                 std_stream == StdStream::IN ? Ember::IOMode::READ
                                                                                        : Ember::IOMode::WRITE,
                                                 node);
            if (st == VFS::IOStatus::NOT_FOUND) {
                // File not found -> Create it
                st = _vfs_module->create(resolved_path, (int) Ember::NodeAttribute::FILE);
                if (st != VFS::IOStatus::CREATED) return {};

                // Try to open it again
                st = _vfs_module->open(resolved_path,
                                       std_stream == StdStream::IN ? Ember::IOMode::READ
                                                                   : Ember::IOMode::WRITE,
                                       node);
            }
            if (st != VFS::IOStatus::OPENED)
                // Cannot open  even after possibly creating it
                return {};

            // The opened file will be added to the active app but should be added to the
            // app to be started
            _active_app->node_table.remove(node->handle);
            app->node_table.add_back(node->handle);
            return SharedPointer<TextStream>(new VFS::FileStream(node));
        }
        return {};
    }

    auto AppModule::setup_std_stream(const SharedPointer<Info>& app,
                                     StdStream                  std_stream,
                                     const Ember::StdIOConfig&  stream_config)
        -> SharedPointer<TextStream> {

        switch (stream_config.target) {
            case Ember::StdIOTarget::VOID:    return SharedPointer<TextStream>(new VoidStream());
            case Ember::StdIOTarget::INHERIT: {
                // Inherit the std stream from the calling app
                switch (std_stream) {
                    case StdStream::IN:  return _active_app->std_in;
                    case StdStream::OUT: return _active_app->std_out;
                    case StdStream::ERR: return _active_app->std_err;
                    default:             return {}; // NONE -> return nullptr
                }
            }
            case Ember::StdIOTarget::FILE:
                return setup_file_stream(app, std_stream, Path(stream_config.argument));
            // case Ember::StdIOTarget::PIPE: return {}; // TODO implement pipes
            default: return {}; // To appease the compiler and linter
        }
    }

    App::AppModule::AppModule()
        : _memory_module(nullptr),
          _cpu_module(nullptr),
          _vfs_module(nullptr),
          _dev_module(nullptr),
          _active_app(nullptr),
          _system_loader_handle(0) {}

    auto AppModule::get_name() const -> String { return "App"; }

    auto AppModule::load(const BootInfo& boot_info) // NOLINT TODO refactor when std::bind is ported
        -> bool {
        System& system = System::instance();
        _memory_module = system.get_module<Memory::MemoryModule>(ModuleSelector::MEMORY);
        _cpu_module    = system.get_module<CPU::CPUModule>(ModuleSelector::CPU);
        _vfs_module    = system.get_module<VFS::VFSModule>(ModuleSelector::VFS);
        _dev_module    = system.get_module<Device::DeviceModule>(ModuleSelector::DEVICE);
        _frame_buffer  = boot_info.framebuffer;

        // Register event hooks
        LOGGER->debug("Registering eventhooks...");
        _cpu_module->install_event_handler(
            CPU::EventHook(CPU::EventHook::THREAD_CREATED).to_string(),
            "App Thread Table Manager - ThreadCreated",
            [this](void* evt_ctx) {
                auto* t       = reinterpret_cast<CPU::Thread*>(evt_ctx);
                t->app_handle = _active_app->handle;
            });
        _cpu_module->install_event_handler(
            CPU::EventHook(CPU::EventHook::THREAD_TERMINATED).to_string(),
            "App Thread Table Manager - ThreadTerminated",
            [this](void* evt_ctx) {
                // Find the app this thread belongs to
                auto* tt_ctx = reinterpret_cast<CPU::ThreadTerminatedContext*>(evt_ctx);
                SharedPointer<Info> finished_app(nullptr);
                for (const auto& app_entry : _app_table) {
                    auto& app = *app_entry.value;
                    if (app->handle == tt_ctx->terminated->app_handle) {
                        app->thread_table.remove(tt_ctx->terminated->handle);
                        if (app->thread_table.is_empty()) finished_app = app;
                        break;
                    }
                }

                // Finish app clean up -> Free base page table and app info struct
                if (finished_app) {
                    LOGGER->trace(R"(Terminating app: "{}-{}"!)",
                                  finished_app->handle,
                                  finished_app->name);

                    Memory::PhysicalMemoryManager* pmm =
                        _memory_module->get_physical_memory_manager();
                    LOGGER->trace("Freeing base page table at {:0=#16x}",
                                  finished_app->base_page_table_address);
                    if (!pmm->free(finished_app->base_page_table_address))
                        LOGGER->warn(R"(Failed to free base page table of "{}-{}.")",
                                     finished_app->handle,
                                     finished_app->name);

                    _app_table.remove(finished_app->handle);
                    // We currently have two refs to the finished app: 1. finishedApp and 2.
                    // _active_app Both will be freed when this event handler finishes
                    if (finished_app.get_ref_count() > 2) {
                        LOGGER->warn(

                            R"(>> Memory Leak << - "{}-{}" has {} references but expected 2.
                                    App info struct will not be freed.)",
                            finished_app->handle,
                            finished_app->name,
                            finished_app.get_ref_count());
                    }
                }

                // Switch the active app if the next thread does belong to another app
                if (_active_app->handle != tt_ctx->next_scheduled->app_handle) {
                    SharedPointer<Info> next_active(nullptr);
                    for (const auto& app_entry : _app_table) {
                        auto& app = *app_entry.value;
                        if (app->handle == tt_ctx->next_scheduled->app_handle) next_active = app;
                    }
                    LOGGER->trace(R"(Switching running app: "{}" -> "{}")",
                                  _active_app->name,
                                  next_active ? next_active->name : "");
                    _active_app = next_active;
                }
            });
        _cpu_module->install_event_handler(
            CPU::EventHook(CPU::EventHook::CONTEXT_SWITCH).to_string(),
            "App Thread Table Manager - ContextSwitch",
            [this](void* evt_ctx) {
                auto* next = reinterpret_cast<CPU::Thread*>(evt_ctx);
                // Switch the active app if the next thead belongs to another app
                if (next->app_handle != _active_app->handle) {
                    for (const auto& app_entry : _app_table) {
                        auto& app = *app_entry.value;
                        if (app->handle == next->app_handle) {
                            LOGGER->trace(R"(Switching running app: "{}-{}" -> "{}-{}")",
                                          _active_app->handle,
                                          _active_app->name,
                                          app->handle,
                                          app->name);
                            _active_app = app;
                            break;
                        }
                    }
                }
            });

        _vfs_module->install_event_handler(
            VFS::EventHook(VFS::EventHook::NODE_OPENED).to_string(),
            "App Node Table Manager - On Open",
            [this](void* evt_ctx) {
                U16 handle = *reinterpret_cast<U16*>(evt_ctx);
                LOGGER->trace(R"(Add node handle {} to node table of app "{}-{}".)",
                              handle,
                              _active_app->handle,
                              _active_app->name);
                _active_app->node_table.add_back(handle);
            });
        _vfs_module->install_event_handler(
            VFS::EventHook(VFS::EventHook::NODE_CLOSED).to_string(),
            "App Node Table Manager - On Close",
            [this](void* evt_ctx) {
                U16 handle = *reinterpret_cast<U16*>(evt_ctx);
                LOGGER->trace(R"(Remove node handle {} from the node table of app "{}-{}".)",
                              handle,
                              _active_app->handle,
                              _active_app->name);
                _active_app->node_table.remove(handle);
            });

        _vfs_module->install_event_handler(
            VFS::EventHook(VFS::EventHook::DIRECTORY_STREAM_OPENED).to_string(),
            "App Directory Stream Table Manager - On Open",
            [this](void* evt_ctx) {
                U16 handle = *reinterpret_cast<U16*>(evt_ctx);
                LOGGER->trace(

                    R"(Add directory stream handle {} to directory stream table of app "{}-{}".)",
                    handle,
                    _active_app->handle,
                    _active_app->name);
                _active_app->directory_stream_table.add_back(handle);
            });
        _vfs_module->install_event_handler(
            VFS::EventHook(VFS::EventHook::DIRECTORY_STREAM_CLOSED).to_string(),
            "App Directory Stream Table Manager - On Close",
            [this](void* evt_ctx) {
                U16 handle = *reinterpret_cast<U16*>(evt_ctx);
                LOGGER->trace(

                    R"(Remove directory stream handle {} from the directory stream table of app "{}-{}".)",
                    handle,
                    _active_app->handle,
                    _active_app->name);
                _active_app->directory_stream_table.remove(handle);
            });

        // A dummy app that belongs to the kernel itself, which owns the kernel logs files and all
        // threads running at this moment (idle, terminator and boot)
        auto kernel_app     = SharedPointer<Info>(new Info());
        kernel_app->name    = "KApp";
        kernel_app->vendor  = "Ewogijk";
        kernel_app->version = {.major       = MAJOR,
                               .minor       = MINOR,
                               .patch       = PATCH,
                               .pre_release = PRERELEASE};
        kernel_app->handle  = _app_handle_counter.acquire();

        // This is a dummy app that will be removed hence the standard IO streams are attached to
        // nothing
        kernel_app->std_out = SharedPointer<TextStream>(new VoidStream());
        kernel_app->std_err = kernel_app->std_out;
        kernel_app->std_in  = kernel_app->std_in;

        kernel_app->base_page_table_address = Memory::get_base_page_table_address();
        _app_table.put(kernel_app->handle, kernel_app);

        for (auto& t : _cpu_module->get_thread_table()) {
            kernel_app->thread_table.add_back(t->handle);
            t->app_handle = kernel_app->handle;
        }

        for (auto& f_e : _vfs_module->get_node_table())
            kernel_app->node_table.add_back(f_e->handle);

        _active_app = kernel_app;
        LOGGER->debug(R"(Initialize the kernel app "v{} " by {}.)",
                      kernel_app->name,
                      kernel_app->version.to_string(),
                      kernel_app->vendor);
        return true;
    }

    auto AppModule::get_app_table() const -> LinkedList<Info*> {
        LinkedList<Info*> apps;
        for (const auto& app_entry : _app_table) apps.add_back(app_entry.value->get());
        return apps;
    }

    auto AppModule::get_active_app() const -> Info* { return _active_app.get(); }

    void AppModule::dump_app_table(const SharedPointer<TextStream>& stream) const {
        constexpr U8 COLUMN_COUNT = 7;
        Table<SharedPointer<Info>, COLUMN_COUNT>::make_table(
            [this](const SharedPointer<Info>& info) -> Array<String, COLUMN_COUNT> {
                return {
                    String::format("{}-{}", info->handle, info->name),
                    info->version.to_string(),
                    info->vendor,
                    info->location.to_string(),
                    ID_list_to_string(info->thread_table),
                    ID_list_to_string(info->node_table),
                    ID_list_to_string(info->directory_stream_table),
                };
            })
            .with_headers({"ID-Name",
                           "Version",
                           "Vendor",
                           "Location",
                           "Thread Table",
                           "Node Table",
                           "Directory Stream Table"})
            .with_data(_app_table.values())
            .print(stream);
    }

    auto AppModule::start_system_loader(const Path& system_loader_executable,
                                        const Path& working_directory) -> LoadStatus {
        if (!_app_handle_counter.has_more()) return LoadStatus::LOAD_ERROR;
        ELFLoader   loader(_memory_module, _vfs_module);
        auto        app = SharedPointer<Info>(new Info());
        CPU::Stack  user_stack;
        VirtualAddr start_info_addr = 0;
        LOGGER->info("Loading OS: {}", system_loader_executable.to_string());
        char*      dummy_args[1] = {nullptr}; // NOLINT syscall arg, must use ptr
        LoadStatus load_status   = loader.load(system_loader_executable,
                                             dummy_args,
                                             app,
                                             user_stack,
                                             start_info_addr,
                                             true);
        if (load_status != LoadStatus::LOADED) {
            LOGGER->warn("Failed to load OS. Status: {}", load_status.to_string());
            return load_status;
        }

        // Hook up the OS stdin/stderr to the terminal stream that renders on the display
        app->std_out = SharedPointer<TextStream>(new TerminalStream(_cpu_module,
                                                                    &_frame_buffer,
                                                                    &LAT15TERMINUS16,
                                                                    Pixie::BLACK,
                                                                    Pixie::VSCODE_WHITE));
        // Set the error stream also to the terminal stream, just print text in red
        app->std_err = app->std_out;
        // Hook up the stdin to the keyboard
        app->std_in = SharedPointer<TextStream>(_dev_module->get_keyboard().get());

        _system_loader_handle =
            schedule_for_start(app,
                               user_stack,
                               memory_addr_to_pointer<CPU::StartInfo>(start_info_addr),
                               move(working_directory));
        return LoadStatus::RUNNING;
    }

    auto AppModule::start_new_app(const Path&               executable,
                                  char**                    argv,
                                  const Path&               working_directory,
                                  const Ember::StdIOConfig& stdin_config,
                                  const Ember::StdIOConfig& stdout_config,
                                  const Ember::StdIOConfig& stderr_config) -> StartStatus {
        if (!_app_handle_counter.has_more())
            return {.load_result = LoadStatus::LOAD_ERROR, .handle = -1};
        ELFLoader   loader(_memory_module, _vfs_module);
        auto        app = SharedPointer<Info>(new Info());
        CPU::Stack  user_stack;
        VirtualAddr start_info_addr = 0;
        LOGGER->info("Loading executable: {}", executable.to_string());
        LoadStatus load_status =
            loader.load(executable, argv, app, user_stack, start_info_addr, false);
        if (load_status != LoadStatus::LOADED) {
            LOGGER->warn("Failed to load executable. Status: {}", load_status.to_string());
            return {.load_result = load_status, .handle = -1};
        }

        auto std_in = setup_std_stream(app, StdStream::IN, stdin_config);
        if (!std_in) {
            LOGGER->warn("{}: Could not open \"{}\" stdin stream.",
                         executable.to_string(),
                         stdin_config.target.to_string());
            return {.load_result = LoadStatus::BAD_STDIO, .handle = -1};
        }

        auto std_out = setup_std_stream(app, StdStream::OUT, stdout_config);
        if (!std_out) {
            LOGGER->warn("{}: Could not open \"{}\" stdin stream.",
                         executable.to_string(),
                         stdout_config.target.to_string());
            return {.load_result = LoadStatus::BAD_STDIO, .handle = -1};
        }

        SharedPointer<TextStream> std_err;
        if (stdout_config.target == stderr_config.target) {
            // Point stderr to stdout
            std_err = std_out;
        } else {
            // Open new stream for stderr
            std_err = setup_std_stream(app, StdStream::ERR, stderr_config);
            if (!std_err) {
                LOGGER->warn("{}: Could not open \"{}\" stdin stream.",
                             executable.to_string(),
                             stderr_config.target.to_string());
                return {.load_result = LoadStatus::BAD_STDIO, .handle = -1};
            }
        }
        app->std_in  = move(std_in);
        app->std_out = move(std_out);
        app->std_err = move(std_err);
        int app_id   = schedule_for_start(app,
                                        user_stack,
                                        memory_addr_to_pointer<CPU::StartInfo>(start_info_addr),
                                        move(working_directory));
        return {.load_result = LoadStatus::RUNNING, .handle = app_id};
    }

    void AppModule::exit_running_app(int exit_code) {
        // The system loader is not allowed to exit!
        // While technically okay, this would lead to the system with only the idle thread running
        // which renders it useless.
        if (_system_loader_handle == _active_app->handle) {
#ifdef SHUTDOWN_ON_SYSTEM_LOADER_EXIT
            System::instance().shutdown();
#else
            System::instance().panic("The system loader shall not exit!");
#endif
        }

        _active_app->exit_code = exit_code;

        // Close std io streams
        _active_app->std_in->close();
        _active_app->std_out->close();
        _active_app->std_err->close();

        LOGGER->debug(R"(App "{}-{}" has exited.)", _active_app->handle, _active_app->name);
        LOGGER->debug("Freeing user mode memory...");
        if (!_memory_module->get_virtual_memory_manager()->free_virtual_address_space(
                _active_app->base_page_table_address)) {
            LOGGER->warn(R"(Failed to free virtual address space of app "{}-{}")",
                         _active_app->handle,
                         _active_app->name);
        }

        LOGGER->debug("Terminating all app threads...");
        for (auto r_t : _active_app->thread_table) {
            if (!_cpu_module->terminate_thread(r_t)
                && r_t != _cpu_module->get_scheduler()->get_running_thread()->handle) {
                LOGGER->warn(R"(Failed to terminate thread with ID {}.)", r_t);
            }
        }
        _active_app->thread_table.clear();

        LOGGER->debug("Closing all open nodes of the app...");
        for (auto handle : _active_app->node_table) {
            auto node = _vfs_module->find_node(handle);
            if (node)
                node->close();
            else
                LOGGER->warn(R"(Failed to close node with handle {}.)", handle);
        }
        _active_app->node_table.clear();

        // Schedule all threads joining with this app
        auto* scheduler = _cpu_module->get_scheduler();
        scheduler->lock();
        LOGGER->debug("Scheduling all joining threads...");
        for (auto& j_t : _active_app->joining_thread_table) {
            j_t->join_app_id = 0;
            scheduler->schedule(j_t);
        }
        _active_app->joining_thread_table.clear();
        scheduler->unlock();

        CPU::thread_exit(exit_code);
    }

    auto AppModule::join(U16 handle) -> int {
        // Important: We need to keep a copy of the shared pointer here, so that the app info does
        // not get freed
        //              when the final context switch from its main thread to the next thread
        //              happens after it has exited, otherwise the info gets freed, and it is no
        //              longer possible to access its exit code.
        SharedPointer<Info> app;
        for (const auto& app_entry : _app_table) {
            auto& a = *app_entry.value; // NOLINT app_table cannot contain null entries
            if (a->handle == handle) app = a;
        }
        if (!app) {
            LOGGER->debug(R"(No app with ID {} was found.)", handle);
            return INT_MAX;
        }

        auto* scheduler = _cpu_module->get_scheduler();
        scheduler->lock();
        auto r_t = scheduler->get_running_thread();
        LOGGER->debug(R"(Thread "{}-{}" is joining with app "{}-{}")",
                      r_t->handle,
                      r_t->name,
                      app->handle,
                      app->name);
        r_t->join_app_id = app->handle;
        r_t->state       = CPU::ThreadState::WAITING;
        app->joining_thread_table.add_back(r_t);
        scheduler->execute_next_thread();
        // The "unlock" call will trigger a context switch to whatever next thread will be run and
        // this thread will wait until it is scheduled again in the "exit_running_app" function.
        scheduler->unlock();
        // The application has exited here, meaning this thread was rescheduled in
        // "exit_running_app" at some point thus the exit_code of the app is now set
        return app->exit_code;
    }
} // namespace Rune::App
