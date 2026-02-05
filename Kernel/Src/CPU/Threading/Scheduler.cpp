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

#include <KRE/Logging.h>

#include <CPU/CPU.h>
#include <CPU/Interrupt/Interrupt.h>
#include <CPU/Threading/Stack.h>

namespace Rune::CPU {
    const SharedPointer<Logger> LOGGER = LogContext::instance().get_logger("CPU.Scheduler");

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                  Private Functions
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    void Scheduler::setup_kernel_stack(const SharedPointer<CPU::Thread>& thread) {
        // Create kernel stack - only used for system calls and interrupts
        // Context switches will only ever happen between kernel stacks that's
        // why we have to set it up, so it jumps to the thread startup function
        auto* stack_bottom = new U8[Thread::KERNEL_STACK_SIZE];
        auto  stack_top    = setup_trampoline_kernel_stack(memory_pointer_to_addr(stack_bottom)
                                                           + Thread::KERNEL_STACK_SIZE,
                                                       memory_pointer_to_addr(_thread_enter));

        thread->kernel_stack_top    = stack_top;
        thread->kernel_stack_bottom = stack_bottom;
    }

    auto Scheduler::next_scheduled_thread() -> SharedPointer<Thread> {
        if (!_thread_garbage_bin.is_empty()) // Clean up terminated threads whenever possible
            return _garbage_collector_thread;

        auto t = _ready_queue->dequeue();
        if (!t) t = _idle_thread; // Switch to the idle thread if no other thread is ready
        return t;
    }

    void Scheduler::lock() {
        if (_irq_disable_counter == 0) interrupt_disable();
        _irq_disable_counter++;
    }

    void Scheduler::unlock() {
        if (_irq_disable_counter > 0) _irq_disable_counter--;
        if (_irq_disable_counter == 0) interrupt_enable();
    }

    void Scheduler::perform_context_switch() {
        auto next_thread = next_scheduled_thread();
        if (next_thread == _idle_thread) {
            if (_running_thread == _idle_thread) return; // Just keep the idle thread running

            if (_running_thread->state == ThreadState::RUNNING)
                return; // Let the last non-idle thread keep running
        }

        if (_running_thread == _idle_thread) {
            // Do not reschedule the Idle Thread
            _idle_thread->state = ThreadState::BLOCKED;
        } else {
            switch (_running_thread->state) {
                case ThreadState::NONE:
                case ThreadState::CREATED:
                case ThreadState::READY:
                    LOGGER->warn(R"({}: Invalid thread state "{}" (perform_context_switch))",
                                 _running_thread->get_unique_name(),
                                 _running_thread->state.to_string());
                    break;
                case ThreadState::RUNNING:
                case ThreadState::AWAIT_BLOCK:
                    if (!_ready_queue->enqueue(_running_thread)) {
                        LOGGER->warn(R"({}: Reschedule failed)",
                                     _running_thread->get_unique_name());
                    } else {
                        // Only change from RUNNING -> READY state not AWAIT_BLOCK -> READY
                        // Why? Use case of await_block function is following:
                        //
                        // scheduler->await_block();
                        // ... more code <-- Timer interrupt and preemption could happen
                        // scheduler->block();
                        //
                        // A thread that is in the AWAIT_BLOCK state could be preempted anytime
                        // before it is blocked, therefore the state must be preserved across
                        // context switches, otherwise the block() call will fail
                        if (_running_thread->state == ThreadState::RUNNING)
                            _running_thread->state = ThreadState::READY;
                    }
                    break;
                default: // ThreadState::BLOCKED or ThreadState::STOPPED
                    // Do not reschedule blocked and stopped threads
                    break;
            }
        }

        // Switch to next thread
        LOGGER->trace(R"(Context switch: {} -> {})",
                      _running_thread->get_unique_name(),
                      next_thread->get_unique_name());

        auto* old_thread       = _running_thread.get();
        _running_thread        = move(next_thread);
        _running_thread->state = _running_thread->state == ThreadState::AWAIT_BLOCK
                                     ? ThreadState::AWAIT_BLOCK
                                     : ThreadState::RUNNING;
        _on_context_switch(forward<Thread*>(_running_thread.get()));
        current_core()->switch_to_thread(old_thread, _running_thread.get());
    }

    Scheduler::Scheduler()
        : _running_thread(nullptr),
          _idle_thread(nullptr),
          _garbage_collector_thread(nullptr),
          _on_context_switch([](Thread* next) { SILENCE_UNUSED(next) }) {}

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Properties
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    auto Scheduler::get_ready_queue() -> MultiLevelQueue* { return _ready_queue; }

    auto Scheduler::get_thread_garbage_bin() -> LinkedList<SharedPointer<Thread>>* {
        return &_thread_garbage_bin;
    }

    auto Scheduler::get_running_thread() -> SharedPointer<Thread> { return _running_thread; }

