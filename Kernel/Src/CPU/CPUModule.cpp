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

#include <CPU/CPUModule.h>

#include <Memory/Paging.h>

#include <CPU/Threading/CriticalSection.h>

namespace Rune::CPU {
    const SharedPointer<Logger> LOGGER = LogContext::instance().get_logger("CPU.CPUModule");

    // NOLINTBEGIN
    Function<void(Thread*, Thread*)> ON_THREAD_STOPPED = [](Thread* term, Thread* next) {
        SILENCE_UNUSED(term)
        SILENCE_UNUSED(next)
    };
    // NOLINTEND

    void thread_exit(int exit_code) {
        // Use the raw pointer to avoid referencing the shared pointer which will never be cleaned
        // up because of the context switch in "unlock", so C++ never gets to call the destructor on
        // "t"
        auto* t = g_scheduler.get_running_thread().get();
        LOGGER->trace(R"(Thread {} has finished. Exit Code: {})", t->get_unique_name(), exit_code);

        g_scheduler.stop();
    }

    void thread_enter() {
        g_scheduler.on_thread_enter();
        // Use raw pointer -> See "thread_exit" for explanation
        auto* t = g_scheduler.get_running_thread().get();
        if (t->user_stack.stack_top == 0) {
            LOGGER->trace("Will execute main in kernel mode.");
            current_core()->execute_in_kernel_mode(t, memory_pointer_to_addr(&thread_exit));
        } else {
            LOGGER->trace("Will execute main in user mode.");
            current_core()->execute_in_user_mode(t);
        }
    }

    auto idle_thread(StartInfo* start_info) -> int {
        SILENCE_UNUSED(start_info)
        for (;;) {
            interrupt_irq_enable();
            halt();
            interrupt_irq_disable();
        }
        return 0;
    }

