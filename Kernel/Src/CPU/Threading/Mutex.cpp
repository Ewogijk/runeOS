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

#include <CPU/Threading/Mutex.h>

#include <CPU/Threading/Atomic.h>
#include <CPU/Threading/CriticalSection.h>

namespace Rune::CPU {
    const SharedPointer<Logger> LOGGER = LogContext::instance().get_logger("CPU.Mutex");

    void Mutex::trace_state(const String& action) {
        String wq;
        for (auto& thread : _wait_queue) wq += String::format("{}, ", thread->get_unique_name());
        LOGGER->trace(R"("{}: {}, O={}, WQ={})",
                      get_unique_name(),
                      action,
                      _owner ? _owner->get_unique_name() : "",
                      wq);
    }

    Mutex::Mutex(MutexHandle handle, const String& name) : Resource(handle, name) {}

    auto Mutex::get_owner() const -> Thread* { return _owner ? _owner.get() : nullptr; }

    auto Mutex::get_waiting_threads() const -> LinkedList<Thread*> {
        LinkedList<Thread*> copy;
        for (auto& t : _wait_queue) copy.add_back(t.get());
        return copy;
    }

    void Mutex::lock() {
        // Retry loop for mutex locking
        // Reasoning: After a thread is woken up it must try to lock again and not assume it can
        //              simply claim the mutex, because another thread could swoop in and claim the
        //              lock before the woken thread comes to atomic_compare_exchange.
        while (!atomic_compare_exchange_acquire(&_lock, 0, 1)) {
            auto calling_thread = g_scheduler.get_running_thread();
            // _wait_queue is a non synchronized linkedlist -> need spinlock protection here
            // Could also use lock-free queue implementation. Is maybe better?
            {
                CriticalSection<Spinlock> lock(_wait_queue_lock);
                _wait_queue.add_back(calling_thread);
                g_scheduler.mark_as_block_pending();
            }
            trace_state("lock-fail");
            // await_block()/block() mechanic solves the lost wakeup problem
            g_scheduler.block();

            // Check if fast handoff to calling thread was done in unlock()
            // If yes, just return because _lock == 1 and _owner == calling_thread
            if (_owner->get_handle() == calling_thread->get_handle()) {
                trace_state("lock-fast-handoff");
                return;
            }
        }
        _owner = g_scheduler.get_running_thread();
        trace_state("lock-good");
    }

    auto Mutex::try_lock() -> bool {
        if (!atomic_compare_exchange_acquire(&_lock, 0, 1)) {
            trace_state("try_lock-fail");
            return false;
        }
        _owner = g_scheduler.get_running_thread();
        trace_state("try_lock-good");
        return true;
    }

    void Mutex::unlock() {
        auto& calling_thread = g_scheduler.get_running_thread();
        // Only the owning thread is allowed to unlock the mutex
        if (!_owner) return;
        if (calling_thread->get_handle() != _owner->get_handle()) return;

        SharedPointer<Thread> thread_to_wake;
        {
            CriticalSection<Spinlock> lock(_wait_queue_lock);
            if (!_wait_queue.empty()) {
                thread_to_wake = _wait_queue.remove_front().value();
            }
            _owner = thread_to_wake;
        }
        // Perform fast handoff if the mutex has a new owner, otherwise unlock it
        if (!_owner) atomic_store_release(&_lock, 0);
        trace_state("unlock");
        g_scheduler.unblock(thread_to_wake);
    }

    auto Mutex::remove_thread(MutexHandle handle) -> bool {
        if (atomic_load_acquire(&_lock) == 0) return false;

        if (_owner->get_handle() == handle) {
            g_scheduler.block(_owner);
            unlock();
        } else {
            CriticalSection<Spinlock> _(_wait_queue_lock);
            SharedPointer<Thread> to_remove;
            for (auto& waiting : _wait_queue) {
                if (waiting->get_handle() == handle) {
                    to_remove = waiting;
                    break;
                }
            }
            if (!to_remove) return false;
            _wait_queue.remove(to_remove);
        }
        return true;
    }

    auto mutex_row_converter(const SharedPointer<Mutex>& mutex) -> Array<String, 3> {
        Thread* owner           = mutex->get_owner();
        String  waiting_threads = "";
        for (auto& t : mutex->get_waiting_threads()) waiting_threads += t->get_unique_name();
        if (waiting_threads.is_empty()) waiting_threads = "-";
        return {mutex->get_unique_name(),
                owner != nullptr ? owner->get_unique_name() : "-",
                waiting_threads};
    }

    ResourceCache<Mutex, 3> g_mutex_cache({"ID-Name", "Owner", "WaitQueue"}, &mutex_row_converter);
} // namespace Rune::CPU
