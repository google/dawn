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

    auto* expected = ToValueBase(c.expected);
    auto* expr = Call(sem::str(builtin), std::move(args));

    GlobalConst("C", expr);
    auto* expected_expr = expected->Expr(*this);
    GlobalConst("E", expected_expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    const sem::Constant* value = sem->ConstantValue();
    ASSERT_NE(value, nullptr);
    EXPECT_TYPE(value->Type(), sem->Type());

    auto* expected_sem = Sem().Get(expected_expr);
    const sem::Constant* expected_value = expected_sem->ConstantValue();
    ASSERT_NE(expected_value, nullptr);
    EXPECT_TYPE(expected_value->Type(), expected_sem->Type());

    // @TODO(amaiorano): Rewrite using ScalarArgsFrom()
    ForEachElemPair(value, expected_value, [&](const sem::Constant* a, const sem::Constant* b) {
        std::visit(
            [&](auto&& ct_expected) {
                using T = typename std::decay_t<decltype(ct_expected)>::ElementType;

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
            },
            c.expected);

        return HasFailure() ? Action::kStop : Action::kContinue;
    });
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

    ConcatIntoIf<!finite_only>(  //
        cases, std::vector<Case>{
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
               });

    return cases;
}
INSTANTIATE_TEST_SUITE_P(  //
    Atan2,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kAtan2),
                     testing::ValuesIn(Concat(Atan2Cases<AFloat, true>(),  //
                                              Atan2Cases<f32, false>(),
                                              Atan2Cases<f16, false>()))));

template <typename T, bool finite_only>
std::vector<Case> AtanCases() {
    std::vector<Case> cases = {
        C({T(1.0)}, kPiOver4<T>).FloatComp(),
        C({-T(1.0)}, -kPiOver4<T>).FloatComp(),

        // If i is +/-0, +/-0 is returned
        C({T(0.0)}, T(0.0)).PosOrNeg(),

        // Vector tests
        C({Vec(T(0.0), T(1.0), -T(1.0))}, Vec(T(0.0), kPiOver4<T>, -kPiOver4<T>)).FloatComp(),
    };

    ConcatIntoIf<!finite_only>(  //
        cases, std::vector<Case>{
                   // If i is +/-INF, +/-PI/2 is returned
                   C({T::Inf()}, kPiOver2<T>).PosOrNeg().FloatComp(),
                   C({-T::Inf()}, -kPiOver2<T>).FloatComp(),

                   // If i is NaN, NaN is returned
                   C({T::NaN()}, T::NaN()),

                   // Vector tests
                   C({Vec(T::Inf(), -T::Inf(), T::Inf(), -T::Inf())},  //
                     Vec(kPiOver2<T>, -kPiOver2<T>, kPiOver2<T>, -kPiOver2<T>))
                       .FloatComp(),
               });

    return cases;
}
INSTANTIATE_TEST_SUITE_P(  //
    Atan,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kAtan),
                     testing::ValuesIn(Concat(AtanCases<AFloat, true>(),  //
                                              AtanCases<f32, false>(),
                                              AtanCases<f16, false>()))));

template <typename T, bool finite_only>
std::vector<Case> AtanhCases() {
    std::vector<Case> cases = {
        // If i is +/-0, +/-0 is returned
        C({T(0.0)}, T(0.0)).PosOrNeg(),

        C({T(0.9)}, T(1.4722193)).FloatComp(),

        // Vector tests
        C({Vec(T(0.0), T(0.9), -T(0.9))}, Vec(T(0.0), T(1.4722193), -T(1.4722193))).FloatComp(),
    };

    ConcatIntoIf<!finite_only>(  //
        cases, std::vector<Case>{
                   // If i is NaN, NaN is returned
                   C({T::NaN()}, T::NaN()),

                   // Vector tests
                   C({Vec(T::NaN(), T::NaN())}, Vec(T::NaN(), T::NaN())).FloatComp(),
               });

    return cases;
}
INSTANTIATE_TEST_SUITE_P(  //
    Atanh,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kAtanh),
                     testing::ValuesIn(Concat(AtanhCases<AFloat, true>(),  //
                                              AtanhCases<f32, false>(),
                                              AtanhCases<f16, false>()))));

