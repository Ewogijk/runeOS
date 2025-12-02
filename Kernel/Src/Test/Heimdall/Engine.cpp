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

#include <Test/Heimdall/Engine.h>

#include <Test/Heimdall/HRE.h>
#include <Test/Heimdall/Reporter.h>
#include <Test/Heimdall/TestTracker.h>

/// atexit will be automatically generated when static local variables are declared
/// This function will be provided by the heimdall runtime environment but a forward declaration
/// is needed for the compiler
auto atexit(void (*func)()) -> int;

namespace Heimdall {

    auto Engine::configure(const HStringList& options) -> bool {
        _configuration.options = options;
        hre_configure(_configuration);
        if (_configuration.reporter_registry.is_empty()) {
            hre_emergency_log("ERROR: No reporters have been configured! Aborting test run...\n");
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

    auto Engine::execute(const HStringList& options) -> TestResult {
        if (!configure(options)) return TestResult::CONFIGURATION_FAIL;

        auto&  test_tracker         = get_test_tracker();
        size_t overall_total_tests  = 0;
        size_t overall_tests_passed = 0;
        size_t overall_tests_failed = 0;

        HStringList reporter_names;
        for (size_t i = 0; i < _configuration.reporter_registry.size(); i++)
            reporter_names.insert(_configuration.reporter_registry[i]->get_name());
        TestRunInfo test_run_info{.heimdall_major = 0,
                                  .heimdall_minor = 1,
                                  .heimdall_patch = 0,
                                  .reporter_names = reporter_names};
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
                TestInfo test_info{.name = test.name};
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

        return overall_tests_passed == overall_total_tests ? TestResult::PASS : TestResult::FAIL;
    }

    auto get_engine() -> Engine& {
        static Engine engine;
        return engine;
    }

    auto execute_tests(const HStringList& options) -> TestResult {
        return get_engine().execute(options);
    }
} // namespace Heimdall
