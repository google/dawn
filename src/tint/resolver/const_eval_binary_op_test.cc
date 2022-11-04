// Copyright 2022 The Tint Authors.
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
using ::testing::HasSubstr;

namespace tint::resolver {
namespace {

// Bring in std::ostream& operator<<(std::ostream& o, const Types& types)
using resolver::operator<<;

struct Case {
    Types lhs;
    Types rhs;
    Types expected;
    bool overflow;
};

struct ErrorCase {
    Types lhs;
    Types rhs;
};

/// Creates a Case with Values of any type
template <typename T, typename U, typename V>
Case C(Value<T> lhs, Value<U> rhs, Value<V> expected, bool overflow = false) {
    return Case{std::move(lhs), std::move(rhs), std::move(expected), overflow};
}

/// Convenience overload that creates a Case with just scalars
template <typename T, typename U, typename V, typename = std::enable_if_t<!IsValue<T>>>
Case C(T lhs, U rhs, V expected, bool overflow = false) {
    return Case{Val(lhs), Val(rhs), Val(expected), overflow};
}

/// Prints Case to ostream
static std::ostream& operator<<(std::ostream& o, const Case& c) {
    o << "lhs: " << c.lhs << ", rhs: " << c.rhs << ", expected: " << c.expected
      << ", overflow: " << c.overflow;
    return o;
}

/// Prints ErrorCase to ostream
std::ostream& operator<<(std::ostream& o, const ErrorCase& c) {
    o << c.lhs << ", " << c.rhs;
    return o;
}

using ResolverConstEvalBinaryOpTest = ResolverTestWithParam<std::tuple<ast::BinaryOp, Case>>;
TEST_P(ResolverConstEvalBinaryOpTest, Test) {
    Enable(ast::Extension::kF16);
    auto op = std::get<0>(GetParam());
    auto& c = std::get<1>(GetParam());

    auto* expected = ToValueBase(c.expected);
    if (expected->IsAbstract() && c.overflow) {
        // Overflow is not allowed for abstract types. This is tested separately.
        return;
    }

    auto* lhs = ToValueBase(c.lhs);
    auto* rhs = ToValueBase(c.rhs);

    auto* lhs_expr = lhs->Expr(*this);
    auto* rhs_expr = rhs->Expr(*this);
    auto* expr = create<ast::BinaryExpression>(op, lhs_expr, rhs_expr);
    GlobalConst("C", expr);
    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    const sem::Constant* value = sem->ConstantValue();
    ASSERT_NE(value, nullptr);
    EXPECT_TYPE(value->Type(), sem->Type());

    auto values_flat = ScalarArgsFrom(value);
    auto expected_values_flat = expected->Args();
    ASSERT_EQ(values_flat.values.Length(), expected_values_flat.values.Length());
    for (size_t i = 0; i < values_flat.values.Length(); ++i) {
        auto& a = values_flat.values[i];
        auto& b = expected_values_flat.values[i];
        EXPECT_EQ(a, b);
        if (expected->IsIntegral()) {
            // Check that the constant's integer doesn't contain unexpected
            // data in the MSBs that are outside of the bit-width of T.
            EXPECT_EQ(builder::As<AInt>(a), builder::As<AInt>(b));
        }
    }
}

INSTANTIATE_TEST_SUITE_P(MixedAbstractArgs,
                         ResolverConstEvalBinaryOpTest,
                         testing::Combine(testing::Values(ast::BinaryOp::kAdd),
                                          testing::ValuesIn(std::vector{
                                              // Mixed abstract type args
                                              C(1_a, 2.3_a, 3.3_a),
                                              C(2.3_a, 1_a, 3.3_a),
                                          })));

template <typename T>
std::vector<Case> OpAddIntCases() {
    static_assert(IsIntegral<T>);
    return {
        C(T{0}, T{0}, T{0}),
        C(T{1}, T{2}, T{3}),
        C(T::Lowest(), T{1}, T{T::Lowest() + 1}),
        C(T::Highest(), Negate(T{1}), T{T::Highest() - 1}),
        C(T::Lowest(), T::Highest(), Negate(T{1})),
        C(T::Highest(), T{1}, T::Lowest(), true),
        C(T::Lowest(), Negate(T{1}), T::Highest(), true),
    };
}
template <typename T>
std::vector<Case> OpAddFloatCases() {
    static_assert(IsFloatingPoint<T>);
    return {
        C(T{0}, T{0}, T{0}),
        C(T{1}, T{2}, T{3}),
        C(T::Lowest(), T{1}, T{T::Lowest() + 1}),
        C(T::Highest(), Negate(T{1}), T{T::Highest() - 1}),
        C(T::Lowest(), T::Highest(), T{0}),
        C(T::Highest(), T::Highest(), T::Inf(), true),
        C(T::Lowest(), Negate(T::Highest()), -T::Inf(), true),
    };
}
INSTANTIATE_TEST_SUITE_P(Add,
                         ResolverConstEvalBinaryOpTest,
                         testing::Combine(testing::Values(ast::BinaryOp::kAdd),
                                          testing::ValuesIn(Concat(  //
                                              OpAddIntCases<AInt>(),
                                              OpAddIntCases<i32>(),
                                              OpAddIntCases<u32>(),
                                              OpAddFloatCases<AFloat>(),
                                              OpAddFloatCases<f32>(),
                                              OpAddFloatCases<f16>()))));

template <typename T>
std::vector<Case> OpSubIntCases() {
    static_assert(IsIntegral<T>);
    return {
        C(T{0}, T{0}, T{0}),
        C(T{3}, T{2}, T{1}),
        C(T{T::Lowest() + 1}, T{1}, T::Lowest()),
        C(T{T::Highest() - 1}, Negate(T{1}), T::Highest()),
        C(Negate(T{1}), T::Highest(), T::Lowest()),
        C(T::Lowest(), T{1}, T::Highest(), true),
        C(T::Highest(), Negate(T{1}), T::Lowest(), true),
    };
}
template <typename T>
std::vector<Case> OpSubFloatCases() {
    static_assert(IsFloatingPoint<T>);
    return {
        C(T{0}, T{0}, T{0}),
        C(T{3}, T{2}, T{1}),
        C(T::Highest(), T{1}, T{T::Highest() - 1}),
        C(T::Lowest(), Negate(T{1}), T{T::Lowest() + 1}),
        C(T{0}, T::Highest(), T::Lowest()),
        C(T::Highest(), Negate(T::Highest()), T::Inf(), true),
        C(T::Lowest(), T::Highest(), -T::Inf(), true),
    };
}
INSTANTIATE_TEST_SUITE_P(Sub,
                         ResolverConstEvalBinaryOpTest,
                         testing::Combine(testing::Values(ast::BinaryOp::kSubtract),
                                          testing::ValuesIn(Concat(  //
                                              OpSubIntCases<AInt>(),
                                              OpSubIntCases<i32>(),
                                              OpSubIntCases<u32>(),
                                              OpSubFloatCases<AFloat>(),
                                              OpSubFloatCases<f32>(),
                                              OpSubFloatCases<f16>()))));

template <typename T>
std::vector<Case> OpMulScalarCases() {
    return {
        C(T{0}, T{0}, T{0}),
        C(T{1}, T{2}, T{2}),
        C(T{2}, T{3}, T{6}),
        C(Negate(T{2}), T{3}, Negate(T{6})),
        C(T::Highest(), T{1}, T::Highest()),
        C(T::Lowest(), T{1}, T::Lowest()),
        C(T::Highest(), T::Highest(), Mul(T::Highest(), T::Highest()), true),
        C(T::Lowest(), T::Lowest(), Mul(T::Lowest(), T::Lowest()), true),
    };
}

template <typename T>
std::vector<Case> OpMulVecCases() {
    return {
        // s * vec3 = vec3
        C(Val(T{2.0}), Vec(T{1.25}, T{2.25}, T{3.25}), Vec(T{2.5}, T{4.5}, T{6.5})),
        // vec3 * s = vec3
        C(Vec(T{1.25}, T{2.25}, T{3.25}), Val(T{2.0}), Vec(T{2.5}, T{4.5}, T{6.5})),
        // vec3 * vec3 = vec3
        C(Vec(T{1.25}, T{2.25}, T{3.25}), Vec(T{2.0}, T{2.0}, T{2.0}), Vec(T{2.5}, T{4.5}, T{6.5})),
    };
}

template <typename T>
std::vector<Case> OpMulMatCases() {
    return {
        // s * mat3x2 = mat3x2
        C(Val(T{2.25}),
          Mat({T{1.0}, T{4.0}},  //
              {T{2.0}, T{5.0}},  //
              {T{3.0}, T{6.0}}),
          Mat({T{2.25}, T{9.0}},   //
              {T{4.5}, T{11.25}},  //
              {T{6.75}, T{13.5}})),
        // mat3x2 * s = mat3x2
        C(Mat({T{1.0}, T{4.0}},  //
              {T{2.0}, T{5.0}},  //
              {T{3.0}, T{6.0}}),
          Val(T{2.25}),
          Mat({T{2.25}, T{9.0}},   //
              {T{4.5}, T{11.25}},  //
              {T{6.75}, T{13.5}})),
        // vec3 * mat2x3 = vec2
        C(Vec(T{1.25}, T{2.25}, T{3.25}),  //
          Mat({T{1.0}, T{2.0}, T{3.0}},    //
              {T{4.0}, T{5.0}, T{6.0}}),   //
          Vec(T{15.5}, T{35.75})),
        // mat2x3 * vec2 = vec3
        C(Mat({T{1.0}, T{2.0}, T{3.0}},   //
              {T{4.0}, T{5.0}, T{6.0}}),  //
          Vec(T{1.25}, T{2.25}),          //
          Vec(T{10.25}, T{13.75}, T{17.25})),
        // mat3x2 * mat2x3 = mat2x2
        C(Mat({T{1.0}, T{2.0}},              //
              {T{3.0}, T{4.0}},              //
              {T{5.0}, T{6.0}}),             //
          Mat({T{1.25}, T{2.25}, T{3.25}},   //
              {T{4.25}, T{5.25}, T{6.25}}),  //
          Mat({T{24.25}, T{31.0}},           //
              {T{51.25}, T{67.0}})),         //
    };
}

INSTANTIATE_TEST_SUITE_P(Mul,
                         ResolverConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(ast::BinaryOp::kMultiply),
                             testing::ValuesIn(Concat(  //
                                 OpMulScalarCases<AInt>(),
                                 OpMulScalarCases<i32>(),
                                 OpMulScalarCases<u32>(),
                                 OpMulScalarCases<AFloat>(),
                                 OpMulScalarCases<f32>(),
                                 OpMulScalarCases<f16>(),
                                 OpMulVecCases<AInt>(),
                                 OpMulVecCases<i32>(),
                                 OpMulVecCases<u32>(),
                                 OpMulVecCases<AFloat>(),
                                 OpMulVecCases<f32>(),
                                 OpMulVecCases<f16>(),
                                 OpMulMatCases<AFloat>(),
                                 OpMulMatCases<f32>(),
                                 OpMulMatCases<f16>()))));

template <typename T>
std::vector<Case> OpDivIntCases() {
    std::vector<Case> r = {
        C(Val(T{0}), Val(T{1}), Val(T{0})),
        C(Val(T{1}), Val(T{1}), Val(T{1})),
        C(Val(T{1}), Val(T{1}), Val(T{1})),
        C(Val(T{2}), Val(T{1}), Val(T{2})),
        C(Val(T{4}), Val(T{2}), Val(T{2})),
        C(Val(T::Highest()), Val(T{1}), Val(T::Highest())),
        C(Val(T::Lowest()), Val(T{1}), Val(T::Lowest())),
        C(Val(T::Highest()), Val(T::Highest()), Val(T{1})),
        C(Val(T{0}), Val(T::Highest()), Val(T{0})),
        C(Val(T{0}), Val(T::Lowest()), Val(T{0})),
    };
    ConcatIntoIf<IsIntegral<T>>(  //
        r, std::vector<Case>{
               // e1, when e2 is zero.
               C(T{123}, T{0}, T{123}, true),
           });
    ConcatIntoIf<IsSignedIntegral<T>>(  //
        r, std::vector<Case>{
               // e1, when e1 is the most negative value in T, and e2 is -1.
               C(T::Smallest(), T{-1}, T::Smallest(), true),
           });
    return r;
}

template <typename T>
std::vector<Case> OpDivFloatCases() {
    return {
        C(Val(T{0}), Val(T{1}), Val(T{0})),
        C(Val(T{1}), Val(T{1}), Val(T{1})),
        C(Val(T{1}), Val(T{1}), Val(T{1})),
        C(Val(T{2}), Val(T{1}), Val(T{2})),
        C(Val(T{4}), Val(T{2}), Val(T{2})),
        C(Val(T::Highest()), Val(T{1}), Val(T::Highest())),
        C(Val(T::Lowest()), Val(T{1}), Val(T::Lowest())),
        C(Val(T::Highest()), Val(T::Highest()), Val(T{1})),
        C(Val(T{0}), Val(T::Highest()), Val(T{0})),
        C(Val(T{0}), Val(T::Lowest()), Val(-T{0})),
        C(T{123}, T{0}, T::Inf(), true),
        C(T{-123}, -T{0}, T::Inf(), true),
        C(T{-123}, T{0}, -T::Inf(), true),
        C(T{123}, -T{0}, -T::Inf(), true),
    };
}
INSTANTIATE_TEST_SUITE_P(Div,
                         ResolverConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(ast::BinaryOp::kDivide),
                             testing::ValuesIn(Concat(  //
                                 OpDivIntCases<AInt>(),
                                 OpDivIntCases<i32>(),
                                 OpDivIntCases<u32>(),
                                 OpDivFloatCases<AFloat>(),
                                 OpDivFloatCases<f32>(),
                                 OpDivFloatCases<f16>()))));

