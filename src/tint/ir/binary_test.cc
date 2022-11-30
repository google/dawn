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

#include <sstream>

#include "src/tint/ir/instruction.h"
#include "src/tint/ir/test_helper.h"

namespace tint::ir {
namespace {

using IR_InstructionTest = TestHelper;

TEST_F(IR_InstructionTest, CreateAnd) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr = b.builder.And(b.builder.Constant(i32(4)), b.builder.Constant(i32(2)));

    EXPECT_EQ(instr->GetKind(), Binary::Kind::kAnd);

    ASSERT_TRUE(instr->Result()->Is<Temp>());
    EXPECT_EQ(Temp::Id(42), instr->Result()->As<Temp>()->AsId());

    ASSERT_TRUE(instr->LHS()->Is<Constant>());
    auto lhs = instr->LHS()->As<Constant>();
    ASSERT_TRUE(lhs->IsI32());
    EXPECT_EQ(i32(4), lhs->AsI32());

    ASSERT_TRUE(instr->RHS()->Is<Constant>());
    auto rhs = instr->RHS()->As<Constant>();
    ASSERT_TRUE(rhs->IsI32());
    EXPECT_EQ(i32(2), rhs->AsI32());

    std::stringstream str;
    instr->ToString(str);
    EXPECT_EQ(str.str(), "%42 = 4 & 2");
}

TEST_F(IR_InstructionTest, CreateOr) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr = b.builder.Or(b.builder.Constant(i32(4)), b.builder.Constant(i32(2)));

    EXPECT_EQ(instr->GetKind(), Binary::Kind::kOr);

    ASSERT_TRUE(instr->Result()->Is<Temp>());
    EXPECT_EQ(Temp::Id(42), instr->Result()->As<Temp>()->AsId());

    ASSERT_TRUE(instr->LHS()->Is<Constant>());
    auto lhs = instr->LHS()->As<Constant>();
    ASSERT_TRUE(lhs->IsI32());
    EXPECT_EQ(i32(4), lhs->AsI32());

    ASSERT_TRUE(instr->RHS()->Is<Constant>());
    auto rhs = instr->RHS()->As<Constant>();
    ASSERT_TRUE(rhs->IsI32());
    EXPECT_EQ(i32(2), rhs->AsI32());

    std::stringstream str;
    instr->ToString(str);
    EXPECT_EQ(str.str(), "%42 = 4 | 2");
}

TEST_F(IR_InstructionTest, CreateXor) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr = b.builder.Xor(b.builder.Constant(i32(4)), b.builder.Constant(i32(2)));

    EXPECT_EQ(instr->GetKind(), Binary::Kind::kXor);

    ASSERT_TRUE(instr->Result()->Is<Temp>());
    EXPECT_EQ(Temp::Id(42), instr->Result()->As<Temp>()->AsId());

    ASSERT_TRUE(instr->LHS()->Is<Constant>());
    auto lhs = instr->LHS()->As<Constant>();
    ASSERT_TRUE(lhs->IsI32());
    EXPECT_EQ(i32(4), lhs->AsI32());

    ASSERT_TRUE(instr->RHS()->Is<Constant>());
    auto rhs = instr->RHS()->As<Constant>();
    ASSERT_TRUE(rhs->IsI32());
    EXPECT_EQ(i32(2), rhs->AsI32());

    std::stringstream str;
    instr->ToString(str);
    EXPECT_EQ(str.str(), "%42 = 4 ^ 2");
}

