
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

#ifndef RUNEOS_EXPRESSION_H
#define RUNEOS_EXPRESSION_H

#include <KRE/String.h>

namespace Heimdall {
    /**
     * CRTP base class for expressions.
     *
     * @tparam Derived Type of derived expression.
     */
    template <typename Derived>
    class ExprBase {
        friend Derived; // Allow Derived to call private constructors

        ExprBase()                                   = default;
        ExprBase(const ExprBase&)                    = default;
        auto operator=(const ExprBase&) -> ExprBase& = default;

        ExprBase(ExprBase&&)                    = default;
        auto operator=(ExprBase&&) -> ExprBase& = default;

      public:
        virtual ~ExprBase() = default;

        auto get_result() -> bool { return static_cast<Derived*>(this)->get_result(); }

        auto get_expanded_expr() -> Rune::String {
            return static_cast<Derived*>(this)->get_expanded_expr();
        }
    };

    /**
     * The result of evaluating a binary expression.
     * @tparam LHS LHS type.
     * @tparam RHS RHS type.
     */
    template <typename LHS, typename RHS>
    class BinaryExprEvaluation : public ExprBase<BinaryExprEvaluation<LHS, RHS>> {
        bool         _result{};
        LHS          _lhs;
        Rune::String _op;
        RHS          _rhs;

      public:
        BinaryExprEvaluation(bool result, LHS lhs, Rune::String op, RHS rhs)
            : ExprBase<BinaryExprEvaluation<LHS, RHS>>(),
              _result(result),
              _lhs(lhs),
              _op(op),
              _rhs(rhs) {}

        auto get_result() -> bool { return _result; }

        auto get_expanded_expr() -> Rune::String {
            return Rune::String::format("{} {} {}", _lhs, _op, _rhs);
        }

        auto get_lhs() -> LHS { return _lhs; }

        auto get_rhs() -> RHS { return _rhs; }
    };

    /**
     * A unary or binary expression that can be evaluated.
     *
     * The result of unary expressions is in the 'value' member, binary expressions are evaluated
     * via operator overloading.
     *
     * @tparam LHS_OR_Value A unary expression type or type of the left hand side value of a
     *                      binary expression.
     */
    template <typename LHS_OR_Value>
    class UnaryExpr : public ExprBase<UnaryExpr<LHS_OR_Value>> {
        LHS_OR_Value _value;

      public:
        UnaryExpr(LHS_OR_Value value) : ExprBase<UnaryExpr<LHS_OR_Value>>(), _value(value) {}

        auto get_result() -> bool { return static_cast<bool>(_value); }

        auto get_expanded_expr() -> Rune::String { return Rune::String::format("{}", _value); }

        auto get_value() -> LHS_OR_Value { return _value; }

#define DEFINE_OP_OVERLOAD(op)                                                                     \
    template <typename RHS>                                                                        \
    constexpr friend auto operator op(UnaryExpr lhs, RHS rhs)                                      \
        ->BinaryExprEvaluation<LHS_OR_Value, RHS> {                                                \
        return {static_cast<bool>(lhs._value op rhs), lhs._value, #op, rhs};                       \
    }

        DEFINE_OP_OVERLOAD(==)
        DEFINE_OP_OVERLOAD(!=)
        DEFINE_OP_OVERLOAD(<)
        DEFINE_OP_OVERLOAD(<=)
        DEFINE_OP_OVERLOAD(>)
        DEFINE_OP_OVERLOAD(>=)

#undef DEFINE_OP_OVERLOAD
    };

    /**
     * The Interpreter is the entry point to evaluating expressions given in the REQUIRE macro,
     * essentially it boils down converting the left hand side of an expression to UnaryExpr so that
     * the overloaded operators are used:
     *
     * REQUIRE(a == b) -> Interpreter() << a == b -> UnaryExpr(a) == b -> BinaryExprEvaluation(a ==
     * b, a, "==", b)
     *
     * The '<<' operator was chosen because it has the highest precedence of all non-arithmetic
     * binary operators to ensure it will always be called first.
     *
     * Unary expressions can also be evaluated:
     *
     * REQUIRE(a) -> Interpreter() << a -> UnaryExpr(a)
     */
    struct Interpreter {
        template <typename T>
        constexpr friend auto operator<<(Interpreter d, T value) {
            SILENCE_UNUSED(d)
            return UnaryExpr<T>{value};
        }
    };
} // namespace Heimdall

#endif // RUNEOS_EXPRESSION_H
