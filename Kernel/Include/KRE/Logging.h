
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

    // ========================================================================================== //
    // Logging API - DEPRECATED
    // Kept for backwards compatibility until all call sides have been removed
    // ========================================================================================== //

    /**
     * The severity of a log message.
     * <p>
     * Priorities: Trace < Debug < Info < Warn < Error < Critical.
     * </p>
     */
#define LOG_LEVELS_DEP(X)                                                                          \
    X(LogLevelDep, TRACE, 0x1)                                                                     \
    X(LogLevelDep, DEBUG, 0x2)                                                                     \
    X(LogLevelDep, INFO, 0x3)                                                                      \
    X(LogLevelDep, WARN, 0x4)                                                                      \
    X(LogLevelDep, ERROR, 0x5)                                                                     \
    X(LogLevelDep, CRITICAL, 0x6)

    DECLARE_ENUM(LogLevelDep, LOG_LEVELS_DEP, 0x0) // NOLINT

    /**
     * A log event tracks information about a log message.
     */
    struct LogEventDep {
        LogLevelDep log_level;
        String      formatted_log_msg; // Preformatted log message.
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
        virtual auto layout(LogLevelDep   log_level,
                            const String& logger_name,
                            const String& log_msg_template,
                            Argument*     arg_list,
                            size_t        arg_size) -> String = 0;
    };

    /**
     * The layout for the early boot phases when most kernel features have not been started yet.
     *
     * Layout: [LOG_LEVEL][LOGGER_NAME] LOG_MESSAGE
     */
    class EarlyBootLayout : public Layout {
      public:
        auto layout(LogLevelDep   log_level,
                    const String& logger_name,
                    const String& log_msg_template,
                    Argument*     arg_list,
                    size_t        arg_size) -> String override;
    };

    /**
     * The LogEventDistributor stores all registered layouts and targets and is the central delivery
     * point of all log events.
     *
     * <p>
     *  Layouts and targets have a unique name that loggers can reference to define their message
     *  layout and the targets they want to deliver their messages to.
     * </p>
     */
    class LogEventDistributor {
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
        // HashMap<String, LinkedList<LogEvent>>      _log_event_cache;

        static void deliver_log_event(const SharedPointer<TextStream>& target,
                                      LogLevelDep                      log_level,
                                      const String&                    formatted_log_msg);

      public:
        LogEventDistributor() = default;

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
        void log(LogLevelDep               log_level,
                 const String&             logger_name,
                 const String&             log_msg_template,
                 Argument*                 arg_list,
                 size_t                    arg_size,
                 const String&             layout_ref,
                 const LinkedList<String>& target_refs);
    };

    /**
     * The logger configuration stores the log level, layout ref and target stream refs of a logger.
     */
    struct LoggerConfig {
        LogLevelDep        log_level;
        String             layout_ref;
        LinkedList<String> target_refs;
    };

    /**
     * A logger creates
     */
    class Logger {
        LogEventDistributor* _distributor;
        String               _name;
        LoggerConfig         _config;

      public:
        Logger(LogEventDistributor* distributor, const String& name, const LoggerConfig& config)
            : _distributor(distributor),
              _name(move(name)),
              _config(move(config)) {}

        /**
         *
         * @return The name of the logger.
         */
        [[nodiscard]] auto get_name() const -> String;

        /**
         *
         * @return The log level of the logger.
         */
        [[nodiscard]] auto get_log_level() const -> LogLevelDep;

        /**
         * Change the log level of the logger.
         * @param log_level New log level.
         */
        void set_log_level(LogLevelDep log_level);

        /**
         * Change the layout ref of the logger.
         * @param layout_ref New layout ref.
         */
        void set_layout_ref(const String& layout_ref);

        /// @brief Log a message of the given log level.
        /// @param log_level Log level of the message.
        /// @param fmt       The message as a format string.
        /// @param arg_list  Format string arguments.
        /// @param arg_size  Number of arguments.
        void log(LogLevelDep log_level, const String& fmt, Argument* arg_list, size_t arg_size) {
            if ((int) log_level < (int) _config.log_level) return;
            _distributor->log(log_level,
                              _name,
                              fmt,
                              arg_list,
                              arg_size,
                              _config.layout_ref,
                              _config.target_refs);
        }

        /**
         * Log a trace message.
         *
         * @param fmt    The message as a format string.
         * @param args   Arguments for the format string.
         */
        template <typename... Args>
        void trace(const String& fmt, Args... args) {
            Argument arg_array[] = {args...}; // NOLINT
            log(LogLevelDep::TRACE, fmt, arg_array, sizeof...(Args));
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
            log(LogLevelDep::DEBUG, fmt, arg_array, sizeof...(Args));
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
            log(LogLevelDep::INFO, fmt, arg_array, sizeof...(Args));
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
            log(LogLevelDep::WARN, fmt, arg_array, sizeof...(Args));
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
            log(LogLevelDep::ERROR, fmt, arg_array, sizeof...(Args));
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
            log(LogLevelDep::CRITICAL, fmt, arg_array, sizeof...(Args));
        }
    };

    /**
     * The log context is the main entry point to the logging API. It allows registration of
     * layouts and targets, and handles creation and configuration of logger instances.
     */
    class LogContext {
        LogEventDistributor                    _distributor;
        HashMap<String, SharedPointer<Logger>> _loggers;

        HashMap<String, LoggerConfig> _default_configs;

        LogContext(const HashMap<String, LoggerConfig>& default_configs);
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

            [[nodiscard]] auto to_string() const -> String;
        };

        static auto is_identifier(const String& str) -> bool;

        static auto parse_selector(const String& selector) -> Optional<Selector>;

        auto filter_loggers(Selector selector) -> LinkedList<SharedPointer<Logger>>;

      public:
        static const String ROOT_NAMESPACE;

        LogContext(const LogContext&)                    = delete;
        LogContext(LogContext&&)                         = delete;
        auto operator=(const LogContext&) -> LogContext& = delete;
        auto operator=(LogContext&&) -> LogContext&      = delete;

        /**
         *
         * @return An instance of the log context.
         */
        static auto instance() -> LogContext& {
            // TODO use compile time configuration with macros??
            LogLevelDep                   log_level = LogLevelDep::DEBUG;
            HashMap<String, LoggerConfig> default_configs;
            default_configs[ROOT_NAMESPACE] = {
                .log_level   = log_level,
                .layout_ref  = "earlyboot",
                .target_refs = {"e9", "Boot"}
            };
            default_configs["App"] = {
                .log_level   = log_level,
                .layout_ref  = "earlyboot",
                .target_refs = {"e9", "App"}
            };
            default_configs["Boot"] = {
                .log_level   = log_level,
                .layout_ref  = "earlyboot",
                .target_refs = {"e9", "Boot"}
            };
            default_configs["CPU"] = {
                .log_level   = log_level,
                .layout_ref  = "earlyboot",
                .target_refs = {"e9", "CPU"}
            };
            default_configs["Device"] = {
                .log_level   = log_level,
                .layout_ref  = "earlyboot",
                .target_refs = {"e9", "Device"}
            };
            default_configs["Memory"] = {
                .log_level   = log_level,
                .layout_ref  = "earlyboot",
                .target_refs = {"e9", "Memory"}
            };
            default_configs["SystemCall"] = {
                .log_level   = log_level,
                .layout_ref  = "earlyboot",
                .target_refs = {"e9", "SystemCall"}
            };
            default_configs["VFS"] = {
                .log_level   = log_level,
                .layout_ref  = "earlyboot",
                .target_refs = {"e9", "VFS"}
            };
            static LogContext instance(default_configs);
            return instance;
        }

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
                        LogLevelDep               level,
                        const String&             layout_ref,
                        const LinkedList<String>& target_refs) -> SharedPointer<Logger>;

        /**
         * Create a new logger instance with the requested name and configured default log level,
         * layout ref and target refs.
         *
         * <p>
         *  Logger names are grouped by namespaces and follow the form NAMESPACE.NAME. All loggers
         *  are part of the implicit root namespace if no namespace is explicitly defined. Names
         *  must be unique in a namespace.
         *  The dot selector * can be used to address all loggers in a namespace e.g. NAMESPACE.*.
         *  Using * alone will address all loggers.
         * </p>
         * @param name Unique logger name.
         * @return A pointer to the logger instance or a null pointer if a logger with the name
         *          already exists.
         */
        auto get_logger(const String& name) -> SharedPointer<Logger>;

        /**
         * Change the log level of a single logger or a selection of loggers.
         *
         * @param selector Name of a logger or a selection of loggers.
         * @param level New log level.
         * @return True: The log level of at least one logger is changed. False: No logger(s) with
         *          requested name was found.
         */
        auto set_log_level(const String& selector, LogLevelDep level) -> bool;

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

    // ========================================================================================== //
    // Logging API
    // ========================================================================================== //

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
    using ThreadResolver = Function<Ember::Handle()>;

    /// @brief Returns the handle of the running application.
    using AppResolver = Function<Ember::Handle()>;

    /// @brief Configure the thread and app resolver.
    /// @param thread_resolver
    /// @param app_resolver
    void log_configure(AppResolver app_resolver, ThreadResolver thread_resolver);

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
