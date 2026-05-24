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

#ifndef RUNEOS_THREADPOOL_H
#define RUNEOS_THREADPOOL_H

#include <KRE/Collections/Array.h>
#include <KRE/Collections/LinkedList.h>

#include <KRE/System/System.h>

#include <Memory/Paging.h>

#include <CPU/CPUModule.h>
#include <CPU/Threading/ConditionVariable.h>
#include <CPU/Threading/Mutex.h>

namespace Rune::CPU {

    /// @brief
    using Task = Function<void()>;

    struct WorkerThread {
        StartInfo    m_start_info;
        ThreadHandle m_handle;

        friend auto operator==(const WorkerThread& a, const WorkerThread& b) -> bool {
            return a.m_handle == b.m_handle;
        }

        friend auto operator!=(const WorkerThread& a, const WorkerThread& b) -> bool {
            return a.m_handle != b.m_handle;
        }
    };

    /// @brief
    template <size_t N>
    class ThreadPool {
        ConditionVariable      m_cv;
        Array<WorkerThread, N> m_threads;
        String                 m_this_addr;
        char* m_worker_thread_argv[2]; // NOLINT cppcoreguidelines-avoid-c-arrays: Is Kernel ABI

        Mutex                        m_task_queue_mutex;
        LinkedList<Function<void()>> m_task_queue;

        bool m_running{true};

        /// @brief Thread pool name for debugging purposes.
        String m_name;

        static auto exec_worker_thread(StartInfo* start_info) -> int {
            if (start_info->argc == 0) return -1;
            VirtualAddr tp_addr = 0;
            if (!parse_int(start_info->argv[0], Radix::HEX, tp_addr)) return -1;

            auto* thread_pool = memory_addr_to_pointer<ThreadPool>(tp_addr);
            while (true) {
                Task task = [] -> void {};
                {
                    CriticalSection _(thread_pool->m_task_queue_mutex);

                    thread_pool->m_cv.wait(thread_pool->m_task_queue_mutex, [&] -> bool {
                        return !thread_pool->m_task_queue.is_empty() || !thread_pool->m_running;
                    });

                    if (!thread_pool->m_running && thread_pool->m_task_queue.is_empty()) return 0;

                    task = *thread_pool->m_task_queue.head();
                    thread_pool->m_task_queue.remove_front();
                }
                task();
            }
        }

      public:
        explicit ThreadPool(const String& thread_pool_name)
            : m_cv(System::instance().get_module<CPUModule>(ModuleSelector::CPU)->get_scheduler()),
              m_this_addr(int_to_string(memory_pointer_to_addr(this), Radix::HEX)),
              m_worker_thread_argv(),
              m_task_queue_mutex(
                  Resource<MutexHandle>::HANDLE_NONE,
                  "ThreadPool Mutex",
                  System::instance().get_module<CPUModule>(ModuleSelector::CPU)->get_scheduler()),
              m_name(thread_pool_name) {

            auto* cpu_module = System::instance().get_module<CPUModule>(ModuleSelector::CPU);
            // NOLINTBEGIN
            // cppcoreguidelines-pro-type-const-cast: Needed for assignment
            m_worker_thread_argv[0] = const_cast<char*>(m_this_addr.to_cstr());
            // NOLINTEND

            for (size_t i = 0; i < N; i++) {
                m_threads[i].m_start_info.argc = 1;
                m_threads[i].m_start_info.argv = m_worker_thread_argv;
                m_threads[i].m_start_info.main = &exec_worker_thread;

                ThreadHandle handle = cpu_module->schedule_new_thread(
                    String::format("{}#{}", thread_pool_name, i),
                    &m_threads[i].m_start_info,
                    Memory::get_base_page_table_address(),
                    SchedulingPolicy::LOW_LATENCY,
                    {.stack_bottom = nullptr, .stack_top = 0x0, .stack_size = 0x0});

                if (handle != Resource<ThreadHandle>::HANDLE_NONE) m_threads[i].m_handle = handle;
            }
        }

        ~ThreadPool() {
            {
                CriticalSection _(m_task_queue_mutex);
                m_running = false;
            }
            m_cv.notify_all();

            auto* cpu_module = System::instance().get_module<CPUModule>(ModuleSelector::CPU);

            for (const auto& worker_thread : m_threads)
                cpu_module->sync_on_thread_stop(worker_thread.m_handle);
        }

        [[nodiscard]] auto get_thread_pool_name() const -> String { return m_name; }

        [[nodiscard]] auto get_worker_threads() const -> LinkedList<ThreadHandle> {
            LinkedList<ThreadHandle> out_list;
            for (const auto& worker_thread : m_threads) {
                out_list.add_back(worker_thread.m_handle);
            }
            return out_list;
        }

        void submit(Task task) {
            CriticalSection _(m_task_queue_mutex);
            m_task_queue.add_back(move(task));
            m_cv.notify_one();
        }
    };
} // namespace Rune::CPU

#endif // RUNEOS_THREADPOOL_H
