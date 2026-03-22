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

#include <CPU/CPU.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                  Arch independent implementations
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

namespace Rune::CPU {
    DEFINE_ENUM(ThreadState, THREAD_STATES, 0x0)

    DEFINE_ENUM(SchedulingPolicy, SCHEDULING_POLICIES, 0x0)

    Thread::Thread(MutexHandle handle, const String& name) : Resource(handle, name) {}

    auto operator==(const Thread& one, const Thread& two) -> bool {
        return one.get_handle() == two.get_handle();
    }

    auto operator!=(const Thread& one, const Thread& two) -> bool {
        return one.get_handle() != two.get_handle();
    }

    DEFINE_TYPED_ENUM(PrivilegeLevel, U8, PRIVILEGE_LEVELS, 0x0)
} // namespace Rune::CPU