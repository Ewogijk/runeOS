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
#include <Test/Heimdall/GnomeReporter.h>
#include <Test/Heimdall/Engine.h>
#include <Test/Heimdall/HRE.h>
#include <Test/Heimdall/JUnitReporter.h>
#include <Test/Heimdall/Reporter.h>
#include <Test/Heimdall/Test.h>

/// atexit will be automatically generated when static local variables are declared
/// This function will be provided by the heimdall runtime environment but a forward declaration
/// is needed for the compiler
auto atexit(void (*func)()) -> int;

namespace Heimdall {

    HString Engine::CONSOLE_REPORTER      = "console-reporter";
    HString Engine::JUNIT_REPORTER        = "junit-reporter";
    HString Engine::GNOME_REPORTER        = "gnome-reporter";
    HString Engine::TEST_REPORT_DIRECTORY = "test-report-directory";

    auto Engine::configure(const OptionList& options) -> bool {
        _configuration.options        = options;
        bool has_test_report_location = false;
        for (size_t i = 0; i < _configuration.options.size(); i++) {
            Option opt = _configuration.options[i];
            if (opt.name == CONSOLE_REPORTER)
                _configuration.reporter_registry.insert(new ConsoleReporter());
            if (opt.name == JUNIT_REPORTER)
                _configuration.reporter_registry.insert(new JUnitReporter());
            if (opt.name == GNOME_REPORTER)
                _configuration.reporter_registry.insert(new GnomeReporter());
            if (opt.name == TEST_REPORT_DIRECTORY) has_test_report_location = true;
        }
        if (_configuration.reporter_registry.is_empty()) {
            hre_log_emergency("ERROR: No reporters have been configured! Aborting test run...\n");
            return false;
        }
        if (!has_test_report_location) {
            hre_log_emergency("ERROR: Missing test report location option.");
            return false;
        }
        return true;
    }

    auto Engine::get_current_test_result() const -> TestResult { return _test_result; }

    void Engine::report_assertion_begin(const AssertionInfo& assert_info) {
        for (size_t i = 0; i < _configuration.reporter_registry.size(); i++)
            _configuration.reporter_registry[i]->on_assertion_begin(assert_info);
    }

    void Engine::report_assertion_end(const AssertionStats& assert_stats) {
        for (size_t i = 0; i < _configuration.reporter_registry.size(); i++)
            _configuration.reporter_registry[i]->on_assertion_end(assert_stats);
        _test_result = assert_stats.result ? TestResult::PASS : TestResult::FAIL;
    }

    void Engine::execute(const OptionList& options) {
        if (!configure(options)) return;

        auto&   test_tracker         = get_test_tracker();
        size_t  overall_total_tests  = 0;
        size_t  overall_tests_passed = 0;
        size_t  overall_tests_failed = 0;
        HString test_report_directory;

        HStringList reporter_names;
        for (size_t i = 0; i < _configuration.reporter_registry.size(); i++)
            reporter_names.insert(_configuration.reporter_registry[i]->get_name());
        HStringList str_options;
        for (size_t i = 0; i < _configuration.options.size(); i++) {
            if (_configuration.options[i].value.is_empty()) {
                str_options.insert(_configuration.options[i].name);
            } else {
                str_options.insert(_configuration.options[i].name + "="
                                   + _configuration.options[i].value);
                if (_configuration.options[i].name == TEST_REPORT_DIRECTORY)
                    test_report_directory = _configuration.options[i].value;
            }
        }

        TestRunInfo test_run_info{
            .heimdall_major        = 0,
            .heimdall_minor        = 1,
            .heimdall_patch        = 0,
            .hre                   = hre_get_runtime_name(),
            .options               = str_options,
            .reporter_names        = reporter_names,
            .test_report_directory = test_report_directory,
        };
        for (size_t i = 0; i < _configuration.reporter_registry.size(); i++)
            _configuration.reporter_registry[i]->on_test_run_begin(test_run_info);

        HStringList test_suites = test_tracker.keys();
        for (size_t i = 0; i < test_suites.size(); i++) {
            auto   suite_name   = test_suites[i];
            auto   tests        = test_tracker.find(suite_name);
            size_t total_tests  = tests.size();
            size_t passed_tests = 0;
            size_t failed_tests = 0;

            TestSuiteInfo test_suite_info{.name = suite_name, .total_tests = total_tests};
            for (size_t i = 0; i < _configuration.reporter_registry.size(); i++)
                _configuration.reporter_registry[i]->on_test_suite_begin(test_suite_info);

            for (size_t i = 0; i < tests.size(); i++) {
                auto     test = tests[i];
                TestInfo test_info{.name = test.name, .file = test.scl.file, .line = test.scl.line};
                for (size_t i = 0; i < _configuration.reporter_registry.size(); i++)
                    _configuration.reporter_registry[i]->on_test_begin(test_info);

                test.test_function();

                TestStats test_stats{.name = test.name, .result = _test_result == TestResult::PASS};
                for (size_t i = 0; i < _configuration.reporter_registry.size(); i++)
                    _configuration.reporter_registry[i]->on_test_end(test_stats);

                if (_test_result == TestResult::PASS)
                    passed_tests++;
                else
                    failed_tests++;

                _test_result = TestResult::PASS;
            }

            TestSuiteStats test_suite_stats{.name         = suite_name,
                                            .total_tests  = total_tests,
                                            .passed_tests = passed_tests,
                                            .failed_tests = failed_tests};
            for (size_t i = 0; i < _configuration.reporter_registry.size(); i++)
                _configuration.reporter_registry[i]->on_test_suite_end(test_suite_stats);

            overall_total_tests  += total_tests;
            overall_tests_passed += passed_tests;
            overall_tests_failed += failed_tests;
        }

        TestRunStats test_run_stats{.total_tests  = overall_total_tests,
                                    .passed_tests = overall_tests_passed,
                                    .failed_tests = overall_tests_failed};
        for (size_t i = 0; i < _configuration.reporter_registry.size(); i++)
            _configuration.reporter_registry[i]->on_test_run_end(test_run_stats);
    }

    auto get_engine() -> Engine& {
        static Engine engine;
        return engine;
    }

    void execute_tests(const OptionList& options) { get_engine().execute(options); }
} // namespace Heimdall
