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

#include <BuiltInPlugin/AHCIDriverPlugin.h>

#include <Memory/MemorySubsystem.h>

#include <CPU/CPUSubsystem.h>

#include <Device/DeviceSubsystem.h>
#include <Device/AHCI/AHCI.h>


namespace Rune::BuiltInPlugin {
    PluginInfo AHCI_INFO = {
            "AHCI",
            "Ewogijk",
            {
                    1,
                    0,
                    0,
                    ""
            }
    };


    PluginInfo AHCIDriverPlugin::get_info() const {
        return AHCI_INFO;
    }


    bool AHCIDriverPlugin::start(const SubsystemRegistry& ks_registry) {
        auto* ms = ks_registry.get_as<Memory::MemorySubsystem>(KernelSubsystem::MEMORY);
        auto* cs = ks_registry.get_as<CPU::CPUSubsystem>(KernelSubsystem::CPU);
        auto* ds = ks_registry.get_as<Device::DeviceSubsystem>(KernelSubsystem::DEVICE);
        ds->set_ahci_driver(
                UniquePointer(
                        new Device::AHCIDriver(
                                ms->get_heap(),
                                cs->get_system_timer(),
                                ds->get_logger()
                        ))
        );
        return true;
    }
}