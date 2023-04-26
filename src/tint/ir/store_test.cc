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

TEST_F(IR_InstructionTest, CreateStore) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_runtime_id = Runtime::Id(42);

    auto* rt = b.builder.Runtime(b.builder.ir.types.Get<type::I32>());
    const auto* inst = b.builder.Store(rt, b.builder.Constant(4_i));

    ASSERT_TRUE(inst->Result()->Is<Runtime>());
    ASSERT_NE(inst->Result()->Type(), nullptr);
    EXPECT_EQ(Runtime::Id(42), inst->Result()->As<Runtime>()->AsId());

    ASSERT_TRUE(inst->from()->Is<Constant>());
    auto lhs = inst->from()->As<Constant>()->value;
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());

    utils::StringStream str;
    inst->ToString(str);
    EXPECT_EQ(str.str(), "%42 (i32) = 4");
}

TEST_F(IR_InstructionTest, Store_Usage) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_runtime_id = Runtime::Id(42);
    auto* rt = b.builder.Runtime(b.builder.ir.types.Get<type::I32>());
    const auto* inst = b.builder.Store(rt, b.builder.Constant(4_i));

    ASSERT_NE(inst->Result(), nullptr);
    ASSERT_EQ(inst->Result()->Usage().Length(), 1u);
    EXPECT_EQ(inst->Result()->Usage()[0], inst);

    ASSERT_NE(inst->from(), nullptr);
    ASSERT_EQ(inst->from()->Usage().Length(), 1u);
    EXPECT_EQ(inst->from()->Usage()[0], inst);
}

}  // namespace
}  // namespace tint::ir
