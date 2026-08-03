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

#include <Boot/QEMUConsoleLogger.h>

#include <Ember/MachineBits.h>

#include <KRE/Logging.h>
#include <KRE/System/System.h>

#include <CPU/CPUModule.h>
#include <CPU/E9Stream.h>
#include <CPU/Job.h>

#include <App/AppModule.h>

namespace Rune {
    /// @brief Background color for FATAL log messages.
    constexpr Pixel BG_COLOR_CRITICAL = Pixie::VSCODE_RED;

    /// @brief Log level based text color decoding for log messages.
    constexpr Array<Pixel, 6> FG_COLOR = {
        Pixie::VSCODE_CYAN,   // TRACE
        Pixie::VSCODE_BLUE,   // DEBUG
        Pixie::VSCODE_WHITE,  // INFO
        Pixie::VSCODE_YELLOW, // WARN
        Pixie::VSCODE_RED,    // ERROR
        Pixie::VSCODE_WHITE   // FATAL (Red background color)
    };

    /// @brief Wait time before the kernel log buffer is polled again.
    constexpr U8 WAIT_TIME_MILLIS = 1;

    /// @brief Background thread name.
    const char* THREAD_NAME = "QEMUCon";

    /// @brief Start info of the background thread.
    ThreadStartupPacket g_start_info;

    ///@brief E9 target stream.
    CPU::E9Stream g_qemu_console_stream;

    auto layout_message(const Ember::LogEvent& evt) -> String {
        auto*  app_module = System::instance().get_module<App::AppModule>(ModuleSelector::APP);
        String file       = evt.m_file_name;
        String app_name;
        if (evt.m_app_handle > Ember::HANDLE_NONE) {
            for (auto* app_info : app_module->get_app_table()) {
                if (evt.m_app_handle == app_info->handle) {
                    app_name = app_info->name;
                }
            }
        }
        if (app_name.is_empty()) app_name = int_to_string(evt.m_app_handle, Radix::DECIMAL);

        String thread_name;
        if (evt.m_thread_handle > Ember::HANDLE_NONE) {
            auto thread = CPU::g_thread_cache.find(evt.m_thread_handle);
            if (thread) thread_name = thread->get_name();
        }
        if (thread_name.is_empty())
            thread_name += int_to_string(evt.m_thread_handle, Radix::DECIMAL);

        auto formatted_log_msg = String::format("{} | {}:{} | {}:{} | {}",
                                                Ember::LogLevel(evt.m_log_level).to_string(),
                                                file,
                                                evt.m_line_number,
                                                app_name,
                                                thread_name,
                                                evt.m_message);
        return formatted_log_msg;
    }

    void sent_to_e9_stream(Ember::LogLevel log_level, const String& formatted_log_msg) {
        if (g_qemu_console_stream.is_ansi_supported()) {
            // Only set the background color to red when a critical message is logged
            // and keep the default background color of the stream for other log levels
            // Setting the background color in all cases looks strange on other terminals
            // e.g. Clion, powershell, etc.
            if (log_level == Ember::LogLevel::FATAL)
                g_qemu_console_stream.set_background_color(BG_COLOR_CRITICAL);
            g_qemu_console_stream.set_foreground_color(FG_COLOR[log_level - 1]);
        }
        g_qemu_console_stream.write_line(formatted_log_msg);
        if (g_qemu_console_stream.is_ansi_supported()) g_qemu_console_stream.reset_style();
        g_qemu_console_stream.flush();
    }

    auto redirect_logs_to_e9(ThreadStartupPacket* si) -> int {
        auto  read_cursor = log_get_read_cursor();
        auto* timer =
            System::instance().get_module<CPU::CPUModule>(ModuleSelector::CPU)->get_system_timer();

        while (true) {
            auto maybe_evt = read_cursor.next();
            while (maybe_evt) {
                auto evt = maybe_evt.value();
                sent_to_e9_stream(Ember::LogLevel(evt.m_log_level), layout_message(evt));
                maybe_evt = read_cursor.next();
            }

            timer->sleep_milli(WAIT_TIME_MILLIS);
        }
        return 0;
    }

    void qemu_consoler_logger_start() {
        auto* cpu_module    = System::instance().get_module<CPU::CPUModule>(ModuleSelector::CPU);
        char* dummy_args[1] = {nullptr}; // NOLINT
        g_start_info.argc   = 0;
        g_start_info.argv   = dummy_args;
        g_start_info.main   = &redirect_logs_to_e9;
        cpu_module->schedule_new_thread(
            THREAD_NAME,
            &g_start_info,
            Memory::get_base_page_table_address(),
            CPU::SchedulingPolicy::LOW_LATENCY,
            {.stack_bottom = nullptr, .stack_top = 0x0, .stack_size = 0x0});
    }
} // namespace Rune