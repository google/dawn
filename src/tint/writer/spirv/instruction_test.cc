// Copyright 2020 The Tint Authors.
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

#include "src/tint/writer/spirv/instruction.h"

#include <string>

#include "gtest/gtest.h"

namespace tint::writer::spirv {
namespace {

using InstructionTest = testing::Test;

TEST_F(InstructionTest, Create) {
    Instruction i(spv::Op::OpEntryPoint, {Operand(1.2f), Operand(1u), Operand("my_str")});
    EXPECT_EQ(i.opcode(), spv::Op::OpEntryPoint);
    ASSERT_EQ(i.operands().size(), 3u);

    const auto& ops = i.operands();
    ASSERT_TRUE(std::holds_alternative<float>(ops[0]));
    EXPECT_FLOAT_EQ(std::get<float>(ops[0]), 1.2f);

    ASSERT_TRUE(std::holds_alternative<uint32_t>(ops[1]));
    EXPECT_EQ(std::get<uint32_t>(ops[1]), 1u);

    ASSERT_TRUE(std::holds_alternative<std::string>(ops[2]));
    EXPECT_EQ(std::get<std::string>(ops[2]), "my_str");
}

TEST_F(InstructionTest, Length) {
    Instruction i(spv::Op::OpEntryPoint, {Operand(1.2f), Operand(1u), Operand("my_str")});
    EXPECT_EQ(i.word_length(), 5u);
}

}  // namespace
}  // namespace tint::writer::spirv
