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

#include <SystemCall/Definition.h>

namespace Rune::SystemCall {
    auto define6(const Ember::ResourceID                         ID,
                 const String&                                   name,
                 const Function<S64(void*,
                                    Ember::SystemCallArgument,
                                    Ember::SystemCallArgument,
                                    Ember::SystemCallArgument,
                                    Ember::SystemCallArgument,
                                    Ember::SystemCallArgument,
                                    Ember::SystemCallArgument)>& handler,
                 void*                                           context) -> Definition {
        return {.ID   = ID,
                .name = name,
                .sys_call_handler =
                    [handler](void* sys_call_ctx,
                              Ember::SystemCallArgument arg1,
                              Ember::SystemCallArgument arg2,
                              Ember::SystemCallArgument arg3,
                              Ember::SystemCallArgument arg4,
                              Ember::SystemCallArgument arg5,
                              Ember::SystemCallArgument arg6) {
                        return handler(forward<void*>(sys_call_ctx),
                                       forward<Ember::SystemCallArgument>(arg1),
                                       forward<Ember::SystemCallArgument>(arg2),
                                       forward<Ember::SystemCallArgument>(arg3),
                                       forward<Ember::SystemCallArgument>(arg4),
                                       forward<Ember::SystemCallArgument>(arg5),
                                       forward<Ember::SystemCallArgument>(arg6));
                    },
                .context = context};
    }

    auto define5(Ember::ResourceID                               ID,
                 const String&                                   name,
                 const Function<S64(void*,
                                    Ember::SystemCallArgument,
                                    Ember::SystemCallArgument,
                                    Ember::SystemCallArgument,
                                    Ember::SystemCallArgument,
                                    Ember::SystemCallArgument)>& handler,
                 void*                                           context) -> Definition {
        return {.ID   = ID,
                .name = name,
                .sys_call_handler =
                    [handler](void* sys_call_ctx,
                              Ember::SystemCallArgument arg1,
                              Ember::SystemCallArgument arg2,
                              Ember::SystemCallArgument arg3,
                              Ember::SystemCallArgument arg4,
                              Ember::SystemCallArgument arg5,
                              Ember::SystemCallArgument arg6) {
                        SILENCE_UNUSED(arg6)
                        return handler(forward<void*>(sys_call_ctx),
                                       forward<Ember::SystemCallArgument>(arg1),
                                       forward<Ember::SystemCallArgument>(arg2),
                                       forward<Ember::SystemCallArgument>(arg3),
                                       forward<Ember::SystemCallArgument>(arg4),
                                       forward<Ember::SystemCallArgument>(arg5));
                    },
                .context = context};
    }

    auto define4(Ember::ResourceID                               ID,
                 const String&                                   name,
                 const Function<S64(void*,
                                    Ember::SystemCallArgument,
                                    Ember::SystemCallArgument,
                                    Ember::SystemCallArgument,
                                    Ember::SystemCallArgument)>& handler,
                 void*                                           context) -> Definition {
        return {.ID   = ID,
                .name = name,
                .sys_call_handler =
                    [handler](void* sys_call_ctx,
                              Ember::SystemCallArgument arg1,
                              Ember::SystemCallArgument arg2,
                              Ember::SystemCallArgument arg3,
                              Ember::SystemCallArgument arg4,
                              Ember::SystemCallArgument arg5,
                              Ember::SystemCallArgument arg6) {
                        SILENCE_UNUSED(arg5)
                        SILENCE_UNUSED(arg6)
                        return handler(forward<void*>(sys_call_ctx),
                                       forward<Ember::SystemCallArgument>(arg1),
                                       forward<Ember::SystemCallArgument>(arg2),
                                       forward<Ember::SystemCallArgument>(arg3),
                                       forward<Ember::SystemCallArgument>(arg4));
                    },
                .context = context};
    }

