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

namespace Rune::CPU {
    const SharedPointer<Logger> LOGGER = LogContext::instance().get_logger("CPU.CPUSubsystem");
    // NOLINTBEGIN
    Scheduler*                       SCHEDULER          = nullptr;
    Function<void(Thread*, Thread*)> NOTIFY_THREAD_BOOM = [](Thread* term, Thread* next) {
        SILENCE_UNUSED(term)
        SILENCE_UNUSED(next)
    };
    // NOLINTEND

    void thread_exit(int exit_code) {
        // Use the raw pointer to avoid referencing the shared pointer which will never be cleaned
        // up because of the context switch in "unlock", so C++ never gets to call the destructor on
        // "t"
        auto* t = SCHEDULER->get_running_thread().get();
        LOGGER->trace(R"(Thread "{}-{}" has finished. Exit Code: {})",
                      t->handle,
                      t->name,
                      exit_code);

        SCHEDULER->lock();
        SCHEDULER->terminate();
        SCHEDULER->unlock();
    }

    void thread_enter() {
        SCHEDULER->unlock();
        // Use raw pointer -> See "ThreadExit" for explanation
        auto* t = SCHEDULER->get_running_thread().get();
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
            interrupt_enable();
            halt();
            interrupt_disable();
        }
        return 0;
    }

    auto terminator_thread(StartInfo* start_info) -> int {
        SILENCE_UNUSED(start_info)
        for (;;) {
            SCHEDULER->lock();
            auto*                 terminated_threads = SCHEDULER->get_terminated_threads();
            SharedPointer<Thread> cT                 = !terminated_threads->is_empty()
                                                           ? *terminated_threads->head()
                                                           : SharedPointer<Thread>(nullptr);
            terminated_threads->remove_front();
            while (cT) {
                auto dT = cT;
                cT      = !terminated_threads->is_empty() ? *terminated_threads->head()
                                                          : SharedPointer<Thread>(nullptr);
                terminated_threads->remove_front();
                LOGGER->trace(R"(Terminating thread: "{}-{}")", dT->handle, dT->name);

                auto* next = SCHEDULER->get_ready_queue()->peek();
                if (next == nullptr) next = SCHEDULER->get_idle_thread().get();
                NOTIFY_THREAD_BOOM(forward<Thread*>(dT.get()), forward<Thread*>(next));
                delete[] dT->kernel_stack_bottom;

                if (dT.get_ref_count() > 1) {
                    LOGGER->warn(
                        R"(>> Memory Leak << - "{}-{}" has {} references but expected 1. Thread struct will not be freed.)",
                        dT->handle,
                        dT->name,
                        dT.get_ref_count());
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

    char*     CPUModule::DUMMY_ARGS[]; // NOLINT Array disallowed! Is part of Kernel ABI
    StartInfo CPUModule::TERMINATOR_THREAD_START_INFO;
    StartInfo CPUModule::IDLE_THREAD_START_INFO;

    auto CPUModule::create_thread(const String&    thread_name,
                                  StartInfo*       start_info,
                                  PhysicalAddr     base_pt_addr,
                                  SchedulingPolicy policy,
                                  Stack            user_stack) -> SharedPointer<Thread> {
        SharedPointer<Thread> new_thread(new Thread);
        new_thread->name                    = move(thread_name);
        new_thread->start_info              = start_info;
        new_thread->base_page_table_address = base_pt_addr;
        new_thread->policy                  = policy;
        new_thread->user_stack              = move(user_stack);
        fire(EventHook(EventHook::THREAD_CREATED).to_string(), new_thread.get());
        return new_thread;
    }

    CPUModule::CPUModule() : _active_pic(nullptr) {}

    auto CPUModule::get_name() const -> String { return "CPU"; }

    auto CPUModule::load(const BootInfo& boot_info) -> bool {

        // Init Event Hook table
        _event_hook_table.put(EventHook(EventHook::THREAD_CREATED).to_string(),
                              LinkedList<EventHandlerTableEntry>());
        _event_hook_table.put(EventHook(EventHook::THREAD_TERMINATED).to_string(),
                              LinkedList<EventHandlerTableEntry>());
        _event_hook_table.put(EventHook(EventHook::CONTEXT_SWITCH).to_string(),
                              LinkedList<EventHandlerTableEntry>());

        install_event_handler(
            EventHook(EventHook::THREAD_TERMINATED).to_string(),
            "Thread Table Cleaner",
            [this](void* evt_ctx) {
                auto*                 ctx = reinterpret_cast<ThreadTerminatedContext*>(evt_ctx);
                SharedPointer<Thread> to_remove(nullptr);
                for (const auto& t : _thread_table) {
                    if (ctx->terminated->handle == (*t.value)->handle) {
                        to_remove = *t.value;
                        break;
                    }
                }

                if (to_remove) {
                    LOGGER->trace(R"(Removing "{}-{}" from the thread table.)",
                                  to_remove->handle,
                                  to_remove->name);
                    _thread_table.remove(to_remove->handle);
                } else {
                    LOGGER->warn(R"(Terminated thread "{}-{}" was not found in the thread table.)",
                                 ctx->terminated->handle,
                                 ctx->terminated->name);
                }
            });

        // Init Interrupts/IRQs
        LOGGER->debug("Loading interrupt vector table...");
        interrupt_load_vector_table();
        if (_pic_driver_table.is_empty()) {
            LOGGER->critical("No PIC drivers are installed...");
            return false;
        }
        LOGGER->debug("Trying to detect a PIC device...");
        int pic_idx = irq_init(get_pic_driver_table());
        if (pic_idx < 0) {
            LOGGER->critical("No PIC device could be detected...");
            return false;
        }
        _active_pic = _pic_driver_table[pic_idx]->get();
        LOGGER->debug(R"("{}" has been initialized.)", _active_pic->get_name());

        // Init Scheduling
        LOGGER->debug("Starting the Scheduler...");
        PhysicalAddr base_pt_addr         = Memory::get_base_page_table_address();
        DUMMY_ARGS[0]                     = nullptr;
        TERMINATOR_THREAD_START_INFO.argc = 0;
        TERMINATOR_THREAD_START_INFO.argv = DUMMY_ARGS;
        TERMINATOR_THREAD_START_INFO.main = &terminator_thread;
        auto thread_terminator =
            create_thread(TERMINATOR_THREAD_NAME,
                          &TERMINATOR_THREAD_START_INFO,
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
        if (!_scheduler.init(Memory::get_base_page_table_address(),
                             boot_info.stack,
                             le_idle_thread,
                             thread_terminator,
                             &thread_enter)) {
            LOGGER->critical("Failed to start the SCHEDULER!");
            return false;
        }
        SCHEDULER          = &_scheduler;
        NOTIFY_THREAD_BOOM = [this](Thread* term, Thread* next) {
            ThreadTerminatedContext tt_ctx = {.terminated     = move(term),
                                              .next_scheduled = move(next)};
            fire(EventHook(EventHook::THREAD_TERMINATED).to_string(), (void*) &tt_ctx);
        };
        _scheduler.set_on_context_switch([this](Thread* next) {
            fire(EventHook(EventHook::CONTEXT_SWITCH).to_string(), (void*) next);
        });
        _scheduler.get_running_thread()->handle = _thread_handle_counter.acquire();
        thread_terminator->handle               = _thread_handle_counter.acquire();
        le_idle_thread->handle                  = _thread_handle_counter.acquire();
        _thread_table.put(_scheduler.get_running_thread()->handle, _scheduler.get_running_thread());
        _thread_table.put(thread_terminator->handle, thread_terminator);
        _thread_table.put(le_idle_thread->handle, le_idle_thread);

        // Init Timer
        LOGGER->debug("Starting the timer...");
        if (!_timer) {
            LOGGER->critical("No timer driver installed!");
            return false;
        }
        constexpr U64 TIMER_FREQ = 1000;
        constexpr U32 QUANTUM    = 50000000; // Each thread can run for a maximum of 50ms at a time
        if (!_timer->start(&_scheduler, TimerMode::PERIODIC, TIMER_FREQ, QUANTUM)) {
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
    auto CPUModule::install_irq_handler(U8                irq_line,
                                        U16               dev_id,
                                        const String&     dev_name,
                                        const IRQHandler& handler) -> bool {
        return irq_install_handler(irq_line, dev_id, dev_name, handler);
    }

    auto CPUModule::uninstall_irq_handler(U8 irq_line, U16 dev_handle) -> bool {
        return irq_uninstall_handler(irq_line, dev_handle);
    }
    // NOLINTEND
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      High Level Threading API
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    auto CPUModule::get_scheduler() -> Scheduler* { return &_scheduler; }

    auto CPUModule::get_thread_table() -> LinkedList<Thread*> {
        LinkedList<Thread*> copy;
        for (const auto& t : _thread_table) copy.add_back(t.value->get());
        return copy;
    }

    void CPUModule::dump_thread_table(const SharedPointer<TextStream>& stream) const {
        Table<SharedPointer<Thread>, 4>::make_table(
            [](const SharedPointer<Thread>& thread) -> Array<String, 4> {
                return {String::format("{}-{}", thread->handle, thread->name),
                        thread->state.to_string(),
                        thread->policy.to_string(),
                        String::format("{}", thread->app_handle)};
            })
            .with_headers({"ID-Name", "State", "Policy", "App"})
            .with_data(_thread_table.values())
            .print(stream);
    }

    auto CPUModule::find_thread(int handle) -> Thread* {
        Thread* le_thread = nullptr;
        for (const auto& t : _thread_table) {
            if (t.value->get()->handle == handle) { // NOLINT Only end() is null
                le_thread = t.value->get();
                break;
            }
        }
        return le_thread;
    }

    auto CPUModule::schedule_new_thread(const String&    thread_name,
                                        StartInfo*       start_info,
                                        PhysicalAddr     base_pt_addr,
                                        SchedulingPolicy policy,
                                        Stack            user_stack) -> U16 {
        if (!_thread_handle_counter.has_more()) return 0;

        SharedPointer<Thread> new_thread =
            create_thread(thread_name, move(start_info), base_pt_addr, policy, move(user_stack));
        _scheduler.lock();
        if (!_scheduler.schedule_new_thread(new_thread)) {
            return 0;
        }

        new_thread->handle = _thread_handle_counter.acquire();
        _thread_table.put(new_thread->handle, new_thread);
        _scheduler.unlock();
        return new_thread->handle;
    }

    auto CPUModule::terminate_thread(int handle) -> bool {
        // Check if a thread with the ID exists
        SharedPointer<Thread> da_thread(nullptr);
        for (const auto& t : _thread_table) {
            if (t.value->get()->handle == handle) { // NOLINT Only end() is null
                da_thread = *t.value;
                break;
            }
        }
        if (!da_thread) {
            LOGGER->warn("No thread with handle {} exists", handle);
            return false;
        }

        // Check where the thread currently is e.g. locked by a mutex and remove it from the queue
        LOGGER->trace(R"(Terminating thread "{}-{}")", da_thread->handle, da_thread->name);
        switch (da_thread->state) { // NOLINT All cases are handled
            case ThreadState::NONE:
                LOGGER->error(R"("{}-{}" has invalid state "None".)",
                              da_thread->handle,
                              da_thread->name);
                return false;
            case ThreadState::READY:
                if (!_scheduler.get_ready_queue()->remove(handle)) {
                    LOGGER->error(R"("{}-{}" is missing from the ready queue.)",
                                  da_thread->handle,
                                  da_thread->name);
                    return false;
                }
                break;
            case ThreadState::RUNNING:
                // Do not terminate the running thread because we do not want a context switch to
                // happen
                LOGGER->trace(R"("{}-{}" is running, will not terminate.)",
                              da_thread->handle,
                              da_thread->name);
                return true; // Early return, so we can just terminate the thread after the switch
            case ThreadState::SLEEPING:
                if (!_timer->remove_sleeping_thread(handle)) {
                    LOGGER->error(R"("{}-{}" is missing from the wait queue of the timer.)",
                                  da_thread->handle,
                                  da_thread->name);
                    return false;
                }
                break;
            case ThreadState::WAITING: {
                if (da_thread->mutex_id < 0) {
                    LOGGER->error(R"("{}-{}" has not mutex ID assigned.)",
                                  da_thread->handle,
                                  da_thread->name);
                    return false;
                }

                SharedPointer<Mutex> m(nullptr);
                for (const auto& mm : _mutex_table) {
                    if (mm.value->get()->handle // NOLINT Only end() is null
                        == da_thread->mutex_id) {
                        m = *mm.value;
                        break;
                    }
                }
                if (!m) {
                    LOGGER->error("No mutex with ID {} was found.",
                                  da_thread->handle,
                                  da_thread->name,
                                  da_thread->mutex_id);
                    return false;
                }

                if (!m->remove_waiting_thread(da_thread->handle)) {
                    LOGGER->error(R"("{}-{}" was not the owner or in the waiting queue of "{}-{}")",
                                  da_thread->handle,
                                  da_thread->name,
                                  m->handle,
                                  m->name);
                    return false;
                }
                break;
            }
            case ThreadState::TERMINATED:
                LOGGER->trace(R"("{}-{}" is already terminated.)",
                              da_thread->handle,
                              da_thread->name);
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

    auto CPUModule::get_mutex_table() -> LinkedList<Mutex*> {
        LinkedList<Mutex*> copy;
        for (const auto& m : _mutex_table) copy.add_back(m.value->get());
        return copy;
    }

    auto CPUModule::find_mutex(U16 mutex_handle) -> SharedPointer<Mutex> {
        auto it = _mutex_table.find(mutex_handle);
        return it == _mutex_table.end() ? SharedPointer<Mutex>() : *it->value;
    }

    void CPUModule::dump_mutex_table(const SharedPointer<TextStream>& stream) const {
        Table<SharedPointer<Mutex>, 3>::make_table(
            [](const SharedPointer<Mutex>& mutex) -> Array<String, 3> {
                Thread* owner           = mutex->get_owner();
                String  waiting_threads = "";
                for (auto& t : mutex->get_waiting_threads())
                    waiting_threads += String::format("{}-{}, ", t->handle, t->name);
                if (waiting_threads.is_empty()) waiting_threads = "-";
                return {String::format("{}-{}", mutex->handle, mutex->name),
                        owner ? String::format("{}-{}", owner->handle, owner->name) : "-",
                        waiting_threads};
            })
            .with_headers({"ID-Name", "Owner", "WaitQueue"})
            .with_data(_mutex_table.values())
            .print(stream);
    }

    auto CPUModule::create_mutex(String name) -> SharedPointer<Mutex> {
        if (!_mutex_handle_counter.has_more()) return SharedPointer<Mutex>(nullptr);
        auto m    = SharedPointer<Mutex>(new Mutex(&_scheduler, move(name)));
        m->handle = _mutex_handle_counter.acquire();
        _mutex_table.put(m->handle, m);
        return m;
    }

    auto CPUModule::release_mutex(U16 mutex_handle) -> bool {
        SharedPointer<Mutex> to_remove;
        for (const auto& m : _mutex_table) {
            if (m.value->get()->handle == mutex_handle) { // NOLINT Only end() is null
                to_remove = *m.value;
                break;
            }
        }
        if (!to_remove) return false;

        _mutex_table.remove(to_remove->handle);
        return true;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Time API
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    void CPUModule::install_timer_driver(UniquePointer<Timer> driver) {
        if (driver) _timer = move(driver);
    }

    auto CPUModule::get_system_timer() -> Timer* { return _timer.get(); }
} // namespace Rune::CPU
