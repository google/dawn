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
// See the License for the empecific language governing permissions and
// limitations under the License.

#include "src/tint/resolver/const_eval_test.h"

#include "src/tint/utils/result.h"

using namespace tint::number_suffixes;  // NOLINT
using ::testing::HasSubstr;

namespace tint::resolver {
namespace {

// Bring in std::ostream& operator<<(std::ostream& o, const Types& types)
using resolver::operator<<;

struct Case {
    Case(utils::VectorRef<Types> in_args, utils::VectorRef<Types> expected_values)
        : args(std::move(in_args)),
          expected(Success{std::move(expected_values), CheckConstantFlags{}}) {}

    Case(utils::VectorRef<Types> in_args, std::string expected_err)
        : args(std::move(in_args)), expected(Failure{std::move(expected_err)}) {}

    /// Expected value may be positive or negative
    Case& PosOrNeg() {
        Success s = expected.Get();
        s.flags.pos_or_neg = true;
        expected = s;
        return *this;
    }

    /// Expected value should be compared using EXPECT_FLOAT_EQ instead of EXPECT_EQ.
    /// If optional epsilon is passed in, will be compared using EXPECT_NEAR with that epsilon.
    Case& FloatComp(std::optional<double> epsilon = {}) {
        Success s = expected.Get();
        s.flags.float_compare = true;
        s.flags.float_compare_epsilon = epsilon;
        expected = s;
        return *this;
    }

    struct Success {
        utils::Vector<Types, 2> values;
        CheckConstantFlags flags;
    };
    struct Failure {
        std::string error;
    };

    utils::Vector<Types, 8> args;
    utils::Result<Success, Failure> expected;
};

static std::ostream& operator<<(std::ostream& o, const Case& c) {
    o << "args: ";
    for (auto& a : c.args) {
        o << a << ", ";
    }
    o << "expected: ";
    if (c.expected) {
        auto s = c.expected.Get();
        if (s.values.Length() == 1) {
            o << s.values[0];
        } else {
            o << "[";
            for (auto& v : s.values) {
                if (&v != &s.values[0]) {
                    o << ", ";
                }
                o << v;
            }
            o << "]";
        }
        o << ", pos_or_neg: " << s.flags.pos_or_neg;
        o << ", float_compare: " << s.flags.float_compare;
    } else {
        o << "[ERROR: " << c.expected.Failure().error << "]";
    }
    return o;
}

using ScalarTypes = std::variant<AInt, AFloat, u32, i32, f32, f16>;

/// Creates a Case with Values for args and result
static Case C(std::initializer_list<Types> args, Types result) {
    return Case{utils::Vector<Types, 8>{args}, utils::Vector<Types, 2>{std::move(result)}};
}

/// Creates a Case with Values for args and result
static Case C(std::initializer_list<Types> args, std::initializer_list<Types> results) {
    return Case{utils::Vector<Types, 8>{args}, utils::Vector<Types, 2>{results}};
}

/// Convenience overload that creates a Case with just scalars
static Case C(std::initializer_list<ScalarTypes> sargs, ScalarTypes sresult) {
    utils::Vector<Types, 8> args;
    for (auto& sa : sargs) {
        std::visit([&](auto&& v) { return args.Push(Val(v)); }, sa);
    }
    Types result = Val(0_a);
    std::visit([&](auto&& v) { result = Val(v); }, sresult);
    return Case{std::move(args), utils::Vector<Types, 2>{std::move(result)}};
}

/// Creates a Case with Values for args and result
static Case C(std::initializer_list<ScalarTypes> sargs,
              std::initializer_list<ScalarTypes> sresults) {
    utils::Vector<Types, 8> args;
    for (auto& sa : sargs) {
        std::visit([&](auto&& v) { return args.Push(Val(v)); }, sa);
    }
    utils::Vector<Types, 2> results;
    for (auto& sa : sresults) {
        std::visit([&](auto&& v) { return results.Push(Val(v)); }, sa);
    }
    return Case{std::move(args), std::move(results)};
}

/// Creates a Case with Values for args and expected error
static Case E(std::initializer_list<Types> args, std::string err) {
    return Case{utils::Vector<Types, 8>{args}, std::move(err)};
}

/// Convenience overload that creates an expected-error Case with just scalars
static Case E(std::initializer_list<ScalarTypes> sargs, std::string err) {
    utils::Vector<Types, 8> args;
    for (auto& sa : sargs) {
        std::visit([&](auto&& v) { return args.Push(Val(v)); }, sa);
    }
    return Case{std::move(args), std::move(err)};
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

    auto* expr = Call(Source{{12, 34}}, sem::str(builtin), std::move(args));
    GlobalConst("C", expr);

    if (c.expected) {
        auto expected_case = c.expected.Get();

        ASSERT_TRUE(r()->Resolve()) << r()->error();

        auto* sem = Sem().Get(expr);
        ASSERT_NE(sem, nullptr);
        const sem::Constant* value = sem->ConstantValue();
        ASSERT_NE(value, nullptr);
        EXPECT_TYPE(value->Type(), sem->Type());

        if (value->Type()->Is<sem::Struct>()) {
            // The result type of the constant-evaluated expression is a structure.
            // Compare each of the fields individually.
            for (size_t i = 0; i < expected_case.values.Length(); i++) {
                CheckConstant(value->Index(i), ToValueBase(expected_case.values[i]),
                              expected_case.flags);
            }
        } else {
            // Return type is not a structure. Just compare the single value
            ASSERT_EQ(expected_case.values.Length(), 1u)
                << "const-eval returned non-struct, but Case expected multiple values";
            CheckConstant(value, ToValueBase(expected_case.values[0]), expected_case.flags);
        }
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(), c.expected.Failure().error);
    }
}

INSTANTIATE_TEST_SUITE_P(  //
    MixedAbstractArgs,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kAtan2),
                     testing::ValuesIn(std::vector{
                         C({0_a, -0.0_a}, kPi<AFloat>),
                         C({1.0_a, 0_a}, kPiOver2<AFloat>),
                     })));

template <typename T>
std::vector<Case> AbsCases() {
    std::vector<Case> cases = {
        C({T(0)}, T(0)),
        C({T(2.0)}, T(2.0)),
        C({T::Highest()}, T::Highest()),

        // Vector tests
        C({Vec(T(2.0), T::Highest())}, Vec(T(2.0), T::Highest())),
    };
    ConcatIntoIf<IsSignedIntegral<T>>(
        cases,
        std::vector<Case>{
            C({Negate(T(0))}, T(0)),
            C({Negate(T(2.0))}, T(2.0)),
            // If e is signed and is the largest negative, the result is e
            C({T::Lowest()}, T::Lowest()),

            // 1 more then min i32
            C({Negate(T(2147483647))}, T(2147483647)),

            C({Vec(T(0), Negate(T(0)))}, Vec(T(0), T(0))),
            C({Vec(Negate(T(2.0)), T(2.0), T::Highest())}, Vec(T(2.0), T(2.0), T::Highest())),
        });
    return cases;
}
INSTANTIATE_TEST_SUITE_P(  //
    Abs,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kAbs),
                     testing::ValuesIn(Concat(AbsCases<AInt>(),  //
                                              AbsCases<i32>(),
                                              AbsCases<u32>(),
                                              AbsCases<AFloat>(),
                                              AbsCases<f32>(),
                                              AbsCases<f16>()))));

static std::vector<Case> AllCases() {
    return {
        C({Val(true)}, Val(true)),
        C({Val(false)}, Val(false)),

        C({Vec(true, true)}, Val(true)),
        C({Vec(true, false)}, Val(false)),
        C({Vec(false, true)}, Val(false)),
        C({Vec(false, false)}, Val(false)),

        C({Vec(true, true, true)}, Val(true)),
        C({Vec(false, true, true)}, Val(false)),
        C({Vec(true, false, true)}, Val(false)),
        C({Vec(true, true, false)}, Val(false)),
        C({Vec(false, false, false)}, Val(false)),

        C({Vec(true, true, true, true)}, Val(true)),
        C({Vec(false, true, true, true)}, Val(false)),
        C({Vec(true, false, true, true)}, Val(false)),
        C({Vec(true, true, false, true)}, Val(false)),
        C({Vec(true, true, true, false)}, Val(false)),
        C({Vec(false, false, false, false)}, Val(false)),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    All,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kAll), testing::ValuesIn(AllCases())));

static std::vector<Case> AnyCases() {
    return {
        C({Val(true)}, Val(true)),
        C({Val(false)}, Val(false)),

        C({Vec(true, true)}, Val(true)),
        C({Vec(true, false)}, Val(true)),
        C({Vec(false, true)}, Val(true)),
        C({Vec(false, false)}, Val(false)),

        C({Vec(true, true, true)}, Val(true)),
        C({Vec(false, true, true)}, Val(true)),
        C({Vec(true, false, true)}, Val(true)),
        C({Vec(true, true, false)}, Val(true)),
        C({Vec(false, false, false)}, Val(false)),

        C({Vec(true, true, true, true)}, Val(true)),
        C({Vec(false, true, true, true)}, Val(true)),
        C({Vec(true, false, true, true)}, Val(true)),
        C({Vec(true, true, false, true)}, Val(true)),
        C({Vec(true, true, true, false)}, Val(true)),
        C({Vec(false, false, false, false)}, Val(false)),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Any,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kAny), testing::ValuesIn(AnyCases())));

template <typename T>
std::vector<Case> Atan2Cases() {
    return {
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
}
INSTANTIATE_TEST_SUITE_P(  //
    Atan2,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kAtan2),
                     testing::ValuesIn(Concat(Atan2Cases<AFloat>(),  //
                                              Atan2Cases<f32>(),
                                              Atan2Cases<f16>()))));

template <typename T>
std::vector<Case> AtanCases() {
    return {
        C({T(1.0)}, kPiOver4<T>).FloatComp(),
        C({-T(1.0)}, -kPiOver4<T>).FloatComp(),

        // If i is +/-0, +/-0 is returned
        C({T(0.0)}, T(0.0)).PosOrNeg(),

        // Vector tests
        C({Vec(T(0.0), T(1.0), -T(1.0))}, Vec(T(0.0), kPiOver4<T>, -kPiOver4<T>)).FloatComp(),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Atan,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kAtan),
                     testing::ValuesIn(Concat(AtanCases<AFloat>(),  //
                                              AtanCases<f32>(),
                                              AtanCases<f16>()))));

template <typename T>
std::vector<Case> AtanhCases() {
    return {
        // If i is +/-0, +/-0 is returned
        C({T(0.0)}, T(0.0)).PosOrNeg(),

        C({T(0.9)}, T(1.4722193)).FloatComp(),

        // Vector tests
        C({Vec(T(0.0), T(0.9), -T(0.9))}, Vec(T(0.0), T(1.4722193), -T(1.4722193))).FloatComp(),

        E({T(1.1)},
          "12:34 error: atanh must be called with a value in the range (-1 .. 1) (exclusive)"),
        E({-T(1.1)},
          "12:34 error: atanh must be called with a value in the range (-1 .. 1) (exclusive)"),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Atanh,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kAtanh),
                     testing::ValuesIn(Concat(AtanhCases<AFloat>(),  //
                                              AtanhCases<f32>(),
                                              AtanhCases<f16>()))));