    auto define3(Ember::ResourceID                               ID,
                 const String&                                   name,
                 const Function<S64(void*,
                                    Ember::SystemCallArgument,
                                    Ember::SystemCallArgument,
                                    Ember::SystemCallArgument)>& handler,
                 void*                                           context) -> Definition {
        return {.ID   = ID,
                .name = name,
                .sys_call_handler =
                    [handler](void* sys_call_ctx,
                              Ember::SystemCallArgument arg1,
                              Ember::SystemCallArgument arg2,
                              Ember::SystemCallArgument arg3,
                              Ember::SystemCallArgument arg4,
                              Ember::SystemCallArgument arg5,
                              Ember::SystemCallArgument arg6) {
                        SILENCE_UNUSED(arg4)
                        SILENCE_UNUSED(arg5)
                        SILENCE_UNUSED(arg6)
                        return handler(forward<void*>(sys_call_ctx),
                                       forward<Ember::SystemCallArgument>(arg1),
                                       forward<Ember::SystemCallArgument>(arg2),
                                       forward<Ember::SystemCallArgument>(arg3));
                    },
                .context = context};
    }

    auto define2(
        Ember::ResourceID                                                                 ID,
        const String&                                                                     name,
        const Function<S64(void*, Ember::SystemCallArgument, Ember::SystemCallArgument)>& handler,
        void* context) -> Definition {
        return {.ID   = ID,
                .name = name,
                .sys_call_handler =
                    [handler](void* sys_call_ctx,
                              Ember::SystemCallArgument arg1,
                              Ember::SystemCallArgument arg2,
                              Ember::SystemCallArgument arg3,
                              Ember::SystemCallArgument arg4,
                              Ember::SystemCallArgument arg5,
                              Ember::SystemCallArgument arg6) {
                        SILENCE_UNUSED(arg3)
                        SILENCE_UNUSED(arg4)
                        SILENCE_UNUSED(arg5)
                        SILENCE_UNUSED(arg6)
                        return handler(forward<void*>(sys_call_ctx),
                                       forward<Ember::SystemCallArgument>(arg1),
                                       forward<Ember::SystemCallArgument>(arg2));
                    },
                .context = context};
    }

    auto define1(Ember::ResourceID                                      ID,
                 const String&                                          name,
                 const Function<S64(void*, Ember::SystemCallArgument)>& handler,
                 void*                                                  context) -> Definition {
        return {.ID   = ID,
                .name = name,
                .sys_call_handler =
                    [handler](void* sys_call_ctx,
                              Ember::SystemCallArgument arg1,
                              Ember::SystemCallArgument arg2,
                              Ember::SystemCallArgument arg3,
                              Ember::SystemCallArgument arg4,
                              Ember::SystemCallArgument arg5,
                              Ember::SystemCallArgument arg6) {
                        SILENCE_UNUSED(arg2)
                        SILENCE_UNUSED(arg3)
                        SILENCE_UNUSED(arg4)
                        SILENCE_UNUSED(arg5)
                        SILENCE_UNUSED(arg6)
                        return handler(forward<void*>(sys_call_ctx),
                                       forward<Ember::SystemCallArgument>(arg1));
                    },
                .context = context};
    }

    auto define0(Ember::ResourceID           ID,
                 const String&               name,
                 const Function<S64(void*)>& handler,
                 void*                       context) -> Definition {
        return {.ID   = ID,
                .name = name,
                .sys_call_handler =
                    [handler](void* sys_call_ctx,
                              Ember::SystemCallArgument arg1,
                              Ember::SystemCallArgument arg2,
                              Ember::SystemCallArgument arg3,
                              Ember::SystemCallArgument arg4,
                              Ember::SystemCallArgument arg5,
                              Ember::SystemCallArgument arg6) {
                        SILENCE_UNUSED(arg1)
                        SILENCE_UNUSED(arg2)
                        SILENCE_UNUSED(arg3)
                        SILENCE_UNUSED(arg4)
                        SILENCE_UNUSED(arg5)
                        SILENCE_UNUSED(arg6)
                        return handler(forward<void*>(sys_call_ctx));
                    },
                .context = context};
    }
} // namespace Rune::SystemCall
