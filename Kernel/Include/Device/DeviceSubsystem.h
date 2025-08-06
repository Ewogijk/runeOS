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

#ifndef RUNEOS_DEVICESUBSYSTEM_H
#define RUNEOS_DEVICESUBSYSTEM_H


#include <KernelRuntime/Subsystem.h>

#include <Memory/MemorySubsystem.h>

#include <CPU/CPUSubsystem.h>

#include <Device/AHCI/AHCI.h>
#include <Device/Keyboard/PS2Keyboard.h>


namespace Rune::Device {
    class DeviceSubsystem : public Subsystem {
        UniquePointer<AHCIDriver>      _ahci_driver;
        SharedPointer<VirtualKeyboard> _keyboard;

    public:

        explicit DeviceSubsystem();


        ~DeviceSubsystem() override = default;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                      KernelSubsystem Overrides
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


        [[nodiscard]] String get_name() const override;


        bool start(const BootLoaderInfo& boot_info, const SubsystemRegistry& k_subsys_reg) override;


        void set_logger(SharedPointer<Logger> logger) override;


        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                  Device specific functions
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        void set_ahci_driver(UniquePointer<AHCIDriver> ahci_driver);


        AHCIDriver& get_ahic_driver();


        SharedPointer<VirtualKeyboard> get_keyboard();
    };
}


#endif //RUNEOS_DEVICESUBSYSTEM_H
