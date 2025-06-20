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

#ifndef RUNEOS_SYSTEMCALLSUBSYSTEM_H
#define RUNEOS_SYSTEMCALLSUBSYSTEM_H


#include <LibK/Subsystem.h>

#include <CPU/CPU.h>

#include <SystemCall/SystemCall.h>
#include <SystemCall/KernelGuardian.h>


namespace Rune::SystemCall {


    class Subsystem : public LibK::Subsystem {
        KernelGuardian _k_guard;

        LibK::TableFormatter<SystemCallInfo> _system_call_table_fmt;

    public:

        Subsystem();


        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                      Kernel Subsystem Overrides
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


        [[nodiscard]] String get_name() const override;


        bool start(const LibK::BootLoaderInfo& boot_info, const LibK::SubsystemRegistry& k_subsys_reg) override;


        void set_logger(SharedPointer<LibK::Logger> logger) override;


        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          System Call API
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


        /**
         * @brief Lightweight wrapper around the "SystemCallGetTable" function.
         */
        [[nodiscard]] LinkedList<SystemCallInfo> get_system_call_table() const;


        /**
         * @brief Dump the system call table to the stream.
         * @param stream
         */
        void dump_system_call_table(const SharedPointer<LibK::TextStream>& stream) const;


        /**
         * @brief Lightweight wrapper around the "SystemCallInstall" function.
         */
        bool install_system_call(const Definition& system_call_definition);


        /**
         * @brief Lightweight wrapper around the "SystemCallUninstall" function.
         */
        bool uninstall_system_call(U16 system_call_id);
    };
}

#endif //RUNEOS_SYSTEMCALLSUBSYSTEM_H
