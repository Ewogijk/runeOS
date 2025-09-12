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

#include <CPU/Time/DeltaQueue.h>

namespace Rune::CPU {
    DeltaQueue::DeltaQueue() : _first(nullptr), _last(nullptr) {}

    DQNode* DeltaQueue::first() const { return _first; }

    void DeltaQueue::update_wake_time(U64 time_decrement) {
        if (_first) _first->wake_time -= time_decrement;
    }

    void DeltaQueue::enqueue(const SharedPointer<Thread>& thread, U64 wake_time) {
        if (!thread) return;

        auto* new_dq_node            = new DQNode();
        new_dq_node->sleeping_thread = thread;
        new_dq_node->wake_time       = wake_time;
        if (_first == nullptr) {
            _first = _last = new_dq_node;
        } else {
            auto* c_dq_node = _first;
            while (c_dq_node != nullptr) {
                if (new_dq_node->wake_time >= c_dq_node->wake_time) {
                    new_dq_node->wake_time -= c_dq_node->wake_time;
                } else {
                    if (c_dq_node->prev == nullptr) {
                        c_dq_node->prev   = new_dq_node;
                        new_dq_node->next = _first;
                        _first            = new_dq_node;
                    } else {
                        c_dq_node->prev->next = new_dq_node;
                        new_dq_node->prev     = c_dq_node->prev;
                        c_dq_node->prev       = new_dq_node;
                        new_dq_node->next     = c_dq_node;
                    }
                    // Update next nodes wake times
                    auto* ccDQNode = new_dq_node->next;
                    while (ccDQNode != nullptr) {
                        ccDQNode->wake_time -= new_dq_node->wake_time;
                        ccDQNode             = ccDQNode->next;
                    }
                    return;
                }
                c_dq_node = c_dq_node->next;
            }
            _last->next       = new_dq_node;
            new_dq_node->prev = _last;
            _last             = new_dq_node;
        }
    }

    SharedPointer<Thread> DeltaQueue::dequeue() {
        if (_first == nullptr) return SharedPointer<Thread>(nullptr);

        if (_first->wake_time == 0) {
            DQNode* f = _first;
            _first    = _first->next;
            if (_first == nullptr)
                _last = nullptr;
            else
                _first->prev = nullptr;
            auto t             = f->sleeping_thread;
            f->sleeping_thread = SharedPointer<Thread>(nullptr);
            f->prev            = nullptr;
            f->next            = nullptr;
            delete f;
            return t;
        }
        return SharedPointer<Thread>(nullptr);
    }

    bool DeltaQueue::remove_waiting_thread(int t_id) {
        if (_first == nullptr) return false;

        DQNode* c_node  = _first;
        bool    removed = false;
        while (c_node) {
            if (c_node->sleeping_thread->handle == t_id) {
                DQNode* next = c_node->next;
                if (next) {
                    next->wake_time += c_node->wake_time;
                    next->prev       = c_node->prev;
                }

                DQNode* prev = c_node->prev;
                if (prev) prev->next = next;

                removed                 = true;
                c_node->prev            = nullptr;
                c_node->next            = nullptr;
                c_node->wake_time       = 0;
                c_node->sleeping_thread = SharedPointer<Thread>(nullptr);
                delete c_node;
                break;
            }
            c_node = c_node->next;
        }
        return removed;
    }

} // namespace Rune::CPU