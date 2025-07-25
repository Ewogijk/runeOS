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

#include <Pickaxe/SystemCall.h>

namespace Rune::Pickaxe {
    /**
     * @brief Information about a thread for user space.
     */
    struct ThreadControlBlock {
        U16 thread_ID;
        void* stack_addr;
        size_t stack_size;
    };

    /**
    * If the mutex_name is an empty string then the kernel will choose a name for it.
    *
    * @brief Create mutex with the requested name.
    * @param mutex_name   Name of the mutex.
    * @return >0: Handle to the mutex.
    *          -1: The mutex name is null or exceeds the maximum length of 128 bytes.
    *          -2: Failed to create the mutex.
    */
    int mutex_create(const char* mutex_name);


    /**
     * If the mutex is already locked the system call will block the calling thread until the mutex is unlocked.
     *
     * @brief Lock the mutex with the requested handle.
     * @param handle       The handle of a mutex.
     * @return 0: The mutex got locked
     *          -1: The handle is zero.
     *          -2: No mutex with the requested handle was found.
     */
    int mutex_lock(U16 handle);


    /**
     * If the mutex is not locked by the calling thread then this system call will do nothing.
     *
     * @brief Unlock the mutex with the requested handle.
     * @param handle       The handle of a mutex.
     * @return 0: The mutex got locked
     *          -1: The handle is zero.
     *          -2: No mutex with the requested handle was found.
     */
    int mutex_unlock(U16 handle);


    /**
     * @brief Free all resources associated with the mutex with the requested handle.
     * @param handle       The handle of a mutex.
     * @return 0: The mutex got locked
     *          -1: The handle is zero.
     *          -2: Failed to release the mutex.
     */
    int mutex_release(U16 handle);


    /**
     * @brief Get the ID of the currently running thread.
     * @param ID_out       U16 pointer where the thread ID will be put.
     * @return >0: Thread ID.
     *          -1: The buffer is null or in kernel memory.
     */
    int get_thread_ID();


    /**
     * @brief Get the thread control block of the currently running thread.
     * @param tcb_out      Thread control block.
     * @return 0: Success.
     *          -1: The tcb_out buffer is null or in kernel memory.
     */
    S64 get_thread_control_block(ThreadControlBlock& tcb_out);
}

#endif //RUNEOS_THREADMANAGEMENT_H