template <typename T, bool equals>
std::vector<Case> OpEqualCases() {
    return {
        C(Val(T{0}), Val(T{0}), Val(true == equals)),
        C(Val(T{0}), Val(T{1}), Val(false == equals)),
        C(Val(T{1}), Val(T{0}), Val(false == equals)),
        C(Val(T{1}), Val(T{1}), Val(true == equals)),
        C(Vec(T{0}, T{0}), Vec(T{0}, T{0}), Vec(true == equals, true == equals)),
        C(Vec(T{1}, T{0}), Vec(T{0}, T{1}), Vec(false == equals, false == equals)),
        C(Vec(T{1}, T{1}), Vec(T{0}, T{1}), Vec(false == equals, true == equals)),
    };
}
INSTANTIATE_TEST_SUITE_P(Equal,
                         ResolverConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(ast::BinaryOp::kEqual),
                             testing::ValuesIn(Concat(  //
                                 OpEqualCases<AInt, true>(),
                                 OpEqualCases<i32, true>(),
                                 OpEqualCases<u32, true>(),
                                 OpEqualCases<AFloat, true>(),
                                 OpEqualCases<f32, true>(),
                                 OpEqualCases<f16, true>(),
                                 OpEqualCases<bool, true>()))));
INSTANTIATE_TEST_SUITE_P(NotEqual,
                         ResolverConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(ast::BinaryOp::kNotEqual),
                             testing::ValuesIn(Concat(  //
                                 OpEqualCases<AInt, false>(),
                                 OpEqualCases<i32, false>(),
                                 OpEqualCases<u32, false>(),
                                 OpEqualCases<AFloat, false>(),
                                 OpEqualCases<f32, false>(),
                                 OpEqualCases<f16, false>(),
                                 OpEqualCases<bool, false>()))));