    auto thread_garbage_collector(StartInfo* start_info) -> int {
        SILENCE_UNUSED(start_info)
        for (;;) {
            interrupt_irq_disable();
            auto*                           tgb = g_scheduler.get_thread_garbage_bin();
            Optional<SharedPointer<Thread>> cT  = tgb->remove_front();
            while (cT) {
                auto dT = cT;
                cT      = tgb->remove_front();
                LOGGER->trace(R"(Terminating thread: {})", dT.value()->get_unique_name());

                auto* next = g_scheduler.get_ready_queue()->peek();
                if (next == nullptr) next = g_scheduler.get_idle_thread().get();
                ON_THREAD_STOPPED(forward<Thread*>(dT.value().get()), forward<Thread*>(next));
                delete[] dT.value()->kernel_stack_bottom;

                if (dT.value().get_ref_count() > 1) {
                    LOGGER->warn(
                        R"(>> Memory Leak << - {} has {} references but expected 1. Thread struct will not be freed.)",
                        dT.value()->get_unique_name(),
                        dT.value().get_ref_count());
                }
                // dT gets deleted here after it goes out of scope
            }
            g_scheduler.await_block();
            interrupt_irq_enable();
            g_scheduler.block();
        }
        return 0;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Subsystem
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    DEFINE_ENUM(EventHook, CPU_EVENT_HOOKS, 0x0)

    char*     CPUModule::DUMMY_ARGS[]; // NOLINT Array disallowed! Is part of Kernel ABI
    StartInfo CPUModule::GCT_START_INFO;
    StartInfo CPUModule::IDLE_THREAD_START_INFO;

    auto create_thread(const String&    thread_name,
                       StartInfo*       start_info,
                       PhysicalAddr     base_pt_addr,
                       SchedulingPolicy policy,
                       Stack            user_stack) -> SharedPointer<Thread> {
        SharedPointer<Thread> new_thread    = g_thread_cache.allocate(thread_name);
        new_thread->start_info              = start_info;
        new_thread->base_page_table_address = base_pt_addr;
        new_thread->policy                  = policy;
        new_thread->user_stack              = move(user_stack);
        return new_thread;
    }

    CPUModule::CPUModule() = default;

    auto CPUModule::get_name() const -> String { return "CPU"; }

    auto CPUModule::load(const BootInfo& boot_info) -> bool {

        // Init Event Hook table
        _event_hook_table.put(EventHook(EventHook::THREAD_CREATED).to_string(),
                              LinkedList<EventHandlerTableEntry>());
        _event_hook_table.put(EventHook(EventHook::THREAD_STOPPED).to_string(),
                              LinkedList<EventHandlerTableEntry>());
        _event_hook_table.put(EventHook(EventHook::THREAD_PREEMPTED).to_string(),
                              LinkedList<EventHandlerTableEntry>());

        // Init Interrupts/IRQs
        LOGGER->debug("Loading interrupt vector table...");
        interrupt_load_vector_table();
        if (_pic_driver_table.empty()) {
            LOGGER->critical("No PIC drivers are installed...");
            return false;
        }
        LOGGER->debug("Trying to detect a PIC device...");
        int pic_idx = irq_init(get_pic_driver_table());
        if (pic_idx < 0) {
            LOGGER->critical("No PIC device could be detected...");
            return false;
        }
        _active_pic = _pic_driver_table[pic_idx].get();
        LOGGER->debug(R"("{}" has been initialized.)", _active_pic->get_name());

        // Init Scheduling
        LOGGER->debug("Starting the Scheduler...");
        PhysicalAddr base_pt_addr = Memory::get_base_page_table_address();
        // Set the code running since the start of the machine as the initial thread
        // No "main" is needed as the code is already running
        // The idea is to make further initializations and then ditch the bootstrap thread as soon
        // as possible because the stack provided by limine lies in a reclaimed memory region
        // thus it will be reused sooner than later and the initial thread will crash at this point
        auto bootstrap_thread = g_thread_cache.allocate(BOOTSTRAP_THREAD_NAME);

        bootstrap_thread->base_page_table_address = base_pt_addr;
        bootstrap_thread->kernel_stack_top        = boot_info.stack;
        bootstrap_thread->policy                  = SchedulingPolicy::LOW_LATENCY;

        DUMMY_ARGS[0]       = nullptr;
        GCT_START_INFO.argc = 0;
        GCT_START_INFO.argv = DUMMY_ARGS;
        GCT_START_INFO.main = &thread_garbage_collector;
        auto garbage_collector_thread =
            create_thread(GARBAGE_COLLECTOR_THREAD_NAME,
                          &GCT_START_INFO,
                          base_pt_addr,
                          SchedulingPolicy::NONE,
                          {.stack_bottom = nullptr, .stack_top = 0x0, .stack_size = 0x0});
        IDLE_THREAD_START_INFO.argc = 0;
        IDLE_THREAD_START_INFO.argv = DUMMY_ARGS;
        IDLE_THREAD_START_INFO.main = &idle_thread;
        auto le_idle_thread =
            create_thread(IDLE_THREAD_NAME,
                          &IDLE_THREAD_START_INFO,
                          base_pt_addr,
                          SchedulingPolicy::NONE,
                          {.stack_bottom = nullptr, .stack_top = 0x0, .stack_size = 0x0});
        if (!g_scheduler.init(bootstrap_thread,
                              le_idle_thread,
                              garbage_collector_thread,
                              &thread_enter)) {
            LOGGER->critical("Failed to start the scheduler!");
            return false;
        }
        ON_THREAD_STOPPED = [this](Thread* term, Thread* next) -> void {
            ThreadPreemptionContext tt_ctx = {.stopped = move(term), .next_scheduled = move(next)};

            // Wake up joining threads
            auto maybe_wait_list = _on_stop_syncing_threads.find(term->get_handle());
            if (maybe_wait_list != _on_stop_syncing_threads.end()) {
                for (SharedPointer<Thread>& t : *maybe_wait_list->value) {
                    t->m_sync_stop_thread_handle = Resource<ThreadHandle>::HANDLE_NONE;
                    g_scheduler.unblock(t);
                }
                maybe_wait_list->value->clear();
                _on_stop_syncing_threads.remove(term->get_handle());
            }

            fire(EventHook(EventHook::THREAD_STOPPED).to_string(),
                 reinterpret_cast<void*>(&tt_ctx));

            if (g_thread_cache.free(term->get_handle())) {
                LOGGER->trace(R"(Removed {} from the thread cache)", term->get_unique_name());
            } else {
                LOGGER->warn(R"({} was not found in the thread cache)", term->get_unique_name());
            }
        };
        g_scheduler.set_on_context_switch([this](Thread* next) -> void {
            fire(EventHook(EventHook::THREAD_PREEMPTED).to_string(), reinterpret_cast<void*>(next));
        });

        // Init Timer
        LOGGER->debug("Starting the timer...");
        if (!_timer) {
            LOGGER->critical("No timer driver installed!");
            return false;
        }
        constexpr U64 TIMER_FREQ = 1000;
        constexpr U32 QUANTUM    = 50000000; // Each thread can run for a maximum of 50ms at a time
        if (!_timer->start(&g_scheduler, TimerMode::PERIODIC, TIMER_FREQ, QUANTUM)) {
            LOGGER->critical("Could not start the timer!");
            return false;
        }

        LOGGER->debug("Detecting other CPU cores...");
        if (!init_other_cores()) {
            LOGGER->critical("Failed to detect other CPU cores!");
            return false;
        }
        return true;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Interrupt functions
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    auto CPUModule::get_active_pic() -> PICDriver* { return _active_pic; }

    auto CPUModule::get_pic_driver_table() -> LinkedList<PICDriver*> {
        LinkedList<PICDriver*> dt;
        for (auto& d : _pic_driver_table) dt.add_back(d.get());
        return dt;
    }

    auto CPUModule::install_pic_driver(UniquePointer<PICDriver> driver) -> bool {
        if (!driver) return false;
        _pic_driver_table.add_back(move(driver));
        return true;
    }

    // NOLINTBEGIN For consistency, these are members
    auto CPUModule::install_irq_handler(U8                          irq_line,
                                        U16                         dev_handle,
                                        const String&               dev_name,
                                        const FastInterruptHandler& handler) -> bool {
        return irq_install_handler(irq_line, dev_handle, dev_name, handler);
    }

    auto CPUModule::uninstall_irq_handler(U8 irq_line, U16 dev_handle) -> bool {
        return irq_uninstall_handler(irq_line, dev_handle);
    }

    // NOLINTEND
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      High Level Threading API
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    // NOLINTBEGIN Kept for backwards compatibility
    auto CPUModule::get_scheduler() -> Scheduler* { return &g_scheduler; }

    auto CPUModule::get_thread_table() -> LinkedList<SharedPointer<Thread>> {
        return g_thread_cache.get_resources();
    }

    void CPUModule::dump_thread_table(const SharedPointer<TextStream>& stream) const {
        g_thread_cache.print(stream);
    }

    auto CPUModule::find_thread(Handle handle) -> SharedPointer<Thread> {
        return g_thread_cache.find(handle);
    }
    // NOLINTEND

    auto CPUModule::schedule_new_thread(const String&    thread_name,
                                        StartInfo*       start_info,
                                        PhysicalAddr     base_pt_addr,
                                        SchedulingPolicy policy,
                                        Stack            user_stack) -> ThreadHandle {
        SharedPointer<Thread> new_thread =
            create_thread(thread_name, move(start_info), base_pt_addr, policy, move(user_stack));
        fire(EventHook(EventHook::THREAD_CREATED).to_string(), new_thread.get());
        if (!g_scheduler.schedule(new_thread)) {
            g_thread_cache.free(new_thread->get_handle());
            return Resource<ThreadHandle>::HANDLE_NONE;
        }

        return new_thread->get_handle();
    }

    auto CPUModule::stop_thread(int handle) -> bool { // NOLINT
        // Check if a thread with the handle exists
        auto thread_to_stop = g_thread_cache.find(handle);
        if (!thread_to_stop) {
            LOGGER->debug("No thread with handle {} exists", handle);
            return false;
        }

        {
            // Disable external interrupts, thus disabling context switches
            // Fixes memory leak warnings in case the calling thread is preempted and thread_to_stop
            // stops execution -> Thread garbage collector would report a memory leak because
            // thread_to_stop holds a reference
            CriticalSection<InterruptSaveLock> _(m_lock);
            // Check where the thread currently is e.g. locked by a mutex and remove it from the
            // queue
            LOGGER->trace(R"(Terminating thread {})", thread_to_stop->get_unique_name());
            switch (thread_to_stop->state) { // NOLINT All cases are handled
                case ThreadState::NONE:
                    LOGGER->error(R"({} has invalid state "None".)",
                                  thread_to_stop->get_unique_name());
                    return false;
                case ThreadState::CREATED:
                    // NOOP -> Thread has been created and is not yet scheduled, thus there is no
                    // need to stop it
                    return true;
                case ThreadState::READY:
                    if (!g_scheduler.get_ready_queue()->remove(handle)) {
                        LOGGER->error(R"({} is missing from the ready queue.)",
                                      thread_to_stop->get_unique_name());
                        return false;
                    }
                    break;
                case ThreadState::RUNNING:
                    // Do not stop the running thread because we do not want a context switch to
                    // happen
                    LOGGER->trace(R"({} is running, will not stop.)",
                                  thread_to_stop->get_unique_name());
                    return true;
                case ThreadState::AWAIT_BLOCK:
                    if (g_scheduler.get_running_thread()->get_handle()
                        == static_cast<ThreadHandle>(handle)) {
                        LOGGER->trace(R"({} is running, will not stop.)",
                                      thread_to_stop->get_unique_name());
                        return true;
                    } else {
                        if (!g_scheduler.get_ready_queue()->remove(handle)) {
                            LOGGER->error(R"({} is missing from the ready queue.)",
                                          thread_to_stop->get_unique_name());
                            return false;
                        }
                    }
                    break;
                case ThreadState::BLOCKED:
                    if (thread_to_stop->timer_handle > 0) {
                        if (!_timer->remove_sleeping_thread(handle)) {
                            LOGGER->error(R"({} is missing from the wait queue of the timer.)",
                                          thread_to_stop->get_unique_name());
                            return false;
                        }
                    } else if (thread_to_stop->mutex_handle > 0) {
                        SharedPointer<Mutex> m(nullptr);
                        for (const auto& mm : g_mutex_cache.get_resources()) {
                            if (mm->get_handle() // NOLINT Only end() is null
                                == thread_to_stop->mutex_handle) {
                                m = mm;
                                break;
                            }
                        }
                        if (!m) {
                            LOGGER->error("No mutex with handle {} was found.",
                                          thread_to_stop->get_handle(),
                                          thread_to_stop->get_name(),
                                          thread_to_stop->mutex_handle);
                            return false;
                        }

                        if (!m->remove_thread(thread_to_stop->get_handle())) {
                            LOGGER->error(R"({} was not the owner or in the waiting queue of {})",
                                          thread_to_stop->get_unique_name(),
                                          m->get_unique_name());
                            return false;
                        }
                    }
                    break;
                case ThreadState::STOPPED:
                    LOGGER->trace(R"({} is already stopped.)", thread_to_stop->get_unique_name());
                    return true;
            }
        }

        g_scheduler.stop(thread_to_stop);
        return true;
    }

    auto CPUModule::sync_with_thread_stop(ThreadHandle handle) -> bool {
        CriticalSection<InterruptSaveLock> _(m_lock);
        if (!g_thread_cache.find(handle)) return false;
        auto                               calling_thread  = g_scheduler.get_running_thread();
        auto                               maybe_wait_list = _on_stop_syncing_threads.find(handle);
        if (maybe_wait_list == _on_stop_syncing_threads.end()) {
            _on_stop_syncing_threads[handle] = LinkedList<SharedPointer<Thread>>();
            maybe_wait_list                  = _on_stop_syncing_threads.find(handle);
        }
        LOGGER->trace("{} R{} sync on {} R{} stop",
                      calling_thread->get_unique_name(),
                      calling_thread.get_ref_count(),
                      g_thread_cache.find(handle)->get_unique_name(),
                      g_thread_cache.find(handle).get_ref_count());
        maybe_wait_list->value->add_back(calling_thread);
        calling_thread->m_sync_stop_thread_handle = handle;
        g_scheduler.await_block();
        g_scheduler.block(calling_thread);
        return true;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Mutex API
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    // NOLINTBEGIN kept for backwards compatibility
    auto CPUModule::get_mutex_table() -> LinkedList<SharedPointer<Mutex>> {
        return g_mutex_cache.get_resources();
    }

    auto CPUModule::find_mutex(U16 mutex_handle) -> SharedPointer<Mutex> {
        return g_mutex_cache.find(mutex_handle);
    }

    void CPUModule::dump_mutex_table(const SharedPointer<TextStream>& stream) const {
        return g_mutex_cache.print(stream);
    }

    auto CPUModule::create_mutex(String name) -> SharedPointer<Mutex> {
        return g_mutex_cache.allocate(move(name));
    }

    auto CPUModule::release_mutex(U16 mutex_handle) -> bool {
        return g_mutex_cache.free(mutex_handle);
    }
    // NOLINTEND

    // ========================================================================================== //
    // Semaphore API
    // ========================================================================================== //

    // NOLINTBEGIN Kept for backward compatibility
    auto CPUModule::get_semaphore_table() -> LinkedList<SharedPointer<Semaphore>> {
        return g_semaphore_cache.get_resources();
    }

    auto CPUModule::find_semaphore(SemaphoreHandle handle) -> SharedPointer<Semaphore> {
        return g_semaphore_cache.find(handle);
    }

    void CPUModule::dump_semaphore_table(const SharedPointer<TextStream>& stream) const {
        g_semaphore_cache.print(stream);
    }

    auto CPUModule::create_semaphore(String name, int counter_start, int counter_max)
        -> SharedPointer<Semaphore> {
        return g_semaphore_cache.allocate(name, counter_start, counter_max);
    }

    auto CPUModule::free_semaphore(SemaphoreHandle handle) -> bool {
        return g_semaphore_cache.free(handle);
    }
    // NOLINTEND

    // ========================================================================================== //
    // Time API
    // ========================================================================================== //

    void CPUModule::install_timer_driver(UniquePointer<Timer> driver) {
        if (driver) _timer = move(driver);
    }

    auto CPUModule::get_system_timer() -> Timer* { return _timer.get(); }
} // namespace Rune::CPU
