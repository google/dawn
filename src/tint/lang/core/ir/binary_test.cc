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

#include "gmock/gmock.h"
#include "gtest/gtest-spi.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT

using IR_BinaryTest = IRTestHelper;

TEST_F(IR_BinaryTest, Fail_NullType) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            b.Add(nullptr, u32(1), u32(2));
        },
        "");
}

TEST_F(IR_BinaryTest, Result) {
    auto* a = b.Add(mod.Types().i32(), 4_i, 2_i);

    EXPECT_TRUE(a->HasResults());
    EXPECT_FALSE(a->HasMultiResults());
    EXPECT_TRUE(a->Result()->Is<InstructionResult>());
    EXPECT_EQ(a, a->Result()->Source());
}

TEST_F(IR_BinaryTest, CreateAnd) {
    auto* inst = b.And(mod.Types().i32(), 4_i, 2_i);

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Kind(), Binary::Kind::kAnd);
    ASSERT_NE(inst->Results()[0]->Type(), nullptr);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateOr) {
    auto* inst = b.Or(mod.Types().i32(), 4_i, 2_i);

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Kind(), Binary::Kind::kOr);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateXor) {
    auto* inst = b.Xor(mod.Types().i32(), 4_i, 2_i);

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Kind(), Binary::Kind::kXor);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateEqual) {
    auto* inst = b.Equal(mod.Types().bool_(), 4_i, 2_i);

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Kind(), Binary::Kind::kEqual);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateNotEqual) {
    auto* inst = b.NotEqual(mod.Types().bool_(), 4_i, 2_i);

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Kind(), Binary::Kind::kNotEqual);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateLessThan) {
    auto* inst = b.LessThan(mod.Types().bool_(), 4_i, 2_i);

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Kind(), Binary::Kind::kLessThan);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateGreaterThan) {
    auto* inst = b.GreaterThan(mod.Types().bool_(), 4_i, 2_i);

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Kind(), Binary::Kind::kGreaterThan);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateLessThanEqual) {
    auto* inst = b.LessThanEqual(mod.Types().bool_(), 4_i, 2_i);

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Kind(), Binary::Kind::kLessThanEqual);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateGreaterThanEqual) {
    auto* inst = b.GreaterThanEqual(mod.Types().bool_(), 4_i, 2_i);

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Kind(), Binary::Kind::kGreaterThanEqual);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateNot) {
    auto* inst = b.Not(mod.Types().bool_(), true);

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Kind(), Binary::Kind::kEqual);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<constant::Scalar<bool>>());
    EXPECT_TRUE(lhs->As<constant::Scalar<bool>>()->ValueAs<bool>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<constant::Scalar<bool>>());
    EXPECT_FALSE(rhs->As<constant::Scalar<bool>>()->ValueAs<bool>());
}

TEST_F(IR_BinaryTest, CreateShiftLeft) {
    auto* inst = b.ShiftLeft(mod.Types().i32(), 4_i, 2_i);

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Kind(), Binary::Kind::kShiftLeft);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateShiftRight) {
    auto* inst = b.ShiftRight(mod.Types().i32(), 4_i, 2_i);

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Kind(), Binary::Kind::kShiftRight);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateAdd) {
    auto* inst = b.Add(mod.Types().i32(), 4_i, 2_i);

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Kind(), Binary::Kind::kAdd);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateSubtract) {
    auto* inst = b.Subtract(mod.Types().i32(), 4_i, 2_i);

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Kind(), Binary::Kind::kSubtract);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateMultiply) {
    auto* inst = b.Multiply(mod.Types().i32(), 4_i, 2_i);

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Kind(), Binary::Kind::kMultiply);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateDivide) {
    auto* inst = b.Divide(mod.Types().i32(), 4_i, 2_i);

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Kind(), Binary::Kind::kDivide);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateModulo) {
    auto* inst = b.Modulo(mod.Types().i32(), 4_i, 2_i);

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Kind(), Binary::Kind::kModulo);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, Binary_Usage) {
    auto* inst = b.And(mod.Types().i32(), 4_i, 2_i);

    EXPECT_EQ(inst->Kind(), Binary::Kind::kAnd);

    ASSERT_NE(inst->LHS(), nullptr);
    EXPECT_THAT(inst->LHS()->Usages(), testing::UnorderedElementsAre(Usage{inst, 0u}));

    ASSERT_NE(inst->RHS(), nullptr);
    EXPECT_THAT(inst->RHS()->Usages(), testing::UnorderedElementsAre(Usage{inst, 1u}));
}

TEST_F(IR_BinaryTest, Binary_Usage_DuplicateValue) {
    auto val = 4_i;
    auto* inst = b.And(mod.Types().i32(), val, val);

    EXPECT_EQ(inst->Kind(), Binary::Kind::kAnd);
    ASSERT_EQ(inst->LHS(), inst->RHS());

    ASSERT_NE(inst->LHS(), nullptr);
    EXPECT_THAT(inst->LHS()->Usages(),
                testing::UnorderedElementsAre(Usage{inst, 0u}, Usage{inst, 1u}));
}

TEST_F(IR_BinaryTest, Binary_Usage_SetOperand) {
    auto* rhs_a = b.Constant(2_i);
    auto* rhs_b = b.Constant(3_i);
    auto* inst = b.And(mod.Types().i32(), 4_i, rhs_a);

    EXPECT_EQ(inst->Kind(), Binary::Kind::kAnd);

    EXPECT_THAT(rhs_a->Usages(), testing::UnorderedElementsAre(Usage{inst, 1u}));
    EXPECT_THAT(rhs_b->Usages(), testing::UnorderedElementsAre());
    inst->SetOperand(1, rhs_b);
    EXPECT_THAT(rhs_a->Usages(), testing::UnorderedElementsAre());
    EXPECT_THAT(rhs_b->Usages(), testing::UnorderedElementsAre(Usage{inst, 1u}));
}

}  // namespace
}  // namespace tint::ir
