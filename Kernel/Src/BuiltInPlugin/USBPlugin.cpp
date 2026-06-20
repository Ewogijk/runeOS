//  Copyright 2025 Ewogijk
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include <BuiltInPlugin/USBPlugin.h>

#include <KRE/System/System.h>

#include <Device/DeviceModule.h>
#include <Device/USB/XHCI/XHCI.h>

namespace Rune::BuiltInPlugin {
    const PluginInfo PS2KEYBOARD_INFO = {
        .name    = "PS2 Keyboard",
        .vendor  = "Ewogijk",
        .version = {.major = 1, .minor = 0, .patch = 0, .pre_release = ""}
    };

    auto USBPlugin::get_info() const -> PluginInfo { return PS2KEYBOARD_INFO; }

    auto USBPlugin::load() -> bool {
        auto* ds = System::instance().get_module<Device::DeviceModule>(ModuleSelector::DEVICE);
        return ds->register_device_driver(
            SharedPointer<Device::Driver>(new Device::USB::XHCIDriver()));
    }
} // namespace Rune::BuiltInPlugin
