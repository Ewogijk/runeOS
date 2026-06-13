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

#include <KRE/System/System.h>

#include <KRE/CRL/CRL.h>
#include <KRE/CRL/GlobalConstructor.h>

#include <BuiltInPlugin/8259PICDriverPlugin.h>
#include <BuiltInPlugin/ACPIDriverPlugin.h>
#include <BuiltInPlugin/AHCIDriverPlugin.h>
#include <BuiltInPlugin/FATDriverPlugin.h>
#include <BuiltInPlugin/PCIDriverPlugin.h>
#include <BuiltInPlugin/PITDriverPlugin.h>
#include <BuiltInPlugin/PS2KeyboardDriverPlugin.h>

#include <CPU/CPU.h>
#include <CPU/CPUModule.h>
#include <CPU/E9Stream.h>
#include <CPU/Interrupt/Exception.h>

#include <Memory/MemoryModule.h>

#include <Device/DeviceModule.h>

#include <VirtualFileSystem/FileStream.h>
#include <VirtualFileSystem/VFSModule.h>

#include <App/AppModule.h>

#include <SystemCall/SystemCallModule.h>

#include <Boot/DetailedLogLayout.h>

#ifdef RUN_UNIT_TESTS
#include <Test/UnitTest/Runner.h>
#endif

#include <Device/ACPI/ACPI.h>

namespace Rune {
    const SharedPointer<Logger> LOGGER = LogContext::instance().get_logger("System");

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                  Helper Functions
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    void on_pure_virtual_function_callback() {
        System::instance().panic("Pure virtual function not implemented!");
    }

    void on_stack_guard_fail_callback() {
        System::instance().panic("Stack guard failure detected!");
    }

    void on_std_terminate() { System::instance().panic("std::terminate was called!"); }

