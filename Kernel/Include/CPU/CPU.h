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

#ifndef RUNEOS_CPU_H
#define RUNEOS_CPU_H


#include <Hammer/Definitions.h>
#include <Hammer/Enum.h>
#include <Hammer/String.h>

#include <LibK/KMemory.h>


namespace Rune::CPU {
    // Size of a register
#ifdef IS_64_BIT
    using Register = U64;
#else
    using Register = U32;
#endif

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Threading structures
    //
    // Defined here because they are needed by the "Core" class
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    /**
     * @brief Main function of a thread. This is defined as function pointer because it is more efficient, this will
     *        keep the call stack small and simple and a small and simple call stack is easy to debug when the stack
     *        gets messed up. Hint: Debugging a complex and big (or any) stack is not fun!
     */
    using ThreadMain = int (*)(int, char* []);


    /**
     * @brief Describes what a thread is currently doing.
     * <ul>
     *  <li>Ready: The thread is in the ready queue and waiting to be scheduled.</li>
     *  <li>Running: The thread is in the ready queue and waiting to be scheduled.</li>
     *  <li>Sleeping: The thread is in the sleep queue of a timer.</li>
     *  <li>Waiting: The thread is in the wait queue of a mutex.</li>
     *  <li>Terminated: The thread has finished execution and but it's resources are not freed yet.</li>
     * </ul>
     */
#define THREAD_STATES(X)                       \
            X(ThreadState, READY, 0x1)         \
            X(ThreadState, RUNNING, 0x2)       \
            X(ThreadState, SLEEPING, 0x3)      \
            X(ThreadState, WAITING, 0x4)       \
            X(ThreadState, TERMINATED, 0x5)    \



    DECLARE_ENUM(ThreadState, THREAD_STATES, 0x0)  // NOLINT


    /**
     * @brief The scheduling policy describes the priority of a group of threads.
     * <ul>
     *  <li>LowLatency: Highest priority.</li>
     *  <li>Normal: </li>
     *  <li>Background: Lowest priority.</li>
     * </ul>
     */
#define SCHEDULING_POLICIES(X)                      \
            X(SchedulingPolicy, LOW_LATENCY, 0x1)   \
            X(SchedulingPolicy, NORMAL, 0x2)        \
            X(SchedulingPolicy, BACKGROUND, 0x3)    \



    DECLARE_ENUM(SchedulingPolicy, SCHEDULING_POLICIES, 0x0)  // NOLINT


    /**
     * The thread struct contains general information about a running thread.
     */
    struct Thread {
        static constexpr LibK::MemorySize KERNEL_STACK_SIZE = 32 * LibK::MemoryUnit::KiB;

        // Unique ID of the thread
        U16 handle = 0;

        // Handle of the app the thread belongs to
        U16              app_handle = 0;
        String           name       = "";
        ThreadState      state      = ThreadState::NONE;
        SchedulingPolicy policy     = SchedulingPolicy::NONE;

        // The kernel stack is used whenever kernel code is run e.g. because of an interrupt or syscall
        // It is dynamically allocated on the kernel heap and has a preconfigured fixed size
        U8* kernel_stack_bottom = nullptr;                // Pointer to the heap allocated memory
        LibK::VirtualAddr kernel_stack_top = 0x0;

        // The user mode stack is used to run an application, it is allocated in user space memory
        // It will be initially allocated by the kernel but after that it is on the app to manage its stack
        // we just need the stack top so we can swap it with the kernel stack when a syscall is made
        LibK::VirtualAddr user_stack_top = 0x0;

        // Address of the base page table defining the threads virtual address space.
        LibK::PhysicalAddr base_page_table_address = 0x0;

        /**
         * @brief ID of the mutex this thread is owning at the moment.
         */
        int mutex_id = -1;

        /**
         * @brief ID of the application this thread is waiting for to exit.
         */
        int join_app_id = -1;

        // Number of arguments in argv
        int        argc = 0;
        // Null terminated array pointers to the thread arguments
        char** argv = nullptr;
        // Main function of the thread
        ThreadMain main = nullptr;


        friend bool operator==(const Thread& one, const Thread& two);


        friend bool operator!=(const Thread& one, const Thread& two);
    };


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Core API
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    /**
    * @brief Technical specs of the CPU.
    */
    struct TechSpec {
        const String vendor = "";
        const String family = "";
        const String model  = "";
    };


    /**
     * @brief Architectural details of the CPU.
     */
    struct ArchSpec {
        U8 physical_address_width = 0;
    };


