// Copyright 2021 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/tint/resolver/const_eval_test.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

// Bring in std::ostream& operator<<(std::ostream& o, const Types& types)
using resolver::operator<<;

struct Case {
    Case(utils::VectorRef<Types> in_args, Types in_expected)
        : args(std::move(in_args)), expected(std::move(in_expected)) {}

    /// Expected value may be positive or negative
    Case& PosOrNeg() {
        expected_pos_or_neg = true;
        return *this;
    }

    /// Expected value should be compared using FLOAT_EQ instead of EQ
    Case& FloatComp() {
        float_compare = true;
        return *this;
    }

    utils::Vector<Types, 8> args;
    Types expected;
    bool expected_pos_or_neg = false;
    bool float_compare = false;
};

static std::ostream& operator<<(std::ostream& o, const Case& c) {
    o << "args: ";
    for (auto& a : c.args) {
        o << a << ", ";
    }
    o << "expected: " << c.expected << ", expected_pos_or_neg: " << c.expected_pos_or_neg;
    return o;
}

/// Creates a Case with Values for args and result
static Case C(std::initializer_list<Types> args, Types result) {
    return Case{utils::Vector<Types, 8>{args}, std::move(result)};
}

/// Convenience overload that creates a Case with just scalars
using ScalarTypes = std::variant<AInt, AFloat, u32, i32, f32, f16>;
static Case C(std::initializer_list<ScalarTypes> sargs, ScalarTypes sresult) {
    utils::Vector<Types, 8> args;
    for (auto& sa : sargs) {
        std::visit([&](auto&& v) { return args.Push(Val(v)); }, sa);
    }
    Types result = Val(0_a);
    std::visit([&](auto&& v) { result = Val(v); }, sresult);
    return Case{std::move(args), std::move(result)};
}

using ResolverConstEvalBuiltinTest = ResolverTestWithParam<std::tuple<sem::BuiltinType, Case>>;

TEST_P(ResolverConstEvalBuiltinTest, Test) {
    Enable(ast::Extension::kF16);

    auto builtin = std::get<0>(GetParam());
    auto& c = std::get<1>(GetParam());

    utils::Vector<const ast::Expression*, 8> args;
    for (auto& a : c.args) {
        std::visit([&](auto&& v) { args.Push(v.Expr(*this)); }, a);
    }

    std::visit(
        [&](auto&& expected) {
            using T = typename std::decay_t<decltype(expected)>::ElementType;
            auto* expr = Call(sem::str(builtin), std::move(args));

            GlobalConst("C", expr);
            auto* expected_expr = expected.Expr(*this);
            GlobalConst("E", expected_expr);

            EXPECT_TRUE(r()->Resolve()) << r()->error();

            auto* sem = Sem().Get(expr);
            const sem::Constant* value = sem->ConstantValue();
            ASSERT_NE(value, nullptr);
            EXPECT_TYPE(value->Type(), sem->Type());

            auto* expected_sem = Sem().Get(expected_expr);
            const sem::Constant* expected_value = expected_sem->ConstantValue();
            ASSERT_NE(expected_value, nullptr);
            EXPECT_TYPE(expected_value->Type(), expected_sem->Type());

            ForEachElemPair(value, expected_value,
                            [&](const sem::Constant* a, const sem::Constant* b) {
                                auto v = a->As<T>();
                                auto e = b->As<T>();
                                if constexpr (std::is_same_v<bool, T>) {
                                    EXPECT_EQ(v, e);
                                } else if constexpr (IsFloatingPoint<T>) {
                                    if (std::isnan(e)) {
                                        EXPECT_TRUE(std::isnan(v));
                                    } else {
                                        auto vf = (c.expected_pos_or_neg ? Abs(v) : v);
                                        if (c.float_compare) {
                                            EXPECT_FLOAT_EQ(vf, e);
                                        } else {
                                            EXPECT_EQ(vf, e);
                                        }
                                    }
                                } else {
                                    EXPECT_EQ((c.expected_pos_or_neg ? Abs(v) : v), e);
                                    // Check that the constant's integer doesn't contain unexpected
                                    // data in the MSBs that are outside of the bit-width of T.
                                    EXPECT_EQ(a->As<AInt>(), b->As<AInt>());
                                }
                                return HasFailure() ? Action::kStop : Action::kContinue;
                            });
        },
        c.expected);
}

