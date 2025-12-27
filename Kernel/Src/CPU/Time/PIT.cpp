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

#include <CPU/Time/PIT.h>

#include <KRE/BitsAndBytes.h>

namespace Rune::CPU {
    const SharedPointer<Logger> LOGGER = LogContext::instance().get_logger("CPU.PIT");

    enum Channel { ZERO = 0x40, COMMAND = 0x43 };

    enum Mode { SQUARE_WAVE_GENERATOR = 0x36 };

    PIT::PIT() : _irq_handler([] { return IRQState::PENDING; }) {}

    auto PIT::get_name() const -> String { return "PIT"; }

    auto PIT::get_time_since_start() const -> U64 { return _count * _time_between_irq; }

    auto PIT::get_sleeping_threads() const -> LinkedList<SleepingThread> {
        LinkedList<SleepingThread> l;
        DQNode*                    c = _sleeping_threads.first();
        while (c != nullptr) {
            l.add_back({.sleeper = c->sleeping_thread.get(), .wake_time = c->wake_time});
            c = c->next;
        }
        return l;
    }

    auto PIT::start(CPU::Scheduler* scheduler, TimerMode mode, U64 frequency, U64 quantum) -> bool {
        _scheduler         = scheduler;
        _mode              = mode;
        _freq_hz           = frequency;
        _quantum           = quantum;
        _quantum_remaining = _quantum;
        LOGGER->debug("Requested PIT configuration: Mode={}, TargetFrequency={}Hz, Quantum={}",
                      mode.to_string(),
                      frequency,
                      quantum);

        // The PIT is limited by the QuartzFrequency
        if (_freq_hz > QUARTZ_FREQUENCY_HZ) {
            LOGGER->debug("Requested frequency of {}Hz exceeds quartz frequency {}Hz. Will "
                          "operate on quartz frequency.",
                          _freq_hz,
                          QUARTZ_FREQUENCY_HZ);
            _freq_hz = QUARTZ_FREQUENCY_HZ;
        }

        // To calculate the divider based on the quartz frequency and a target frequency we have to
        // solve the general frequency divider formula QUARTZ_FREQUENCY_GHZ / divider = _freq_ghz =>
        // divider = QUARTZ_FREQUENCY_GHZ / _freq_ghz
        U64 pit_divider = QUARTZ_FREQUENCY_HZ / _freq_hz;
        // Frequency formula: _freq_hz = 1 / _time_between_irq -> _time_between_irq = 1 / _freq_hz.
        // We want nanoseconds therefore we use 1000000000 instead of 1
        constexpr U32 NANO_SECOND = 1000000000;
        _time_between_irq         = NANO_SECOND / _freq_hz;
        LOGGER->debug("Time between IRQs will be ~{}ns", _time_between_irq);

        // Configure the frequency divider
        out_b(Channel::COMMAND, Mode::SQUARE_WAVE_GENERATOR);
        out_b(Channel::ZERO, byte_get(pit_divider, 0)); // Transmit low byte first
        out_b(Channel::ZERO, byte_get(pit_divider, 1)); // Then high byte

        _irq_handler = [this] {
            _count++;
            _sleeping_threads.update_wake_time(_time_between_irq);
            _scheduler->lock();
            auto c_t = _sleeping_threads.dequeue();
            while (c_t) {
                LOGGER->trace(R"(Waking thread "{}-{}" up.)", c_t->handle, c_t->name);
                _scheduler->schedule(c_t);
                if (_scheduler->get_ready_queue()->peek() == c_t.get())
                    _scheduler->execute_next_thread(); // Execute the thread immediately if it is
                                                       // first in the ready queue
                c_t = _sleeping_threads.dequeue();
            }

            bool eoi_triggered = false;
            if (_scheduler->is_preemption_allowed()) {
                if (_quantum_remaining <= 0) {
                    irq_send_eoi();
                    eoi_triggered      = true;
                    _quantum_remaining = _quantum;
                    _scheduler->execute_next_thread();
                } else {
                    _quantum_remaining -= _time_between_irq;
                }
            }

            if (!eoi_triggered) irq_send_eoi();
            _scheduler->unlock();
            return IRQState::HANDLED;
        };

        return irq_install_handler(0, 0, "PIT", _irq_handler);
    }

    auto PIT::remove_sleeping_thread(int t_id) -> bool {
        return _sleeping_threads.remove_waiting_thread(t_id);
    }

    void PIT::sleep_until(U64 wake_time_nanos) {
        _scheduler->lock();
        U64 tsb = get_time_since_start();
        if (wake_time_nanos <= tsb) {
            // Wake time is now or in the past -> Don't bother putting the thread to sleep
            _scheduler->unlock();
            return;
        }
        U64 sleep_time_nanos = wake_time_nanos - tsb;

        SharedPointer<Thread> r_t = _scheduler->get_running_thread();
        LOGGER->trace(R"(Putting thread "{}-{}" to sleep for {}ns)",
                      r_t->handle,
                      r_t->name,
                      sleep_time_nanos);
        _sleeping_threads.enqueue(r_t, sleep_time_nanos);
        r_t->state = ThreadState::SLEEPING;
        _scheduler->execute_next_thread();
        _quantum_remaining = _quantum; // Reset the quantum remaining for the next thread
        _scheduler->unlock();
    }
} // namespace Rune::CPU