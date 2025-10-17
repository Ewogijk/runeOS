
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

#ifndef RUNEOS_SYSTEM_H
#define RUNEOS_SYSTEM_H

#include <Ember/Enum.h>

#include <KRE/Memory.h>
#include <KRE/Stream.h>

#include <KRE/Collections/Array.h>

#include <KRE/System/Module.h>
#include <KRE/System/Plugin.h>

#include <CPU/CPU.h>

namespace Rune {

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      System
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * List of all kernel modules.
     */
#define MODULE_SELECTORS(X)                                                                        \
    X(ModuleSelector, MEMORY, 0x1)                                                                 \
    X(ModuleSelector, CPU, 0x2)                                                                    \
    X(ModuleSelector, DEVICE, 0x3)                                                                 \
    X(ModuleSelector, VFS, 0x4)                                                                    \
    X(ModuleSelector, APP, 0x5)                                                                    \
    X(ModuleSelector, SYSTEMCALL, 0x6)

    DECLARE_ENUM(ModuleSelector, MODULE_SELECTORS, 0x0) // NOLINT

    /**
     * A module loader is responsible to instantiate a kernel module and to prepare and finish up
     * module loading.
     */
    class ModuleLoader;

    /**
     * This is the central interface to access the kernel modules and for functionality affecting
     * the whole system. It is designed as singleton that can be retrieved anywhere in the kernel.
     *
     * The system additionally acts as an entry point during the kernel boot process. The boot
     * process is divided into 3 phases that involve multiple steps:
     * 1. Boot phase 1 is the architecture dependent part of the boot because it interfaces directly
     *    with the bootloader. It is responsible set up the boot core and pass the bootloader
     *    information in a @code BootInfo@endcode object to boot phase 2. Phase 1 steps:
     *    1. Initialize the boot core.
     *    2. Create the physical memory map.
     *    3. Gather frame buffer information.
     * 2. Boot phase 2 is architecture independent but still unstable as it relies on bootloader
     *    resources e.g. stack memory. Thus, it is responsible to set up the kernel to run on its
     *    own resources. Boot phase 2 steps:
     *    1. Load the memory module.
     *    2. Call global constructors.
     *    3. Initialize logging.
     *    4. Load the CPU module.
     *    5. Setup panic mode and C++ RTS.
     *    6. Execute boot phase 3 on the "Boot" thread.
     * 3. Boot phase 3 is the last and most stable boot phase as most low level initialization has
     *    been done. It has the job to load the other higher level kernel modules and jump to user
     *    mode. Phase 3 steps are:
     *    1. Load the other kernel modules
     *    2. Execute the Init app in user mode.
     *    3. Shutdown the system after the Init app finished.
     *
     */
    class System {
        /**
         * Number of kernel modules.
         */
        static constexpr U8 MODULE_COUNT = 6;
        /**
         * Number of plugins that come prebuilt with the kernel.
         */
        static constexpr U8 BUILTIN_PLUGIN_COUNT = 4;
        /**
         * Name of the boot thread that runs boot phase 3.
         */
        static constexpr char const* BOOT_THREAD_NAME = "Boot";
        /**
         * Kernel version as provided by Build.h.
         */
        static const Version KERNEL_VERSION;

        SharedPointer<TextStream>            _panic_stream;
        Array<Module*, MODULE_COUNT>         _module_registry{};
        Array<Plugin*, BUILTIN_PLUGIN_COUNT> _builtin_plugin_registry{};
        BootInfo                             _boot_info;

        /**
         * The flag controls if boot phase 2 can be executed.
         *
         * Boot phase 1 triggers phase 2 and at the end of phase 3 this flag will be set to true to
         * disallow calls to boot_phase2() after boot has finished.
         */
        bool _is_booted = false;

        System() = default;

        friend class ModuleLoader;

      public:
        ~System() = default;

        System(const System&)                    = delete;
        System(System&&)                         = delete;
        auto operator=(const System&) -> System& = delete;
        auto operator=(System&&) -> System&      = delete;

