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

#include <CPU/Threading/Scheduler.h>

#include <LibK/Logging.h>

#include <CPU/CPU.h>
#include <CPU/Threading/Stack.h>
#include <CPU/Interrupt/Interrupt.h>


constexpr char const* FILE = "Scheduler";


namespace Rune::CPU {
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      Scheduler Implementation
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    void Scheduler::setup_kernel_stack(const SharedPointer<CPU::Thread>& thread) {
        // Create kernel stack - only used for system calls and interrupts
        // Context switches will only ever happen between kernel stacks that's
        // why we have to set it up, so it jumps to the thread startup function
        auto* stack_bottom = new U8[Thread::KERNEL_STACK_SIZE];
        auto stack_top = setup_trampoline_kernel_stack(
                (uintptr_t) stack_bottom + Thread::KERNEL_STACK_SIZE,
                (LibK::VirtualAddr) (uintptr_t) _thread_enter
        );
        thread->kernel_stack_top    = stack_top;
        thread->kernel_stack_bottom = stack_bottom;
    }


    SharedPointer<Thread> Scheduler::next_scheduled_thread() {
        if (!_terminated_threads.is_empty()) // Clean up terminated threads whenever possible
            return _thread_terminator;

        auto t = _ready_threads->dequeue();
        if (!t)
            t = _idle_thread;    // Switch to the idle thread if no other thread is ready
        return t;
    }


    Scheduler::Scheduler() : _logger(),
                             _running_thread(nullptr),
                             _ready_threads(nullptr),
                             _terminated_threads(),
                             _irq_disable_counter(0),
                             _postpone_ctx_switches(0),
                             _ctx_switches_postponed(false),
                             _allow_preemption(false),
                             _idle_thread(nullptr),
                             _thread_terminator(nullptr),
                             _on_context_switch([](Thread* next) { SILENCE_UNUSED(next) }),
                             _thread_enter(nullptr) {

    }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                          Properties
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    MultiLevelQueue* Scheduler::get_ready_queue() {
        return _ready_threads;
    }


    LinkedList<SharedPointer<Thread>>* Scheduler::get_terminated_threads() {
        return &_terminated_threads;
    }


    SharedPointer<Thread> Scheduler::get_running_thread() {
        return _running_thread;
    }


    SharedPointer<Thread> Scheduler::get_idle_thread() {
        return _idle_thread;
    }


    SharedPointer<Thread> Scheduler::get_thread_terminator() {
        return _thread_terminator;
    }


    bool Scheduler::is_preemption_allowed() const {
        return _allow_preemption;
    }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                          Event Hooks
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    void Scheduler::set_on_context_switch(Function<void(CPU::Thread*)> on_context_switch) {
        _on_context_switch = move(on_context_switch);
    }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                          General Stuff
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    void Scheduler::set_logger(SharedPointer<LibK::Logger> logger) {
        _logger = move(logger);
    }


    bool Scheduler::init(
            LibK::PhysicalAddr base_pt_addr,
            Register stack_top,
            const SharedPointer<Thread>& idle_thread,
            const SharedPointer<Thread>& thread_terminator,
            void (* thread_enter)()
    ) {
        auto* back_ground_threads = new MultiLevelQueue(SchedulingPolicy::BACKGROUND, nullptr);
        auto* normal_threads      = new MultiLevelQueue(SchedulingPolicy::NORMAL, back_ground_threads);
        _ready_threads = new MultiLevelQueue(SchedulingPolicy::LOW_LATENCY, normal_threads);
        _thread_enter  = thread_enter;

        // Set the code running since the start of the machine as the initial thread
        // No "main" is needed as the code is already running
        // The idea is to make further initializations and then ditch the bootstrap thread as soon
        // as possible because the stack provided by limine lies in a reclaimed memory region
        // thus it will be reused sooner than later and the initial thread will crash at this point
        _running_thread = SharedPointer<Thread>(new Thread);
        _running_thread->name                    = BOOTSTRAP_THREAD_NAME;
        _running_thread->base_page_table_address = base_pt_addr;
        _running_thread->kernel_stack_top        = stack_top;
        _running_thread->policy                  = SchedulingPolicy::LOW_LATENCY;

        setup_kernel_stack(thread_terminator);
        _thread_terminator = thread_terminator;
        _thread_terminator->state = ThreadState::WAITING;

        setup_kernel_stack(idle_thread);
        _idle_thread = idle_thread;
        _idle_thread->state = ThreadState::WAITING;

        _allow_preemption = true;
        return true;
    }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                          Actual Scheduling
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    void Scheduler::lock() {
        if (_irq_disable_counter == 0)
            interrupt_disable();
        _irq_disable_counter++;
        _postpone_ctx_switches++;
    }


