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


#include <Ember/SystemCallID.h>


namespace Ember {
    DEFINE_TYPED_ENUM(Memory, U16, MEMORY_SYSCALLS, 0x0)

    DEFINE_TYPED_ENUM(Threading, U16, THREADING_SYSCALLS, 0x0)

    DEFINE_TYPED_ENUM(VFS, U16, VFS_SYSCALLS, 0x0)

    DEFINE_TYPED_ENUM(App, U16, APP_SYSCALLS, 0x0)
}
