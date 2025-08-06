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

#include <SystemCall/SystemCallSubsystem.h>

#include <SystemCall/Bundle.h>

#include <Memory/MemorySubsystem.h>


namespace Rune::SystemCall {
    constexpr char const* FILE = "SystemCall";


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                          Subsystem Overrides
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    SystemCallSubsystem::SystemCallSubsystem() : Subsystem(), _k_guard(), _system_call_table_fmt() {

    }


    String SystemCallSubsystem::get_name() const {
        return "SystemCall";
    }


    void SystemCallSubsystem::set_logger(SharedPointer<Logger> logger) {
        if (!_logger)
            _logger = move(logger);
    }


    bool SystemCallSubsystem::start(const BootLoaderInfo& boot_info, const SubsystemRegistry& k_subsys_reg) {
        SILENCE_UNUSED(boot_info)
        SILENCE_UNUSED(k_subsys_reg)
        // Configure system call table formatter
        LinkedList<Column<SystemCallInfo>> sct_cols;
        sct_cols.add_back(Column<SystemCallInfo>::make_handle_column_table(26));
        sct_cols.add_back(
                {
                        "Requested",
                        10,
                        [](SystemCallInfo* info) {
                            return String::format("{}", info->requested);
                        }
                }
        );
        _system_call_table_fmt.configure("System Call", sct_cols);

        auto* mem_subsys = k_subsys_reg.get_as<Memory::MemorySubsystem>(KernelSubsystem::MEMORY);
        auto user_space_end = mem_subsys->get_virtual_memory_manager()->get_user_space_end();
        _logger->debug(
                FILE,
                "Kernel memory start: {:0=#16x}",
                user_space_end
        );
        _k_guard.set_kernel_memory_start(user_space_end);
        system_call_init(_logger, &_k_guard);


        LinkedList<Bundle> native_sys_calls = system_call_get_native_bundles(&_k_guard, k_subsys_reg);
        for (auto& bundle: native_sys_calls) {
            _logger->debug(FILE, R"(Installing the "{}" system call bundle.)", bundle.name);
            for (auto& def: bundle.system_call_definitions)
                if (!system_call_install(def)) {
                    _logger->error(
                            FILE,
                            R"(Failed to install system call "{}-{}" of bundle {})",
                            def.ID,
                            def.name,
                            bundle.name
                    );
                    return false;
                }
        }

        return true;
    }


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          System Call API
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    LinkedList<SystemCallInfo> SystemCallSubsystem::get_system_call_table() const {
        return system_call_get_table();
    }


    void SystemCallSubsystem::dump_system_call_table(const SharedPointer<TextStream>& stream) const {
        LinkedList<SystemCallInfo> sys_call_table = system_call_get_table();
        auto                       it             = sys_call_table.begin();
        _system_call_table_fmt.dump(
                stream, [&it] {
                    SystemCallInfo* info = nullptr;
                    if (it.has_next()) {
                        info = &(*it);
                        ++it;
                    }
                    return info;
                }
        );
    }


    bool SystemCallSubsystem::install_system_call(const Definition& system_call_definition) {
        return system_call_install(system_call_definition);
    }


    bool SystemCallSubsystem::uninstall_system_call(U16 system_call_id) {
        return system_call_uninstall(system_call_id);
    }
}