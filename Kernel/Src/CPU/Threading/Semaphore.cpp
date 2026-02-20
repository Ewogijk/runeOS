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

#include <CPU/Threading/Semaphore.h>

#include <KRE/Math.h>

#include <CPU/Threading/Atomic.h>
#include <CPU/Threading/LockGuard.h>
#include <CPU/Threading/MemoryBarrier.h>

namespace Rune::CPU {
    const SharedPointer<Logger> LOGGER = LogContext::instance().get_logger("CPU.Semaphore");

    void Semaphore::trace_state(const String& action) {
        String wq;
        for (auto& thread : _wait_queue) wq += String::format("{}, ", thread->get_unique_name());
        int c = _units;
        memory_barrier_read();
        LOGGER->trace(R"("{}: {}, C={}, WQ={})", get_unique_name(), action, _units, wq);
    }

    Semaphore::Semaphore(SemaphoreHandle handle,
                         const String&   name,
                         Scheduler*      scheduler,
                         int             counter_start,
                         int             counter_max)
        : Resource(handle, name),
          _units(counter_start),
          _unit_max(counter_max),
          _scheduler(scheduler),
          _lock(Resource<SpinlockHandle>::HANDLE_NONE,
                String::format("{}Lock", get_unique_name()),
                scheduler) {}

    auto Semaphore::get_available_units() const -> int { return _units; }

    auto Semaphore::get_unit_max() const -> int { return _unit_max; }

    auto Semaphore::get_waiting_threads() const -> LinkedList<Thread*> {
        LinkedList<Thread*> copy;
        for (auto& t : _wait_queue) copy.add_back(t.get());
        return copy;
    }


    void Semaphore::lock() {
        _lock.lock();
        while (_units <= 0) {
            auto calling_thread = _scheduler->get_running_thread();
            _wait_queue.add_back(calling_thread);
            _scheduler->await_block();
            trace_state("lock-fail");
            // Release the spinlock for the block duration to allow threads holding the semaphore
            // to unlock it or other threads to try to lock it
            _lock.unlock();
            _scheduler->block();
            _lock.lock();
        }
        --_units;
        _scheduler->get_running_thread()->semaphore_handle = get_handle();
        trace_state("lock-good");
        _lock.unlock();
    }

    auto Semaphore::try_lock() -> bool {
        LockGuard<Spinlock> lg(_lock);
        if (_units <= 0) {
            trace_state("try_lock-fail");
            return false;
        }
        --_units;
        trace_state("try_lock-good");
        return true;
    }

    void Semaphore::unlock() {
        LockGuard<Spinlock> lg(_lock);
        if (_units >= _unit_max) return;
        _units++;
        SharedPointer<Thread> thread_to_wake;
        if (!_wait_queue.is_empty()) {
            thread_to_wake = *_wait_queue.head();
            _wait_queue.remove_front();
        }
        _scheduler->get_running_thread()->semaphore_handle = Resource<SemaphoreHandle>::HANDLE_NONE;
        _scheduler->unblock(thread_to_wake);
        _units = _units;
        memory_barrier_write();
        trace_state("unlock");
    }

} // namespace Rune::CPU
