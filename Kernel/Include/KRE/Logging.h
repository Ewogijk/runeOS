
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

#include <KRE/Collections/Array.h>
#include <KRE/Collections/HashMap.h>

#include <KRE/Memory.h>
#include <KRE/Stream.h>
#include <KRE/String.h>

namespace Rune {
    /**
     * The severity of a log message.
     * <p>
     * Priorities: Trace < Debug < Info < Warn < Error < Critical.
     * </p>
     */
#define LOG_LEVELS(X)                                                                              \
    X(LogLevel, TRACE, 0x1)                                                                        \
    X(LogLevel, DEBUG, 0x2)                                                                        \
    X(LogLevel, INFO, 0x3)                                                                         \
    X(LogLevel, WARN, 0x4)                                                                         \
    X(LogLevel, ERROR, 0x5)                                                                        \
    X(LogLevel, CRITICAL, 0x6)

    DECLARE_ENUM(LogLevel, LOG_LEVELS, 0x0) // NOLINT

    /**
     * A log event tracks information about a log message.
     */
    struct LogEvent {
        LogLevel  log_level;
        String    logger_name;      // Name of the logger that created the event.
        String    log_msg_template; // Template string to be formatted.
        Argument* arg_list;         // Variadic arguments to substitute in the template string.
        size_t    arg_size;         // Number of variadic arguments.
    };

    /**
     * A layout formats a logging message.
     */
    class Layout {
      public:
        virtual ~Layout() = default;

        /**
         * Format the log message of the log event.
         * @param @log_event Log event.
         * @return A formatted log message.
         */
        virtual auto layout(const LogEvent& log_event) -> String = 0;
    };

    /**
     *
     */
    class EarlyBootLayout : public Layout {
      public:
        auto layout(const LogEvent& log_event) -> String override;
    };

    /**
     * The LogConfig stores all registered layouts and targets and is the central processing point
     * of all log messages.
     *
     * <p>
     *  Layouts and targets have an unique name that loggers can references to define their message
     *  layout and the targets they want to deliver their messages to.
     * </p>
     */
    class LogConfig {
        static constexpr Pixel           BG_COLOR_CRITICAL = Pixie::VSCODE_RED;
        static constexpr Array<Pixel, 6> FG_COLOR          = {
            Pixie::VSCODE_CYAN,   // Trace
            Pixie::VSCODE_BLUE,   // Debug
            Pixie::VSCODE_WHITE,  // Info
            Pixie::VSCODE_YELLOW, // Warn
            Pixie::VSCODE_RED,    // Error
            Pixie::VSCODE_WHITE   // Critical (Red background color)
        };

        HashMap<String, SharedPointer<Layout>>     _layouts;
        HashMap<String, SharedPointer<TextStream>> _target_streams;

      public:
        LogConfig() = default;

        /**
         * Register a new layout under the given name.
         * @param name   Unique name of the layout.
         * @param layout Layout instance.
         * @return True: The layout got registered, False: A layout with the name already exists.
         */
        auto register_layout(const String& name, SharedPointer<Layout> layout) -> bool;

        /**
         * Register a new target under the given name.
         * @param name   Unique name of the target.
         * @param layout Target instance.
         * @return True: The target got registered, False: A target with the name already exists.
         */
        auto register_target_stream(const String& name, SharedPointer<TextStream> target) -> bool;

        /**
         * Try to format the log event with the requested layout and then deliver it to the given
         * list of targets.
         *
         * <p>
         *  If the requested layout is not found the log event is not delivered to any targets and
         *  if a target is not found it will be skipped.
         * </p>
         * @param log_event Log event.
         * @param layout_ref Layout that should format the log event.
         * @param target_refs A list of targets where the formatted log message should be delivered.
         */
        void log(const LogEvent&           log_event,
                 const String&             layout_ref,
                 const LinkedList<String>& target_refs);
    };

    /**
     * A logger creates
     */
    class Logger {
        LogConfig*         _config;
        String             _name;
        LogLevel           _log_level;
        String             _layout_ref;
        LinkedList<String> _target_refs;

        void log(LogLevel log_level, const String& fmt, Argument* arg_list, size_t arg_size) {
            if ((int) log_level < (int) _log_level) return;

            LogEvent log_event = {.log_level        = log_level,
                                  .logger_name      = _name,
                                  .log_msg_template = fmt,
                                  .arg_list         = arg_list,
                                  .arg_size         = arg_size};
            _config->log(log_event, _layout_ref, _target_refs);
        }

