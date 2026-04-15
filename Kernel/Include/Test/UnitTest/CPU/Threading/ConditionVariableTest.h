
#ifndef RUNEOS_CONDITIONVARIABLETEST_H
#define RUNEOS_CONDITIONVARIABLETEST_H

#include <Test/Heimdall/Heimdall.h>

#include <Ember/Ember.h>

#include <KRE/Memory.h>
#include <KRE/String.h>
#include <KRE/System/System.h>

#include <Memory/Paging.h>

#include <CPU/CPUModule.h>
#include <CPU/Threading/ConditionVariable.h>

using namespace Rune;

struct TestThread {
    CPU::ThreadHandle             m_thread_handle = 0;
    SharedPointer<CPU::StartInfo> m_start_info;
};

constexpr U8                          SLEEP_TIME_MS = 50;
TestThread                            test_thread_a;
TestThread                            test_thread_b;
SharedPointer<CPU::ConditionVariable> condition_variable;

auto make_thread_wait(CPU::StartInfo* start_info) -> int {
    condition_variable->wait();
    return 0;
}

auto exec_test_thread(TestThread& tt) -> bool {
    auto* cpu_module     = System::instance().get_module<CPU::CPUModule>(ModuleSelector::CPU);
    char* dummy_args[1]  = {nullptr}; // NOLINT
    auto  start_info     = make_shared<CPU::StartInfo>();
    start_info->argc     = 0;
    start_info->argv     = dummy_args;
    start_info->main     = &make_thread_wait;
    CPU::ThreadHandle th = cpu_module->schedule_new_thread(
        "CV Test Thread",
        start_info.get(),
        Memory::get_base_page_table_address(),
        CPU::SchedulingPolicy::LOW_LATENCY,
        {.stack_bottom = nullptr, .stack_top = 0x0, .stack_size = 0x0});
    if (th == Resource<CPU::ThreadHandle>::HANDLE_NONE) return false;

    tt.m_thread_handle = th;
    tt.m_start_info    = start_info;
    return true;
}

auto find_waiting_thread(CPU::ThreadHandle handle) -> CPU::Thread* {
    for (auto* thread : condition_variable->get_waiting_threads()) {
        if (thread->get_handle() == handle) return thread;
    }
    return nullptr;
}

TEST("wait - One Thread", "ConditionVariable") {
    // Setup
    test_thread_a      = {};
    test_thread_b      = {};
    auto* cpu_module   = System::instance().get_module<CPU::CPUModule>(ModuleSelector::CPU);
    condition_variable = make_shared<CPU::ConditionVariable>(cpu_module->get_scheduler());

    // Test Body
    if (!exec_test_thread(test_thread_a)) {
        REQUIRE(1 == 0) // Test Thread not started -> FAIL the TC
        return;
    }
    cpu_module->get_system_timer()->sleep_milli(SLEEP_TIME_MS);
    auto* thread_a = find_waiting_thread(test_thread_a.m_thread_handle);
    REQUIRE(reinterpret_cast<uintptr_t>(thread_a) != static_cast<uintptr_t>(0));
    REQUIRE(thread_a->state == CPU::ThreadState::BLOCKED)

    // Cleanup
    cpu_module->stop_thread(test_thread_a.m_thread_handle);
    condition_variable = SharedPointer<CPU::ConditionVariable>();
}

TEST("wait - Multiple Threads", "ConditionVariable") {
    // Setup
    test_thread_a      = {};
    test_thread_b      = {};
    auto* cpu_module   = System::instance().get_module<CPU::CPUModule>(ModuleSelector::CPU);
    condition_variable = make_shared<CPU::ConditionVariable>(cpu_module->get_scheduler());

    // Test Body
    if (!exec_test_thread(test_thread_a)) {
        REQUIRE(1 == 0) // Test Thread not started -> FAIL the TC
        return;
    }
    if (!exec_test_thread(test_thread_b)) {
        REQUIRE(1 == 0) // Test Thread not started -> FAIL the TC
        return;
    }
    cpu_module->get_system_timer()->sleep_milli(SLEEP_TIME_MS);

    auto* thread_a = find_waiting_thread(test_thread_a.m_thread_handle);
    auto* thread_b = find_waiting_thread(test_thread_b.m_thread_handle);
    REQUIRE(reinterpret_cast<uintptr_t>(thread_a) != static_cast<uintptr_t>(0))
    REQUIRE(thread_a->state == CPU::ThreadState::BLOCKED)
    REQUIRE(reinterpret_cast<uintptr_t>(thread_b) != static_cast<uintptr_t>(0))
    REQUIRE(thread_b->state == CPU::ThreadState::BLOCKED)

    // Cleanup
    cpu_module->stop_thread(test_thread_a.m_thread_handle);
    cpu_module->stop_thread(test_thread_b.m_thread_handle);
    condition_variable = SharedPointer<CPU::ConditionVariable>();
}

