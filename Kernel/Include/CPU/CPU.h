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

#include <Ember/Ember.h>
#include <Ember/Enum.h>

#include <KRE/String.h>
#include <KRE/Stream.h>

#include <KRE/Memory.h>

namespace Rune::CPU {
    // Size of a register
#ifdef BIT64
    using Register = U64;
#else
    using Register = U32;
#endif

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Threading structures
    //
    // Defined here because they are needed by the "Core" class
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    struct StartInfo;

    /**
     * @brief Main function of a thread. It has the signature int(int, char*[]). Parameters are the
     * number of arguments and a pointer to the array with the string arguments. The return value is
     * the thread status after it finished. status >= 0 -> everything fine, status < 0 -> exit with
     * error.
     */
    using ThreadMain = int (*)(StartInfo*);

    /**
     * @brief Describes what a thread is currently doing.
     * <ul>
     *  <li>Ready: The thread is in the ready queue and waiting to be scheduled.</li>
     *  <li>Running: The thread is in the ready queue and waiting to be scheduled.</li>
     *  <li>Sleeping: The thread is in the sleep queue of a timer.</li>
     *  <li>Waiting: The thread is in the wait queue of a mutex.</li>
     *  <li>Terminated: The thread has finished execution and but it's resources are not freed
     * yet.</li>
     * </ul>
     */
#define THREAD_STATES(X)                                                                           \
    X(ThreadState, READY, 0x1)                                                                     \
    X(ThreadState, RUNNING, 0x2)                                                                   \
    X(ThreadState, SLEEPING, 0x3)                                                                  \
    X(ThreadState, WAITING, 0x4)                                                                   \
    X(ThreadState, TERMINATED, 0x5)

    DECLARE_ENUM(ThreadState, THREAD_STATES, 0x0) // NOLINT

    /**
     * @brief The scheduling policy describes the priority of a group of threads.
     * <ul>
     *  <li>LowLatency: Highest priority.</li>
     *  <li>Normal: </li>
     *  <li>Background: Lowest priority.</li>
     * </ul>
     */
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

    /**
     * @brief The thread arguments, dynamic linker information and other useful information.
     *
     * A thread is either an application main thread or a minor thread. The type of thread
     * determines how much information shall be passed in the start info.
     *
     * <p>
     *  The information passed in the StartInfo is defined as followed:
     *  <ul>
     *   <li>Application Main Thread: All StartInfo information shall be provided.</li>
     *   <li>Minor Thread: Argc, argv and main shall be provided, the state of the other fields is
     * undefined.</li>
     *  </ul>
     * </p>
     */
    struct StartInfo {
        /**
         * @brief Number of arguments.
         */
        int argc;

        /**
         * @brief A null terminated array of string arguments.
         */
        char** argv;

        /**
         * @brief Low and high bytes of a random 16 byte value.
         */
        U64 random_low;
        U64 random_high;

        /**
         * @brief Virtual address of an array where the ELF program headers are stored.
         */
        void* program_header_address;

        /**
         * @brief Size of a program header.
         */
        size_t program_header_size;

        /**
         * @brief Size of the program header array.
         */
        size_t program_header_count;

        /**
         * @brief Main function of the thread.
         */
        ThreadMain main;

        /**
         * @brief Address of a 16 byte random value.
         */
        void* random;
    };

    /**
     * The thread struct contains general information about a running thread.
     */
    struct Thread {
        static constexpr MemorySize KERNEL_STACK_SIZE = 32 * MemoryUnit::KiB;

        // Unique ID of the thread
        U16 handle = 0;

        // Handle of the app the thread belongs to
        U16              app_handle = 0;
        String           name       = "";
        ThreadState      state      = ThreadState::NONE;
        SchedulingPolicy policy     = SchedulingPolicy::NONE;

        // The kernel stack is used whenever kernel code is run e.g. because of an interrupt or
        // syscall It is dynamically allocated on the kernel heap and has a preconfigured fixed size
        U8*         kernel_stack_bottom = nullptr; // Pointer to the heap allocated memory
        VirtualAddr kernel_stack_top    = 0x0;

        // The user mode stack contains application data, is managed by an application.
        Stack user_stack;

        // Address of the base page table defining the threads virtual address space.
        PhysicalAddr base_page_table_address = 0x0;

        /**
         * @brief ID of the mutex this thread is owning at the moment.
         */
        int mutex_id = -1;

        /**
         * @brief ID of the application this thread is waiting for to exit.
         */
        int join_app_id = -1;

        // // Number of arguments in argv
        // int argc = 0;
        // // Null terminated array pointers to the thread arguments
        // char** argv = nullptr;
        // // Main function of the thread
        // ThreadMain main = nullptr;
        /**
         * @brief Thread arguments and more.
         */
        StartInfo* start_info;

        /**
         * The thread control block contains the thread local storage (TLS) and other data, it is
         * maintained by libc. We simply provide easy access to it through an arch specific TLS
         * register.
         */
        void* thread_control_block = nullptr;

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
     *  <li>Kernel: The running program can access all kernel and user memory and run any assembly
     * command.</li> <li>User:   The running program can only access user memory and may not be able
     * to run all assembly commands. Disallowed assembly commands are CPU specific.</li>
     * </ul>
     *
     */
#define PRIVILEGE_LEVELS(X)                                                                        \
    X(PrivilegeLevel, KERNEL, 0x1)                                                                 \
    X(PrivilegeLevel, USER, 0x2)

    DECLARE_TYPED_ENUM(PrivilegeLevel, U8, PRIVILEGE_LEVELS, 0x0) // NOLINT

    /**
     * @brief General information about an entry in the interrupt vector table of the processor.
     */
    struct InterruptVector {
        const U8 vector = 0; // The ID of the interrupt vector
        const VirtualAddr
            handler_addr; // Virtual address of the function handling this interrupt vector
        const PrivilegeLevel
                   level;  // The privilege level at which this interrupt can be manually triggered
        const bool active; // True: The interrupt handler is used, False: It is unused.
    };

    /**
     * @brief A single core on the CPU.
     */
    class Core {
      public:
        virtual ~Core() = default;

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
         * @brief Write the current values of general purpose registers and CPU specific structures
         * to the logger.
         * @param stream
         */
        virtual void dump_core_state(const SharedPointer<TextStream>& stream) = 0;

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
         * A jump to the thread main in user mode will be performed and upon exit the thread must
         * make a system call to terminate itself.
         *
         * @param t
         */
        virtual void execute_in_user_mode(Thread* t) = 0;

        /**
         * Update the TLS struct of the running thread.
         * @param tls_ptr Pointer to a TSL struct.
         */
        virtual void update_thread_local_storage(void* tls_ptr) = 0;
    };

    /**
     * @brief Initialize the bootstrap core, that is the CPU core that is initially running when the
     * device was powered on.
     *
     * The bootstrap core will always have the ID 0.
     *
     * @return True: All CPU features are initialized. False: Failed to initialize the CPU, the
     * kernel boot must be halted.
     */
    bool init_bootstrap_core();

    /**
     * @brief Try to detect and then initialize all other CPU cores on the device.
     * @return True: All other CPU cores have been initialized and are running. False: At least one
     * CPU core could not be initialized.
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
     * Important: The returned virtual address is only valid during handling of a page fault
     * otherwise the virtual address is undefined.
     */
    CLINK Register get_page_fault_address();
} // namespace Rune::CPU

#endif // RUNEOS_CPU_H
