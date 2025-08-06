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

#ifndef RUNEOS_PIT_H
#define RUNEOS_PIT_H


#include <CPU/IO.h>

#include <CPU/Interrupt/IRQ.h>

#include <CPU/Time/Timer.h>
#include <CPU/Time/DeltaQueue.h>


namespace Rune::CPU {
    /**
     * @brief Driver for the programmable interrupt timer.
     */
    class PIT : public Timer {
        /**
         * @brief Frequency is 1.193182MHz -> 1193182Hz
         */
        static constexpr U64 QUARTZ_FREQUENCY_HZ = 1193182;

        SharedPointer<Logger> _logger;
        Scheduler* _scheduler;
        IRQHandler _irq_handler;

        DeltaQueue _sleeping_threads;
        U64        _count;              // Ticks since boot

        // Remaining time in nanoseconds the thread can run before being preempted
        U32 _quantum_remaining;

        // Time in nanoseconds between two IRQs
        U64 _time_between_irq;
    public:
        PIT();


        ~PIT() override = default;


        [[nodiscard]] String get_name() const override;


        [[nodiscard]] U64 get_time_since_start() const override;


        [[nodiscard]] LinkedList<SleepingThread> get_sleeping_threads() const override;


        bool start(
                SharedPointer<Logger> logger,
                CPU::Scheduler* scheduler,
                TimerMode mode,
                U64 frequency,
                U64 quantum
        ) override;


        bool remove_sleeping_thread(int t_id) override;


        void sleep_until(U64 wake_time_nanos) override;
    };

}

#endif //RUNEOS_PIT_H
