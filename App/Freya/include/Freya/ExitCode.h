
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

#ifndef RUNEOS_EXITCODE_H
#define RUNEOS_EXITCODE_H

#include <Ember/Enum.h>

namespace Freya {
#define EXIT_CODES(X)                                                                              \
    X(ExitCode, SERVICES_STARTED, 1)                                                               \
    X(ExitCode, MANDATORY_SERVICE_CRASHED, -1)                                                     \
    X(ExitCode, MISSING_DEPENDENCIES, -2)                                                          \
    X(ExitCode, DEPENDENCY_CYCLE_DETECTED, -3)

    /// @brief Exit codes describing the reason Freya has finished.
    ///
    /// SERVICES_STARTED: All services have successfully started and/or exited.
    /// MANDATORY_SERVICE_CRASHED: A mandatory service has crashed.
    /// MISSING_DEPENDENCIES: A dependency of a service is missing.
    /// DEPENDENCY_CYCLE_DETECTED: A cycle in the dependency graph was detected.
    DECLARE_ENUM(ExitCode, EXIT_CODES, 0x0) // NOLINT

} // namespace Freya

#endif // RUNEOS_EXITCODE_H
