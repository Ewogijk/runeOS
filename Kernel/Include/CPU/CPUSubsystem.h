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

#ifndef RUNEOS_CPUSUBSYSTEM_H
#define RUNEOS_CPUSUBSYSTEM_H


#include <Hammer/Utility.h>

#include <LibK/Subsystem.h>

#include <CPU/CPU.h>
#include <CPU/Interrupt/Interrupt.h>
#include <CPU/Interrupt/IRQ.h>

#include <CPU/Threading/Scheduler.h>
#include <CPU/Threading/Mutex.h>

#include <CPU/Time/PIT.h>


namespace Rune::CPU {

    /**
     * @brief All CPU subsystem event hooks.
     * <ul>
     *  <li>THREAD_CREATED: A new thread object was created and is about to be scheduled
     *                          Event Context - Thread*: The created thread.</li>
     *  <li>THREAD_TERMINATED: A thread has returned from main or requested termination.
     *                          Event Context - ThreadTerminatedContext*: Contains pointers to the terminated and next
     *                          to be scheduled thread.</li>
     *  <li>CONTEXT_SWITCH: A context switch is about to happen. Event Context - Thread*: The next thread that will be
     *                      scheduled.</li>
     * </ul>
     */
#define CPU_EVENT_HOOKS(X)                          \
             X(EventHook, THREAD_CREATED, 0x1)      \
             X(EventHook, THREAD_TERMINATED, 0x2)   \
             X(EventHook, CONTEXT_SWITCH, 0x3)      \



    DECLARE_ENUM(EventHook, CPU_EVENT_HOOKS, 0x0)  // NOLINT


    /**
     * @brief Event context of the "ThreadTerminated" event hook.
     */
    struct ThreadTerminatedContext {
        Thread* terminated     = nullptr;
        Thread* next_scheduled = nullptr;
    };


    class Subsystem : public LibK::Subsystem {
        static constexpr char const* BOOTSTRAP_THREAD_NAME  = "Bootstrap";
        static constexpr char const* TERMINATOR_THREAD_NAME = "The Terminator";
        static constexpr char const* IDLE_THREAD_NAME       = "Idle";

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          Interrupt Properties
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        LinkedList<UniquePointer<PICDriver>> _pic_driver_table;
        PICDriver* _active_pic;


        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          Threading Properties
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        HashMap<U16, SharedPointer<Thread>> _thread_table;
        LibK::TableFormatter<Thread>        _thread_table_fmt;
        LibK::HandleCounter<U16>            _thread_handle_counter;

        HashMap<U16, SharedPointer<Mutex>> _mutex_table;
        LibK::TableFormatter<Mutex>        _mutex_table_fmt;
        LibK::HandleCounter<U16>           _mutex_handle_counter;
        Scheduler                          _scheduler;


        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          Time Properties
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        UniquePointer<Timer> _timer;


        SharedPointer<Thread> create_thread(
                const String& thread_name,
                ThreadMain t_main,
                int argc,
                char* argv[],
                LibK::PhysicalAddr base_pt_addr,
                SchedulingPolicy policy,
                Stack user_stack
        );


    public:

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          Constructors&Destructors
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        Subsystem();


        ~Subsystem() override = default;


        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          Kernel Subsystem Functions
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


        [[nodiscard]] String get_name() const override;


        bool start(const LibK::BootLoaderInfo& evt_ctx, const LibK::SubsystemRegistry& k_subsys_reg) override;


        void set_logger(SharedPointer<LibK::Logger> logger) override;


        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          Interrupt API
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


        /**
         * @brief
         * @return The actively used PIC driver.
         */
        PICDriver* get_active_pic();


        /**
         * @brief The PIC driver table contains all installed PIC drivers.
         * @return A list of installed PIC drivers.
         */
        LinkedList<PICDriver*> get_pic_driver_table();


        /**
         * @brief Install a driver PIC driver that will be responsible for IRQ handling.
         *
         * During CPU init all installed drivers will be asked to try to detect their device in order of installation
         * and the first driver to detect their device will handle interrupt requests.
         *
         * @param driver Pointer to a PIC driver.
         *
         * @returns True: The PIC driver is installed, False: It is not.
         */
        bool install_pic_driver(UniquePointer<PICDriver> driver);


        /**
         * @brief Install the IRQ handler for a device on the specified IRQ line.
         * @param irq_line   Requested IRQ.
         * @param dev_handle Unique device handle.
         * @param dev_name   Name of the device.
         * @param handler
         * @return True: The IRQ handler is installed. False: Installation failed.
         */
        bool install_irq_handler(U8 irq_line, U16 dev_handle, const String& dev_name, const IRQHandler& handler);


        /**
         * @brief Uninstall the IRQ handler for the given device ID from the specified IRQ line.
         * @param irq_line
         * @param dev_handle
         * @return True: The IRQ handler is uninstalled. False: Uninstalling failed.
         */
        bool uninstall_irq_handler(U8 irq_line, U16 dev_handle);


        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                      High Level Threading API
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


