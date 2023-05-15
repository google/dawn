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

#include "src/tint/ir/builder.h"
#include "src/tint/ir/instruction.h"
#include "src/tint/ir/test_helper.h"

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT

using IR_InstructionTest = TestHelper;

TEST_F(IR_InstructionTest, CreateAnd) {
    Builder b;

    const auto* inst = b.And(b.ir.types.Get<type::I32>(), b.Constant(4_i), b.Constant(2_i));

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->kind, Binary::Kind::kAnd);
    ASSERT_NE(inst->result_type, nullptr);
    ASSERT_NE(inst->Type(), nullptr);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->value;
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->value;
    ASSERT_TRUE(rhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_InstructionTest, CreateOr) {
    Builder b;

    const auto* inst = b.Or(b.ir.types.Get<type::I32>(), b.Constant(4_i), b.Constant(2_i));

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->kind, Binary::Kind::kOr);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->value;
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->value;
    ASSERT_TRUE(rhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_InstructionTest, CreateXor) {
    Builder b;

    const auto* inst = b.Xor(b.ir.types.Get<type::I32>(), b.Constant(4_i), b.Constant(2_i));

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->kind, Binary::Kind::kXor);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->value;
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->value;
    ASSERT_TRUE(rhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_InstructionTest, CreateEqual) {
    Builder b;

    const auto* inst = b.Equal(b.ir.types.Get<type::Bool>(), b.Constant(4_i), b.Constant(2_i));

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->kind, Binary::Kind::kEqual);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->value;
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->value;
    ASSERT_TRUE(rhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_InstructionTest, CreateNotEqual) {
    Builder b;

    const auto* inst = b.NotEqual(b.ir.types.Get<type::Bool>(), b.Constant(4_i), b.Constant(2_i));

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->kind, Binary::Kind::kNotEqual);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->value;
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->value;
    ASSERT_TRUE(rhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_InstructionTest, CreateLessThan) {
    Builder b;

    const auto* inst = b.LessThan(b.ir.types.Get<type::Bool>(), b.Constant(4_i), b.Constant(2_i));

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->kind, Binary::Kind::kLessThan);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->value;
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->value;
    ASSERT_TRUE(rhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_InstructionTest, CreateGreaterThan) {
    Builder b;

    const auto* inst =
        b.GreaterThan(b.ir.types.Get<type::Bool>(), b.Constant(4_i), b.Constant(2_i));

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->kind, Binary::Kind::kGreaterThan);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->value;
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->value;
    ASSERT_TRUE(rhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_InstructionTest, CreateLessThanEqual) {
    Builder b;

    const auto* inst =
        b.LessThanEqual(b.ir.types.Get<type::Bool>(), b.Constant(4_i), b.Constant(2_i));

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->kind, Binary::Kind::kLessThanEqual);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->value;
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->value;
    ASSERT_TRUE(rhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_InstructionTest, CreateGreaterThanEqual) {
    Builder b;

    const auto* inst =
        b.GreaterThanEqual(b.ir.types.Get<type::Bool>(), b.Constant(4_i), b.Constant(2_i));

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->kind, Binary::Kind::kGreaterThanEqual);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->value;
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->value;
    ASSERT_TRUE(rhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_InstructionTest, CreateNot) {
    Builder b;
    const auto* inst = b.Not(b.ir.types.Get<type::Bool>(), b.Constant(true));

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->kind, Binary::Kind::kEqual);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->value;
    ASSERT_TRUE(lhs->Is<constant::Scalar<bool>>());
    EXPECT_TRUE(lhs->As<constant::Scalar<bool>>()->ValueAs<bool>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->value;
    ASSERT_TRUE(rhs->Is<constant::Scalar<bool>>());
    EXPECT_FALSE(rhs->As<constant::Scalar<bool>>()->ValueAs<bool>());
}

TEST_F(IR_InstructionTest, CreateShiftLeft) {
    Builder b;

    const auto* inst = b.ShiftLeft(b.ir.types.Get<type::I32>(), b.Constant(4_i), b.Constant(2_i));

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->kind, Binary::Kind::kShiftLeft);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->value;
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->value;
    ASSERT_TRUE(rhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_InstructionTest, CreateShiftRight) {
    Builder b;

    const auto* inst = b.ShiftRight(b.ir.types.Get<type::I32>(), b.Constant(4_i), b.Constant(2_i));

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->kind, Binary::Kind::kShiftRight);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->value;
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->value;
    ASSERT_TRUE(rhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_InstructionTest, CreateAdd) {
    Builder b;

    const auto* inst = b.Add(b.ir.types.Get<type::I32>(), b.Constant(4_i), b.Constant(2_i));

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->kind, Binary::Kind::kAdd);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->value;
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->value;
    ASSERT_TRUE(rhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_InstructionTest, CreateSubtract) {
    Builder b;

    const auto* inst = b.Subtract(b.ir.types.Get<type::I32>(), b.Constant(4_i), b.Constant(2_i));

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->kind, Binary::Kind::kSubtract);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->value;
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->value;
    ASSERT_TRUE(rhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_InstructionTest, CreateMultiply) {
    Builder b;

    const auto* inst = b.Multiply(b.ir.types.Get<type::I32>(), b.Constant(4_i), b.Constant(2_i));

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->kind, Binary::Kind::kMultiply);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->value;
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->value;
    ASSERT_TRUE(rhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_InstructionTest, CreateDivide) {
    Builder b;

    const auto* inst = b.Divide(b.ir.types.Get<type::I32>(), b.Constant(4_i), b.Constant(2_i));

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->kind, Binary::Kind::kDivide);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->value;
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->value;
    ASSERT_TRUE(rhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_InstructionTest, CreateModulo) {
    Builder b;

    const auto* inst = b.Modulo(b.ir.types.Get<type::I32>(), b.Constant(4_i), b.Constant(2_i));

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->kind, Binary::Kind::kModulo);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->value;
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->value;
    ASSERT_TRUE(rhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_InstructionTest, Binary_Usage) {
    Builder b;
    const auto* inst = b.And(b.ir.types.Get<type::I32>(), b.Constant(4_i), b.Constant(2_i));

    EXPECT_EQ(inst->kind, Binary::Kind::kAnd);

    ASSERT_NE(inst->LHS(), nullptr);
    ASSERT_EQ(inst->LHS()->Usage().Length(), 1u);
    EXPECT_EQ(inst->LHS()->Usage()[0], inst);

    ASSERT_NE(inst->RHS(), nullptr);
    ASSERT_EQ(inst->RHS()->Usage().Length(), 1u);
    EXPECT_EQ(inst->RHS()->Usage()[0], inst);
}

TEST_F(IR_InstructionTest, Binary_Usage_DuplicateValue) {
    Builder b;
    auto val = b.Constant(4_i);
    const auto* inst = b.And(b.ir.types.Get<type::I32>(), val, val);

    EXPECT_EQ(inst->kind, Binary::Kind::kAnd);
    ASSERT_EQ(inst->LHS(), inst->RHS());

    ASSERT_NE(inst->LHS(), nullptr);
    ASSERT_EQ(inst->LHS()->Usage().Length(), 1u);
    EXPECT_EQ(inst->LHS()->Usage()[0], inst);
}

}  // namespace
}  // namespace tint::ir
