
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

#ifndef HEIMDALL_HRE_H
#define HEIMDALL_HRE_H

#include <Test/Heimdall/Configuration.h>
#include <Test/Heimdall/HString.h>
#include <Test/Heimdall/Test.h>

namespace Heimdall {

    /// @brief Name of the heimdall runtime environment for informational purposes.
    /// @return HRE name.
    auto hre_get_runtime_name() -> HString;

    /// @brief Log a message in case something terrible happened.
    /// @param message
    void hre_emergency_log(const HString& message);

    /// @brief Perform engine configuration of the heimdall runtime environment.
    /// @param config Configuration provided by the engine.
    void hre_configure(Configuration& config);

    /// @brief Save the test report to path.
    /// @param path Absolute path to the save location of the test report.
    /// @param test_report A test report.
    void hre_save_test_report(const HString& path, const TestReport& test_report);
}

#endif // HEIMDALL_HRE_H
