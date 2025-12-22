
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
#include <Test/UnitTest/tests/Dummy2.h>

#include <KRE/Build.h>

namespace Rune::Test {

    /// @brief Configure the E9 reporter and execute the kernel tests.
    void run_kernel_tests() {
        Heimdall::OptionList options;
#ifdef SHUTDOWN_ON_SYSTEM_LOADER_EXIT
        // This flag comes from Build.h and is only defined when the kernel is build for CI
        // -> Use the JUnitReporter to create a JUnit test report to be displayed by some GitHub
        //      action
        options.insert({.name = Heimdall::Engine::JUNIT_REPORTER, .value = ""});
        options.insert({.name = Heimdall::Engine::GNOME_REPORTER, .value = ""});
#else
        options.insert({.name = Heimdall::Engine::CONSOLE_REPORTER, .value = ""});
#endif
        options.insert({.name = Heimdall::Engine::TEST_RUN_INFO_REPORTER, .value = ""});
        options.insert({.name  = Heimdall::Engine::TEST_REPORT_DIRECTORY,
                        .value = "/System/Heimdall/UnitTest/"});
        Heimdall::execute_tests(options);
    }
} // namespace Rune::Test
