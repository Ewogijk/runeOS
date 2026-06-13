
#ifndef RUNEOS_CONDITIONVARIABLETEST_H
#define RUNEOS_CONDITIONVARIABLETEST_H

#include <Test/Heimdall/Heimdall.h>

#include <Ember/Ember.h>

#include <Test/UnitTest/CPU/Threading/ThreadingTestCommon.h>

#include <KRE/Memory.h>
#include <KRE/String.h>
#include <KRE/System/System.h>

#include <Memory/Paging.h>

#include <CPU/CPUModule.h>
#include <CPU/Threading/ConditionVariable.h>
#include <CPU/Threading/Mutex.h>

using namespace Rune;

constexpr U8                          SLEEP_TIME_MS = 100;
SharedPointer<CPU::ConditionVariable> CONDITION_VARIABLE;
SharedPointer<CPU::Mutex>             MUTEX;
bool                                  IS_STILL_LOCKED = false;

auto make_thread_wait(CPU::StartInfo* start_info) -> int {
    MUTEX->lock();
    CONDITION_VARIABLE->wait(*MUTEX);
    IS_STILL_LOCKED = MUTEX->get_owner() != nullptr;
    MUTEX->unlock();
    return 0;
}

auto find_waiting_thread(CPU::ThreadHandle handle) -> CPU::Thread* {
    for (auto* thread : CONDITION_VARIABLE->get_waiting_threads()) {
        if (thread->get_handle() == handle) return thread;
    }
    return nullptr;
}

TEST("wait - One Thread", "ConditionVariable") {
    // Setup
    auto* cpu_module   = System::instance().get_module<CPU::CPUModule>(ModuleSelector::CPU);
    MUTEX              = make_shared<CPU::Mutex>(Resource<CPU::MutexHandle>::HANDLE_NONE, "");
    CONDITION_VARIABLE = make_shared<CPU::ConditionVariable>();

    // Test Body
    {
        TestThread tt("CV Test Thread", &make_thread_wait, false);
        if (tt.m_thread_handle == Resource<CPU::ThreadHandle>::HANDLE_NONE) {
            REQUIRE(1 == 0) // Test Thread not started -> FAIL the TC
            return;
        }
        cpu_module->get_system_timer()->sleep_milli(SLEEP_TIME_MS);
        auto* thread_a = find_waiting_thread(tt.m_thread_handle);
        REQUIRE(reinterpret_cast<uintptr_t>(thread_a) != static_cast<uintptr_t>(0));
        REQUIRE(thread_a->state == CPU::ThreadState::BLOCKED)

        CONDITION_VARIABLE.reset(); // Clear cv waiting list so thread structs can be freed
    }

    // Cleanup
    CONDITION_VARIABLE = SharedPointer<CPU::ConditionVariable>();
    MUTEX              = SharedPointer<CPU::Mutex>();
    IS_STILL_LOCKED    = false;
}

TEST("wait - Multiple Threads", "ConditionVariable") {
    // Setup
    auto* cpu_module   = System::instance().get_module<CPU::CPUModule>(ModuleSelector::CPU);
    MUTEX              = make_shared<CPU::Mutex>(Resource<CPU::MutexHandle>::HANDLE_NONE, "");
    CONDITION_VARIABLE = make_shared<CPU::ConditionVariable>();

    // Test Body
    {
        TestThread tt_a("CV Test Thread", &make_thread_wait, false);
        TestThread tt_b("CV Test Thread", &make_thread_wait, false);
        if (tt_a.m_thread_handle == Resource<CPU::ThreadHandle>::HANDLE_NONE) {
            REQUIRE(1 == 0) // Test Thread not started -> FAIL the TC
            return;
        }
        if (tt_b.m_thread_handle == Resource<CPU::ThreadHandle>::HANDLE_NONE) {
            REQUIRE(1 == 0) // Test Thread not started -> FAIL the TC
            return;
        }
        cpu_module->get_system_timer()->sleep_milli(SLEEP_TIME_MS);

        auto* thread_a = find_waiting_thread(tt_a.m_thread_handle);
        auto* thread_b = find_waiting_thread(tt_b.m_thread_handle);
        REQUIRE(reinterpret_cast<uintptr_t>(thread_a) != static_cast<uintptr_t>(0))
        REQUIRE(thread_a->state == CPU::ThreadState::BLOCKED)
        REQUIRE(reinterpret_cast<uintptr_t>(thread_b) != static_cast<uintptr_t>(0))
        REQUIRE(thread_b->state == CPU::ThreadState::BLOCKED)

        CONDITION_VARIABLE.reset(); // Clear cv waiting list so thread structs can be freed
    }

    // Cleanup
    CONDITION_VARIABLE = SharedPointer<CPU::ConditionVariable>();
    MUTEX              = SharedPointer<CPU::Mutex>();
    IS_STILL_LOCKED    = false;
}

