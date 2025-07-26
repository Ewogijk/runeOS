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

#ifndef RUNEOS_STATUS_H
#define RUNEOS_STATUS_H


#include <Ember/Enum.h>

namespace Rune::VFS {

    /**
     * Result of formatting a storage device.
     */
#define FORMAT_STATUSES(X)                  \
    X(FormatStatus, FORMATTED, 0x1)         \
    X(FormatStatus, FORMAT_ERROR, 0x2)      \
    X(FormatStatus, UNKNOWN_DRIVER, 0x3)    \
    X(FormatStatus, DEV_ERROR, 0x4) \


    DECLARE_ENUM(FormatStatus, FORMAT_STATUSES, 0x0) //NOLINT


    /**
     * Result of mounting a storage device.
     */
#define MOUNT_STATUSES(X)                   \
    X(MountStatus, MOUNTED, 0x1)            \
    X(MountStatus, UNMOUNTED, 0x2)          \
    X(MountStatus, NOT_MOUNTED, 0x3)        \
    X(MountStatus, ALREADY_MOUNTED, 0x4)    \
    X(MountStatus, NOT_SUPPORTED, 0x5)      \
    X(MountStatus, MOUNT_ERROR, 0x6)        \
    X(MountStatus, BAD_PATH, 0x7)           \
    X(MountStatus, DEV_ERROR, 0x8)          \



    DECLARE_ENUM(MountStatus, MOUNT_STATUSES, 0x0) //NOLINT


    /**
     * Result of accessing a storage device.
     */
#define IO_STATUSES(X)                  \
    X(IOStatus, CREATED, 0x1)           \
    X(IOStatus, OPENED, 0x2)            \
    X(IOStatus, DELETED, 0x3)           \
    X(IOStatus, FOUND, 0x4)             \
    X(IOStatus, NOT_FOUND, 0x5)         \
    X(IOStatus, BAD_PATH, 0x6)          \
    X(IOStatus, BAD_NAME, 0x7)          \
    X(IOStatus, BAD_ATTRIBUTE, 0x8)     \
    X(IOStatus, BAD_NODE_IO_MODE, 0x9)  \
    X(IOStatus, OUT_OF_HANDLES, 0xA)    \
    X(IOStatus, DEV_ERROR, 0xB)         \
    X(IOStatus, DEV_UNKNOWN, 0xC)       \
    X(IOStatus, DEV_OUT_OF_MEMORY, 0xD) \
    X(IOStatus, ACCESS_DENIED, 0xE)     \



    DECLARE_ENUM(IOStatus, IO_STATUSES, 0x0) //NOLINT
}

#endif //RUNEOS_STATUS_H
