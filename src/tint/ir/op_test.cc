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

#include "src/tint/ir/op.h"
#include "src/tint/ir/test_helper.h"

namespace tint::ir {
namespace {

using IR_OpTest = TestHelper;

TEST_F(IR_OpTest, CreateAnd) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_register_id = Register::Id(42);
    auto o = b.builder.And(Register(i32(4)), Register(i32(2)));

    EXPECT_EQ(o.GetKind(), Op::Kind::kAnd);

    ASSERT_TRUE(o.Result().IsTemp());
    EXPECT_EQ(Register::Id(42), o.Result().AsId());

    ASSERT_TRUE(o.HasLHS());
    auto& lhs = o.LHS();
    ASSERT_TRUE(lhs.IsI32());
    EXPECT_EQ(i32(4), lhs.AsI32());

    ASSERT_TRUE(o.HasRHS());
    auto& rhs = o.RHS();
    ASSERT_TRUE(rhs.IsI32());
    EXPECT_EQ(i32(2), rhs.AsI32());

    std::stringstream str;
    str << o;
    EXPECT_EQ(str.str(), "%42 = 4 & 2");
}

TEST_F(IR_OpTest, CreateOr) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_register_id = Register::Id(42);
    auto o = b.builder.Or(Register(i32(4)), Register(i32(2)));

    EXPECT_EQ(o.GetKind(), Op::Kind::kOr);

    ASSERT_TRUE(o.Result().IsTemp());
    EXPECT_EQ(Register::Id(42), o.Result().AsId());

    ASSERT_TRUE(o.HasLHS());
    auto& lhs = o.LHS();
    ASSERT_TRUE(lhs.IsI32());
    EXPECT_EQ(i32(4), lhs.AsI32());

    ASSERT_TRUE(o.HasRHS());
    auto& rhs = o.RHS();
    ASSERT_TRUE(rhs.IsI32());
    EXPECT_EQ(i32(2), rhs.AsI32());

    std::stringstream str;
    str << o;
    EXPECT_EQ(str.str(), "%42 = 4 | 2");
}

TEST_F(IR_OpTest, CreateXor) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_register_id = Register::Id(42);
    auto o = b.builder.Xor(Register(i32(4)), Register(i32(2)));

    EXPECT_EQ(o.GetKind(), Op::Kind::kXor);

    ASSERT_TRUE(o.Result().IsTemp());
    EXPECT_EQ(Register::Id(42), o.Result().AsId());

    ASSERT_TRUE(o.HasLHS());
    auto& lhs = o.LHS();
    ASSERT_TRUE(lhs.IsI32());
    EXPECT_EQ(i32(4), lhs.AsI32());

    ASSERT_TRUE(o.HasRHS());
    auto& rhs = o.RHS();
    ASSERT_TRUE(rhs.IsI32());
    EXPECT_EQ(i32(2), rhs.AsI32());

    std::stringstream str;
    str << o;
    EXPECT_EQ(str.str(), "%42 = 4 ^ 2");
}

TEST_F(IR_OpTest, CreateLogicalAnd) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_register_id = Register::Id(42);
    auto o = b.builder.LogicalAnd(Register(i32(4)), Register(i32(2)));

    EXPECT_EQ(o.GetKind(), Op::Kind::kLogicalAnd);

    ASSERT_TRUE(o.Result().IsTemp());
    EXPECT_EQ(Register::Id(42), o.Result().AsId());

    ASSERT_TRUE(o.HasLHS());
    auto& lhs = o.LHS();
    ASSERT_TRUE(lhs.IsI32());
    EXPECT_EQ(i32(4), lhs.AsI32());

    ASSERT_TRUE(o.HasRHS());
    auto& rhs = o.RHS();
    ASSERT_TRUE(rhs.IsI32());
    EXPECT_EQ(i32(2), rhs.AsI32());

    std::stringstream str;
    str << o;
    EXPECT_EQ(str.str(), "%42 = 4 && 2");
}

