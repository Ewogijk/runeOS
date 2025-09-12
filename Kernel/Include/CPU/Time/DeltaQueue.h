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

#ifndef RUNEOS_DELTAQUEUE_H
#define RUNEOS_DELTAQUEUE_H

#include <CPU/CPU.h>

namespace Rune::CPU {

    struct DQNode {
        SharedPointer<Thread> sleeping_thread = SharedPointer<Thread>(nullptr);
        DQNode*               prev            = nullptr;
        DQNode*               next            = nullptr;
        U64                   wake_time       = 0;
    };

    /**
     * Delta queue implementation that sorts threads by their wake time. Threads with earlier wake time than others will
     * be put before threads that are woken at a later time.
     */
    class DeltaQueue {
        DQNode* _first;
        DQNode* _last;

      public:
        explicit DeltaQueue();

        [[nodiscard]]
        DQNode* first() const;

        // Decrement the wake time of the first thread in the queue.
        void update_wake_time(U64 time_decrement);

        // Sort the thread into the queue according to the given wake time.
        void enqueue(const SharedPointer<Thread>& thread, U64 wake_time);

        // Dequeue the first thread if it has a wake time of zero.
        SharedPointer<Thread> dequeue();

        // Remove a waiting thread at any position from the queue.
        bool remove_waiting_thread(int t_id);
    };
} // namespace Rune::CPU

#endif // RUNEOS_DELTAQUEUE_H
