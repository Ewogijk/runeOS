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

#ifndef EMBER_STATUSCODE_H
#define EMBER_STATUSCODE_H

#include <Ember/Enum.h>

namespace Ember {
    using StatusCode = int;

    /**
     * @brief The status codes returned by system calls.
     * <ul>
     *  <li>OKAY: The system call finished without error.</li>
     *  <li>BAD_ARG: A bad system call argument was passed to the kernel.</li>
     *  <li>FAULT: An error happened during the system call.</li>
     *  <li>UNKNOWN_ID: No resource with the requested ID was found.</li>
     *  <li>ACCESS_DENIED: Access to the resource was denied.</li>
     *  <li>IO_ERROR: An IO error occurred.</li>
     *  <li>DIRECTORY_STREAM_EOD: End of directory reached</li>
     *  <li>DIRECTORY_STREAM_HAS_MORE: Has at least one more node.</li>
     *  <li>NODE_NOT_FOUND: A node does not exist but should exist.</li>
     *  <li>NODE_EXISTS: A node does exist but was expected to not exist.</li>
     *  <li>NODE_IS_DIRECTORY: The node is a directory but should be a file.</li>
     *  <li>NODE_IS_FILE: A node is a file but should be a directory.</li>
     *  <li>NODE_IN_USE: A node is in use by another application.</li>
     *  <li>NODE_CLOSED: The node has already been closed.</li>
     * </ul>
     */
#define STATUSES(X)                             \
        X(Status, OKAY, 0)                      \
        X(Status, BAD_ARG, -1)                  \
        X(Status, FAULT, -2)                    \
        X(Status, UNKNOWN_ID, -3)               \
        X(Status, ACCESS_DENIED, -4)            \
        X(Status, IO_ERROR, -5)                 \
        X(Status, DIRECTORY_STREAM_EOD, 1)      \
        X(Status, DIRECTORY_STREAM_HAS_MORE, 2) \
        X(Status, NODE_NOT_FOUND, -100)         \
        X(Status, NODE_EXISTS, -101)            \
        X(Status, NODE_IS_DIRECTORY, -102)      \
        X(Status, NODE_IS_FILE, -103)           \
        X(Status, NODE_IN_USE, -104)            \
        X(Status, NODE_CLOSED, -105)            \


    DECLARE_TYPED_ENUM(Status, StatusCode, STATUSES, 0x0) // NOLINT
}

#endif //EMBER_STATUSCODE_H