template <typename T, bool less_than>
std::vector<Case> OpLessThanCases() {
    return {
        C(Val(T{0}), Val(T{0}), Val(false == less_than)),
        C(Val(T{0}), Val(T{1}), Val(true == less_than)),
        C(Val(T{1}), Val(T{0}), Val(false == less_than)),
        C(Val(T{1}), Val(T{1}), Val(false == less_than)),
        C(Vec(T{0}, T{0}), Vec(T{0}, T{0}), Vec(false == less_than, false == less_than)),
        C(Vec(T{0}, T{0}), Vec(T{1}, T{1}), Vec(true == less_than, true == less_than)),
        C(Vec(T{1}, T{1}), Vec(T{0}, T{0}), Vec(false == less_than, false == less_than)),
        C(Vec(T{1}, T{0}), Vec(T{0}, T{1}), Vec(false == less_than, true == less_than)),
    };
}
INSTANTIATE_TEST_SUITE_P(LessThan,
                         ResolverConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(ast::BinaryOp::kLessThan),
                             testing::ValuesIn(Concat(  //
                                 OpLessThanCases<AInt, true>(),
                                 OpLessThanCases<i32, true>(),
                                 OpLessThanCases<u32, true>(),
                                 OpLessThanCases<AFloat, true>(),
                                 OpLessThanCases<f32, true>(),
                                 OpLessThanCases<f16, true>()))));
INSTANTIATE_TEST_SUITE_P(GreaterThanEqual,
                         ResolverConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(ast::BinaryOp::kGreaterThanEqual),
                             testing::ValuesIn(Concat(  //
                                 OpLessThanCases<AInt, false>(),
                                 OpLessThanCases<i32, false>(),
                                 OpLessThanCases<u32, false>(),
                                 OpLessThanCases<AFloat, false>(),
                                 OpLessThanCases<f32, false>(),
                                 OpLessThanCases<f16, false>()))));

template <typename T, bool greater_than>
std::vector<Case> OpGreaterThanCases() {
    return {
        C(Val(T{0}), Val(T{0}), Val(false == greater_than)),
        C(Val(T{0}), Val(T{1}), Val(false == greater_than)),
        C(Val(T{1}), Val(T{0}), Val(true == greater_than)),
        C(Val(T{1}), Val(T{1}), Val(false == greater_than)),
        C(Vec(T{0}, T{0}), Vec(T{0}, T{0}), Vec(false == greater_than, false == greater_than)),
        C(Vec(T{1}, T{1}), Vec(T{0}, T{0}), Vec(true == greater_than, true == greater_than)),
        C(Vec(T{0}, T{0}), Vec(T{1}, T{1}), Vec(false == greater_than, false == greater_than)),
        C(Vec(T{1}, T{0}), Vec(T{0}, T{1}), Vec(true == greater_than, false == greater_than)),
    };
}
INSTANTIATE_TEST_SUITE_P(GreaterThan,
                         ResolverConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(ast::BinaryOp::kGreaterThan),
                             testing::ValuesIn(Concat(  //
                                 OpGreaterThanCases<AInt, true>(),
                                 OpGreaterThanCases<i32, true>(),
                                 OpGreaterThanCases<u32, true>(),
                                 OpGreaterThanCases<AFloat, true>(),
                                 OpGreaterThanCases<f32, true>(),
                                 OpGreaterThanCases<f16, true>()))));
