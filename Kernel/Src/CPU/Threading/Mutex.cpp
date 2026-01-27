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

    void Mutex::transfer_ownership() {
        _owner->mutex_handle = Resource<U16>::HANDLE_NONE;
        if (_wait_queue.is_empty()) {
            LOGGER->trace(R"(Mutex "{}-{}": Thread "{}-{}" unlocked mutex.)",
                          get_handle(),
                          get_name(),
                          _owner->handle,
                          _owner->name);
            _owner = SharedPointer<Thread>(nullptr);
        } else {
            auto waiting = *_wait_queue.head();
            _wait_queue.remove_front();
            LOGGER->trace(R"(Mutex "{}-{}": Thread "{}-{}" transferred ownership to "{}-{}".)",
                          get_handle(),
                          get_name(),
                          _owner->handle,
                          _owner->name,
                          waiting->handle,
                          waiting->name);
            _owner                = waiting;
            waiting->mutex_handle = get_handle();
            _scheduler->schedule(waiting);
        }
    }

    Mutex::Mutex(MutexHandle handle, String name, Scheduler* scheduler)
        : Resource(handle, name),
          _scheduler(scheduler),
          _wait_queue_lock(0, "", scheduler) {}

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
            calling_thread->state = ThreadState::PLANNED_WAIT;
            _wait_queue_lock.unlock();

            // Handle the lost wakeup problem
            // If the calling thread is no longer in the PLANNED_WAIT state, it means the owner
            // thread has unlocked the mutex in the meantime and this thread was "woken up"
            // But unlock also set _lock=0 therefore another thread might have locked the mutex in
            // the meantime, thus this thread needs to try to claim it again
            if (calling_thread->state != ThreadState::PLANNED_WAIT) continue;

            calling_thread->state = ThreadState::WAIT_MUTEX;
            _scheduler->execute_next_thread();
        }
        _owner = _scheduler->get_running_thread();
    }

    void Mutex::unlock() {
        auto calling_thread = _scheduler->get_running_thread();
        // Only the owning thread is allowed to unlock a mutex
        if (calling_thread->handle != _owner->handle) return;

        SharedPointer<Thread> thread_to_wake;
        _wait_queue_lock.lock();
        if (!_wait_queue.is_empty()) {
            thread_to_wake = *_wait_queue.head();
            _wait_queue.remove_front();
        }
        _owner = thread_to_wake;
        _wait_queue_lock.unlock();

        atomic_store_release(&_lock, 0);

        if (thread_to_wake) {
            _scheduler->schedule(thread_to_wake); // Set state to ThreadState::READY
        }

    }

    auto Mutex::remove_waiting_thread(U16 t_id) -> bool {
        _scheduler->lock();
        if (!_owner) return false;

        if (_owner->handle == t_id) {
            transfer_ownership();
        } else {
            SharedPointer<Thread> to_remove(nullptr);
            for (auto& waiting : _wait_queue) {
                if (waiting->handle == t_id) {
                    to_remove = waiting;
                    break;
                }
            }
            if (!to_remove) return false;
            _wait_queue.remove(to_remove);
        }
        _scheduler->unlock();
        return true;
    }
} // namespace Rune::CPU