TEST_F(IR_InstructionTest, CreateLogicalAnd) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr =
        b.builder.LogicalAnd(b.builder.Constant(i32(4)), b.builder.Constant(i32(2)));

    EXPECT_EQ(instr->GetKind(), Binary::Kind::kLogicalAnd);

    ASSERT_TRUE(instr->Result()->Is<Temp>());
    EXPECT_EQ(Temp::Id(42), instr->Result()->As<Temp>()->AsId());

    ASSERT_TRUE(instr->LHS()->Is<Constant>());
    auto lhs = instr->LHS()->As<Constant>();
    ASSERT_TRUE(lhs->IsI32());
    EXPECT_EQ(i32(4), lhs->AsI32());

    ASSERT_TRUE(instr->RHS()->Is<Constant>());
    auto rhs = instr->RHS()->As<Constant>();
    ASSERT_TRUE(rhs->IsI32());
    EXPECT_EQ(i32(2), rhs->AsI32());

    std::stringstream str;
    instr->ToString(str);
    EXPECT_EQ(str.str(), "%42 = 4 && 2");
}

TEST_F(IR_InstructionTest, CreateLogicalOr) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr = b.builder.LogicalOr(b.builder.Constant(i32(4)), b.builder.Constant(i32(2)));

    EXPECT_EQ(instr->GetKind(), Binary::Kind::kLogicalOr);

    ASSERT_TRUE(instr->Result()->Is<Temp>());
    EXPECT_EQ(Temp::Id(42), instr->Result()->As<Temp>()->AsId());

    ASSERT_TRUE(instr->LHS()->Is<Constant>());
    auto lhs = instr->LHS()->As<Constant>();
    ASSERT_TRUE(lhs->IsI32());
    EXPECT_EQ(i32(4), lhs->AsI32());

    ASSERT_TRUE(instr->RHS()->Is<Constant>());
    auto rhs = instr->RHS()->As<Constant>();
    ASSERT_TRUE(rhs->IsI32());
    EXPECT_EQ(i32(2), rhs->AsI32());

    std::stringstream str;
    instr->ToString(str);
    EXPECT_EQ(str.str(), "%42 = 4 || 2");
}

TEST_F(IR_InstructionTest, CreateEqual) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr = b.builder.Equal(b.builder.Constant(i32(4)), b.builder.Constant(i32(2)));

    EXPECT_EQ(instr->GetKind(), Binary::Kind::kEqual);

    ASSERT_TRUE(instr->Result()->Is<Temp>());
    EXPECT_EQ(Temp::Id(42), instr->Result()->As<Temp>()->AsId());

    ASSERT_TRUE(instr->LHS()->Is<Constant>());
    auto lhs = instr->LHS()->As<Constant>();
    ASSERT_TRUE(lhs->IsI32());
    EXPECT_EQ(i32(4), lhs->AsI32());

    ASSERT_TRUE(instr->RHS()->Is<Constant>());
    auto rhs = instr->RHS()->As<Constant>();
    ASSERT_TRUE(rhs->IsI32());
    EXPECT_EQ(i32(2), rhs->AsI32());

    std::stringstream str;
    instr->ToString(str);
    EXPECT_EQ(str.str(), "%42 = 4 == 2");
}

TEST_F(IR_InstructionTest, CreateNotEqual) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr = b.builder.NotEqual(b.builder.Constant(i32(4)), b.builder.Constant(i32(2)));

    EXPECT_EQ(instr->GetKind(), Binary::Kind::kNotEqual);

    ASSERT_TRUE(instr->Result()->Is<Temp>());
    EXPECT_EQ(Temp::Id(42), instr->Result()->As<Temp>()->AsId());

    ASSERT_TRUE(instr->LHS()->Is<Constant>());
    auto lhs = instr->LHS()->As<Constant>();
    ASSERT_TRUE(lhs->IsI32());
    EXPECT_EQ(i32(4), lhs->AsI32());

    ASSERT_TRUE(instr->RHS()->Is<Constant>());
    auto rhs = instr->RHS()->As<Constant>();
    ASSERT_TRUE(rhs->IsI32());
    EXPECT_EQ(i32(2), rhs->AsI32());

    std::stringstream str;
    instr->ToString(str);
    EXPECT_EQ(str.str(), "%42 = 4 != 2");
}

