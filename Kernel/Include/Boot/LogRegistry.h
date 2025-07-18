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

#ifndef RUNEOS_LOGREGISTRY_H
#define RUNEOS_LOGREGISTRY_H


#include <Hammer/Collection.h>

#include <LibK/Logging.h>

#include <VirtualFileSystem/VFSSubsystem.h>


namespace Rune {
    class LogRegistry {
        VFS::Subsystem* _vfs_subsystem;
        Path                         _system_directory;
        SharedPointer<LibK::LogFormatter> _log_msg_fmt;

        LinkedList<SharedPointer<LibK::Logger>> _logger_registry;
        SharedPointer<LibK::Logger>             _serial_logger;
        bool                                    _file_logging_available;
    public:

        explicit LogRegistry();


        /**
         * Set the file subsystem and the system directory.
         *
         * @param f_subsystem
         * @param system_directory
         */
        void init(VFS::Subsystem* f_subsystem, const Path& system_directory);


        /**
         * @brief Update the log formatter of all loggers.
         * @param log_formatter
         */
        void update_log_formatter(const SharedPointer<LibK::LogFormatter>& log_formatter);


        /**
         *
         * @param lvl
         * @param logFile
         * @return
         */
        SharedPointer<LibK::Logger> build_logger(LibK::LogLevel lvl, const Path& path);


        /**
         * Enable serial logging for all already registered loggers and future loggers.
         *
         * @param stream
         * @param log_level
         */
        void enable_serial_logging(UniquePointer<LibK::TextStream> stream, LibK::LogLevel log_level);


        /**
         * Enable file logging for all already registered loggers and future loggers.
         */
        void enable_file_logging();
    };

} // EwogijkOS

#endif //RUNEOS_LOGREGISTRY_H
