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

#include <CPU/CPUSubsystem.h>

#include <Memory/Paging.h>


namespace Rune::CPU {
    constexpr char const* FILE      = "CPU";
    Scheduler           * SCHEDULER = nullptr;
    SharedPointer<Logger>      SCHED_LOGGY;
    Function<void(Thread*, Thread*)> NOTIFY_THREAD_BOOM = [](Thread* term, Thread* next) {
        SILENCE_UNUSED(term)
        SILENCE_UNUSED(next)
    };


    void thread_exit(int exit_code) {
        // Use the raw pointer to avoid referencing the shared pointer which will never be cleaned up
        // because of the context switch in "unlock", so C++ never gets to call the destructor on "t"
        auto* t = SCHEDULER->get_running_thread().get();
        SCHED_LOGGY->trace(FILE, R"(Thread "{}-{}" has finished. Exit Code: {})", t->handle, t->name, exit_code);

        SCHEDULER->lock();
        SCHEDULER->terminate();
        SCHEDULER->unlock();
    }


    void thread_enter() {
        SCHEDULER->unlock();
        // Use raw pointer -> See "ThreadExit" for explanation
        auto* t = SCHEDULER->get_running_thread().get();
        if (!t->user_stack.stack_top) {
            SCHED_LOGGY->trace(FILE, "Will execute main in kernel mode.");
            current_core()->execute_in_kernel_mode(t, (uintptr_t) &thread_exit);
        } else {
            SCHED_LOGGY->trace(FILE, "Will execute main in user mode.");
            current_core()->execute_in_user_mode(t);
        }
    }


    int idle_thread(StartInfo* start_info) {
        SILENCE_UNUSED(start_info)
        for (;;) {
            interrupt_enable();
            halt();
            interrupt_disable();
        }
        return 0;
    }


