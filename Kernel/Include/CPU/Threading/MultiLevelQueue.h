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

#ifndef RUNEOS_MULTILEVELQUEUE_H
#define RUNEOS_MULTILEVELQUEUE_H


#include <Hammer/Memory.h>
#include <Hammer/Collection.h>


#include <CPU/CPU.h>


namespace Rune::CPU {
    /**
     * A multi level queue of various queues with different priorities.
     */
    class MultiLevelQueue {
        LinkedList<SharedPointer<Thread>> _threads;
        SchedulingPolicy                  _policy;
        MultiLevelQueue* _lower_policy_queue;

    public:
        explicit MultiLevelQueue(SchedulingPolicy policy, MultiLevelQueue* lower_policy_queue);


        /**
         * @brief Get a list of all enqueued threads across all policy levels..
         * @return All queued threads.
         */
        LinkedList<Thread*> get_queued_threads();


        /**
         *
         * @return Scheduling policy of the queue.
         */
        SchedulingPolicy get_policy();


        /**
         *
         * @return Queue of the next lower priority policy.
         */
        MultiLevelQueue* get_lower_policy_queue();


        /**
         * Search through all queues starting from the queue with the highest scheduling policy priority and return the
         * first thread that can be dequeued without removing it.
         *
         * @return The next thread in line or a null pointer if all queues are empty.
         */
        Thread* peek();


        /**
         * Try to enqueue the thread in the queue with the same scheduling policy
         *
         * @param t Thread.
         *
         * @return True if the thread got enqueued, false if not queue with the same scheduling policy got found.
         */
        bool enqueue(SharedPointer<Thread> t);


        /**
         * Search through all queues starting from the queue with the highest scheduling policy priority and dequeue the
         * first available thread.
         *
         * @return The next thread in line or a null pointer if all queues are empty.
         */
        SharedPointer<Thread> dequeue();


        /**
         * @brief Search in all queues for a thread with the given ID and remove it if found.
         * @param thread_id ID of a thread.
         * @return If removed: The pointer to the thread, If not found: A null pointer.
         */
        SharedPointer<Thread> remove(int thread_id);
    };
}

#endif //RUNEOS_MULTILEVELQUEUE_H
