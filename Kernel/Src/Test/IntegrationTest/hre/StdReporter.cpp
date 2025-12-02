/*noexcept
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

#include <Test/IntegrationTest/hre/StdReporter.h>

#include <format>
#include <iostream>

#include <Test/IntegrationTest/hre/ANSIWriter.h>

namespace Heimdall {
    void StdReporter::write_tag(const HString& tag, const HString& text) {
        std::cout << std::format("[{:<10}] {}", tag.to_c_str(), text.to_c_str()) << std::endl;
    }

    void StdReporter::write_tag(const HString& tag, const HString& text, Pixel pixel) {
        ansi_write_text(std::format("[{:<10}] {}", tag.to_c_str(), text.to_c_str()), pixel);
    }

    void StdReporter::write_divider(char div_char, const HString& text) {
        HString div;
        for (size_t i = 0; i < TAG_WIDTH; ++i) div = div + div_char;
        write_tag(div, text);
    }

    auto StdReporter::get_name() const -> HString { return "E9Reporter"; }

    void StdReporter::on_test_run_begin(const TestRunInfo& test_run_info) {

        auto header = std::format("Heimdall v{}.{}.{}",
                                  test_run_info.heimdall_major,
                                  test_run_info.heimdall_minor,
                                  test_run_info.heimdall_patch);
        std::cout << header << std::endl << std::endl;
        std::cout << "Registered reports: ";
        for (size_t i = 0; i < test_run_info.reporter_names.size(); ++i) {
            if (i < test_run_info.reporter_names.size() - 1)
                std::cout << test_run_info.reporter_names[i].to_c_str() << ", ";
            else
                std::cout << test_run_info.reporter_names[i].to_c_str();
        }
        std::cout << std::endl << std::endl;
    }

    void StdReporter::on_test_run_end(const TestRunStats& test_run_stats) {
        write_divider('=', "");

        write_tag("TOTAL",
                  HString::number_to_string(test_run_stats.total_tests)
                      + (test_run_stats.total_tests > 1 ? " Tests" : " Test"));
        write_tag("PASS",
                  HString::number_to_string(test_run_stats.passed_tests)
                      + (test_run_stats.passed_tests > 1 ? " Tests" : " Test"),
                  GREEN);
        write_tag("FAIL",
                  HString::number_to_string(test_run_stats.failed_tests)
                      + (test_run_stats.failed_tests > 1 ? " Tests" : " Test"),
                  VSCODE_RED);
    }

    void StdReporter::on_test_suite_begin(const TestSuiteInfo& test_suite_info) {
        write_divider('-',
                      test_suite_info.name + " ("
                          + HString::number_to_string(test_suite_info.total_tests)
                          + (test_suite_info.total_tests > 1 ? " Tests" : " Test") + ")");
    }

    void StdReporter::on_test_suite_end(const TestSuiteStats& test_suite_stats) {
        SILENCE_UNUSED(test_suite_stats);
        // Report nothing
    }

    void StdReporter::on_test_begin(const TestInfo& test_info) { write_tag("RUN", test_info.name); }

    void StdReporter::on_test_end(const TestStats& test_stats) {
        if (test_stats.result)
            write_tag("PASS", test_stats.name, GREEN);
        else
            write_tag("FAIL", test_stats.name, VSCODE_RED);
    }

    void StdReporter::on_assertion_begin(const AssertionInfo& assertion_info) {
        SILENCE_UNUSED(assertion_info);
        // Report nothing
    }

    void StdReporter::on_assertion_end(const AssertionStats& assertion_stats) {
        if (!assertion_stats.result) {
            ansi_write_text(std::format("             FAIL at {}:{}",
                                        assertion_stats.scl.file.to_c_str(),
                                        assertion_stats.scl.line),
                            VSCODE_RED);
            std::cout << std::endl;

            ansi_write_text(
                std::format("                       {}\n", assertion_stats.assert.to_c_str()),
                VSCODE_CYAN);
            ansi_write_text(std::format("                 With: {}\n",
                                        assertion_stats.expanded_assert.to_c_str()),
                            VSCODE_CYAN);
            std::cout << std::endl;
        }
    }
} // namespace Heimdall
