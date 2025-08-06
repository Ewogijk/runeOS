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


#include <Ember/Enum.h>

#include <KernelRuntime/String.h>
#include <KernelRuntime/Memory.h>
#include <KernelRuntime/Stream.h>


namespace Rune {
    /**
     * The severity of a log message.
     * <p>
     * Priorities: Trace < Debug < Info < Warn < Error < Critical.
     * </p>
     */
#define LOG_LEVELS(X)           \
    X(LogLevel, TRACE, 0x1)     \
    X(LogLevel, DEBUG, 0x2)     \
    X(LogLevel, INFO, 0x3)      \
    X(LogLevel, WARN, 0x4)      \
    X(LogLevel, ERROR, 0x5)     \
    X(LogLevel, CRITICAL, 0x6)  \



    DECLARE_ENUM(LogLevel, LOG_LEVELS, 0x0) //NOLINT


    /**
     * A formatter for log messages.
     */
    class LogFormatter {
    public:

        virtual ~LogFormatter() = default;


        /**
         * Format a log message.
         *
         * @param log_level
         * @param module       The source file that has logged the message.
         * @param log_msg_tmpl Log message with placeholders for formatting.
         * @param arg_list     List of formatting arguments.
         * @param arg_size     Size of the argument list.
         *
         * @return Formatted log message
         */
        virtual String format_log_message(
                LogLevel log_level,
                const String& module,
                const String& log_msg_tmpl,
                Argument* arg_list,
                size_t arg_size
        ) = 0;
    };


    /**
     * Simple logging class for the kernel.
     */
    class Logger {
    protected:
        SharedPointer<LogFormatter> _log_msg_fmt;
        LogLevel                    _log_level;

    public:

        /**
         * Create a new logger that will log messages for it's configured log level.
         *
         * @param log_msg_fmt Log message formatter.
         * @param log_level  Log level.
         */
        explicit Logger(
                SharedPointer<LogFormatter> log_msg_fmt,
                LogLevel log_level
        );


        virtual ~Logger() = default;


        /**
         *
         * @return Active log formatter.
         */
        [[nodiscard]] SharedPointer<LogFormatter> get_formatter() const;


        /**
         *
         * @return The logger's log level.
         */
        [[nodiscard]] LogLevel get_log_level() const;


        /**
         * @brief
         * @param log_msg_fmt
         */
        void set_log_formatter(SharedPointer<LogFormatter> log_msg_fmt);


        /**
         * Log a message.
         *
         * <p>
         *  This function is intended for functions that need to pass arguments as an array. For general purpose
         *  logging use the other log functions (e.g. Trace, ...).
         * </p>
         *
         * @param log_level Log level of the message.
         * @param module    Name of the module that logged the message.
         * @param fmt       The message as a format string.
         * @param arg_list  Arguments for the format string.
         * @param arg_size  Number of arguments.
         */
        virtual void log(
                LogLevel log_level,
                const String& module,
                const String& fmt,
                Argument* arg_list,
                size_t arg_size
        ) = 0;


        /**
         * Log a message.
         *
         * @param log_level The log level of the message.
         * @param module   Name of the module that logged the message.
         * @param fmt      The message as a format string.
         * @param args     Arguments for the format string.
         *
         */
        template<typename... Args>
        void log(LogLevel log_level, const String& module, const String& fmt, Args... args) {
            Argument arg_array[] = { args... };
            log(log_level, module, fmt, arg_array, sizeof...(Args));
        }


        /**
         * Log a trace message.
         *
         * @param module Name of the module that logged the message.
         * @param fmt    The message as a format string.
         * @param args   Arguments for the format string.
         */
        template<typename... Args>
        void trace(const String& module, const String& fmt, Args... args) {
            Argument arg_array[] = { args... };
            log(LogLevel::TRACE, module, fmt, arg_array, sizeof...(Args));
        }


        /**
         * Log a debug message.
         *
         * @param module Name of the module that logged the message.
         * @param fmt    The message as a format string.
         * @param args   Arguments for the format string.
         */
        template<typename... Args>
        void debug(const String& module, const String& fmt, Args... args) {
            Argument arg_array[] = { args... };
            log(LogLevel::DEBUG, module, fmt, arg_array, sizeof...(Args));
        }


