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

#include "src/tint/writer/spirv/operand.h"

#include "gtest/gtest.h"

namespace tint::writer::spirv {
namespace {

using OperandTest = testing::Test;

TEST_F(OperandTest, CreateFloat) {
    auto o = Operand(1.2f);
    ASSERT_TRUE(std::holds_alternative<float>(o));
    EXPECT_FLOAT_EQ(std::get<float>(o), 1.2f);
}

TEST_F(OperandTest, CreateInt) {
    auto o = Operand(1u);
    ASSERT_TRUE(std::holds_alternative<uint32_t>(o));
    EXPECT_EQ(std::get<uint32_t>(o), 1u);
}

TEST_F(OperandTest, CreateString) {
    auto o = Operand("my string");
    ASSERT_TRUE(std::holds_alternative<std::string>(o));
    EXPECT_EQ(std::get<std::string>(o), "my string");
}

TEST_F(OperandTest, Length_Float) {
    auto o = Operand(1.2f);
    EXPECT_EQ(OperandLength(o), 1u);
}

TEST_F(OperandTest, Length_Int) {
    auto o = U32Operand(1);
    EXPECT_EQ(OperandLength(o), 1u);
}

TEST_F(OperandTest, Length_String) {
    auto o = Operand("my string");
    EXPECT_EQ(OperandLength(o), 3u);
}

TEST_F(OperandTest, Length_String_Empty) {
    auto o = Operand("");
    EXPECT_EQ(OperandLength(o), 1u);
}

}  // namespace
}  // namespace tint::writer::spirv
