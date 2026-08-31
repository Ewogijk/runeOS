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

#ifndef RUNEOS_APP_H
#define RUNEOS_APP_H

#include <limits.h> //NOLINT climits does not exist

#include <KRE/Memory.h>
#include <KRE/Stream.h>
#include <KRE/String.h>

#include <KRE/Collections/LinkedList.h>

#include <CPU/Core.h>

#include <VirtualFileSystem/Path.h>
#include <VirtualFileSystem/VFSModule.h>

#include <App/ELF.h>

namespace Rune::App {

#define LOAD_STATUSES(X)                                                                           \
    X(LoadStatus, LOADED, 0x1)                                                                     \
    X(LoadStatus, RUNNING, 0x2)                                                                    \
    X(LoadStatus, IO_ERROR, 0x3)                                                                   \
    X(LoadStatus, BAD_HEADER, 0x4)                                                                 \
    X(LoadStatus, BAD_SEGMENT, 0x5)                                                                \
    X(LoadStatus, MEMORY_ERROR, 0x6)                                                               \
    X(LoadStatus, LOAD_ERROR, 0x7)                                                                 \
    X(LoadStatus, BAD_VENDOR_INFO, 0x8)                                                            \
    X(LoadStatus, BAD_STDIO, 0x9)

    /// @brief Status of the finished ELF loading.
    DECLARE_ENUM(LoadStatus, LOAD_STATUSES, 0x0) // NOLINT

    /// @brief The Bootstrap region is a memory region at the end of the user space directly
    ///         bordering the kernel space.
    ///
    /// The bootstrap region contains important information to launch the main thread of an
    /// application such as argv, argc, etc. The kernel will initialize this region and then hand
    /// it off the application.
    struct BootstrapRegion {
        static constexpr U8 PROGRAM_HEADER_COUNT = 32;
        /// @brief 1MiB memory region at 128TiB - 1MiB.
        static constexpr VirtualAddr BOOTSTRAP_REGION_START = 0x00007FFFFFF00000;
        static constexpr MemorySize  BOOTSTRAP_REGION_SIZE  = MemoryUnit::MiB;

        /// @brief Thread launch packet of the application main thread.
        Ember::ThreadLaunchPacket m_tlp;

        /// @brief ELF file program header information.
        Array<ELF64ProgramHeader, PROGRAM_HEADER_COUNT> m_program_headers;
    };

    /// @brief General information and used system resources of an app.
    struct Info {
        // ====================================================================================== //
        // General information
        // ====================================================================================== //

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
        Version version;

        /**
         * @brief The current directory of the app.
         *
         * After start this is either the directory of the executable if not explicitly set or an
         * explicitly requested path.
         */
        Path working_directory = Path("");

        /**
         * @brief Exit code of the application, this value will be set when an app makes a system
         * call to quit itself.
         */
        int exit_code = INT_MAX;

        // ====================================================================================== //
        // Resources / Resource tables
        // ====================================================================================== //

        U16          handle                  = 0;
        PhysicalAddr base_page_table_address = 0x0;
        VirtualAddr  entry                   = 0x0;

        BootstrapRegion* m_bootstrap_region;

        /**
         * @brief Application heap
         */
        VirtualAddr heap_start = 0x0;
        VirtualAddr heap_limit = 0x0;

        /**
         * Running threads of the app
         */
        LinkedList<U16> thread_table;

        /**
         * @brief All threads that are joining with this application, meaning waiting for it to
         * exit.
         */
        LinkedList<SharedPointer<CPU::Thread>> joining_thread_table;

        /**
         * @brief All open nodes of the app.
         */
        LinkedList<U16> node_table;

        /**
         * @brief All open directory streams of the app.
         */
        LinkedList<U16> directory_stream_table;

        /**
         * @brief stdio streams.
         */
        SharedPointer<TextStream> std_in;
        SharedPointer<TextStream> std_out;
        SharedPointer<TextStream> std_err;

        friend auto operator==(const Info& one, const Info& two) -> bool;

        friend auto operator!=(const Info& one, const Info& two) -> bool;
    };

    /// @brief The load status of the elf executable and the assigned app handle.
    struct StartStatus {
        LoadStatus    m_load_result = LoadStatus::NONE;
        Ember::Handle m_handle      = Ember::HANDLE_NONE;
    };
}; // namespace Rune::App

#endif // RUNEOS_APP_H
