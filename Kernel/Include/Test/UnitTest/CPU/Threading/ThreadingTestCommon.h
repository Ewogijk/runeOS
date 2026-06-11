
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

#ifndef RUNEOS_THREADINGTESTCOMMON_H
#define RUNEOS_THREADINGTESTCOMMON_H

#include <Memory/Paging.h>

#include <CPU/CPUModule.h>
#include <CPU/Threading/Thread.h>

using namespace Rune;

/// @brief RAII style
struct TestThread {
    const SharedPointer<Logger>   TTLOGGER        = LogContext::instance().get_logger("CPU.TTC");
    CPU::ThreadHandle             m_thread_handle = 0;
    SharedPointer<CPU::StartInfo> m_start_info;
    bool                          m_sync_thread_stop;

    TestThread(const String& name, CPU::ThreadMain thread_main, bool sync_thread_stop)
        : m_sync_thread_stop(sync_thread_stop) {
        auto* cpu_module    = System::instance().get_module<CPU::CPUModule>(ModuleSelector::CPU);
        m_start_info        = make_shared<CPU::StartInfo>();
        char* dummy_args[1] = {nullptr}; // NOLINT
        m_start_info->argc  = 0;
        m_start_info->argv  = dummy_args;
        m_start_info->main  = thread_main;
        m_thread_handle     = cpu_module->schedule_new_thread(
            name,
            m_start_info.get(),
            Memory::get_base_page_table_address(),
            CPU::SchedulingPolicy::LOW_LATENCY,
            {.stack_bottom = nullptr, .stack_top = 0x0, .stack_size = 0x0});
    }

    ~TestThread() {
        auto* cpu_module = System::instance().get_module<CPU::CPUModule>(ModuleSelector::CPU);
        if (m_thread_handle != Resource<CPU::ThreadHandle>::HANDLE_NONE) {
            cpu_module->stop_thread(m_thread_handle);
            if (m_sync_thread_stop) cpu_module->sync_with_thread_stop(m_thread_handle);
        }
    }
};

#endif // RUNEOS_THREADINGTESTCOMMON_H
