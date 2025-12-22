
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

#ifndef RUNEOS_CONFIGLOADER_H
#define RUNEOS_CONFIGLOADER_H

#include "yaml-cpp/node/node.h"

#include <Ember/Enum.h>

#include <Freya/Service.h>

#include <vector>

namespace Freya {

    /// @brief The ServiceLoader is responsible to parse and verify *.service files
    class ServiceLoader {
        static constexpr std::string SERVICE_FILE_EXT = ".service";

        static constexpr const char* NAME               = "Name";
        static constexpr const char* DESCRIPTION        = "Description";
        static constexpr const char* EXEC_START         = "ExecStart";
        static constexpr const char* WAIT_FOR_EXIT      = "WaitForExit";
        static constexpr const char* EXPECTED_EXIT_CODE = "ExpectedExitCode";
        static constexpr const char* MANDATORY          = "Mandatory";
        static constexpr const char* REQUIRES           = "Requires";

        YAML::Node _c_doc;

        /// @brief Check that a service config contains all service properties.
        /// @return True: The service config is valid.<br>
        ///         False: The service config is invalid.
        auto verify_service_config() -> bool;

        /// @brief Create a service from the currently loaded service config.
        /// @return A service created from _c_doc.
        auto create_service() -> Service;

      public:
        /// @brief Load and verify all service configurations in the given directory.
        ///
        /// A service configuration is considered valid if it is valid YAML and follows the schema:
        ///
        /// Name: Name of the service<br>
        /// Description: Description of the service.<br>
        /// ExecStart: The command line input to start the app. Note that the input is simply split
        ///             by ' '.<br>
        /// WaitForExit: yes|no. If yes then Freya waits for the app to finish.<br>
        /// ExpectedExitCode: The expected exit code of the app. If 'WaitForExit==yes', then Freya
        ///                     will check the exit code.<br>
        /// Mandatory: yes|no. If 'WaitForExit==yes' and the ExpectedExitCode is different starting
        ///             other services will be abandoned.<br>
        /// Requires: A list of service that should be started before this service.
        ///
        /// An example service config:
        ///
        /// Name: ListFiles<br>
        /// Description: List all files in "/".<br>
        /// ExecStart: /Apps/ls.app /<br>
        /// WaitForExit: yes<br>
        /// ExpectedExitCode: 0<br>
        /// Mandatory: yes<br>
        /// Requires:
        ///   - CreateFilesInRoot
        ///
        /// @param directory Directory with *.service files.
        /// @return A list of all valid services.
        auto load_services(const std::string& directory) -> std::vector<Service>;
    };
} // namespace Freya

#endif // RUNEOS_CONFIGLOADER_H