    /**
     * @brief A privilege level defines the currently executing code can do on the computer.
     * <ul>
     *  <li>Kernel: The running program can access all kernel and user memory and run any assembly command.</li>
     *  <li>User:   The running program can only access user memory and may not be able to run all assembly commands.
     *              Disallowed assembly commands are CPU specific.</li>
     * </ul>
     *
     */
#define PRIVILEGE_LEVELS(X)             \
         X(PrivilegeLevel, KERNEL, 0x1) \
         X(PrivilegeLevel, USER, 0x2)   \



    DECLARE_TYPED_ENUM(PrivilegeLevel, U8, PRIVILEGE_LEVELS, 0x0)  // NOLINT


    /**
     * @brief General information about an entry in the interrupt vector table of the processor.
     */
    struct InterruptVector {
        const U8                vector = 0;   // The ID of the interrupt vector
        const LibK::VirtualAddr handler_addr; // Virtual address of the function handling this interrupt vector
        const PrivilegeLevel    level;        // The privilege level at which this interrupt can be manually triggered
        const bool              active;       // True: The interrupt handler is used, False: It is unused.
    };


    /**
     * @brief A single core on the CPU.
     */
    class Core {
    public:

        /**
         * @brief Setup CPU specific data structures for this core.
         * @return True: The CPU core is initialized. False: Initialization of this core failed.
         */
        virtual bool init() = 0;


        /**
         * @brief Get the unique ID of this CPU core.
         * @return The core ID.
         */
        virtual U8 get_id() = 0;


        /**
         * @brief Get technical specs of the CPU like the model, etc.
         * @return
         */
        virtual TechSpec get_tech_spec() = 0;


        /**
         * @brief Get the architectural specs of the CPU like the physical address width, etc.
         * @return
         */
        virtual ArchSpec get_arch_details() = 0;


        /**
         * @brief
         * @return The privilege level at which the core currently runs.
         */
        virtual PrivilegeLevel get_current_privilege_level() = 0;


        /**
         * @brief Get the interrupt vector table of the core.
         * @return The interrupt vector table.
         */
        virtual LinkedList<InterruptVector> get_interrupt_vector_table() = 0;


        /**
         * @brief Write the current values of general purpose registers and CPU specific structures to the logger.
         * @param stream
         */
        virtual void dump_core_state(const SharedPointer<LibK::TextStream>& stream) = 0;


        /**
         * @brief Make a context switch from the current thread to the next thread.
         * @param c_thread Currently running thread.
         * @param n_thread Next thread to be scheduled.
         */
        virtual void switch_to_thread(Thread* c_thread, Thread* n_thread) = 0;


        /**
         * @brief Execute the thread main in kernel mode.
         *
         * The thread main will be directly called and upon exit the ThreadExit function is called.
         *
         * @param t
         * @param thread_exit Address of the ThreadExit function.
         */
        virtual void execute_in_kernel_mode(Thread* t, Register thread_exit) = 0;


        /**
         * @brief Execute the thread main in user mode.
         *
         * A jump to the thread main in user mode will be performed and upon exit the thread must make a system call
         * to terminate itself.
         *
         * @param t
         */
        virtual void execute_in_user_mode(Thread* t) = 0;
    };


    /**
     * @brief Initialize the bootstrap core, that is the CPU core that is initially running when the device was powered
     *          on.
     *
     * The bootstrap core will always have the ID 0.
     *
     * @return True: All CPU features are initialized. False: Failed to initialize the CPU, the kernel boot must
     *          be halted.
     */
    bool init_bootstrap_core();


    /**
     * @brief Try to detect and then initialize all other CPU cores on the device.
     * @return True: All other CPU cores have been initialized and are running. False: At least one CPU core could not
     *          be initialized.
     */
    bool init_other_cores();


    /**
     * @brief The CPU core that is currently running the calling code.
     * @return
     */
    Core* current_core();


    /**
     * @brief The core table contains all other detected CPU cores including the bootstrap core.
     * @return A list of all detected CPU cores.
     */
    LinkedList<Core*> get_core_table();


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Assembly Stuff
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    /**
     * @return The current value of the stack pointer.
     */
    CLINK Register get_stack_pointer();


    /**
     * halt the CPU until an interrupt occurs.
     */
    CLINK void halt();


    /**
     * @brief Get the virtual address that was responsible for a page fault.
     *
     * Important: The returned virtual address is only valid during handling of a page fault otherwise the virtual
     *              address is undefined.
     */
    CLINK Register get_page_fault_address();
}

#endif //RUNEOS_CPU_H