TEST("notify_one", "ConditionVariable") {
    // Setup
    test_thread_a      = {};
    test_thread_b      = {};
    auto* cpu_module   = System::instance().get_module<CPU::CPUModule>(ModuleSelector::CPU);
    condition_variable = make_shared<CPU::ConditionVariable>(cpu_module->get_scheduler());

    // Test Body
    if (!exec_test_thread(test_thread_a)) {
        REQUIRE(1 == 0) // Test Thread not started -> FAIL the TC
        return;
    }
    if (!exec_test_thread(test_thread_b)) {
        REQUIRE(1 == 0) // Test Thread not started -> FAIL the TC
        return;
    }
    cpu_module->get_system_timer()->sleep_milli(SLEEP_TIME_MS);

    auto* thread_a = cpu_module->find_thread(test_thread_a.m_thread_handle);
    auto* thread_b = cpu_module->find_thread(test_thread_b.m_thread_handle);
    condition_variable->notify_one();
    REQUIRE(reinterpret_cast<uintptr_t>(thread_a) != static_cast<uintptr_t>(0))
    REQUIRE(reinterpret_cast<uintptr_t>(find_waiting_thread(test_thread_a.m_thread_handle))
            == static_cast<uintptr_t>(0))
    REQUIRE(thread_a->state != CPU::ThreadState::BLOCKED)
    REQUIRE(reinterpret_cast<uintptr_t>(thread_b) != static_cast<uintptr_t>(0))
    REQUIRE(reinterpret_cast<uintptr_t>(find_waiting_thread(test_thread_b.m_thread_handle))
            != static_cast<uintptr_t>(0))
    REQUIRE(thread_b->state == CPU::ThreadState::BLOCKED)

    // Cleanup
    cpu_module->stop_thread(test_thread_a.m_thread_handle);
    cpu_module->stop_thread(test_thread_b.m_thread_handle);
    condition_variable = SharedPointer<CPU::ConditionVariable>();
}

TEST("notify_all", "ConditionVariable") {
    // Setup
    test_thread_a      = {};
    test_thread_b      = {};
    auto* cpu_module   = System::instance().get_module<CPU::CPUModule>(ModuleSelector::CPU);
    condition_variable = make_shared<CPU::ConditionVariable>(cpu_module->get_scheduler());

    // Test Body
    if (!exec_test_thread(test_thread_a)) {
        REQUIRE(1 == 0) // Test Thread not started -> FAIL the TC
        return;
    }
    if (!exec_test_thread(test_thread_b)) {
        REQUIRE(1 == 0) // Test Thread not started -> FAIL the TC
        return;
    }
    cpu_module->get_system_timer()->sleep_milli(SLEEP_TIME_MS);

    auto* thread_a = cpu_module->find_thread(test_thread_a.m_thread_handle);
    auto* thread_b = cpu_module->find_thread(test_thread_b.m_thread_handle);
    condition_variable->notify_all();
    REQUIRE(reinterpret_cast<uintptr_t>(thread_a) != static_cast<uintptr_t>(0))
    REQUIRE(reinterpret_cast<uintptr_t>(find_waiting_thread(test_thread_a.m_thread_handle))
            == static_cast<uintptr_t>(0))
    REQUIRE(thread_a->state != CPU::ThreadState::BLOCKED)
    REQUIRE(reinterpret_cast<uintptr_t>(thread_b) != static_cast<uintptr_t>(0))
    REQUIRE(reinterpret_cast<uintptr_t>(find_waiting_thread(test_thread_b.m_thread_handle))
            == static_cast<uintptr_t>(0))
    REQUIRE(thread_b->state != CPU::ThreadState::BLOCKED)

    // Cleanup
    cpu_module->stop_thread(test_thread_a.m_thread_handle);
    cpu_module->stop_thread(test_thread_b.m_thread_handle);
    condition_variable = SharedPointer<CPU::ConditionVariable>();
}

#endif // RUNEOS_CONDITIONVARIABLETEST_H
