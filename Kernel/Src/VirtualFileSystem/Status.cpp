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

#include <VirtualFileSystem/Status.h>


namespace Rune::VFS {
    IMPLEMENT_ENUM(FormatStatus, FORMAT_STATUSES, 0x0)


    IMPLEMENT_ENUM(MountStatus, MOUNT_STATUSES, 0x0)


    IMPLEMENT_ENUM(IOStatus, IO_STATUSES, 0x0)
}