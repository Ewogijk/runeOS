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

#ifndef RUNEOS_SCHEDULER_H
#define RUNEOS_SCHEDULER_H

#include <KernelRuntime/Collection.h>
#include <KernelRuntime/Utility.h>

#include <KernelRuntime/Logging.h>
#include <KernelRuntime/Memory.h>

#include <CPU/Threading/MultiLevelQueue.h>

namespace Rune::CPU {
    /**
     * A round robin scheduler that utilizes a multilevel queue with each queue having a scheduling
     * policy that determines the priority of threads in that queue.
     *
     * <p>
     *  A thread terminator is used to free memory of terminated threads as well as a idle thread
     * that halts the CPU until another thread is ready to run. The thread terminator has implicitly
     * the highest priority while the idle thread the lowest, both are not scheduled in the
     * multilevel queue.
     * </p>
     */
    class Scheduler {
        static constexpr char const* BOOTSTRAP_THREAD_NAME = "Bootstrap";

        SharedPointer<Logger> _logger;

        SharedPointer<Thread> _running_thread;
        MultiLevelQueue*      _ready_threads;

        /**
         * @brief Whenever this thread contains at least one thread, the thread terminator will be
         * scheduled.
         */
        LinkedList<SharedPointer<Thread>> _terminated_threads;

        /**
         * @brief If (_irqDisableCounter != 0), IRQs are disabled.
         */
        int _irq_disable_counter;

        /**
         * @brief If (_postPoneCtxSwitches != 0), then no context switch will be done.
         */
        int  _postpone_ctx_switches;
        bool _ctx_switches_postponed;

        /**
         * @brief If (_allowPreemption == true) threads can be preempted. Note: We cannot know if a
         * schedule() call happens as part preemption, thus we cannot enforce this rule and must
         * hope preemption is implemented properly.
         */
        bool _allow_preemption;

        SharedPointer<Thread>   _idle_thread;
        SharedPointer<Thread>   _thread_terminator;
        Function<void(Thread*)> _on_context_switch;

        void (*_thread_enter)();

        /**
         * @brief allocate the kernel stacks for the given stack.
         * @param thread
         */
        void setup_kernel_stack(const SharedPointer<Thread>& thread);

        /**
         * @brief Search for the next thread that should be scheduled.
         *
         * If any threads are terminated the thread terminator will be returned, otherwise the next
         * thread from the ready queue will be chosen. Only if the ready queue is empty the idle
         * thread will be returned.
         *
         * @return The next thread for scheduling.
         */
        SharedPointer<Thread> next_scheduled_thread();

      public:
        Scheduler();

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          Properties
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        /**
         * @brief The ready queue contains all threads that are waiting to be scheduled.
         * @return The ready queue.
         */
        MultiLevelQueue* get_ready_queue();

        /**
         * @brief Get all threads that have been marked as terminated that still need to have their
         * memory freed.
         *
         * On the next cycle of the tread terminator their memory will be freed.
         *
         * @return A list of terminated threads.
         */
        LinkedList<SharedPointer<Thread>>* get_terminated_threads();

        /**
         * @brief Get the thread that currently has CPU time, meaning the thread that can execute
         * code.
         * @return The running thread.
         */
        SharedPointer<Thread> get_running_thread();

        /**
         * @brief The idle thread will always be scheduled when there is no other ready thread
         * available.
         *
         * The thread terminator is the second thread to be created during kernel boot thus it will
         * always have ID 1 and furthermore it never terminates.
         *
         * @return The idle thread.
         */
        SharedPointer<Thread> get_idle_thread();

        /**
         * @brief The thread terminator is responsible for freeing the memory allocated for another
         * thread that has finished execution.
         *
         * The thread terminator is the third thread to be created during kernel boot thus it will
         * always have ID 2 and furthermore it never terminates.
         *
         * @return The thread terminator.
         */
        SharedPointer<Thread> get_thread_terminator();

        /**
         * @brief Whenever this function returns true the scheduler can be preempted by e.g. a
         * timer, otherwise a timer must not attempt to call the schedule() function.
         * @return True: Calling schedule() as part of preemption is allowed, False: It is not
         * allowed to call schedule() as part of preemption.
         */
        [[nodiscard]]
        bool is_preemption_allowed() const;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          Event Hooks
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        /**
         *
         * @param onThreadCreated Callback that will be called when a context switch happens.
         *
         * @return True: The callback was registered, False: No.
         */
        void set_on_context_switch(Function<void(Thread*)> on_context_switch);

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          General Stuff
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        /**
         *
         * @param logger
         */
        void set_logger(SharedPointer<Logger> logger);

