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
     * @return >0:       Mutex ID.<br>
     *          BAD_ARG: The mutex name is null or exceeds the string size limit.<br>
     *          FAULT:   Failed to create the mutex.
     */
    S64 mutex_create(void* sys_call_ctx, U64 mutex_name);


    /**
     * If the mutex is already locked the system call will block the calling thread until the mutex is unlocked.
     *
     * @brief Lock the mutex with the requested ID.
     * @param sys_call_ctx A pointer to the thread management context.
     * @param ID           The ID of a mutex.
     * @return OKAY:     The mutex got locked<br>
     *          BAD_ARG: The ID is zero.<br>
     *          UNKNOWN_ID:  No mutex with the requested ID was found.
     */
    S64 mutex_lock(void* sys_call_ctx, U64 ID);


    /**
     * If the mutex is not locked by the calling thread then this system call will do nothing.
     *
     * @brief Unlock the mutex with the requested ID.
     * @param sys_call_ctx A pointer to the thread management context.
     * @param ID           The ID of a mutex.
     * @return OKAY:     The mutex got locked<br>
     *          BAD_ARG: The ID is zero.<br>
     *          UNKNOWN_ID:  No mutex with the requested ID was found.
     */
    S64 mutex_unlock(void* sys_call_ctx, U64 ID);


    /**
     * @brief Free all resources associated with the requested mutex.
     * @param sys_call_ctx A pointer to the thread management context.
     * @param ID           The ID of a mutex.
     * @return OKAY:     The mutex got locked.<br>
     *          BAD_ARG: The ID is zero.<br>
     *          UNKNOWN_ID:  Failed to release the mutex.
     */
    S64 mutex_release(void* sys_call_ctx, U64 ID);


    /**
     * @brief Get the ID of the currently running thread.
     * @param sys_call_ctx A pointer to the thread management context.
     * @return Success: The thread ID.
     */
    S64 get_thread_ID(void* sys_call_ctx);


    /**
     * @brief Set the current thread's thread control block.
     * @param sys_call_ctx A pointer to the thread management context.
     * @param tcb          A pointer to the thread control block.
     * @return OKAY:     Success.<br>
     *          BAD_ARG: The tcb buffer is null or in kernel memory.
     */
    S64 set_thread_control_block(void* sys_call_ctx, U64 tcb);
}

#endif //RUNEOS_THREADMANAGEMENT_H
