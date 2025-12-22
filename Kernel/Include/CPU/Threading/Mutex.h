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

#ifndef RUNEOS_MUTEX_H
#define RUNEOS_MUTEX_H

#include <CPU/Threading/Scheduler.h>

namespace Rune::CPU {
    /**
     * A recursive mutex implementation.
     */
    class Mutex {
        Scheduler* _scheduler;

        SharedPointer<Thread>             _owner;
        LinkedList<SharedPointer<Thread>> _wait_queue;

        void transfer_ownership();

      public:
        // Per requirement of the "Column::make_handle_column_table" these properties must be
        // publicly accessible
        U16    handle; // NOLINT
        String name;   // NOLINT

        // Per definition of the "ResourceTable" a default constructor must be provided
        Mutex();

        Mutex(Scheduler* scheduler, String name);

        /**
         * @brief The thread that is currently locking the mutex.
         * @return Owner thread of the mutex or nullptr if the mutex is not locked.
         */
        [[nodiscard]] auto get_owner() const -> Thread*;

        /**
         * @brief All threads that are waiting for the mutex to be unlocked.
         * @return A copy of the mutexes wait queue.
         */
        [[nodiscard]] auto get_waiting_threads() const -> LinkedList<Thread*>;

        /**
         * @brief Try to lock the mutex.
         *
         * If it is not locked yet then the running thread will acquire the mutex. If the mutex is
         * already locked then all threads other then the owner of the lock will put into a wait
         * queue, while the owner of the thread  is allowed to lock the mutex multiple times.
         */
        void lock();

        /**
         * Unlock the mutex if the calling thread is the owner of the mutex then ownership will be
         * transferred to the next thread in the waiting queue and the thread is woken up. Otherwise
         * nothing will happen.
         */
        void unlock();

        /**
         * @brief Search for a thread with the given ID in the waiting queue and remove it if found.
         *
         * <p>
         *  If the thread was the owner of the mutex, then ownership will be transferred to the next
         * thread in the queue and the thread will be scheduled. If the thread was the only owner of
         * the mutex then it will simply be unlocked.
         * </p>
         * <p>
         *  If the thread was not the owner of the mutex but simply waiting in the queue, then it
         * will just be removed from the queue without any ownership transfer.
         * </p>
         * @param t_id
         * @return True: The thread got removed from the wait queue., False: No thread with the
         * requested ID was found.
         */
        auto remove_waiting_thread(U16 t_id) -> bool;
    };
} // namespace Rune::CPU

#endif // RUNEOS_MUTEX_H
