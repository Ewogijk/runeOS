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

#include <LibK/Logging.h>


#include <Hammer/Utility.h>


namespace Rune::LibK {
    IMPLEMENT_ENUM(LogLevel, LOG_LEVELS, 0x0)


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Logger
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    Logger::Logger(
            SharedPointer<LogFormatter> log_msg_fmt,
            LogLevel log_level
    ) :
            _log_msg_fmt(move(log_msg_fmt)),
            _log_level(log_level) {

    }


    SharedPointer<LogFormatter> Logger::get_formatter() const {
        return _log_msg_fmt;
    }


    LogLevel Logger::get_log_level() const {
        return _log_level;
    }


    void Logger::set_log_formatter(SharedPointer<LogFormatter> log_msg_fmt) {
        _log_msg_fmt = move(log_msg_fmt);
    }


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Simple Log Formatter
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    String SimpleLogFormatter::format_log_message(
            LogLevel log_level,
            const String& module,
            const String& log_msg_tmpl,
            Argument* arg_list,
            size_t arg_size
    ) {
        return String::format("[{}] [{}] ", log_level.to_string(), module)
               + String::format(log_msg_tmpl, arg_list, arg_size);
    }


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Text Stream Logger
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    TextStreamLogger::TextStreamLogger(
            SharedPointer<LibK::LogFormatter> log_msg_fmt,
            LibK::LogLevel log_level,
            UniquePointer<LibK::TextStream> txt_stream
    ) : Logger(move(log_msg_fmt), log_level), _txt_stream(move(txt_stream)) {
        SILENCE_UNUSED(_log_msg_fmt)
    }


    void TextStreamLogger::log(
            LibK::LogLevel log_level,
            const String& module,
            const String& fmt,
            Argument* arg_list,
            size_t arg_size
    ) {
        if ((int) log_level < (int) _log_level)
            return;

        if (_txt_stream->is_ansi_supported()) {
            // Only set the background color to red when a critical message is logged
            // and keep the default background color of the stream for other log levels
            // Setting the background color in all cases looks strange on other terminals
            // e.g. clion, windows terminal, etc.
            if (log_level == LogLevel::CRITICAL)
                _txt_stream->set_background_color(BG_COLOR_CRITICAL);
            _txt_stream->set_foreground_color(FG_COLOR[(int) log_level - 1]);
        }

        String log_message = _log_msg_fmt->format_log_message(
                log_level,
                module,
                fmt,
                arg_list,
                arg_size
        );
        _txt_stream->write_line(log_message);

        if (_txt_stream->is_ansi_supported())
            _txt_stream->reset_style();

        _txt_stream->flush();
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          System Logger
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    SystemLogger::SystemLogger(
            SharedPointer<LogFormatter> log_msg_fmt,
            LogLevel log_level,
            const String& log_file
    ) : Logger(move(log_msg_fmt), log_level),
        _log_file(log_file) {
        SILENCE_UNUSED(log_msg_fmt)
    }


    void SystemLogger::update_log_formatter(const SharedPointer<LogFormatter>& log_msg_fmt) {
        set_log_formatter(log_msg_fmt);
        _serial_logger->set_log_formatter(log_msg_fmt);
        _file_logger->set_log_formatter(log_msg_fmt);
    }


    String SystemLogger::get_log_file() const {
        return _log_file;
    }


    void SystemLogger::set_serial_logger(SharedPointer<Logger> serial_logger) {
        _serial_logger = move(serial_logger);
    }


    void SystemLogger::set_file_logger(UniquePointer<Logger> file_logger) {
        _file_logger = move(file_logger);
    }


    void SystemLogger::log(
            LogLevel log_level,
            const String& module,
            const String& fmt,
            Argument* arg_list,
            size_t arg_size
    ) {
        if (!_serial_logger || !_file_logger) {
            _log_cache.add_back({ log_level, module, String::format(fmt, arg_list, arg_size) });
        }

        if (_serial_logger)
            _serial_logger->log(log_level, module, fmt, arg_list, arg_size);
        if (_file_logger)
            _file_logger->log(log_level, module, fmt, arg_list, arg_size);
    }


    void SystemLogger::flush(bool flush_file) {
        for (auto& msg: _log_cache) {
            if (flush_file)
                _file_logger->log(msg.log_level, msg.file, msg.pre_formatted_text, { }, 0);
            else
                _serial_logger->log(msg.log_level, msg.file, msg.pre_formatted_text, { }, 0);
        }
    }
}