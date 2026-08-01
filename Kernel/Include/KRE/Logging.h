
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

#ifndef RUNEOS_LOGGING_H
#define RUNEOS_LOGGING_H

#include <Ember/Ember.h>
#include <Ember/Enum.h>
#include <Ember/MachineBits.h>

#include <KRE/Collections/Array.h>
#include <KRE/Collections/HashMap.h>
#include <KRE/Collections/RingBuffer.h>

#include <KRE/Memory.h>
#include <KRE/Stream.h>
#include <KRE/String.h>
#include <KRE/Utility.h>

namespace Rune {
    // ====================================================================================== //
    // Configuration
    // ====================================================================================== //

#ifndef LOG_LEVEL
// Fallback to INFO log level
#define LOG_LEVEL LOG_LEVEL_INFO
#endif

#ifndef LOG_BUFFER_SIZE
    // Fallback to 2^16=64KiB log buffer size storing up to 512 log events.
#define LOG_BUFFER_SIZE 16
#endif

    /// @brief
    /// @return A read cursor for the kernel log ringbuffer.
    auto log_get_read_cursor() -> ReadCursor<Ember::LogEvent, LOG_BUFFER_SIZE>;

    /// @brief Returns the handle of the calling thread.
    using ThreadResolver = Ember::Handle (*)();

    /// @brief Returns the handle of the running application.
    using AppResolver = Ember::Handle (*)();

    /// @brief Configure the thread resolver.
    /// @param thread_resolver
    void log_configure_thread_resolver(ThreadResolver thread_resolver);

    /// @brief Configure the app resolver.
    /// @param app_resolver
    void log_configure_app_resolver(AppResolver app_resolver);

    // ====================================================================================== //
    // Behind the scenes logging functions - Use Macros
    // ====================================================================================== //

    /// @brief Log an event within the kernel.
    /// @param log_level
    /// @param file
    /// @param line_number
    /// @param log_message
    /// @param args
    /// @param arg_size
    ///
    /// This is a low-level call to the logging API, use one of the macros instead.
    void log(Ember::LogLevel log_level,
             const char*     file,
             U16             line_number,
             const char*     log_message,
             const Argument* args,
             size_t          arg_size);

    /// @brief Log a trace log message.
    /// @tparam Args
    /// @param file
    /// @param line_number
    /// @param log_message
    /// @param args
    ///
    /// This is a low-level call to the logging API, use one of the macros instead.
    template <typename... Args>
    void trace(const char* file, U16 line_number, const char* log_message, Args... args) {
        Argument arg_array[] = {args...}; // NOLINT
        log(Ember::LogLevel::TRACE, file, line_number, log_message, arg_array, sizeof...(Args));
    }

    /// @brief Log a debug log message.
    /// @tparam Args
    /// @param file
    /// @param line_number
    /// @param log_message
    /// @param args
    ///
    /// This is a low-level call to the logging API, use one of the macros instead.
    template <typename... Args>
    void debug(const char* file, U16 line_number, const char* log_message, Args... args) {
        Argument arg_array[] = {args...}; // NOLINT
        log(Ember::LogLevel::DEBUG, file, line_number, log_message, arg_array, sizeof...(Args));
    }

    /// @brief Log an info log message.
    /// @tparam Args
    /// @param file
    /// @param line_number
    /// @param log_message
    /// @param args
    ///
    /// This is a low-level call to the logging API, use one of the macros instead.
    template <typename... Args>
    void info(const char* file, U16 line_number, const char* log_message, Args... args) {
        Argument arg_array[] = {args...}; // NOLINT
        log(Ember::LogLevel::INFO, file, line_number, log_message, arg_array, sizeof...(Args));
    }

    /// @brief Log a warn log message.
    /// @tparam Args
    /// @param file
    /// @param line_number
    /// @param log_message
    /// @param args
    ///
    /// This is a low-level call to the logging API, use one of the macros instead.
    template <typename... Args>
    void warn(const char* file, U16 line_number, const char* log_message, Args... args) {
        Argument arg_array[] = {args...}; // NOLINT
        log(Ember::LogLevel::WARN, file, line_number, log_message, arg_array, sizeof...(Args));
    }

    /// @brief Log a error log message.
    /// @tparam Args
    /// @param file
    /// @param line_number
    /// @param log_message
    /// @param args
    ///
    /// This is a low-level call to the logging API, use one of the macros instead.
    template <typename... Args>
    void error(const char* file, U16 line_number, const char* log_message, Args... args) {
        Argument arg_array[] = {args...}; // NOLINT
        log(Ember::LogLevel::ERROR, file, line_number, log_message, arg_array, sizeof...(Args));
    }

    /// @brief Log a fatal log message.
    /// @tparam Args
    /// @param file
    /// @param line_number
    /// @param log_message
    /// @param args
    ///
    /// This is a low-level call to the logging API, use one of the macros instead.
    template <typename... Args>
    void fatal(const char* file, U16 line_number, const char* log_message, Args... args) {
        Argument arg_array[] = {args...}; // NOLINT
        log(Ember::LogLevel::FATAL, file, line_number, log_message, arg_array, sizeof...(Args));
    }

    // ====================================================================================== //
    // Macros
    // ====================================================================================== //

#if LOG_LEVEL <= LOG_LEVEL_TRACE
#define TRACE(log_message, ...) trace(__FILE__, __LINE__, log_message __VA_OPT__(, ) __VA_ARGS__);
#else
#define TRACE(log_message, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_DEBUG
#define DEBUG(log_message, ...) debug(__FILE__, __LINE__, log_message __VA_OPT__(, ) __VA_ARGS__);
#else
#define DEBUG(log_message, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_INFO
#define INFO(log_message, ...) info(__FILE__, __LINE__, log_message __VA_OPT__(, ) __VA_ARGS__);
#else
#define INFO(log_message, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_WARN
#define WARN(log_message, ...) warn(__FILE__, __LINE__, log_message __VA_OPT__(, ) __VA_ARGS__);
#else
#define WARN(log_message, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_ERROR
#define ERROR(log_message, ...) error(__FILE__, __LINE__, log_message __VA_OPT__(, ) __VA_ARGS__);
#else
#define ERROR(log_message, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_FATAL
#define FATAL(log_message, ...) fatal(__FILE__, __LINE__, log_message __VA_OPT__(, ) __VA_ARGS__);
#else
#define FATAL(log_message, ...)
#endif

} // namespace Rune
#endif // RUNEOS_LOGGING_H