template <typename T>
std::vector<Case> AcosCases() {
    return {
        // If i is +/-0, +/-0 is returned
        C({T(0.87758256189)}, T(0.5)).FloatComp(),

        C({T(1.0)}, T(0.0)),
        C({-T(1.0)}, kPi<T>).FloatComp(),

        // Vector tests
        C({Vec(T(1.0), -T(1.0))}, Vec(T(0), kPi<T>)).FloatComp(),

        E({T(1.1)},
          "12:34 error: acos must be called with a value in the range [-1 .. 1] (inclusive)"),
        E({-T(1.1)},
          "12:34 error: acos must be called with a value in the range [-1 .. 1] (inclusive)"),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Acos,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kAcos),
                     testing::ValuesIn(Concat(AcosCases<AFloat>(),  //
                                              AcosCases<f32>(),
                                              AcosCases<f16>()))));

template <typename T>
std::vector<Case> AcoshCases() {
    return {
        C({T(1.0)}, T(0.0)),
        C({T(11.5919532755)}, kPi<T>).FloatComp(),

        // Vector tests
        C({Vec(T(1.0), T(11.5919532755))}, Vec(T(0), kPi<T>)).FloatComp(),

        E({T::Smallest()}, "12:34 error: acosh must be called with a value >= 1.0"),
        E({-T(1.1)}, "12:34 error: acosh must be called with a value >= 1.0"),
        E({T(0)}, "12:34 error: acosh must be called with a value >= 1.0"),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Acosh,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kAcosh),
                     testing::ValuesIn(Concat(AcoshCases<AFloat>(),  //
                                              AcoshCases<f32>(),
                                              AcoshCases<f16>()))));

template <typename T>
std::vector<Case> AsinCases() {
    return {
        // If i is +/-0, +/-0 is returned
        C({T(0.0)}, T(0.0)),
        C({-T(0.0)}, -T(0.0)),

        C({T(1.0)}, kPiOver2<T>).FloatComp(),
        C({-T(1.0)}, -kPiOver2<T>).FloatComp(),

        // Vector tests
        C({Vec(T(0.0), T(1.0), -T(1.0))}, Vec(T(0.0), kPiOver2<T>, -kPiOver2<T>)).FloatComp(),

        E({T(1.1)},
          "12:34 error: asin must be called with a value in the range [-1 .. 1] (inclusive)"),
        E({-T(1.1)},
          "12:34 error: asin must be called with a value in the range [-1 .. 1] (inclusive)"),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Asin,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kAsin),
                     testing::ValuesIn(Concat(AsinCases<AFloat>(),  //
                                              AsinCases<f32>(),
                                              AsinCases<f16>()))));

template <typename T>
std::vector<Case> AsinhCases() {
    return {
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
}
INSTANTIATE_TEST_SUITE_P(  //
    Asinh,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kAsinh),
                     testing::ValuesIn(Concat(AsinhCases<AFloat>(),  //
                                              AsinhCases<f32>(),
                                              AsinhCases<f16>()))));

template <typename T>
std::vector<Case> CeilCases() {
    return {
        C({T(0)}, T(0)),
        C({-T(0)}, -T(0)),
        C({-T(1.5)}, -T(1.0)),
        C({T(1.5)}, T(2.0)),
        C({T::Lowest()}, T::Lowest()),
        C({T::Highest()}, T::Highest()),

        C({Vec(T(0), T(1.5), -T(1.5))}, Vec(T(0), T(2.0), -T(1.0))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Ceil,
    ResolverConstEvalBuiltinTest,
    testing::Combine(
        testing::Values(sem::BuiltinType::kCeil),
        testing::ValuesIn(Concat(CeilCases<AFloat>(), CeilCases<f32>(), CeilCases<f16>()))));

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
std::vector<Case> CosCases() {
    std::vector<Case> cases = {
        C({-T(0)}, T(1)),
        C({T(0)}, T(1)),

        C({T(0.75)}, T(0.7316888689)).FloatComp(),

        // Vector test
        C({Vec(T(0), -T(0), T(0.75))}, Vec(T(1), T(1), T(0.7316888689))).FloatComp(),
    };

    return cases;
}
INSTANTIATE_TEST_SUITE_P(  //
    Cos,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kCos),
                     testing::ValuesIn(Concat(CosCases<AFloat>(),  //
                                              CosCases<f32>(),
                                              CosCases<f16>()))));

template <typename T>
std::vector<Case> CoshCases() {
    auto error_msg = [](auto a) {
        return "12:34 error: " + OverflowErrorMessage(a, FriendlyName<decltype(a)>());
    };
    std::vector<Case> cases = {
        C({T(0)}, T(1)),
        C({-T(0)}, T(1)),
        C({T(1)}, T(1.5430806348)).FloatComp(),

        C({T(.75)}, T(1.2946832847)).FloatComp(),

        // Vector tests
        C({Vec(T(0), -T(0), T(1))}, Vec(T(1), T(1), T(1.5430806348))).FloatComp(),

        E({T(10000)}, error_msg(T::Inf())),
    };
    return cases;
}
INSTANTIATE_TEST_SUITE_P(  //
    Cosh,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kCosh),
                     testing::ValuesIn(Concat(CoshCases<AFloat>(),  //
                                              CoshCases<f32>(),
                                              CoshCases<f16>()))));

