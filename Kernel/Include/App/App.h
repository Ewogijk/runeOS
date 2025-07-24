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

#ifndef RUNEOS_APP_H
#define RUNEOS_APP_H


#include <limits.h>

#include <Hammer/Memory.h>
#include <Hammer/Collection.h>
#include <Hammer/String.h>
#include <Hammer/Path.h>

#include <LibK/Stream.h>

#include <CPU/CPUSubsystem.h>

#include <VirtualFileSystem/VFSSubsystem.h>


namespace Rune::App {
#define LOAD_STATUSES(X)                \
    X(LoadStatus, LOADED, 0x1)          \
    X(LoadStatus, RUNNING, 0x2)         \
    X(LoadStatus, IO_ERROR, 0x3)        \
    X(LoadStatus, BAD_HEADER, 0x4)      \
    X(LoadStatus, BAD_SEGMENT, 0x5)     \
    X(LoadStatus, MEMORY_ERROR, 0x6)    \
    X(LoadStatus, LOAD_ERROR, 0x7)      \
    X(LoadStatus, BAD_VENDOR_INFO, 0x8) \
    X(LoadStatus, BAD_STDIO, 0x9)

    /**
     * Status of the finished ELF loading.
     */
    DECLARE_ENUM(LoadStatus, LOAD_STATUSES, 0x0) //NOLINT

    /**
     * General information and used system resources of an app.
     */
    struct Info {
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          General information
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


        /**
         * Path to the executable file
         */
        Path location = Path("");


        /**
         * Name of the app -> Filename without the .app extension
         */
        String name = "";


        /**
         * Vendor of the app (optional)
         */
        String vendor = "";


        /**
         * Versioning information about the app
         */
        LibK::Version version;


        /**
         * @brief The current directory of the app.
         *
         * After start this is either the directory of the executable if not explicitly set or an explicitly requested
         * path.
         */
        Path working_directory = Path("");


        /**
         * @brief Exit code of the application, this value will be set when an app makes a system call to quit itself.
         */
        int exit_code = INT_MAX;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                  Resources / resource tables
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        U16                handle                  = 0;
        LibK::PhysicalAddr base_page_table_address = 0x0;
        LibK::VirtualAddr  entry                   = 0x0;


        /**
         * @brief Application heap
         */
        LibK::VirtualAddr heap_start = 0x0;
        LibK::VirtualAddr heap_limit = 0x0;


        /**
         * Running threads of the app
         */
        LinkedList<int> thread_table = LinkedList<int>();


        /**
         * @brief All threads that are joining with this application, meaning waiting for it to exit.
         */
        LinkedList<SharedPointer<CPU::Thread>> joining_thread_table = LinkedList<SharedPointer<CPU::Thread>>();


        /**
         * @brief All open nodes of the app.
         */
        LinkedList<U16> node_table = LinkedList<U16>();


        /**
         * @brief All open directory streams of the app.
         */
        LinkedList<U16> directory_stream_table = LinkedList<U16>();


        /**
         * @brief stdio streams.
         */
        SharedPointer<LibK::TextStream> std_in  = SharedPointer<LibK::TextStream>();
        SharedPointer<LibK::TextStream> std_out = SharedPointer<LibK::TextStream>();
        SharedPointer<LibK::TextStream> std_err = SharedPointer<LibK::TextStream>();


        friend bool operator==(const Info& one, const Info& two);


        friend bool operator!=(const Info& one, const Info& two);
    };


    /**
     * @brief The load status of the elf executable and the assigned app handle.
     */
    struct StartStatus {
        LoadStatus load_result = LoadStatus::NONE;
        int        handle      = -1;
    };
};

#endif //RUNEOS_APP_H
