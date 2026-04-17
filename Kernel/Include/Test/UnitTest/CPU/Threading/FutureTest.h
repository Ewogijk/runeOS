
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

#ifndef RUNEOS_FUTURETEST_H
#define RUNEOS_FUTURETEST_H

#include <CPU/CPUModule.h>
#include <CPU/Threading/Future.h>
#include <KRE/System/System.h>
#include <Memory/Paging.h>
#include <Test/Heimdall/Heimdall.h>

using namespace Rune;

constexpr U8      RESULT           = 42;
constexpr U8      WAIT_TIME_MILLIS = 50;
CPU::Promise<U8>* PROMISE;

auto set_value_async(CPU::StartInfo* start_info) -> int {
    auto* cpu_module = System::instance().get_module<CPU::CPUModule>(ModuleSelector::CPU);
    cpu_module->get_system_timer()->sleep_milli(WAIT_TIME_MILLIS);
    PROMISE->set_value(RESULT);
    return 0;
}

auto run_set_value_async_job() -> bool {
    auto* cpu_module     = System::instance().get_module<CPU::CPUModule>(ModuleSelector::CPU);
    char* dummy_args[1]  = {nullptr}; // NOLINT
    auto  start_info     = make_shared<CPU::StartInfo>();
    start_info->argc     = 0;
    start_info->argv     = dummy_args;
    start_info->main     = &set_value_async;
    CPU::ThreadHandle th = cpu_module->schedule_new_thread(
        "FutureTest",
        start_info.get(),
        Memory::get_base_page_table_address(),
        CPU::SchedulingPolicy::LOW_LATENCY,
        {.stack_bottom = nullptr, .stack_top = 0x0, .stack_size = 0x0});
    return th != Resource<CPU::ThreadHandle>::HANDLE_NONE;
}

TEST("is_finished - Is unfinished", "Future") {
    // Setup
    auto* cpu_module = System::instance().get_module<CPU::CPUModule>(ModuleSelector::CPU);
    auto  promise    = CPU::Promise<U8>(cpu_module->get_scheduler());
    PROMISE          = &promise;

    // Test Body
    if (!run_set_value_async_job()) {
        REQUIRE(1 == 0) // Job wasn't run -> FAIL the TC
        return;
    }
    REQUIRE(!promise.get_future().is_finished());

    // Cleanup
    PROMISE = nullptr;
}

TEST("is_finished - Is finished", "Future") {
    // Setup
    auto* cpu_module = System::instance().get_module<CPU::CPUModule>(ModuleSelector::CPU);
    auto  promise    = CPU::Promise<U8>(cpu_module->get_scheduler());
    PROMISE          = &promise;

    // Test Body
    if (!run_set_value_async_job()) {
        REQUIRE(1 == 0) // Job wasn't run -> FAIL the TC
        return;
    }
    cpu_module->get_system_timer()->sleep_milli(static_cast<U64>(2)*WAIT_TIME_MILLIS);
    REQUIRE(promise.get_future().is_finished());

    // Cleanup
    PROMISE = nullptr;
}

TEST("get - Is unfinished", "Future") {
    // Setup
    auto* cpu_module = System::instance().get_module<CPU::CPUModule>(ModuleSelector::CPU);
    auto  promise    = CPU::Promise<U8>(cpu_module->get_scheduler());
    PROMISE          = &promise;

    // Test Body
    if (!run_set_value_async_job()) {
        REQUIRE(1 == 0) // Job wasn't run -> FAIL the TC
        return;
    }
    REQUIRE(promise.get_future().get() == RESULT);

    // Cleanup
    PROMISE = nullptr;
}

TEST("get - Is finished", "Future") {
    // Setup
    auto* cpu_module = System::instance().get_module<CPU::CPUModule>(ModuleSelector::CPU);
    auto  promise    = CPU::Promise<U8>(cpu_module->get_scheduler());
    PROMISE          = &promise;

    // Test Body
    if (!run_set_value_async_job()) {
        REQUIRE(1 == 0) // Job wasn't run -> FAIL the TC
        return;
    }
    cpu_module->get_system_timer()->sleep_milli(static_cast<U64>(2)*WAIT_TIME_MILLIS);
    REQUIRE(promise.get_future().get() == RESULT);

    // Cleanup
    PROMISE = nullptr;
}

TEST("Promise.set_value", "Future") {
    // Setup
    auto* cpu_module = System::instance().get_module<CPU::CPUModule>(ModuleSelector::CPU);
    auto  promise    = CPU::Promise<U8>(cpu_module->get_scheduler());
    PROMISE          = &promise;

    // Test Body
    REQUIRE(!promise.get_future().is_finished());
    promise.set_value(RESULT);
    REQUIRE(promise.get_future().is_finished());
    REQUIRE(promise.get_future().get() == RESULT);

    // Cleanup
    PROMISE = nullptr;
}

#endif // RUNEOS_FUTURETEST_H
