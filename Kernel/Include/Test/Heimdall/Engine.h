
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

#ifndef RUNEOS_ENGINE_H
#define RUNEOS_ENGINE_H

#include <Test/Heimdall/Configuration.h>
#include <Test/Heimdall/Expression.h>
#include <Test/Heimdall/Reporter.h>

namespace Heimdall {

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      Engine
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /// @brief All possible test results.
    enum class TestResult {
        PASS,
        FAIL,
    };

    /// @brief The test engine configures the library and executes all tests.
    class Engine {
        TestResult    _test_result; // Result of the currently running test.
        Configuration _configuration;

        auto configure(const HStringList& options) -> bool;

      public:
        [[nodiscard]] auto get_current_test_result() const -> TestResult;

        /**
         * Report of an assertion handler that it is about to evaluate an assertion.
         * @param assert_info Assertion info.
         */
        void report_assertion_begin(const AssertionInfo& assert_info);

        /**
         * Report of an assertion handler that it has evaluated an assertion.
         * @param assert_info Assertion info.
         */
        void report_assertion_end(const AssertionStats& assert_stats);

        /// @brief Configure heimdall according to the options and then execute all registered
        ///         tests.
        ///
        /// Options are either general or heimdall runtime environment specific.
        ///
        /// General options:
        ///     - None
        ///
        /// Kernel Test Options:
        ///     - e9-reporter: Log the test report to the E9 port.
        ///
        /// @param options List of options.
        void execute(const HStringList& options);
    };

    /// @brief
    /// @return A reference to the test engine.
    auto get_engine() -> Engine&;

    /// @brief Configure heimdall according to the options and then execute all registered tests.
    ///
    /// A convenience method for 'Heimdall::get_engine.execute(options)'
    ///
    /// @param options List of options.
    void execute_tests(const HStringList& options);
} // namespace Heimdall

#endif // RUNEOS_ENGINE_H
