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

#include <Device/DeviceSubsystem.h>

#include <Device/PCI.h>

namespace Rune::Device {
    const SharedPointer<Logger> LOGGER = LogContext::instance().get_logger("DeviceSubsystem");

    DeviceSubsystem::DeviceSubsystem()
        : Subsystem(),
          _ahci_driver(nullptr),
          _keyboard(new PS2Keyboard()) {}

    String DeviceSubsystem::get_name() const { return "Device"; }

    bool DeviceSubsystem::start(const BootLoaderInfo&    boot_info,
                                const SubsystemRegistry& k_subsys_reg) {
        SILENCE_UNUSED(boot_info)
        SILENCE_UNUSED(k_subsys_reg)
        PCI::discover_devices(*_ahci_driver);

        _keyboard->start();
        return true;
    }

    void DeviceSubsystem::set_logger(SharedPointer<LegacyLogger> logger) {
        if (!_logger) _logger = logger;
    }

    void DeviceSubsystem::set_ahci_driver(UniquePointer<Device::AHCIDriver> ahci_driver) {
        _ahci_driver = move(ahci_driver);
    }

    AHCIDriver& DeviceSubsystem::get_ahic_driver() { return *_ahci_driver; }

    SharedPointer<VirtualKeyboard> DeviceSubsystem::get_keyboard() { return _keyboard; }
} // namespace Rune::Device