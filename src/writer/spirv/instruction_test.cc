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

#include "src/writer/spirv/instruction.h"

#include "gtest/gtest.h"
#include "src/writer/spirv/operand.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using InstructionTest = testing::Test;

TEST_F(InstructionTest, Create) {
  Instruction i(spv::Op::OpEntryPoint, {Operand::Float(1.2f), Operand::Int(1),
                                        Operand::String("my_str")});
  EXPECT_EQ(i.opcode(), spv::Op::OpEntryPoint);
  ASSERT_EQ(i.operands().size(), 3u);

  const auto& ops = i.operands();
  EXPECT_TRUE(ops[0].IsFloat());
  EXPECT_FLOAT_EQ(ops[0].to_f(), 1.2f);

  EXPECT_TRUE(ops[1].IsInt());
  EXPECT_EQ(ops[1].to_i(), 1u);

  EXPECT_TRUE(ops[2].IsString());
  EXPECT_EQ(ops[2].to_s(), "my_str");
}

TEST_F(InstructionTest, Length) {
  Instruction i(spv::Op::OpEntryPoint, {Operand::Float(1.2f), Operand::Int(1),
                                        Operand::String("my_str")});
  EXPECT_EQ(i.word_length(), 5u);
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
