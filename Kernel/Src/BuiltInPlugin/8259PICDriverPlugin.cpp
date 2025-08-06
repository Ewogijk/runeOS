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

#include <BuiltInPlugin/8259PICDriverPlugin.h>

#include <CPU/CPUSubsystem.h>

#include <CPU/Interrupt/8259PIC.h>


namespace Rune::BuiltInPlugin {
    PluginInfo _8259_PIC_INFO = {
            "8259 PIC",
            "Ewogijk",
            {
                    1,
                    0,
                    0,
                    ""
            }
    };


    PluginInfo _8259PICDriverPlugin::get_info() const {
        return _8259_PIC_INFO;
    }


    bool _8259PICDriverPlugin::start(const SubsystemRegistry &ks_registry) {
        auto* cs = ks_registry.get_as<CPU::CPUSubsystem>(KernelSubsystem::CPU);
        cs->install_pic_driver(UniquePointer<CPU::PICDriver>(new CPU::_8259PIC()));

        return true;
    }
}