        /**
         * @brief Initialize the scheduler by creating the system threads, after initialization
         * finished successfully other threads can be scheduled.
         *
         * Initialization includes following steps:
         * <ol>
         *  <li>The Bootstrap thread is created and set as the running thread, as it was running
         * implicitly since the computer was turned on.</li> <li> The idle thread is created.</li>
         *  <li> The thread terminator is created.</li>
         * </ol>
         *
         * @param base_pt_addr Address of the base page table defining the virtual address space of
         * the thread.
         * @param stack_top   Top of the bootstrap stack.
         * @return
         */
        bool init(PhysicalAddr                 base_pt_addr,
                  CPU::Register                stack_top,
                  const SharedPointer<Thread>& idle_thread,
                  const SharedPointer<Thread>& thread_terminator,
                  void                         (*thread_enter)());

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          Actual Scheduling
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        /**
         * Disable interrupts and postpone context switches until the scheduler is unlocked. Two
         * separate locks are used for disabling interrupts and postponing context switches. Both
         * locks can be acquired multiple times.
         */
        void lock();

        /**
         * Release both locks once. If the last interrupt disable lock is released, interrupts are
         * enabled. If the last postpone context switches lock is released, a context switch is
         * triggered.
         */
        void unlock();

        /**
         * @brief Setup the kernel stack of the newly created thread and put it into the ready
         * queue.
         *
         * This function only sets the KernelStackBottom, KernelStackTop and State field of the
         * thread, the other fields must be set before passing the thread to this function.
         *
         * @param thread New thread to be scheduled.
         * @return True: The thread is in the ready queue waiting to be scheduled, False: The thread
         * has policy "None" or could not be put into the ready queue.
         */
        bool schedule_new_thread(const SharedPointer<Thread>& thread);

        /**
         * @brief Put the thread in the "Ready" state and place it in the ready queue. The thread
         * will be executed
         *
         * <p>
         *  This function is intended to schedule an already initialized thread that was blocked /
         * waiting in another wait queue and should now be scheduled for execution.
         * </p>
         * <p>
         *  It is the callers responsibility to remove the thread from it's wait queue before
         * calling this function.
         * </p>
         *
         * @param thread
         * @param preemptRunningThread
         * @return True: The thread is in the ready queue, False: The could not be enqueued in the
         * ready queue, it was null or it is the currently running thread.
         */
        bool schedule(const SharedPointer<Thread>& thread);

        /**
         * @brief Trigger a context switch to continue execution of the next ready thread if context
         * switches are allowed meaning not postponed.
         *
         * <p>
         *  Important note: It is the callers responsibility to lock/unlock the scheduler
         * before/after calling this function.
         * </p>
         * <p>
         *  If there are any terminated threads, then the thread terminator is scheduled. Otherwise
         * the next thread in the highest available scheduling policy of the ready queue is
         * scheduled. Should no thread be ready for scheduling the idle thread will be scheduled.
         * </p>
         * <p>
         *  When the currently running thread is still in the "Running" state and no other thread is
         * ready then it will be rescheduled otherwise not.
         * </p>
         * <p>
         *  Preemption is disabled while the idle task is running.
         * </p>
         */
        void execute_next_thread();

        /**
         * @brief Mark the given thread as terminated and put it into the terminated threads queue.
         * If the thread is the currently running thread a context switch will be initiated.
         *
         * <p>
         *   It is the callers responsibility to make sure that the thread gets removed from it's
         * current wait queue, before it is put in the terminated thread queue.
         * </p>
         * <p>
         *  Important note: It is the callers responsibility to lock/unlock the scheduler
         * before/after calling this function.
         * </p>
         * @param thread
         */
        void terminate(const SharedPointer<Thread>& thread);

        /**
         * @brief terminate the currently running thread, this is basically a call to
         * terminate(get_running_thread()). See terminate(SharedPointer<Thread> thread) for the
         * details.
         */
        void terminate();
    };
} // namespace Rune::CPU

#endif // RUNEOS_SCHEDULER_H
