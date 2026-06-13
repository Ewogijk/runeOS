
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

#include <Test/Heimdall/Heimdall.h>

#include <CPU/Threading/Future.h>

#include <Test/UnitTest/CPU/Threading/ThreadingTestCommon.h>

#include <KRE/System/System.h>

#include <CPU/CPUModule.h>

#include <Memory/Paging.h>

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

TEST("is_finished - Is unfinished", "Future") {
    // Setup
    auto promise = CPU::Promise<U8>();
    PROMISE      = &promise;
    auto future  = promise.get_future();

    // Test Body
    {
        TestThread tt("FutureTest", set_value_async, true);
        if (tt.m_thread_handle == Resource<CPU::ThreadHandle>::HANDLE_NONE) {
            REQUIRE(1 == 0) // Job wasn't run -> FAIL the TC
            return;
        }
        REQUIRE(!future.is_finished());
    }

    // Cleanup
    PROMISE = nullptr;
}

TEST("is_finished - Is finished", "Future") {
    // Setup
    auto* cpu_module = System::instance().get_module<CPU::CPUModule>(ModuleSelector::CPU);
    auto  promise    = CPU::Promise<U8>();
    PROMISE          = &promise;
    auto future      = promise.get_future();

    // Test Body
    {
        TestThread tt("FutureTest", set_value_async, true);
        if (tt.m_thread_handle == Resource<CPU::ThreadHandle>::HANDLE_NONE) {
            REQUIRE(1 == 0) // Job wasn't run -> FAIL the TC
            return;
        }
        cpu_module->get_system_timer()->sleep_milli(static_cast<U64>(2) * WAIT_TIME_MILLIS);
        REQUIRE(future.is_finished());
    }

    // Cleanup
    PROMISE = nullptr;
}

TEST("get - Is unfinished", "Future") {
    // Setup
    auto promise = CPU::Promise<U8>();
    PROMISE      = &promise;
    auto future  = promise.get_future();

    // Test Body
    {
        TestThread tt("FutureTest", set_value_async, true);
        if (tt.m_thread_handle == Resource<CPU::ThreadHandle>::HANDLE_NONE) {
            REQUIRE(1 == 0) // Job wasn't run -> FAIL the TC
            return;
        }
        REQUIRE(future.get() == RESULT);
    }

    // Cleanup
    PROMISE = nullptr;
}

TEST("get - Is finished", "Future") {
    // Setup
    auto* cpu_module = System::instance().get_module<CPU::CPUModule>(ModuleSelector::CPU);
    auto  promise    = CPU::Promise<U8>();
    PROMISE          = &promise;
    auto future      = promise.get_future();

    // Test Body
    {
        TestThread tt("FutureTest", set_value_async, true);
        if (tt.m_thread_handle == Resource<CPU::ThreadHandle>::HANDLE_NONE) {
            REQUIRE(1 == 0) // Job wasn't run -> FAIL the TC
            return;
        }
        cpu_module->get_system_timer()->sleep_milli(static_cast<U64>(2) * WAIT_TIME_MILLIS);
        REQUIRE(future.get() == RESULT);
    }

    // Cleanup
    PROMISE = nullptr;
}

TEST("Promise.set_value", "Future") {
    // Setup
    auto promise = CPU::Promise<U8>();
    PROMISE      = &promise;
    auto future  = promise.get_future();

    // Test Body
    REQUIRE(!future.is_finished());
    promise.set_value(RESULT);
    REQUIRE(future.is_finished());
    REQUIRE(future.get() == RESULT);

    // Cleanup
    PROMISE = nullptr;
}

#endif // RUNEOS_FUTURETEST_H