      public:
        Logger(LogConfig*                config,
               const String&             name,
               LogLevel                  level,
               const String&             layout_ref,
               const LinkedList<String>& target_refs)
            : _config(config),
              _name(move(name)),
              _log_level(level),
              _layout_ref(move(layout_ref)),
              _target_refs(move(target_refs)) {}

        /**
         *
         * @return The name of the logger.
         */
        [[nodiscard]] auto get_name() const -> String;

        /**
         *
         * @return The log level of the logger.
         */
        [[nodiscard]] auto get_log_level() const -> LogLevel;

        /**
         * Change the log level of the logger.
         * @param log_level New log level.
         */
        void set_log_level(LogLevel log_level);

        /**
         * Change the layout ref of the logger.
         * @param layout_ref New layout ref.
         */
        void set_layout_ref(const String& layout_ref);

        /**
         * Log a trace message.
         *
         * @param fmt    The message as a format string.
         * @param args   Arguments for the format string.
         */
        template <typename... Args>
        void trace(const String& fmt, Args... args) {
            Argument arg_array[] = {args...}; // NOLINT
            log(LogLevel::TRACE, fmt, arg_array, sizeof...(Args));
        }

        /**
         * Log a debug message.
         *
         * @param fmt    The message as a format string.
         * @param args   Arguments for the format string.
         */
        template <typename... Args>
        void debug(const String& fmt, Args... args) {
            Argument arg_array[] = {args...}; // NOLINT
            log(LogLevel::DEBUG, fmt, arg_array, sizeof...(Args));
        }

        /**
         * Log an info message.
         *
         * @param fmt    The message as a format string.
         * @param args   Arguments for the format string.
         */
        template <typename... Args>
        void info(const String& fmt, Args... args) {
            Argument arg_array[] = {args...}; // NOLINT
            log(LogLevel::INFO, fmt, arg_array, sizeof...(Args));
        }

        /**
         * Log a warn message.
         *
         * @param fmt    The message as a format string.
         * @param args   Arguments for the format string.
         */
        template <typename... Args>
        void warn(const String& fmt, Args... args) {
            Argument arg_array[] = {args...}; // NOLINT
            log(LogLevel::WARN, fmt, arg_array, sizeof...(Args));
        }

        /**
         * Log an error message.
         *
         * @param fmt    The message as a format string.
         * @param args   Arguments for the format string.
         */
        template <typename... Args>
        void error(const String& fmt, Args... args) {
            Argument arg_array[] = {args...}; // NOLINT
            log(LogLevel::ERROR, fmt, arg_array, sizeof...(Args));
        }

        /**
         * Log a critical message.
         *
         * @param fmt    The message as a format string.
         * @param args  Arguments for the format string.
         *
         */
        template <typename... Args>
        void critical(const String& fmt, Args... args) {
            Argument arg_array[] = {args...}; // NOLINT
            log(LogLevel::CRITICAL, fmt, arg_array, sizeof...(Args));
        }
    };

    /**
     * The log context is the main entry point to the logging API. It allows registration of
     * layouts and targets, and handles creation and configuration of logger instances.
     */
    class LogContext {
        LogConfig                              _config;
        HashMap<String, SharedPointer<Logger>> _loggers;

        static LogContext INSTANCE;

        LogContext()  = default;
        ~LogContext() = default;

        /**
         * Grammar:
         * <ul>
         *  <li>Input      = Star
         *                      | Identifier
         *                      | Identifier, ".", Star
         *                      | Identifier, ".", Identifier</li>
         *  <li>Star       = "*"</li>
         *  <li>Identifier = [a-zA-Z0-9]*</li>
         * </ul>
         */
        struct Selector {
            String the_namespace;
            String name;
        };

        static auto is_identifier(const String& str) -> bool;

        static auto parse_selector(const String& selector) -> Optional<Selector>;

      public:
        LogContext(const LogContext&)                    = delete;
        LogContext(LogContext&&)                         = delete;
        auto operator=(const LogContext&) -> LogContext& = delete;
        auto operator=(LogContext&&) -> LogContext&      = delete;

        /**
         *
         * @return An instance of the log context.
         */
        static auto instance() -> LogContext& { return INSTANCE; }

