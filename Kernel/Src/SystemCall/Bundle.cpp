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

#include <SystemCall/Bundle.h>

#include <Ember/SystemCallID.h>

#include <KRE/System/System.h>

#include <SystemCall/AppBundle.h>
#include <SystemCall/KernelGuardian.h>
#include <SystemCall/MemoryBundle.h>
#include <SystemCall/ThreadingBundle.h>
#include <SystemCall/VFSBundle.h>

#include <App/AppModule.h>

#include <Device/DeviceModule.h>

#include <CPU/CPUModule.h>

#include <VirtualFileSystem/VFSModule.h>

namespace Rune::SystemCall {
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          App System Call Bundle
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    AppSystemCallContext APP_SYSCALL_CTX;

    Bundle make_app_bundle(KernelGuardian* k_guard) {
        System& system     = System::instance();
        auto*   app_module = system.get_module<App::AppModule>(ModuleSelector::APP);
        auto*   dev_module = system.get_module<Device::DeviceModule>(ModuleSelector::DEVICE);
        auto*   cpu_module = system.get_module<CPU::CPUModule>(ModuleSelector::CPU);
        auto*   vfs_module = system.get_module<VFS::VFSModule>(ModuleSelector::VFS);
        APP_SYSCALL_CTX    = {k_guard, app_module, dev_module, cpu_module, vfs_module};

        LinkedList<Definition> defs;
        defs.add_back(define1(Ember::App::READ_STDIN,
                              Ember::App(Ember::App::READ_STDIN).to_string(),
                              &read_stdin,
                              &APP_SYSCALL_CTX));
        defs.add_back(define2(Ember::App::WRITE_STDOUT,
                              Ember::App(Ember::App::WRITE_STDOUT).to_string(),
                              &write_stdout,
                              &APP_SYSCALL_CTX));
        defs.add_back(define2(Ember::App::WRITE_STDERR,
                              Ember::App(Ember::App::WRITE_STDERR).to_string(),
                              &write_stderr,
                              &APP_SYSCALL_CTX));
        defs.add_back(define0(Ember::App::GET_ID,
                              Ember::App(Ember::App::GET_ID).to_string(),
                              &get_ID,
                              &APP_SYSCALL_CTX));
        defs.add_back(define6(Ember::App::START,
                              Ember::App(Ember::App::START).to_string(),
                              &app_start,
                              &APP_SYSCALL_CTX));
        defs.add_back(define1(Ember::App::EXIT,
                              Ember::App(Ember::App::EXIT).to_string(),
                              &app_exit,
                              &APP_SYSCALL_CTX));
        defs.add_back(define1(Ember::App::JOIN,
                              Ember::App(Ember::App::JOIN).to_string(),
                              &app_join,
                              &APP_SYSCALL_CTX));
        defs.add_back(define2(Ember::App::CURRENT_DIRECTORY,
                              Ember::App(Ember::App::CURRENT_DIRECTORY).to_string(),
                              &app_current_directory,
                              &APP_SYSCALL_CTX));
        defs.add_back(define1(Ember::App::CHANGE_DIRECTORY,
                              Ember::App(Ember::App::CHANGE_DIRECTORY).to_string(),
                              &app_change_directory,
                              &APP_SYSCALL_CTX));

        return {"App", defs};
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          VFS System Call Bundle
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    VFSSystemCallContext VFS_SYSCALL_CTX;

    Bundle make_vfs_bundle(KernelGuardian* k_guard) {
        System& system     = System::instance();
        auto*   app_module = system.get_module<App::AppModule>(ModuleSelector::APP);
        auto*   vfs_module = system.get_module<VFS::VFSModule>(ModuleSelector::VFS);
        VFS_SYSCALL_CTX    = {
            k_guard,
            vfs_module,
            app_module,
        };

        LinkedList<Definition> defs;

        defs.add_back(define2(Ember::VFS::GET_NODE_INFO,
                              Ember::VFS(Ember::VFS::GET_NODE_INFO).to_string(),
                              &vfs_get_node_info,
                              &VFS_SYSCALL_CTX));
        defs.add_back(define2(Ember::VFS::CREATE,
                              Ember::VFS(Ember::VFS::CREATE).to_string(),
                              &vfs_create,
                              &VFS_SYSCALL_CTX));
        defs.add_back(define2(Ember::VFS::OPEN,
                              Ember::VFS(Ember::VFS::OPEN).to_string(),
                              &vfs_open,
                              &VFS_SYSCALL_CTX));
        defs.add_back(define1(Ember::VFS::DELETE,
                              Ember::VFS(Ember::VFS::DELETE).to_string(),
                              &vfs_delete,
                              &VFS_SYSCALL_CTX));
        defs.add_back(define1(Ember::VFS::CLOSE,
                              Ember::VFS(Ember::VFS::CLOSE).to_string(),
                              &vfs_close,
                              &VFS_SYSCALL_CTX));
        defs.add_back(define3(Ember::VFS::READ,
                              Ember::VFS(Ember::VFS::READ).to_string(),
                              &vfs_read,
                              &VFS_SYSCALL_CTX));
        defs.add_back(define3(Ember::VFS::WRITE,
                              Ember::VFS(Ember::VFS::WRITE).to_string(),
                              &vfs_write,
                              &VFS_SYSCALL_CTX));
        defs.add_back(define3(Ember::VFS::SEEK,
                              Ember::VFS(Ember::VFS::SEEK).to_string(),
                              &vfs_seek,
                              &VFS_SYSCALL_CTX));
        defs.add_back(define1(Ember::VFS::DIRECTORY_STREAM_OPEN,
                              Ember::VFS(Ember::VFS::DIRECTORY_STREAM_OPEN).to_string(),
                              &vfs_directory_stream_open,
                              &VFS_SYSCALL_CTX));
        defs.add_back(define2(Ember::VFS::DIRECTORY_STREAM_NEXT,
                              Ember::VFS(Ember::VFS::DIRECTORY_STREAM_NEXT).to_string(),
                              &vfs_directory_stream_next,
                              &VFS_SYSCALL_CTX));
        defs.add_back(define1(Ember::VFS::DIRECTORY_STREAM_CLOSE,
                              Ember::VFS(Ember::VFS::DIRECTORY_STREAM_CLOSE).to_string(),
                              &vfs_directory_stream_close,
                              &VFS_SYSCALL_CTX));
        return {"VFS", defs};
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      Memory System Call Bundle
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    MemorySystemCallContext MM_SYSCALL_CTX;

    Bundle make_memory_bundle(KernelGuardian* k_guard) {
        System& system        = System::instance();
        auto*   app_module    = system.get_module<App::AppModule>(ModuleSelector::APP);
        auto*   memory_module = system.get_module<Memory::MemoryModule>(ModuleSelector::MEMORY);
        MM_SYSCALL_CTX        = {
            k_guard,
            memory_module,
            app_module,
        };

        LinkedList<Definition> defs;
        defs.add_back(define0(Ember::Memory::GET_PAGE_SIZE,
                              Ember::Memory(Ember::Memory::GET_PAGE_SIZE).to_string(),
                              &memory_get_page_size,
                              &MM_SYSCALL_CTX));
        defs.add_back(define3(Ember::Memory::ALLOCATE_PAGE,
                              Ember::Memory(Ember::Memory::ALLOCATE_PAGE).to_string(),
                              &memory_allocate_page,
                              &MM_SYSCALL_CTX));
        defs.add_back(define2(Ember::Memory::FREE_PAGE,
                              Ember::Memory(Ember::Memory::FREE_PAGE).to_string(),
                              &memory_free_page,
                              &MM_SYSCALL_CTX));
        return {"Memory", defs};
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      Threading System Call Context
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    ThreadingSystemCallContext T_SYSCALL_CTX;

    Bundle make_threading_bundle(KernelGuardian* k_guard) {
        System& system     = System::instance();
        auto*   app_module = system.get_module<App::AppModule>(ModuleSelector::APP);
        auto*   cpu_module = system.get_module<CPU::CPUModule>(ModuleSelector::CPU);
        T_SYSCALL_CTX      = {
            k_guard,
            cpu_module,
            app_module,
        };

        LinkedList<Definition> defs;
        defs.add_back(define1(Ember::Threading::MUTEX_CREATE,
                              Ember::Threading(Ember::Threading::MUTEX_CREATE).to_string(),
                              &mutex_create,
                              &T_SYSCALL_CTX));
        defs.add_back(define1(Ember::Threading::MUTEX_LOCK,
                              Ember::Threading(Ember::Threading::MUTEX_LOCK).to_string(),
                              &mutex_lock,
                              &T_SYSCALL_CTX));
        defs.add_back(define1(Ember::Threading::MUTEX_UNLOCK,
                              Ember::Threading(Ember::Threading::MUTEX_UNLOCK).to_string(),
                              &mutex_unlock,
                              &T_SYSCALL_CTX));
        defs.add_back(define1(Ember::Threading::MUTEX_FREE,
                              Ember::Threading(Ember::Threading::MUTEX_FREE).to_string(),
                              &mutex_free,
                              &T_SYSCALL_CTX));
        defs.add_back(define0(Ember::Threading::THREAD_GET_ID,
                              Ember::Threading(Ember::Threading::THREAD_GET_ID).to_string(),
                              &get_thread_ID,
                              &T_SYSCALL_CTX));
        defs.add_back(
            define1(Ember::Threading::THREAD_CONTROL_BLOCK_SET,
                    Ember::Threading(Ember::Threading::THREAD_CONTROL_BLOCK_SET).to_string(),
                    &set_thread_control_block,
                    &T_SYSCALL_CTX));
        return {"Threading", defs};
    }

    LinkedList<Bundle> system_call_get_native_bundles(KernelGuardian* k_guard) {
        LinkedList<Bundle> bundles;
        bundles.add_back(make_app_bundle(k_guard));
        bundles.add_back(make_vfs_bundle(k_guard));
        bundles.add_back(make_memory_bundle(k_guard));
        bundles.add_back(make_threading_bundle(k_guard));
        return bundles;
    }
} // namespace Rune::SystemCall