        /**
         * @brief Get the scheduler itself, which gives access to the Low Level Threading API.
         * @return The scheduler.
         *
         */
        Scheduler* get_scheduler();


        /**
         * @brief A list of all threads in the system.
         * @return The thread table.
         */
        LinkedList<Thread*> get_thread_table();


        /**
         * @brief Dump the thread table to the stream.
         * @param logger
         */
        void dump_thread_table(const SharedPointer<LibK::TextStream>& stream) const;


        /**
         * @brief Get a thread with the given ID.
         * @param id ID of a thread.
         * @return Pointer to the thread with the requested ID or a null pointer if not found.
         */
        Thread* find_thread(int handle);


        /**
         * @brief Allocate memory for a new thread structure, put it in the thread table and enqueue it to be scheduled
         *          in the future.
         *
         * Each thread will be assigned a unique ID, the kernel stack is allocated and setup for the first context
         * switch. As part of the setup a null frame is pushed onto the stack to enable stack tracing.
         *
         * <p>
         *  Note: The user stack must already be setup! The scheduler cannot do so, because the user stack may be in
         *  another VAS and therefore inaccessible.
         * </p>
         *
         * @param thread_name    Name of the thread.
         * @param t_main         Main function of the thread. It has the signature int(int, char*[]). Parameters are the
         *                          number of arguments and a pointer to the array with the string arguments. The return
         *                          value is the thread status after it finished. status >= 0 -> everything fine,
         *                          status < 0 -> exit with error.
         * @param argc           The number of arguments for the thread.
         * @param argv           A null terminated array of string arguments for the thread.
         * @param base_pt_addr   Address of the base page table defining the virtual address space of the thread.
         * @param policy         Scheduling policy to use for the thread.
         * @param user_stack     The user mode stack.
         *
         * @return The ID if the scheduled thread, 0 if the thread could not be created or scheduled.
         */
        U16 schedule_new_thread(
                const String& thread_name,
                ThreadMain t_main,
                int argc,
                char* argv[],
                LibK::PhysicalAddr base_pt_addr,
                SchedulingPolicy policy,
                Stack user_stack
        );


        /**
         * @brief Mark the thread with the requested handle as terminated, except if it is the running thread. The
         *          thread will no longer be scheduled and after the next context switch its allocated memory will be
         *          freed.
         *
         * <p>
         *  The function will try to determine the location of the thread based on it's current state. For example: If
         *  the thread is in the "Sleeping" state, it will be removed from the timers wait queue and put into the
         *  terminated threads queue.
         * </p>
         * <p>
         *  The function is guaranteed to always exit, that is it will never trigger a context switch by itself. This is
         *  the reason why the currently running thread cannot be terminated, since this inevitable triggers a context
         *  switch.
         * </p>
         *
         * @param handle
         * @return True: The thread is marked as terminated, False: No thread with the ID was found or it is currently
         *          running.
         */
        bool terminate_thread(int handle);


        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          Mutex API
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


        /**
         * @brief A list of all currently acquired mutexes.
         * @return The mutex table.
         */
        LinkedList<Mutex*> get_mutex_table();


        /**
         * @brief Try to find the mutex with the given handle.
         * @param mutex_handle Handle of a mutex.
         * @return A pointer to a mutex, null if not found.
         */
        SharedPointer<Mutex> find_mutex(U16 mutex_handle);


        /**
         * @brief Dump the mutex table to the stream.
         * @param logger
         * @param logLvl
         */
        void dump_mutex_table(const SharedPointer<LibK::TextStream>& stream) const;


        /**
         * @brief Create a new mutex instance with the given name and add it to the mutex table.
         *
         * Each acquired mutex must be freed via the ReleaseMutex() function to avoid leaked resources.
         *
         * @param name Name of the mutex.
         * @return A new mutex instance.
         */
        SharedPointer<Mutex> create_mutex(String name);


        /**
         * @brief Free the memory of the mutex with the given handle.
         *
         * The caller of this functions is responsible to free the shared pointer on it's own end to avoid leaks.
         *
         * @param mutex_handle
         * @return True: The mutex was released, False: No mutex with the given ID was found.
         */
        bool release_mutex(U16 mutex_handle);


        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          Time Functions
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


        /**
         * A driver for a timer.
         *
         * @param driver
         */
        void install_timer_driver(UniquePointer<Timer> driver);


        /**
         *
         * @return
         */
        Timer* get_system_timer();
    };


    /**
     * @brief Mark the currently running thread as terminated which will immediately trigger a context switch to the
     *          next thread.
     *
     * This is the clean and advised way of terminating a thread whose main function has returned.
     * @param exit_code Exit code as returned by the main function of the thread.
     */
    void thread_exit(int exit_code);

}

#endif //RUNEOS_CPUSUBSYSTEM_H
