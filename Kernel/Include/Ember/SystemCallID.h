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

#ifndef RUNEOS_SYSTEMCALLID_H
#define RUNEOS_SYSTEMCALLID_H

#include <Ember/Definitions.h>


namespace Ember {
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          "System" System Calls
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Memory System Calls
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    constexpr U16 MEMORY_GET_PAGE_SIZE = 100;
    constexpr U16 MEMORY_ALLOCATE_PAGE = 101;
    constexpr U16 MEMORY_FREE_PAGE     = 102;


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Threading System Calls
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    constexpr U16 THREADING_MUTEX_CREATE             = 200;
    constexpr U16 THREADING_MUTEX_LOCK               = 201;
    constexpr U16 THREADING_MUTEX_UNLOCK             = 202;
    constexpr U16 THREADING_MUTEX_RELEASE            = 203;
    constexpr U16 THREADING_THREAD_GET_ID            = 204;
    constexpr U16 THREADING_THREAD_CONTROL_BLOCK_SET = 205;


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          VFS System Calls
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    constexpr U16 VFS_GET_NODE_INFO          = 300;
    constexpr U16 VFS_CREATE                 = 301;
    constexpr U16 VFS_OPEN                   = 302;
    constexpr U16 VFS_DELETE                 = 303;
    constexpr U16 VFS_CLOSE                  = 304;
    constexpr U16 VFS_READ                   = 305;
    constexpr U16 VFS_WRITE                  = 306;
    constexpr U16 VFS_SEEK                   = 307;
    constexpr U16 VFS_DIRECTORY_STREAM_OPEN  = 308;
    constexpr U16 VFS_DIRECTORY_STREAM_NEXT  = 309;
    constexpr U16 VFS_DIRECTORY_STREAM_CLOSE = 310;


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          App System Calls
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    constexpr U16 APP_READ_STD_IN              = 400;
    constexpr U16 APP_WRITE_STD_OUT            = 401;
    constexpr U16 APP_WRITE_STD_ERR            = 402;
    constexpr U16 APP_START                    = 403;
    constexpr U16 APP_EXIT                     = 404;
    constexpr U16 APP_JOIN                     = 405;
    constexpr U16 APP_GET_WORKING_DIRECTORY    = 406;
    constexpr U16 APP_CHANGE_WORKING_DIRECTORY = 407;
}

#endif //RUNEOS_SYSTEMCALLID_H