INSTANTIATE_TEST_SUITE_P(LessThanEqual,
                         ResolverConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(ast::BinaryOp::kLessThanEqual),
                             testing::ValuesIn(Concat(  //
                                 OpGreaterThanCases<AInt, false>(),
                                 OpGreaterThanCases<i32, false>(),
                                 OpGreaterThanCases<u32, false>(),
                                 OpGreaterThanCases<AFloat, false>(),
                                 OpGreaterThanCases<f32, false>(),
                                 OpGreaterThanCases<f16, false>()))));

static std::vector<Case> OpAndBoolCases() {
    return {
        C(true, true, true),
        C(true, false, false),
        C(false, true, false),
        C(false, false, false),
        C(Vec(true, true), Vec(true, false), Vec(true, false)),
        C(Vec(true, true), Vec(false, true), Vec(false, true)),
        C(Vec(true, false), Vec(true, false), Vec(true, false)),
        C(Vec(false, true), Vec(true, false), Vec(false, false)),
        C(Vec(false, false), Vec(true, false), Vec(false, false)),
    };
}
template <typename T>
std::vector<Case> OpAndIntCases() {
    using B = BitValues<T>;
    return {
        C(T{0b1010}, T{0b1111}, T{0b1010}),
        C(T{0b1010}, T{0b0000}, T{0b0000}),
        C(T{0b1010}, T{0b0011}, T{0b0010}),
        C(T{0b1010}, T{0b1100}, T{0b1000}),
        C(T{0b1010}, T{0b0101}, T{0b0000}),
        C(B::All, B::All, B::All),
        C(B::LeftMost, B::LeftMost, B::LeftMost),
        C(B::RightMost, B::RightMost, B::RightMost),
        C(B::All, T{0}, T{0}),
        C(T{0}, B::All, T{0}),
        C(B::LeftMost, B::AllButLeftMost, T{0}),
        C(B::AllButLeftMost, B::LeftMost, T{0}),
        C(B::RightMost, B::AllButRightMost, T{0}),
        C(B::AllButRightMost, B::RightMost, T{0}),
        C(Vec(B::All, B::LeftMost, B::RightMost),      //
          Vec(B::All, B::All, B::All),                 //
          Vec(B::All, B::LeftMost, B::RightMost)),     //
        C(Vec(B::All, B::LeftMost, B::RightMost),      //
          Vec(T{0}, T{0}, T{0}),                       //
          Vec(T{0}, T{0}, T{0})),                      //
        C(Vec(B::LeftMost, B::RightMost),              //
          Vec(B::AllButLeftMost, B::AllButRightMost),  //
          Vec(T{0}, T{0})),
    };
}
INSTANTIATE_TEST_SUITE_P(And,
                         ResolverConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(ast::BinaryOp::kAnd),
                             testing::ValuesIn(            //
                                 Concat(OpAndBoolCases(),  //
                                        OpAndIntCases<AInt>(),
                                        OpAndIntCases<i32>(),
                                        OpAndIntCases<u32>()))));

static std::vector<Case> OpOrBoolCases() {
    return {
        C(true, true, true),
        C(true, false, true),
        C(false, true, true),
        C(false, false, false),
        C(Vec(true, true), Vec(true, false), Vec(true, true)),
        C(Vec(true, true), Vec(false, true), Vec(true, true)),
        C(Vec(true, false), Vec(true, false), Vec(true, false)),
        C(Vec(false, true), Vec(true, false), Vec(true, true)),
        C(Vec(false, false), Vec(true, false), Vec(true, false)),
    };
}
template <typename T>
std::vector<Case> OpOrIntCases() {
    using B = BitValues<T>;
    return {
        C(T{0b1010}, T{0b1111}, T{0b1111}),
        C(T{0b1010}, T{0b0000}, T{0b1010}),
        C(T{0b1010}, T{0b0011}, T{0b1011}),
        C(T{0b1010}, T{0b1100}, T{0b1110}),
        C(T{0b1010}, T{0b0101}, T{0b1111}),
        C(B::All, B::All, B::All),
        C(B::LeftMost, B::LeftMost, B::LeftMost),
        C(B::RightMost, B::RightMost, B::RightMost),
        C(B::All, T{0}, B::All),
        C(T{0}, B::All, B::All),
        C(B::LeftMost, B::AllButLeftMost, B::All),
        C(B::AllButLeftMost, B::LeftMost, B::All),
        C(B::RightMost, B::AllButRightMost, B::All),
        C(B::AllButRightMost, B::RightMost, B::All),
        C(Vec(B::All, B::LeftMost, B::RightMost),      //
          Vec(B::All, B::All, B::All),                 //
          Vec(B::All, B::All, B::All)),                //
        C(Vec(B::All, B::LeftMost, B::RightMost),      //
          Vec(T{0}, T{0}, T{0}),                       //
          Vec(B::All, B::LeftMost, B::RightMost)),     //
        C(Vec(B::LeftMost, B::RightMost),              //
          Vec(B::AllButLeftMost, B::AllButRightMost),  //
          Vec(B::All, B::All)),
    };
}
INSTANTIATE_TEST_SUITE_P(Or,
                         ResolverConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(ast::BinaryOp::kOr),
                             testing::ValuesIn(Concat(OpOrBoolCases(),
                                                      OpOrIntCases<AInt>(),
                                                      OpOrIntCases<i32>(),
                                                      OpOrIntCases<u32>()))));

TEST_F(ResolverConstEvalTest, NotAndOrOfVecs) {
    // const C = !((vec2(true, true) & vec2(true, false)) | vec2(false, true));
    auto v1 = Vec(true, true).Expr(*this);
    auto v2 = Vec(true, false).Expr(*this);
    auto v3 = Vec(false, true).Expr(*this);
    auto expr = Not(Or(And(v1, v2), v3));
    GlobalConst("C", expr);
    auto expected_expr = Vec(false, false).Expr(*this);
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

    ForEachElemPair(value, expected_value, [&](const sem::Constant* a, const sem::Constant* b) {
        EXPECT_EQ(a->As<bool>(), b->As<bool>());
        return HasFailure() ? Action::kStop : Action::kContinue;
    });
}

