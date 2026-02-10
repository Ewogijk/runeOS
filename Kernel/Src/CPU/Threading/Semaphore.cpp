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
        int c = _counter;
        memory_barrier_read();
        LOGGER->trace(R"("{}: {}, C={}, WQ={})", get_unique_name(), action, _counter, wq);
    }

    Semaphore::Semaphore(SemaphoreHandle handle,
                         const String&   name,
                         Scheduler*      scheduler,
                         int             counter_start,
                         int             counter_max)
        : Resource(handle, name),
          _counter(counter_start),
          _counter_max(counter_max),
          _scheduler(scheduler),
          _lock(Resource<SpinlockHandle>::HANDLE_NONE,
                String::format("{}Lock", get_unique_name()),
                scheduler) {}

    void Semaphore::lock() {
        _lock.lock();
        while (_counter <= 0) {
            auto calling_thread = _scheduler->get_running_thread();
            _wait_queue.add_back(calling_thread);
            _scheduler->await_block();
            trace_state("wait");
            // Release the spinlock for the block duration to allow threads holding the semaphore
            // to unlock it or other threads to try to lock it
            _lock.unlock();
            _scheduler->block();
            _lock.lock();
        }
        --_counter;
        _scheduler->get_running_thread()->semaphore_handle = get_handle();
        trace_state("acquire");
        _lock.unlock();
    }

    void Semaphore::unlock() {
        LockGuard<Spinlock> lock(_lock);
        // _lock.lock();
        _counter = min(_counter + 1, _counter_max);
        SharedPointer<Thread> thread_to_wake;
        if (!_wait_queue.is_empty()) {
            thread_to_wake = *_wait_queue.head();
            _wait_queue.remove_front();
        }
        _scheduler->get_running_thread()->semaphore_handle = Resource<SemaphoreHandle>::HANDLE_NONE;
        _scheduler->unblock(thread_to_wake);
        _counter = _counter;
        memory_barrier_write();
        trace_state("unlock");
        // _lock.unlock();
    }

} // namespace Rune::CPU
