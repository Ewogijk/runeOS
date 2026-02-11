
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

#ifndef RUNEOS_SEMAPHORE_H
#define RUNEOS_SEMAPHORE_H

#include <KRE/System/Resource.h>

#include <CPU/CPU.h>
#include <CPU/Threading/Spinlock.h>

namespace Rune::CPU {

    /// @brief A counting semaphore using a spinlock for synchronization.
    ///
    /// The semaphore keeps track of a maximum value for the counter, it cannot be incremented
    /// beyond this value.
    class Semaphore : public Resource<SemaphoreHandle> {
        int _counter     = 0;
        int _counter_max = 0;

        Scheduler*                        _scheduler;
        Spinlock                          _lock;
        LinkedList<SharedPointer<Thread>> _wait_queue;

        /// @brief Trace the count and wait queue upon an action e.g. lock.
        /// @param action
        void trace_state(const String& action);

      public:
        Semaphore(SemaphoreHandle handle,
                  const String&   name,
                  Scheduler*      scheduler,
                  int             counter_start,
                  int             counter_max);

        Semaphore(const Semaphore&)                    = delete;
        Semaphore(Semaphore&&)                         = delete;
        auto operator=(const Semaphore&) -> Semaphore& = delete;
        auto operator=(Semaphore&&) -> Semaphore&      = delete;

        /// @brief Try to decrease the counter of the semaphore.
        ///
        /// When the counter has reached zero the calling thread will be blocked until the counter
        /// has been increased by a call to `unlock()`.
        void lock();

        /// @brief Try to lock the semaphore and return immediately.
        /// @return True: The mutex has been locked, False: Otherwise.
        auto try_lock() -> bool;

        /// @brief Try to increment the counter and unblock a waiting thread.
        ///
        /// If the counter has reached the maximum value this function will return immediately
        /// without unblocking any waiting threads.
        void unlock();
    };
} // namespace Rune::CPU

#endif // RUNEOS_SEMAPHORE_H