template <typename T>
std::vector<Case> XorCases() {
    using B = BitValues<T>;
    return {
        C(T{0b1010}, T{0b1111}, T{0b0101}),
        C(T{0b1010}, T{0b0000}, T{0b1010}),
        C(T{0b1010}, T{0b0011}, T{0b1001}),
        C(T{0b1010}, T{0b1100}, T{0b0110}),
        C(T{0b1010}, T{0b0101}, T{0b1111}),
        C(B::All, B::All, T{0}),
        C(B::LeftMost, B::LeftMost, T{0}),
        C(B::RightMost, B::RightMost, T{0}),
        C(B::All, T{0}, B::All),
        C(T{0}, B::All, B::All),
        C(B::LeftMost, B::AllButLeftMost, B::All),
        C(B::AllButLeftMost, B::LeftMost, B::All),
        C(B::RightMost, B::AllButRightMost, B::All),
        C(B::AllButRightMost, B::RightMost, B::All),
        C(Vec(B::All, B::LeftMost, B::RightMost),             //
          Vec(B::All, B::All, B::All),                        //
          Vec(T{0}, B::AllButLeftMost, B::AllButRightMost)),  //
        C(Vec(B::All, B::LeftMost, B::RightMost),             //
          Vec(T{0}, T{0}, T{0}),                              //
          Vec(B::All, B::LeftMost, B::RightMost)),            //
        C(Vec(B::LeftMost, B::RightMost),                     //
          Vec(B::AllButLeftMost, B::AllButRightMost),         //
          Vec(B::All, B::All)),
    };
}
INSTANTIATE_TEST_SUITE_P(Xor,
                         ResolverConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(ast::BinaryOp::kXor),
                             testing::ValuesIn(Concat(XorCases<AInt>(),  //
                                                      XorCases<i32>(),   //
                                                      XorCases<u32>()))));

template <typename T>
std::vector<Case> ShiftLeftCases() {
    // Shift type is u32 for non-abstract
    using ST = std::conditional_t<IsAbstract<T>, T, u32>;
    using B = BitValues<T>;
    auto r = std::vector<Case>{
        C(T{0b1010}, ST{0}, T{0b0000'0000'1010}),  //
        C(T{0b1010}, ST{1}, T{0b0000'0001'0100}),  //
        C(T{0b1010}, ST{2}, T{0b0000'0010'1000}),  //
        C(T{0b1010}, ST{3}, T{0b0000'0101'0000}),  //
        C(T{0b1010}, ST{4}, T{0b0000'1010'0000}),  //
        C(T{0b1010}, ST{5}, T{0b0001'0100'0000}),  //
        C(T{0b1010}, ST{6}, T{0b0010'1000'0000}),  //
        C(T{0b1010}, ST{7}, T{0b0101'0000'0000}),  //
        C(T{0b1010}, ST{8}, T{0b1010'0000'0000}),  //
        C(B::LeftMost, ST{0}, B::LeftMost),        //

        C(Vec(T{0b1010}, T{0b1010}),                                            //
          Vec(ST{0}, ST{1}),                                                    //
          Vec(T{0b0000'0000'1010}, T{0b0000'0001'0100})),                       //
        C(Vec(T{0b1010}, T{0b1010}),                                            //
          Vec(ST{2}, ST{3}),                                                    //
          Vec(T{0b0000'0010'1000}, T{0b0000'0101'0000})),                       //
        C(Vec(T{0b1010}, T{0b1010}),                                            //
          Vec(ST{4}, ST{5}),                                                    //
          Vec(T{0b0000'1010'0000}, T{0b0001'0100'0000})),                       //
        C(Vec(T{0b1010}, T{0b1010}, T{0b1010}),                                 //
          Vec(ST{6}, ST{7}, ST{8}),                                             //
          Vec(T{0b0010'1000'0000}, T{0b0101'0000'0000}, T{0b1010'0000'0000})),  //
    };

    // Abstract 0 can be shifted by any u32 value (0 to 2^32), whereas concrete 0 (or any number)
    // can only by shifted by a value less than the number of bits of the lhs.
    // (see ResolverConstEvalShiftLeftConcreteGeqBitWidthError for negative tests)
    ConcatIntoIf<IsAbstract<T>>(  //
        r, std::vector<Case>{
               C(T{0}, ST{64}, T{0}),                              //
               C(T{0}, ST{65}, T{0}),                              //
               C(T{0}, ST{65}, T{0}),                              //
               C(T{0}, ST{10000}, T{0}),                           //
               C(T{0}, ST{u32::Highest()}, T{0}),                  //
               C(Negate(T{0}), ST{64}, Negate(T{0})),              //
               C(Negate(T{0}), ST{65}, Negate(T{0})),              //
               C(Negate(T{0}), ST{65}, Negate(T{0})),              //
               C(Negate(T{0}), ST{10000}, Negate(T{0})),           //
               C(Negate(T{0}), ST{u32::Highest()}, Negate(T{0})),  //
           });

    // Cases that are fine for signed values (no sign change), but would overflow unsigned values.
    // See ResolverConstEvalBinaryOpTest_Overflow for negative tests.
    ConcatIntoIf<IsSignedIntegral<T>>(  //
        r, std::vector<Case>{
               C(B::TwoLeftMost, ST{1}, B::LeftMost),      //
               C(B::All, ST{1}, B::AllButRightMost),       //
               C(B::All, ST{B::NumBits - 1}, B::LeftMost)  //
           });

    // Cases that are fine for unsigned values, but would overflow (sign change) signed
    // values. See ShiftLeftSignChangeErrorCases() for negative tests.
    ConcatIntoIf<IsUnsignedIntegral<T>>(  //
        r, std::vector<Case>{
               C(T{0b0001}, ST{B::NumBits - 1}, B::Lsh(0b0001, B::NumBits - 1)),
               C(T{0b0010}, ST{B::NumBits - 2}, B::Lsh(0b0010, B::NumBits - 2)),
               C(T{0b0100}, ST{B::NumBits - 3}, B::Lsh(0b0100, B::NumBits - 3)),
               C(T{0b1000}, ST{B::NumBits - 4}, B::Lsh(0b1000, B::NumBits - 4)),

               C(T{0b0011}, ST{B::NumBits - 2}, B::Lsh(0b0011, B::NumBits - 2)),
               C(T{0b0110}, ST{B::NumBits - 3}, B::Lsh(0b0110, B::NumBits - 3)),
               C(T{0b1100}, ST{B::NumBits - 4}, B::Lsh(0b1100, B::NumBits - 4)),

               C(B::AllButLeftMost, ST{1}, B::AllButRightMost),
           });

    return r;
}
INSTANTIATE_TEST_SUITE_P(ShiftLeft,
                         ResolverConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(ast::BinaryOp::kShiftLeft),
                             testing::ValuesIn(Concat(ShiftLeftCases<AInt>(),  //
                                                      ShiftLeftCases<i32>(),   //
                                                      ShiftLeftCases<u32>()))));

