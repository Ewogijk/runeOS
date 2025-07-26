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

#ifndef RUNEOS_ERROR_H
#define RUNEOS_ERROR_H


namespace Ember::Error {
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          System Call Error Codes
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    constexpr int BAD_ARG       = -100; // A bad system call argument was passed to the kernel.
    constexpr int FAULT         = -101; // An error happened during the system call.
    constexpr int BAD_ID        = -102; // No resource with the requested ID was found.
    constexpr int ACCESS_DENIED = -103; // Access to the resource was denied.
    constexpr int IO            = -104; // An IO error occurred.

    constexpr int NODE_NOT_FOUND    = -105; // A node does not exist but should exist.
    constexpr int NODE_EXISTS       = -106; // A node does exist but was expected to not exist.
    constexpr int NODE_IS_DIRECTORY = -107; // The node is a directory but should be a file.
    constexpr int NODE_IS_FILE      = -108; // A node is a file but should be a directory.
    constexpr int NODE_IN_USE       = -109; // A node is in use by another application.
    constexpr int NODE_CLOSED       = -110; // The node has already been closed.
}

#endif //RUNEOS_ERROR_H
