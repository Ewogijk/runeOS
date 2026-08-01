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

#include <KRE/Logging.h>

#include "CPU/Threading/Scheduler.h"

#include <KRE/Collections/RingBuffer.h>

namespace Rune {
    // ====================================================================================== //
    // Log Buffer
    // ====================================================================================== //

    RingBuffer<Ember::LogEvent, LOG_BUFFER_SIZE> g_kernel_log_buffer;

    auto log_get_read_cursor() -> ReadCursor<Ember::LogEvent, LOG_BUFFER_SIZE> {
        return g_kernel_log_buffer.create_read_cursor();
    }

    // ====================================================================================== //
    // Logging
    // ====================================================================================== //

    auto resolve_to_none_handle() -> Ember::Handle { return Ember::HANDLE_NONE; }

    ThreadResolver                       g_thread_resolver = &resolve_to_none_handle;
    AppResolver                          g_app_resolver    = &resolve_to_none_handle;
    Array<char, Ember::LOG_MESSAGE_SIZE> g_formatted_log_message_buf;

    void log_configure_thread_resolver(ThreadResolver thread_resolver) {
        g_thread_resolver = move(thread_resolver);
    }

    void log_configure_app_resolver(AppResolver app_resolver) {
        g_app_resolver = move(app_resolver);
    }

    void log(Ember::LogLevel log_level,
             const char*     file,
             U16             line_number,
             const char*     log_message,
             const Argument* args,
             size_t          arg_size) {
        // __FILE__ includes the relative path to the file: a/b/code.cpp
        // We want to strip the path and file extension: code
        size_t offset          = 0;
        size_t file_name_begin = 0;
        size_t file_name_end   = 0;
        while (file[offset] != 0) {
            char c = file[offset];
            if (c == '/') file_name_begin = offset + 1;
            if (c == '.') {
                file_name_end = offset;
                break;
            }
            offset++;
        }

        Ember::LogEvent evt{.m_log_level     = log_level,
                            .m_file_name     = {},
                            .m_line_number   = line_number,
                            .m_app_handle    = g_app_resolver(),
                            .m_thread_handle = g_thread_resolver(),
                            .m_message       = {}};

        // NOLINTBEGIN
        memcpy(evt.m_file_name,
               const_cast<char*>(&file[file_name_begin]),
               file_name_end - file_name_begin);
        // NOLINTEND

        size_t i = interpolate(log_message,
                               g_formatted_log_message_buf.data(),
                               Ember::LOG_MESSAGE_SIZE,
                               args,
                               arg_size);
        memcpy(evt.m_message, g_formatted_log_message_buf.data(), i);

        g_kernel_log_buffer.append(move(evt));
    }
} // namespace Rune
