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

#include <Test/Heimdall/AssertionHandler.h>

namespace Heimdall {
    AssertionHandler::AssertionHandler(Engine* engine) : _engine(engine) {}

    auto AssertionHandler::handle_expr(UnaryExpr<bool>           expr,
                                       const HString&            expr_str,
                                       const SourceCodeLocation& scl) -> bool {
        if (_engine->get_current_test_result() == TestResult::FAIL) return false;

        AssertionInfo info{.scl = scl, .assert = expr_str};
        _engine->report_assertion_begin(info);
        bool           result = expr.get_result();
        AssertionStats stats{.scl             = scl,
                             .assert          = HString("REQUIRE(") + expr_str + ")",
                             .expanded_assert = expr.get_expanded_expr(),
                             .result          = result};
        _engine->report_assertion_end(stats);
        return result;
    }

} // namespace Heimdall