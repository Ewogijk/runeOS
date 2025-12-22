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

#include <Test/Heimdall/JUnitReporter.h>

namespace Heimdall {
    auto JUnitReporter::get_name() const -> HString { return "JUnitReporter"; }

    void JUnitReporter::on_test_run_begin(const TestRunInfo& test_run_info) {
        _test_report_directory = test_run_info.test_report_directory;
    }

    void JUnitReporter::on_test_run_end(const TestRunStats& test_run_stats) {
        SILENCE_UNUSED(test_run_stats)
        // Save test report to file
        HString xml("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        xml = xml + "<testsuites>\n";
        for (size_t i = 0; i < _test_suites.size(); i++) {
            JUnitTestSuite test_suite = _test_suites[i];
            xml = xml + HString("  <testsuite name=\"") + test_suite.name + "\" tests=\""
                  + HString::number_to_string(test_suite.tests) + "\" failures=\""
                  + HString::number_to_string(test_suite.failures)
                  + "\" errors=\"0\" skipped=\"0\" assertions=\""
                  + HString::number_to_string(test_suite.assertions) + "\" time=\"0\">\n";

            for (size_t j = 0; j < test_suite.test_list.size(); j++) {
                JUnitTest test = test_suite.test_list[j];
                xml            = xml + "    <testcase name=\"" + test.name + "\" assertions=\""
                      + HString::number_to_string(test.assertions) + "\" time=\"0\" file=\""
                      + test.file + "\" line=\"" + HString::number_to_string(test.line) + "\"";

                if (!test.passed) {
                    xml = xml + ">\n      <failure message=\"" + test.message
                          + "\"/>\n    </testcase>\n";
                } else {
                    xml = xml + "/>\n";
                }
            }

            xml = xml + HString("  </testsuite>\n");
        }
        xml = xml + "</testsuites>\n";
        hre_save_to_file(_test_report_directory + "JUnitReport.xml", xml);
    }

    void JUnitReporter::on_test_suite_begin(const TestSuiteInfo& test_suite_info) {
        _c_test_suite           = {};
        _root_test_suite.tests += test_suite_info.total_tests;
        _c_test_suite.name      = test_suite_info.name;
        _c_test_suite.tests     = test_suite_info.total_tests;
    }

    void JUnitReporter::on_test_suite_end(const TestSuiteStats& test_suite_stats) {
        SILENCE_UNUSED(test_suite_stats)
        _test_suites.insert(_c_test_suite);
    }

    void JUnitReporter::on_test_begin(const TestInfo& test_info) {
        _c_test      = {};
        _c_test.name = test_info.name;
        _c_test.file = test_info.file;
        _c_test.line = test_info.line;
    }

    void JUnitReporter::on_test_end(const TestStats& test_stats) {
        SILENCE_UNUSED(test_stats)
        _c_test_suite.test_list.insert(_c_test);
    }

    void JUnitReporter::on_assertion_begin(const AssertionInfo& assertion_info) {
        SILENCE_UNUSED(assertion_info)
        _root_test_suite.assertions++;
        _c_test_suite.assertions++;
        _c_test.assertions++;
    }

    void JUnitReporter::on_assertion_end(const AssertionStats& assertion_stats) {
        if (!assertion_stats.result) {
            _root_test_suite.failures++;
            _c_test_suite.failures++;
            _c_test.message = HString("Failed: ") + assertion_stats.expanded_assert + " with "
                              + assertion_stats.assert + " at " + assertion_stats.scl.file + ":"
                              + HString::number_to_string(assertion_stats.scl.line);
        }
        _c_test.passed = assertion_stats.result;
    }
} // namespace Heimdall
