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

#include <Test/Heimdall/Test.h>

/// atexit will be automatically generated when static local variables are declared
/// This function will be provided by the heimdall runtime environment but a forward declaration
/// is needed for the compiler
auto atexit(void (*func)()) -> int;

namespace Heimdall {
    DEFINE_ENUM(TestResult, TEST_RESULTS, 0x0)

    auto get_test_tracker() -> TestTracker& {
        static TestTracker test_registry;
        return test_registry;
    }

    auto register_test(const HString& name,
                       const HString& test_suite,
                       void           (*test_function)(),
                       const char*    source_file,
                       size_t         line) -> bool {
        auto& test_tracker = get_test_tracker();

        // All tests must belong to a test suite and a test suite name cannot be the empty string
        // So if a user does not define a test suite we put the test in the "All Tests" test suite
        HString suite = test_suite;
        if (suite.is_empty()) suite = "All Tests";

        // Add test suite if it is missing
        if (!test_tracker.contains(test_suite)) test_tracker.create_test_suite(test_suite);
        test_tracker.insert_test(test_suite,
                                 Test{
                                 .name          = name,
                                 .test_function = test_function,
                                 .scl           = {source_file, line}
        });
        return true;
    }
} // namespace Heimdall
