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

#include <Boot/Boot.h>

#include <Boot/LogRegistry.h>
#include <Boot/FancyLogFormatter.h>

#include <KernelRuntime/String.h>
#include <KernelRuntime/Path.h>

#include <KernelRuntime/Logging.h>
#include <KernelRuntime/CppLanguageSupport.h>

#include <App/AppSubsystem.h>

#include <Memory/MemorySubsystem.h>
#include <Memory/Paging.h>

#include <CPU/CPU.h>
#include <CPU/CPUSubsystem.h>
#include <CPU/Interrupt/Exception.h>
#include <CPU/E9Stream.h>

#include <Device/DeviceSubsystem.h>

#include <VirtualFileSystem/VFSSubsystem.h>
#include <VirtualFileSystem/FileStream.h>

#include <BuiltInPlugin/FATDriverPlugin.h>
#include <BuiltInPlugin/AHCIDriverPlugin.h>
#include <BuiltInPlugin/PITDriverPlugin.h>
#include <BuiltInPlugin/8259PICDriverPlugin.h>

#include <SystemCall/SystemCallSubsystem.h>


namespace Rune {
    constexpr char const* FILE = "Boot";

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                          System Configuration
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    constexpr char const* BOOT_THREAD_NAME = "Boot";
    LogLevel                  KERNEL_LOG_LEVEL = LogLevel::INFO;
    Version                   KERNEL_VERSION   = { K_MAJOR, K_MINOR, K_PATCH, K_PRERELEASE };
    SharedPointer<TextStream> PANIC_STREAM;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                              Kernel Subsystems and Built-in Plugins
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    /**
     * Only the memory subsystem is statically allocated because other subsystems (may) need the kernel heap.
     */
    Memory::MemorySubsystem MEMORY_SUBSYSTEM = Memory::MemorySubsystem();

    constexpr size_t SUBSYSTEM_COUNT = 6;
    Subsystem* KERNEL_SUBSYSTEMS[SUBSYSTEM_COUNT] = { };
    SubsystemRegistry K_SUBSYS_REG(KERNEL_SUBSYSTEMS, SUBSYSTEM_COUNT);


    constexpr char const* LOG_FILE_EXTENSION = ".log";
    LogRegistry                 LOG_REG;
    SharedPointer<Logger> SYSTEM_LOGGER;
    BootLoaderInfo        BOOT_INFO = { };

    constexpr size_t BUILT_IN_PLUGIN_COUNT = 4;
    Plugin* BUILT_IN_PLUGINS[BUILT_IN_PLUGIN_COUNT] = { };


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                          CPP Runtime Callbacks
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    void on_pure_virtual_function_callback() {
        SYSTEM_LOGGER->critical(FILE, "Pure virtual function without implementation called!");
    }


    void on_stack_guard_fail_callback() {
        SYSTEM_LOGGER->critical(FILE, "Yoho, the stack got smashed real hard!");
        while (true) CPU::halt();
    }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                          Boot Functions
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    void start_kernel_subsystem(Subsystem* k_subsys) {
        if (!k_subsys->start(BOOT_INFO, K_SUBSYS_REG)) {
            SYSTEM_LOGGER->critical(FILE, "Subsystem start failure: {}", k_subsys->get_name());
            while (true) CPU::halt();
        }
        SYSTEM_LOGGER->info(FILE, "Subsystem started: {}", k_subsys->get_name());
    }


    void start_built_in_plugins() {
        BUILT_IN_PLUGINS[0] = new BuiltInPlugin::AHCIDriverPlugin();
        BUILT_IN_PLUGINS[1] = new BuiltInPlugin::FATDriverPlugin();
        BUILT_IN_PLUGINS[2] = new BuiltInPlugin::PITDriverPlugin();
        BUILT_IN_PLUGINS[3] = new BuiltInPlugin::_8259PICDriverPlugin();

        for (auto plugin: BUILT_IN_PLUGINS) {
            PluginInfo info = plugin->get_info();
            if (!plugin->start(K_SUBSYS_REG)) {
                SYSTEM_LOGGER->critical(
                        FILE,
                        "Plugin start failure: {} v{} by {}",
                        info.name,
                        info.version.to_string(),
                        info.vendor
                );
                while (true) CPU::halt();
            }
            SYSTEM_LOGGER->info(
                    FILE,
                    "Plugin started: {} v{} by {}",
                    info.name,
                    info.version.to_string(),
                    info.vendor
            );
        }
    }


