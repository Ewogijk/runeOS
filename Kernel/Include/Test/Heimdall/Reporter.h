
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

#ifndef RUNEOS_REPORTER_H
#define RUNEOS_REPORTER_H

#include <Ember/Ember.h>

#include <Test/Heimdall/HString.h>
#include <Test/Heimdall/SourceCodeLocation.h>

namespace Heimdall {

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                  Infos and Stats
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /// @brief Info about the starting test run.
    struct TestRunInfo {
        U8          heimdall_major;
        U8          heimdall_minor;
        U8          heimdall_patch;
        HString     hre;
        HStringList options;
        HStringList reporter_names;
    };

    /// @brief The results of executing the test run.
    struct TestRunStats {
        size_t total_tests;
        size_t passed_tests;
        size_t failed_tests;
    };

    /// @brief Info about the starting test.
    struct TestInfo {
        HString name;
    };

    /// @brief The results of executing a test.
    struct TestStats {
        HString name;
        bool    result;
    };

    /// @brief Info about the starting test suite.
    struct TestSuiteInfo {
        HString name;
        size_t  total_tests;
    };

    /// @brief The results of executing the test suite.
    struct TestSuiteStats {
        HString name;
        size_t  total_tests;
        size_t  passed_tests;
        size_t  failed_tests;
    };

    /// @brief Info about the starting assertion.
    struct AssertionInfo {
        SourceCodeLocation scl;
        HString            assert;
    };

    /// @brief The results of evaluating the assertion.
    struct AssertionStats {
        SourceCodeLocation scl;
        HString            assert;
        HString            expanded_assert;
        bool               result;
    };

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      Reporter
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /// @brief The reporter writes the test results to some destination.
    class Reporter {
      public:
        virtual ~Reporter() = default;

        /// @brief
        /// @return The name of the reporter.
        [[nodiscard]] virtual auto get_name() const -> HString = 0;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                   Test Events
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        /// @brief The function will be called by the test engine before the first test case is
        ///         executed.
        /// @param test_run_info
        virtual void on_test_run_begin(const TestRunInfo& test_run_info) = 0;

        /// @brief The function will be called by the test engine after the last test case was
        ///         executed.
        /// @param test_run_stats
        virtual void on_test_run_end(const TestRunStats& test_run_stats) = 0;

        /// @brief The function will be called by the test engine before the first test case of a
        /// test
        ///         suite is executed.
        /// @param test_suite_info
        virtual void on_test_suite_begin(const TestSuiteInfo& test_suite_info) = 0;

        /// @brief The function will be called by the test engine after the last test case of a
        ///         test.
        /// @param test_suite_stats
        virtual void on_test_suite_end(const TestSuiteStats& test_suite_stats) = 0;

        /// @brief The function will be called by the test engine before a test case is executed.
        /// @param test_info
        virtual void on_test_begin(const TestInfo& test_info) = 0;

        /// @brief The function will be called by the test engine after a test case was executed.
        /// @param test_stats
        virtual void on_test_end(const TestStats& test_stats) = 0;

        /// @brief The function will be called by the test engine before an assertion is evaluated.
        /// @param assertion_info
        virtual void on_assertion_begin(const AssertionInfo& assertion_info) = 0;

        /// @brief The function will be called by the test engine after an assertion was evaluated.
        /// @param assertion_stats
        virtual void on_assertion_end(const AssertionStats& assertion_stats) = 0;
    };

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                  Reporter Registry
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /// @brief The reporter registry contains all configured reporters.
    /// Note: The class is part of the heimdall runtime environment (hre).
    class ReporterRegistry {
        struct ReporterListDetail;
        ReporterListDetail* _list_detail;

      public:
        ReporterRegistry();
        ~ReporterRegistry();

        ReporterRegistry(const ReporterRegistry& other);
        ReporterRegistry(ReporterRegistry&& other) noexcept;
        auto operator=(const ReporterRegistry& other) -> ReporterRegistry&;
        auto operator=(ReporterRegistry&& other) noexcept -> ReporterRegistry&;

        friend void swap(ReporterRegistry& fst, ReporterRegistry& sec) noexcept;

        [[nodiscard]] auto is_empty() const -> bool;

        [[nodiscard]] auto size() const -> size_t;

        void insert(Reporter* reporter);

        auto operator[](size_t index) const -> Reporter*;
    };
} // namespace Heimdall

#endif // RUNEOS_REPORTER_H