TEST_F(IR_InstructionTest, CreateLessThan) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr = b.builder.LessThan(b.builder.Constant(i32(4)), b.builder.Constant(i32(2)));

    EXPECT_EQ(instr->GetKind(), Binary::Kind::kLessThan);

    ASSERT_TRUE(instr->Result()->Is<Temp>());
    EXPECT_EQ(Temp::Id(42), instr->Result()->As<Temp>()->AsId());

    ASSERT_TRUE(instr->LHS()->Is<Constant>());
    auto lhs = instr->LHS()->As<Constant>();
    ASSERT_TRUE(lhs->IsI32());
    EXPECT_EQ(i32(4), lhs->AsI32());

    ASSERT_TRUE(instr->RHS()->Is<Constant>());
    auto rhs = instr->RHS()->As<Constant>();
    ASSERT_TRUE(rhs->IsI32());
    EXPECT_EQ(i32(2), rhs->AsI32());

    std::stringstream str;
    instr->ToString(str);
    EXPECT_EQ(str.str(), "%42 = 4 < 2");
}

TEST_F(IR_InstructionTest, CreateGreaterThan) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr =
        b.builder.GreaterThan(b.builder.Constant(i32(4)), b.builder.Constant(i32(2)));

    EXPECT_EQ(instr->GetKind(), Binary::Kind::kGreaterThan);

    ASSERT_TRUE(instr->Result()->Is<Temp>());
    EXPECT_EQ(Temp::Id(42), instr->Result()->As<Temp>()->AsId());

    ASSERT_TRUE(instr->LHS()->Is<Constant>());
    auto lhs = instr->LHS()->As<Constant>();
    ASSERT_TRUE(lhs->IsI32());
    EXPECT_EQ(i32(4), lhs->AsI32());

    ASSERT_TRUE(instr->RHS()->Is<Constant>());
    auto rhs = instr->RHS()->As<Constant>();
    ASSERT_TRUE(rhs->IsI32());
    EXPECT_EQ(i32(2), rhs->AsI32());

    std::stringstream str;
    instr->ToString(str);
    EXPECT_EQ(str.str(), "%42 = 4 > 2");
}

TEST_F(IR_InstructionTest, CreateLessThanEqual) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr =
        b.builder.LessThanEqual(b.builder.Constant(i32(4)), b.builder.Constant(i32(2)));

    EXPECT_EQ(instr->GetKind(), Binary::Kind::kLessThanEqual);

    ASSERT_TRUE(instr->Result()->Is<Temp>());
    EXPECT_EQ(Temp::Id(42), instr->Result()->As<Temp>()->AsId());

    ASSERT_TRUE(instr->LHS()->Is<Constant>());
    auto lhs = instr->LHS()->As<Constant>();
    ASSERT_TRUE(lhs->IsI32());
    EXPECT_EQ(i32(4), lhs->AsI32());

    ASSERT_TRUE(instr->RHS()->Is<Constant>());
    auto rhs = instr->RHS()->As<Constant>();
    ASSERT_TRUE(rhs->IsI32());
    EXPECT_EQ(i32(2), rhs->AsI32());

    std::stringstream str;
    instr->ToString(str);
    EXPECT_EQ(str.str(), "%42 = 4 <= 2");
}

TEST_F(IR_InstructionTest, CreateGreaterThanEqual) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr =
        b.builder.GreaterThanEqual(b.builder.Constant(i32(4)), b.builder.Constant(i32(2)));

    EXPECT_EQ(instr->GetKind(), Binary::Kind::kGreaterThanEqual);

    ASSERT_TRUE(instr->Result()->Is<Temp>());
    EXPECT_EQ(Temp::Id(42), instr->Result()->As<Temp>()->AsId());

    ASSERT_TRUE(instr->LHS()->Is<Constant>());
    auto lhs = instr->LHS()->As<Constant>();
    ASSERT_TRUE(lhs->IsI32());
    EXPECT_EQ(i32(4), lhs->AsI32());

    ASSERT_TRUE(instr->RHS()->Is<Constant>());
    auto rhs = instr->RHS()->As<Constant>();
    ASSERT_TRUE(rhs->IsI32());
    EXPECT_EQ(i32(2), rhs->AsI32());

    std::stringstream str;
    instr->ToString(str);
    EXPECT_EQ(str.str(), "%42 = 4 >= 2");
}

