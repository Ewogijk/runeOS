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

#include <CPU/Threading/MultiLevelQueue.h>


namespace Rune::CPU {
    MultiLevelQueue::MultiLevelQueue(SchedulingPolicy policy, MultiLevelQueue* lower_policy_queue)
            : _policy(policy),
              _lower_policy_queue(lower_policy_queue) {

    }


    LinkedList<Thread*> MultiLevelQueue::get_queued_threads() {
        LinkedList<Thread*> l;
        auto* c_thread_q = this;
        while (c_thread_q != nullptr) {
            for (auto& t: c_thread_q->_threads)
                l.add_back(t.get());
            c_thread_q = c_thread_q->_lower_policy_queue;
        }
        return l;
    }


    SchedulingPolicy MultiLevelQueue::get_policy() {
        return _policy;
    }


    MultiLevelQueue* MultiLevelQueue::get_lower_policy_queue() {
        return _lower_policy_queue;
    }


    Thread* MultiLevelQueue::peek() {
        auto* c_thread_q = this;
        while (c_thread_q != nullptr) {
            if (!c_thread_q->_threads.is_empty())
                return (*c_thread_q->_threads.head()).get();
            c_thread_q = c_thread_q->_lower_policy_queue;
        }
        return nullptr;
    }


    bool MultiLevelQueue::enqueue(SharedPointer<Thread> t) {
        auto* c_thread_q = this;
        while (c_thread_q != nullptr) {
            if (c_thread_q->get_policy() == t->policy) {
                c_thread_q->_threads.add_back(t);
                return true;
            }
            c_thread_q = c_thread_q->_lower_policy_queue;
        }
        return false;
    }


    SharedPointer<Thread> MultiLevelQueue::dequeue() {
        auto* c_thread_q = this;
        while (c_thread_q != nullptr) {
            if (!c_thread_q->_threads.is_empty()) {
                SharedPointer<Thread> t = *c_thread_q->_threads.head();
                c_thread_q->_threads.remove_front();
                return t;
            }
            c_thread_q = c_thread_q->_lower_policy_queue;
        }
        return SharedPointer<Thread>(nullptr);
    }


    SharedPointer<Thread> MultiLevelQueue::remove(int thread_id) {
        auto* c_thread_q = this;
        while (c_thread_q != nullptr) {
            SharedPointer<Thread> to_delete(nullptr);
            for (auto& t: c_thread_q->_threads) {
                if (t->handle == thread_id) {
                    to_delete = t;
                    break;
                }
            }
            if (to_delete) {
                c_thread_q->_threads.remove(to_delete);
                return to_delete;
            }

            c_thread_q = c_thread_q->_lower_policy_queue;
        }
        return SharedPointer<Thread>(nullptr);
    }
}