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

#ifndef EMBER_SYSTEMCALLHANDLE_H
#define EMBER_SYSTEMCALLHANDLE_H

#include <Ember/Ember.h>
#include <Ember/Enum.h>

namespace Ember {

    ///@brief The memory system call handles.
#define MEMORY_SYSCALLS(X)                                                                         \
    X(GET_PAGE_SIZE, 100)                                                                          \
    X(ALLOCATE_PAGE, 101)                                                                          \
    X(FREE_PAGE, 102)

    EMBER_DEFINE_SMART_ENUM(Memory, MEMORY_SYSCALLS, 0x0, Handle) // NOLINT

    /// @brief @brief The threading system call handles.
#define THREADING_SYSCALLS(X)                                                                      \
    X(MUTEX_CREATE, 200)                                                                           \
    X(MUTEX_LOCK, 201)                                                                             \
    X(MUTEX_UNLOCK, 202)                                                                           \
    X(MUTEX_FREE, 203)                                                                             \
    X(THREAD_GET_ID, 204)                                                                          \
    X(THREAD_CONTROL_BLOCK_SET, 205)

    EMBER_DEFINE_SMART_ENUM(Threading, THREADING_SYSCALLS, 0x0, Handle) // NOLINT

    ///@brief The VFS system call handles.
#define VFS_SYSCALLS(X)                                                                            \
    X(GET_NODE_INFO, 300)                                                                          \
    X(GET_NODE_INFO_BY_ID, 301)                                                                    \
    X(CREATE, 302)                                                                                 \
    X(OPEN, 303)                                                                                   \
    X(DELETE, 304)                                                                                 \
    X(CLOSE, 305)                                                                                  \
    X(READ, 306)                                                                                   \
    X(WRITE, 307)                                                                                  \
    X(SEEK, 308)                                                                                   \
    X(DIRECTORY_STREAM_OPEN, 309)                                                                  \
    X(DIRECTORY_STREAM_NEXT, 310)                                                                  \
    X(DIRECTORY_STREAM_CLOSE, 311)

    EMBER_DEFINE_SMART_ENUM(VFS, VFS_SYSCALLS, 0x0, Handle) // NOLINT

    ///@brief The app system call handles.
#define APP_SYSCALLS(X)                                                                            \
    X(READ_STDIN, 400)                                                                             \
    X(WRITE_STDOUT, 401)                                                                           \
    X(WRITE_STDERR, 402)                                                                           \
    X(GET_ID, 403)                                                                                 \
    X(START, 404)                                                                                  \
    X(EXIT, 405)                                                                                   \
    X(JOIN, 406)                                                                                   \
    X(CURRENT_DIRECTORY, 407)                                                                      \
    X(CHANGE_DIRECTORY, 408)

    EMBER_DEFINE_SMART_ENUM(App, APP_SYSCALLS, 0x0, Handle) // NOLINT
} // namespace Ember

#endif // EMBER_SYSTEMCALLHANDLE_H
