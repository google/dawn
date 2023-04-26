// Copyright 2023 The Tint Authors.
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

#include "src/tint/ir/instruction.h"
#include "src/tint/ir/test_helper.h"
#include "src/tint/utils/string_stream.h"

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT

using IR_InstructionTest = TestHelper;

TEST_F(IR_InstructionTest, CreateAddressOf) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_runtime_id = Runtime::Id(42);
    // TODO(dsinclair): This would be better as an identifier, but works for now.
    const auto* instr =
        b.builder.AddressOf(b.builder.ir.types.Get<type::Pointer>(
                                b.builder.ir.types.Get<type::I32>(),
                                builtin::AddressSpace::kPrivate, builtin::Access::kReadWrite),
                            b.builder.Constant(4_i));

    EXPECT_EQ(instr->GetKind(), Unary::Kind::kAddressOf);

    ASSERT_TRUE(instr->Result()->Is<Runtime>());
    ASSERT_NE(instr->Result()->Type(), nullptr);
    EXPECT_EQ(Runtime::Id(42), instr->Result()->As<Runtime>()->AsId());

    ASSERT_TRUE(instr->Val()->Is<Constant>());
    auto lhs = instr->Val()->As<Constant>()->value;
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    utils::StringStream str;
    instr->ToString(str);
    EXPECT_EQ(str.str(), "%42 (ptr<private, i32, read_write>) = &4");
}

TEST_F(IR_InstructionTest, CreateComplement) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_runtime_id = Runtime::Id(42);
    const auto* instr =
        b.builder.Complement(b.builder.ir.types.Get<type::I32>(), b.builder.Constant(4_i));

    EXPECT_EQ(instr->GetKind(), Unary::Kind::kComplement);

    ASSERT_TRUE(instr->Result()->Is<Runtime>());
    EXPECT_EQ(Runtime::Id(42), instr->Result()->As<Runtime>()->AsId());

    ASSERT_TRUE(instr->Val()->Is<Constant>());
    auto lhs = instr->Val()->As<Constant>()->value;
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    utils::StringStream str;
    instr->ToString(str);
    EXPECT_EQ(str.str(), "%42 (i32) = ~4");
}

TEST_F(IR_InstructionTest, CreateIndirection) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_runtime_id = Runtime::Id(42);
    // TODO(dsinclair): This would be better as an identifier, but works for now.
    const auto* instr =
        b.builder.Indirection(b.builder.ir.types.Get<type::I32>(), b.builder.Constant(4_i));

    EXPECT_EQ(instr->GetKind(), Unary::Kind::kIndirection);

    ASSERT_TRUE(instr->Result()->Is<Runtime>());
    EXPECT_EQ(Runtime::Id(42), instr->Result()->As<Runtime>()->AsId());

    ASSERT_TRUE(instr->Val()->Is<Constant>());
    auto lhs = instr->Val()->As<Constant>()->value;
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    utils::StringStream str;
    instr->ToString(str);
    EXPECT_EQ(str.str(), "%42 (i32) = *4");
}

TEST_F(IR_InstructionTest, CreateNegation) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_runtime_id = Runtime::Id(42);
    const auto* instr =
        b.builder.Negation(b.builder.ir.types.Get<type::I32>(), b.builder.Constant(4_i));

    EXPECT_EQ(instr->GetKind(), Unary::Kind::kNegation);

    ASSERT_TRUE(instr->Result()->Is<Runtime>());
    EXPECT_EQ(Runtime::Id(42), instr->Result()->As<Runtime>()->AsId());

    ASSERT_TRUE(instr->Val()->Is<Constant>());
    auto lhs = instr->Val()->As<Constant>()->value;
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    utils::StringStream str;
    instr->ToString(str);
    EXPECT_EQ(str.str(), "%42 (i32) = -4");
}

TEST_F(IR_InstructionTest, CreateNot) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_runtime_id = Runtime::Id(42);
    const auto* instr =
        b.builder.Not(b.builder.ir.types.Get<type::Bool>(), b.builder.Constant(true));

    EXPECT_EQ(instr->GetKind(), Unary::Kind::kNot);

    ASSERT_TRUE(instr->Result()->Is<Runtime>());
    EXPECT_EQ(Runtime::Id(42), instr->Result()->As<Runtime>()->AsId());

    ASSERT_TRUE(instr->Val()->Is<Constant>());
    auto lhs = instr->Val()->As<Constant>()->value;
    ASSERT_TRUE(lhs->Is<constant::Scalar<bool>>());
    EXPECT_TRUE(lhs->As<constant::Scalar<bool>>()->ValueAs<bool>());

    utils::StringStream str;
    instr->ToString(str);
    EXPECT_EQ(str.str(), "%42 (bool) = !true");
}

TEST_F(IR_InstructionTest, Unary_Usage) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_runtime_id = Runtime::Id(42);
    const auto* instr =
        b.builder.Negation(b.builder.ir.types.Get<type::I32>(), b.builder.Constant(4_i));

    EXPECT_EQ(instr->GetKind(), Unary::Kind::kNegation);

    ASSERT_NE(instr->Result(), nullptr);
    ASSERT_EQ(instr->Result()->Usage().Length(), 1u);
    EXPECT_EQ(instr->Result()->Usage()[0], instr);

    ASSERT_NE(instr->Val(), nullptr);
    ASSERT_EQ(instr->Val()->Usage().Length(), 1u);
    EXPECT_EQ(instr->Val()->Usage()[0], instr);
}

}  // namespace
}  // namespace tint::ir
