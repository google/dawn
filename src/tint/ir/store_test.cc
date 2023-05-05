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

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT

using IR_InstructionTest = TestHelper;

TEST_F(IR_InstructionTest, CreateStore) {
    auto& b = CreateEmptyBuilder();

    // TODO(dsinclair): This is wrong, but we don't have anything correct to store too at the
    // moment.
    auto* to = b.builder.Discard();
    const auto* inst = b.builder.Store(to, b.builder.Constant(4_i));

    ASSERT_TRUE(inst->Is<Store>());
    ASSERT_EQ(inst->to, to);

    ASSERT_TRUE(inst->from->Is<Constant>());
    auto lhs = inst->from->As<Constant>()->value;
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_InstructionTest, Store_Usage) {
    auto& b = CreateEmptyBuilder();

    auto* to = b.builder.Discard();
    const auto* inst = b.builder.Store(to, b.builder.Constant(4_i));

    ASSERT_NE(inst->to, nullptr);
    ASSERT_EQ(inst->to->Usage().Length(), 1u);
    EXPECT_EQ(inst->to->Usage()[0], inst);

    ASSERT_NE(inst->from, nullptr);
    ASSERT_EQ(inst->from->Usage().Length(), 1u);
    EXPECT_EQ(inst->from->Usage()[0], inst);
}

}  // namespace
}  // namespace tint::ir