        /**
         *
         * @return Get the instance of the system.
         */
        static auto instance() -> System&;

        /**
         * Get a pointer to the requested kernel module.
         *
         * For each kernel module there is a module selector. ModuleType and mod_sel must match
         * since each kernel module is stored as Module* and will be cast to ModuleType*.
         *
         * Important: This means mod_sel and ModuleType must match, otherwise the behavior of the
         * returned kernel module instance is undefined.
         *
         * @tparam ModuleType Type of the kernel module.
         * @param mod_sel The selected kernel module to be returned.
         * @return The instance of the selected kernel module cast to ModuleType.
         */
        template <typename ModuleType>
        auto get_module(ModuleSelector mod_sel) -> ModuleType* {
            return reinterpret_cast<ModuleType*>(_module_registry[mod_sel.to_value() - 1]);
        }

        /**
         * Run boot phase 2.
         *
         * Note: This function will be called at the end of boot phase 1 and will be disabled at the
         * end of boot phase 3.
         *
         * @param boot_info Boot info provided by boot phase 1.
         */
        void boot_phase2(BootInfo boot_info);

        /**
         * Run boot phase 3.
         *
         * Note: This function will be scheduled in a new thread at then end of boot phase 2 and
         * will be disabled at the end of boot phase 3.
         * @param start_info Thread start info.
         * @return Undefined. The function runs until system shutdown.
         */
        friend auto boot_phase3(CPU::StartInfo* start_info) -> int;
    };

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      Module Loader
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    class ModuleLoader {
        static size_t module_index;
        static size_t plugin_index;

        /**
         *
         * @return An instance of the kernel module.
         */
        virtual auto alloc_module() -> Module* = 0;

        /**
         * Run system configuration required before the kernel module can be loaded, e.g. load
         * plugins.
         *
         * This function will be called before a kernel module is loaded.
         * @param module Instance of the kernel module.
         */
        virtual void on_pre_load(Module* module) = 0;

        /**
         * Run system configuration that requires the kernel module to be loaded.
         *
         * This function will be called after a kernel module was loaded.
         * @param module Instance of the kernel module.
         */
        virtual void on_post_load(Module* module) = 0;

      protected:
        /**
         * Register and load a kernel plugin.
         * @param plugin A kernel plugin.
         */
        void load_plugin(Plugin* plugin);

      public:
        virtual ~ModuleLoader() = default;

        /**
         * Instantiate and load a kernel module.
         *
         * Loading consists of the following steps:
         *  1. Call alloc_module() to instantiate the kernel module.
         *  2. Register the kernel module in the system.
         *  3. Call on_pre_load().
         *  4. Try to load the kernel module, if loading fails the system will be halted.
         *  5. Call on_post_load()
         */
        void load();
    };

    /**
     * Loader of the CPU kernel module.
     */
    class CPUModuleLoader : public ModuleLoader {
        auto alloc_module() -> Module* override;
        void on_pre_load(Module* module) override;
        void on_post_load(Module* module) override;
    };

    /**
     * Loader of the Device kernel module.
     */
    class DeviceModuleLoader : public ModuleLoader {
        auto alloc_module() -> Module* override;
        void on_pre_load(Module* module) override;
        void on_post_load(Module* module) override;
    };

    /**
     * Loader of the VFS kernel module.
     */
    class VFSModuleLoader : public ModuleLoader {
        auto alloc_module() -> Module* override;
        void on_pre_load(Module* module) override;
        void on_post_load(Module* module) override;
    };

    /**
     * Loader of the App kernel module.
     */
    class AppModuleLoader : public ModuleLoader {
        auto alloc_module() -> Module* override;
        void on_pre_load(Module* module) override;
        void on_post_load(Module* module) override;
    };

    /**
     * Loader of the SystemCall kernel module.
     */
    class SystemCallModuleLoader : public ModuleLoader {
        auto alloc_module() -> Module* override;
        void on_pre_load(Module* module) override;
        void on_post_load(Module* module) override;
    };
} // namespace Rune

#endif // RUNEOS_SYSTEM_H
