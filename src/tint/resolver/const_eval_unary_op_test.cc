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

namespace tint::resolver {
namespace {

// Bring in std::ostream& operator<<(std::ostream& o, const Types& types)
using resolver::operator<<;

struct Case {
    Types input;
    Types expected;
};

static std::ostream& operator<<(std::ostream& o, const Case& c) {
    o << "input: " << c.input << ", expected: " << c.expected;
    return o;
}

/// Creates a Case with Values of any type
template <typename T, typename U>
Case C(Value<T> input, Value<U> expected) {
    return Case{std::move(input), std::move(expected)};
}

/// Convenience overload to creates a Case with just scalars
template <typename T, typename U, typename = std::enable_if_t<!IsValue<T>>>
Case C(T input, U expected) {
    return Case{Val(input), Val(expected)};
}

using ResolverConstEvalUnaryOpTest = ResolverTestWithParam<std::tuple<ast::UnaryOp, Case>>;

TEST_P(ResolverConstEvalUnaryOpTest, Test) {
    Enable(ast::Extension::kF16);

    auto op = std::get<0>(GetParam());
    auto& c = std::get<1>(GetParam());
    std::visit(
        [&](auto&& expected) {
            using T = typename std::decay_t<decltype(expected)>::ElementType;

            auto* input_expr = std::visit([&](auto&& value) { return value.Expr(*this); }, c.input);
            auto* expr = create<ast::UnaryOpExpression>(op, input_expr);

            GlobalConst("C", expr);
            auto* expected_expr = expected.Expr(*this);
            GlobalConst("E", expected_expr);
            ASSERT_TRUE(r()->Resolve()) << r()->error();

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
                                EXPECT_EQ(a->As<T>(), b->As<T>());
                                if constexpr (IsIntegral<T>) {
                                    // Check that the constant's integer doesn't contain unexpected
                                    // data in the MSBs that are outside of the bit-width of T.
                                    EXPECT_EQ(a->As<AInt>(), b->As<AInt>());
                                }
                                return HasFailure() ? Action::kStop : Action::kContinue;
                            });
        },
        c.expected);
}
INSTANTIATE_TEST_SUITE_P(Complement,
                         ResolverConstEvalUnaryOpTest,
                         testing::Combine(testing::Values(ast::UnaryOp::kComplement),
                                          testing::ValuesIn({
                                              // AInt
                                              C(0_a, 0xffffffffffffffff_a),
                                              C(0xffffffffffffffff_a, 0_a),
                                              C(0xf0f0f0f0f0f0f0f0_a, 0x0f0f0f0f0f0f0f0f_a),
                                              C(0xaaaaaaaaaaaaaaaa_a, 0x5555555555555555_a),
                                              C(0x5555555555555555_a, 0xaaaaaaaaaaaaaaaa_a),
                                              // u32
                                              C(0_u, 0xffffffff_u),
                                              C(0xffffffff_u, 0_u),
                                              C(0xf0f0f0f0_u, 0x0f0f0f0f_u),
                                              C(0xaaaaaaaa_u, 0x55555555_u),
                                              C(0x55555555_u, 0xaaaaaaaa_u),
                                              // i32
                                              C(0_i, -1_i),
                                              C(-1_i, 0_i),
                                              C(1_i, -2_i),
                                              C(-2_i, 1_i),
                                              C(2_i, -3_i),
                                              C(-3_i, 2_i),
                                          })));

INSTANTIATE_TEST_SUITE_P(Negation,
                         ResolverConstEvalUnaryOpTest,
                         testing::Combine(testing::Values(ast::UnaryOp::kNegation),
                                          testing::ValuesIn({
                                              // AInt
                                              C(0_a, -0_a),
                                              C(-0_a, 0_a),
                                              C(1_a, -1_a),
                                              C(-1_a, 1_a),
                                              C(AInt::Highest(), -AInt::Highest()),
                                              C(-AInt::Highest(), AInt::Highest()),
                                              C(AInt::Lowest(), Negate(AInt::Lowest())),
                                              C(Negate(AInt::Lowest()), AInt::Lowest()),
                                              // i32
                                              C(0_i, -0_i),
                                              C(-0_i, 0_i),
                                              C(1_i, -1_i),
                                              C(-1_i, 1_i),
                                              C(i32::Highest(), -i32::Highest()),
                                              C(-i32::Highest(), i32::Highest()),
                                              C(i32::Lowest(), Negate(i32::Lowest())),
                                              C(Negate(i32::Lowest()), i32::Lowest()),
                                              // AFloat
                                              C(0.0_a, -0.0_a),
                                              C(-0.0_a, 0.0_a),
                                              C(1.0_a, -1.0_a),
                                              C(-1.0_a, 1.0_a),
                                              C(AFloat::Highest(), -AFloat::Highest()),
                                              C(-AFloat::Highest(), AFloat::Highest()),
                                              C(AFloat::Lowest(), Negate(AFloat::Lowest())),
                                              C(Negate(AFloat::Lowest()), AFloat::Lowest()),
                                              // f32
                                              C(0.0_f, -0.0_f),
                                              C(-0.0_f, 0.0_f),
                                              C(1.0_f, -1.0_f),
                                              C(-1.0_f, 1.0_f),
                                              C(f32::Highest(), -f32::Highest()),
                                              C(-f32::Highest(), f32::Highest()),
                                              C(f32::Lowest(), Negate(f32::Lowest())),
                                              C(Negate(f32::Lowest()), f32::Lowest()),
                                              // f16
                                              C(0.0_h, -0.0_h),
                                              C(-0.0_h, 0.0_h),
                                              C(1.0_h, -1.0_h),
                                              C(-1.0_h, 1.0_h),
                                              C(f16::Highest(), -f16::Highest()),
                                              C(-f16::Highest(), f16::Highest()),
                                              C(f16::Lowest(), Negate(f16::Lowest())),
                                              C(Negate(f16::Lowest()), f16::Lowest()),
                                          })));

// Make sure UBSan doesn't trip on C++'s undefined behaviour of negating the smallest negative
// number.
TEST_F(ResolverConstEvalTest, UnaryNegateLowestAbstract) {
    // const break_me = -(-9223372036854775808);
    auto* c = GlobalConst("break_me", Negation(Negation(Expr(9223372036854775808_a))));
    (void)c;
    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(c);
    EXPECT_EQ(sem->ConstantValue()->As<AInt>(), 9223372036854775808_a);
}

INSTANTIATE_TEST_SUITE_P(Not,
                         ResolverConstEvalUnaryOpTest,
                         testing::Combine(testing::Values(ast::UnaryOp::kNot),
                                          testing::ValuesIn({
                                              C(true, false),
                                              C(false, true),
                                              C(Vec(true, true), Vec(false, false)),
                                              C(Vec(true, false), Vec(false, true)),
                                              C(Vec(false, true), Vec(true, false)),
                                              C(Vec(false, false), Vec(true, true)),
                                          })));

}  // namespace
}  // namespace tint::resolver
