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

#include <Test/UnitTest/hre/E9Reporter.h>

namespace Heimdall {
    void E9Reporter::write_tag(const HString& tag, const HString& text) {
        _e9.write_formatted("[{:<10}] {}\n", tag.to_c_str(), text.to_c_str());
    }

    void E9Reporter::write_colored_tag(const HString& tag, const HString& text, Rune::Pixel pixel) {
        _e9.set_foreground_color(pixel);
        _e9.write_formatted("[{:<10}] {}\n", tag.to_c_str(), text.to_c_str());
        _e9.reset_style();
    }

    void E9Reporter::write_divider(char div_char, const HString& text) {
        HString div;
        for (size_t i = 0; i < TAG_WIDTH; ++i) div = div + div_char;
        write_tag(div, text);
    }

    auto E9Reporter::get_name() const -> HString { return "E9Reporter"; }

    void E9Reporter::on_test_run_begin(const TestRunInfo& test_run_info) {
        _e9.write_formatted("Heimdall v{}.{}.{}\n\n",
                            test_run_info.heimdall_major,
                            test_run_info.heimdall_minor,
                            test_run_info.heimdall_patch);

        _e9.write_formatted("Heimdall Runtime Environment: {}\n", test_run_info.hre.to_c_str());

        _e9.write_formatted("Options: ");
        for (size_t i = 0; i < test_run_info.options.size(); ++i) {
            if (i < test_run_info.options.size() - 1)
                _e9.write_formatted("{}, ", test_run_info.options[i].to_c_str());
            else
                _e9.write_formatted("{}", test_run_info.options[i].to_c_str());
        }
        _e9.write_line("");

        _e9.write_formatted("Reporters: ");
        for (size_t i = 0; i < test_run_info.reporter_names.size(); ++i) {
            if (i < test_run_info.reporter_names.size() - 1)
                _e9.write_formatted("{}, ", test_run_info.reporter_names[i].to_c_str());
            else
                _e9.write_formatted("{}", test_run_info.reporter_names[i].to_c_str());
        }
        _e9.write_line("\n");
    }

    void E9Reporter::on_test_run_end(const TestRunStats& test_run_stats) {
        write_divider('=', "");

        write_tag("TOTAL",
                  HString::number_to_string(test_run_stats.total_tests)
                      + (test_run_stats.total_tests > 1 ? " Tests" : " Test"));
        write_colored_tag("PASS",
                          HString::number_to_string(test_run_stats.passed_tests)
                              + (test_run_stats.passed_tests > 1 ? " Tests" : " Test"),
                          Rune::Pixie::GREEN);
        write_colored_tag("FAIL",
                          HString::number_to_string(test_run_stats.failed_tests)
                              + (test_run_stats.failed_tests > 1 ? " Tests" : " Test"),
                          Rune::Pixie::VSCODE_RED);
    }

    void E9Reporter::on_test_suite_begin(const TestSuiteInfo& test_suite_info) {
        write_divider('-',
                      test_suite_info.name + " ("
                          + HString::number_to_string(test_suite_info.total_tests)
                          + (test_suite_info.total_tests > 1 ? " Tests" : " Test") + ")");
    }

    void E9Reporter::on_test_suite_end(const TestSuiteStats& test_suite_stats) {
        SILENCE_UNUSED(test_suite_stats);
        // Report nothing
    }

    void E9Reporter::on_test_begin(const TestInfo& test_info) { write_tag("RUN", test_info.name); }

    void E9Reporter::on_test_end(const TestStats& test_stats) {
        if (test_stats.result)
            write_colored_tag("PASS", test_stats.name, Rune::Pixie::GREEN);
        else
            write_colored_tag("FAIL", test_stats.name, Rune::Pixie::VSCODE_RED);
    }

    void E9Reporter::on_assertion_begin(const AssertionInfo& assertion_info) {
        SILENCE_UNUSED(assertion_info);
        // Report nothing
    }

    void E9Reporter::on_assertion_end(const AssertionStats& assertion_stats) {
        if (!assertion_stats.result) {
            _e9.set_foreground_color(Rune::Pixie::VSCODE_RED);
            _e9.write_formatted("             FAIL at {}:{}\n",
                                assertion_stats.scl.file.to_c_str(),
                                assertion_stats.scl.line);
            _e9.reset_style();
            _e9.set_foreground_color(Rune::Pixie::VSCODE_CYAN);
            _e9.write_formatted("                       {}\n", assertion_stats.assert.to_c_str());
            _e9.write_formatted("                 With: {}\n",
                                assertion_stats.expanded_assert.to_c_str());
            _e9.reset_style();
        }
    }
} // namespace Heimdall
