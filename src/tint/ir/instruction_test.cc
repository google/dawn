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

using IR_BinaryTest = TestHelper;

TEST_F(IR_BinaryTest, CreateAnd) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr = b.builder.And(b.builder.Constant(i32(4)), b.builder.Constant(i32(2)));

    EXPECT_EQ(instr->GetKind(), Instruction::Kind::kAnd);

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
    instr->ToString(str, program->Symbols());
    EXPECT_EQ(str.str(), "%42 = 4 & 2");
}

TEST_F(IR_BinaryTest, CreateOr) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr = b.builder.Or(b.builder.Constant(i32(4)), b.builder.Constant(i32(2)));

    EXPECT_EQ(instr->GetKind(), Instruction::Kind::kOr);

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
    instr->ToString(str, program->Symbols());
    EXPECT_EQ(str.str(), "%42 = 4 | 2");
}

TEST_F(IR_BinaryTest, CreateXor) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr = b.builder.Xor(b.builder.Constant(i32(4)), b.builder.Constant(i32(2)));

    EXPECT_EQ(instr->GetKind(), Instruction::Kind::kXor);

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
    instr->ToString(str, program->Symbols());
    EXPECT_EQ(str.str(), "%42 = 4 ^ 2");
}

TEST_F(IR_BinaryTest, CreateLogicalAnd) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr =
        b.builder.LogicalAnd(b.builder.Constant(i32(4)), b.builder.Constant(i32(2)));

    EXPECT_EQ(instr->GetKind(), Instruction::Kind::kLogicalAnd);

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
    instr->ToString(str, program->Symbols());
    EXPECT_EQ(str.str(), "%42 = 4 && 2");
}

TEST_F(IR_BinaryTest, CreateLogicalOr) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr = b.builder.LogicalOr(b.builder.Constant(i32(4)), b.builder.Constant(i32(2)));

    EXPECT_EQ(instr->GetKind(), Instruction::Kind::kLogicalOr);

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
    instr->ToString(str, program->Symbols());
    EXPECT_EQ(str.str(), "%42 = 4 || 2");
}

TEST_F(IR_BinaryTest, CreateEqual) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr = b.builder.Equal(b.builder.Constant(i32(4)), b.builder.Constant(i32(2)));

    EXPECT_EQ(instr->GetKind(), Instruction::Kind::kEqual);

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
    instr->ToString(str, program->Symbols());
    EXPECT_EQ(str.str(), "%42 = 4 == 2");
}

TEST_F(IR_BinaryTest, CreateNotEqual) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr = b.builder.NotEqual(b.builder.Constant(i32(4)), b.builder.Constant(i32(2)));

    EXPECT_EQ(instr->GetKind(), Instruction::Kind::kNotEqual);

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
    instr->ToString(str, program->Symbols());
    EXPECT_EQ(str.str(), "%42 = 4 != 2");
}

TEST_F(IR_BinaryTest, CreateLessThan) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr = b.builder.LessThan(b.builder.Constant(i32(4)), b.builder.Constant(i32(2)));

    EXPECT_EQ(instr->GetKind(), Instruction::Kind::kLessThan);

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
    instr->ToString(str, program->Symbols());
    EXPECT_EQ(str.str(), "%42 = 4 < 2");
}

TEST_F(IR_BinaryTest, CreateGreaterThan) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr =
        b.builder.GreaterThan(b.builder.Constant(i32(4)), b.builder.Constant(i32(2)));

    EXPECT_EQ(instr->GetKind(), Instruction::Kind::kGreaterThan);

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
    instr->ToString(str, program->Symbols());
    EXPECT_EQ(str.str(), "%42 = 4 > 2");
}

TEST_F(IR_BinaryTest, CreateLessThanEqual) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr =
        b.builder.LessThanEqual(b.builder.Constant(i32(4)), b.builder.Constant(i32(2)));

    EXPECT_EQ(instr->GetKind(), Instruction::Kind::kLessThanEqual);

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
    instr->ToString(str, program->Symbols());
    EXPECT_EQ(str.str(), "%42 = 4 <= 2");
}

TEST_F(IR_BinaryTest, CreateGreaterThanEqual) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr =
        b.builder.GreaterThanEqual(b.builder.Constant(i32(4)), b.builder.Constant(i32(2)));

    EXPECT_EQ(instr->GetKind(), Instruction::Kind::kGreaterThanEqual);

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
    instr->ToString(str, program->Symbols());
    EXPECT_EQ(str.str(), "%42 = 4 >= 2");
}

TEST_F(IR_BinaryTest, CreateShiftLeft) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr = b.builder.ShiftLeft(b.builder.Constant(i32(4)), b.builder.Constant(i32(2)));

    EXPECT_EQ(instr->GetKind(), Instruction::Kind::kShiftLeft);

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
    instr->ToString(str, program->Symbols());
    EXPECT_EQ(str.str(), "%42 = 4 << 2");
}

TEST_F(IR_BinaryTest, CreateShiftRight) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr =
        b.builder.ShiftRight(b.builder.Constant(i32(4)), b.builder.Constant(i32(2)));

    EXPECT_EQ(instr->GetKind(), Instruction::Kind::kShiftRight);

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
    instr->ToString(str, program->Symbols());
    EXPECT_EQ(str.str(), "%42 = 4 >> 2");
}

TEST_F(IR_BinaryTest, CreateAdd) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr = b.builder.Add(b.builder.Constant(i32(4)), b.builder.Constant(i32(2)));

    EXPECT_EQ(instr->GetKind(), Instruction::Kind::kAdd);

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
    instr->ToString(str, program->Symbols());
    EXPECT_EQ(str.str(), "%42 = 4 + 2");
}

TEST_F(IR_BinaryTest, CreateSubtract) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr = b.builder.Subtract(b.builder.Constant(i32(4)), b.builder.Constant(i32(2)));

    EXPECT_EQ(instr->GetKind(), Instruction::Kind::kSubtract);

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
    instr->ToString(str, program->Symbols());
    EXPECT_EQ(str.str(), "%42 = 4 - 2");
}

TEST_F(IR_BinaryTest, CreateMultiply) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr = b.builder.Multiply(b.builder.Constant(i32(4)), b.builder.Constant(i32(2)));

    EXPECT_EQ(instr->GetKind(), Instruction::Kind::kMultiply);

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
    instr->ToString(str, program->Symbols());
    EXPECT_EQ(str.str(), "%42 = 4 * 2");
}

TEST_F(IR_BinaryTest, CreateDivide) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr = b.builder.Divide(b.builder.Constant(i32(4)), b.builder.Constant(i32(2)));

    EXPECT_EQ(instr->GetKind(), Instruction::Kind::kDivide);

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
    instr->ToString(str, program->Symbols());
    EXPECT_EQ(str.str(), "%42 = 4 / 2");
}

TEST_F(IR_BinaryTest, CreateModulo) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr = b.builder.Modulo(b.builder.Constant(i32(4)), b.builder.Constant(i32(2)));

    EXPECT_EQ(instr->GetKind(), Instruction::Kind::kModulo);

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
    instr->ToString(str, program->Symbols());
    EXPECT_EQ(str.str(), "%42 = 4 % 2");
}

}  // namespace
}  // namespace tint::ir