    int terminator_thread(StartInfo* start_info) {
        SILENCE_UNUSED(start_info)
        for (;;) {
            SCHEDULER->lock();
            auto* terminated_threads = SCHEDULER->get_terminated_threads();
            SharedPointer<Thread> cT = !terminated_threads->is_empty()
                                       ? *terminated_threads->head()
                                       : SharedPointer<Thread>(nullptr);
            terminated_threads->remove_front();
            while (cT) {
                auto dT = cT;
                cT = !terminated_threads->is_empty()
                     ? *terminated_threads->head()
                     : SharedPointer<Thread>(nullptr);
                terminated_threads->remove_front();
                SCHED_LOGGY->trace(
                        FILE,
                        R"(Terminating thread: "{}-{}")",
                        dT->handle,
                        dT->name
                );

                auto next = SCHEDULER->get_ready_queue()->peek();
                if (!next)
                    next = SCHEDULER->get_idle_thread().get();
                NOTIFY_THREAD_BOOM(forward<Thread*>(dT.get()), forward<Thread*>(next));
                delete[] dT->kernel_stack_bottom;

                if (dT.get_ref_count() > 1) {
                    SCHED_LOGGY->warn(
                            FILE,
                            R"(>> Memory Leak << - "{}-{}" has {} references but expected 1. Thread struct will not be freed.)",
                            dT->handle,
                            dT->name,
                            dT.get_ref_count()
                    );
                }
                // dT gets deleted here after it goes out of scope
            }
            SCHEDULER->get_running_thread()->state = ThreadState::WAITING;
            SCHEDULER->execute_next_thread();
            SCHEDULER->unlock();
        }
        return 0;
    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                          Subsystem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    DEFINE_ENUM(EventHook, CPU_EVENT_HOOKS, 0x0)

    char* CPUSubsystem::DUMMY_ARGS[];
    StartInfo CPUSubsystem::TERMINATOR_THREAD_START_INFO;
    StartInfo CPUSubsystem::IDLE_THREAD_START_INFO;

    SharedPointer<Thread> CPUSubsystem::create_thread(
            const String& thread_name,
            StartInfo* start_info,
            PhysicalAddr base_pt_addr,
            SchedulingPolicy policy,
            Stack user_stack
    ) {
        SharedPointer<Thread> new_thread(new Thread);
        new_thread->name                    = move(thread_name);
        new_thread->start_info = start_info;
        new_thread->base_page_table_address = base_pt_addr;
        new_thread->policy                  = policy;
        new_thread->user_stack = move(user_stack);
        fire(EventHook(EventHook::THREAD_CREATED).to_string(), new_thread.get());
        return new_thread;
    }


    CPUSubsystem::CPUSubsystem() :
            Subsystem(),
            _pic_driver_table(),
            _active_pic(nullptr),
            _thread_table(),
            _thread_table_fmt(),
            _thread_handle_counter(),
            _mutex_table(),
            _mutex_table_fmt(),
            _mutex_handle_counter(),
            _scheduler(),
            _timer() {

    }


    String CPUSubsystem::get_name() const {
        return "CPU";
    }


    bool CPUSubsystem::start(
            const BootLoaderInfo& boot_info,
            const SubsystemRegistry& k_subsys_reg
    ) {
        SILENCE_UNUSED(k_subsys_reg)

        // Init Event Hook table
        _event_hook_table.put(
                EventHook(EventHook::THREAD_CREATED).to_string(),
                LinkedList<EventHandlerTableEntry>()
        );
        _event_hook_table.put(
                EventHook(EventHook::THREAD_TERMINATED).to_string(),
                LinkedList<EventHandlerTableEntry>()
        );
        _event_hook_table.put(
                EventHook(EventHook::CONTEXT_SWITCH).to_string(),
                LinkedList<EventHandlerTableEntry>());

        install_event_handler(
                EventHook(EventHook::THREAD_TERMINATED).to_string(),
                "Thread Table Cleaner",
                [this](void* evt_ctx) {
                    auto* ctx = (ThreadTerminatedContext*) evt_ctx;
                    SharedPointer<Thread> to_remove(nullptr);
                    for (auto& t: _thread_table) {
                        if (ctx->terminated->handle == (*t.value)->handle) {
                            to_remove = *t.value;
                            break;
                        }
                    }

                    if (to_remove) {
                        _logger->trace(
                                FILE,
                                R"(Removing "{}-{}" from the thread table.)",
                                to_remove->handle,
                                to_remove->name
                        );
                        _thread_table.remove(to_remove->handle);
                    } else {
                        _logger->warn(
                                FILE,
                                R"(Terminated thread "{}-{}" was not found in the thread table.)",
                                ctx->terminated->handle,
                                ctx->terminated->name
                        );
                    }
                }
        );


        //Init Resource Tables
        LinkedList<Column<Thread>> tt_cols;
        tt_cols.add_back(Column<Thread>::make_handle_column_table(26));
        tt_cols.add_back({ "State", 12, [](Thread* t) { return t->state.to_string(); }});
        tt_cols.add_back({ "Policy", 12, [](Thread* t) { return t->policy.to_string(); }});
        tt_cols.add_back(
                {
                        "App",
                        5,
                        [](Thread* t) { return String::format("{}", t->app_handle); }
                }
        );
        _thread_table_fmt.configure("Thread", tt_cols);

        LinkedList<Column<Mutex>> mt_cols;
        mt_cols.add_back(Column<Mutex>::make_handle_column_table(26));
        mt_cols.add_back(
                {
                        "Owner",
                        26,
                        [](Mutex* m) {
                            Thread* owner = m->get_owner();
                            return owner ? String::format("{}-{}", owner->handle, owner->name) : "-";
                        }
                }
        );
        mt_cols.add_back(
                {
                        "WaitQueue",
                        52,
                        [](Mutex* m) {
                            String waiting_threads = "";
                            for (auto& t: m->get_waiting_threads())
                                waiting_threads += String::format("{}-{}, ", t->handle, t->name);

                            if (waiting_threads.is_empty())
                                waiting_threads = "-";
                            return waiting_threads;
                        }
                }
        );
        _mutex_table_fmt.configure("Mutex", mt_cols);

        // Init Interrupts/IRQs
        _logger->debug(FILE, "Loading interrupt vector table...");
        interrupt_load_vector_table();
        if (_pic_driver_table.is_empty()) {
            _logger->critical(FILE, "No PIC drivers are installed...");
            return false;
        }
        _logger->debug(FILE, "Trying to detect a PIC device...");
        int pic_idx = irq_init(get_pic_driver_table());
        if (pic_idx < 0) {
            _logger->critical(FILE, "No PIC device could be detected...");
            return false;
        }
        _active_pic = _pic_driver_table[pic_idx]->get();
        _logger->debug(FILE, R"("{}" has been initialized.)", _active_pic->get_name());

        // Init Scheduling
        _logger->debug(FILE, "Starting the Scheduler...");
        PhysicalAddr base_pt_addr = Memory::get_base_page_table_address();
        DUMMY_ARGS[0] = nullptr;
        TERMINATOR_THREAD_START_INFO.argc = 0;
        TERMINATOR_THREAD_START_INFO.argv = DUMMY_ARGS;
        TERMINATOR_THREAD_START_INFO.main = &terminator_thread;
        auto thread_terminator = create_thread(
                TERMINATOR_THREAD_NAME,
                &TERMINATOR_THREAD_START_INFO,
                base_pt_addr,
                SchedulingPolicy::NONE,
                { nullptr, 0x0, 0x0 }
        );
        IDLE_THREAD_START_INFO.argc = 0;
        IDLE_THREAD_START_INFO.argv = DUMMY_ARGS;
        IDLE_THREAD_START_INFO.main = &idle_thread;
        auto le_idle_thread    = create_thread(
                IDLE_THREAD_NAME,
                &IDLE_THREAD_START_INFO,
                base_pt_addr,
                SchedulingPolicy::NONE,
                { nullptr, 0x0, 0x0 }
        );
        if (!_scheduler.init(
                Memory::get_base_page_table_address(),
                boot_info.stack,
                le_idle_thread,
                thread_terminator,
                &thread_enter
        )) {
            _logger->critical(FILE, "Failed to start the SCHEDULER!");
            return false;
        }
        SCHEDULER          = &_scheduler;
        SCHED_LOGGY        = _logger;
        NOTIFY_THREAD_BOOM = [this](Thread* term, Thread* next) {
            ThreadTerminatedContext tt_ctx = { move(term), move(next) };
            fire(EventHook(EventHook::THREAD_TERMINATED).to_string(), (void*) &tt_ctx);
        };
        _scheduler.set_on_context_switch(
                [this](Thread* next) {
                    fire(EventHook(EventHook::CONTEXT_SWITCH).to_string(), (void*) next);
                }
        );
        _scheduler.get_running_thread()->handle = _thread_handle_counter.acquire_handle();
        thread_terminator->handle               = _thread_handle_counter.acquire_handle();
        le_idle_thread->handle                  = _thread_handle_counter.acquire_handle();
        _thread_table.put(_scheduler.get_running_thread()->handle, _scheduler.get_running_thread());
        _thread_table.put(thread_terminator->handle, thread_terminator);
        _thread_table.put(le_idle_thread->handle, le_idle_thread);

        // Init Timer
        _logger->debug(FILE, "Starting the timer...");
        if (!_timer) {
            _logger->critical(FILE, "No timer driver installed!");
            return false;
        }
        U64 timer_freq = 1000;
        U32 quantum    = 50000000;    // Each thread can run for a maximum of 50ms at a time
        if (!_timer->start(_logger, &_scheduler, TimerMode::PERIODIC, timer_freq, quantum)) {
            _logger->critical(FILE, "Could not start the timer!");
            return false;
        }


        _logger->debug(FILE, "Detecting other CPU cores...");
        if (!init_other_cores()) {
            _logger->critical(FILE, "Failed to detect other CPU cores!");
            return false;
        }
        return true;
    }


    void CPUSubsystem::set_logger(SharedPointer<Logger> logger) {
        if (!_logger) {
            _logger = logger;
            _scheduler.set_logger(logger);
        }
    }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                          Interrupt functions
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    PICDriver* CPUSubsystem::get_active_pic() {
        return _active_pic;
    }


    LinkedList<PICDriver*> CPUSubsystem::get_pic_driver_table() {
        LinkedList<PICDriver*> dt;
        for (auto& d: _pic_driver_table)
            dt.add_back(d.get());
        return dt;
    }


    bool CPUSubsystem::install_pic_driver(UniquePointer<PICDriver> driver) {
        if (!driver)
            return false;
        _pic_driver_table.add_back(move(driver));
        return true;
    }


    bool CPUSubsystem::install_irq_handler(U8 irq_line, U16 dev_id, const String& dev_name, const IRQHandler& handler) {
        return irq_install_handler(irq_line, dev_id, dev_name, handler);
    }


    bool CPUSubsystem::uninstall_irq_handler(U8 irq_line, U16 dev_handle) {
        return irq_uninstall_handler(irq_line, dev_handle);
    }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                      High Level Threading API
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    Scheduler* CPUSubsystem::get_scheduler() {
        return &_scheduler;
    }


    LinkedList<Thread*> CPUSubsystem::get_thread_table() {
        LinkedList<Thread*> copy;
        for (auto& t: _thread_table)
            copy.add_back(t.value->get());
        return copy;
    }


    void CPUSubsystem::dump_thread_table(const SharedPointer<TextStream>& stream) const {
        auto it = _thread_table.begin();
        _thread_table_fmt.dump(
                stream, [&it] {
                    Thread* t = nullptr;
                    if (it.has_next()) {
                        t = it->value->get();
                        ++it;
                    }
                    return t;
                }
        );
    }


    Thread* CPUSubsystem::find_thread(int handle) {
        Thread   * le_thread = nullptr;
        for (auto& t: _thread_table) {
            if (t.value->get()->handle == handle) {
                le_thread = t.value->get();
                break;
            }
        }
        return le_thread;
    }


    U16 CPUSubsystem::schedule_new_thread(
            const String& thread_name,
            StartInfo* start_info,
            PhysicalAddr base_pt_addr,
            SchedulingPolicy policy,
            Stack user_stack
    ) {
        if (!_thread_handle_counter.has_more_handles())
            return 0;

        SharedPointer<Thread> new_thread = create_thread(
                thread_name,
                move(start_info),
                base_pt_addr,
                policy,
                move(user_stack)
        );
        _scheduler.lock();
        if (!_scheduler.schedule_new_thread(new_thread)) {
            return 0;
        }

        new_thread->handle = _thread_handle_counter.acquire_handle();
        _thread_table.put(new_thread->handle, new_thread);
        _scheduler.unlock();
        return new_thread->handle;
    }


    bool CPUSubsystem::terminate_thread(int handle) {
        // Check if a thread with the ID exists
        SharedPointer<Thread> da_thread(nullptr);
        for (auto& t: _thread_table) {
            if (t.value->get()->handle == handle) {
                da_thread = *t.value;
                break;
            }
        }
        if (!da_thread) {
            _logger->warn(FILE, "No thread with handle {} exists", handle);
            return false;
        }

        // Check where the thread currently is e.g. locked by a mutex and remove it from the queue
        _logger->trace(FILE, R"(Terminating thread "{}-{}")", da_thread->handle, da_thread->name);
        switch (da_thread->state) {
            case ThreadState::NONE:
                _logger->error(
                        FILE,
                        R"("{}-{}" has invalid state "None".)",
                        da_thread->handle,
                        da_thread->name
                );
                return false;
            case ThreadState::READY:
                if (!_scheduler.get_ready_queue()->remove(handle)) {
                    _logger->error(
                            FILE,
                            R"("{}-{}" is missing from the ready queue.)",
                            da_thread->handle,
                            da_thread->name
                    );
                    return false;
                }
                break;
            case ThreadState::RUNNING:
                // Do not terminate the running thread because we do not want a context switch to happen
                _logger->trace(FILE, R"("{}-{}" is running, will not terminate.)", da_thread->handle, da_thread->name);
                return true;    // Early return, so we can just terminate the thread after the switch
            case ThreadState::SLEEPING:
                if (!_timer->remove_sleeping_thread(handle)) {
                    _logger->error(
                            FILE,
                            R"("{}-{}" is missing from the wait queue of the timer.)",
                            da_thread->handle,
                            da_thread->name
                    );
                    return false;
                }
                break;
            case ThreadState::WAITING: {
                if (da_thread->mutex_id < 0) {
                    _logger->error(
                            FILE,
                            R"("{}-{}" has not mutex ID assigned.)",
                            da_thread->handle,
                            da_thread->name
                    );
                    return false;
                }

                SharedPointer<Mutex> m(nullptr);
                for (auto& mm: _mutex_table) {
                    if (mm.value->get()->handle == da_thread->mutex_id) {
                        m = *mm.value;
                        break;
                    }
                }
                if (!m) {
                    _logger->error(
                            FILE,
                            "No mutex with ID {} was found.",
                            da_thread->handle,
                            da_thread->name,
                            da_thread->mutex_id
                    );
                    return false;
                }

                if (!m->remove_waiting_thread(da_thread->handle)) {
                    _logger->error(
                            FILE,
                            R"("{}-{}" was not the owner or in the waiting queue of "{}-{}")",
                            da_thread->handle,
                            da_thread->name,
                            m->handle,
                            m->name
                    );
                    return false;
                }
                break;
            }
            case ThreadState::TERMINATED:
                _logger->trace(
                        FILE,
                        R"("{}-{}" is already terminated.)",
                        da_thread->handle,
                        da_thread->name
                );
                break;
        }


        _scheduler.lock();
        _scheduler.terminate(da_thread);
        _scheduler.unlock();
        return true;
    }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                          Mutex API
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    LinkedList<Mutex*> CPUSubsystem::get_mutex_table() {
        LinkedList<Mutex*> copy;
        for (auto& m: _mutex_table)
            copy.add_back(m.value->get());
        return copy;
    }


    SharedPointer<Mutex> CPUSubsystem::find_mutex(U16 mutex_handle) {
        auto it = _mutex_table.find(mutex_handle);
        return it == _mutex_table.end() ? SharedPointer<Mutex>() : *it->value;
    }


    void CPUSubsystem::dump_mutex_table(const SharedPointer<TextStream>& stream) const {
        auto it = _mutex_table.begin();
        _mutex_table_fmt.dump(
                stream, [&it] {
                    Mutex* m = nullptr;
                    if (it.has_next()) {
                        m = it->value->get();
                        ++it;
                    }
                    return m;
                }
        );
    }


    SharedPointer<Mutex> CPUSubsystem::create_mutex(String name) {
        if (!_mutex_handle_counter.has_more_handles())
            return SharedPointer<Mutex>(nullptr);
        auto m = SharedPointer<Mutex>(new Mutex(&_scheduler, _logger, move(name)));
        m->handle = _mutex_handle_counter.acquire_handle();
        _mutex_table.put(m->handle, m);
        return m;
    }


    bool CPUSubsystem::release_mutex(U16 mutex_handle) {
        SharedPointer<Mutex> to_remove;
        for (auto& m: _mutex_table) {
            if (m.value->get()->handle == mutex_handle) {
                to_remove = *m.value;
                break;
            }
        }
        if (!to_remove)
            return false;

        _mutex_table.remove(to_remove->handle);
        return true;
    }


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Time API
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    void CPUSubsystem::install_timer_driver(UniquePointer<Timer> driver) {
        if (driver)
            _timer = move(driver);
    }


    Timer* CPUSubsystem::get_system_timer() {
        return _timer.get();
    }
}
