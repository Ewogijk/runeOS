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

#include <Test/Heimdall/ConsoleReporter.h>

namespace Heimdall {
    HString ConsoleReporter::pad(const HString& text, unsigned char fill, bool left) {
        int size_diff = TAG_WIDTH - text.size();
        if (size_diff < 0) return text;
        HString out;
        if (left) {
            out = text;
            for (size_t i = 0; i < size_diff; i++) out = out + fill;
        } else {
            for (size_t i = 0; i < size_diff; i++) out = out + fill;
            out = out + text;
        }
        return out;
    }

    void
    ConsoleReporter::write_tag(const HString& tag, const HString& text, Color color, bool pad_pos) {
        hre_log_console(HString("[") + pad(tag, ' ', pad_pos) + "] " + text + "\n", color);
    }

    void ConsoleReporter::write_tag(const HString& tag, const HString& text, bool pad_pos) {
        hre_log_console(HString("[") + pad(tag, ' ', pad_pos) + "] " + text + "\n");
    }

    void ConsoleReporter::write_divider(char div_char, const HString& text) {
        write_tag(pad("", div_char, true), text, true);
    }

    auto ConsoleReporter::get_name() const -> HString { return "ConsoleReporter"; }

    void ConsoleReporter::on_test_run_begin(const TestRunInfo& test_run_info) {

        hre_log_console(HString("Heimdall v")
                        + HString::number_to_string(test_run_info.heimdall_major) + "."
                        + HString::number_to_string(test_run_info.heimdall_minor) + "."
                        + HString::number_to_string(test_run_info.heimdall_patch) + "\n\n");
        hre_log_console(HString("Runtime Environment: ") + test_run_info.hre.to_c_str() + "\n");

        hre_log_console("Options: ");
        for (size_t i = 0; i < test_run_info.options.size(); ++i) {
            if (i < test_run_info.options.size() - 1)
                hre_log_console(test_run_info.options[i] + ", ");
            else
                hre_log_console(test_run_info.options[i]);
        }
        hre_log_console("\n");

        hre_log_console("Reporters: ");
        for (size_t i = 0; i < test_run_info.reporter_names.size(); ++i) {
            if (i < test_run_info.reporter_names.size() - 1)
                hre_log_console(test_run_info.reporter_names[i] + ", ");
            else
                hre_log_console(test_run_info.reporter_names[i]);
        }
        hre_log_console("\n\n");
    }

    void ConsoleReporter::on_test_run_end(const TestRunStats& test_run_stats) {
        write_divider('=', "");

        write_tag("TOTAL",
                  HString::number_to_string(test_run_stats.total_tests)
                      + (test_run_stats.total_tests > 1 ? " Tests" : " Test"),
                  true);
        write_tag("PASS",
                  HString::number_to_string(test_run_stats.passed_tests)
                      + (test_run_stats.passed_tests > 1 ? " Tests" : " Test"),
                  GREEN,
                  true);
        write_tag("FAIL",
                  HString::number_to_string(test_run_stats.failed_tests)
                      + (test_run_stats.failed_tests > 1 ? " Tests" : " Test"),
                  VSCODE_RED,
                  true);
    }

    void ConsoleReporter::on_test_suite_begin(const TestSuiteInfo& test_suite_info) {
        write_divider('-',
                      test_suite_info.name + " ("
                          + HString::number_to_string(test_suite_info.total_tests)
                          + (test_suite_info.total_tests > 1 ? " Tests" : " Test") + ")");
    }

    void ConsoleReporter::on_test_suite_end(const TestSuiteStats& test_suite_stats) {
        SILENCE_UNUSED(test_suite_stats);
        // Report nothing
    }

    void ConsoleReporter::on_test_begin(const TestInfo& test_info) {
        write_tag("RUN", test_info.name, true);
    }

    void ConsoleReporter::on_test_end(const TestStats& test_stats) {
        if (test_stats.result)
            write_tag("PASS", test_stats.name, GREEN, false);
        else
            write_tag("FAIL", test_stats.name, VSCODE_RED, false);
    }

    void ConsoleReporter::on_assertion_begin(const AssertionInfo& assertion_info) {
        SILENCE_UNUSED(assertion_info);
        // Report nothing
    }

    void ConsoleReporter::on_assertion_end(const AssertionStats& assertion_stats) {
        if (!assertion_stats.result) {
            hre_log_console(HString("             FAIL at ") + assertion_stats.scl.file + ":"
                                + assertion_stats.scl.line + "\n",
                            VSCODE_RED);
            hre_log_console(HString("                       ") + assertion_stats.assert + "\n",
                            VSCODE_CYAN);
            hre_log_console(HString("                 With: ") + assertion_stats.expanded_assert
                                + "\n",
                            VSCODE_CYAN);
        }
    }
} // namespace Heimdall
