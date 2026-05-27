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

#include <KRE/Memory.h>
#include <KRE/Stream.h>
#include <KRE/String.h>
#include <KRE/System/Resource.h>

#include <CPU/Threading/Thread.h>

namespace Rune::CPU {
    // Size of a register
#ifdef BIT64
    using Register = U64;
#else
    using Register = U32;
#endif

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      Core API
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * @brief Technical specs of the CPU.
     */
    struct TechSpec {
        String vendor = "";
        String family = "";
        String model  = "";
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
        U8 vector = 0; // The ID of the interrupt vector
        VirtualAddr
            handler_addr{}; // Virtual address of the function handling this interrupt vector
        PrivilegeLevel
             level;    // The privilege level at which this interrupt can be manually triggered
        bool active{}; // True: The interrupt handler is used, False: It is unused.
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
        virtual auto init() -> bool = 0;

        /**
         * @brief Get the unique ID of this CPU core.
         * @return The core ID.
         */
        virtual auto get_id() -> U8 = 0;

        /**
         * @brief Get technical specs of the CPU like the model, etc.
         * @return
         */
        virtual auto get_tech_spec() -> TechSpec = 0;

        /**
         * @brief Get the architectural specs of the CPU like the physical address width, etc.
         * @return
         */
        virtual auto get_arch_details() -> ArchSpec = 0;

        /**
         * @brief
         * @return The privilege level at which the core currently runs.
         */
        virtual auto get_current_privilege_level() -> PrivilegeLevel = 0;

        /**
         * @brief Get the interrupt vector table of the core.
         * @return The interrupt vector table.
         */
        virtual auto get_interrupt_vector_table() -> LinkedList<InterruptVector> = 0;

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
    auto init_boot_core() -> bool;

    /**
     * @brief Try to detect and then initialize all other CPU cores on the device.
     * @return True: All other CPU cores have been initialized and are running. False: At least one
     * CPU core could not be initialized.
     */
    auto init_other_cores() -> bool;

    /**
     * @brief The CPU core that is currently running the calling code.
     * @return
     */
    auto current_core() -> Core*;

    /**
     * @brief The core table contains all other detected CPU cores including the bootstrap core.
     * @return A list of all detected CPU cores.
     */
    auto get_core_table() -> LinkedList<Core*>;

    /**
     *
     * @return The size of a physical address in bits.
     */
    auto get_physical_address_width() -> U8;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Assembly Stuff
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * @return The current value of the stack pointer.
     */
    CLINK auto get_stack_pointer() -> Register;

    /// @brief Pause the CPU in an optimized way in terms of performance/power usage. This function
    ///         is intended to be used when waiting in a loop, e.g. in a spinlock.
    CLINK void pause();

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
    CLINK auto get_page_fault_address() -> Register;

} // namespace Rune::CPU

#endif // RUNEOS_CPU_H