        /**
         * Log an info message.
         *
         * @param module Name of the module that logged the message.
         * @param fmt    The message as a format string.
         * @param args   Arguments for the format string.
         */
        template<typename... Args>
        void info(const String& module, const String& fmt, Args... args) {
            Argument arg_array[] = { args... };
            log(LogLevel::INFO, module, fmt, arg_array, sizeof...(Args));
        }


        /**
         * Log a warn message.
         *
         * @param module Name of the module that logged the message.
         * @param fmt    The message as a format string.
         * @param args   Arguments for the format string.
         */
        template<typename... Args>
        void warn(const String& module, const String& fmt, Args... args) {
            Argument arg_array[] = { args... };
            log(LogLevel::WARN, module, fmt, arg_array, sizeof...(Args));
        }


        /**
         * Log an error message.
         *
         * @param module Name of the module that logged the message.
         * @param fmt    The message as a format string.
         * @param args   Arguments for the format string.
         */
        template<typename... Args>
        void error(const String& module, const String& fmt, Args... args) {
            Argument arg_array[] = { args... };
            log(LogLevel::ERROR, module, fmt, arg_array, sizeof...(Args));
        }


        /**
         * Log a critical message.
         *
         * @param module Name of the module that logged the message.
         * @param fmt    The message as a format string.
         * @param args  Arguments for the format string.
         *
         */
        template<typename... Args>
        void critical(const String& module, const String& fmt, Args... args) {
            Argument arg_array[] = { args... };
            log(LogLevel::CRITICAL, module, fmt, arg_array, sizeof...(Args));
        }
    };


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Simple Log Formatter
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    class SimpleLogFormatter : public LogFormatter {
    public:
        String format_log_message(
                LogLevel log_level,
                const String& module,
                const String& log_msg_tmpl,
                Argument* arg_list,
                size_t arg_size
        ) override;
    };


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Text Stream Logger
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    class TextStreamLogger : public Logger {
        static constexpr Pixel BG_COLOR_CRITICAL = Pixie::VSCODE_RED;
        static constexpr Pixel FG_COLOR[6] = {
                Pixie::VSCODE_CYAN,     // Trace
                Pixie::VSCODE_BLUE,     // Debug
                Pixie::VSCODE_WHITE,    // Info
                Pixie::VSCODE_YELLOW,   // Warn
                Pixie::VSCODE_RED,      // Error
                Pixie::VSCODE_WHITE     // Critical (Red background color)
        };

        UniquePointer<TextStream> _txt_stream;
    public:
        TextStreamLogger(
                SharedPointer<LogFormatter> log_msg_fmt,
                LogLevel log_level,
                UniquePointer<TextStream> txt_stream
        );


        void log(
                LogLevel log_level,
                const String& module,
                const String& fmt,
                Argument* arg_list,
                size_t arg_size
        ) override;
    };


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          System Logger
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    /**
     * A cached log message with preformatted text since storing the template arguments is a pain in the ass.
     */
    struct CachedLogMessage {
        LogLevel log_level          = LogLevel::NONE;
        String   file               = "";
        String   pre_formatted_text = "";
    };


    /**
     * Main kernel logger that logs either to both serial port and files in dev builds or to files in a non dev build.
     */
    class SystemLogger : public Logger {
        const String _log_file;

        /**
         * Messages will be cached until serial and file logging are available.
         */
        LinkedList<CachedLogMessage> _log_cache;

        SharedPointer<Logger> _serial_logger;
        UniquePointer<Logger> _file_logger;

    public:
        SystemLogger(
                SharedPointer<LogFormatter> log_msg_fmt,
                LogLevel log_level,
                const String& log_file
        );


        void log(
                LogLevel log_level,
                const String& module,
                const String& fmt,
                Argument* arg_list,
                size_t arg_size
        ) override;


        /**
         * @brief Update the log formatters of the serial and file logger.
         * @param log_msg_fmt
         */
        void update_log_formatter(const SharedPointer<LogFormatter>& log_msg_fmt);


        /**
         *
         * @return Path to the log file.
         */
        [[nodiscard]] String get_log_file() const;


        /**
         *
         * @param serial_logger A logger logging to some serial stream.
         */
        void set_serial_logger(SharedPointer<Logger> serial_logger);


        /**
         *
         * @param file_logger A logger logging to a file.
         */
        void set_file_logger(UniquePointer<Logger> file_logger);


        /**
         * Flush the cached log messages.
         *
         * @param flush_file True: Flush to the log file, False: Flush to the serial connection.
         */
        void flush(bool flush_file);
    };
}

#endif //RUNEOS_LOGGING_H
