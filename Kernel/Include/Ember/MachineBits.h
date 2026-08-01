
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

#ifndef RUNEOS_SYSTEMBITS_H
#define RUNEOS_SYSTEMBITS_H

#include <Ember/Ember.h>

namespace Ember {
    // ========================================================================================== //
    // Logging
    // ========================================================================================== //

#define LOG_LEVEL_TRACE 0x1
#define LOG_LEVEL_DEBUG 0x2
#define LOG_LEVEL_INFO  0x3
#define LOG_LEVEL_WARN  0x4
#define LOG_LEVEL_ERROR 0x5
#define LOG_LEVEL_FATAL 0x6

#define LOG_LEVELS(X)                                                                              \
    X(LogLevel, TRACE, LOG_LEVEL_TRACE)                                                            \
    X(LogLevel, DEBUG, LOG_LEVEL_DEBUG)                                                            \
    X(LogLevel, INFO, LOG_LEVEL_INFO)                                                              \
    X(LogLevel, WARN, LOG_LEVEL_WARN)                                                              \
    X(LogLevel, ERROR, LOG_LEVEL_ERROR)                                                            \
    X(LogLevel, FATAL, LOG_LEVEL_FATAL)

    DECLARE_TYPED_ENUM(LogLevel, U8, LOG_LEVELS, 0x0) // NOLINT

    // ====================================================================================== //
    // Log Event
    // ====================================================================================== //

    /// @brief Byte size of a log file name.
    constexpr size_t LOG_FILE_NAME_SIZE = 20;

    /// @brief ASCII encoded name of the file name that logged a log event.
    using LogFileName = char[LOG_FILE_NAME_SIZE]; // NOLINT modernize-avoid-c-arrays: Is ABI

    /// @brief The maximum size of a log message.
    constexpr size_t LOG_MESSAGE_SIZE = 101;

    /// @brief The log message of a log event.
    using LogMessage = char[LOG_MESSAGE_SIZE]; // NOLINT modernize-avoid-c-arrays: Is ABI

    /// @brief A log event represents a notable operation executed within the kernel.
    struct LogEvent {
        /// @brief Log level of the event.
        U8 m_log_level{LogLevel::NONE};

        /// @brief Name of the source file that created the event.
        LogFileName m_file_name{};

        /// @brief Line in the source file.
        U16 m_line_number{0};

        ///@brief Handle of the running application at the time of the event.
        Handle m_app_handle{HANDLE_NONE};

        ///@brief Handle of the executing thread at the time of the event.
        Handle m_thread_handle{HANDLE_NONE};

        /// @brief User defined log message.
        LogMessage m_message{};
    } PACKED;
} // namespace Ember

#endif // RUNEOS_SYSTEMBITS_H
