
//  Copyright 2026 Ewogijk
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

#ifndef RUNEOS_THREAD_H
#define RUNEOS_THREAD_H

#include <Ember/Ember.h>

#include <KRE/CPU.h>
#include <KRE/Resource.h>

namespace Rune::CPU {
    /// @brief Describes what a thread is currently doing.
    /// @param X
    ///
    /// - CREATED: The thread has been created but has not started execution yet.
    /// - READY: The thread is in the ready queue of the scheduler and waiting to be scheduled.
    /// - RUNNING: The thread code is currently being executed.
    /// - BLOCK_PENDING: The thread is running but planned to be blocked in the future.
    /// - BLOCKED: The thread is not in the ready queue of the scheduler and waiting for a condition
    ///             to fulfill before it will be scheduled again.
    /// - STOPPED: The thread has finished execution, but its heap memory has yet to be freed.
#define THREAD_STATES(X)                                                                           \
    X(ThreadState, CREATED, 0x1)                                                                   \
    X(ThreadState, READY, 0x2)                                                                     \
    X(ThreadState, RUNNING, 0x3)                                                                   \
    X(ThreadState, BLOCK_PENDING, 0x4)                                                             \
    X(ThreadState, BLOCKED, 0x5)                                                                   \
    X(ThreadState, STOPPED, 0x6)

    DECLARE_ENUM(ThreadState, THREAD_STATES, 0x0) // NOLINT

    /// @brief The scheduling policy describes the priority of a group of threads.
    /// @param X
    ///
    /// - LowLatency: Highest priority.
    /// - Normal:
    /// - Background: Lowest priority.
#define SCHEDULING_POLICIES(X)                                                                     \
    X(SchedulingPolicy, LOW_LATENCY, 0x1)                                                          \
    X(SchedulingPolicy, NORMAL, 0x2)                                                               \
    X(SchedulingPolicy, BACKGROUND, 0x3)

    DECLARE_ENUM(SchedulingPolicy, SCHEDULING_POLICIES, 0x0) // NOLINT

    /**
     * @brief A thread stack.
     */
    struct Stack {
        void*       stack_bottom = nullptr; // First allocated stack page
        VirtualAddr stack_top    = 0x0;     // Stack pointer
        MemorySize  stack_size   = 0x0;     // Maximum stack size
    };

    /// @brief The thread struct contains technical and informational data about a thread object.
    ///
    /// Threads are a resource and therefore associated with a unique handle and a name for
    /// debugging purposes.
    ///
    /// A thread has a state which must only be modified by the scheduler or undefined behavior will
    /// occur. Only upon instantiation, where it must be assigned the ThreadState::CREATED.
    ///
    /// Each thread will be maintained by either the scheduler, some synchronization primitive or
    /// a timer. To be able to track where the thread is currently being maintained a reference to
    /// the maintaining resource is kept in the thread struct and only one reference must be set at
    /// once at all times. If the thread is maintained by the scheduler, no resource references must
    /// be set.
    struct Thread : public Resource<Ember::Handle> {
        static constexpr MemorySize KERNEL_STACK_SIZE = 32 * MemoryUnit::KiB;

        Thread(Ember::Handle handle, const String& name);

        // Handle of the app the thread belongs to
        Ember::Handle    m_app_handle = 0;
        ThreadState      m_state      = ThreadState::CREATED;
        SchedulingPolicy m_policy     = SchedulingPolicy::NONE;

        /// @brief The kernel stack is used whenever kernel code is run e.g. because of an interrupt
        ///         or syscall It is dynamically allocated on the kernel heap and has a
        ///         preconfigured fixed size
        U8*         m_kernel_stack_bottom = nullptr; // Pointer to the heap allocated memory
        VirtualAddr m_kernel_stack_top    = 0x0;

        /// @brief The user mode stack contains application data, it is managed by an application.
        Stack m_user_stack;

        /// @brief Address of the base page table defining the threads virtual address space.
        PhysicalAddr m_base_page_table_address = 0x0;

        /// @brief A pointer to the ThreadLaunchPacket (TLP) of this thread.
        ///
        /// A TLP must be allocated in kernelspace for kernel threads and in userspace for user
        /// threads.
        UniquePointer<Ember::ThreadLaunchPacket> m_tlp;

        /// @brief The thread control block contains the thread local storage (TLS) and other data,
        ///         it is maintained by libc. We simply provide easy access to it through an arch
        ///         specific TLS register.
        void* m_thread_control_block = nullptr;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                  Resource Refs
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        /// @brief Handle of the timer that maintains the thread.
        Ember::Handle m_timer_handle = Ember::HANDLE_NONE;

        /// @brief Handle of the mutex that maintains the thread.
        Ember::Handle m_mutex_handle = Ember::HANDLE_NONE;

        /// @brief Handle of the semaphore that maintains the thread.
        Ember::Handle m_semaphore_handle = Ember::HANDLE_NONE;

        /// @brief Handle of the thread that this thread is waiting for to exit.
        Ember::Handle m_sync_stop_thread_handle = Ember::HANDLE_NONE;

        /// @brief ID of the application this thread is waiting for to exit.
        Ember::Handle m_join_app_handle = -1;

        friend auto operator==(const Thread& one, const Thread& two) -> bool;

        friend auto operator!=(const Thread& one, const Thread& two) -> bool;
    };

    ///@brief Kernel-wide thread cache.
    extern ResourceCache<Thread, 4> g_thread_cache;
} // namespace Rune::CPU

#endif // RUNEOS_THREAD_H
