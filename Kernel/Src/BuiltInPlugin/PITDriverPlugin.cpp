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

#include <BuiltInPlugin/PITDriverPlugin.h>

#include <CPU/CPUSubsystem.h>

#include <CPU/Time/PIT.h>


namespace Rune::BuiltInPlugin {
    LibK::PluginInfo PIT_INFO = {
            "PIT",
            "Ewogijk",
            {
                    1,
                    0,
                    0,
                    ""
            }
    };


    LibK::PluginInfo PITDriverPlugin::get_info() const {
        return PIT_INFO;
    }


    bool PITDriverPlugin::start(const LibK::SubsystemRegistry& ks_registry) {
        auto* cs = ks_registry.get_as<CPU::Subsystem>(LibK::KernelSubsystem::CPU);
        cs->install_timer_driver(UniquePointer<CPU::Timer>(new CPU::PIT()));

        return true;
    }
}