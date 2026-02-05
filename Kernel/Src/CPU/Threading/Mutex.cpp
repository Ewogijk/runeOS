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

#include "CPU/Threading/Atomic.h"

#include <CPU/Threading/Mutex.h>

namespace Rune::CPU {
    const SharedPointer<Logger> LOGGER = LogContext::instance().get_logger("CPU.Mutex");

    void Mutex::trace_state(const String& action) {
        String wq;
        for (auto& thread : _wait_queue)
            wq += String::format("{}-{}, ", thread->handle, thread->name);
        LOGGER->trace(R"(": {}, O={}-{}, WQ={})",
                      get_unique_name(),
                      action,
                      _owner ? _owner->handle : Resource<MutexHandle>::HANDLE_NONE,
                      _owner ? _owner->name : "",
                      wq);
    }

    Mutex::Mutex(MutexHandle handle, String name, Scheduler* scheduler)
        : Resource(handle, name),
          _scheduler(scheduler),
          _wait_queue_lock(Resource<SpinlockHandle>::HANDLE_NONE,
                           String::format("{}WQLock", get_unique_name()),
                           scheduler) {}

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
            auto calling_thread = _scheduler->get_running_thread();
            // _wait_queue is a non synchronized linkedlist -> need spinlock protection here
            // Could also use lock-free queue implementation. Is maybe better?
            _wait_queue_lock.lock();
            _wait_queue.add_back(calling_thread);
            _scheduler->await_block();
            trace_state("try lock");
            _wait_queue_lock.unlock();
            // await_block()/block() mechanic solves the lost wakeup problem
            _scheduler->block();

            // Check if fast handoff to calling thread was done in unlock()
            // If yes, just return because _lock == 1 and _owner == calling_thread
            if (_owner->handle == calling_thread->handle) return;
        }
        _owner = _scheduler->get_running_thread();
        trace_state("lock");
    }

    void Mutex::unlock() {
        auto calling_thread = _scheduler->get_running_thread();
        // Only the owning thread is allowed to unlock the mutex
        if (calling_thread->handle != _owner->handle) return;

        SharedPointer<Thread> thread_to_wake;
        _wait_queue_lock.lock();
        if (!_wait_queue.is_empty()) {
            thread_to_wake = *_wait_queue.head();
            _wait_queue.remove_front();
        }
        _owner = thread_to_wake;
        trace_state("unlock");
        _wait_queue_lock.unlock();

        // Perform fast handoff if the mutex has a new owner, otherwise unlock it
        if (!_owner) atomic_store_release(&_lock, 0);
        _scheduler->unblock(thread_to_wake);
    }

    auto Mutex::remove_thread(MutexHandle handle) -> bool {
        if (atomic_load_acquire(&_lock) == 0) return false;

        if (_owner->handle == handle) {
            _scheduler->block(_owner);
            unlock();
        } else {
            SharedPointer<Thread> to_remove;
            _wait_queue_lock.lock();
            for (auto& waiting : _wait_queue) {
                if (waiting->handle == handle) {
                    to_remove = waiting;
                    break;
                }
            }
            if (!to_remove) return false;
            _wait_queue.remove(to_remove);
            _wait_queue_lock.unlock();
        }
        return true;
    }
} // namespace Rune::CPU
