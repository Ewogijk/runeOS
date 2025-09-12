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

#include <CPU/Time/Timer.h>

namespace Rune::CPU {
    DEFINE_ENUM(TimerMode, TIMER_MODES, 0x0)

    Timer::Timer() : _mode(TimerMode::NONE), _freq_hz(0), _quantum(0) {}

    U64 Timer::get_frequency() const { return _freq_hz; }

    TimerMode Timer::get_mode() const { return _mode; }

    U64 Timer::get_quantum() const { return _quantum; }

    void Timer::sleep_nano(U64 time_nanos) { sleep_until(get_time_since_start() + time_nanos); }

    void Timer::sleep_micro(U64 time_micros) {
        U64 time_nanos = time_micros * 1000;
        sleep_until(get_time_since_start() + time_nanos);
    }

    void Timer::sleep_milli(U64 time_millis) {
        U64 time_nanos = time_millis * 1000000;
        U64 res        = get_time_since_start() + time_nanos;
        sleep_until(res);
    }

    void Timer::sleep_second(U64 time_seconds) {
        U64 time_nanos = time_seconds * 1000000000;
        sleep_until(get_time_since_start() + time_nanos);
    }
} // namespace Rune::CPU
