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
    auto DeltaQueue::remove(DQNode* node) -> SharedPointer<Thread> {
        if (node == nullptr) return {};

        if (node->m_next != nullptr) {
            if (node->m_wake_time > 0) node->m_next->m_wake_time += node->m_wake_time;
            node->m_next->m_prev = node->m_prev;
        }
        if (node->m_prev != nullptr) node->m_prev->m_next = node->m_next;

        if (node == m_first) m_first = node->m_next;
        if (node == m_last) m_last = node->m_prev;

        auto t                = node->m_sleeping_thread;
        node->m_prev            = nullptr;
        node->m_next            = nullptr;
        node->m_wake_time       = 0;
        node->m_sleeping_thread = SharedPointer<Thread>();
        delete node;

        return t;
    }

    DeltaQueue::DeltaQueue() = default;

    DeltaQueue::~DeltaQueue() {
        while (m_first != nullptr) {
            auto* node = m_first;
            m_first     = m_first->m_next;
            delete node;
        }
    }

    auto DeltaQueue::first() const -> DQNode* { return m_first; }

    void DeltaQueue::update_wake_time(U64 time_decrement) {
        if (m_first != nullptr) m_first->m_wake_time -= time_decrement;
    }

    void DeltaQueue::enqueue(const SharedPointer<Thread>& thread, U64 wake_time) {
        if (!thread) return;

        auto* new_dq_node            = new DQNode();
        new_dq_node->m_sleeping_thread = thread;
        new_dq_node->m_wake_time       = wake_time;
        if (m_first == nullptr) {
            m_first = m_last = new_dq_node;
        } else {
            auto* c_dq_node = m_first;
            while (c_dq_node != nullptr) {
                if (new_dq_node->m_wake_time >= c_dq_node->m_wake_time) {
                    new_dq_node->m_wake_time -= c_dq_node->m_wake_time;
                } else {
                    if (c_dq_node->m_prev == nullptr) {
                        c_dq_node->m_prev   = new_dq_node;
                        new_dq_node->m_next = m_first;
                        m_first            = new_dq_node;
                    } else {
                        c_dq_node->m_prev->m_next = new_dq_node;
                        new_dq_node->m_prev     = c_dq_node->m_prev;
                        c_dq_node->m_prev       = new_dq_node;
                        new_dq_node->m_next     = c_dq_node;
                    }
                    // Update m_next nodes wake time
                    if (new_dq_node->m_next != nullptr)
                        new_dq_node->m_next->m_wake_time -= new_dq_node->m_wake_time;
                    return;
                }
                c_dq_node = c_dq_node->m_next;
            }
            m_last->m_next       = new_dq_node;
            new_dq_node->m_prev = m_last;
            m_last             = new_dq_node;
        }
    }

    auto DeltaQueue::dequeue() -> SharedPointer<Thread> {
        if (m_first == nullptr) return {};
        if (m_first->m_wake_time == 0) {
            return remove(m_first);
        }
        return {};
    }

    auto DeltaQueue::remove_waiting_thread(Handle thread_handle) -> bool {
        if (m_first == nullptr) return false;

        DQNode* to_remove = nullptr;
        DQNode* c_node    = m_first;
        while (c_node != nullptr) {
            if (c_node->m_sleeping_thread->get_handle() == thread_handle) {
                to_remove = c_node;
                break;
            }
            c_node = c_node->m_next;
        }
        return remove(to_remove).get() != nullptr;
    }

} // namespace Rune::CPU