TEST_F(IR_OpTest, CreateLogicalOr) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_register_id = Register::Id(42);
    auto o = b.builder.LogicalOr(Register(i32(4)), Register(i32(2)));

    EXPECT_EQ(o.GetKind(), Op::Kind::kLogicalOr);

    ASSERT_TRUE(o.Result().IsTemp());
    EXPECT_EQ(Register::Id(42), o.Result().AsId());

    ASSERT_TRUE(o.HasLHS());
    auto& lhs = o.LHS();
    ASSERT_TRUE(lhs.IsI32());
    EXPECT_EQ(i32(4), lhs.AsI32());

    ASSERT_TRUE(o.HasRHS());
    auto& rhs = o.RHS();
    ASSERT_TRUE(rhs.IsI32());
    EXPECT_EQ(i32(2), rhs.AsI32());

    std::stringstream str;
    str << o;
    EXPECT_EQ(str.str(), "%42 = 4 || 2");
}

TEST_F(IR_OpTest, CreateEqual) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_register_id = Register::Id(42);
    auto o = b.builder.Equal(Register(i32(4)), Register(i32(2)));

    EXPECT_EQ(o.GetKind(), Op::Kind::kEqual);

    ASSERT_TRUE(o.Result().IsTemp());
    EXPECT_EQ(Register::Id(42), o.Result().AsId());

    ASSERT_TRUE(o.HasLHS());
    auto& lhs = o.LHS();
    ASSERT_TRUE(lhs.IsI32());
    EXPECT_EQ(i32(4), lhs.AsI32());

    ASSERT_TRUE(o.HasRHS());
    auto& rhs = o.RHS();
    ASSERT_TRUE(rhs.IsI32());
    EXPECT_EQ(i32(2), rhs.AsI32());

    std::stringstream str;
    str << o;
    EXPECT_EQ(str.str(), "%42 = 4 == 2");
}

TEST_F(IR_OpTest, CreateNotEqual) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_register_id = Register::Id(42);
    auto o = b.builder.NotEqual(Register(i32(4)), Register(i32(2)));

    EXPECT_EQ(o.GetKind(), Op::Kind::kNotEqual);

    ASSERT_TRUE(o.Result().IsTemp());
    EXPECT_EQ(Register::Id(42), o.Result().AsId());

    ASSERT_TRUE(o.HasLHS());
    auto& lhs = o.LHS();
    ASSERT_TRUE(lhs.IsI32());
    EXPECT_EQ(i32(4), lhs.AsI32());

    ASSERT_TRUE(o.HasRHS());
    auto& rhs = o.RHS();
    ASSERT_TRUE(rhs.IsI32());
    EXPECT_EQ(i32(2), rhs.AsI32());

    std::stringstream str;
    str << o;
    EXPECT_EQ(str.str(), "%42 = 4 != 2");
}

TEST_F(IR_OpTest, CreateLessThan) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_register_id = Register::Id(42);
    auto o = b.builder.LessThan(Register(i32(4)), Register(i32(2)));

    EXPECT_EQ(o.GetKind(), Op::Kind::kLessThan);

    ASSERT_TRUE(o.Result().IsTemp());
    EXPECT_EQ(Register::Id(42), o.Result().AsId());

    ASSERT_TRUE(o.HasLHS());
    auto& lhs = o.LHS();
    ASSERT_TRUE(lhs.IsI32());
    EXPECT_EQ(i32(4), lhs.AsI32());

    ASSERT_TRUE(o.HasRHS());
    auto& rhs = o.RHS();
    ASSERT_TRUE(rhs.IsI32());
    EXPECT_EQ(i32(2), rhs.AsI32());

    std::stringstream str;
    str << o;
    EXPECT_EQ(str.str(), "%42 = 4 < 2");
}

TEST_F(IR_OpTest, CreateGreaterThan) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_register_id = Register::Id(42);
    auto o = b.builder.GreaterThan(Register(i32(4)), Register(i32(2)));

    EXPECT_EQ(o.GetKind(), Op::Kind::kGreaterThan);

    ASSERT_TRUE(o.Result().IsTemp());
    EXPECT_EQ(Register::Id(42), o.Result().AsId());

    ASSERT_TRUE(o.HasLHS());
    auto& lhs = o.LHS();
    ASSERT_TRUE(lhs.IsI32());
    EXPECT_EQ(i32(4), lhs.AsI32());

    ASSERT_TRUE(o.HasRHS());
    auto& rhs = o.RHS();
    ASSERT_TRUE(rhs.IsI32());
    EXPECT_EQ(i32(2), rhs.AsI32());

    std::stringstream str;
    str << o;
    EXPECT_EQ(str.str(), "%42 = 4 > 2");
}

