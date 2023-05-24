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

#include "src/tint/ir/builder.h"
#include "src/tint/ir/instruction.h"
#include "src/tint/ir/test_helper.h"

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT

using IR_InstructionTest = TestHelper;

TEST_F(IR_InstructionTest, CreateComplement) {
    Module mod;
    Builder b{mod};
    auto* inst = b.Complement(mod.Types().i32(), b.Constant(4_i));

    ASSERT_TRUE(inst->Is<Unary>());
    EXPECT_EQ(inst->Kind(), Unary::Kind::kComplement);

    ASSERT_TRUE(inst->Val()->Is<Constant>());
    auto lhs = inst->Val()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_InstructionTest, CreateNegation) {
    Module mod;
    Builder b{mod};
    auto* inst = b.Negation(mod.Types().i32(), b.Constant(4_i));

    ASSERT_TRUE(inst->Is<Unary>());
    EXPECT_EQ(inst->Kind(), Unary::Kind::kNegation);

    ASSERT_TRUE(inst->Val()->Is<Constant>());
    auto lhs = inst->Val()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_InstructionTest, Unary_Usage) {
    Module mod;
    Builder b{mod};
    auto* inst = b.Negation(mod.Types().i32(), b.Constant(4_i));

    EXPECT_EQ(inst->Kind(), Unary::Kind::kNegation);

    ASSERT_NE(inst->Val(), nullptr);
    ASSERT_EQ(inst->Val()->Usage().Length(), 1u);
    EXPECT_EQ(inst->Val()->Usage()[0], inst);
}

}  // namespace
}  // namespace tint::ir