TEST_F(IR_InstructionTest, CreateShiftLeft) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr = b.builder.ShiftLeft(b.builder.Constant(i32(4)), b.builder.Constant(i32(2)));

    EXPECT_EQ(instr->GetKind(), Binary::Kind::kShiftLeft);

    ASSERT_TRUE(instr->Result()->Is<Temp>());
    EXPECT_EQ(Temp::Id(42), instr->Result()->As<Temp>()->AsId());

    ASSERT_TRUE(instr->LHS()->Is<Constant>());
    auto lhs = instr->LHS()->As<Constant>();
    ASSERT_TRUE(lhs->IsI32());
    EXPECT_EQ(i32(4), lhs->AsI32());

    ASSERT_TRUE(instr->RHS()->Is<Constant>());
    auto rhs = instr->RHS()->As<Constant>();
    ASSERT_TRUE(rhs->IsI32());
    EXPECT_EQ(i32(2), rhs->AsI32());

    std::stringstream str;
    instr->ToString(str);
    EXPECT_EQ(str.str(), "%42 = 4 << 2");
}

TEST_F(IR_InstructionTest, CreateShiftRight) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr =
        b.builder.ShiftRight(b.builder.Constant(i32(4)), b.builder.Constant(i32(2)));

    EXPECT_EQ(instr->GetKind(), Binary::Kind::kShiftRight);

    ASSERT_TRUE(instr->Result()->Is<Temp>());
    EXPECT_EQ(Temp::Id(42), instr->Result()->As<Temp>()->AsId());

    ASSERT_TRUE(instr->LHS()->Is<Constant>());
    auto lhs = instr->LHS()->As<Constant>();
    ASSERT_TRUE(lhs->IsI32());
    EXPECT_EQ(i32(4), lhs->AsI32());

    ASSERT_TRUE(instr->RHS()->Is<Constant>());
    auto rhs = instr->RHS()->As<Constant>();
    ASSERT_TRUE(rhs->IsI32());
    EXPECT_EQ(i32(2), rhs->AsI32());

    std::stringstream str;
    instr->ToString(str);
    EXPECT_EQ(str.str(), "%42 = 4 >> 2");
}

TEST_F(IR_InstructionTest, CreateAdd) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr = b.builder.Add(b.builder.Constant(i32(4)), b.builder.Constant(i32(2)));

    EXPECT_EQ(instr->GetKind(), Binary::Kind::kAdd);

    ASSERT_TRUE(instr->Result()->Is<Temp>());
    EXPECT_EQ(Temp::Id(42), instr->Result()->As<Temp>()->AsId());

    ASSERT_TRUE(instr->LHS()->Is<Constant>());
    auto lhs = instr->LHS()->As<Constant>();
    ASSERT_TRUE(lhs->IsI32());
    EXPECT_EQ(i32(4), lhs->AsI32());

    ASSERT_TRUE(instr->RHS()->Is<Constant>());
    auto rhs = instr->RHS()->As<Constant>();
    ASSERT_TRUE(rhs->IsI32());
    EXPECT_EQ(i32(2), rhs->AsI32());

    std::stringstream str;
    instr->ToString(str);
    EXPECT_EQ(str.str(), "%42 = 4 + 2");
}

