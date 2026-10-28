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

#ifndef RUNEOS_STATUS_H
#define RUNEOS_STATUS_H

#include <Ember/Enum.h>

namespace Rune::VFS {

    /**
     * Result of formatting a storage device.
     */
#define FORMAT_STATUSES(X)                                                                         \
    X(FORMATTED, 0x1)                                                                              \
    X(FORMAT_ERROR, 0x2)                                                                           \
    X(UNKNOWN_DRIVER, 0x3)                                                                         \
    X(DEV_ERROR, 0x4)

    EMBER_DEFINE_SMART_ENUM(FormatStatus, FORMAT_STATUSES, 0x0) // NOLINT

    /**
     * Result of mounting a storage device.
     */
#define MOUNT_STATUSES(X)                                                                          \
    X(MOUNTED, 0x1)                                                                                \
    X(UNMOUNTED, 0x2)                                                                              \
    X(NOT_MOUNTED, 0x3)                                                                            \
    X(ALREADY_MOUNTED, 0x4)                                                                        \
    X(NOT_SUPPORTED, 0x5)                                                                          \
    X(MOUNT_ERROR, 0x6)                                                                            \
    X(BAD_PATH, 0x7)                                                                               \
    X(DEV_ERROR, 0x8)

    EMBER_DEFINE_SMART_ENUM(MountStatus, MOUNT_STATUSES, 0x0) // NOLINT

    /**
     * Result of accessing a storage device.
     */
#define IO_STATUSES(X)                                                                             \
    X(CREATED, 0x1)                                                                                \
    X(OPENED, 0x2)                                                                                 \
    X(DELETED, 0x3)                                                                                \
    X(FOUND, 0x4)                                                                                  \
    X(NOT_FOUND, 0x5)                                                                              \
    X(BAD_PATH, 0x6)                                                                               \
    X(BAD_NAME, 0x7)                                                                               \
    X(BAD_ATTRIBUTE, 0x8)                                                                          \
    X(BAD_NODE_IO_MODE, 0x9)                                                                       \
    X(OUT_OF_HANDLES, 0xA)                                                                         \
    X(DEV_ERROR, 0xB)                                                                              \
    X(DEV_UNKNOWN, 0xC)                                                                            \
    X(DEV_OUT_OF_MEMORY, 0xD)                                                                      \
    X(ACCESS_DENIED, 0xE)

    EMBER_DEFINE_SMART_ENUM(IOStatus, IO_STATUSES, 0x0) // NOLINT
} // namespace Rune::VFS

#endif // RUNEOS_STATUS_H