template <typename T>
std::vector<Case> CountLeadingZerosCases() {
    using B = BitValues<T>;
    return {
        C({B::Lsh(1, 31)}, T(0)),  //
        C({B::Lsh(1, 30)}, T(1)),  //
        C({B::Lsh(1, 29)}, T(2)),  //
        C({B::Lsh(1, 28)}, T(3)),
        //...
        C({B::Lsh(1, 3)}, T(28)),  //
        C({B::Lsh(1, 2)}, T(29)),  //
        C({B::Lsh(1, 1)}, T(30)),  //
        C({B::Lsh(1, 0)}, T(31)),

        C({T(0b1111'0000'1111'0000'1111'0000'1111'0000)}, T(0)),
        C({T(0b0111'1000'0111'1000'0111'1000'0111'1000)}, T(1)),
        C({T(0b0011'1100'0011'1100'0011'1100'0011'1100)}, T(2)),
        C({T(0b0001'1110'0001'1110'0001'1110'0001'1110)}, T(3)),
        //...
        C({T(0b0000'0000'0000'0000'0000'0000'0000'0111)}, T(29)),
        C({T(0b0000'0000'0000'0000'0000'0000'0000'0011)}, T(30)),
        C({T(0b0000'0000'0000'0000'0000'0000'0000'0001)}, T(31)),
        C({T(0b0000'0000'0000'0000'0000'0000'0000'0000)}, T(32)),

        // Same as above, but remove leading 0
        C({T(0b1111'1000'0111'1000'0111'1000'0111'1000)}, T(0)),
        C({T(0b1011'1100'0011'1100'0011'1100'0011'1100)}, T(0)),
        C({T(0b1001'1110'0001'1110'0001'1110'0001'1110)}, T(0)),
        //...
        C({T(0b1000'0000'0000'0000'0000'0000'0000'0111)}, T(0)),
        C({T(0b1000'0000'0000'0000'0000'0000'0000'0011)}, T(0)),
        C({T(0b1000'0000'0000'0000'0000'0000'0000'0001)}, T(0)),
        C({T(0b1000'0000'0000'0000'0000'0000'0000'0000)}, T(0)),

        // Vector tests
        C({Vec(B::Lsh(1, 31), B::Lsh(1, 30), B::Lsh(1, 29))}, Vec(T(0), T(1), T(2))),
        C({Vec(B::Lsh(1, 2), B::Lsh(1, 1), B::Lsh(1, 0))}, Vec(T(29), T(30), T(31))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    CountLeadingZeros,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kCountLeadingZeros),
                     testing::ValuesIn(Concat(CountLeadingZerosCases<i32>(),  //
                                              CountLeadingZerosCases<u32>()))));

template <typename T>
std::vector<Case> CountTrailingZerosCases() {
    using B = BitValues<T>;
    return {
        C({B::Lsh(1, 31)}, T(31)),  //
        C({B::Lsh(1, 30)}, T(30)),  //
        C({B::Lsh(1, 29)}, T(29)),  //
        C({B::Lsh(1, 28)}, T(28)),
        //...
        C({B::Lsh(1, 3)}, T(3)),  //
        C({B::Lsh(1, 2)}, T(2)),  //
        C({B::Lsh(1, 1)}, T(1)),  //
        C({B::Lsh(1, 0)}, T(0)),

        C({T(0b0000'1111'0000'1111'0000'1111'0000'1111)}, T(0)),
        C({T(0b0001'1110'0001'1110'0001'1110'0001'1110)}, T(1)),
        C({T(0b0011'1100'0011'1100'0011'1100'0011'1100)}, T(2)),
        C({T(0b0111'1000'0111'1000'0111'1000'0111'1000)}, T(3)),
        //...
        C({T(0b1110'0000'0000'0000'0000'0000'0000'0000)}, T(29)),
        C({T(0b1100'0000'0000'0000'0000'0000'0000'0000)}, T(30)),
        C({T(0b1000'0000'0000'0000'0000'0000'0000'0000)}, T(31)),
        C({T(0b0000'0000'0000'0000'0000'0000'0000'0000)}, T(32)),

        //// Same as above, but remove trailing 0
        C({T(0b0001'1110'0001'1110'0001'1110'0001'1111)}, T(0)),
        C({T(0b0011'1100'0011'1100'0011'1100'0011'1101)}, T(0)),
        C({T(0b0111'1000'0111'1000'0111'1000'0111'1001)}, T(0)),
        //...
        C({T(0b1110'0000'0000'0000'0000'0000'0000'0001)}, T(0)),
        C({T(0b1100'0000'0000'0000'0000'0000'0000'0001)}, T(0)),
        C({T(0b1000'0000'0000'0000'0000'0000'0000'0001)}, T(0)),
        C({T(0b0000'0000'0000'0000'0000'0000'0000'0001)}, T(0)),

        // Vector tests
        C({Vec(B::Lsh(1, 31), B::Lsh(1, 30), B::Lsh(1, 29))}, Vec(T(31), T(30), T(29))),
        C({Vec(B::Lsh(1, 2), B::Lsh(1, 1), B::Lsh(1, 0))}, Vec(T(2), T(1), T(0))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    CountTrailingZeros,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kCountTrailingZeros),
                     testing::ValuesIn(Concat(CountTrailingZerosCases<i32>(),  //
                                              CountTrailingZerosCases<u32>()))));

template <typename T>
std::vector<Case> CountOneBitsCases() {
    using B = BitValues<T>;
    return {
        C({T(0)}, T(0)),  //

        C({B::Lsh(1, 31)}, T(1)),  //
        C({B::Lsh(1, 30)}, T(1)),  //
        C({B::Lsh(1, 29)}, T(1)),  //
        C({B::Lsh(1, 28)}, T(1)),
        //...
        C({B::Lsh(1, 3)}, T(1)),  //
        C({B::Lsh(1, 2)}, T(1)),  //
        C({B::Lsh(1, 1)}, T(1)),  //
        C({B::Lsh(1, 0)}, T(1)),

        C({T(0b1010'1010'1010'1010'1010'1010'1010'1010)}, T(16)),
        C({T(0b0000'1111'0000'1111'0000'1111'0000'1111)}, T(16)),
        C({T(0b0101'0000'0000'0000'0000'0000'0000'0101)}, T(4)),

        // Vector tests
        C({Vec(B::Lsh(1, 31), B::Lsh(1, 30), B::Lsh(1, 29))}, Vec(T(1), T(1), T(1))),
        C({Vec(B::Lsh(1, 2), B::Lsh(1, 1), B::Lsh(1, 0))}, Vec(T(1), T(1), T(1))),

        C({Vec(T(0b1010'1010'1010'1010'1010'1010'1010'1010),
               T(0b0000'1111'0000'1111'0000'1111'0000'1111),
               T(0b0101'0000'0000'0000'0000'0000'0000'0101))},
          Vec(T(16), T(16), T(4))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    CountOneBits,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kCountOneBits),
                     testing::ValuesIn(Concat(CountOneBitsCases<i32>(),  //
                                              CountOneBitsCases<u32>()))));

template <typename T>
std::vector<Case> CrossCases() {
    constexpr auto vec_x = [](T v) { return Vec(T(v), T(0), T(0)); };
    constexpr auto vec_y = [](T v) { return Vec(T(0), T(v), T(0)); };
    constexpr auto vec_z = [](T v) { return Vec(T(0), T(0), T(v)); };

    const auto zero = Vec(T(0), T(0), T(0));
    const auto unit_x = vec_x(T(1));
    const auto unit_y = vec_y(T(1));
    const auto unit_z = vec_z(T(1));
    const auto neg_unit_x = vec_x(-T(1));
    const auto neg_unit_y = vec_y(-T(1));
    const auto neg_unit_z = vec_z(-T(1));
    const auto highest_x = vec_x(T::Highest());
    const auto highest_y = vec_y(T::Highest());
    const auto highest_z = vec_z(T::Highest());
    const auto smallest_x = vec_x(T::Smallest());
    const auto smallest_y = vec_y(T::Smallest());
    const auto smallest_z = vec_z(T::Smallest());
    const auto lowest_x = vec_x(T::Lowest());
    const auto lowest_y = vec_y(T::Lowest());
    const auto lowest_z = vec_z(T::Lowest());

    std::vector<Case> r = {
        C({zero, zero}, zero),

        C({unit_x, unit_x}, zero),
        C({unit_y, unit_y}, zero),
        C({unit_z, unit_z}, zero),

        C({smallest_x, smallest_x}, zero),
        C({smallest_y, smallest_y}, zero),
        C({smallest_z, smallest_z}, zero),

        C({lowest_x, lowest_x}, zero),
        C({lowest_y, lowest_y}, zero),
        C({lowest_z, lowest_z}, zero),

        C({highest_x, highest_x}, zero),
        C({highest_y, highest_y}, zero),
        C({highest_z, highest_z}, zero),

        C({smallest_x, highest_x}, zero),
        C({smallest_y, highest_y}, zero),
        C({smallest_z, highest_z}, zero),

        C({unit_x, neg_unit_x}, zero).PosOrNeg(),
        C({unit_y, neg_unit_y}, zero).PosOrNeg(),
        C({unit_z, neg_unit_z}, zero).PosOrNeg(),

        C({unit_x, unit_y}, unit_z),
        C({unit_y, unit_x}, neg_unit_z),

        C({unit_z, unit_x}, unit_y),
        C({unit_x, unit_z}, neg_unit_y),

        C({unit_y, unit_z}, unit_x),
        C({unit_z, unit_y}, neg_unit_x),

        C({vec_x(T(1)), vec_y(T(2))}, vec_z(T(2))),
        C({vec_y(T(1)), vec_x(T(2))}, vec_z(-T(2))),
        C({vec_x(T(2)), vec_y(T(3))}, vec_z(T(6))),
        C({vec_y(T(2)), vec_x(T(3))}, vec_z(-T(6))),

        C({Vec(T(1), T(2), T(3)), Vec(T(1), T(5), T(7))}, Vec(T(-1), T(-4), T(3))),
        C({Vec(T(33), T(44), T(55)), Vec(T(13), T(42), T(39))}, Vec(T(-594), T(-572), T(814))),
        C({Vec(T(3.5), T(4), T(5.5)), Vec(T(1), T(4.5), T(3.5))},
          Vec(T(-10.75), T(-6.75), T(11.75))),
    };

    std::string pos_error_msg =
        "12:34 error: " + OverflowErrorMessage(T::Highest(), "*", T::Highest());
    std::string neg_error_msg =
        "12:34 error: " + OverflowErrorMessage(T::Lowest(), "*", T::Lowest());
    ConcatInto(  //
        r, std::vector<Case>{
               E({highest_x, highest_y}, pos_error_msg),
               E({highest_y, highest_x}, pos_error_msg),
               E({highest_z, highest_x}, pos_error_msg),
               E({highest_x, highest_z}, pos_error_msg),
               E({highest_y, highest_z}, pos_error_msg),
               E({highest_z, highest_y}, pos_error_msg),
               E({lowest_x, lowest_y}, neg_error_msg),
               E({lowest_y, lowest_x}, neg_error_msg),
               E({lowest_z, lowest_x}, neg_error_msg),
               E({lowest_x, lowest_z}, neg_error_msg),
               E({lowest_y, lowest_z}, neg_error_msg),
               E({lowest_z, lowest_y}, neg_error_msg),
           });

    return r;
}
INSTANTIATE_TEST_SUITE_P(  //
    Cross,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kCross),
                     testing::ValuesIn(Concat(CrossCases<AFloat>(),  //
                                              CrossCases<f32>(),     //
                                              CrossCases<f16>()))));

template <typename T>
std::vector<Case> DotCases() {
    auto r = std::vector<Case>{
        C({Vec(T(0), T(0)), Vec(T(0), T(0))}, Val(T(0))),
        C({Vec(T(0), T(0), T(0)), Vec(T(0), T(0), T(0))}, Val(T(0))),
        C({Vec(T(0), T(0), T(0), T(0)), Vec(T(0), T(0), T(0), T(0))}, Val(T(0))),
        C({Vec(T(1), T(2), T(3), T(4)), Vec(T(5), T(6), T(7), T(8))}, Val(T(70))),

        C({Vec(T(1), T(1)), Vec(T(1), T(1))}, Val(T(2))),
        C({Vec(T(1), T(2)), Vec(T(2), T(1))}, Val(T(4))),
        C({Vec(T(2), T(2)), Vec(T(2), T(2))}, Val(T(8))),

        C({Vec(T::Highest(), T::Highest()), Vec(T(1), T(0))}, Val(T::Highest())),
        C({Vec(T::Lowest(), T::Lowest()), Vec(T(1), T(0))}, Val(T::Lowest())),
    };

    if constexpr (IsAbstract<T> || IsFloatingPoint<T>) {
        auto error_msg = [](auto a, const char* op, auto b) {
            return "12:34 error: " + OverflowErrorMessage(a, op, b) + R"(
12:34 note: when calculating dot)";
        };
        ConcatInto(  //
            r, std::vector<Case>{
                   E({Vec(T::Highest(), T::Highest()), Vec(T(1), T(1))},
                     error_msg(T::Highest(), "+", T::Highest())),
                   E({Vec(T::Lowest(), T::Lowest()), Vec(T(1), T(1))},
                     error_msg(T::Lowest(), "+", T::Lowest())),
               });
    } else {
        // Overflow is not an error for concrete integrals
        ConcatInto(  //
            r, std::vector<Case>{
                   C({Vec(T::Highest(), T::Highest()), Vec(T(1), T(1))},
                     Val(Add(T::Highest(), T::Highest()))),
                   C({Vec(T::Lowest(), T::Lowest()), Vec(T(1), T(1))},
                     Val(Add(T::Lowest(), T::Lowest()))),
               });
    }
    return r;
}
INSTANTIATE_TEST_SUITE_P(  //
    Dot,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kDot),
                     testing::ValuesIn(Concat(DotCases<AInt>(),    //
                                              DotCases<i32>(),     //
                                              DotCases<u32>(),     //
                                              DotCases<AFloat>(),  //
                                              DotCases<f32>(),     //
                                              DotCases<f16>()))));

template <typename T>
std::vector<Case> DeterminantCases() {
    auto error_msg = [](auto a, const char* op, auto b) {
        return "12:34 error: " + OverflowErrorMessage(a, op, b) + R"(
12:34 note: when calculating determinant)";
    };

    auto r = std::vector<Case>{
        // All zero == 0
        C({Mat({T(0), T(0)},    //
               {T(0), T(0)})},  //
          Val(T(0))),

        C({Mat({T(0), T(0), T(0)},    //
               {T(0), T(0), T(0)},    //
               {T(0), T(0), T(0)})},  //
          Val(T(0))),

        C({Mat({T(0), T(0), T(0), T(0)},    //
               {T(0), T(0), T(0), T(0)},    //
               {T(0), T(0), T(0), T(0)},    //
               {T(0), T(0), T(0), T(0)})},  //
          Val(T(0))),

        // All same == 0
        C({Mat({T(42), T(42)},    //
               {T(42), T(42)})},  //
          Val(T(0))),

        C({Mat({T(42), T(42), T(42)},    //
               {T(42), T(42), T(42)},    //
               {T(42), T(42), T(42)})},  //
          Val(T(0))),

        C({Mat({T(42), T(42), T(42), T(42)},    //
               {T(42), T(42), T(42), T(42)},    //
               {T(42), T(42), T(42), T(42)},    //
               {T(42), T(42), T(42), T(42)})},  //
          Val(T(0))),

        // Various values
        C({Mat({-T(2), T(17)},   //
               {T(5), T(45)})},  //
          Val(-T(175))),

        C({Mat({T(4), T(6), -T(13)},    //
               {T(12), T(5), T(8)},     //
               {T(9), T(17), T(16)})},  //
          Val(-T(3011))),

        C({Mat({T(2), T(9), T(8), T(1)},       //
               {-T(4), T(11), -T(3), T(7)},    //
               {T(6), T(5), T(12), -T(6)},     //
               {T(3), -T(10), T(4), -T(7)})},  //
          Val(T(469))),

        // Overflow during multiply
        E({Mat({T::Highest(), T(0)},  //
               {T(0), T(2)})},        //
          error_msg(T::Highest(), "*", T(2))),

        // Overflow during subtract
        E({Mat({T::Highest(), T::Lowest()},  //
               {T(1), T(1)})},               //
          error_msg(T::Highest(), "-", T::Lowest())),
    };

    return r;
}
INSTANTIATE_TEST_SUITE_P(  //
    Determinant,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kDeterminant),
                     testing::ValuesIn(Concat(DeterminantCases<AFloat>(),  //
                                              DeterminantCases<f32>(),     //
                                              DeterminantCases<f16>()))));

template <typename T>
std::vector<Case> FirstLeadingBitCases() {
    using B = BitValues<T>;
    auto r = std::vector<Case>{
        // Both signed and unsigned return T(-1) for input 0
        C({T(0)}, T(-1)),

        C({B::Lsh(1, 30)}, T(30)),  //
        C({B::Lsh(1, 29)}, T(29)),  //
        C({B::Lsh(1, 28)}, T(28)),
        //...
        C({B::Lsh(1, 3)}, T(3)),  //
        C({B::Lsh(1, 2)}, T(2)),  //
        C({B::Lsh(1, 1)}, T(1)),  //
        C({B::Lsh(1, 0)}, T(0)),

        C({T(0b0000'0000'0100'1000'1000'1000'0000'0000)}, T(22)),
        C({T(0b0000'0000'0000'0100'1000'1000'0000'0000)}, T(18)),

        // Vector tests
        C({Vec(B::Lsh(1, 30), B::Lsh(1, 29), B::Lsh(1, 28))}, Vec(T(30), T(29), T(28))),
        C({Vec(B::Lsh(1, 2), B::Lsh(1, 1), B::Lsh(1, 0))}, Vec(T(2), T(1), T(0))),
    };

    ConcatIntoIf<IsUnsignedIntegral<T>>(  //
        r, std::vector<Case>{
               C({B::Lsh(1, 31)}, T(31)),

               C({T(0b1111'1111'1111'1111'1111'1111'1111'1110)}, T(31)),
               C({T(0b1111'1111'1111'1111'1111'1111'1111'1100)}, T(31)),
               C({T(0b1111'1111'1111'1111'1111'1111'1111'1000)}, T(31)),
               //...
               C({T(0b1110'0000'0000'0000'0000'0000'0000'0000)}, T(31)),
               C({T(0b1100'0000'0000'0000'0000'0000'0000'0000)}, T(31)),
               C({T(0b1000'0000'0000'0000'0000'0000'0000'0000)}, T(31)),
           });

    ConcatIntoIf<IsSignedIntegral<T>>(  //
        r, std::vector<Case>{
               // Signed returns -1 for input -1
               C({T(-1)}, T(-1)),

               C({B::Lsh(1, 31)}, T(30)),

               C({T(0b1111'1111'1111'1111'1111'1111'1111'1110)}, T(0)),
               C({T(0b1111'1111'1111'1111'1111'1111'1111'1100)}, T(1)),
               C({T(0b1111'1111'1111'1111'1111'1111'1111'1000)}, T(2)),
               //...
               C({T(0b1110'0000'0000'0000'0000'0000'0000'0000)}, T(28)),
               C({T(0b1100'0000'0000'0000'0000'0000'0000'0000)}, T(29)),
               C({T(0b1000'0000'0000'0000'0000'0000'0000'0000)}, T(30)),
           });

    return r;
}
INSTANTIATE_TEST_SUITE_P(  //
    FirstLeadingBit,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kFirstLeadingBit),
                     testing::ValuesIn(Concat(FirstLeadingBitCases<i32>(),  //
                                              FirstLeadingBitCases<u32>()))));

template <typename T>
std::vector<Case> FirstTrailingBitCases() {
    using B = BitValues<T>;
    auto r = std::vector<Case>{
        C({T(0)}, T(-1)),

        C({B::Lsh(1, 31)}, T(31)),  //
        C({B::Lsh(1, 30)}, T(30)),  //
        C({B::Lsh(1, 29)}, T(29)),  //
        C({B::Lsh(1, 28)}, T(28)),
        //...
        C({B::Lsh(1, 3)}, T(3)),  //
        C({B::Lsh(1, 2)}, T(2)),  //
        C({B::Lsh(1, 1)}, T(1)),  //
        C({B::Lsh(1, 0)}, T(0)),

        C({T(0b0000'0000'0100'1000'1000'1000'0000'0000)}, T(11)),
        C({T(0b0000'0100'1000'1000'1000'0000'0000'0000)}, T(15)),

        // Vector tests
        C({Vec(B::Lsh(1, 31), B::Lsh(1, 30), B::Lsh(1, 29))}, Vec(T(31), T(30), T(29))),
        C({Vec(B::Lsh(1, 2), B::Lsh(1, 1), B::Lsh(1, 0))}, Vec(T(2), T(1), T(0))),
    };

    return r;
}
INSTANTIATE_TEST_SUITE_P(  //
    FirstTrailingBit,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kFirstTrailingBit),
                     testing::ValuesIn(Concat(FirstTrailingBitCases<i32>(),  //
                                              FirstTrailingBitCases<u32>()))));

template <typename T>
std::vector<Case> FloorCases() {
    return {
        C({T(0)}, T(0)),
        C({-T(0)}, -T(0)),
        C({-T(1.5)}, -T(2.0)),
        C({T(1.5)}, T(1.0)),
        C({T::Lowest()}, T::Lowest()),
        C({T::Highest()}, T::Highest()),

        C({Vec(T(0), T(1.5), -T(1.5))}, Vec(T(0), T(1.0), -T(2.0))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Floor,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kFloor),
                     testing::ValuesIn(Concat(FloorCases<AFloat>(),  //
                                              FloorCases<f32>(),
                                              FloorCases<f16>()))));
template <typename T>
std::vector<Case> FrexpCases() {
    using F = T;                                                         // fract type
    using E = std::conditional_t<std::is_same_v<T, AFloat>, AInt, i32>;  // exp type

    auto cases = std::vector<Case>{
        // Scalar tests
        //  in         fract     exp
        C({T(-3.5)}, {F(-0.875), E(2)}),  //
        C({T(-3.0)}, {F(-0.750), E(2)}),  //
        C({T(-2.5)}, {F(-0.625), E(2)}),  //
        C({T(-2.0)}, {F(-0.500), E(2)}),  //
        C({T(-1.5)}, {F(-0.750), E(1)}),  //
        C({T(-1.0)}, {F(-0.500), E(1)}),  //
        C({T(+0.0)}, {F(+0.000), E(0)}),  //
        C({T(+1.0)}, {F(+0.500), E(1)}),  //
        C({T(+1.5)}, {F(+0.750), E(1)}),  //
        C({T(+2.0)}, {F(+0.500), E(2)}),  //
        C({T(+2.5)}, {F(+0.625), E(2)}),  //
        C({T(+3.0)}, {F(+0.750), E(2)}),  //
        C({T(+3.5)}, {F(+0.875), E(2)}),  //

        // Vector tests
        //         in                 fract                    exp
        C({Vec(T(-2.5), T(+1.0))}, {Vec(F(-0.625), F(+0.500)), Vec(E(2), E(1))}),
        C({Vec(T(+3.5), T(-2.5))}, {Vec(F(+0.875), F(-0.625)), Vec(E(2), E(2))}),
    };

    ConcatIntoIf<std::is_same_v<T, f16>>(cases, std::vector<Case>{
                                                    C({T::Highest()}, {F(0x0.ffep0), E(16)}),  //
                                                    C({T::Lowest()}, {F(-0x0.ffep0), E(16)}),  //
                                                    C({T::Smallest()}, {F(0.5), E(-13)}),      //
                                                });

    ConcatIntoIf<std::is_same_v<T, f32>>(cases,
                                         std::vector<Case>{
                                             C({T::Highest()}, {F(0x0.ffffffp0), E(128)}),  //
                                             C({T::Lowest()}, {F(-0x0.ffffffp0), E(128)}),  //
                                             C({T::Smallest()}, {F(0.5), E(-125)}),         //
                                         });

    ConcatIntoIf<std::is_same_v<T, AFloat>>(
        cases, std::vector<Case>{
                   C({T::Highest()}, {F(0x0.fffffffffffff8p0), E(1024)}),  //
                   C({T::Lowest()}, {F(-0x0.fffffffffffff8p0), E(1024)}),  //
                   C({T::Smallest()}, {F(0.5), E(-1021)}),                 //
               });
    return cases;
}
INSTANTIATE_TEST_SUITE_P(  //
    Frexp,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kFrexp),
                     testing::ValuesIn(Concat(FrexpCases<AFloat>(),  //
                                              FrexpCases<f32>(),     //
                                              FrexpCases<f16>()))));

template <typename T>
std::vector<Case> InsertBitsCases() {
    using UT = Number<std::make_unsigned_t<UnwrapNumber<T>>>;

    auto e = /* */ T(0b0101'1100'0011'1010'0101'1100'0011'1010);
    auto newbits = T{0b1010'0011'1100'0101'1010'0011'1100'0101};

    auto r = std::vector<Case>{
        // args: e, newbits, offset, count

        // If count is 0, result is e
        C({e, newbits, UT(0), UT(0)}, e),  //
        C({e, newbits, UT(1), UT(0)}, e),  //
        C({e, newbits, UT(2), UT(0)}, e),  //
        C({e, newbits, UT(3), UT(0)}, e),  //
        // ...
        C({e, newbits, UT(29), UT(0)}, e),  //
        C({e, newbits, UT(30), UT(0)}, e),  //
        C({e, newbits, UT(31), UT(0)}, e),

        // Copy 1 to 32 bits of newbits to e at offset 0
        C({e, newbits, UT(0), UT(1)}, T(0b0101'1100'0011'1010'0101'1100'0011'1011)),
        C({e, newbits, UT(0), UT(2)}, T(0b0101'1100'0011'1010'0101'1100'0011'1001)),
        C({e, newbits, UT(0), UT(3)}, T(0b0101'1100'0011'1010'0101'1100'0011'1101)),
        C({e, newbits, UT(0), UT(4)}, T(0b0101'1100'0011'1010'0101'1100'0011'0101)),
        C({e, newbits, UT(0), UT(5)}, T(0b0101'1100'0011'1010'0101'1100'0010'0101)),
        C({e, newbits, UT(0), UT(6)}, T(0b0101'1100'0011'1010'0101'1100'0000'0101)),
        // ...
        C({e, newbits, UT(0), UT(29)}, T(0b0100'0011'1100'0101'1010'0011'1100'0101)),
        C({e, newbits, UT(0), UT(30)}, T(0b0110'0011'1100'0101'1010'0011'1100'0101)),
        C({e, newbits, UT(0), UT(31)}, T(0b0010'0011'1100'0101'1010'0011'1100'0101)),
        C({e, newbits, UT(0), UT(32)}, T(0b1010'0011'1100'0101'1010'0011'1100'0101)),

        // Copy at varying offsets and counts
        C({e, newbits, UT(3), UT(8)}, T(0b0101'1100'0011'1010'0101'1110'0010'1010)),
        C({e, newbits, UT(8), UT(8)}, T(0b0101'1100'0011'1010'1100'0101'0011'1010)),
        C({e, newbits, UT(15), UT(1)}, T(0b0101'1100'0011'1010'1101'1100'0011'1010)),
        C({e, newbits, UT(16), UT(16)}, T(0b1010'0011'1100'0101'0101'1100'0011'1010)),

        // Vector tests
        C({Vec(T(0b1111'0000'1111'0000'1111'0000'1111'0000),  //
               T(0b0000'1111'0000'1111'0000'1111'0000'1111),  //
               T(0b1010'0101'1010'0101'1010'0101'1010'0101)),
           Vec(T(0b1111'1111'1111'1111'1111'1111'1111'1111),  //
               T(0b1111'1111'1111'1111'1111'1111'1111'1111),  //
               T(0b1111'1111'1111'1111'1111'1111'1111'1111)),
           Val(UT(3)), Val(UT(8))},
          Vec(T(0b1111'0000'1111'0000'1111'0111'1111'1000),  //
              T(0b0000'1111'0000'1111'0000'1111'1111'1111),  //
              T(0b1010'0101'1010'0101'1010'0111'1111'1101))),
    };

    const char* error_msg =
        "12:34 error: 'offset + 'count' must be less than or equal to the bit width of 'e'";
    ConcatInto(  //
        r, std::vector<Case>{
               E({T(1), T(1), UT(33), UT(0)}, error_msg),         //
               E({T(1), T(1), UT(34), UT(0)}, error_msg),         //
               E({T(1), T(1), UT(1000), UT(0)}, error_msg),       //
               E({T(1), T(1), UT::Highest(), UT()}, error_msg),   //
               E({T(1), T(1), UT(0), UT(33)}, error_msg),         //
               E({T(1), T(1), UT(0), UT(34)}, error_msg),         //
               E({T(1), T(1), UT(0), UT(1000)}, error_msg),       //
               E({T(1), T(1), UT(0), UT::Highest()}, error_msg),  //
               E({T(1), T(1), UT(33), UT(33)}, error_msg),        //
               E({T(1), T(1), UT(34), UT(34)}, error_msg),        //
               E({T(1), T(1), UT(1000), UT(1000)}, error_msg),    //
               E({T(1), T(1), UT::Highest(), UT(1)}, error_msg),
               E({T(1), T(1), UT(1), UT::Highest()}, error_msg),
               E({T(1), T(1), UT::Highest(), u32::Highest()}, error_msg),
           });

    return r;
}
INSTANTIATE_TEST_SUITE_P(  //
    InsertBits,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kInsertBits),
                     testing::ValuesIn(Concat(InsertBitsCases<i32>(),  //
                                              InsertBitsCases<u32>()))));

template <typename T>
std::vector<Case> DegreesAFloatCases() {
    return std::vector<Case>{
        C({T(0)}, T(0)),                             //
        C({-T(0)}, -T(0)),                           //
        C({T(0.698132)}, T(40)).FloatComp(),         //
        C({-T(1.5708)}, -T(90.000214)).FloatComp(),  //
        C({T(1.5708)}, T(90.000214)).FloatComp(),    //
        C({T(6.28319)}, T(360.00027)).FloatComp(),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    DegreesAFloat,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kDegrees),
                     testing::ValuesIn(DegreesAFloatCases<AFloat>())));

template <typename T>
std::vector<Case> DegreesF32Cases() {
    return std::vector<Case>{
        C({T(0)}, T(0)),                             //
        C({-T(0)}, -T(0)),                           //
        C({T(0.698132)}, T(40)).FloatComp(),         //
        C({-T(1.5708)}, -T(90.000206)).FloatComp(),  //
        C({T(1.5708)}, T(90.000206)).FloatComp(),    //
        C({T(6.28319)}, T(360.00024)).FloatComp(),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    DegreesF32,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kDegrees),
                     testing::ValuesIn(DegreesF32Cases<f32>())));

template <typename T>
std::vector<Case> DegreesF16Cases() {
    return std::vector<Case>{
        C({T(0)}, T(0)),                            //
        C({-T(0)}, -T(0)),                          //
        C({T(0.698132)}, T(39.96875)).FloatComp(),  //
        C({-T(1.5708)}, -T(89.9375)).FloatComp(),   //
        C({T(1.5708)}, T(89.9375)).FloatComp(),     //
        C({T(6.28319)}, T(359.75)).FloatComp(),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    DegreesF16,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kDegrees),
                     testing::ValuesIn(DegreesF16Cases<f16>())));

template <typename T>
std::vector<Case> ExpCases() {
    auto error_msg = [](auto a) { return "12:34 error: " + OverflowExpErrorMessage("e", a); };
    return std::vector<Case>{C({T(0)}, T(1)),   //
                             C({-T(0)}, T(1)),  //
                             C({T(2)}, T(7.3890562)).FloatComp(),
                             C({-T(2)}, T(0.13533528)).FloatComp(),  //
                             C({T::Lowest()}, T(0)),

                             E({T::Highest()}, error_msg(T::Highest()))};
}
INSTANTIATE_TEST_SUITE_P(  //
    Exp,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kExp),
                     testing::ValuesIn(Concat(ExpCases<AFloat>(),  //
                                              ExpCases<f32>(),
                                              ExpCases<f16>()))));

template <typename T>
std::vector<Case> Exp2Cases() {
    auto error_msg = [](auto a) { return "12:34 error: " + OverflowExpErrorMessage("2", a); };
    return std::vector<Case>{
        C({T(0)}, T(1)),   //
        C({-T(0)}, T(1)),  //
        C({T(2)}, T(4.0)),
        C({-T(2)}, T(0.25)),  //
        C({T::Lowest()}, T(0)),

        E({T::Highest()}, error_msg(T::Highest())),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Exp2,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kExp2),
                     testing::ValuesIn(Concat(Exp2Cases<AFloat>(),  //
                                              Exp2Cases<f32>(),
                                              Exp2Cases<f16>()))));

template <typename T>
std::vector<Case> ExtractBitsCases() {
    using UT = Number<std::make_unsigned_t<UnwrapNumber<T>>>;

    // If T is signed, fills most significant bits of `val` with 1s
    auto set_msbs_if_signed = [](T val) {
        if constexpr (IsSignedIntegral<T>) {
            T result = T(~0);
            for (size_t b = 0; val; ++b) {
                if ((val & 1) == 0) {
                    result = result & ~(1 << b);  // Clear bit b
                }
                val = val >> 1;
            }
            return result;
        } else {
            return val;
        }
    };

    auto e = T(0b10100011110001011010001111000101);
    auto f = T(0b01010101010101010101010101010101);
    auto g = T(0b11111010001111000101101000111100);

    auto r = std::vector<Case>{
        // args: e, offset, count

        // If count is 0, result is 0
        C({e, UT(0), UT(0)}, T(0)),  //
        C({e, UT(1), UT(0)}, T(0)),  //
        C({e, UT(2), UT(0)}, T(0)),  //
        C({e, UT(3), UT(0)}, T(0)),
        // ...
        C({e, UT(29), UT(0)}, T(0)),  //
        C({e, UT(30), UT(0)}, T(0)),  //
        C({e, UT(31), UT(0)}, T(0)),

        // Extract at offset 0, varying counts
        C({e, UT(0), UT(1)}, set_msbs_if_signed(T(0b1))),    //
        C({e, UT(0), UT(2)}, T(0b01)),                       //
        C({e, UT(0), UT(3)}, set_msbs_if_signed(T(0b101))),  //
        C({e, UT(0), UT(4)}, T(0b0101)),                     //
        C({e, UT(0), UT(5)}, T(0b00101)),                    //
        C({e, UT(0), UT(6)}, T(0b000101)),                   //
        // ...
        C({e, UT(0), UT(28)}, T(0b0011110001011010001111000101)),                        //
        C({e, UT(0), UT(29)}, T(0b00011110001011010001111000101)),                       //
        C({e, UT(0), UT(30)}, set_msbs_if_signed(T(0b100011110001011010001111000101))),  //
        C({e, UT(0), UT(31)}, T(0b0100011110001011010001111000101)),                     //
        C({e, UT(0), UT(32)}, T(0b10100011110001011010001111000101)),                    //

        // Extract at varying offsets and counts
        C({e, UT(0), UT(1)}, set_msbs_if_signed(T(0b1))),                   //
        C({e, UT(31), UT(1)}, set_msbs_if_signed(T(0b1))),                  //
        C({e, UT(3), UT(5)}, set_msbs_if_signed(T(0b11000))),               //
        C({e, UT(4), UT(7)}, T(0b0111100)),                                 //
        C({e, UT(10), UT(16)}, set_msbs_if_signed(T(0b1111000101101000))),  //
        C({e, UT(10), UT(22)}, set_msbs_if_signed(T(0b1010001111000101101000))),

        // Vector tests
        C({Vec(e, f, g),                          //
           Val(UT(5)), Val(UT(8))},               //
          Vec(T(0b00011110),                      //
              set_msbs_if_signed(T(0b10101010)),  //
              set_msbs_if_signed(T(0b11010001)))),
    };

    const char* error_msg =
        "12:34 error: 'offset + 'count' must be less than or equal to the bit width of 'e'";
    ConcatInto(  //
        r, std::vector<Case>{
               E({T(1), UT(33), UT(0)}, error_msg),
               E({T(1), UT(34), UT(0)}, error_msg),
               E({T(1), UT(1000), UT(0)}, error_msg),
               E({T(1), UT::Highest(), UT(0)}, error_msg),
               E({T(1), UT(0), UT(33)}, error_msg),
               E({T(1), UT(0), UT(34)}, error_msg),
               E({T(1), UT(0), UT(1000)}, error_msg),
               E({T(1), UT(0), UT::Highest()}, error_msg),
               E({T(1), UT(33), UT(33)}, error_msg),
               E({T(1), UT(34), UT(34)}, error_msg),
               E({T(1), UT(1000), UT(1000)}, error_msg),
               E({T(1), UT::Highest(), UT(1)}, error_msg),
               E({T(1), UT(1), UT::Highest()}, error_msg),
               E({T(1), UT::Highest(), UT::Highest()}, error_msg),
           });

    return r;
}
INSTANTIATE_TEST_SUITE_P(  //
    ExtractBits,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kExtractBits),
                     testing::ValuesIn(Concat(ExtractBitsCases<i32>(),  //
                                              ExtractBitsCases<u32>()))));

template <typename T>
std::vector<Case> LengthCases() {
    const auto kSqrtOfHighest = T(std::sqrt(T::Highest()));
    const auto kSqrtOfHighestSquared = T(kSqrtOfHighest * kSqrtOfHighest);

    auto error_msg = [](auto a, const char* op, auto b) {
        return "12:34 error: " + OverflowErrorMessage(a, op, b) + R"(
12:34 note: when calculating length)";
    };
    return {
        C({T(0)}, T(0)),
        C({Vec(T(0), T(0))}, Val(T(0))),
        C({Vec(T(0), T(0), T(0))}, Val(T(0))),
        C({Vec(T(0), T(0), T(0), T(0))}, Val(T(0))),

        C({T(1)}, T(1)),
        C({Vec(T(1), T(1))}, Val(T(std::sqrt(2)))),
        C({Vec(T(1), T(1), T(1))}, Val(T(std::sqrt(3)))),
        C({Vec(T(1), T(1), T(1), T(1))}, Val(T(std::sqrt(4)))),

        C({T(2)}, T(2)),
        C({Vec(T(2), T(2))}, Val(T(std::sqrt(8)))),
        C({Vec(T(2), T(2), T(2))}, Val(T(std::sqrt(12)))),
        C({Vec(T(2), T(2), T(2), T(2))}, Val(T(std::sqrt(16)))),

        C({Vec(T(2), T(3))}, Val(T(std::sqrt(13)))),
        C({Vec(T(2), T(3), T(4))}, Val(T(std::sqrt(29)))),
        C({Vec(T(2), T(3), T(4), T(5))}, Val(T(std::sqrt(54)))),

        C({T(-5)}, T(5)),
        C({T::Highest()}, T::Highest()),
        C({T::Lowest()}, T::Highest()),

        C({Vec(T(-2), T(-3), T(-4), T(-5))}, Val(T(std::sqrt(54)))),
        C({Vec(T(2), T(-3), T(4), T(-5))}, Val(T(std::sqrt(54)))),
        C({Vec(T(-2), T(3), T(-4), T(5))}, Val(T(std::sqrt(54)))),

        C({Vec(kSqrtOfHighest, T(0))}, Val(kSqrtOfHighest)).FloatComp(0.2),
        C({Vec(T(0), kSqrtOfHighest)}, Val(kSqrtOfHighest)).FloatComp(0.2),

        C({Vec(-kSqrtOfHighest, T(0))}, Val(kSqrtOfHighest)).FloatComp(0.2),
        C({Vec(T(0), -kSqrtOfHighest)}, Val(kSqrtOfHighest)).FloatComp(0.2),

        // Overflow when squaring a term
        E({Vec(T::Highest(), T(0))}, error_msg(T::Highest(), "*", T::Highest())),
        E({Vec(T(0), T::Highest())}, error_msg(T::Highest(), "*", T::Highest())),
        // Overflow when adding squared terms
        E({Vec(kSqrtOfHighest, kSqrtOfHighest)},
          error_msg(kSqrtOfHighestSquared, "+", kSqrtOfHighestSquared)),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Length,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kLength),
                     testing::ValuesIn(Concat(LengthCases<AFloat>(),  //
                                              LengthCases<f32>(),
                                              LengthCases<f16>()))));

template <typename T>
std::vector<Case> LogCases() {
    auto error_msg = [] { return "12:34 error: log must be called with a value > 0"; };
    return std::vector<Case>{C({T(1)}, T(0)),                              //
                             C({T(54.598150033)}, T(4)).FloatComp(0.002),  //

                             E({T::Lowest()}, error_msg()), E({T(0)}, error_msg()),
                             E({-T(0)}, error_msg())};
}
INSTANTIATE_TEST_SUITE_P(  //
    Log,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kLog),
                     testing::ValuesIn(Concat(LogCases<AFloat>(),  //
                                              LogCases<f32>(),
                                              LogCases<f16>()))));
template <typename T>
std::vector<Case> LogF16Cases() {
    return std::vector<Case>{
        C({T::Highest()}, T(11.085938)).FloatComp(),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    LogF16,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kLog),
                     testing::ValuesIn(LogF16Cases<f16>())));
template <typename T>
std::vector<Case> LogF32Cases() {
    return std::vector<Case>{
        C({T::Highest()}, T(88.722839)).FloatComp(),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    LogF32,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kLog),
                     testing::ValuesIn(LogF32Cases<f32>())));

template <typename T>
std::vector<Case> LogAbstractCases() {
    return std::vector<Case>{
        C({T::Highest()}, T(709.78271)).FloatComp(),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    LogAbstract,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kLog),
                     testing::ValuesIn(LogAbstractCases<AFloat>())));

template <typename T>
std::vector<Case> Log2Cases() {
    auto error_msg = [] { return "12:34 error: log2 must be called with a value > 0"; };
    return std::vector<Case>{
        C({T(1)}, T(0)),  //
        C({T(4)}, T(2)),  //

        E({T::Lowest()}, error_msg()),
        E({T(0)}, error_msg()),
        E({-T(0)}, error_msg()),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Log2,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kLog2),
                     testing::ValuesIn(Concat(Log2Cases<AFloat>(),  //
                                              Log2Cases<f32>(),
                                              Log2Cases<f16>()))));
template <typename T>
std::vector<Case> Log2F16Cases() {
    return std::vector<Case>{
        C({T::Highest()}, T(15.9922)).FloatComp(),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Log2F16,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kLog2),
                     testing::ValuesIn(Log2F16Cases<f16>())));
template <typename T>
std::vector<Case> Log2F32Cases() {
    return std::vector<Case>{
        C({T::Highest()}, T(128)).FloatComp(),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Log2F32,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kLog2),
                     testing::ValuesIn(Log2F32Cases<f32>())));
template <typename T>
std::vector<Case> Log2AbstractCases() {
    return std::vector<Case>{
        C({T::Highest()}, T(1024)).FloatComp(),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Log2Abstract,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kLog2),
                     testing::ValuesIn(Log2AbstractCases<AFloat>())));

template <typename T>
std::vector<Case> MaxCases() {
    return {
        C({T(0), T(0)}, T(0)),
        C({T(0), T::Highest()}, T::Highest()),
        C({T::Lowest(), T(0)}, T(0)),
        C({T::Highest(), T::Lowest()}, T::Highest()),
        C({T::Highest(), T::Highest()}, T::Highest()),
        C({T::Lowest(), T::Lowest()}, T::Lowest()),

        // Vector tests
        C({Vec(T(0), T(0)), Vec(T(0), T(42))}, Vec(T(0), T(42))),
        C({Vec(T::Lowest(), T(0)), Vec(T(0), T::Lowest())}, Vec(T(0), T(0))),
        C({Vec(T::Lowest(), T::Highest()), Vec(T::Highest(), T::Lowest())},
          Vec(T::Highest(), T::Highest())),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Max,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kMax),
                     testing::ValuesIn(Concat(MaxCases<AInt>(),  //
                                              MaxCases<i32>(),
                                              MaxCases<u32>(),
                                              MaxCases<AFloat>(),
                                              MaxCases<f32>(),
                                              MaxCases<f16>()))));

template <typename T>
std::vector<Case> MinCases() {
    return {C({T(0), T(0)}, T(0)),                //
            C({T(0), T(42)}, T(0)),               //
            C({T::Lowest(), T(0)}, T::Lowest()),  //
            C({T(0), T::Highest()}, T(0)),        //
            C({T::Highest(), T::Lowest()}, T::Lowest()),
            C({T::Highest(), T::Highest()}, T::Highest()),
            C({T::Lowest(), T::Lowest()}, T::Lowest()),

            // Vector tests
            C({Vec(T(0), T(0)), Vec(T(0), T(42))}, Vec(T(0), T(0))),
            C({Vec(T::Lowest(), T(0), T(1)), Vec(T(0), T(42), T::Highest())},
              Vec(T::Lowest(), T(0), T(1)))};
}
INSTANTIATE_TEST_SUITE_P(  //
    Min,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kMin),
                     testing::ValuesIn(Concat(MinCases<AInt>(),  //
                                              MinCases<i32>(),
                                              MinCases<u32>(),
                                              MinCases<AFloat>(),
                                              MinCases<f32>(),
                                              MinCases<f16>()))));
template <typename T>
std::vector<Case> ModfCases() {
    return {
        // Scalar tests
        //  in     fract    whole
        C({T(0.0)}, {T(0.0), T(0.0)}),              //
        C({T(1.0)}, {T(0.0), T(1.0)}),              //
        C({T(2.0)}, {T(0.0), T(2.0)}),              //
        C({T(1.5)}, {T(0.5), T(1.0)}),              //
        C({T(4.25)}, {T(0.25), T(4.0)}),            //
        C({T(-1.0)}, {T(0.0), T(-1.0)}),            //
        C({T(-2.0)}, {T(0.0), T(-2.0)}),            //
        C({T(-1.5)}, {T(-0.5), T(-1.0)}),           //
        C({T(-4.25)}, {T(-0.25), T(-4.0)}),         //
        C({T::Lowest()}, {T(0.0), T::Lowest()}),    //
        C({T::Highest()}, {T(0.0), T::Highest()}),  //

        // Vector tests
        //         in                 fract                    whole
        C({Vec(T(0.0), T(0.0))}, {Vec(T(0.0), T(0.0)), Vec(T(0.0), T(0.0))}),
        C({Vec(T(1.0), T(2.0))}, {Vec(T(0.0), T(0.0)), Vec(T(1), T(2))}),
        C({Vec(T(-2.0), T(1.0))}, {Vec(T(0.0), T(0.0)), Vec(T(-2), T(1))}),
        C({Vec(T(1.5), T(-2.25))}, {Vec(T(0.5), T(-0.25)), Vec(T(1.0), T(-2.0))}),
        C({Vec(T::Lowest(), T::Highest())}, {Vec(T(0.0), T(0.0)), Vec(T::Lowest(), T::Highest())}),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Modf,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kModf),
                     testing::ValuesIn(Concat(ModfCases<AFloat>(),  //
                                              ModfCases<f32>(),     //
                                              ModfCases<f16>()))));

std::vector<Case> Pack4x8snormCases() {
    return {
        C({Vec(f32(0), f32(0), f32(0), f32(0))}, Val(u32(0x0000'0000))),
        C({Vec(f32(0), f32(0), f32(0), f32(-1))}, Val(u32(0x8100'0000))),
        C({Vec(f32(0), f32(0), f32(0), f32(1))}, Val(u32(0x7f00'0000))),
        C({Vec(f32(0), f32(0), f32(-1), f32(0))}, Val(u32(0x0081'0000))),
        C({Vec(f32(0), f32(1), f32(0), f32(0))}, Val(u32(0x0000'7f00))),
        C({Vec(f32(-1), f32(0), f32(0), f32(0))}, Val(u32(0x0000'0081))),
        C({Vec(f32(1), f32(-1), f32(1), f32(-1))}, Val(u32(0x817f'817f))),
        C({Vec(f32::Highest(), f32(-0.5), f32(0.5), f32::Lowest())}, Val(u32(0x8140'c17f))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Pack4x8snorm,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kPack4X8Snorm),
                     testing::ValuesIn(Pack4x8snormCases())));

std::vector<Case> Pack4x8unormCases() {
    return {
        C({Vec(f32(0), f32(0), f32(0), f32(0))}, Val(u32(0x0000'0000))),
        C({Vec(f32(0), f32(0), f32(0), f32(1))}, Val(u32(0xff00'0000))),
        C({Vec(f32(0), f32(0), f32(1), f32(0))}, Val(u32(0x00ff'0000))),
        C({Vec(f32(0), f32(1), f32(0), f32(0))}, Val(u32(0x0000'ff00))),
        C({Vec(f32(1), f32(0), f32(0), f32(0))}, Val(u32(0x0000'00ff))),
        C({Vec(f32(1), f32(0), f32(1), f32(0))}, Val(u32(0x00ff'00ff))),
        C({Vec(f32::Highest(), f32(0), f32(0.5), f32::Lowest())}, Val(u32(0x0080'00ff))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Pack4x8unorm,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kPack4X8Unorm),
                     testing::ValuesIn(Pack4x8unormCases())));

std::vector<Case> Pack2x16floatCases() {
    return {
        C({Vec(f32(f16::Lowest()), f32(f16::Highest()))}, Val(u32(0x7bff'fbff))),
        C({Vec(f32(1), f32(-1))}, Val(u32(0xbc00'3c00))),
        C({Vec(f32(0), f32(0))}, Val(u32(0x0000'0000))),
        C({Vec(f32(10), f32(-10.5))}, Val(u32(0xc940'4900))),

        E({Vec(f32(0), f32::Highest())},
          "12:34 error: value 3.4028234663852885981e+38 cannot be represented as 'f16'"),
        E({Vec(f32::Lowest(), f32(0))},
          "12:34 error: value -3.4028234663852885981e+38 cannot be represented as 'f16'"),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Pack2x16float,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kPack2X16Float),
                     testing::ValuesIn(Pack2x16floatCases())));

std::vector<Case> Pack2x16snormCases() {
    return {
        C({Vec(f32(0), f32(0))}, Val(u32(0x0000'0000))),
        C({Vec(f32(0), f32(-1))}, Val(u32(0x8001'0000))),
        C({Vec(f32(0), f32(1))}, Val(u32(0x7fff'0000))),
        C({Vec(f32(-1), f32(0))}, Val(u32(0x0000'8001))),
        C({Vec(f32(1), f32(0))}, Val(u32(0x0000'7fff))),
        C({Vec(f32(1), f32(-1))}, Val(u32(0x8001'7fff))),
        C({Vec(f32::Highest(), f32::Lowest())}, Val(u32(0x8001'7fff))),
        C({Vec(f32(-0.5), f32(0.5))}, Val(u32(0x4000'c001))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Pack2x16snorm,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kPack2X16Snorm),
                     testing::ValuesIn(Pack2x16snormCases())));

std::vector<Case> Pack2x16unormCases() {
    return {
        C({Vec(f32(0), f32(1))}, Val(u32(0xffff'0000))),
        C({Vec(f32(1), f32(0))}, Val(u32(0x0000'ffff))),
        C({Vec(f32(0.5), f32(0))}, Val(u32(0x0000'8000))),
        C({Vec(f32::Highest(), f32::Lowest())}, Val(u32(0x0000'ffff))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Pack2x16unorm,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kPack2X16Unorm),
                     testing::ValuesIn(Pack2x16unormCases())));

template <typename T>
std::vector<Case> ReverseBitsCases() {
    using B = BitValues<T>;
    return {
        C({T(0)}, T(0)),

        C({B::Lsh(1, 0)}, B::Lsh(1, 31)),  //
        C({B::Lsh(1, 1)}, B::Lsh(1, 30)),  //
        C({B::Lsh(1, 2)}, B::Lsh(1, 29)),  //
        C({B::Lsh(1, 3)}, B::Lsh(1, 28)),  //
        C({B::Lsh(1, 4)}, B::Lsh(1, 27)),  //
        //...
        C({B::Lsh(1, 27)}, B::Lsh(1, 4)),  //
        C({B::Lsh(1, 28)}, B::Lsh(1, 3)),  //
        C({B::Lsh(1, 29)}, B::Lsh(1, 2)),  //
        C({B::Lsh(1, 30)}, B::Lsh(1, 1)),  //
        C({B::Lsh(1, 31)}, B::Lsh(1, 0)),  //

        C({/**/ T(0b00010001000100010000000000000000)},
          /* */ T(0b00000000000000001000100010001000)),

        C({/**/ T(0b00011000000110000000000000000000)},
          /* */ T(0b00000000000000000001100000011000)),

        C({/**/ T(0b00000100000000001111111111111111)},
          /* */ T(0b11111111111111110000000000100000)),

        C({/**/ T(0b10010101111000110000011111101010)},
          /* */ T(0b01010111111000001100011110101001)),

        // Vector tests
        C({/**/ Vec(T(0b00010001000100010000000000000000),  //
                    T(0b00011000000110000000000000000000),  //
                    T(0b00000000000000001111111111111111))},
          /* */ Vec(T(0b00000000000000001000100010001000),  //
                    T(0b00000000000000000001100000011000),  //
                    T(0b11111111111111110000000000000000))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    ReverseBits,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kReverseBits),
                     testing::ValuesIn(Concat(ReverseBitsCases<i32>(),  //
                                              ReverseBitsCases<u32>()))));

template <typename T>
std::vector<Case> RadiansCases() {
    return std::vector<Case>{
        C({T(0)}, T(0)),                         //
        C({-T(0)}, -T(0)),                       //
        C({T(40)}, T(0.69813168)).FloatComp(),   //
        C({-T(90)}, -T(1.5707964)).FloatComp(),  //
        C({T(90)}, T(1.5707964)).FloatComp(),    //
        C({T(360)}, T(6.2831855)).FloatComp(),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Radians,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kRadians),
                     testing::ValuesIn(Concat(RadiansCases<AFloat>(),  //
                                              RadiansCases<f32>()))));

template <typename T>
std::vector<Case> RadiansF16Cases() {
    return std::vector<Case>{
        C({T(0)}, T(0)),                         //
        C({-T(0)}, -T(0)),                       //
        C({T(40)}, T(0.69726562)).FloatComp(),   //
        C({-T(90)}, -T(1.5693359)).FloatComp(),  //
        C({T(90)}, T(1.5693359)).FloatComp(),    //
        C({T(360)}, T(6.2773438)).FloatComp(),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    RadiansF16,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kRadians),
                     testing::ValuesIn(RadiansF16Cases<f16>())));

template <typename T>
std::vector<Case> RoundCases() {
    std::vector<Case> cases = {
        C({T(0.0)}, T(0.0)),      //
        C({-T(0.0)}, -T(0.0)),    //
        C({T(1.5)}, T(2.0)),      //
        C({T(2.5)}, T(2.0)),      //
        C({T(2.4)}, T(2.0)),      //
        C({T(2.6)}, T(3.0)),      //
        C({T(1.49999)}, T(1.0)),  //
        C({T(1.50001)}, T(2.0)),  //
        C({-T(1.5)}, -T(2.0)),    //
        C({-T(2.5)}, -T(2.0)),    //
        C({-T(2.6)}, -T(3.0)),    //
        C({-T(2.4)}, -T(2.0)),    //

        // Vector tests
        C({Vec(T(0.0), T(1.5), T(2.5))}, Vec(T(0.0), T(2.0), T(2.0))),
    };

    return cases;
}
INSTANTIATE_TEST_SUITE_P(  //
    Round,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kRound),
                     testing::ValuesIn(Concat(RoundCases<AFloat>(),  //
                                              RoundCases<f32>(),
                                              RoundCases<f16>()))));

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
std::vector<Case> SinCases() {
    std::vector<Case> cases = {
        C({-T(0)}, -T(0)),
        C({T(0)}, T(0)),
        C({T(0.75)}, T(0.68163876)).FloatComp(),
        C({-T(0.75)}, -T(0.68163876)).FloatComp(),

        // Vector test
        C({Vec(T(0), -T(0), T(0.75))}, Vec(T(0), -T(0), T(0.68163876))).FloatComp(),
    };

    return cases;
}
INSTANTIATE_TEST_SUITE_P(  //
    Sin,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kSin),
                     testing::ValuesIn(Concat(SinCases<AFloat>(),  //
                                              SinCases<f32>(),
                                              SinCases<f16>()))));

template <typename T>
std::vector<Case> SinhCases() {
    auto error_msg = [](auto a) {
        return "12:34 error: " + OverflowErrorMessage(a, FriendlyName<decltype(a)>());
    };
    std::vector<Case> cases = {
        C({T(0)}, T(0)),
        C({-T(0)}, -T(0)),
        C({T(1)}, T(1.1752012)).FloatComp(),
        C({T(-1)}, -T(1.1752012)).FloatComp(),

        // Vector tests
        C({Vec(T(0), -T(0), T(1))}, Vec(T(0), -T(0), T(1.1752012))).FloatComp(),

        E({T(10000)}, error_msg(T::Inf())),
    };
    return cases;
}
INSTANTIATE_TEST_SUITE_P(  //
    Sinh,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kSinh),
                     testing::ValuesIn(Concat(SinhCases<AFloat>(),  //
                                              SinhCases<f32>(),
                                              SinhCases<f16>()))));

template <typename T>
std::vector<Case> SmoothstepCases() {
    auto error_msg = [](auto a, const char* op, auto b) {
        return "12:34 error: " + OverflowErrorMessage(a, op, b) + R"(
12:34 note: when calculating smoothstep)";
    };
    return {
        // t == 0
        C({T(4), T(6), T(2)}, T(0)),
        // t == 1
        C({T(4), T(6), T(8)}, T(1)),
        // t == .5
        C({T(4), T(6), T(5)}, T(.5)),

        // Vector tests
        C({Vec(T(4), T(4)), Vec(T(6), T(6)), Vec(T(2), T(8))}, Vec(T(0), T(1))),

        // `x - low` underflows
        E({T::Highest(), T(1), T::Lowest()}, error_msg(T::Lowest(), "-", T::Highest())),
        // `high - low` underflows
        E({T::Highest(), T::Lowest(), T(0)}, error_msg(T::Lowest(), "-", T::Highest())),
        // Divide by zero on `(x - low) / (high - low)`
        E({T(0), T(0), T(0)}, error_msg(T(0), "/", T(0))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Smoothstep,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kSmoothstep),
                     testing::ValuesIn(Concat(SmoothstepCases<AFloat>(),  //
                                              SmoothstepCases<f32>(),
                                              SmoothstepCases<f16>()))));

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

template <typename T>
std::vector<Case> SqrtCases() {
    std::vector<Case> cases = {
        C({-T(0)}, -T(0)),  //
        C({T(0)}, T(0)),    //
        C({T(25)}, T(5)),

        // Vector tests
        C({Vec(T(25), T(100))}, Vec(T(5), T(10))),

        E({-T(25)}, "12:34 error: sqrt must be called with a value >= 0"),
    };
    return cases;
}
INSTANTIATE_TEST_SUITE_P(  //
    Sqrt,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kSqrt),
                     testing::ValuesIn(Concat(SqrtCases<AFloat>(),  //
                                              SqrtCases<f32>(),
                                              SqrtCases<f16>()))));

template <typename T>
std::vector<Case> TanCases() {
    std::vector<Case> cases = {
        C({-T(0)}, -T(0)),
        C({T(0)}, T(0)),
        C({T(.75)}, T(0.9315964599)).FloatComp(),

        // Vector test
        C({Vec(T(0), -T(0), T(.75))}, Vec(T(0), -T(0), T(0.9315964599))).FloatComp(),
    };

    return cases;
}
INSTANTIATE_TEST_SUITE_P(  //
    Tan,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kTan),
                     testing::ValuesIn(Concat(TanCases<AFloat>(),  //
                                              TanCases<f32>(),
                                              TanCases<f16>()))));

template <typename T>
std::vector<Case> TanhCases() {
    std::vector<Case> cases = {
        C({T(0)}, T(0)),
        C({-T(0)}, -T(0)),
        C({T(1)}, T(0.761594156)).FloatComp(),
        C({T(-1)}, -T(0.761594156)).FloatComp(),

        // Vector tests
        C({Vec(T(0), -T(0), T(1))}, Vec(T(0), -T(0), T(0.761594156))).FloatComp(),
    };

    return cases;
}
INSTANTIATE_TEST_SUITE_P(  //
    Tanh,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kTanh),
                     testing::ValuesIn(Concat(TanhCases<AFloat>(),  //
                                              TanhCases<f32>(),
                                              TanhCases<f16>()))));

template <typename T>
std::vector<Case> TransposeCases() {
    return {
        // 2x2
        C({Mat({T(1), T(2)},    //
               {T(3), T(4)})},  //
          Mat({T(1), T(3)},     //
              {T(2), T(4)})),

        // 3x3
        C({Mat({T(1), T(2), T(3)},    //
               {T(4), T(5), T(6)},    //
               {T(7), T(8), T(9)})},  //
          Mat({T(1), T(4), T(7)},     //
              {T(2), T(5), T(8)},     //
              {T(3), T(6), T(9)})),

        // 4x4
        C({Mat({T(1), T(2), T(3), T(4)},        //
               {T(5), T(6), T(7), T(8)},        //
               {T(9), T(10), T(11), T(12)},     //
               {T(13), T(14), T(15), T(16)})},  //
          Mat({T(1), T(5), T(9), T(13)},        //
              {T(2), T(6), T(10), T(14)},       //
              {T(3), T(7), T(11), T(15)},       //
              {T(4), T(8), T(12), T(16)})),

        // 4x2
        C({Mat({T(1), T(2), T(3), T(4)},    //
               {T(5), T(6), T(7), T(8)})},  //
          Mat({T(1), T(5)},                 //
              {T(2), T(6)},                 //
              {T(3), T(7)},                 //
              {T(4), T(8)})),

        // 2x4
        C({Mat({T(1), T(2)},             //
               {T(3), T(4)},             //
               {T(5), T(6)},             //
               {T(7), T(8)})},           //
          Mat({T(1), T(3), T(5), T(7)},  //
              {T(2), T(4), T(6), T(8)})),

        // 3x2
        C({Mat({T(1), T(2), T(3)},    //
               {T(4), T(5), T(6)})},  //
          Mat({T(1), T(4)},           //
              {T(2), T(5)},           //
              {T(3), T(6)})),

        // 2x3
        C({Mat({T(1), T(2)},       //
               {T(3), T(4)},       //
               {T(5), T(6)})},     //
          Mat({T(1), T(3), T(5)},  //
              {T(2), T(4), T(6)})),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Transpose,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kTranspose),
                     testing::ValuesIn(Concat(TransposeCases<AFloat>(),  //
                                              TransposeCases<f32>(),
                                              TransposeCases<f16>()))));

template <typename T>
std::vector<Case> TruncCases() {
    std::vector<Case> cases = {C({T(0)}, T(0)),    //
                               C({-T(0)}, -T(0)),  //
                               C({T(1.5)}, T(1)),  //
                               C({-T(1.5)}, -T(1)),

                               // Vector tests
                               C({Vec(T(0.0), T(1.5), -T(2.2))}, Vec(T(0), T(1), -T(2)))};

    return cases;
}
INSTANTIATE_TEST_SUITE_P(  //
    Trunc,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kTrunc),
                     testing::ValuesIn(Concat(TruncCases<AFloat>(),  //
                                              TruncCases<f32>(),
                                              TruncCases<f16>()))));

std::vector<Case> Unpack4x8snormCases() {
    return {
        C({Val(u32(0x0000'0000))}, Vec(f32(0), f32(0), f32(0), f32(0))),
        C({Val(u32(0x8100'0000))}, Vec(f32(0), f32(0), f32(0), f32(-1))),
        C({Val(u32(0x7f00'0000))}, Vec(f32(0), f32(0), f32(0), f32(1))),
        C({Val(u32(0x0081'0000))}, Vec(f32(0), f32(0), f32(-1), f32(0))),
        C({Val(u32(0x0000'7f00))}, Vec(f32(0), f32(1), f32(0), f32(0))),
        C({Val(u32(0x0000'0081))}, Vec(f32(-1), f32(0), f32(0), f32(0))),
        C({Val(u32(0x817f'817f))}, Vec(f32(1), f32(-1), f32(1), f32(-1))),
        C({Val(u32(0x816d'937f))},
          Vec(f32(1), f32(-0.8582677165354), f32(0.8582677165354), f32(-1))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Unpack4x8snorm,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kUnpack4X8Snorm),
                     testing::ValuesIn(Unpack4x8snormCases())));

std::vector<Case> Unpack4x8unormCases() {
    return {
        C({Val(u32(0x0000'0000))}, Vec(f32(0), f32(0), f32(0), f32(0))),
        C({Val(u32(0xff00'0000))}, Vec(f32(0), f32(0), f32(0), f32(1))),
        C({Val(u32(0x00ff'0000))}, Vec(f32(0), f32(0), f32(1), f32(0))),
        C({Val(u32(0x0000'ff00))}, Vec(f32(0), f32(1), f32(0), f32(0))),
        C({Val(u32(0x0000'00ff))}, Vec(f32(1), f32(0), f32(0), f32(0))),
        C({Val(u32(0x00ff'00ff))}, Vec(f32(1), f32(0), f32(1), f32(0))),
        C({Val(u32(0x0066'00ff))}, Vec(f32(1), f32(0), f32(0.4), f32(0))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Unpack4x8unorm,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kUnpack4X8Unorm),
                     testing::ValuesIn(Unpack4x8unormCases())));

std::vector<Case> Unpack2x16floatCases() {
    return {
        C({Val(u32(0x7bff'fbff))}, Vec(f32(f16::Lowest()), f32(f16::Highest()))),
        C({Val(u32(0xbc00'3c00))}, Vec(f32(1), f32(-1))),
        C({Val(u32(0x0000'0000))}, Vec(f32(0), f32(0))),
        C({Val(u32(0xc940'4900))}, Vec(f32(10), f32(-10.5))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Unpack2x16float,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kUnpack2X16Float),
                     testing::ValuesIn(Unpack2x16floatCases())));

std::vector<Case> Unpack2x16snormCases() {
    return {
        C({Val(u32(0x0000'0000))}, Vec(f32(0), f32(0))),
        C({Val(u32(0x8001'0000))}, Vec(f32(0), f32(-1))),
        C({Val(u32(0x7fff'0000))}, Vec(f32(0), f32(1))),
        C({Val(u32(0x0000'8001))}, Vec(f32(-1), f32(0))),
        C({Val(u32(0x0000'7fff))}, Vec(f32(1), f32(0))),
        C({Val(u32(0x8001'7fff))}, Vec(f32(1), f32(-1))),
        C({Val(u32(0x8001'7fff))}, Vec(f32(1), f32(-1))),
        C({Val(u32(0x4000'999a))}, Vec(f32(-0.80001220740379), f32(0.500015259254737))).FloatComp(),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Unpack2x16snorm,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kUnpack2X16Snorm),
                     testing::ValuesIn(Unpack2x16snormCases())));

std::vector<Case> Unpack2x16unormCases() {
    return {
        C({Val(u32(0xffff'0000))}, Vec(f32(0), f32(1))),
        C({Val(u32(0x0000'ffff))}, Vec(f32(1), f32(0))),
        C({Val(u32(0x0000'6666))}, Vec(f32(0.4), f32(0))),
        C({Val(u32(0x0000'ffff))}, Vec(f32(1), f32(0))),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    Unpack2x16unorm,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kUnpack2X16Unorm),
                     testing::ValuesIn(Unpack2x16unormCases())));

std::vector<Case> QuantizeToF16Cases() {
    return {
        C({0_f}, 0_f),    //
        C({-0_f}, -0_f),  //
        C({1_f}, 1_f),    //
        C({-1_f}, -1_f),  //

        //   0.00006106496 quantized to 0.000061035156 = 0x1p-14
        C({0.00006106496_f}, 0.000061035156_f),    //
        C({-0.00006106496_f}, -0.000061035156_f),  //

        //   1.0004883 quantized to 1.0 = 0x1p0
        C({1.0004883_f}, 1.0_f),    //
        C({-1.0004883_f}, -1.0_f),  //

        //   8196.0 quantized to 8192.0 = 0x1p13
        C({8196_f}, 8192_f),    //
        C({-8196_f}, -8192_f),  //

        // Value in subnormal f16 range
        C({0x0.034p-14_f}, 0x0.034p-14_f),    //
        C({-0x0.034p-14_f}, -0x0.034p-14_f),  //
        C({0x0.068p-14_f}, 0x0.068p-14_f),    //
        C({-0x0.068p-14_f}, -0x0.068p-14_f),  //

        //   0x0.06b7p-14 quantized to 0x0.068p-14
        C({0x0.06b7p-14_f}, 0x0.068p-14_f),    //
        C({-0x0.06b7p-14_f}, -0x0.068p-14_f),  //

        // Vector tests
        C({Vec(0_f, -0_f)}, Vec(0_f, -0_f)),  //
        C({Vec(1_f, -1_f)}, Vec(1_f, -1_f)),  //

        C({Vec(0.00006106496_f, -0.00006106496_f, 1.0004883_f, -1.0004883_f)},
          Vec(0.000061035156_f, -0.000061035156_f, 1.0_f, -1.0_f)),

        C({Vec(8196_f, 8192_f, 0x0.034p-14_f)}, Vec(8192_f, 8192_f, 0x0.034p-14_f)),

        C({Vec(0x0.034p-14_f, -0x0.034p-14_f, 0x0.068p-14_f, -0x0.068p-14_f)},
          Vec(0x0.034p-14_f, -0x0.034p-14_f, 0x0.068p-14_f, -0x0.068p-14_f)),

        // Value out of f16 range
        E({65504.003_f}, "12:34 error: value 65504.00390625 cannot be represented as 'f16'"),
        E({-65504.003_f}, "12:34 error: value -65504.00390625 cannot be represented as 'f16'"),
        E({0x1.234p56_f}, "12:34 error: value 81979586966978560 cannot be represented as 'f16'"),
        E({0x4.321p65_f},
          "12:34 error: value 1.5478871919272394752e+20 cannot be represented as 'f16'"),
        E({Vec(65504.003_f, 0_f)},
          "12:34 error: value 65504.00390625 cannot be represented as 'f16'"),
        E({Vec(0_f, -0x4.321p65_f)},
          "12:34 error: value -1.5478871919272394752e+20 cannot be represented as 'f16'"),
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    QuantizeToF16,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kQuantizeToF16),
                     testing::ValuesIn(QuantizeToF16Cases())));

}  // namespace
}  // namespace tint::resolver