    void turn_on_serial_logging(UniquePointer<TextStream> txt_stream) {
        LOG_REG.enable_serial_logging(move(txt_stream), KERNEL_LOG_LEVEL);

        SYSTEM_LOGGER->info(
                FILE,
                "runeKernel v{}",
                KERNEL_VERSION.to_string()
        );
        SYSTEM_LOGGER->info(
                FILE,
                "Loaded by {} - v{}",
                BOOT_INFO.boot_loader_name,
                BOOT_INFO.boot_loader_version
        );
        SYSTEM_LOGGER->info(
                FILE,
                "Subsystem started: {}",
                MEMORY_SUBSYSTEM.get_name().to_cstr()
        );
        MEMORY_SUBSYSTEM.log_start_routine_phases();
    }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                      Main Kernel Thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    int kernel_boot_phase_2(CPU::StartInfo* start_info) {
        SILENCE_UNUSED(start_info)

        // Install the panic handler
        PANIC_STREAM = SharedPointer<TextStream>(new CPU::E9Stream);
        CPU::exception_install_panic_stream(PANIC_STREAM);

        // Start the other kernel subsystems
        for (size_t i = 2; i < SUBSYSTEM_COUNT; i++) {
            Subsystem* k_subsys = KERNEL_SUBSYSTEMS[i];
            start_kernel_subsystem(k_subsys);
#ifndef IS_QEMU_HOST
            if (i == 2) { // Device subsystem
                //TODO Set Logging via UART driver
            }
#endif
            if (i == 3) // VFS subsystem
                LOG_REG.enable_file_logging();
        }

        // Switch to a more detailed log formatter
        auto* cpu_subsys = K_SUBSYS_REG.get_as<CPU::CPUSubsystem>(KernelSubsystem::CPU);
        auto* app_subsys = K_SUBSYS_REG.get_as<App::AppSubsystem>(KernelSubsystem::APP);
        auto fancy_log_fmt = SharedPointer<LogFormatter>(new FancyLogFormatter(cpu_subsys, app_subsys));
        LOG_REG.update_log_formatter(fancy_log_fmt);
        SYSTEM_LOGGER->set_log_formatter(fancy_log_fmt);

        // Load the OS
        auto* file_subsys = K_SUBSYS_REG.get_as<VFS::VFSSubsystem>(KernelSubsystem::VFS);
        Path          os(OS);
        VFS::NodeInfo dummy;
        VFS::IOStatus st = file_subsys->get_node_info(os, dummy);
        if (st != VFS::IOStatus::FOUND) {
            SYSTEM_LOGGER->critical(FILE, R"("{}": OS not found!)", os.to_string());
            while (true) CPU::halt();
        }

        App::LoadStatus ls = app_subsys->start_os(os, Path::ROOT);
        if (ls != App::LoadStatus::RUNNING) {
            SYSTEM_LOGGER->critical(
                    FILE,
                    R"("{}": OS start failure! Reason: {})",
                    os.to_string(),
                    ls.to_string()
            );
        }

        // At this point the OS should have taken over control of the system, so we let the boot thread terminate
        return 0;
    }


    void kernel_boot(BootLoaderInfo boot_loader_info) {
        // Kernel boot Phase 1 is still running on the implicit Bootstrap Thread using the bootloader resources
        // Main goal here is to set up memory management, interrupts and scheduling asap
        BOOT_INFO = {
                boot_loader_info.boot_loader_name,
                boot_loader_info.boot_loader_version,
                boot_loader_info.physical_memory_map,
                boot_loader_info.framebuffer,
                boot_loader_info.base_page_table_addr,
                boot_loader_info.stack
        };

        if (!MEMORY_SUBSYSTEM.start(boot_loader_info, K_SUBSYS_REG))
            while (true) CPU::halt();

        // Allocate the kernel subsystems - We do this here because the log registry needs an allocated FILE subsystem
        KERNEL_SUBSYSTEMS[0] = &MEMORY_SUBSYSTEM;
        KERNEL_SUBSYSTEMS[1] = new CPU::CPUSubsystem();
        KERNEL_SUBSYSTEMS[2] = new Device::DeviceSubsystem();
        KERNEL_SUBSYSTEMS[3] = new VFS::VFSSubsystem();
        KERNEL_SUBSYSTEMS[4] = new App::AppSubsystem();
        KERNEL_SUBSYSTEMS[5] = new SystemCall::SystemCallSubsystem();

        // Setup logging
        LOG_REG.init(
                K_SUBSYS_REG.get_as<VFS::VFSSubsystem>(KernelSubsystem::VFS),
                Path("/System")
        );
        SYSTEM_LOGGER = LOG_REG.build_logger(
                KERNEL_LOG_LEVEL,
                Path("System.log")
        );
        MEMORY_SUBSYSTEM.set_logger(
                LOG_REG.build_logger(
                        KERNEL_LOG_LEVEL,
                        Path(MEMORY_SUBSYSTEM.get_name()) / (MEMORY_SUBSYSTEM.get_name() + LOG_FILE_EXTENSION)
                )
        );
#ifdef IS_QEMU_HOST
        // Enable Serial logging via E9 port on Qemu
        turn_on_serial_logging(UniquePointer<TextStream>(new CPU::E9Stream()));
#endif

        init_kernel_runtime_env(
                &on_pure_virtual_function_callback,
                &on_stack_guard_fail_callback
        );

        // Initialize the logging in the kernel subsystems and start the built-in kernel plugins
        for (size_t i = 1; i < SUBSYSTEM_COUNT; i++) {
            auto* k_subsys = KERNEL_SUBSYSTEMS[i];
            k_subsys->set_logger(
                    LOG_REG.build_logger(
                            KERNEL_LOG_LEVEL,
                            Path(k_subsys->get_name()) / (k_subsys->get_name() + LOG_FILE_EXTENSION)
                    )
            );
        }
        start_built_in_plugins();

        auto* cpu_subsys = K_SUBSYS_REG.get_as<CPU::CPUSubsystem>(KernelSubsystem::CPU);
        start_kernel_subsystem(cpu_subsys);
        cpu_subsys->get_scheduler()->lock();
        cpu_subsys->get_scheduler()->terminate(); // Schedule bootstrap termination after unlock
        char* dummy_args[1] = {
                nullptr
        };
        CPU::StartInfo start_info;
        start_info.argc = 0;
        start_info.argv = dummy_args;
        start_info.main = &kernel_boot_phase_2;
        cpu_subsys->schedule_new_thread(
                BOOT_THREAD_NAME,
                &start_info,
                Memory::get_base_page_table_address(),
                CPU::SchedulingPolicy::LOW_LATENCY,
                { nullptr, 0x0, 0x0 }
        );
        cpu_subsys->get_scheduler()->unlock();
    }
}