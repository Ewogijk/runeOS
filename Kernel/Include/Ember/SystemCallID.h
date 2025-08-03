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

#ifndef EMBER_SYSTEMCALLID_H
#define EMBER_SYSTEMCALLID_H

#include <Ember/Ember.h>
#include <Ember/Enum.h>


namespace Ember {

    /**
     * @brief The memory system call IDs.
     * <ul>
     *  <li>TODO describe enum values</li>
     * </ul>
     */
#define MEMORY_SYSCALLS(X)              \
        X(Memory, GET_PAGE_SIZE, 100)   \
        X(Memory, ALLOCATE_PAGE, 101)   \
        X(Memory, FREE_PAGE, 102)

    DECLARE_TYPED_ENUM(Memory, ResourceID, MEMORY_SYSCALLS, 0x0) // NOLINT


    /**
     * @brief The threading system call IDs.
     * <ul>
     *  <li>TODO describe enum values</li>
     * </ul>
     */
#define THREADING_SYSCALLS(X)                       \
        X(Threading, MUTEX_CREATE, 200)             \
        X(Threading, MUTEX_LOCK, 201)               \
        X(Threading, MUTEX_UNLOCK, 202)             \
        X(Threading, MUTEX_FREE, 203)               \
        X(Threading, THREAD_GET_ID, 204)            \
        X(Threading, THREAD_CONTROL_BLOCK_SET, 205)

    DECLARE_TYPED_ENUM(Threading, ResourceID, THREADING_SYSCALLS, 0x0) // NOLINT


    /**
     * @brief The VFS system call IDs.
     * <ul>
     *  <li>TODO describe enum values</li>
     * </ul>
     */
#define VFS_SYSCALLS(X)                     \
        X(VFS, GET_NODE_INFO, 300)          \
        X(VFS, CREATE, 301)                 \
        X(VFS, OPEN, 302)                   \
        X(VFS, DELETE, 303)                 \
        X(VFS, CLOSE, 304)                  \
        X(VFS, READ, 305)                   \
        X(VFS, WRITE, 306)                  \
        X(VFS, SEEK, 307)                   \
        X(VFS, DIRECTORY_STREAM_OPEN, 308)  \
        X(VFS, DIRECTORY_STREAM_NEXT, 309)  \
        X(VFS, DIRECTORY_STREAM_CLOSE, 310)

    DECLARE_TYPED_ENUM(VFS, ResourceID, VFS_SYSCALLS, 0x0) // NOLINT


    /**
     * @brief The app system call IDs.
     * <ul>
     *  <li>TODO describe enum values</li>
     * </ul>
     */
#define APP_SYSCALLS(X)                 \
        X(App, READ_STDIN, 400)         \
        X(App, WRITE_STDOUT, 401)       \
        X(App, WRITE_STDERR, 402)       \
        X(App, GET_ID, 403)             \
        X(App, START, 404)              \
        X(App, EXIT, 405)               \
        X(App, JOIN, 406)               \
        X(App, CURRENT_DIRECTORY, 407)  \
        X(App, CHANGE_DIRECTORY, 408)

    DECLARE_TYPED_ENUM(App, ResourceID, APP_SYSCALLS, 0x0) // NOLINT
}

#endif //EMBER_SYSTEMCALLID_H