INSTANTIATE_TEST_SUITE_P(  //
    MixedAbstractArgs,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kAtan2),
                     testing::ValuesIn(std::vector{
                         C({0_a, -0.0_a}, kPi<AFloat>),
                         C({1.0_a, 0_a}, kPiOver2<AFloat>),
                     })));

template <typename T, bool finite_only>
std::vector<Case> Atan2Cases() {
    std::vector<Case> cases = {
        // If y is +/-0 and x is negative or -0, +/-PI is returned
        C({T(0.0), -T(0.0)}, kPi<T>).PosOrNeg().FloatComp(),

        // If y is +/-0 and x is positive or +0, +/-0 is returned
        C({T(0.0), T(0.0)}, T(0.0)).PosOrNeg(),

        // If x is +/-0 and y is negative, -PI/2 is returned
        C({-T(1.0), T(0.0)}, -kPiOver2<T>).FloatComp(),  //
        C({-T(1.0), -T(0.0)}, -kPiOver2<T>).FloatComp(),

        // If x is +/-0 and y is positive, +PI/2 is returned
        C({T(1.0), T(0.0)}, kPiOver2<T>).FloatComp(),  //
        C({T(1.0), -T(0.0)}, kPiOver2<T>).FloatComp(),

        // Vector tests
        C({Vec(T(0.0), T(0.0)), Vec(-T(0.0), T(0.0))}, Vec(kPi<T>, T(0.0))).PosOrNeg().FloatComp(),
        C({Vec(-T(1.0), -T(1.0)), Vec(T(0.0), -T(0.0))}, Vec(-kPiOver2<T>, -kPiOver2<T>))
            .FloatComp(),
        C({Vec(T(1.0), T(1.0)), Vec(T(0.0), -T(0.0))}, Vec(kPiOver2<T>, kPiOver2<T>)).FloatComp(),
    };

    if constexpr (!finite_only) {
        std::vector<Case> non_finite_cases = {
            // If y is +/-INF and x is finite, +/-PI/2 is returned
            C({T::Inf(), T(0.0)}, kPiOver2<T>).PosOrNeg().FloatComp(),
            C({-T::Inf(), T(0.0)}, kPiOver2<T>).PosOrNeg().FloatComp(),

            // If y is +/-INF and x is -INF, +/-3PI/4 is returned
            C({T::Inf(), -T::Inf()}, k3PiOver4<T>).PosOrNeg().FloatComp(),
            C({-T::Inf(), -T::Inf()}, k3PiOver4<T>).PosOrNeg().FloatComp(),

            // If y is +/-INF and x is +INF, +/-PI/4 is returned
            C({T::Inf(), T::Inf()}, kPiOver4<T>).PosOrNeg().FloatComp(),
            C({-T::Inf(), T::Inf()}, kPiOver4<T>).PosOrNeg().FloatComp(),

            // If x is -INF and y is finite and positive, +PI is returned
            C({T(0.0), -T::Inf()}, kPi<T>).FloatComp(),

            // If x is -INF and y is finite and negative, -PI is returned
            C({-T(0.0), -T::Inf()}, -kPi<T>).FloatComp(),

            // If x is +INF and y is finite and positive, +0 is returned
            C({T(0.0), T::Inf()}, T(0.0)),

            // If x is +INF and y is finite and negative, -0 is returned
            C({-T(0.0), T::Inf()}, -T(0.0)),

            // If either x is NaN or y is NaN, NaN is returned
            C({T::NaN(), T(0.0)}, T::NaN()),
            C({T(0.0), T::NaN()}, T::NaN()),
            C({T::NaN(), T::NaN()}, T::NaN()),

            // Vector tests
            C({Vec(T::Inf(), -T::Inf(), T::Inf(), -T::Inf()),  //
               Vec(T(0.0), T(0.0), -T::Inf(), -T::Inf())},     //
              Vec(kPiOver2<T>, kPiOver2<T>, k3PiOver4<T>, k3PiOver4<T>))
                .PosOrNeg()
                .FloatComp(),
        };
        cases = Concat(cases, non_finite_cases);
    }

    return cases;
}
INSTANTIATE_TEST_SUITE_P(  //
    Atan2,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kAtan2),
                     testing::ValuesIn(Concat(Atan2Cases<AFloat, true>(),  //
                                              Atan2Cases<f32, false>(),
                                              Atan2Cases<f16, false>()))));

