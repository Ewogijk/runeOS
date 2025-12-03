
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

#include <Test/Heimdall/Heimdall.h>

#include <Test/UnitTest/tests/Dummy.h>

namespace Rune::Test {

    /// @brief Configure the E9 reporter and execute the kernel tests.
    auto run_kernel_tests() -> Heimdall::TestResult {
        Heimdall::OptionList options;
        options.insert({.name = "e9-reporter", .value = ""});
        options.insert(
            {.name = "test-report-location", .value = "/System/Heimdall/UnitTestReport.txt"});
        return Heimdall::execute_tests(options).result;
    }
} // namespace Rune::Test
