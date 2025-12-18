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

#include <Test/Heimdall/TestRunInfoReporter.h>

namespace Heimdall {
    auto TestRunInfoReporter::get_name() const -> HString {
        return "TestRunInfoReporter";
    }

    void TestRunInfoReporter::on_test_run_begin(const TestRunInfo& test_run_info) {
        SILENCE_UNUSED(test_run_info);
        HString env_info = HString("Heimdall v")
                           + HString::number_to_string(test_run_info.heimdall_major) + "."
                           + HString::number_to_string(test_run_info.heimdall_minor) + "."
                           + HString::number_to_string(test_run_info.heimdall_patch) + "\n\n";
        env_info =
            env_info + HString("Runtime Environment: ") + test_run_info.hre.to_c_str() + "\n";

        env_info = env_info + "Options: ";
        for (size_t i = 0; i < test_run_info.options.size(); ++i) {
            if (i < test_run_info.options.size() - 1)
                env_info = env_info + test_run_info.options[i] + ", ";
            else
                env_info = env_info + test_run_info.options[i];
        }
        env_info = env_info + "\n";

        env_info = env_info + "Reporters: ";
        for (size_t i = 0; i < test_run_info.reporter_names.size(); ++i) {
            if (i < test_run_info.reporter_names.size() - 1)
                env_info = env_info + test_run_info.reporter_names[i] + ", ";
            else
                env_info = env_info + test_run_info.reporter_names[i];
        }
        env_info = env_info + "\n\n";
        hre_save_to_file(test_run_info.test_report_directory + "TestRunInfo.txt",
                         env_info);
    }

    void TestRunInfoReporter::on_test_run_end(const TestRunStats& test_run_stats) {
        SILENCE_UNUSED(test_run_stats);
    }

    void TestRunInfoReporter::on_test_suite_begin(const TestSuiteInfo& test_suite_info) {
        SILENCE_UNUSED(test_suite_info);
    }

    void TestRunInfoReporter::on_test_suite_end(const TestSuiteStats& test_suite_stats) {
        SILENCE_UNUSED(test_suite_stats);
        // Report nothing
    }

    void TestRunInfoReporter::on_test_begin(const TestInfo& test_info) {
        SILENCE_UNUSED(test_info);
    }

    void TestRunInfoReporter::on_test_end(const TestStats& test_stats) {
        SILENCE_UNUSED(test_stats);
    }

    void TestRunInfoReporter::on_assertion_begin(const AssertionInfo& assertion_info) {
        SILENCE_UNUSED(assertion_info);
        // Report nothing
    }

    void TestRunInfoReporter::on_assertion_end(const AssertionStats& assertion_stats) {
        SILENCE_UNUSED(assertion_stats);
    }
} // namespace Heimdall
