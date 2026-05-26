//  Copyright 2025 Ewogijk
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include <CPU/Job.h>

#include <CPU/Threading/ThreadPool.h>

namespace Rune::CPU {
    constexpr U8 THREAD_POOL_SIZE = 4;

    ThreadPool<THREAD_POOL_SIZE> g_delayed_interrupt_handler_pool("Delayed Interrupt Handler Pool");

    auto job_schedule_delayed_interrupt_handler(DelayedInterruptHandler dih, InterruptPacket packet)
        -> void {
        if (!g_delayed_interrupt_handler_pool.running()) g_delayed_interrupt_handler_pool.start();
        g_delayed_interrupt_handler_pool.submit([dih, packet] -> void { dih(move(packet)); });
    }
} // namespace Rune::CPU