// Tests for errors on overflow/underflow of binary operations with abstract numbers
struct OverflowCase {
    ast::BinaryOp op;
    Types lhs;
    Types rhs;
};

static std::ostream& operator<<(std::ostream& o, const OverflowCase& c) {
    o << ast::FriendlyName(c.op) << ", lhs: " << c.lhs << ", rhs: " << c.rhs;
    return o;
}
using ResolverConstEvalBinaryOpTest_Overflow = ResolverTestWithParam<OverflowCase>;
TEST_P(ResolverConstEvalBinaryOpTest_Overflow, Test) {
    Enable(ast::Extension::kF16);
    auto& c = GetParam();
    auto* lhs = ToValueBase(c.lhs);
    auto* rhs = ToValueBase(c.rhs);
    auto* lhs_expr = lhs->Expr(*this);
    auto* rhs_expr = rhs->Expr(*this);
    auto* expr = create<ast::BinaryExpression>(Source{{1, 1}}, c.op, lhs_expr, rhs_expr);
    GlobalConst("C", expr);
    ASSERT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), HasSubstr("1:1 error: '"));
    EXPECT_THAT(r()->error(), HasSubstr("' cannot be represented as '" + lhs->TypeName() + "'"));
}
INSTANTIATE_TEST_SUITE_P(
    Test,
    ResolverConstEvalBinaryOpTest_Overflow,
    testing::Values(

        // scalar-scalar add
        OverflowCase{ast::BinaryOp::kAdd, Val(AInt::Highest()), Val(1_a)},
        OverflowCase{ast::BinaryOp::kAdd, Val(AInt::Lowest()), Val(-1_a)},
        OverflowCase{ast::BinaryOp::kAdd, Val(AFloat::Highest()), Val(AFloat::Highest())},
        OverflowCase{ast::BinaryOp::kAdd, Val(AFloat::Lowest()), Val(AFloat::Lowest())},
        // scalar-scalar subtract
        OverflowCase{ast::BinaryOp::kSubtract, Val(AInt::Lowest()), Val(1_a)},
        OverflowCase{ast::BinaryOp::kSubtract, Val(AInt::Highest()), Val(-1_a)},
        OverflowCase{ast::BinaryOp::kSubtract, Val(AFloat::Highest()), Val(AFloat::Lowest())},
        OverflowCase{ast::BinaryOp::kSubtract, Val(AFloat::Lowest()), Val(AFloat::Highest())},

        // scalar-scalar multiply
        OverflowCase{ast::BinaryOp::kMultiply, Val(AInt::Highest()), Val(2_a)},
        OverflowCase{ast::BinaryOp::kMultiply, Val(AInt::Lowest()), Val(-2_a)},

        // scalar-vector multiply
        OverflowCase{ast::BinaryOp::kMultiply, Val(AInt::Highest()), Vec(2_a, 1_a)},
        OverflowCase{ast::BinaryOp::kMultiply, Val(AInt::Lowest()), Vec(-2_a, 1_a)},

        // vector-matrix multiply

        // Overflow from first multiplication of dot product of vector and matrix column 0
        // i.e. (v[0] * m[0][0] + v[1] * m[0][1])
        //            ^
        OverflowCase{ast::BinaryOp::kMultiply,       //
                     Vec(AFloat::Highest(), 1.0_a),  //
                     Mat({2.0_a, 1.0_a},             //
                         {1.0_a, 1.0_a})},

        // Overflow from second multiplication of dot product of vector and matrix column 0
        // i.e. (v[0] * m[0][0] + v[1] * m[0][1])
        //                             ^
        OverflowCase{ast::BinaryOp::kMultiply,       //
                     Vec(1.0_a, AFloat::Highest()),  //
                     Mat({1.0_a, 2.0_a},             //
                         {1.0_a, 1.0_a})},

        // Overflow from addition of dot product of vector and matrix column 0
        // i.e. (v[0] * m[0][0] + v[1] * m[0][1])
        //                      ^
        OverflowCase{ast::BinaryOp::kMultiply,                   //
                     Vec(AFloat::Highest(), AFloat::Highest()),  //
                     Mat({1.0_a, 1.0_a},                         //
                         {1.0_a, 1.0_a})},

        // matrix-matrix multiply

        // Overflow from first multiplication of dot product of lhs row 0 and rhs column 0
        // i.e. m1[0][0] * m2[0][0] + m1[0][1] * m[1][0]
        //               ^
        OverflowCase{ast::BinaryOp::kMultiply,        //
                     Mat({AFloat::Highest(), 1.0_a},  //
                         {1.0_a, 1.0_a}),             //
                     Mat({2.0_a, 1.0_a},              //
                         {1.0_a, 1.0_a})},

        // Overflow from second multiplication of dot product of lhs row 0 and rhs column 0
        // i.e. m1[0][0] * m2[0][0] + m1[0][1] * m[1][0]
        //                                     ^
        OverflowCase{ast::BinaryOp::kMultiply,        //
                     Mat({1.0_a, AFloat::Highest()},  //
                         {1.0_a, 1.0_a}),             //
                     Mat({1.0_a, 1.0_a},              //
                         {2.0_a, 1.0_a})},

        // Overflow from addition of dot product of lhs row 0 and rhs column 0
        // i.e. m1[0][0] * m2[0][0] + m1[0][1] * m[1][0]
        //                          ^
        OverflowCase{ast::BinaryOp::kMultiply,         //
                     Mat({AFloat::Highest(), 1.0_a},   //
                         {AFloat::Highest(), 1.0_a}),  //
                     Mat({1.0_a, 1.0_a},               //
                         {1.0_a, 1.0_a})},

        // Divide by zero
        OverflowCase{ast::BinaryOp::kDivide, Val(123_a), Val(0_a)},
        OverflowCase{ast::BinaryOp::kDivide, Val(-123_a), Val(-0_a)},
        OverflowCase{ast::BinaryOp::kDivide, Val(-123_a), Val(0_a)},
        OverflowCase{ast::BinaryOp::kDivide, Val(123_a), Val(-0_a)},

        // Most negative value divided by -1
        OverflowCase{ast::BinaryOp::kDivide, Val(AInt::Lowest()), Val(-1_a)},

        // ShiftLeft of AInts that result in values not representable as AInts.
        // Note that for i32/u32, these would error because shift value is larger than 32.
        OverflowCase{ast::BinaryOp::kShiftLeft,                   //
                     Val(AInt{BitValues<AInt>::All}),             //
                     Val(AInt{BitValues<AInt>::NumBits})},        //
        OverflowCase{ast::BinaryOp::kShiftLeft,                   //
                     Val(AInt{BitValues<AInt>::RightMost}),       //
                     Val(AInt{BitValues<AInt>::NumBits})},        //
        OverflowCase{ast::BinaryOp::kShiftLeft,                   //
                     Val(AInt{BitValues<AInt>::AllButLeftMost}),  //
                     Val(AInt{BitValues<AInt>::NumBits})},        //
        OverflowCase{ast::BinaryOp::kShiftLeft,                   //
                     Val(AInt{BitValues<AInt>::AllButLeftMost}),  //
                     Val(AInt{BitValues<AInt>::NumBits + 1})},    //
        OverflowCase{ast::BinaryOp::kShiftLeft,                   //
                     Val(AInt{BitValues<AInt>::AllButLeftMost}),  //
                     Val(AInt{BitValues<AInt>::NumBits + 1000})},

        // ShiftLeft of u32s that overflow (non-zero bits get shifted out)
        OverflowCase{ast::BinaryOp::kShiftLeft, Val(0b00010_u), Val(31_u)},
        OverflowCase{ast::BinaryOp::kShiftLeft, Val(0b00100_u), Val(30_u)},
        OverflowCase{ast::BinaryOp::kShiftLeft, Val(0b01000_u), Val(29_u)},
        OverflowCase{ast::BinaryOp::kShiftLeft, Val(0b10000_u), Val(28_u)},
        // ...
        OverflowCase{ast::BinaryOp::kShiftLeft, Val(u32(1u << 28)), Val(4_u)},
        OverflowCase{ast::BinaryOp::kShiftLeft, Val(u32(1u << 29)), Val(3_u)},
        OverflowCase{ast::BinaryOp::kShiftLeft, Val(u32(1u << 30)), Val(2_u)},
        OverflowCase{ast::BinaryOp::kShiftLeft, Val(u32(1u << 31)), Val(1_u)},
        // And some more
        OverflowCase{ast::BinaryOp::kShiftLeft, Val(BitValues<u32>::All), Val(1_u)},
        OverflowCase{ast::BinaryOp::kShiftLeft, Val(BitValues<u32>::AllButLeftMost), Val(2_u)}

        ));