TEST("notify_one", "ConditionVariable") {
    // Setup
    auto* cpu_module   = System::instance().get_module<CPU::CPUModule>(ModuleSelector::CPU);
    MUTEX              = make_shared<CPU::Mutex>(Resource<CPU::MutexHandle>::HANDLE_NONE, "");
    CONDITION_VARIABLE = make_shared<CPU::ConditionVariable>();
    // Test Body
    {
        TestThread tt_a("CV Test Thread", &make_thread_wait, true);
        TestThread tt_b("CV Test Thread", &make_thread_wait, false);
        if (tt_a.m_thread_handle == Resource<CPU::ThreadHandle>::HANDLE_NONE) {
            REQUIRE(1 == 0) // Test Thread not started -> FAIL the TC
            return;
        }
        if (tt_b.m_thread_handle == Resource<CPU::ThreadHandle>::HANDLE_NONE) {
            REQUIRE(1 == 0) // Test Thread not started -> FAIL the TC
            return;
        }
        cpu_module->get_system_timer()->sleep_milli(SLEEP_TIME_MS);

        auto* thread_a = cpu_module->find_thread(tt_a.m_thread_handle).get();
        auto* thread_b = cpu_module->find_thread(tt_b.m_thread_handle).get();
        CONDITION_VARIABLE->notify_one();
        cpu_module->get_system_timer()->sleep_milli(SLEEP_TIME_MS);
        REQUIRE(reinterpret_cast<uintptr_t>(thread_a) != static_cast<uintptr_t>(0))
        REQUIRE(reinterpret_cast<uintptr_t>(find_waiting_thread(tt_a.m_thread_handle))
                == static_cast<uintptr_t>(0))
        REQUIRE(thread_a->state != CPU::ThreadState::BLOCKED)
        REQUIRE(IS_STILL_LOCKED)
        REQUIRE(reinterpret_cast<uintptr_t>(thread_b) != static_cast<uintptr_t>(0))
        REQUIRE(reinterpret_cast<uintptr_t>(find_waiting_thread(tt_b.m_thread_handle))
                != static_cast<uintptr_t>(0))
        REQUIRE(thread_b->state == CPU::ThreadState::BLOCKED)

        CONDITION_VARIABLE.reset(); // Clear cv waiting list so thread structs can be freed
    }

    // Cleanup
    CONDITION_VARIABLE = SharedPointer<CPU::ConditionVariable>();
    MUTEX              = SharedPointer<CPU::Mutex>();
    IS_STILL_LOCKED    = false;
}

TEST("notify_all", "ConditionVariable") {
    // Setup
    auto* cpu_module   = System::instance().get_module<CPU::CPUModule>(ModuleSelector::CPU);
    MUTEX              = make_shared<CPU::Mutex>(Resource<CPU::MutexHandle>::HANDLE_NONE, "");
    CONDITION_VARIABLE = make_shared<CPU::ConditionVariable>();

    // Test Body
    {
        TestThread tt_a("CV Test Thread", &make_thread_wait, true);
        TestThread tt_b("CV Test Thread", &make_thread_wait, true);
        if (tt_a.m_thread_handle == Resource<CPU::ThreadHandle>::HANDLE_NONE) {
            REQUIRE(1 == 0) // Test Thread not started -> FAIL the TC
            return;
        }
        if (tt_b.m_thread_handle == Resource<CPU::ThreadHandle>::HANDLE_NONE) {
            REQUIRE(1 == 0) // Test Thread not started -> FAIL the TC
            return;
        }
        cpu_module->get_system_timer()->sleep_milli(SLEEP_TIME_MS);

        auto* thread_a = cpu_module->find_thread(tt_a.m_thread_handle).get();
        auto* thread_b = cpu_module->find_thread(tt_b.m_thread_handle).get();
        CONDITION_VARIABLE->notify_all();
        REQUIRE(reinterpret_cast<uintptr_t>(thread_a) != static_cast<uintptr_t>(0))
        REQUIRE(reinterpret_cast<uintptr_t>(find_waiting_thread(tt_a.m_thread_handle))
                == static_cast<uintptr_t>(0))
        REQUIRE(thread_a->state != CPU::ThreadState::BLOCKED)
        REQUIRE(reinterpret_cast<uintptr_t>(thread_b) != static_cast<uintptr_t>(0))
        REQUIRE(reinterpret_cast<uintptr_t>(find_waiting_thread(tt_b.m_thread_handle))
                == static_cast<uintptr_t>(0))
        REQUIRE(thread_b->state != CPU::ThreadState::BLOCKED)
    }

    // Cleanup
    CONDITION_VARIABLE = SharedPointer<CPU::ConditionVariable>();
    MUTEX              = SharedPointer<CPU::Mutex>();
    IS_STILL_LOCKED    = false;
}

#endif // RUNEOS_CONDITIONVARIABLETEST_H
