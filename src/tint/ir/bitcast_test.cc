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
#include "src/tint/ir/constant.h"
#include "src/tint/ir/instruction.h"
#include "src/tint/ir/test_helper.h"

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT

using IR_InstructionTest = TestHelper;

TEST_F(IR_InstructionTest, Bitcast) {
    Module mod;
    Builder b{mod};
    const auto* inst = b.Bitcast(mod.Types().i32(), b.Constant(4_i));

    ASSERT_TRUE(inst->Is<ir::Bitcast>());
    ASSERT_NE(inst->Type(), nullptr);

    const auto args = inst->Args();
    ASSERT_EQ(args.Length(), 1u);
    ASSERT_TRUE(args[0]->Is<Constant>());
    auto val = args[0]->As<Constant>()->Value();
    ASSERT_TRUE(val->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, val->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_InstructionTest, Bitcast_Usage) {
    Module mod;
    Builder b{mod};
    const auto* inst = b.Bitcast(mod.Types().i32(), b.Constant(4_i));

    const auto args = inst->Args();
    ASSERT_EQ(args.Length(), 1u);
    ASSERT_NE(args[0], nullptr);
    ASSERT_EQ(args[0]->Usage().Length(), 1u);
    EXPECT_EQ(args[0]->Usage()[0], inst);
}

}  // namespace
}  // namespace tint::ir
