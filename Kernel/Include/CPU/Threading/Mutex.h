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

#include <KRE/System/Resource.h>

#include <CPU/Threading/Scheduler.h>
#include <CPU/Threading/Spinlock.h>

namespace Rune::CPU {
    /// @brief A fair lock-free non-recursive mutex.
    ///
    /// Fairness is guaranteed when the mutex is unlocked, this means the next thread in the wait
    /// queue of the mutex will always acquire the mutex when it is unlocked.
    class Mutex : public Resource<MutexHandle> {
        int _lock = 0;

        Scheduler*                        _scheduler;
        SharedPointer<Thread>             _owner;
        Spinlock                          _wait_queue_lock;
        LinkedList<SharedPointer<Thread>> _wait_queue;

        void trace_state(const String& action);

      public:
        Mutex(MutexHandle handle, const String& name, Scheduler* scheduler);

        Mutex(const Mutex&)                    = delete;
        Mutex(Mutex&&)                         = delete;
        auto operator=(const Mutex&) -> Mutex& = delete;
        auto operator=(Mutex&&) -> Mutex&      = delete;

        /// @brief
        /// @return A reference to the thread owning the mutex.
        [[nodiscard]] auto get_owner() const -> Thread*;

        /// @brief
        /// @return A list of threads waiting for the mutex to be unlocked.
        [[nodiscard]] auto get_waiting_threads() const -> LinkedList<Thread*>;

        /// @brief Try to lock the mutex, if it is already locked the calling thread will be
        ///         blocked.
        ///
        /// It is the callers responsibility to not lock the mutex twice by the same thread, if done
        /// the thread will be deadlocked.
        void lock();

        /// @brief Try to unlock the mutex.
        ///
        /// Only the owner thread can unlock the mutex.
        void unlock();

        /// @brief Remove the owner thread or a waiting thread from the mutex.
        /// @param handle Handle of the thread to be removed.
        /// @return True: The thread is no longer maintained by the mutex, False: Otherwise.
        ///
        /// If the mutex has no owner, this function will return false without doing anything.
        /// When the owning thread should be removed, the owning thread is blocked and the mutex
        /// will be given to the next waiting thread. In case of a waiting thread, it will be simply
        /// removed from the wait queue.
        ///
        /// It is the callers responsibility to maintain the removed thread.
        ///
        /// Note: Use this function with care, because undefined behavior could occur as a result,
        ///         especially when removing the owner thread.
        auto remove_thread(MutexHandle handle) -> bool;
    };
} // namespace Rune::CPU

#endif // RUNEOS_MUTEX_H
