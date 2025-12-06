
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

#ifndef HEIMDALL_JUNITREPORTER_H
#define HEIMDALL_JUNITREPORTER_H

#include <Test/Heimdall/Reporter.h>

namespace Heimdall {
    struct JUnitTest {
        HString name       = "";
        size_t  assertions = 0;
        HString file       = "";
        size_t  line       = 0;
        bool    passed     = false;
        HString message    = "";
    };

    class JUnitTestList {
        struct JUnitTestListDetail;
        JUnitTestListDetail* _list_detail;

      public:
        JUnitTestList();
        ~JUnitTestList();

        JUnitTestList(const JUnitTestList& other);
        JUnitTestList(JUnitTestList&& other) noexcept;
        auto operator=(const JUnitTestList& other) -> JUnitTestList&;
        auto operator=(JUnitTestList&& other) noexcept -> JUnitTestList&;

        friend void swap(JUnitTestList& fst, JUnitTestList& sec) noexcept;

        auto size() -> size_t;
        void insert(const JUnitTest& test);
        auto operator[](size_t idx) -> JUnitTest;
    };

    struct JUnitTestSuite {
        HString       name       = "";
        size_t        tests      = 0;
        size_t        failures   = 0;
        size_t        assertions = 0;
        JUnitTestList test_list;
    };

    class JUnitTestSuiteList {
        struct JUnitTestSuiteListDetail;
        JUnitTestSuiteListDetail* _list_detail;

      public:
        JUnitTestSuiteList();
        ~JUnitTestSuiteList();

        JUnitTestSuiteList(const JUnitTestSuiteList& other);
        JUnitTestSuiteList(JUnitTestSuiteList&& other) noexcept;
        auto operator=(const JUnitTestSuiteList& other) -> JUnitTestSuiteList&;
        auto operator=(JUnitTestSuiteList&& other) noexcept -> JUnitTestSuiteList&;

        friend void swap(JUnitTestSuiteList& fst, JUnitTestSuiteList& sec) noexcept;

        auto size() -> size_t;
        void insert(const JUnitTestSuite& test_suite);
        auto operator[](size_t idx) -> JUnitTestSuite;
    };

    /// @brief The JUnitReporter saves the test report in the JUnit XML format to the test report
    ///         file.
    class JUnitReporter : public Reporter {
        JUnitTestSuite     _root_test_suite;
        JUnitTestSuiteList _test_suites;

        JUnitTestSuite _c_test_suite;
        JUnitTest      _c_test;

        HString _test_report_file;

      public:
        [[nodiscard]] auto get_name() const -> HString override;
        void               on_test_run_begin(const TestRunInfo& test_run_info) override;
        void               on_test_run_end(const TestRunStats& test_run_stats) override;
        void               on_test_suite_begin(const TestSuiteInfo& test_suite_info) override;
        void               on_test_suite_end(const TestSuiteStats& test_suite_stats) override;
        void               on_test_begin(const TestInfo& test_info) override;
        void               on_test_end(const TestStats& test_stats) override;
        void               on_assertion_begin(const AssertionInfo& assertion_info) override;
        void               on_assertion_end(const AssertionStats& assertion_stats) override;
    };
} // namespace Heimdall

#endif // HEIMDALL_JUNITREPORTER_H