TEST_F(ResolverConstEvalTest, BinaryAbstractAddOverflow_AInt) {
    GlobalConst("c", Add(Source{{1, 1}}, Expr(AInt::Highest()), 1_a));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "1:1 error: '9223372036854775807 + 1' cannot be represented as 'abstract-int'");
}

TEST_F(ResolverConstEvalTest, BinaryAbstractAddUnderflow_AInt) {
    GlobalConst("c", Add(Source{{1, 1}}, Expr(AInt::Lowest()), -1_a));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "1:1 error: '-9223372036854775808 + -1' cannot be represented as 'abstract-int'");
}

TEST_F(ResolverConstEvalTest, BinaryAbstractAddOverflow_AFloat) {
    GlobalConst("c", Add(Source{{1, 1}}, Expr(AFloat::Highest()), AFloat::Highest()));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "1:1 error: '1.7976931348623157081e+308 + 1.7976931348623157081e+308' cannot be represented as 'abstract-float'");
}

TEST_F(ResolverConstEvalTest, BinaryAbstractAddUnderflow_AFloat) {
    GlobalConst("c", Add(Source{{1, 1}}, Expr(AFloat::Lowest()), AFloat::Lowest()));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        "1:1 error: '-1.7976931348623157081e+308 + -1.7976931348623157081e+308' cannot be represented as 'abstract-float'");
}

// Mixed AInt and AFloat args to test implicit conversion to AFloat
INSTANTIATE_TEST_SUITE_P(
    AbstractMixed,
    ResolverConstEvalBinaryOpTest,
    testing::Combine(
        testing::Values(ast::BinaryOp::kAdd),
        testing::Values(C(Val(1_a), Val(2.3_a), Val(3.3_a)),
                        C(Val(2.3_a), Val(1_a), Val(3.3_a)),
                        C(Val(1_a), Vec(2.3_a, 2.3_a, 2.3_a), Vec(3.3_a, 3.3_a, 3.3_a)),
                        C(Vec(2.3_a, 2.3_a, 2.3_a), Val(1_a), Vec(3.3_a, 3.3_a, 3.3_a)),
                        C(Vec(2.3_a, 2.3_a, 2.3_a), Val(1_a), Vec(3.3_a, 3.3_a, 3.3_a)),
                        C(Val(1_a), Vec(2.3_a, 2.3_a, 2.3_a), Vec(3.3_a, 3.3_a, 3.3_a)),
                        C(Mat({1_a, 2_a},        //
                              {1_a, 2_a},        //
                              {1_a, 2_a}),       //
                          Mat({1.2_a, 2.3_a},    //
                              {1.2_a, 2.3_a},    //
                              {1.2_a, 2.3_a}),   //
                          Mat({2.2_a, 4.3_a},    //
                              {2.2_a, 4.3_a},    //
                              {2.2_a, 4.3_a})),  //
                        C(Mat({1.2_a, 2.3_a},    //
                              {1.2_a, 2.3_a},    //
                              {1.2_a, 2.3_a}),   //
                          Mat({1_a, 2_a},        //
                              {1_a, 2_a},        //
                              {1_a, 2_a}),       //
                          Mat({2.2_a, 4.3_a},    //
                              {2.2_a, 4.3_a},    //
                              {2.2_a, 4.3_a}))   //
                        )));

