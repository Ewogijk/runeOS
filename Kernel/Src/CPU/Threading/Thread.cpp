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

#include <CPU/Threading/Thread.h>

namespace Rune::CPU {
    ResourceCache<Thread, 4>
        g_thread_cache({"ID-Name", "State", "Policy", "App"},
                       [](const SharedPointer<Thread>& thread) -> Array<String, 4> {
                           return {thread->get_unique_name(),
                                   thread->state.to_string(),
                                   thread->policy.to_string(),
                                   String::format("{}", thread->app_handle)};
                       });
} // namespace Rune::CPU
