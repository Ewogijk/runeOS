
//  Copyright 2025 Ewogijk
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#ifndef RUNEOS_THREADPOOLTEST_H
#define RUNEOS_THREADPOOLTEST_H

#include <Test/Heimdall/Heimdall.h>

#include <CPU/Threading/ThreadPool.h>

#include <KRE/System/System.h>

#include <CPU/CPUModule.h>

using namespace Rune;

const String     THREAD_POOL_NAME("Test ThreadPool");
constexpr size_t NUM_THREADS = 2;

TEST("Destruction", "ThreadPool") {
    // Setup
    LinkedList<CPU::ThreadHandle> worker_threads;

    // Test body
    {
        // Force destruction of thread pool, then check that threads have been stopped
        CPU::ThreadPool<NUM_THREADS> thread_pool(THREAD_POOL_NAME);
        thread_pool.start();
        worker_threads = thread_pool.worker_threads();
    }
    auto* cpu_module = System::instance().get_module<CPU::CPUModule>(ModuleSelector::CPU);
    for (const auto thread_handle : worker_threads) {
        auto thread = cpu_module->find_thread(thread_handle);
        REQUIRE(thread.get() == nullptr);
    }
}

TEST("start", "ThreadPool") {
    // Setup
    CPU::ThreadPool<NUM_THREADS> thread_pool(THREAD_POOL_NAME);
    thread_pool.start();

    // Test body
    auto* cpu_module = System::instance().get_module<CPU::CPUModule>(ModuleSelector::CPU);
    for (const auto thread_handle : thread_pool.worker_threads()) {
        auto thread = cpu_module->find_thread(thread_handle);
        REQUIRE(thread.get() != nullptr);
    }
}

TEST("submit", "ThreadPool") {
    // Setup
    CPU::ThreadPool<NUM_THREADS> thread_pool(THREAD_POOL_NAME);
    thread_pool.start();
    constexpr U8 SLEEP_TIME_MS  = 250;
    int          counter        = 0;
    int          counter_target = 2 * NUM_THREADS;

    // Test body
    for (int i = 0; i < counter_target; i++) {
        thread_pool.submit([&counter]() -> void { counter++; });
    }
    // Wait for the tasks to finish
    System::instance()
        .get_module<CPU::CPUModule>(ModuleSelector::CPU)
        ->get_system_timer()
        ->sleep_milli(SLEEP_TIME_MS);
    REQUIRE(counter == counter_target);
}

#endif // RUNEOS_THREADPOOLTEST_H
