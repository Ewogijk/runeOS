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

#include <Boot/LogRegistry.h>


#include <VirtualFileSystem/FileStream.h>


namespace Rune {
    LogRegistry::LogRegistry() :
            _vfs_subsystem(nullptr),
            _system_directory(),
            _log_msg_fmt(nullptr),
            _logger_registry(),
            _serial_logger(),
            _file_logging_available(false) {

    }


    void LogRegistry::init(VFS::Subsystem* f_subsystem, const Path& system_directory) {
        _vfs_subsystem    = f_subsystem;
        _system_directory = move(system_directory);
        _log_msg_fmt      = SharedPointer<LibK::LogFormatter>(new LibK::SimpleLogFormatter());
    }


    void LogRegistry::update_log_formatter(const SharedPointer<LibK::LogFormatter>& log_formatter) {
        _log_msg_fmt = move(log_formatter);
        for (auto& logger: _logger_registry)
            ((LibK::SystemLogger*) logger.get())->update_log_formatter(_log_msg_fmt);
    }


    SharedPointer<LibK::Logger> LogRegistry::build_logger(LibK::LogLevel lvl, const Path& path) {
        SharedPointer<LibK::Logger> logger(
                new LibK::SystemLogger(
                        _log_msg_fmt,
                        lvl,
                        path.to_string()
                )
        );

        if (_serial_logger)
            ((LibK::SystemLogger*) logger.get())->set_serial_logger(_serial_logger);

        if (_file_logging_available) {
            SharedPointer<VFS::Node> node;
            VFS::IOStatus            io_status = _vfs_subsystem->open(
                    _system_directory / path,
                    VFS::IOMode::APPEND,
                    node
            );
            if (io_status == VFS::IOStatus::OPENED) {
                ((LibK::SystemLogger*) logger.get())->set_file_logger(
                        UniquePointer<LibK::Logger>(
                                new LibK::TextStreamLogger(
                                        _log_msg_fmt,
                                        lvl,
                                        UniquePointer<LibK::TextStream>(
                                                new VFS::FileStream(node)
                                        )
                                )
                        )
                );
            }
        }

        _logger_registry.add_back(logger);
        return logger;
    }


    void LogRegistry::enable_serial_logging(UniquePointer<LibK::TextStream> stream, LibK::LogLevel log_level) {
        _serial_logger = SharedPointer<LibK::Logger>(
                new LibK::TextStreamLogger(
                        _log_msg_fmt,
                        log_level,
                        move(stream)
                )
        );
        for (auto& l: _logger_registry)
            ((LibK::SystemLogger*) l.get())->set_serial_logger(_serial_logger);
    }


    void LogRegistry::enable_file_logging() {
        _file_logging_available = true;
        for (auto& l: _logger_registry) {
            Path     log_file = _system_directory / ((LibK::SystemLogger*) l.get())->get_log_file();
            VFS::IOStatus st       = _vfs_subsystem->create(
                    log_file,
                    VFS::NodeAttribute::FILE | VFS::NodeAttribute::SYSTEM
            );
            if (st != VFS::IOStatus::CREATED && st != VFS::IOStatus::FOUND) {
                return;
            }

            SharedPointer<VFS::Node> node;
            VFS::IOStatus            io_status = _vfs_subsystem->open(
                    _system_directory / ((LibK::SystemLogger*) l.get())->get_log_file(),
                    VFS::IOMode::WRITE,
                    node
            );
            if (io_status == VFS::IOStatus::OPENED) {
                ((LibK::SystemLogger*) l.get())->set_file_logger(
                        UniquePointer<LibK::Logger>(
                                new LibK::TextStreamLogger(
                                        _log_msg_fmt,
                                        l->get_log_level(),
                                        UniquePointer<LibK::TextStream>(
                                                new VFS::FileStream(node)
                                        )
                                )
                        )
                );
                ((LibK::SystemLogger*) l.get())->flush(true);
            }
        }
    }
}
