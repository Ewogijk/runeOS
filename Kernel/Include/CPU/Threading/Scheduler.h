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

#include <KRE/Collections/LinkedList.h>
#include <KRE/Utility.h>

#include <KRE/Logging.h>
#include <KRE/Memory.h>

#include <CPU/Threading/MultiLevelQueue.h>

namespace Rune::CPU {
    /// @brief A scheduler using a multilevel queue with policies grouping threads of the same
    ///         priority together.
    ///
    /// The scheduler manages two special threads, the Garbage Collector Thread (GCT) and the Idle
    /// Thread (IT). Former is used to run a cleanup task on stopped threads while the later will be
    /// run whenever the ready queue is empty. The GCT is implicitly the highest priority thread and
    /// the IT is the lowest priority thread.
    class Scheduler {
        static constexpr char const* BOOTSTRAP_THREAD_NAME = "Bootstrap";

        SharedPointer<Thread> _running_thread;
        MultiLevelQueue*      _ready_queue{nullptr};

        /// @brief Contains references of threads that have been stopped but their allocated memory
        ///         has yet to be freed by the Garbage Collector Thread.
        LinkedList<SharedPointer<Thread>> _thread_garbage_bin;

        /// @brief If (_irqDisableCounter != 0), IRQs are disabled.
        int _irq_disable_counter{0};

        SharedPointer<Thread>   _idle_thread;
        SharedPointer<Thread>   _garbage_collector_thread;
        Function<void(Thread*)> _on_context_switch;

        void (*_thread_enter)(){nullptr};

        /// @brief Allocate the kernel stacks for the given stack.
        /// @param thread
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
        auto next_scheduled_thread() -> SharedPointer<Thread>;

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

        /// @brief Perform the context switch from the running thread to the next thread in the
        /// ready
        ///         queue.
        ///
        /// Note: This function is NOT thread safe!
        void perform_context_switch();

      public:
        Scheduler();

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                      Properties
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        /// @brief
        /// @return The ready queue containing all threads waiting to be scheduled.
        auto get_ready_queue() -> MultiLevelQueue*;

        /// @brief Get a reference to the Thread Garbage Bin (TGB).
        ///
        /// The TGB contains all threads that have been stopped by a call to stop(), but yet need
        /// their kernel stack memory to be freed.
        ///
        /// @return A reference to the TGB.
        auto get_thread_garbage_bin() -> LinkedList<SharedPointer<Thread>>*;

        /// @brief
        /// @return Get a reference to the thread is currently running.
        auto get_running_thread() -> SharedPointer<Thread>;

        /// @brief Return the Idle Thread that will be run when the ready queue is empty.
        /// @return A reference to the Idle Thread.
        auto get_idle_thread() -> SharedPointer<Thread>;

        /// @brief Return the Garbage Collector Thread  (GCT) that frees the memory allocated for
        ///         stopped threads.
        /// @return A reference to the Garbage Collector Thread .
        auto get_garbage_collector_thread() -> SharedPointer<Thread>;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          Event Hooks
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        /**
         *
         * @param onThreadCreated Callback that will be called when a context switch happens.
         *
         * @return True: The callback was registered, False: No.
         */
        void set_on_context_switch(Function<void(Thread*)> on_context_switch);

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                  Scheduling API
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

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
        auto init(const SharedPointer<Thread>& bootstrap_thread,
                  const SharedPointer<Thread>& idle_thread,
                  const SharedPointer<Thread>& thread_terminator,
                  void                         (*thread_enter)()) -> bool;

        /// @brief Allocate the kernel stack of the thread and put it in the ready queue.
        /// @param thread A thread object that has not been executed yet.
        /// @return True: The thread has been put in the ready queue, False: The thread was not in
        ///             the CREATED state or the kernel stack could not be allocated.
        ///
        /// The thread needs to be in the ThreadState::CREATED state, otherwise scheduling of the
        /// thread is rejected. In other words, it must be a newly created thread that has never
        /// been run yet.
        ///
        /// Only the kernel stack will be allocated and set by this function. Initializing the other
        /// members of the thread struct is the responsibility of the caller.
        ///
        /// Thread state transition: ThreadState::CREATED -> ThreadState::READY
        ///
        /// Note: This function is thread safe.
        auto schedule(const SharedPointer<Thread>& thread) -> bool;

        /// @brief Activate thread preemption in the thread_enter function.
        ///
        /// During thread preemption the internal lock will be set before the context switch from
        /// thread A and will be unlocked by thread B after the context switch.
        /// First-time execution of a thread is different since the context switch will jump to the
        /// thread_enter function which is not in control of the scheduler, thus this function must
        /// be called manually.
        ///
        /// Note: Calling this function is mandatory, otherwise preemptive multithreading is not
        ///         possible.
        ///
        /// Note2: This function is thread safe.
        void on_thread_enter();

