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

    // TODO(dsinclair): This would be better as an identifier, but works for now.
    const auto* inst =
        b.builder.AddressOf(b.builder.ir.types.Get<type::Pointer>(
                                b.builder.ir.types.Get<type::I32>(),
                                builtin::AddressSpace::kPrivate, builtin::Access::kReadWrite),
                            b.builder.Constant(4_i));

    ASSERT_TRUE(inst->Is<Unary>());
    EXPECT_EQ(inst->GetKind(), Unary::Kind::kAddressOf);

    ASSERT_NE(inst->Type(), nullptr);

    ASSERT_TRUE(inst->Val()->Is<Constant>());
    auto lhs = inst->Val()->As<Constant>()->value;
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    utils::StringStream str;
    inst->ToInstruction(str);
    EXPECT_EQ(str.str(), "%1(ptr<private, i32, read_write>) = addr_of 4i");
}

TEST_F(IR_InstructionTest, CreateComplement) {
    auto& b = CreateEmptyBuilder();
    const auto* inst =
        b.builder.Complement(b.builder.ir.types.Get<type::I32>(), b.builder.Constant(4_i));

    ASSERT_TRUE(inst->Is<Unary>());
    EXPECT_EQ(inst->GetKind(), Unary::Kind::kComplement);

    ASSERT_TRUE(inst->Val()->Is<Constant>());
    auto lhs = inst->Val()->As<Constant>()->value;
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    utils::StringStream str;
    inst->ToInstruction(str);
    EXPECT_EQ(str.str(), "%1(i32) = bit_complement 4i");
}

TEST_F(IR_InstructionTest, CreateIndirection) {
    auto& b = CreateEmptyBuilder();

    // TODO(dsinclair): This would be better as an identifier, but works for now.
    const auto* inst =
        b.builder.Indirection(b.builder.ir.types.Get<type::I32>(), b.builder.Constant(4_i));

    ASSERT_TRUE(inst->Is<Unary>());
    EXPECT_EQ(inst->GetKind(), Unary::Kind::kIndirection);

    ASSERT_TRUE(inst->Val()->Is<Constant>());
    auto lhs = inst->Val()->As<Constant>()->value;
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    utils::StringStream str;
    inst->ToInstruction(str);
    EXPECT_EQ(str.str(), "%1(i32) = indirection 4i");
}

TEST_F(IR_InstructionTest, CreateNegation) {
    auto& b = CreateEmptyBuilder();
    const auto* inst =
        b.builder.Negation(b.builder.ir.types.Get<type::I32>(), b.builder.Constant(4_i));

    ASSERT_TRUE(inst->Is<Unary>());
    EXPECT_EQ(inst->GetKind(), Unary::Kind::kNegation);

    ASSERT_TRUE(inst->Val()->Is<Constant>());
    auto lhs = inst->Val()->As<Constant>()->value;
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    utils::StringStream str;
    inst->ToInstruction(str);
    EXPECT_EQ(str.str(), "%1(i32) = negation 4i");
}

TEST_F(IR_InstructionTest, CreateNot) {
    auto& b = CreateEmptyBuilder();
    const auto* inst =
        b.builder.Not(b.builder.ir.types.Get<type::Bool>(), b.builder.Constant(true));

    ASSERT_TRUE(inst->Is<Unary>());
    EXPECT_EQ(inst->GetKind(), Unary::Kind::kNot);

    ASSERT_TRUE(inst->Val()->Is<Constant>());
    auto lhs = inst->Val()->As<Constant>()->value;
    ASSERT_TRUE(lhs->Is<constant::Scalar<bool>>());
    EXPECT_TRUE(lhs->As<constant::Scalar<bool>>()->ValueAs<bool>());

    utils::StringStream str;
    inst->ToInstruction(str);
    EXPECT_EQ(str.str(), "%1(bool) = log_not true");
}

TEST_F(IR_InstructionTest, Unary_Usage) {
    auto& b = CreateEmptyBuilder();
    const auto* inst =
        b.builder.Negation(b.builder.ir.types.Get<type::I32>(), b.builder.Constant(4_i));

    EXPECT_EQ(inst->GetKind(), Unary::Kind::kNegation);

    ASSERT_NE(inst->Val(), nullptr);
    ASSERT_EQ(inst->Val()->Usage().Length(), 1u);
    EXPECT_EQ(inst->Val()->Usage()[0], inst);
}

}  // namespace
}  // namespace tint::ir
