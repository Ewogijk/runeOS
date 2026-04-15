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

#include <CPU/Threading/ConditionVariable.h>

#include <KRE/System/System.h>

namespace Rune::CPU {
    void ConditionVariable::wake_one() {
        SharedPointer<Thread> thread = *m_waiters.head();
        m_waiters.remove_front();
        m_scheduler->unblock(thread);
    }

    ConditionVariable::ConditionVariable(Scheduler* scheduler)
        : m_scheduler(scheduler),
          m_spinlock(Resource<MutexHandle>::HANDLE_NONE, "CV", scheduler) {}

    auto ConditionVariable::get_waiting_threads() const -> LinkedList<Thread*> {
        LinkedList<Thread*> copy;
        for (auto& t : m_waiters) copy.add_back(t.get());
        return copy;
    }

    void ConditionVariable::wait() {
        {
            CriticalSection<Spinlock> _(m_spinlock);
            m_waiters.add_back(m_scheduler->get_running_thread());
        }
        m_scheduler->await_block();
        m_scheduler->block();
    }

    void ConditionVariable::notify_one() {
        CriticalSection<Spinlock> _(m_spinlock);
        if (m_waiters.is_empty()) return;
        wake_one();
    }

    void ConditionVariable::notify_all() {
        CriticalSection<Spinlock> _(m_spinlock);
        while (!m_waiters.is_empty()) wake_one();
    }
} // namespace Rune::CPU
