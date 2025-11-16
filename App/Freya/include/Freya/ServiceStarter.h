
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

#ifndef RUNEOS_SERVICESTARTER_H
#define RUNEOS_SERVICESTARTER_H

#include <string>
#include <vector>

#include <Freya/Service.h>
#include <Freya/ExitCode.h>

namespace Freya {

    /// @brief The ServiceStarter starts all services as configured in the order determined by the
    ///         topological order of the dependency graph.
    class ServiceStarter {
        static auto split(const std::string& s, char delim) -> std::vector<std::string>;

      public:
        /// @brief Start all services based on their topological order.
        ///
        /// If starting a service fails, it will be skipped if it is not mandatory, otherwise no
        /// more services will be started and the service start will be considered failed.
        ///
        /// Additionally, if 'service.wait_for_exit == true', then Freya will wait for the service
        /// to finish and verify that 'service.expected_exit_code == actual_exit_code'. When the
        /// condition is false, again, if the service is not mandatory the failure is ignored,
        /// otherwise the service start is considered failed.
        ///
        /// @param registry Service registry.
        /// @param sorted_services Names of all services in topological ordering.
        /// @return SERVICES_STARTED: All services have been started.<br>
        ///         MANDATORY_SERVICE_CRASHED: A mandatory service has crashed.
        auto start_services(ServiceRegistry&                registry,
                            const std::vector<std::string>& sorted_services) -> int;
    };
} // namespace Freya

#endif // RUNEOS_SERVICESTARTER_H
