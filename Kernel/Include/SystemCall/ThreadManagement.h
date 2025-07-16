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

#ifndef RUNEOS_THREADMANAGEMENT_H
#define RUNEOS_THREADMANAGEMENT_H

#include <SystemCall/KernelGuardian.h>

#include <CPU/CPUSubsystem.h>

#include <App/AppSubsystem.h>


namespace Rune::SystemCall {

    /**
     * @brief Information about a thread for user space.
     */
    struct ThreadControlBlock {
        U16 thread_ID;
        void* stack_addr;
        size_t stack_size;
    };


    /**
     * @brief The context for all threading related system calls.
     */
    struct ThreadManagementContext {
        KernelGuardian* k_guard    = nullptr;
        CPU::Subsystem* cpu_subsys = nullptr;
        App::Subsystem* app_subsys = nullptr;
    };


    /**
     * If the mutex_name is an empty string then the kernel will choose a name for it.
     *
     * @brief Create mutex with the requested name.
     * @param sys_call_ctx A pointer to the thread management context.
     * @param mutex_name   Name of the mutex.
     * @return >0: Handle to the mutex.
     *          -1: The mutex name is null or exceeds the maximum length of 128 bytes.
     *          -2: Failed to create the mutex.
     */
    S64 mutex_create(void* sys_call_ctx, U64 mutex_name);


    /**
     * If the mutex is already locked the system call will block the calling thread until the mutex is unlocked.
     *
     * @brief Lock the mutex with the requested handle.
     * @param sys_call_ctx A pointer to the thread management context.
     * @param handle       The handle of a mutex.
     * @return 0: The mutex got locked
     *          -1: The handle is zero.
     *          -2: No mutex with the requested handle was found.
     */
    S64 mutex_lock(void* sys_call_ctx, U64 handle);


    /**
     * If the mutex is not locked by the calling thread then this system call will do nothing.
     *
     * @brief Unlock the mutex with the requested handle.
     * @param sys_call_ctx A pointer to the thread management context.
     * @param handle       The handle of a mutex.
     * @return 0: The mutex got locked
     *          -1: The handle is zero.
     *          -2: No mutex with the requested handle was found.
     */
    S64 mutex_unlock(void* sys_call_ctx, U64 handle);


    /**
     * @brief Free all resources associated with the mutex with the requested handle.
     * @param sys_call_ctx A pointer to the thread management context.
     * @param handle       The handle of a mutex.
     * @return 0: The mutex got locked
     *          -1: The handle is zero.
     *          -2: Failed to release the mutex.
     */
    S64 mutex_release(void* sys_call_ctx, U64 handle);


    /**
     * @brief Get the ID of the currently running thread.
     * @param sys_call_ctx A pointer to the thread management context.
     * @param ID_out       U16 pointer where the thread ID will be put.
     * @return 0: Success.
     *          -1: The ID_out buffer is null or in kernel memory.
     */
    S64 get_thread_ID(void* sys_call_ctx, U64 ID_out);


    /**
     * @brief Get the thread control block of the currently running thread.
     * @param sys_call_ctx A pointer to the thread management context.
     * @param tcb_out      Thread control block buffer.
     * @return 0: Success.
     *          -1: The tcb_out buffer is null or in kernel memory.
     */
    S64 get_thread_control_block(void* sys_call_ctx, U64 tcb_out);
}

#endif //RUNEOS_THREADMANAGEMENT_H
