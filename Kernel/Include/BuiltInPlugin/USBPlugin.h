
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

#ifndef RUNEOS_USBPLUGIN_H
#define RUNEOS_USBPLUGIN_H

#include <KRE/System/Plugin.h>

namespace Rune::BuiltInPlugin {

    class USBPlugin : public Plugin {
    public:
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                      Constructors&Destructors
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        USBPlugin() = default;

        ~USBPlugin() override = default;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                  Kernel Extension Overrides
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        [[nodiscard]] auto get_info() const -> PluginInfo override;

        [[nodiscard]] auto load() -> bool override;
    };
} // namespace Rune::BuiltInPlugin

#endif // RUNEOS_USBPLUGIN_H
