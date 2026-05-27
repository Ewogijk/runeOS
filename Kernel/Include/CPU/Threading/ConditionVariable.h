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

#ifndef RUNEOS_CONDITIONVARIABLE_H
#define RUNEOS_CONDITIONVARIABLE_H

#include <KRE/Collections/LinkedList.h>
#include <KRE/Utility.h>

#include <CPU/Threading/CriticalSection.h>
#include <CPU/Threading/Mutex.h>
#include <CPU/Threading/Scheduler.h>
#include <CPU/Threading/Spinlock.h>

namespace Rune::CPU {
    /// @brief A condition variable is a synchronization primitive that blocks one or more threads
    ///         until the condition variable is notified.
    class ConditionVariable {
        Spinlock                          m_spinlock;
        LinkedList<SharedPointer<Thread>> m_waiters;

        /// @brief Wake the first thread in the waiting list.
        void wake_one();

      public:
        ConditionVariable();

        /// @brief
        /// @return A list of threads waiting for the condition variable to be notified.
        [[nodiscard]] auto get_waiting_threads() const -> LinkedList<Thread*>;

        /// @brief Block the calling thread until the condition variable is notified.
        /// @param mutex Mutex that was locked by the calling thread.
        void wait(Mutex& mutex);

        /// @brief Block the calling thread until the condition variable is notified and predicate
        /// returns true.
        /// @param mutex Mutex that was locked by the calling thread.
        /// @param predicate Predicate to check wether the waiting can be completed.
        void wait(Mutex& mutex, Function<bool()> predicate);

        /// @brief Unblock a waiting thread.
        void notify_one();

        /// @brief Unblock all waiting threads.
        void notify_all();
    };
} // namespace Rune::CPU

#endif // RUNEOS_CONDITIONVARIABLE_H
