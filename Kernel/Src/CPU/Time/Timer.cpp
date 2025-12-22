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

    auto Timer::get_frequency() const -> U64 { return _freq_hz; }

    auto Timer::get_mode() const -> TimerMode { return _mode; }

    auto Timer::get_quantum() const -> U64 { return _quantum; }

    void Timer::sleep_nano(U64 time_nanos) { sleep_until(get_time_since_start() + time_nanos); }

    void Timer::sleep_micro(U64 time_micros) {
        constexpr U16 MICRO_TO_NANO = 1000;
        U64 time_nanos = time_micros * MICRO_TO_NANO;
        sleep_until(get_time_since_start() + time_nanos);
    }

    void Timer::sleep_milli(U64 time_millis) {
        constexpr U32 MILLI_TO_NANO = 1000000;
        U64 time_nanos = time_millis * MILLI_TO_NANO;
        U64 res        = get_time_since_start() + time_nanos;
        sleep_until(res);
    }

    void Timer::sleep_second(U64 time_seconds) {
        constexpr U32 SECOND_TO_NANO = 1000000000;
        U64 time_nanos = time_seconds * SECOND_TO_NANO;
        sleep_until(get_time_since_start() + time_nanos);
    }
} // namespace Rune::CPU
