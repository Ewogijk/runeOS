
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

#ifndef RUNEOS_ASSERTIONHANDLER_H
#define RUNEOS_ASSERTIONHANDLER_H

#include <Test/Heimdall/Engine.h>
#include <Test/Heimdall/Expression.h>
#include <Test/Heimdall/SourceCodeLocation.h>

namespace Heimdall {

    /// @brief The assertion handler parses, evaluates and reports the result of the expression
    ///         inside a REQUIRE statement.
    class AssertionHandler {
        Engine* _engine;

      public:
        explicit AssertionHandler(Engine* engine);

        /// @brief Evaluate a binary expression.
        /// @tparam LHS
        /// @tparam RHS
        /// @param expr Binary expression inside the REQUIRE.
        /// @param expr_str String representation of the REQUIRE.
        /// @param scl File name and code line of the REQUIRE.
        /// @return Boolean result of evaluating the expression.
        template <typename LHS, typename RHS>
        auto handle_expr(BinaryExprEvaluation<LHS, RHS> expr,
                         const HString&                 expr_str,
                         const SourceCodeLocation&      scl) -> bool {
            if (_engine->get_current_test_result() == TestResult::FAIL) return false;

            AssertionInfo info{.scl = scl, .assert = expr_str};
            _engine->report_assertion_begin(info);

            bool result = expr.get_result();

            AssertionStats stats{.scl             = scl,
                                 .assert          = HString("REQUIRE(") + expr_str + ")",
                                 .expanded_assert = expr.get_expanded_expr(),
                                 .result          = result};
            _engine->report_assertion_end(stats);
            return result;
        }

        /// @brief Evaluate an unary expression.
        /// @param expr Unary expression inside the REQUIRE.
        /// @param expr_str String representation of the REQUIRE.
        /// @param scl File name and code line of the REQUIRE.
        /// @return Boolean result of evaluating the expression.
        auto handle_expr(UnaryExpr<bool>           expr,
                         const HString&            expr_str,
                         const SourceCodeLocation& scl) -> bool;
    };
} // namespace Heimdall

#endif // RUNEOS_ASSERTIONHANDLER_H
