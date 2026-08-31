//  Copyright 2026 Ewogijk
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

#include <Ember/SystemCallHandle.h>

namespace Ember {
    DEFINE_TYPED_ENUM(Memory, Ember::Handle, MEMORY_SYSCALLS, 0x0)

    DEFINE_TYPED_ENUM(Threading, Ember::Handle, THREADING_SYSCALLS, 0x0)

    DEFINE_TYPED_ENUM(VFS, Ember::Handle, VFS_SYSCALLS, 0x0)

    DEFINE_TYPED_ENUM(App, Ember::Handle, APP_SYSCALLS, 0x0)
} // namespace Ember