// AInt left shift negative value -> error
TEST_F(ResolverConstEvalTest, BinaryAbstractShiftLeftByNegativeValue_Error) {
    GlobalConst("c", Shl(Expr(1_a), Expr(Source{{1, 1}}, -1_a)));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "1:1 error: value -1 cannot be represented as 'u32'");
}

// AInt left shift by AInt or u32 always results in an AInt
TEST_F(ResolverConstEvalTest, BinaryAbstractShiftLeftRemainsAbstract) {
    auto* expr1 = Shl(Expr(1_a), Expr(1_u));
    GlobalConst("c1", expr1);

    auto* expr2 = Shl(Expr(1_a), Expr(1_a));
    GlobalConst("c2", expr2);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem1 = Sem().Get(expr1);
    ASSERT_NE(sem1, nullptr);
    auto* sem2 = Sem().Get(expr2);
    ASSERT_NE(sem2, nullptr);

    auto aint_ty = create<sem::AbstractInt>();
    EXPECT_EQ(sem1->Type(), aint_ty);
    EXPECT_EQ(sem2->Type(), aint_ty);
}

// i32/u32 left shift by >= 32 -> error
using ResolverConstEvalShiftLeftConcreteGeqBitWidthError = ResolverTestWithParam<ErrorCase>;
TEST_P(ResolverConstEvalShiftLeftConcreteGeqBitWidthError, Test) {
    auto* lhs_expr = ToValueBase(GetParam().lhs)->Expr(*this);
    auto* rhs_expr = ToValueBase(GetParam().rhs)->Expr(*this);
    GlobalConst("c", Shl(Source{{1, 1}}, lhs_expr, rhs_expr));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        "1:1 error: shift left value must be less than the bit width of the lhs, which is 32");
}
INSTANTIATE_TEST_SUITE_P(Test,
                         ResolverConstEvalShiftLeftConcreteGeqBitWidthError,
                         testing::Values(                                       //
                             ErrorCase{Val(0_u), Val(32_u)},                    //
                             ErrorCase{Val(0_u), Val(33_u)},                    //
                             ErrorCase{Val(0_u), Val(34_u)},                    //
                             ErrorCase{Val(0_u), Val(10000_u)},                 //
                             ErrorCase{Val(0_u), Val(u32::Highest())},          //
                             ErrorCase{Val(0_i), Val(32_u)},                    //
                             ErrorCase{Val(0_i), Val(33_u)},                    //
                             ErrorCase{Val(0_i), Val(34_u)},                    //
                             ErrorCase{Val(0_i), Val(10000_u)},                 //
                             ErrorCase{Val(0_i), Val(u32::Highest())},          //
                             ErrorCase{Val(Negate(0_u)), Val(32_u)},            //
                             ErrorCase{Val(Negate(0_u)), Val(33_u)},            //
                             ErrorCase{Val(Negate(0_u)), Val(34_u)},            //
                             ErrorCase{Val(Negate(0_u)), Val(10000_u)},         //
                             ErrorCase{Val(Negate(0_u)), Val(u32::Highest())},  //
                             ErrorCase{Val(Negate(0_i)), Val(32_u)},            //
                             ErrorCase{Val(Negate(0_i)), Val(33_u)},            //
                             ErrorCase{Val(Negate(0_i)), Val(34_u)},            //
                             ErrorCase{Val(Negate(0_i)), Val(10000_u)},         //
                             ErrorCase{Val(Negate(0_i)), Val(u32::Highest())},  //
                             ErrorCase{Val(1_i), Val(32_u)},                    //
                             ErrorCase{Val(1_i), Val(33_u)},                    //
                             ErrorCase{Val(1_i), Val(34_u)},                    //
                             ErrorCase{Val(1_i), Val(10000_u)},                 //
                             ErrorCase{Val(1_i), Val(u32::Highest())},          //
                             ErrorCase{Val(1_u), Val(32_u)},                    //
                             ErrorCase{Val(1_u), Val(33_u)},                    //
                             ErrorCase{Val(1_u), Val(34_u)},                    //
                             ErrorCase{Val(1_u), Val(10000_u)},                 //
                             ErrorCase{Val(1_u), Val(u32::Highest())}           //
                             ));

// AInt left shift results in sign change error
using ResolverConstEvalShiftLeftSignChangeError = ResolverTestWithParam<ErrorCase>;
TEST_P(ResolverConstEvalShiftLeftSignChangeError, Test) {
    auto* lhs_expr = ToValueBase(GetParam().lhs)->Expr(*this);
    auto* rhs_expr = ToValueBase(GetParam().rhs)->Expr(*this);
    GlobalConst("c", Shl(Source{{1, 1}}, lhs_expr, rhs_expr));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "1:1 error: shift left operation results in sign change");
}
template <typename T>
std::vector<ErrorCase> ShiftLeftSignChangeErrorCases() {
    // Shift type is u32 for non-abstract
    using ST = std::conditional_t<IsAbstract<T>, T, u32>;
    using B = BitValues<T>;
    return {
        {Val(T{0b0001}), Val(ST{B::NumBits - 1})},
        {Val(T{0b0010}), Val(ST{B::NumBits - 2})},
        {Val(T{0b0100}), Val(ST{B::NumBits - 3})},
        {Val(T{0b1000}), Val(ST{B::NumBits - 4})},
        {Val(T{0b0011}), Val(ST{B::NumBits - 2})},
        {Val(T{0b0110}), Val(ST{B::NumBits - 3})},
        {Val(T{0b1100}), Val(ST{B::NumBits - 4})},
        {Val(B::AllButLeftMost), Val(ST{1})},
        {Val(B::AllButLeftMost), Val(ST{B::NumBits - 1})},
        {Val(B::LeftMost), Val(ST{1})},
        {Val(B::LeftMost), Val(ST{B::NumBits - 1})},
    };
}
INSTANTIATE_TEST_SUITE_P(Test,
                         ResolverConstEvalShiftLeftSignChangeError,
                         testing::ValuesIn(Concat(  //
                             ShiftLeftSignChangeErrorCases<AInt>(),
                             ShiftLeftSignChangeErrorCases<i32>())));

}  // namespace
}  // namespace tint::resolver
