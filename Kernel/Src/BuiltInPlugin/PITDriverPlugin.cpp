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

#include <KRE/System/System.h>

#include <CPU/CPUModule.h>

#include <CPU/Time/PIT.h>

namespace Rune::BuiltInPlugin {
    const PluginInfo PIT_INFO = {
        .name    = "PIT",
        .vendor  = "Ewogijk",
        .version = {.major = 1, .minor = 0, .patch = 0, .pre_release = ""}
    };

    auto PITDriverPlugin::get_info() const -> PluginInfo { return PIT_INFO; }

    auto PITDriverPlugin::load() -> bool {
        auto* cs = System::instance().get_module<CPU::CPUModule>(ModuleSelector::CPU);
        cs->install_timer_driver(UniquePointer<CPU::Timer>(new CPU::PIT()));

        return true;
    }
} // namespace Rune::BuiltInPlugin