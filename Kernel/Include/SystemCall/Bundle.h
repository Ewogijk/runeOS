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

#ifndef RUNEOS_BUNDLE_H
#define RUNEOS_BUNDLE_H

#include <KernelRuntime/Subsystem.h>

#include <SystemCall/Definition.h>
#include <SystemCall/KernelGuardian.h>

namespace Rune::SystemCall {

    /**
     * @brief A system call bundle is a list of system calls, the system call context and a range of ID's to be used for
     *          system calls.
     *
     */
    struct Bundle {
        String                 name                    = "";
        LinkedList<Definition> system_call_definitions = LinkedList<Definition>();
    };

    /**
     * @brief Get the list of all natively supported system call bundles.
     * @param k_guard      Kernel guardian.
     * @param k_subsys_reg Kernel subsystem registry.
     * @return System call bundles.
     */
    LinkedList<Bundle> system_call_get_native_bundles(KernelGuardian* k_guard, const SubsystemRegistry& k_subsys_reg);
} // namespace Rune::SystemCall

#endif // RUNEOS_BUNDLE_H
