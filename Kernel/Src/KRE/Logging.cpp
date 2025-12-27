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

namespace Rune {
    DEFINE_ENUM(LogLevel, LOG_LEVELS, 0x0)

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      EarlyBootLayout
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    auto EarlyBootLayout::layout(LogLevel      log_level,
                                 const String& logger_name,
                                 const String& log_msg_template,
                                 Argument*     arg_list,
                                 size_t        arg_size) -> String {
        return String::format("[{}] [{}] ", log_level.to_string(), logger_name)
               + String::format(log_msg_template, static_cast<const Argument*>(arg_list), arg_size);
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                  LogEventDistributor
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    void LogEventDistributor::deliver_log_event(const SharedPointer<TextStream>& target,
                                                LogLevel                         log_level,
                                                const String& formatted_log_msg) {
        if (target->is_ansi_supported()) {
            // Only set the background color to red when a critical message is logged
            // and keep the default background color of the stream for other log levels
            // Setting the background color in all cases looks strange on other terminals
            // e.g. Clion, powershell, etc.
            if (log_level == LogLevel::CRITICAL) target->set_background_color(BG_COLOR_CRITICAL);
            target->set_foreground_color(FG_COLOR[(int) log_level - 1]);
        }
        target->write_line(formatted_log_msg);
        if (target->is_ansi_supported()) target->reset_style();
        target->flush();
    }

    auto LogEventDistributor::register_layout(const String& name, SharedPointer<Layout> layout)
        -> bool {
        if (_layouts.find(name) != _layouts.end()) return false;
        return _layouts.put(name, move(layout)) != _layouts.end();
    }

    auto LogEventDistributor::register_target_stream(const String&             name,
                                                     SharedPointer<TextStream> target) -> bool {
        if (_target_streams.find(name) != _target_streams.end()) return false;
        bool success = _target_streams.put(name, move(target)) != _target_streams.end();
        if (success) {
            // Flush the log event cache
            auto maybe_cache = _log_event_cache.find(name);
            if (maybe_cache != _log_event_cache.end()) {
                LinkedList<LogEvent> cache = *maybe_cache->value;
                for (auto& log_event : *maybe_cache->value)
                    deliver_log_event(target, log_event.log_level, log_event.formatted_log_msg);
                cache.clear();
                _log_event_cache.remove(name);
            }
        }

        return success;
    }

    void LogEventDistributor::log(LogLevel                  log_level,
                                  const String&             logger_name,
                                  const String&             log_msg_template,
                                  Argument*                 arg_list,
                                  size_t                    arg_size,
                                  const String&             layout_ref,
                                  const LinkedList<String>& target_refs) {
        auto maybe_layout = _layouts.find(layout_ref);
        if (maybe_layout == _layouts.end()) return;
        String formatted_log_message =
            (*maybe_layout->value)
                ->layout(log_level, logger_name, log_msg_template, arg_list, arg_size);

        for (auto& target : target_refs) {
            auto maybe_target = _target_streams.find(target);
            if (maybe_target == _target_streams.end()) {
                // Creates list if missing
                _log_event_cache[target].add_back(
                    {.log_level = log_level, .formatted_log_msg = formatted_log_message});
                continue;
            }

            SharedPointer<TextStream> target_stream = (*maybe_target->value);
            deliver_log_event(target_stream, log_level, formatted_log_message);
        }
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      Logger
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    auto Logger::get_name() const -> String { return _name; }

    auto Logger::get_log_level() const -> LogLevel { return _config.log_level; }

    void Logger::set_log_level(LogLevel new_log_level) { _config.log_level = new_log_level; }

    void Logger::set_layout_ref(const String& layout_ref) { _config.layout_ref = move(layout_ref); }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      LogContext
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    auto LogContext::Selector::to_string() const -> String { return the_namespace + '.' + name; }

    auto LogContext::is_identifier(const String& str) -> bool {
        if (str.is_empty()) return true;
        for (const auto& chr : str) {
            bool is_digit = (chr >= '0' && chr <= '9');
            bool is_upper = (chr >= 'A' && chr <= 'Z');
            bool is_lower = (chr >= 'a' && chr <= 'z');

            if (!(is_digit || is_upper || is_lower)) return false;
        }
        return true;
    }

    auto LogContext::parse_selector(const String& selector) -> Optional<Selector> {
        LinkedList<String> parts = selector.split('.');
        switch (parts.size()) {
            case 1: {
                String part0 = *parts[0];
                if (part0 != "*" && !is_identifier(part0)) return NULL_OPT;
                return make_optional<Selector>(*parts[0], "");
            }
            case 2: {
                String part0 = *parts[0];
                String part1 = *parts[1];
                if (part0 == "*" || !is_identifier(part0)) return NULL_OPT;
                if (part1 != "*" && !is_identifier(part1)) return NULL_OPT;
                return make_optional<Selector>(*parts[0], *parts[1]);
            }
            default: return NULL_OPT;
        }
    }

    auto LogContext::filter_loggers(Selector selector) -> LinkedList<SharedPointer<Logger>> {
        LinkedList<SharedPointer<Logger>> filter_list;
        if (selector.the_namespace == "*") {
            // Select all loggers
            for (const auto& logger : _loggers) filter_list.add_back(*logger.value); // NOLINT
        } else {
            if (selector.name == "*") {
                // Select all loggers in a namespace
                for (const auto& logger : _loggers) {
                    if (logger.key->starts_with(selector.the_namespace)) // NOLINT
                        filter_list.add_back(*logger.value);
                }
            } else {
                // Select a single logger
                auto maybe_logger = _loggers.find(selector.to_string());
                if (maybe_logger == _loggers.end()) return LinkedList<SharedPointer<Logger>>();
                filter_list.add_back(*maybe_logger->value);
            }
        }
        return filter_list;
    }

    const String LogContext::ROOT_NAMESPACE = "root";

    LogContext::LogContext(const HashMap<String, LoggerConfig>& default_configs)
        : _default_configs(default_configs) {}

    auto LogContext::get_logger(const String&             name,
                                LogLevel                  level,
                                const String&             layout_ref,
                                const LinkedList<String>& target_refs) -> SharedPointer<Logger> {
        auto register_logger = [this, &name, level, &layout_ref, &target_refs](
                                   Selector sel) -> Optional<SharedPointer<Logger>> {
            SILENCE_UNUSED(sel)
            auto maybe_logger = _loggers.find(name);
            if (maybe_logger != _loggers.end()) return NULL_OPT;

            LoggerConfig config = {.log_level   = level,
                                   .layout_ref  = layout_ref,
                                   .target_refs = target_refs};
            auto logger = _loggers.put(name, make_shared<Logger>(&_distributor, name, config));
            return make_optional<SharedPointer<Logger>>(*logger->value);
        };
        return parse_selector(name)
            .and_then<SharedPointer<Logger>>(register_logger)
            .value_or(SharedPointer<Logger>());
    }

    auto LogContext::get_logger(const String& name) -> SharedPointer<Logger> {
        auto register_logger = [this, &name](Selector sel) -> Optional<SharedPointer<Logger>> {
            SILENCE_UNUSED(sel)
            auto maybe_logger = _loggers.find(name);
            if (maybe_logger != _loggers.end()) return NULL_OPT;

            auto maybe_config = _default_configs.find(sel.the_namespace);
            // Use the root namespace config if no namespace is selected
            if (maybe_config == _default_configs.end())
                maybe_config = _default_configs.find(ROOT_NAMESPACE);

            auto logger =
                _loggers.put(name, make_shared<Logger>(&_distributor, name, *maybe_config->value));
            return make_optional<SharedPointer<Logger>>(*logger->value);
        };
        return parse_selector(name)
            .and_then<SharedPointer<Logger>>(register_logger)
            .value_or(SharedPointer<Logger>());
    }

    auto LogContext::set_log_level(const String& selector, LogLevel level) -> bool {
        auto change_level = [this, level](Selector sel) -> Optional<bool> {
            LinkedList<SharedPointer<Logger>> filter_list = filter_loggers(sel);
            if (filter_list.is_empty()) return NULL_OPT;
            for (auto& logger : filter_list) logger->set_log_level(level);
            return make_optional<bool>(true);
        };
        return parse_selector(selector).and_then<bool>(change_level).value_or(false);
    }

    auto LogContext::set_layout_ref(const String& selector, const String& layout_ref) -> bool {
        auto change_layout_ref = [this, &layout_ref](Selector sel) -> Optional<bool> {
            LinkedList<SharedPointer<Logger>> filter_list = filter_loggers(sel);
            if (filter_list.is_empty()) return NULL_OPT;
            for (auto& logger : filter_list) logger->set_layout_ref(layout_ref);
            return make_optional<bool>(true);
        };
        return parse_selector(selector).and_then<bool>(change_layout_ref).value_or(false);
    }

    auto LogContext::register_layout(const String& name, SharedPointer<Layout> layout) -> bool {
        return _distributor.register_layout(name, layout);
    }

    auto LogContext::register_target_stream(const String& name, SharedPointer<TextStream> target)
        -> bool {
        return _distributor.register_target_stream(name, target);
    }
} // namespace Rune