TEST_F(IR_InstructionTest, CreateSubtract) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr = b.builder.Subtract(b.builder.Constant(i32(4)), b.builder.Constant(i32(2)));

    EXPECT_EQ(instr->GetKind(), Binary::Kind::kSubtract);

    ASSERT_TRUE(instr->Result()->Is<Temp>());
    EXPECT_EQ(Temp::Id(42), instr->Result()->As<Temp>()->AsId());

    ASSERT_TRUE(instr->LHS()->Is<Constant>());
    auto lhs = instr->LHS()->As<Constant>();
    ASSERT_TRUE(lhs->IsI32());
    EXPECT_EQ(i32(4), lhs->AsI32());

    ASSERT_TRUE(instr->RHS()->Is<Constant>());
    auto rhs = instr->RHS()->As<Constant>();
    ASSERT_TRUE(rhs->IsI32());
    EXPECT_EQ(i32(2), rhs->AsI32());

    std::stringstream str;
    instr->ToString(str);
    EXPECT_EQ(str.str(), "%42 = 4 - 2");
}

TEST_F(IR_InstructionTest, CreateMultiply) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr = b.builder.Multiply(b.builder.Constant(i32(4)), b.builder.Constant(i32(2)));

    EXPECT_EQ(instr->GetKind(), Binary::Kind::kMultiply);

    ASSERT_TRUE(instr->Result()->Is<Temp>());
    EXPECT_EQ(Temp::Id(42), instr->Result()->As<Temp>()->AsId());

    ASSERT_TRUE(instr->LHS()->Is<Constant>());
    auto lhs = instr->LHS()->As<Constant>();
    ASSERT_TRUE(lhs->IsI32());
    EXPECT_EQ(i32(4), lhs->AsI32());

    ASSERT_TRUE(instr->RHS()->Is<Constant>());
    auto rhs = instr->RHS()->As<Constant>();
    ASSERT_TRUE(rhs->IsI32());
    EXPECT_EQ(i32(2), rhs->AsI32());

    std::stringstream str;
    instr->ToString(str);
    EXPECT_EQ(str.str(), "%42 = 4 * 2");
}

TEST_F(IR_InstructionTest, CreateDivide) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr = b.builder.Divide(b.builder.Constant(i32(4)), b.builder.Constant(i32(2)));

    EXPECT_EQ(instr->GetKind(), Binary::Kind::kDivide);

    ASSERT_TRUE(instr->Result()->Is<Temp>());
    EXPECT_EQ(Temp::Id(42), instr->Result()->As<Temp>()->AsId());

    ASSERT_TRUE(instr->LHS()->Is<Constant>());
    auto lhs = instr->LHS()->As<Constant>();
    ASSERT_TRUE(lhs->IsI32());
    EXPECT_EQ(i32(4), lhs->AsI32());

    ASSERT_TRUE(instr->RHS()->Is<Constant>());
    auto rhs = instr->RHS()->As<Constant>();
    ASSERT_TRUE(rhs->IsI32());
    EXPECT_EQ(i32(2), rhs->AsI32());

    std::stringstream str;
    instr->ToString(str);
    EXPECT_EQ(str.str(), "%42 = 4 / 2");
}

TEST_F(IR_InstructionTest, CreateModulo) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr = b.builder.Modulo(b.builder.Constant(i32(4)), b.builder.Constant(i32(2)));

    EXPECT_EQ(instr->GetKind(), Binary::Kind::kModulo);

    ASSERT_TRUE(instr->Result()->Is<Temp>());
    EXPECT_EQ(Temp::Id(42), instr->Result()->As<Temp>()->AsId());

    ASSERT_TRUE(instr->LHS()->Is<Constant>());
    auto lhs = instr->LHS()->As<Constant>();
    ASSERT_TRUE(lhs->IsI32());
    EXPECT_EQ(i32(4), lhs->AsI32());

    ASSERT_TRUE(instr->RHS()->Is<Constant>());
    auto rhs = instr->RHS()->As<Constant>();
    ASSERT_TRUE(rhs->IsI32());
    EXPECT_EQ(i32(2), rhs->AsI32());

    std::stringstream str;
    instr->ToString(str);
    EXPECT_EQ(str.str(), "%42 = 4 % 2");
}

}  // namespace
}  // namespace tint::ir
