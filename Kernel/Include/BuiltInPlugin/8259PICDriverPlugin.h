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

#ifndef RUNEOS_8259PICDRIVERPLUGIN_H
#define RUNEOS_8259PICDRIVERPLUGIN_H


#include <KernelRuntime/Plugin.h>


namespace Rune::BuiltInPlugin {

    class _8259PICDriverPlugin : public Plugin {
    public:

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                      Constructors&Destructors
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


        _8259PICDriverPlugin() = default;


        ~_8259PICDriverPlugin() override = default;


        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                  Kernel Extension Overrides
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


        [[nodiscard]] PluginInfo get_info() const override;


        [[nodiscard]] bool start(const SubsystemRegistry &ks_registry) override;
    };
}

#endif //RUNEOS_8259PICDRIVERPLUGIN_H