        /**
         * Create a new logger instance with the requested configuration.
         *
         * <p>
         *  Logger names are grouped by namespaces and follow the form NAMESPACE.NAME. All loggers
         *  are part of the implicit root namespace if no namespace is explicitly defined. Names
         *  must be unique in a namespace.
         *  The dot selector * can be used to address all loggers in a namespace e.g. NAMESPACE.*.
         *  Using * alone will address all loggers.
         * </p>
         * @param name Unique logger name.
         * @param level Log level of the logger.
         * @param layout_ref Reference to the log message layout.
         * @param target_refs References to the logger targets.
         * @return A pointer to the logger instance or a null pointer if a logger with the name
         *          already exists.
         */
        auto get_logger(const String&             name,
                        LogLevel                  level,
                        const String&             layout_ref,
                        const LinkedList<String>& target_refs) -> SharedPointer<Logger>;

        /**
         * Change the log level of a single logger or a selection of loggers.
         *
         * @param selector Name of a logger or a selection of loggers.
         * @param level New log level.
         * @return True: The log level of at least one logger is changed. False: No logger(s) with
         *          requested name was found.
         */
        auto set_log_level(const String& selector, LogLevel level) -> bool;

        /**
         * Change the layout ref of a single logger or a selection of loggers.
         *
         * @param selector Name of a logger or a selection of loggers.
         * @param layout_ref New layout ref.
         * @return True: The layout ref of at least one logger is changed. False: No logger(s) with
         *          requested name was found.
         */
        auto set_layout_ref(const String& selector, const String& layout_ref) -> bool;

        /**
         * Register a new layout under the given name.
         * @param name   Unique name of the layout.
         * @param layout Layout instance.
         * @return True: The layout got registered, False: A layout with the name already exists.
         */
        auto register_layout(const String& name, SharedPointer<Layout> layout) -> bool;

