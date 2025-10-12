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

#include <Test/Heimdall/TestTracker.h>

namespace Heimdall {
    auto get_test_tracker() -> TestTracker& {
        static TestTracker test_registry;
        return test_registry;
    }

    auto register_test(const Rune::String&           name,
                       const Rune::String&           test_suite,
                       const Rune::Function<void()>& test_function) -> bool {
        auto& test_tracker = get_test_tracker();

        // All tests must belong to a test suite and a test suite name cannot be the empty string
        // So if a user does not define a test suite we put the test in the "All Tests" test suite
        Rune::String suite = test_suite;
        if (suite.is_empty()) suite = "All Tests";

        // Add test suite if it is missing
        auto maybe_suite = test_tracker.find(test_suite);
        if (maybe_suite == test_tracker.end()) test_tracker[suite] = Rune::LinkedList<Test>();

        test_tracker[suite].add_back(Test{.name = name, .test_function = test_function});
        return true;
    }
} // namespace Heimdall
