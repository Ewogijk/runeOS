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

#ifndef RUNEOS_DEVICEMODULE_H
#define RUNEOS_DEVICEMODULE_H

#include <KRE/System/Module.h>

#include <Memory/MemoryModule.h>

#include <CPU/CPUModule.h>

#include <Device/AHCI/AHCI.h>
#include <Device/Keyboard/PS2Keyboard.h>

namespace Rune::Device {
    class DeviceModule : public Module {
        UniquePointer<AHCIDriver>      _ahci_driver;
        SharedPointer<VirtualKeyboard> _keyboard;

      public:
        explicit DeviceModule();

        ~DeviceModule() override = default;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                      KernelSubsystem Overrides
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        [[nodiscard]]
        auto get_name() const -> String override;

        auto load(const BootInfo& boot_info) -> bool override;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                  Device specific functions
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        void set_ahci_driver(UniquePointer<AHCIDriver> ahci_driver);

        auto get_ahci_driver() -> AHCIDriver*;

        auto get_keyboard() -> SharedPointer<VirtualKeyboard>;
    };
} // namespace Rune::Device

#endif // RUNEOS_DEVICEMODULE_H
