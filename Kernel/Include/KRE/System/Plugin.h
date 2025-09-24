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

#ifndef RUNEOS_PLUGIN_H
#define RUNEOS_PLUGIN_H

#include <KRE/String.h>
#include <KRE/System/Subsystem.h>

namespace Rune {

    /**
     * Information about the Kernel Extension like it's unique name, vendor and version information.
     */
    struct PluginInfo {
        /**
         * Unique Kernel Extension name.
         */
        const String name;

        /**
         * Creator of the kernel extension.
         */
        const String vendor;

        /**
         * @brief The version of the kernel extension.
         */
        Version version = {0, 0, 0, ""};
    };

    /**
     * Adds additional functionality to a Kernel Subsystem. E.g. A device driver or basically any
     * software component.
     */
    class Plugin {

      public:
        virtual ~Plugin() = default;

        /**
         *
         * @return Info about the kernel extension e.g. its name or vendor.
         */
        [[nodiscard]]
        virtual PluginInfo get_info() const = 0;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                      Functions
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        /**
         * Start the Kernel Extension.
         *
         * @return True: The extension has started, False: If not.
         */
        [[nodiscard]]
        virtual bool start(const SubsystemRegistry& ks_registry) = 0;
    };
} // namespace Rune

#endif // RUNEOS_PLUGIN_H
