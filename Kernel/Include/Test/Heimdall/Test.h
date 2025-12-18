
/*
 *  Copyright 2025 Ewogijk
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.Test
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

#ifndef RUNEOS_TEST_H
#define RUNEOS_TEST_H

#include <Ember/Enum.h>

#include <Test/Heimdall/HString.h>
#include <Test/Heimdall/SourceCodeLocation.h>

namespace Heimdall {
    /// @brief Information about a test.
    struct Test {
        HString            name;
        void               (*test_function)();
        SourceCodeLocation scl;
    };

#define TEST_RESULTS(X)                                                                            \
    X(TestResult, PASS, 0x1)                                                                       \
    X(TestResult, FAIL, 0x2)                                                                       \
    X(TestResult, CONFIG_ERROR, 0x3)

    /// @brief All possible test results.
    ///
    /// PASS: A single or the overall test is pass.<br>
    /// FAIL: A single or the overall test is fail.<br>
    /// CONFIG_ERROR: Heimdall could not be configured.<br>
    DECLARE_ENUM(TestResult, TEST_RESULTS, 0x0) // NOLINT

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                  List Wrapper for Test
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /// @brief A list of tests.
    /// Note: The class is part of the heimdall runtime environment (hre).
    class TestList {
        struct TestListDetail;
        TestListDetail* _list_detail;

      public:
        TestList();
        ~TestList();

        TestList(const TestList& other);
        TestList(TestList&& other) noexcept;
        auto operator=(const TestList& other) -> TestList&;
        auto operator=(TestList&& other) noexcept -> TestList&;

        friend void swap(TestList& fst, TestList& sec) noexcept;

        [[nodiscard]] auto size() const -> size_t;
        void               insert(const Test& test);
        auto               operator[](size_t index) const -> Test;
    };

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                             HashMap Wrapper for test tracking
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /// @brief The test tracker contains all test suites and their tests.
    ///
    /// Note: The class is part of the heimdall runtime environment (hre).
    class TestTracker {
        struct DictDetail;
        DictDetail* _dict_detail;

      public:
        TestTracker();
        ~TestTracker();

        TestTracker(const TestTracker& other);
        TestTracker(TestTracker&& other) noexcept;
        auto operator=(const TestTracker& other) -> TestTracker&;
        auto operator=(TestTracker&& other) noexcept -> TestTracker&;

        friend void swap(TestTracker& fst, TestTracker& sec) noexcept;

        [[nodiscard]] auto keys() const -> HStringList;
        [[nodiscard]] auto find(const HString& test_suite) const -> TestList;
        [[nodiscard]] auto contains(const HString& test_suite) const -> bool;
        void               create_test_suite(const HString& test_suite);
        void               insert_test(const HString& test_suite, const Test& test);
    };

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                  Heimdall Internal API
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /// @brief The TestTracker must be globally accessible for test registration and the execution
    /// engine.
    /// @return A reference to the global test tracker.
    auto get_test_tracker() -> TestTracker&;

    /// @brief
    /// @param name Name of the test.
    /// @param test_suite Name of the test's test suite.
    /// @param test_function Test function.
    /// @return Always true.
    auto register_test(const HString& name,
                       const HString& test_suite,
                       void           (*test_function)(),
                       const char*    source_file,
                       size_t         line) -> bool;
} // namespace Heimdall

#endif // RUNEOS_TEST_H
