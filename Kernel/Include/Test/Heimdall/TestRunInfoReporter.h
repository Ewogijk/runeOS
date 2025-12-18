
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

#ifndef HEIMDALL_ENVIRONMENTREPORTER_H
#define HEIMDALL_ENVIRONMENTREPORTER_H

#include <Test/Heimdall/Reporter.h>
#include <Test/Heimdall/HRE.h>

namespace Heimdall {

    /// @brief The ConsoleReporter prints a test report in a human-readable format to the console
    class TestRunInfoReporter : public Reporter {
        HString _test_report_directory;
    public:
        [[nodiscard]] auto get_name() const -> HString override;
        void               on_test_run_begin(const TestRunInfo& test_run_info) override;
        void               on_test_run_end(const TestRunStats& test_run_stats) override;
        void               on_test_suite_begin(const TestSuiteInfo& test_suite_info) override;
        void               on_test_suite_end(const TestSuiteStats& test_suite_stats) override;
        void               on_test_begin(const TestInfo& test_info) override;
        void               on_test_end(const TestStats& test_stats) override;
        void               on_assertion_begin(const AssertionInfo& assertion_info) override;
        void               on_assertion_end(const AssertionStats& assertion_stats) override;
    };
}
#endif // HEIMDALL_ENVIRONMENTREPORTER_H