TEST_F(IR_OpTest, CreateLessThanEqual) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_register_id = Register::Id(42);
    auto o = b.builder.LessThanEqual(Register(i32(4)), Register(i32(2)));

    EXPECT_EQ(o.GetKind(), Op::Kind::kLessThanEqual);

    ASSERT_TRUE(o.Result().IsTemp());
    EXPECT_EQ(Register::Id(42), o.Result().AsId());

    ASSERT_TRUE(o.HasLHS());
    auto& lhs = o.LHS();
    ASSERT_TRUE(lhs.IsI32());
    EXPECT_EQ(i32(4), lhs.AsI32());

    ASSERT_TRUE(o.HasRHS());
    auto& rhs = o.RHS();
    ASSERT_TRUE(rhs.IsI32());
    EXPECT_EQ(i32(2), rhs.AsI32());

    std::stringstream str;
    str << o;
    EXPECT_EQ(str.str(), "%42 = 4 <= 2");
}

TEST_F(IR_OpTest, CreateGreaterThanEqual) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_register_id = Register::Id(42);
    auto o = b.builder.GreaterThanEqual(Register(i32(4)), Register(i32(2)));

    EXPECT_EQ(o.GetKind(), Op::Kind::kGreaterThanEqual);

    ASSERT_TRUE(o.Result().IsTemp());
    EXPECT_EQ(Register::Id(42), o.Result().AsId());

    ASSERT_TRUE(o.HasLHS());
    auto& lhs = o.LHS();
    ASSERT_TRUE(lhs.IsI32());
    EXPECT_EQ(i32(4), lhs.AsI32());

    ASSERT_TRUE(o.HasRHS());
    auto& rhs = o.RHS();
    ASSERT_TRUE(rhs.IsI32());
    EXPECT_EQ(i32(2), rhs.AsI32());

    std::stringstream str;
    str << o;
    EXPECT_EQ(str.str(), "%42 = 4 >= 2");
}

TEST_F(IR_OpTest, CreateShiftLeft) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_register_id = Register::Id(42);
    auto o = b.builder.ShiftLeft(Register(i32(4)), Register(i32(2)));

    EXPECT_EQ(o.GetKind(), Op::Kind::kShiftLeft);

    ASSERT_TRUE(o.Result().IsTemp());
    EXPECT_EQ(Register::Id(42), o.Result().AsId());

    ASSERT_TRUE(o.HasLHS());
    auto& lhs = o.LHS();
    ASSERT_TRUE(lhs.IsI32());
    EXPECT_EQ(i32(4), lhs.AsI32());

    ASSERT_TRUE(o.HasRHS());
    auto& rhs = o.RHS();
    ASSERT_TRUE(rhs.IsI32());
    EXPECT_EQ(i32(2), rhs.AsI32());

    std::stringstream str;
    str << o;
    EXPECT_EQ(str.str(), "%42 = 4 << 2");
}

TEST_F(IR_OpTest, CreateShiftRight) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_register_id = Register::Id(42);
    auto o = b.builder.ShiftRight(Register(i32(4)), Register(i32(2)));

    EXPECT_EQ(o.GetKind(), Op::Kind::kShiftRight);

    ASSERT_TRUE(o.Result().IsTemp());
    EXPECT_EQ(Register::Id(42), o.Result().AsId());

    ASSERT_TRUE(o.HasLHS());
    auto& lhs = o.LHS();
    ASSERT_TRUE(lhs.IsI32());
    EXPECT_EQ(i32(4), lhs.AsI32());

    ASSERT_TRUE(o.HasRHS());
    auto& rhs = o.RHS();
    ASSERT_TRUE(rhs.IsI32());
    EXPECT_EQ(i32(2), rhs.AsI32());

    std::stringstream str;
    str << o;
    EXPECT_EQ(str.str(), "%42 = 4 >> 2");
}

