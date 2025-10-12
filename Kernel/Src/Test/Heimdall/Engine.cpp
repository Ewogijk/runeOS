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

#include <Test/Heimdall/Reporter.h>
#include <Test/Heimdall/TestTracker.h>
#include <Test/Heimdall/UserConfig.h>

namespace Heimdall {

    auto Engine::configure() -> bool {
#ifdef E9_REPORTER
        _reporter_registry.add_back(
                Rune::UniquePointer<Heimdall::Reporter>(new Heimdall::E9Reporter()));
#endif
        if (_reporter_registry.is_empty()) {
            Rune::CPU::E9Stream e9;
            e9.set_foreground_color(Rune::Pixie::VSCODE_RED);
            e9.write_formatted("ERROR: No reporters have been configured!\n");
            e9.reset_style();
            return false;
        }
        return true;
    }

    auto Engine::get_current_test_result() const -> TestResult {
        return _test_result;
    }


    void Engine::report_assertion_begin(const AssertionInfo& assert_info) {
        for (auto& reporter : _reporter_registry) reporter->on_assertion_begin(assert_info);
    }

    void Engine::report_assertion_end(const AssertionStats& assert_stats) {
        for (auto& reporter : _reporter_registry) reporter->on_assertion_end(assert_stats);
        _test_result = assert_stats.result ? TestResult::PASS : TestResult::FAIL;
    }

    void Engine::execute() {
        if (!configure()) return;

        auto&               test_tracker = get_test_tracker();
        Rune::CPU::E9Stream e9;
        size_t              overall_total_tests  = 0;
        size_t              overall_tests_passed = 0;
        size_t              overall_tests_failed = 0;

        Rune::LinkedList<Rune::String> reporter_names;
        for (auto& reporter : _reporter_registry) reporter_names.add_back(reporter->get_name());
        TestRunInfo test_run_info{.heimdall_major = 0,
                                  .heimdall_minor = 1,
                                  .heimdall_patch = 0,
                                  .reporter_names = Rune::move(reporter_names)};
        for (auto& reporter : _reporter_registry) reporter->on_test_run_begin(test_run_info);

        for (const auto& test_suite : test_tracker) {
            auto   suite_name   = *test_suite.key;
            auto   tests        = *test_suite.value;
            size_t total_tests  = tests.size();
            size_t passed_tests = 0;
            size_t failed_tests = 0;

            TestSuiteInfo test_suite_info{.name = suite_name, .total_tests = total_tests};
            for (auto& reporter : _reporter_registry)
                reporter->on_test_suite_begin(test_suite_info);

            for (auto& test : tests) {

                TestInfo test_info{.name = test.name};
                for (auto& reporter : _reporter_registry) reporter->on_test_begin(test_info);

                test.test_function();

                TestStats test_stats{.name = test.name, .result = _test_result == TestResult::PASS};
                for (auto& reporter : _reporter_registry) reporter->on_test_end(test_stats);

                if (_test_result == TestResult::PASS)
                    passed_tests++;
                else
                    failed_tests++;
            }

            TestSuiteStats test_suite_stats{.name         = suite_name,
                                            .total_tests  = total_tests,
                                            .passed_tests = passed_tests,
                                            .failed_tests = failed_tests};
            for (auto& reporter : _reporter_registry) reporter->on_test_suite_end(test_suite_stats);

            overall_total_tests  += total_tests;
            overall_tests_passed += passed_tests;
            overall_tests_failed += failed_tests;
        }

        TestRunStats test_run_stats{.total_tests  = overall_total_tests,
                                    .passed_tests = overall_tests_passed,
                                    .failed_tests = overall_tests_failed};
        for (auto& reporter : _reporter_registry) reporter->on_test_run_end(test_run_stats);
    }

    auto get_engine() -> Engine& {
        static Engine engine;
        return engine;
    }

    void execute_tests() { get_engine().execute(); }
} // namespace Heimdall