        /**
         * Register a new target under the given name.
         * @param name   Unique name of the target.
         * @param layout Target instance.
         * @return True: The target got registered, False: A target with the name already exists.
         */
        auto register_target_stream(const String& name, SharedPointer<TextStream> target) -> bool;
    };

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
        virtual String format_log_message(LogLevel      log_level,
                                          const String& module,
                                          const String& log_msg_tmpl,
                                          Argument*     arg_list,
                                          size_t        arg_size) = 0;
    };

    /**
     * Simple logging class for the kernel.
     */
    class LegacyLogger {
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
        explicit LegacyLogger(SharedPointer<LogFormatter> log_msg_fmt, LogLevel log_level);

        virtual ~LegacyLogger() = default;

        /**
         *
         * @return Active log formatter.
         */
        [[nodiscard]]
        SharedPointer<LogFormatter> get_formatter() const;

        /**
         *
         * @return The logger's log level.
         */
        [[nodiscard]]
        LogLevel get_log_level() const;

        /**
         * @brief
         * @param log_msg_fmt
         */
        void set_log_formatter(SharedPointer<LogFormatter> log_msg_fmt);

        /**
         * Log a message.
         *
         * <p>
         *  This function is intended for functions that need to pass arguments as an array. For
         * general purpose logging use the other log functions (e.g. Trace, ...).
         * </p>
         *
         * @param log_level Log level of the message.
         * @param module    Name of the module that logged the message.
         * @param fmt       The message as a format string.
         * @param arg_list  Arguments for the format string.
         * @param arg_size  Number of arguments.
         */
        virtual void log(LogLevel      log_level,
                         const String& module,
                         const String& fmt,
                         Argument*     arg_list,
                         size_t        arg_size) = 0;

        /**
         * Log a message.
         *
         * @param log_level The log level of the message.
         * @param module   Name of the module that logged the message.
         * @param fmt      The message as a format string.
         * @param args     Arguments for the format string.
         *
         */
        template <typename... Args>
        void log(LogLevel log_level, const String& module, const String& fmt, Args... args) {
            Argument arg_array[] = {args...};
            log(log_level, module, fmt, arg_array, sizeof...(Args));
        }

        /**
         * Log a trace message.
         *
         * @param module Name of the module that logged the message.
         * @param fmt    The message as a format string.
         * @param args   Arguments for the format string.
         */
        template <typename... Args>
        void trace(const String& module, const String& fmt, Args... args) {
            Argument arg_array[] = {args...};
            log(LogLevel::TRACE, module, fmt, arg_array, sizeof...(Args));
        }

        /**
         * Log a debug message.
         *
         * @param module Name of the module that logged the message.
         * @param fmt    The message as a format string.
         * @param args   Arguments for the format string.
         */
        template <typename... Args>
        void debug(const String& module, const String& fmt, Args... args) {
            Argument arg_array[] = {args...};
            log(LogLevel::DEBUG, module, fmt, arg_array, sizeof...(Args));
        }

        /**
         * Log an info message.
         *
         * @param module Name of the module that logged the message.
         * @param fmt    The message as a format string.
         * @param args   Arguments for the format string.
         */
        template <typename... Args>
        void info(const String& module, const String& fmt, Args... args) {
            Argument arg_array[] = {args...};
            log(LogLevel::INFO, module, fmt, arg_array, sizeof...(Args));
        }

        /**
         * Log a warn message.
         *
         * @param module Name of the module that logged the message.
         * @param fmt    The message as a format string.
         * @param args   Arguments for the format string.
         */
        template <typename... Args>
        void warn(const String& module, const String& fmt, Args... args) {
            Argument arg_array[] = {args...};
            log(LogLevel::WARN, module, fmt, arg_array, sizeof...(Args));
        }

        /**
         * Log an error message.
         *
         * @param module Name of the module that logged the message.
         * @param fmt    The message as a format string.
         * @param args   Arguments for the format string.
         */
        template <typename... Args>
        void error(const String& module, const String& fmt, Args... args) {
            Argument arg_array[] = {args...};
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
        template <typename... Args>
        void critical(const String& module, const String& fmt, Args... args) {
            Argument arg_array[] = {args...};
            log(LogLevel::CRITICAL, module, fmt, arg_array, sizeof...(Args));
        }
    };

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      Simple Log Formatter
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    class SimpleLogFormatter : public LogFormatter {
      public:
        String format_log_message(LogLevel      log_level,
                                  const String& module,
                                  const String& log_msg_tmpl,
                                  Argument*     arg_list,
                                  size_t        arg_size) override;
    };

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      Text Stream Logger
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    class TextStreamLogger : public LegacyLogger {
        static constexpr Pixel BG_COLOR_CRITICAL = Pixie::VSCODE_RED;
        static constexpr Pixel FG_COLOR[6]       = {
            Pixie::VSCODE_CYAN,   // Trace
            Pixie::VSCODE_BLUE,   // Debug
            Pixie::VSCODE_WHITE,  // Info
            Pixie::VSCODE_YELLOW, // Warn
            Pixie::VSCODE_RED,    // Error
            Pixie::VSCODE_WHITE   // Critical (Red background color)
        };

        UniquePointer<TextStream> _txt_stream;

      public:
        TextStreamLogger(SharedPointer<LogFormatter> log_msg_fmt,
                         LogLevel                    log_level,
                         UniquePointer<TextStream>   txt_stream);

        void log(LogLevel      log_level,
                 const String& module,
                 const String& fmt,
                 Argument*     arg_list,
                 size_t        arg_size) override;
    };

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      System Logger
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * A cached log message with preformatted text since storing the template arguments is a pain in
     * the ass.
     */
    struct CachedLogMessage {
        LogLevel log_level          = LogLevel::NONE;
        String   file               = "";
        String   pre_formatted_text = "";
    };

    /**
     * Main kernel logger that logs either to both serial port and files in dev builds or to files
     * in a non dev build.
     */
    class SystemLogger : public LegacyLogger {
        const String _log_file;

        /**
         * Messages will be cached until serial and file logging are available.
         */
        LinkedList<CachedLogMessage> _log_cache;

        SharedPointer<LegacyLogger> _serial_logger;
        UniquePointer<LegacyLogger> _file_logger;

      public:
        SystemLogger(SharedPointer<LogFormatter> log_msg_fmt,
                     LogLevel                    log_level,
                     const String&               log_file);

        void log(LogLevel      log_level,
                 const String& module,
                 const String& fmt,
                 Argument*     arg_list,
                 size_t        arg_size) override;

        /**
         * @brief Update the log formatters of the serial and file logger.
         * @param log_msg_fmt
         */
        void update_log_formatter(const SharedPointer<LogFormatter>& log_msg_fmt);

        /**
         *
         * @return Path to the log file.
         */
        [[nodiscard]]
        auto get_log_file() const -> String;

        /**
         *
         * @param serial_logger A logger logging to some serial stream.
         */
        void set_serial_logger(SharedPointer<LegacyLogger> serial_logger);

        /**
         *
         * @param file_logger A logger logging to a file.
         */
        void set_file_logger(UniquePointer<LegacyLogger> file_logger);

        /**
         * Flush the cached log messages.
         *
         * @param flush_file True: Flush to the log file, False: Flush to the serial connection.
         */
        void flush(bool flush_file);
    };
} // namespace Rune
#endif // RUNEOS_LOGGING_H
