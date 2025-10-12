
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

#include <KRE/String.h>
#include <KRE/Collections/HashMap.h>

namespace Heimdall {
    /**
     * Information about a test to be run.
     */
    struct Test {
        Rune::String           name;
        Rune::Function<void()> test_function;
    };

    /**
     * The TestTracker keeps track of all test suites and their associated test cases.
     */
    using TestTracker = Rune::HashMap<Rune::String, Rune::LinkedList<Test>>;

    /**
     * The TestTracker must be globally accessible for test registration and the execution engine.
     *
     * @return A reference to the global test tracker.
     */
    auto get_test_tracker() -> TestTracker&;

    /**
     *
     * @param name Name of the test.
     * @param test_suite Name of the test's test suite.
     * @param test_function Test function.
     * @return Always true.
     */
    auto register_test(const Rune::String&           name,
                       const Rune::String&           test_suite,
                       const Rune::Function<void()>& test_function) -> bool;
}

#endif // RUNEOS_TEST_H