    void
    on_handle_contract_violation(std::contracts::contract_violation const& contract_violation) {
        Array<const char*, 3> kind_to_string     = {"pre", "post", "assert"};
        Array<const char*, 4> semantic_to_string = {"ignore",
                                                    "observe",
                                                    "enforce",
                                                    "quick_enforce"};
        Array<const char*, 2> mode_to_string     = {"predicate_false", "evaluation_exception"};

        auto kind     = contract_violation.kind();
        auto semantic = contract_violation.semantic();
        auto mode     = contract_violation.mode();
        auto location = contract_violation.location();

        LOGGER->critical("Contract violation detected (semantic={}, mode={})",
                         semantic_to_string[static_cast<int>(semantic) - 1],
                         mode_to_string[static_cast<int>(mode) - 1]);
        LOGGER->critical("{}:{}:{}: Contract of {} was violated.",
                         location.file_name(),
                         location.line(),
                         location.column(),
                         location.function_name());
        LOGGER->critical("          {} condition: {}",
                         kind_to_string[static_cast<int>(kind) - 1],
                         contract_violation.comment());
        System::instance().panic("Terminating kernel execution...");
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      System
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    DEFINE_ENUM(ModuleSelector, MODULE_SELECTORS, 0x0)

    const Version System::KERNEL_VERSION = {.major       = MAJOR,
                                            .minor       = MINOR,
                                            .patch       = PATCH,
                                            .pre_release = PRERELEASE};

    auto System::instance() -> System& {
        static System instance;
        return instance;
    }

    auto boot_phase3(CPU::StartInfo* start_info) -> int {
        SILENCE_UNUSED(start_info);

        auto& system = System::instance();
        if (system._is_booted) {
            LOGGER->warn("Kernel boot phase 3 has already run! Aborting...");
            return 0;
        }

        Array<ModuleLoader*, 4> module_loaders{
            new DeviceModuleLoader(),
            new VFSModuleLoader(),
            new AppModuleLoader(),
            new SystemCallModuleLoader(),
        };
        for (auto& module_loader : module_loaders) module_loader->load();

        auto* cpu_module = system.get_module<CPU::CPUModule>(ModuleSelector::CPU);
        auto* app_module = system.get_module<App::AppModule>(ModuleSelector::APP);
        LogContext::instance().register_layout(
            "detailed-layout",
            SharedPointer<Layout>(new DetailedLogLayout(cpu_module, app_module)));
        LogContext::instance().set_layout_ref("*", "detailed-layout");

#ifdef RUN_UNIT_TESTS
        LOGGER->info("Run kernel unit tests");
        Test::run_kernel_tests();
#endif

        // Pass control to the system loader
        auto*         vfs_module = system.get_module<VFS::VFSModule>(ModuleSelector::VFS);
        Path          system_loader(SYSTEM_LOADER);
        VFS::NodeInfo dummy;
        VFS::IOStatus st = vfs_module->get_node_info(system_loader, dummy);
        if (st != VFS::IOStatus::FOUND)
            system.panic(R"("{}": System loader not found!)", system_loader.to_string());

        system._is_booted  = true;
        App::LoadStatus ls = app_module->start_system_loader(system_loader, Path::ROOT);
        if (ls != App::LoadStatus::RUNNING) {
            system.panic(R"("{}": System loader start failure! Reason: {})",
                         system_loader.to_string(),
                         ls.to_string());
        }
        return 0;
    }

    void System::boot_phase2(BootInfo boot_info) {
        if (_is_booted) {
            LOGGER->warn("Kernel boot phase 2 has already run! Aborting...");
            return;
        }

        // Kernel boot phase 2 is still running on the implicit Bootstrap Thread
        // using the bootloader resources (mainly the bootloader stack) The main
        // goal here is to init dynamic memory, call global constructors, set up
        // interrupts and scheduling to get a stable kernel
        // Then we will run kernel boot phase 3 on our own resources instead of
        // the bootloaders
        _boot_info = boot_info;

        // It is not possible to use a module loader for the memory module,
        // because loggers are not instantiated yet. Global constructors would
        // need to be called first, but we also want to have dynamic memory in
        // global constructors... so here is a chicken-and-egg problem, hence we
        // manually load the memory module

        // Furthermore, the memory module has to be statically allocated but
        // cannot be defined as a global variable because it will not be
        // initialized (no global constructor call yet), hence we use a little
        // trick. Static local variables live in global scope but are lazily
        // initialized, this means the memory module will be initialized, will
        // not go out of scope once boot phase2 is finished, and the constructor
        // will not be called again when global constructors are called
        static Memory::MemoryModule mem_module;
        if (!mem_module.load(boot_info))
            while (true) CPU::halt();
        _module_registry[0] = &mem_module;

        call_global_constructors();

        LogContext& ctx = LogContext::instance();
        ctx.register_layout("earlyboot", SharedPointer<Layout>(new EarlyBootLayout()));
        ctx.register_target_stream("e9", SharedPointer<TextStream>(new CPU::E9Stream()));
        LOGGER->info("runeKernel v{}", KERNEL_VERSION.to_string());
        LOGGER->info("Loaded by {} - v{}",
                     _boot_info.boot_loader_name,
                     _boot_info.boot_loader_version);
        LOGGER->info("Load module: {:<40} OKAY", (mem_module.get_name() + " ..."));
        mem_module.log_post_load();

        CPUModuleLoader().load();

        _panic_stream = SharedPointer<TextStream>(new CPU::E9Stream);
        CPU::exception_install_panic_stream(_panic_stream);
        init_cpp_runtime_layer(&on_pure_virtual_function_callback,
                               &on_stack_guard_fail_callback,
                               &on_std_terminate,
                               &on_handle_contract_violation);

        auto*          cpu_module    = get_module<CPU::CPUModule>(ModuleSelector::CPU);
        char*          dummy_args[1] = {nullptr}; // NOLINT
        CPU::StartInfo start_info{};
        start_info.argc = 0;
        start_info.argv = dummy_args;
        start_info.main = &boot_phase3;
        cpu_module->schedule_new_thread(
            BOOT_THREAD_NAME,
            &start_info,
            Memory::get_base_page_table_address(),
            CPU::SchedulingPolicy::LOW_LATENCY,
            {.stack_bottom = nullptr, .stack_top = 0x0, .stack_size = 0x0});
        cpu_module->get_scheduler()->mark_as_block_pending();
        cpu_module->get_scheduler()->block(); // Stop Bootstrap Thread and switch to Boot thread
    }

    auto System::get_boot_info() -> BootInfo& { return _boot_info; }

    void System::shutdown() { // NOLINT
        auto*               dm    = get_module<Device::DeviceModule>(ModuleSelector::DEVICE);
        Device::ACPIRequest a_req = Device::ACPIRequest::SHUTDOWN;
        Device::IORequest   req{.m_in_buffer = &a_req, .m_out_buffer = nullptr};
        dm->control_device(dm->device_tree()->get_handle(), req);

        panic("Power off failed. Will spin forever... (Power off manually).");
    }

    void System::reboot() {
        LOGGER->info("Performing reboot. Try ACPI reset...");
        auto*               dm    = get_module<Device::DeviceModule>(ModuleSelector::DEVICE);
        Device::ACPIRequest a_req = Device::ACPIRequest::REBOOT;
        Device::IORequest   req{.m_in_buffer = &a_req, .m_out_buffer = nullptr};
        dm->control_device(dm->device_tree()->get_handle(), req);

        LOGGER->info("Fallback to PS/2 Controller reset...");
        constexpr U8 PS2_COMMAND_PORT  = 0x64;
        constexpr U8 PS2_RESET_COMMAND = 0xFE;
        U8           PS2_STATUS        = 0x02; // Initially assume input buffer full
        while (bit_check(PS2_STATUS, 1)) PS2_STATUS = CPU::in_b(PS2_COMMAND_PORT);
        CPU::out_b(PS2_COMMAND_PORT, PS2_RESET_COMMAND);
        CPU::halt();

        LOGGER->info("Fallback to reset control register...");
        constexpr U16 RESET_CONTROL_REGISTER = 0xCF9;
        constexpr U8  SYSTEM_RESET           = 0x02;
        constexpr U8  RESET_CPU              = 0x04;
        // constexpr U8  FULL_RESET             = 0x08; // Power cycle when set
        CPU::out_b(RESET_CONTROL_REGISTER, SYSTEM_RESET | RESET_CPU);

        panic("All reboot options failed. Will spin forever... (Restart "
              "manually).");
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                  Module Loader
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    // System will allocate the memory module and assign it to position 0 in the
    // module registry, thus we start the module_index at 1
    size_t ModuleLoader::module_index = 1;

    size_t ModuleLoader::plugin_index = 0;

    void ModuleLoader::load_plugin(Plugin* plugin) { // NOLINT
        System::instance()._builtin_plugin_registry[plugin_index++] = plugin;
        String plugin_info = plugin->get_info().to_string() + " ...";
        if (!plugin->load()) {
            LOGGER->critical("Load plugin: {:<40} FAILED", plugin_info);
            while (true) CPU::halt();
        }
        LOGGER->info("Load plugin: {:<40} OKAY", plugin_info);
    }

    void ModuleLoader::load() {
        System& system = System::instance();
        Module* module = alloc_module();

        system._module_registry[module_index++] = module;

        on_pre_load(module);

        String module_name = module->get_name() + " ...";
        if (!module->load(system._boot_info)) {
            LOGGER->critical("Load module: {:<40} FAILED", module_name);
            while (true) CPU::halt();
        }
        LOGGER->info("Load module: {:<40} OKAY", module_name);

        on_post_load(module);
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                  CPU Module Loader
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    auto CPUModuleLoader::alloc_module() -> Module* { return new CPU::CPUModule(); }

    void CPUModuleLoader::on_pre_load(Module* module) {
        SILENCE_UNUSED(module);
        load_plugin(new BuiltInPlugin::_8259PICDriverPlugin());
        load_plugin(new BuiltInPlugin::PITDriverPlugin());
    }

    void CPUModuleLoader::on_post_load(Module* module) { SILENCE_UNUSED(module); }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                  Device Module Loader
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    auto DeviceModuleLoader::alloc_module() -> Module* { return new Device::DeviceModule; }

    void DeviceModuleLoader::on_pre_load(Module* module) {
        SILENCE_UNUSED(module);
        load_plugin(new BuiltInPlugin::ACPIDriverPlugin());
        load_plugin(new BuiltInPlugin::PCIDriverPlugin());
        load_plugin(new BuiltInPlugin::PS2KeyboardDriverPlugin());
        load_plugin(new BuiltInPlugin::AHCIDriverPlugin());
    }

    void DeviceModuleLoader::on_post_load(Module* module) { SILENCE_UNUSED(module); }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                  VFS Module Loader
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    auto VFSModuleLoader::alloc_module() -> Module* { return new VFS::VFSModule; }

    void VFSModuleLoader::on_pre_load(Module* module) {
        SILENCE_UNUSED(module)
        load_plugin(new BuiltInPlugin::FATDriverPlugin());
    }

    void VFSModuleLoader::on_post_load(Module* module) {
        SILENCE_UNUSED(module);
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                  App Module Loader
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    auto AppModuleLoader::alloc_module() -> Module* { return new App::AppModule(); }

    void AppModuleLoader::on_pre_load(Module* module) { SILENCE_UNUSED(module) }

    void AppModuleLoader::on_post_load(Module* module) {
        SILENCE_UNUSED(module);
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                              SystemCall Module Loader
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    auto SystemCallModuleLoader::alloc_module() -> Module* {
        return new SystemCall::SystemCallModule();
    }

    void SystemCallModuleLoader::on_pre_load(Module* module) { SILENCE_UNUSED(module) }

    void SystemCallModuleLoader::on_post_load(Module* module) {
        SILENCE_UNUSED(module);
    }
} // namespace Rune
