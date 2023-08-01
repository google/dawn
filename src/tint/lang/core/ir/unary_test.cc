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

#include "gmock/gmock.h"
#include "gtest/gtest-spi.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT

using IR_UnaryTest = IRTestHelper;

TEST_F(IR_UnaryTest, CreateComplement) {
    auto* inst = b.Complement(mod.Types().i32(), 4_i);

    ASSERT_TRUE(inst->Is<Unary>());
    EXPECT_EQ(inst->Kind(), Unary::Kind::kComplement);

    ASSERT_TRUE(inst->Val()->Is<Constant>());
    auto lhs = inst->Val()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_UnaryTest, CreateNegation) {
    auto* inst = b.Negation(mod.Types().i32(), 4_i);

    ASSERT_TRUE(inst->Is<Unary>());
    EXPECT_EQ(inst->Kind(), Unary::Kind::kNegation);

    ASSERT_TRUE(inst->Val()->Is<Constant>());
    auto lhs = inst->Val()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_UnaryTest, Usage) {
    auto* inst = b.Negation(mod.Types().i32(), 4_i);

    EXPECT_EQ(inst->Kind(), Unary::Kind::kNegation);

    ASSERT_NE(inst->Val(), nullptr);
    EXPECT_THAT(inst->Val()->Usages(), testing::UnorderedElementsAre(Usage{inst, 0u}));
}

TEST_F(IR_UnaryTest, Result) {
    auto* inst = b.Negation(mod.Types().i32(), 4_i);
    EXPECT_TRUE(inst->HasResults());
    EXPECT_FALSE(inst->HasMultiResults());
    EXPECT_TRUE(inst->Result()->Is<InstructionResult>());
    EXPECT_EQ(inst->Result()->Source(), inst);
}

TEST_F(IR_UnaryTest, Fail_NullType) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            b.Negation(nullptr, 1_i);
        },
        "");
}

}  // namespace
}  // namespace tint::ir
