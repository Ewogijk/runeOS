
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

#include <Test/Heimdall/SourceCodeLocation.h>

#include <KRE/String.h>
#include <KRE/Memory.h>
#include <KRE/Collections/LinkedList.h>

#include <CPU/E9Stream.h>

namespace Heimdall {

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                  Infos and Stats
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * Info about the starting test run.
     */
    struct TestRunInfo {
        U8 heimdall_major;
        U8 heimdall_minor;
        U8 heimdall_patch;
        Rune::LinkedList<Rune::String> reporter_names;
    };

    /**
     * The results of executing the test run.
     */
    struct TestRunStats {
        size_t total_tests;
        size_t passed_tests;
        size_t failed_tests;
    };

    /**
     * Info about the starting test.
     */
    struct TestInfo {
        Rune::String name;
    };

    /**
     * The results of executing a test.
     */
    struct TestStats {
        Rune::String name;
        bool         result;
    };

    /**
     * Info about the starting test suite.
     */
    struct TestSuiteInfo {
        Rune::String name;
        size_t       total_tests;
    };

    /**
     * The results of executing the test suite.
     */
    struct TestSuiteStats {
        Rune::String name;
        size_t       total_tests;
        size_t       passed_tests;
        size_t       failed_tests;
    };

    /**
     * Info about the starting assertion.
     */
    struct AssertionInfo {
        SourceCodeLocation scl;
        Rune::String       assert;
    };

    /**
     * The results of evaluating the assertion.
     */
    struct AssertionStats {
        SourceCodeLocation scl;
        Rune::String       assert;
        Rune::String       expanded_assert;
        bool               result;
    };

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      Reporter
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * The reporter writes the test results to some destination.
     */
    class Reporter {
      public:
        virtual ~Reporter() = default;

        /**
         *
         * @return The name of the reporter.
         */
        [[nodiscard]] virtual auto get_name() const -> Rune::String = 0;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                   Test Events
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        /**
         * The function will be called by the test engine before the first test case is
         * executed.
         * @param test_run_info
         */
        virtual void on_test_run_begin(const TestRunInfo& test_run_info) = 0;

        /**
         * The function will be called by the test engine after the last test case was
         * executed.
         * @param test_run_stats
         */
        virtual void on_test_run_end(const TestRunStats& test_run_stats) = 0;

        /**
         * The function will be called by the test engine before the first test case of a test
         * suite is executed.
         * @param test_suite_info
         */
        virtual void on_test_suite_begin(const TestSuiteInfo& test_suite_info) = 0;

        /**
         * The function will be called by the test engine after the last test case of a test
         * suite was executed.
         * @param test_suite_stats
         */
        virtual void on_test_suite_end(const TestSuiteStats& test_suite_stats) = 0;

        /**
         * The function will be called by the test engine before a test case is executed.
         * @param test_info
         */
        virtual void on_test_begin(const TestInfo& test_info) = 0;

        /**
         * The function will be called by the test engine after a test case was executed.
         * @param test_stats
         */
        virtual void on_test_end(const TestStats& test_stats) = 0;

        /**
         * The function will be called by the test engine before an assertion is evaluated.
         * @param assertion_info
         */
        virtual void on_assertion_begin(const AssertionInfo& assertion_info) = 0;

        /**
         * The function will be called by the test engine after an assertion was evaluated.
         * @param assertion_stats
         */
        virtual void on_assertion_end(const AssertionStats& assertion_stats) = 0;
    };

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      E9 Reporter
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * Reports on the E9 port so that Qemu can forward the test results to the console of the
     * host machine.
     */
    class E9Reporter : public Reporter {
        static constexpr size_t TAG_WIDTH = 10;
        Rune::CPU::E9Stream     _e9{};

        void write_tag(const Rune::String& tag, const Rune::String& text);

        void
        write_colored_tag(const Rune::String& tag, const Rune::String& text, Rune::Pixel pixel);

        void write_divider(char div_char, const Rune::String& text);

      public:
        E9Reporter() = default;

        [[nodiscard]] auto get_name() const -> Rune::String override;
        void on_test_run_begin(const TestRunInfo& test_run_info) override;
        void on_test_run_end(const TestRunStats& test_run_stats) override;
        void on_test_suite_begin(const TestSuiteInfo& test_suite_info) override;
        void on_test_suite_end(const TestSuiteStats& test_suite_stats) override;
        void on_test_begin(const TestInfo& test_info) override;
        void on_test_end(const TestStats& test_stats) override;
        void on_assertion_begin(const AssertionInfo& assertion_info) override;
        void on_assertion_end(const AssertionStats& assertion_stats) override;
    };

} // namespace Heimdall

#endif // RUNEOS_REPORTER_H
