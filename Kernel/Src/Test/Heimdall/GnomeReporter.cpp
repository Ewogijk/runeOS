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

#include <Test/Heimdall/HRE.h>

#include <Test/Heimdall/GnomeReporter.h>

namespace Heimdall {
    auto GnomeReporter::get_name() const -> HString { return "GnomeReporter"; }

    void GnomeReporter::on_test_run_begin(const TestRunInfo& test_run_info) {
        _test_report_directory = test_run_info.test_report_directory;
    }

    void GnomeReporter::on_test_run_end(const TestRunStats& test_run_stats) {
        SILENCE_UNUSED(test_run_stats)
        if (test_run_stats.failed_tests == 0)
            hre_save_test_report(_test_report_directory + "Pass.txt", "");
    }

    void GnomeReporter::on_test_suite_begin(const TestSuiteInfo& test_suite_info) {
        SILENCE_UNUSED(test_suite_info)
    }

    void GnomeReporter::on_test_suite_end(const TestSuiteStats& test_suite_stats) {
        SILENCE_UNUSED(test_suite_stats)
    }

    void GnomeReporter::on_test_begin(const TestInfo& test_info) { SILENCE_UNUSED(test_info) }

    void GnomeReporter::on_test_end(const TestStats& test_stats) { SILENCE_UNUSED(test_stats) }

    void GnomeReporter::on_assertion_begin(const AssertionInfo& assertion_info) {
        SILENCE_UNUSED(assertion_info)
    }

    void GnomeReporter::on_assertion_end(const AssertionStats& assertion_stats) {
        SILENCE_UNUSED(assertion_stats)
    }
} // namespace Heimdall
