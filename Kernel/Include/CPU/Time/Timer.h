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

#ifndef RUNEOS_TIMER_H
#define RUNEOS_TIMER_H

#include <KRE/Collections/LinkedList.h>

#include <CPU/Threading/Scheduler.h>

namespace Rune::CPU {

    /**
     * @brief All kernel supported timer modes of operation.
     * <ul>
     *  <li>Periodic: The timer will raises periodic IRQs e.g. every 1ms</li>
     *  <li>OneShot: The timer uses a countdown mechanism to raise an IRQ when the countdown reaches
     * zero. The countdown is set by software.</li>
     * </ul>
     */
#define TIMER_MODES(X)                                                                             \
    X(TimerMode, PERIODIC, 0x1)                                                                    \
    X(TimerMode, ONE_SHOT, 0x2)

    DECLARE_ENUM(TimerMode, TIMER_MODES, 0x0) // NOLINT

    /**
     * @brief A thread an it's wake time in nanoseconds.
     */
    struct SleepingThread {
        Thread* sleeper   = nullptr;
        U64     wake_time = 0;
    };

    /**
     * A configurable timer that can generate interrupts at a specified frequency.
     */
    class Timer {
      protected:
        TimerMode _mode;

        // In order to avoid using floating point values we measure everything in hertz e.g. 1MHz ->
        // 1000000Hz
        U64 _freq_hz;

        // Time in nanoseconds a thread can run before being preempted
        U64 _quantum;

      public:
        explicit Timer();

        virtual ~Timer() = default;

        /**
         * @brief
         * @return The name of the timer device.
         */
        [[nodiscard]]
        virtual String get_name() const = 0;

        /**
         * @brief
         * @return The configured frequency in Hz.
         */
        [[nodiscard]]
        U64 get_frequency() const;

        /**
         * @brief
         * @return The current mode of operation.
         */
        [[nodiscard]]
        TimerMode get_mode() const;

        /**
         * @brief
         * @return The quantum each thread gets before being preempted.
         */
        [[nodiscard]]
        U64 get_quantum() const;

        /**
         *
         * @return The time since the timer was started in nanoseconds.
         */
        [[nodiscard]]
        virtual U64 get_time_since_start() const = 0;

        /**
         * @brief Get all threads that have been put to sleep by this timer.
         * @return A list of sleeping threads.
         */
        [[nodiscard]]
        virtual LinkedList<SleepingThread> get_sleeping_threads() const = 0;

        /**
         * @brief Start the timer and thus enabling preemptive multithreading and sleeping for
         * threads.
         *
         * <p>
         *  The timer itself will be initialized with the requested mode of operation and it is
         * tried to operate it at the requested frequency in Hz. If frequency is bigger then the
         * maximum possible frequency in Hz the timer supports it will be configured to run at it's
         * maximum frequency instead.
         * </p>
         *
         * <p>
         *  Preemptive multithreading will be initialized using the requested quantum in
         * nanoseconds, that is the maximum time a thread is allowed to run without being preempted.
         * </p>
         *
         * @param logger
         * @param sched
         * @param mode      Timer mode of operation.
         * @param frequency Requested timer frequency in Hz.
         * @param quantum   Maximum time in nanoseconds a thread can run before being preempted.
         * @return True: The timer is started, thread sleep and preemptive multithreading is now
         * working. False: The timer could not be started, no sleeping and preemptive multithreading
         * is possible.
         */
        virtual bool start(SharedPointer<Logger> logger,
                           CPU::Scheduler*       scheduler,
                           TimerMode             mode,
                           U64                   frequency,
                           U64                   quantum) = 0;

        /**
         * @brief Search for a thread with requested ID in the wait queue and remove it if found.
         * @param t_id
         * @return True: The thread is removed, False: It is not.
         */
        virtual bool remove_sleeping_thread(int t_id) = 0;

        /**
         * @brief Put the currently running thread to sleep and wake it at the specified wake time.
         * If the wake time is in the past the function will do nothing.
         *
         * Note that it is not guaranteed that the thread will be woken immediately as higher
         * priority threads could be scheduled first.
         *
         * @param wake_time_nanos Wake time in nanoseconds
         */
        virtual void sleep_until(U64 wake_time_nanos) = 0;

        /**
         * @brief Put the currently running thread to sleep and wake it in the specified amount of
         * nanoseconds.
         *
         * Note that it is not guaranteed that the thread will be woken immediately as higher
         * priority threads could be scheduled first.
         *
         * @param time_nanos
         */
        void sleep_nano(U64 time_nanos);

        /**
         * @brief Put the currently running thread to sleep and wake it in the specified amount of
         * micro seconds.
         *
         * Note that it is not guaranteed that the thread will be woken immediately as higher
         * priority threads could be scheduled first.
         *
         * @param time_micros
         */
        void sleep_micro(U64 time_micros);

        /**
         * @brief Put the currently running thread to sleep and wake it in the specified amount of
         * milli seconds.
         *
         * Note that it is not guaranteed that the thread will be woken immediately as higher
         * priority threads could be scheduled first.
         *
         * @param time_millis
         */
        void sleep_milli(U64 time_millis);

        /**
         * @brief Put the currently running thread to sleep and wake it in the specified amount of
         * seconds.
         *
         * Note that it is not guaranteed that the thread will be woken immediately as higher
         * priority threads could be scheduled first.
         *
         * @param time_seconds
         */
        void sleep_second(U64 time_seconds);
    };
} // namespace Rune::CPU

#endif // RUNEOS_TIMER_H
