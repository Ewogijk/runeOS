
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

#ifndef RUNEOS_MACROS_H
#define RUNEOS_MACROS_H

#include <Test/Heimdall/AssertionHandler.h>
#include <Test/Heimdall/TestTracker.h>

namespace Heimdall {
#define ACTUAL_CONCAT(a, b) a##b
#define CONCAT(a, b)        ACTUAL_CONCAT(a, b)

#define TEST_WITH_SUITE(name, test_suite)                                                          \
    namespace CONCAT(Test_, __LINE__) {                                                            \
        void       test_function();                                                                \
        const bool CONCAT(reg_test_,                                                               \
                          __LINE__) = Heimdall::register_test(name, test_suite, &test_function);   \
    };                                                                                             \
    void CONCAT(Test_, __LINE__)::test_function()

#define TEST_NO_SUITE(name)                                                                        \
    namespace CONCAT(Test_, __LINE__) {                                                            \
        void       test_function();                                                                \
        const bool CONCAT(reg_test_,                                                               \
                          __LINE__) = Heimdall::register_test(name, "", &test_function);           \
    };                                                                                             \
    void CONCAT(Test_, __LINE__)::test_function()

#define DELEGATE_TEST(_1, _2, name, ...) name

    /// @brief
    /// Define a test case in an optional test suite. If no test suite is declared the test will
    /// automatically be assigned to the "All Tests" test suite.
    ///
    /// Usage:
    /// TEST("My Test", "My Testsuite") {
    ///      // test code goes here
    /// }
    ///
    /// OR
    ///
    /// TEST("My Test") {
    ///      // test code goes here
    /// }
#define TEST(...) DELEGATE_TEST(__VA_ARGS__, TEST_WITH_SUITE, TEST_NO_SUITE)(__VA_ARGS__)

    /// @brief
    /// Define an expression that will be evaluated and reported. If the expression fails, the test
    /// will be aborted.
    ///
    /// Usage:
    ///
    /// REQUIRE ( expression )
    ///
    /// Examples:
    ///
    ///  REQUIRE( sum(1, 1) == 2 )
    ///
    ///  REQUIRE( !object.some_property() )
    ///
#define REQUIRE(...)                                                                               \
    {                                                                                              \
        Heimdall::AssertionHandler assert_handler(&Heimdall::get_engine());                        \
        if (!assert_handler.handle_expr(Heimdall::Interpreter() << __VA_ARGS__,                    \
                                        #__VA_ARGS__,                                              \
                                        {__FILE__, __LINE__})) {                                   \
            return;                                                                                \
        }                                                                                          \
    }
} // namespace Heimdall

#endif // RUNEOS_MACROS_H