template <typename T>
std::vector<Case> ClampCases() {
    return {
        C({T(0), T(0), T(0)}, T(0)),
        C({T(0), T(42), T::Highest()}, T(42)),
        C({T::Lowest(), T(0), T(42)}, T(0)),
        C({T(0), T::Lowest(), T::Highest()}, T(0)),
        C({T(0), T::Highest(), T::Lowest()}, T::Lowest()),
        C({T::Highest(), T::Highest(), T::Highest()}, T::Highest()),
        C({T::Lowest(), T::Lowest(), T::Lowest()}, T::Lowest()),
        C({T::Highest(), T::Lowest(), T::Highest()}, T::Highest()),
        C({T::Lowest(), T::Lowest(), T::Highest()}, T::Lowest()),

        // Vector tests
        C({Vec(T(0), T(0)),                         //
           Vec(T(0), T(42)),                        //
           Vec(T(0), T::Highest())},                //
          Vec(T(0), T(42))),                        //
        C({Vec(T::Lowest(), T(0), T(0)),            //
           Vec(T(0), T::Lowest(), T::Highest()),    //
           Vec(T(42), T::Highest(), T::Lowest())},  //
          Vec(T(0), T(0), T::Lowest())),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Clamp,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kClamp),
                     testing::ValuesIn(Concat(ClampCases<AInt>(),  //
                                              ClampCases<i32>(),
                                              ClampCases<u32>(),
                                              ClampCases<AFloat>(),
                                              ClampCases<f32>(),
                                              ClampCases<f16>()))));

template <typename T>
std::vector<Case> SelectCases() {
    return {
        C({Val(T{1}), Val(T{2}), Val(false)}, Val(T{1})),
        C({Val(T{1}), Val(T{2}), Val(true)}, Val(T{2})),

        C({Val(T{2}), Val(T{1}), Val(false)}, Val(T{2})),
        C({Val(T{2}), Val(T{1}), Val(true)}, Val(T{1})),

        C({Vec(T{1}, T{2}), Vec(T{3}, T{4}), Vec(false, false)}, Vec(T{1}, T{2})),
        C({Vec(T{1}, T{2}), Vec(T{3}, T{4}), Vec(false, true)}, Vec(T{1}, T{4})),
        C({Vec(T{1}, T{2}), Vec(T{3}, T{4}), Vec(true, false)}, Vec(T{3}, T{2})),
        C({Vec(T{1}, T{2}), Vec(T{3}, T{4}), Vec(true, true)}, Vec(T{3}, T{4})),

        C({Vec(T{1}, T{1}, T{2}, T{2}),     //
           Vec(T{2}, T{2}, T{1}, T{1}),     //
           Vec(false, true, false, true)},  //
          Vec(T{1}, T{2}, T{2}, T{1})),     //
    };
}
static std::vector<Case> SelectBoolCases() {
    return {
        C({Val(true), Val(false), Val(false)}, Val(true)),
        C({Val(true), Val(false), Val(true)}, Val(false)),

        C({Val(false), Val(true), Val(true)}, Val(true)),
        C({Val(false), Val(true), Val(false)}, Val(false)),

        C({Vec(true, true, false, false),   //
           Vec(false, false, true, true),   //
           Vec(false, true, true, false)},  //
          Vec(true, false, true, false)),   //
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Select,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kSelect),
                     testing::ValuesIn(Concat(SelectCases<AInt>(),  //
                                              SelectCases<i32>(),
                                              SelectCases<u32>(),
                                              SelectCases<AFloat>(),
                                              SelectCases<f32>(),
                                              SelectCases<f16>(),
                                              SelectBoolCases()))));

}  // namespace
}  // namespace tint::resolver
