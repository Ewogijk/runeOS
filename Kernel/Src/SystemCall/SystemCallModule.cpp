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

#include <SystemCall/SystemCallModule.h>

#include <KRE/System/System.h>

#include <SystemCall/Bundle.h>

#include <Memory/MemoryModule.h>

namespace Rune::SystemCall {
    const SharedPointer<Logger> LOGGER =
        LogContext::instance().get_logger("SystemCall.SystemCallSubsystem");

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Subsystem Overrides
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    SystemCallModule::SystemCallModule() = default;

    auto SystemCallModule::get_name() const -> String { return "SystemCall"; }

    auto SystemCallModule::load(const BootInfo& boot_info) -> bool {
        SILENCE_UNUSED(boot_info)
        System& system = System::instance();

        auto* mem_module     = system.get_module<Memory::MemoryModule>(ModuleSelector::MEMORY);
        auto  user_space_end = mem_module->get_virtual_memory_manager()->get_user_space_end();
        LOGGER->debug("Kernel memory start: {:0=#16x}", user_space_end);
        _k_guard.set_kernel_memory_start(user_space_end);
        system_call_init(&_k_guard);

        LinkedList<Bundle> native_sys_calls = system_call_get_native_bundles(&_k_guard);
        for (auto& bundle : native_sys_calls) {
            LOGGER->debug(R"(Installing the "{}" system call bundle.)", bundle.name);
            for (auto& def : bundle.system_call_definitions)
                if (!system_call_install(def)) {
                    LOGGER->error(R"(Failed to install system call "{}-{}" of bundle {})",
                                  def.ID,
                                  def.name,
                                  bundle.name);
                    return false;
                }
        }

        return true;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          System Call API
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    // NOLINTBEGIN Should be member functions for consistent API
    auto SystemCallModule::get_system_call_table() const -> LinkedList<SystemCallInfo> {
        return system_call_get_table();
    }

    void SystemCallModule::dump_system_call_table(const SharedPointer<TextStream>& stream) const {
        Table<SystemCallInfo, 2>::make_table([](const SystemCallInfo& sci) -> Array<String, 2> {
            return {String::format("{}-{}", sci.handle, sci.name),
                    String::format("{}", sci.requested)};
        })
            .with_headers({"ID-Name", "Requested"})
            .with_data(system_call_get_table())
            .print(stream);
    }

    auto SystemCallModule::install_system_call(const Definition& system_call_definition) -> bool {
        return system_call_install(system_call_definition);
    }

    auto SystemCallModule::uninstall_system_call(U16 system_call_id) -> bool {
        return system_call_uninstall(system_call_id);
    }
    // NOLINTEND
} // namespace Rune::SystemCall