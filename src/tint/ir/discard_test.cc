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

using IR_InstructionTest = TestHelper;

TEST_F(IR_InstructionTest, Discard) {
    auto& b = CreateEmptyBuilder();

    b.builder.next_runtime_id = Runtime::Id(42);
    const auto* instr = b.builder.Discard();

    ASSERT_TRUE(instr->Result()->Is<Runtime>());
    EXPECT_EQ(Runtime::Id(42), instr->Result()->As<Runtime>()->AsId());
    ASSERT_NE(instr->Result()->Type(), nullptr);
    ASSERT_NE(instr->Result()->Type()->As<type::Void>(), nullptr);

    utils::StringStream str;
    instr->ToString(str);
    EXPECT_EQ(str.str(), "%42 (void) = discard");
}

}  // namespace
}  // namespace tint::ir