TEST_F(ResolverConstEvalBuiltinTest, Atanh_OutsideRange_Positive) {
    auto* expr = Call(Source{{12, 24}}, "atanh", Expr(1.0_a));

    GlobalConst("C", expr);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:24 error: atanh must be called with a value in the range (-1, 1)");
}

TEST_F(ResolverConstEvalBuiltinTest, Atanh_OutsideRange_Negative) {
    auto* expr = Call(Source{{12, 24}}, "atanh", Negation(1.0_a));

    GlobalConst("C", expr);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:24 error: atanh must be called with a value in the range (-1, 1)");
}

TEST_F(ResolverConstEvalBuiltinTest, Atanh_OutsideRange_Positive_INF) {
    auto* expr = Call(Source{{12, 24}}, "atanh", Expr(f32::Inf()));

    GlobalConst("C", expr);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:24 error: atanh must be called with a value in the range (-1, 1)");
}

TEST_F(ResolverConstEvalBuiltinTest, Atanh_OutsideRange_Negative_INF) {
    auto* expr = Call(Source{{12, 24}}, "atanh", Negation(f32::Inf()));

    GlobalConst("C", expr);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:24 error: atanh must be called with a value in the range (-1, 1)");
}

template <typename T, bool finite_only>
std::vector<Case> AsinCases() {
    std::vector<Case> cases = {
        // If i is +/-0, +/-0 is returned
        C({T(0.0)}, T(0.0)),
        C({-T(0.0)}, -T(0.0)),

        C({T(1.0)}, kPiOver2<T>).FloatComp(),
        C({-T(1.0)}, -kPiOver2<T>).FloatComp(),

        // Vector tests
        C({Vec(T(0.0), T(1.0), -T(1.0))}, Vec(T(0.0), kPiOver2<T>, -kPiOver2<T>)).FloatComp(),
    };

    ConcatIntoIf<!finite_only>(  //
        cases, std::vector<Case>{
                   // If i is NaN, NaN is returned
                   C({T::NaN()}, T::NaN()),

                   // Vector tests
                   C({Vec(T::NaN(), T::NaN())}, Vec(T::NaN(), T::NaN())).FloatComp(),
               });

    return cases;
}
INSTANTIATE_TEST_SUITE_P(  //
    Asin,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kAsin),
                     testing::ValuesIn(Concat(AsinCases<AFloat, true>(),  //
                                              AsinCases<f32, false>(),
                                              AsinCases<f16, false>()))));

TEST_F(ResolverConstEvalBuiltinTest, Asin_OutsideRange_Positive) {
    auto* expr = Call(Source{{12, 24}}, "asin", Expr(1.1_a));

    GlobalConst("C", expr);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:24 error: asin must be called with a value in the range [-1, 1]");
}

TEST_F(ResolverConstEvalBuiltinTest, Asin_OutsideRange_Negative) {
    auto* expr = Call(Source{{12, 24}}, "asin", Negation(1.1_a));

    GlobalConst("C", expr);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:24 error: asin must be called with a value in the range [-1, 1]");
}

TEST_F(ResolverConstEvalBuiltinTest, Asin_OutsideRange_Positive_INF) {
    auto* expr = Call(Source{{12, 24}}, "asin", Expr(f32::Inf()));

    GlobalConst("C", expr);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:24 error: asin must be called with a value in the range [-1, 1]");
}

TEST_F(ResolverConstEvalBuiltinTest, Asin_OutsideRange_Negative_INF) {
    auto* expr = Call(Source{{12, 24}}, "asin", Negation(f32::Inf()));

    GlobalConst("C", expr);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:24 error: asin must be called with a value in the range [-1, 1]");
}

