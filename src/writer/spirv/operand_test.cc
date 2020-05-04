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

#include "src/writer/spirv/operand.h"

#include "gtest/gtest.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using OperandTest = testing::Test;

TEST_F(OperandTest, CreateFloat) {
  auto o = Operand::Float(1.2f);
  EXPECT_TRUE(o.IsFloat());
  EXPECT_FLOAT_EQ(o.to_f(), 1.2f);
}

TEST_F(OperandTest, CreateInt) {
  auto o = Operand::Int(1);
  EXPECT_TRUE(o.IsInt());
  EXPECT_EQ(o.to_i(), 1u);
}

TEST_F(OperandTest, CreateString) {
  auto o = Operand::String("my string");
  EXPECT_TRUE(o.IsString());
  EXPECT_EQ(o.to_s(), "my string");
}

TEST_F(OperandTest, Length_Float) {
  auto o = Operand::Float(1.2f);
  EXPECT_EQ(o.length(), 1u);
}

TEST_F(OperandTest, Length_Int) {
  auto o = Operand::Int(1);
  EXPECT_EQ(o.length(), 1u);
}

TEST_F(OperandTest, Length_String) {
  auto o = Operand::String("my string");
  EXPECT_EQ(o.length(), 3u);
}

TEST_F(OperandTest, Length_String_Empty) {
  auto o = Operand::String("");
  EXPECT_EQ(o.length(), 1u);
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