TEST_F(IR_OpTest, CreateAdd) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_register_id = Register::Id(42);
    auto o = b.builder.Add(Register(i32(4)), Register(i32(2)));

    EXPECT_EQ(o.GetKind(), Op::Kind::kAdd);

    ASSERT_TRUE(o.Result().IsTemp());
    EXPECT_EQ(Register::Id(42), o.Result().AsId());

    ASSERT_TRUE(o.HasLHS());
    auto& lhs = o.LHS();
    ASSERT_TRUE(lhs.IsI32());
    EXPECT_EQ(i32(4), lhs.AsI32());

    ASSERT_TRUE(o.HasRHS());
    auto& rhs = o.RHS();
    ASSERT_TRUE(rhs.IsI32());
    EXPECT_EQ(i32(2), rhs.AsI32());

    std::stringstream str;
    str << o;
    EXPECT_EQ(str.str(), "%42 = 4 + 2");
}

TEST_F(IR_OpTest, CreateSubtract) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_register_id = Register::Id(42);
    auto o = b.builder.Subtract(Register(i32(4)), Register(i32(2)));

    EXPECT_EQ(o.GetKind(), Op::Kind::kSubtract);

    ASSERT_TRUE(o.Result().IsTemp());
    EXPECT_EQ(Register::Id(42), o.Result().AsId());

    ASSERT_TRUE(o.HasLHS());
    auto& lhs = o.LHS();
    ASSERT_TRUE(lhs.IsI32());
    EXPECT_EQ(i32(4), lhs.AsI32());

    ASSERT_TRUE(o.HasRHS());
    auto& rhs = o.RHS();
    ASSERT_TRUE(rhs.IsI32());
    EXPECT_EQ(i32(2), rhs.AsI32());

    std::stringstream str;
    str << o;
    EXPECT_EQ(str.str(), "%42 = 4 - 2");
}

TEST_F(IR_OpTest, CreateMultiply) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_register_id = Register::Id(42);
    auto o = b.builder.Multiply(Register(i32(4)), Register(i32(2)));

    EXPECT_EQ(o.GetKind(), Op::Kind::kMultiply);

    ASSERT_TRUE(o.Result().IsTemp());
    EXPECT_EQ(Register::Id(42), o.Result().AsId());

    ASSERT_TRUE(o.HasLHS());
    auto& lhs = o.LHS();
    ASSERT_TRUE(lhs.IsI32());
    EXPECT_EQ(i32(4), lhs.AsI32());

    ASSERT_TRUE(o.HasRHS());
    auto& rhs = o.RHS();
    ASSERT_TRUE(rhs.IsI32());
    EXPECT_EQ(i32(2), rhs.AsI32());

    std::stringstream str;
    str << o;
    EXPECT_EQ(str.str(), "%42 = 4 * 2");
}

TEST_F(IR_OpTest, CreateDivide) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_register_id = Register::Id(42);
    auto o = b.builder.Divide(Register(i32(4)), Register(i32(2)));

    EXPECT_EQ(o.GetKind(), Op::Kind::kDivide);

    ASSERT_TRUE(o.Result().IsTemp());
    EXPECT_EQ(Register::Id(42), o.Result().AsId());

    ASSERT_TRUE(o.HasLHS());
    auto& lhs = o.LHS();
    ASSERT_TRUE(lhs.IsI32());
    EXPECT_EQ(i32(4), lhs.AsI32());

    ASSERT_TRUE(o.HasRHS());
    auto& rhs = o.RHS();
    ASSERT_TRUE(rhs.IsI32());
    EXPECT_EQ(i32(2), rhs.AsI32());

    std::stringstream str;
    str << o;
    EXPECT_EQ(str.str(), "%42 = 4 / 2");
}

TEST_F(IR_OpTest, CreateModulo) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_register_id = Register::Id(42);
    auto o = b.builder.Modulo(Register(i32(4)), Register(i32(2)));

    EXPECT_EQ(o.GetKind(), Op::Kind::kModulo);

    ASSERT_TRUE(o.Result().IsTemp());
    EXPECT_EQ(Register::Id(42), o.Result().AsId());

    ASSERT_TRUE(o.HasLHS());
    auto& lhs = o.LHS();
    ASSERT_TRUE(lhs.IsI32());
    EXPECT_EQ(i32(4), lhs.AsI32());

    ASSERT_TRUE(o.HasRHS());
    auto& rhs = o.RHS();
    ASSERT_TRUE(rhs.IsI32());
    EXPECT_EQ(i32(2), rhs.AsI32());

    std::stringstream str;
    str << o;
    EXPECT_EQ(str.str(), "%42 = 4 % 2");
}

}  // namespace
}  // namespace tint::ir
