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

#include <Device/DeviceModule.h>

#include <Device/PCI.h>

namespace Rune::Device {
    const SharedPointer<Logger> LOGGER =
        LogContext::instance().get_logger("Device.DeviceSubsystem");

    DeviceModule::DeviceModule() : _ahci_driver(nullptr), _keyboard(new PS2Keyboard()) {}

    auto DeviceModule::get_name() const -> String { return "Device"; }

    auto DeviceModule::load(const BootInfo& boot_info) -> bool {
        SILENCE_UNUSED(boot_info)
        PCI::discover_devices(*_ahci_driver);

        _keyboard->start();
        return true;
    }

    void DeviceModule::set_ahci_driver(UniquePointer<Device::AHCIDriver> ahci_driver) {
        _ahci_driver = move(ahci_driver);
    }

    auto DeviceModule::get_ahci_driver() -> AHCIDriver* { return _ahci_driver.get(); }

    auto DeviceModule::get_keyboard() -> SharedPointer<VirtualKeyboard> { return _keyboard; }
} // namespace Rune::Device