template <typename T, bool finite_only>
std::vector<Case> AsinhCases() {
    std::vector<Case> cases = {
        // If i is +/-0, +/-0 is returned
        C({T(0.0)}, T(0.0)),
        C({-T(0.0)}, -T(0.0)),

        C({T(0.9)}, T(0.80886693565278)).FloatComp(),
        C({-T(2.0)}, -T(1.4436354751788)).FloatComp(),

        // Vector tests
        C({Vec(T(0.0), T(0.9), -T(2.0))},  //
          Vec(T(0.0), T(0.8088669356278), -T(1.4436354751788)))
            .FloatComp(),
    };

    ConcatIntoIf<!finite_only>(  //
        cases, std::vector<Case>{
                   // If i is +/- INF, +/-INF is returned
                   C({T::Inf()}, T::Inf()),
                   C({-T::Inf()}, -T::Inf()),

                   // If i is NaN, NaN is returned
                   C({T::NaN()}, T::NaN()),

                   // Vector tests
                   C({Vec(T::Inf(), T::NaN(), -T::Inf())},  //
                     Vec(T::Inf(), T::NaN(), -T::Inf())),
               });

    return cases;
}
INSTANTIATE_TEST_SUITE_P(  //
    Asinh,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kAsinh),
                     testing::ValuesIn(Concat(AsinhCases<AFloat, true>(),  //
                                              AsinhCases<f32, false>(),
                                              AsinhCases<f16, false>()))));

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
std::vector<Case> SaturateCases() {
    return {
        C({T(0)}, T(0)),
        C({T(1)}, T(1)),
        C({T::Lowest()}, T(0)),
        C({T::Highest()}, T(1)),

        // Vector tests
        C({Vec(T(0), T(0))},                       //
          Vec(T(0), T(0))),                        //
        C({Vec(T(1), T(1))},                       //
          Vec(T(1), T(1))),                        //
        C({Vec(T::Lowest(), T(0), T::Highest())},  //
          Vec(T(0), T(0), T(1))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Saturate,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kSaturate),
                     testing::ValuesIn(Concat(SaturateCases<AFloat>(),  //
                                              SaturateCases<f32>(),
                                              SaturateCases<f16>()))));

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

template <typename T>
std::vector<Case> SignCases() {
    return {
        C({-T(1)}, -T(1)),
        C({-T(0.5)}, -T(1)),
        C({T(0)}, T(0)),
        C({-T(0)}, T(0)),
        C({T(0.5)}, T(1)),
        C({T(1)}, T(1)),

        C({T::Highest()}, T(1.0)),
        C({T::Lowest()}, -T(1.0)),

        // Vector tests
        C({Vec(-T(0.5), T(0), T(0.5))}, Vec(-T(1.0), T(0.0), T(1.0))),
        C({Vec(T::Highest(), T::Lowest())}, Vec(T(1.0), -T(1.0))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Sign,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kSign),
                     testing::ValuesIn(Concat(SignCases<AFloat>(),  //
                                              SignCases<f32>(),
                                              SignCases<f16>()))));

template <typename T>
std::vector<Case> StepCases() {
    return {
        C({T(0), T(0)}, T(1.0)),
        C({T(0), T(0.5)}, T(1.0)),
        C({T(0.5), T(0)}, T(0.0)),
        C({T(1), T(0.5)}, T(0.0)),
        C({T(0.5), T(1)}, T(1.0)),
        C({T(1.5), T(1)}, T(0.0)),
        C({T(1), T(1.5)}, T(1.0)),
        C({T(-1), T(1)}, T(1.0)),
        C({T(-1), T(1)}, T(1.0)),
        C({T(1), T(-1)}, T(0.0)),
        C({T(-1), T(-1.5)}, T(0.0)),
        C({T(-1.5), T(-1)}, T(1.0)),
        C({T::Highest(), T::Lowest()}, T(0.0)),
        C({T::Lowest(), T::Highest()}, T(1.0)),

        // Vector tests
        C({Vec(T(0), T(0)), Vec(T(0), T(0))}, Vec(T(1.0), T(1.0))),
        C({Vec(T(-1), T(1)), Vec(T(0), T(0))}, Vec(T(1.0), T(0.0))),
        C({Vec(T::Highest(), T::Lowest()), Vec(T::Lowest(), T::Highest())}, Vec(T(0.0), T(1.0))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Step,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kStep),
                     testing::ValuesIn(Concat(StepCases<AFloat>(),  //
                                              StepCases<f32>(),
                                              StepCases<f16>()))));

}  // namespace
}  // namespace tint::resolver
