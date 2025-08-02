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

#ifndef RUNEOS_DEFINITION_H
#define RUNEOS_DEFINITION_H


#include <Ember/Definitions.h>
#include <Ember/StatusCode.h>
#include <Ember/SystemCallID.h>

#include <Hammer/String.h>
#include <Hammer/Utility.h>


namespace Rune::SystemCall {
    /**
     * @brief Function definition of a system call handler.
     */
    using Handler = Function<Ember::StatusCode(
        void*,
        Ember::SystemCallArgument,
        Ember::SystemCallArgument,
        Ember::SystemCallArgument,
        Ember::SystemCallArgument,
        Ember::SystemCallArgument,
        Ember::SystemCallArgument)>;


    /**
     * @brief Convenience constant that defines a system call handler that simply returns -1.
     */
    inline const Handler SYS_CALL_HANDLER_NONE = [](
        const void*                     sys_call_ctx,
        const Ember::SystemCallArgument arg1,
        const Ember::SystemCallArgument arg2,
        const Ember::SystemCallArgument arg3,
        const Ember::SystemCallArgument arg4,
        const Ember::SystemCallArgument arg5,
        const Ember::SystemCallArgument arg6
    ) {
        SILENCE_UNUSED(sys_call_ctx)
        SILENCE_UNUSED(arg1)
        SILENCE_UNUSED(arg2)
        SILENCE_UNUSED(arg3)
        SILENCE_UNUSED(arg4)
        SILENCE_UNUSED(arg5)
        SILENCE_UNUSED(arg6)
        return -1;
    };


    /**
     * @brief Defines a system call to the kernel.
     *
     * <p>
     *  Each system call has a unique ID and has an associated system call context which contains all the information
     *  needed for the system call to run.
     * </p>
     * <p>
     *  System calls are exported by each subsystem and will at some later point imported by the "SystemCall" subsystem
     *  which integrates them into the architecture dependant system call architecture.
     * </p>
     */
    struct Definition {
        Ember::SystemCallID ID           = 0;
        String              name             = "";
        Handler             sys_call_handler = SYS_CALL_HANDLER_NONE;
        void*               context          = nullptr;
    };


    /**
     * @brief Define a system call that takes all six user application arguments.
     * @param sys_call_ctx
     * @param name
     * @param handler
     * @param context
     * @return
     */
    Definition define6(
        Ember::SystemCallID sys_call_ctx,
        const String&       name,
        const Function<S64(
            void*,
            Ember::SystemCallArgument,
            Ember::SystemCallArgument,
            Ember::SystemCallArgument,
            Ember::SystemCallArgument,
            Ember::SystemCallArgument,
            Ember::SystemCallArgument)>& handler,
        void*                            context
    );


    /**
     * @brief Define a system call that takes only five user application arguments.
     * @param ID
     * @param name
     * @param handler
     * @param context
     * @return
     */
    Definition define5(
        Ember::SystemCallID ID,
        const String&       name,
        const Function<S64(
            void*,
            Ember::SystemCallArgument,
            Ember::SystemCallArgument,
            Ember::SystemCallArgument,
            Ember::SystemCallArgument,
            Ember::SystemCallArgument)>& handler,
        void*                            context
    );


    /**
     * @brief Define a system call that takes only four user application arguments.
     * @param ID
     * @param name
     * @param handler
     * @param context
     * @return
     */
    Definition define4(
        Ember::SystemCallID ID,
        const String&       name,
        const Function<S64(
            void*,
            Ember::SystemCallArgument,
            Ember::SystemCallArgument,
            Ember::SystemCallArgument,
            Ember::SystemCallArgument)>& handler,
        void*                            context
    );


    /**
     * @brief Define a system call that takes only three user application arguments.
     * @param ID
     * @param name
     * @param handler
     * @param context
     * @return
     */
    Definition define3(
        Ember::SystemCallID ID,
        const String&       name,
        const Function<S64(
            void*,
            Ember::SystemCallArgument,
            Ember::SystemCallArgument,
            Ember::SystemCallArgument)>& handler,
        void*                            context
    );


    /**
     * @brief Define a system call that takes only two user application arguments.
     * @param ID
     * @param name
     * @param handler
     * @param context
     * @return
     */
    Definition define2(
        Ember::SystemCallID                                                               ID,
        const String&                                                                     name,
        const Function<S64(void*, Ember::SystemCallArgument, Ember::SystemCallArgument)>& handler,
        void*                                                                             context
    );


    /**
     * @brief Define a system call that takes only one user application argument.
     * @param ID
     * @param name
     * @param handler
     * @param context
     * @return
     */
    Definition define1(
        Ember::SystemCallID                                    ID,
        const String&                                          name,
        const Function<S64(void*, Ember::SystemCallArgument)>& handler,
        void*                                                  context
    );


    /**
     * @brief Define a system call that takes only one user application argument.
     * @param ID
     * @param name
     * @param handler
     * @param context
     * @return
     */
    Definition define0(
        Ember::SystemCallID  ID,
        const String&        name,
        Function<S64(void*)> handler,
        void*                context
    );
}

#endif //RUNEOS_DEFINITION_H
