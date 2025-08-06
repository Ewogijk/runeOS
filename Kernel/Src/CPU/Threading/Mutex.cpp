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


namespace Rune::CPU {
    constexpr char const* File = "Mutex";


    void Mutex::transfer_ownership() {
        _owner->mutex_id = 0;
        if (_wait_queue.is_empty()) {
            _logger->trace(
                    File,
                    R"(Mutex "{}-{}": Thread "{}-{}" unlocked mutex.)",
                    handle,
                    name,
                    _owner->handle,
                    _owner->name
            );
            _owner = SharedPointer<Thread>(nullptr);
        } else {
            auto waiting = *_wait_queue.head();
            _wait_queue.remove_front();
            _logger->trace(
                    File,
                    R"(Mutex "{}-{}": Thread "{}-{}" transferred ownership to "{}-{}".)",
                    handle,
                    name,
                    _owner->handle,
                    _owner->name,
                    waiting->handle,
                    waiting->name
            );
            _owner = waiting;
            waiting->mutex_id = handle;
            _scheduler->schedule(waiting);
        }
    }


    Mutex::Mutex() : _scheduler(nullptr), _logger(nullptr), handle(0), name("") {

    }


    Mutex::Mutex(Scheduler* scheduler, SharedPointer<Logger> logger, String name)
            : _scheduler(scheduler),
              _logger(move(logger)),
              handle(0),
              name(move(name)) {

    }


    Thread* Mutex::get_owner() const {
        return _owner ? _owner.get() : nullptr;
    }


    LinkedList<Thread*> Mutex::get_waiting_threads() const {
        LinkedList<Thread*> copy;
        for (auto& t: _wait_queue)
            copy.add_back(t.get());
        return copy;
    }


    void Mutex::lock() {
        _scheduler->lock();
        auto t = _scheduler->get_running_thread();
        t->mutex_id = handle;
        if (!_owner) {
            _logger->trace(File, R"(Mutex "{}-{}": Thread "{}-{}" acquired mutex.)", handle, name, t->handle, t->name);
            _owner = t;
            _scheduler->unlock();
            return;
        }

        if (t->handle != _owner->handle) {
            _logger->trace(
                    File,
                    R"(Mutex "{}-{}": Thread "{}-{}" is put in wait queue.)",
                    handle,
                    name,
                    t->handle,
                    t->name
            );
            _wait_queue.add_back(t);
            t->state = ThreadState::WAITING;
            _scheduler->execute_next_thread();
        } // else Allow the owner to lock the mutex recursively
        _scheduler->unlock();
    }


    void Mutex::unlock() {
        _scheduler->lock();
        if (!_owner) {
            _scheduler->unlock();
            return;
        }

        // Only the owner is allowed to unlock the mutex
        if (_scheduler->get_running_thread()->handle == _owner->handle) {
            transfer_ownership();
            if (_scheduler->get_ready_queue()->peek() == _owner.get())
                _scheduler->execute_next_thread();    // Execute the thread immediately if it is first in the ready queue
        }
        _scheduler->unlock();
    }


    bool Mutex::remove_waiting_thread(int t_id) {
        _scheduler->lock();
        if (!_owner)
            return false;

        if (_owner->handle == t_id) {
            transfer_ownership();
        } else {
            SharedPointer<Thread> to_remove(nullptr);
            for (auto& waiting: _wait_queue) {
                if (waiting->handle == t_id) {
                    to_remove = waiting;
                    break;
                }
            }
            if (!to_remove)
                return false;
            _wait_queue.remove(to_remove);
        }
        _scheduler->unlock();
        return true;
    }
}