        /// @brief Immediately trigger a context switch from the running thread to the next highest
        ///         priority thread if preemption is allowed.
        ///
        /// If any threads are in the Thread Garbage Bin then the Garbage Collector Thread  will be
        /// run next, otherwise the highest priority thread in the ready queue will be chosen.
        /// Should the ready queue be empty then the Idle Thread is run next.
        ///
        /// Thread state transition:
        /// - Running thread
        ///     - ThreadState::RUNNING -> ThreadState::READY
        ///     - ThreadState::AWAIT_BLOCK -> ThreadState::AWAIT_BLOCK
        /// - Next thread:
        ///     - ThreadState::READY -> ThreadState::RUNNING
        ///     - ThreadState::AWAIT_BLOCK -> ThreadState::AWAIT_BLOCK
        ///
        /// Note: This function is thread safe.
        void preempt_running_thread();

        /// @brief Mark the running thread to be blocked at some point in the future.
        ///
        /// This function is implemented as a means to solve the lost-wake-up problem at scheduler
        /// level and there kernel-wide. The lost-wake-up problem occurs when thread A is put in a
        /// synchronized wait queue, but the actual blocking happens at a later point. Another
        /// thread B could decide to unblock thread A (because it sees it in the wait queue), that
        /// has not been blocked yet. Now, when thread A is actually blocked it will never be
        /// unblocked, because it was already and thus removed from the wait queue.
        ///
        /// The intention of this function is to plan to block a thread by making a state transition
        /// to ThreadState::AWAIT_BLOCK state and later call block() which atomically triggers a
        /// context switch if and only if the thread is still in said state.
        ///
        /// Thread state transition: ThreadState::RUNNING -> ThreadState::AWAIT_BLOCK
        ///
        /// Note: This function is thread safe.
        void await_block();

        /// @brief Preempt the given thread if and only if it is in the ThreadState::AWAIT_BLOCK
        ///         state.
        /// @param thread The thread to be blocked.
        ///
        /// If thread is currently running it will be preempted immediately without rescheduling it,
        /// otherwise the thread will be removed from the ready queue. After blocking the reference
        /// to the thread will no longer be maintained by the scheduler, it is the callers
        /// responsibility to do so.
        ///
        /// Thread state transition: ThreadState::AWAIT_BLOCK -> ThreadState::BLOCKED
        ///
        /// Note: This function is thread safe.
        void block(const SharedPointer<Thread>& thread);

        /// @brief Preempt the running thread if it is in the ThreadState::AWAIT_BLOCK state.
        ///
        /// This function is a shortcut for: block(get_running_thread()).
        ///
        /// Thread state transition: ThreadState::AWAIT_BLOCK -> ThreadState::BLOCKED
        ///
        /// Note: This function is thread safe.
        void block();

        /// @brief Put the given thread in the ready queue if it is in the ThreadState::BLOCKED
        ///         state.
        /// @param thread Thread to be unblocked.
        ///
        /// The blocked thread will be marked with ThreadState::READY and put into the ready queue.
        /// The caller gives responsibility of maintaining the reference to the thread object back
        /// to the scheduler.
        ///
        /// Thread state transition:
        ///     - ThreadState::BLOCKED -> ThreadState::READY
        ///     - ThreadState::AWAIT_BLOCK -> ThreadState::READY
        ///
        /// Note: This function is thread safe.
        void unblock(const SharedPointer<Thread>& thread);

        /// @brief Stop the given thread from being executed in the future and move it to the Thread
        ///         Garbage Bin (TGB).
        /// @param thread Thread to be stopped.
        ///
        /// The given thread will be removed from the ready queue and placed in the TGB. Should the
        /// given thread be the running thread then it will be preempted immediately, otherwise the
        /// thread will simply not be scheduled anymore and cleaned up by the Thread Garbage
        /// Collector after the next thread preemption.
        ///
        /// Thread state transition: Any state -> ThreadState::STOPPED
        ///
        /// Note: This function is thread safe.
        void stop(const SharedPointer<Thread>& thread);

        /// @brief Move the running thread to the Thread Garbage Bin and preempt it.
        ///
        /// This function is a shortcut for: stop(get_running_thread()).
        ///
        /// Thread state transition: Any state -> ThreadState::STOPPED
        ///
        /// Note: This function is thread safe.
        void stop();
    };
} // namespace Rune::CPU

#endif // RUNEOS_SCHEDULER_H
