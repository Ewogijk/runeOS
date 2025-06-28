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
    Definition define6(
            U16 handle,
            const String& name,
            const Function<S64(void*, U64, U64, U64, U64, U64, U64)>& handler,
            void* context
    ) {
        return {
                handle,
                name,
                [handler](
                        void* sys_call_ctx,
                        U64 arg1,
                        U64 arg2,
                        U64 arg3,
                        U64 arg4,
                        U64 arg5,
                        U64 arg6
                ) {
                    return handler(
                            forward<void*>(sys_call_ctx),
                            forward<U64>(arg1),
                            forward<U64>(arg2),
                            forward<U64>(arg3),
                            forward<U64>(arg4),
                            forward<U64>(arg5),
                            forward<U64>(arg6)
                    );
                },
                context
        };
    }


    Definition define5(
            U16 handle,
            const String& name,
            const Function<S64(void*, U64, U64, U64, U64, U64)>& handler,
            void* context
    ) {
        return {
                handle,
                name,
                [handler](
                        void* sys_call_ctx,
                        U64 arg1,
                        U64 arg2,
                        U64 arg3,
                        U64 arg4,
                        U64 arg5,
                        U64 arg6
                ) {
                    SILENCE_UNUSED(arg6)
                    return handler(
                            forward<void*>(sys_call_ctx),
                            forward<U64>(arg1),
                            forward<U64>(arg2),
                            forward<U64>(arg3),
                            forward<U64>(arg4),
                            forward<U64>(arg5)
                    );
                },
                context
        };
    }


    Definition define4(
            U16 handle,
            const String& name,
            const Function<S64(void*, U64, U64, U64, U64)>& handler,
            void* context
    ) {
        return {
                handle,
                name,
                [handler](
                        void* sys_call_ctx,
                        U64 arg1,
                        U64 arg2,
                        U64 arg3,
                        U64 arg4,
                        U64 arg5,
                        U64 arg6
                ) {
                    SILENCE_UNUSED(arg5)
                    SILENCE_UNUSED(arg6)
                    return handler(
                            forward<void*>(sys_call_ctx),
                            forward<U64>(arg1),
                            forward<U64>(arg2),
                            forward<U64>(arg3),
                            forward<U64>(arg4)
                    );
                },
                context
        };
    }


    Definition define3(
            U16 handle,
            const String& name,
            const Function<S64(void*, U64, U64, U64)>& handler,
            void* context
    ) {
        return {
                handle,
                name,
                [handler](
                        void* sys_call_ctx,
                        U64 arg1,
                        U64 arg2,
                        U64 arg3,
                        U64 arg4,
                        U64 arg5,
                        U64 arg6
                ) {
                    SILENCE_UNUSED(arg4)
                    SILENCE_UNUSED(arg5)
                    SILENCE_UNUSED(arg6)
                    return handler(
                            forward<void*>(sys_call_ctx),
                            forward<U64>(arg1),
                            forward<U64>(arg2),
                            forward<U64>(arg3)
                    );
                },
                context
        };
    }


    Definition define2(
            U16 handle,
            const String& name,
            const Function<S64(void*, U64, U64)>& handler,
            void* context
    ) {
        return {
                handle,
                name,
                [handler](
                        void* sys_call_ctx,
                        U64 arg1,
                        U64 arg2,
                        U64 arg3,
                        U64 arg4,
                        U64 arg5,
                        U64 arg6
                ) {
                    SILENCE_UNUSED(arg3)
                    SILENCE_UNUSED(arg4)
                    SILENCE_UNUSED(arg5)
                    SILENCE_UNUSED(arg6)
                    return handler(
                            forward<void*>(sys_call_ctx),
                            forward<U64>(arg1),
                            forward<U64>(arg2)
                    );
                },
                context
        };
    }


    Definition define1(
            U16 handle,
            const String& name,
            const Function<S64(void*, U64)>& handler,
            void* context
    ) {
        return {
                handle,
                name,
                [handler](
                        void* sys_call_ctx,
                        U64 arg1,
                        U64 arg2,
                        U64 arg3,
                        U64 arg4,
                        U64 arg5,
                        U64 arg6
                ) {
                    SILENCE_UNUSED(arg2)
                    SILENCE_UNUSED(arg3)
                    SILENCE_UNUSED(arg4)
                    SILENCE_UNUSED(arg5)
                    SILENCE_UNUSED(arg6)
                    return handler(
                            forward<void*>(sys_call_ctx),
                            forward<U64>(arg1)
                    );
                },
                context
        };
    }


    Definition define0(
            U16 handle,
            const String& name,
            Function<S64(void*)> handler,
            void* context
    ) {
        return {
                handle,
                name,
                [handler](
                        void* sys_call_ctx,
                        U64 arg1,
                        U64 arg2,
                        U64 arg3,
                        U64 arg4,
                        U64 arg5,
                        U64 arg6
                ) {
                    SILENCE_UNUSED(arg1)
                    SILENCE_UNUSED(arg2)
                    SILENCE_UNUSED(arg3)
                    SILENCE_UNUSED(arg4)
                    SILENCE_UNUSED(arg5)
                    SILENCE_UNUSED(arg6)
                    return handler(forward<void*>(sys_call_ctx));
                },
                context
        };
    }
}