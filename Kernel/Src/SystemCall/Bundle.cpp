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

#include <SystemCall/KernelGuardian.h>
#include <SystemCall/AppManagement.h>
#include <SystemCall/VFS.h>
#include <SystemCall/MemoryManagement.h>
#include <SystemCall/ThreadManagement.h>

#include <Device/DeviceSubsystem.h>

#include <CPU/CPUSubsystem.h>

#include <VirtualFileSystem//VFSSubsystem.h>


namespace Rune::SystemCall {
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          App Management Systemcalls
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    AppManagementContext APP_MNG_CTX;


    Bundle make_app_management_bundle(
            KernelGuardian* k_guard,
            const LibK::SubsystemRegistry& k_subsys_reg
    ) {
        auto* app_subsys    = k_subsys_reg.get_as<App::Subsystem>(LibK::KernelSubsystem::APP);
        auto* device_subsys = k_subsys_reg.get_as<Device::Subsystem>(LibK::KernelSubsystem::DEVICE);
        auto* cpu_subsys    = k_subsys_reg.get_as<CPU::Subsystem>(LibK::KernelSubsystem::CPU);
        auto* file_subsys   = k_subsys_reg.get_as<VFS::Subsystem>(LibK::KernelSubsystem::VFS);
        APP_MNG_CTX = {
                k_guard,
                app_subsys,
                device_subsys,
                cpu_subsys,
                file_subsys
        };

        LinkedList<Definition> defs;
        defs.add_back(define1(200, "read_std_in", &read_std_in, &APP_MNG_CTX));
        defs.add_back(define1(201, "write_std_out", &write_std_out, &APP_MNG_CTX));
        defs.add_back(define1(202, "write_std_err", &write_std_err, &APP_MNG_CTX));
        defs.add_back(define6(203, "app_start", &app_start, &APP_MNG_CTX));
        defs.add_back(define1(204, "app_exit", &app_exit, &APP_MNG_CTX));
        defs.add_back(define1(205, "app_join", &app_join, &APP_MNG_CTX));
        defs.add_back(define2(206, "app_get_working_directory", &app_get_working_directory, &APP_MNG_CTX));
        defs.add_back(define1(207, "app_change_working_directory", &app_change_working_directory, &APP_MNG_CTX));

        return {
                "App Management",
                defs
        };
    }


    VFSContext VFS_CTX;


    Bundle make_vfs_bundle(
            KernelGuardian* k_guard,
            const LibK::SubsystemRegistry& k_subsys_reg
    ) {
        VFS_CTX = {
                k_guard,
                k_subsys_reg.get_as<VFS::Subsystem>(LibK::KernelSubsystem::VFS),
                k_subsys_reg.get_as<App::Subsystem>(LibK::KernelSubsystem::APP)
        };

        LinkedList<Definition> defs;

        defs.add_back(define2(100, "vfs_get_node_info", &vfs_get_node_info, &VFS_CTX));
        defs.add_back(define2(101, "vfs_create", &vfs_create, &VFS_CTX));
        defs.add_back(define2(102, "vfs_open", &vfs_open, &VFS_CTX));
        defs.add_back(define1(103, "vfs_delete", &vfs_delete, &VFS_CTX));
        defs.add_back(define1(104, "vfs_close", &vfs_close, &VFS_CTX));
        defs.add_back(define3(105, "vfs_read", &vfs_read, &VFS_CTX));
        defs.add_back(define3(106, "vfs_write", &vfs_write, &VFS_CTX));
        defs.add_back(define3(107, "vfs_seek", &vfs_seek, &VFS_CTX));
        defs.add_back(define1(108, "vfs_directory_stream_open", &vfs_directory_stream_open, &VFS_CTX));
        defs.add_back(define2(109, "vfs_directory_stream_next", &vfs_directory_stream_next, &VFS_CTX));
        defs.add_back(define1(110, "vfs_directory_stream_close", &vfs_directory_stream_close, &VFS_CTX));
        return {
                "VFS",
                defs
        };
    }


    MemoryManagementContext MM_CTX;


    Bundle make_mm_bundle(
            KernelGuardian* k_guard,
            const LibK::SubsystemRegistry& k_subsys_reg
    ) {
        MM_CTX = {
                k_guard,
                k_subsys_reg.get_as<Memory::Subsystem>(LibK::KernelSubsystem::MEMORY),
                k_subsys_reg.get_as<App::Subsystem>(LibK::KernelSubsystem::APP)
        };

        LinkedList<Definition> defs;
        defs.add_back(define0(0, "memory_get_page_size", &memory_get_page_size, &MM_CTX));
        defs.add_back(define3(1, "memory_allocate_page", &memory_allocate_page, &MM_CTX));
        defs.add_back(define2(2, "memory_free_page", &memory_free_page, &MM_CTX));
        return {
                "MemoryManagement",
                defs
        };
    }


    ThreadManagementContext TM_CTX;


    Bundle make_tm_bundle(
            KernelGuardian* k_guard,
            const LibK::SubsystemRegistry& k_subsys_reg
    ) {
        TM_CTX = {
                k_guard,
                k_subsys_reg.get_as<CPU::Subsystem>(LibK::KernelSubsystem::CPU),
                k_subsys_reg.get_as<App::Subsystem>(LibK::KernelSubsystem::APP)
        };

        LinkedList<Definition> defs;
        defs.add_back(define1(300, "mutex_create", &mutex_create, &TM_CTX));
        defs.add_back(define1(301, "mutex_lock", &mutex_lock, &TM_CTX));
        defs.add_back(define1(302, "mutex_unlock", &mutex_unlock, &TM_CTX));
        defs.add_back(define1(303, "mutex_release", &mutex_release, &TM_CTX));
        defs.add_back(define0(304, "get_thread_ID", &get_thread_ID, &TM_CTX));
        defs.add_back(define1(305, "get_thread_control_block", &get_thread_control_block, &TM_CTX));
        defs.add_back(define1(306, "set_thread_control_block", &set_thread_control_block, &TM_CTX));
        return {
                "ThreadManagement",
                defs
        };
    }


    LinkedList<Bundle> system_call_get_native_bundles(
            KernelGuardian* k_guard,
            const LibK::SubsystemRegistry& k_subsys_reg
    ) {
        LinkedList<Bundle> bundles;
        bundles.add_back(make_app_management_bundle(k_guard, k_subsys_reg));
        bundles.add_back(make_vfs_bundle(k_guard, k_subsys_reg));
        bundles.add_back(make_mm_bundle(k_guard, k_subsys_reg));
        bundles.add_back(make_tm_bundle(k_guard, k_subsys_reg));
        return bundles;
    }
}