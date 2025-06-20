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

#ifndef RUNEOS_SYSTEMCALL_H
#define RUNEOS_SYSTEMCALL_H


#include <Hammer/Definitions.h>

#include <LibK/Logging.h>

#include <SystemCall/Definition.h>
#include <SystemCall/KernelGuardian.h>


namespace Rune::SystemCall {
    /**
     * @brief General information about an installed system call.
     */
    struct SystemCallInfo {
        U16    handle    = 0;
        String name      = "";
        U64    requested = 0;
    };


    /**
     * @brief Initialize the system call infrastructure, upon successful initialization the kernel can handle system
     *          calls from user mode applications.
     *
     * @return True: The kernel is ready to handle system calls, False: System calls cannot be used.
     */
    bool system_call_init(SharedPointer<LibK::Logger> logger, KernelGuardian* k_guard);


    /**
     * @brief Get all installed system calls.
     *
     * Unused system calls will not be represented in the system call table.
     *
     * @return A list of all installed system calls.
     */
    LinkedList <SystemCallInfo> system_call_get_table();


    /**
     * @brief Try to install the system call definition.
     *
     * The installation will fail, if another system call with the same ID is already installed.
     *
     * <p>
     *  Note: While the name must no be unique, ideally it should be unique so it is easier to identify.
     * </p>
     *
     * @param sys_call_def
     * @return True: The system call is installed, False: The requested system call is already in use, the handler
     *          was not installed.
     */
    bool system_call_install(const Definition& sys_call_def);


    /**
     * @brief Try to uninstall the system call with the given ID.
     * @param systemCallID ID of the system call.
     * @return True: The system call was uninstalled, False: No system call with the ID was installed or the ID is
     *          invalid (systemCallID >= SystemCallLimit).
     */
    bool system_call_uninstall(U16 systemCallID);
}

#endif //RUNEOS_SYSTEMCALL_H
