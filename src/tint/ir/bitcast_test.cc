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

#include <sstream>

#include "src/tint/ir/instruction.h"
#include "src/tint/ir/test_helper.h"

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT
                                        //
using IR_InstructionTest = TestHelper;

TEST_F(IR_InstructionTest, Bitcast) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr =
        b.builder.Bitcast(b.builder.ir.types.Get<type::I32>(), b.builder.Constant(4_i));

    ASSERT_TRUE(instr->Result()->Is<Temp>());
    EXPECT_EQ(Temp::Id(42), instr->Result()->As<Temp>()->AsId());
    ASSERT_NE(instr->Result()->Type(), nullptr);

    ASSERT_TRUE(instr->Val()->Is<Constant>());
    auto val = instr->Val()->As<Constant>()->value;
    ASSERT_TRUE(val->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, val->As<constant::Scalar<i32>>()->ValueAs<i32>());

    std::stringstream str;
    instr->ToString(str, b.builder.ir.symbols);
    EXPECT_EQ(str.str(), "%42 (i32) = bitcast(4)");
}

TEST_F(IR_InstructionTest, Bitcast_Usage) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_temp_id = Temp::Id(42);
    const auto* instr =
        b.builder.Bitcast(b.builder.ir.types.Get<type::I32>(), b.builder.Constant(4_i));

    ASSERT_NE(instr->Result(), nullptr);
    ASSERT_EQ(instr->Result()->Usage().Length(), 1u);
    EXPECT_EQ(instr->Result()->Usage()[0], instr);

    ASSERT_NE(instr->Val(), nullptr);
    ASSERT_EQ(instr->Val()->Usage().Length(), 1u);
    EXPECT_EQ(instr->Val()->Usage()[0], instr);
}

}  // namespace
}  // namespace tint::ir