    void Scheduler::unlock() {
        if (_postpone_ctx_switches > 0)
            _postpone_ctx_switches--;
        if (_postpone_ctx_switches == 0) {
            if (_ctx_switches_postponed) {
                _ctx_switches_postponed = false;
                execute_next_thread();
            }
        }

        if (_irq_disable_counter > 0)
            _irq_disable_counter--;
        if (_irq_disable_counter == 0)
            interrupt_enable();
    }


    bool Scheduler::schedule_new_thread(const SharedPointer<CPU::Thread>& thread) {
        if (thread->policy == SchedulingPolicy::NONE) {
            _logger->error(FILE, R"(Attempt to schedule a thread with policy "None")");
            return false;
        }

        setup_kernel_stack(thread);
        if (!_ready_threads->enqueue(thread)) {
            _logger->error(
                    FILE,
                    R"(Failed to put initialized thread "{}" in the ready queue... Freeing allocated stack memory.)",
                    thread->name
            );
            delete[] thread->kernel_stack_bottom;
            return false;
        }
        thread->state = ThreadState::READY;
        return true;
    }


    bool Scheduler::schedule(const SharedPointer<CPU::Thread>& thread) {
        if (!thread || thread == _running_thread)
            return false;

        if (!_ready_threads->enqueue(thread)) {
            _logger->error(FILE, R"(Failed to put thread "{}" in the ready queue.)", thread->name);
            return false;
        }
        thread->state = ThreadState::READY;
        _logger->trace(FILE, R"(Thread "{}-{}" has been scheduled.)", thread->handle, thread->name);
        return true;
    }


    void Scheduler::execute_next_thread() {
        if (_postpone_ctx_switches != 0) {
            _ctx_switches_postponed = true;
            return;
        }

        auto next_thread = next_scheduled_thread();
        if (next_thread == _idle_thread) {
            if (_running_thread == _idle_thread)
                return;  // Just keep the idle thread running

            if (_running_thread->state == ThreadState::RUNNING)
                return; // Let the last non-idle thread keep running
        }

        if (_running_thread == _idle_thread) {
            // Do not reschedule the idle thread -> We do not want it to be regularly scheduled
            _idle_thread->state = ThreadState::WAITING;
        } else if (_running_thread->state == ThreadState::RUNNING) {
            // Reschedule thread
            if (!_ready_threads->enqueue(_running_thread))
                _logger->warn(FILE, R"(Failed to reschedule "{}-{}")", _running_thread->handle, _running_thread->name);
            else {
                if (_running_thread->state == ThreadState::RUNNING)
                    _running_thread->state = ThreadState::READY;
            }
        }

        // Switch to next thread
        _logger->trace(
                FILE,
                R"(Context switch: "{}-{}" -> "{}-{}")",
                _running_thread->handle,
                _running_thread->name,
                next_thread->handle,
                next_thread->name
        );

        auto* old_thread = _running_thread.get();
        _running_thread = move(next_thread);
        _running_thread->state = ThreadState::RUNNING;
        _allow_preemption = _running_thread != _idle_thread;
        _on_context_switch(forward<Thread*>(_running_thread.get()));
        current_core()->switch_to_thread(old_thread, _running_thread.get());
    }


    void Scheduler::terminate(const SharedPointer<CPU::Thread>& thread) {
        if (!thread)
            return;

        thread->state  = ThreadState::TERMINATED;
        thread->policy = SchedulingPolicy::NONE;
        _terminated_threads.add_back(thread);

        // Schedule another thread if the currently running thread is terminated
        if (thread == _running_thread)
            execute_next_thread();
    }


    void Scheduler::terminate() {
        terminate(_running_thread);
    }

}