    auto Scheduler::get_idle_thread() -> SharedPointer<Thread> { return _idle_thread; }

    auto Scheduler::get_garbage_collector_thread() -> SharedPointer<Thread> {
        return _garbage_collector_thread;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      Event Hooks
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    void Scheduler::set_on_context_switch(Function<void(CPU::Thread*)> on_context_switch) {
        _on_context_switch = move(on_context_switch);
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                  Scheduling API
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    auto Scheduler::init(const SharedPointer<Thread>& bootstrap_thread,
                         const SharedPointer<Thread>& idle_thread,
                         const SharedPointer<Thread>& thread_terminator,
                         void                         (*thread_enter)()) -> bool {
        auto* back_ground_threads = new MultiLevelQueue(SchedulingPolicy::BACKGROUND, nullptr);
        auto* normal_threads = new MultiLevelQueue(SchedulingPolicy::NORMAL, back_ground_threads);
        _ready_queue         = new MultiLevelQueue(SchedulingPolicy::LOW_LATENCY, normal_threads);
        _thread_enter        = thread_enter;
        _running_thread      = bootstrap_thread;

        setup_kernel_stack(thread_terminator);
        _garbage_collector_thread        = thread_terminator;
        _garbage_collector_thread->state = ThreadState::BLOCKED;

        setup_kernel_stack(idle_thread);
        _idle_thread        = idle_thread;
        _idle_thread->state = ThreadState::BLOCKED;
        return true;
    }

    auto Scheduler::schedule(const SharedPointer<Thread>& thread) -> bool {
        lock();
        if (!thread) {
            unlock();
            return false;
        }
        if (thread->state != ThreadState::CREATED) {
            LOGGER->error(R"({}-{}: Invalid thread state "{}" (schedule))",
                          thread->get_unique_name(),
                          thread->state.to_string());
            unlock();
            return false;
        }
        if (thread->policy == SchedulingPolicy::NONE) {
            LOGGER->error(R"({}-{}: Invalid thread policy "NONE")", thread->get_unique_name());
            unlock();
            return false;
        }

        setup_kernel_stack(thread);
        if (!_ready_queue->enqueue(thread)) {
            LOGGER->error(R"({}-{}: Schedule failed... Freeing kernel stack)",
                          thread->get_unique_name());
            delete[] thread->kernel_stack_bottom;
            unlock();
            return false;
        }
        thread->state = ThreadState::READY;
        unlock();
        return true;
    }

    void Scheduler::on_thread_enter() { unlock(); }

    void Scheduler::preempt_running_thread() {
        lock();
        perform_context_switch();
        unlock();
    }

    void Scheduler::await_block() {
        lock();
        _running_thread->state = ThreadState::AWAIT_BLOCK;
        unlock();
    }

    void Scheduler::block(const SharedPointer<Thread>& thread) {
        lock();
        if (!thread) {
            unlock();
            return;
        }
        if (thread->state != ThreadState::AWAIT_BLOCK) {
            LOGGER->error(R"({}-{}: Invalid thread state "{}" (block))",
                          _running_thread->get_unique_name(),
                          _running_thread->state.to_string());
            unlock();
            return;
        }
        thread->state = ThreadState::BLOCKED;
        if (thread != _running_thread)
            _ready_queue->remove(thread->get_handle()); // Remove the thread from the schedule
        else
            perform_context_switch(); // Preempt the running thread

        unlock();
    }

    void Scheduler::block() { block(_running_thread); }

    void Scheduler::unblock(const SharedPointer<Thread>& thread) {
        lock();
        if (!thread) {
            unlock();
            return;
        }
        if (thread->state != ThreadState::BLOCKED && thread->state != ThreadState::AWAIT_BLOCK) {
            LOGGER->error(R"({}-{}: Invalid thread state "{}" (unblock))",
                          thread->get_unique_name(),
                          thread->state.to_string());
            unlock();
            return;
        }

        if (!_ready_queue->enqueue(thread)) {
            LOGGER->error(R"({}-{}: Scheduling failed)", thread->get_unique_name());
            unlock();
            return;
        }
        thread->state = ThreadState::READY;
        // Context switch immediately if thread was scheduled as next thread
        if (thread->get_handle() == _ready_queue->peek()->get_handle()) perform_context_switch();
        unlock();
    }

    void Scheduler::stop(const SharedPointer<Thread>& thread) {
        lock();
        if (!thread) {
            unlock();
            return;
        }
        thread->state = ThreadState::STOPPED;
        _thread_garbage_bin.add_back(thread);
        if (thread != _running_thread)
            _ready_queue->remove(thread->get_handle()); // Remove the thread from the schedule
        else
            perform_context_switch(); // Preempt the running thread
        unlock();
    }

    void Scheduler::stop() { stop(_running_thread); }
} // namespace Rune::CPU