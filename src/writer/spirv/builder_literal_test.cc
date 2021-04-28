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

#include "src/writer/spirv/spv_dump.h"
#include "src/writer/spirv/test_helper.h"

namespace tint {
namespace writer {
namespace spirv {

using BuilderTest = TestHelper;

TEST_F(BuilderTest, Literal_Bool_True) {
  auto* b_true = create<ast::BoolLiteral>(true);
  WrapInFunction(b_true);

  spirv::Builder& b = Build();

  auto id = b.GenerateLiteralIfNeeded(nullptr, b_true);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(2u, id);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeBool
%2 = OpConstantTrue %1
)");
}

TEST_F(BuilderTest, Literal_Bool_False) {
  auto* b_false = create<ast::BoolLiteral>(false);
  WrapInFunction(b_false);

  spirv::Builder& b = Build();

  auto id = b.GenerateLiteralIfNeeded(nullptr, b_false);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(2u, id);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeBool
%2 = OpConstantFalse %1
)");
}

TEST_F(BuilderTest, Literal_Bool_Dedup) {
  auto* b_true = create<ast::BoolLiteral>(true);
  auto* b_false = create<ast::BoolLiteral>(false);
  WrapInFunction(b_true, b_false);

  spirv::Builder& b = Build();

  ASSERT_NE(b.GenerateLiteralIfNeeded(nullptr, b_true), 0u);
  ASSERT_FALSE(b.has_error()) << b.error();
  ASSERT_NE(b.GenerateLiteralIfNeeded(nullptr, b_false), 0u);
  ASSERT_FALSE(b.has_error()) << b.error();
  ASSERT_NE(b.GenerateLiteralIfNeeded(nullptr, b_true), 0u);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeBool
%2 = OpConstantTrue %1
%3 = OpConstantFalse %1
)");
}

TEST_F(BuilderTest, Literal_I32) {
  auto* i = create<ast::SintLiteral>(-23);
  WrapInFunction(i);
  spirv::Builder& b = Build();

  auto id = b.GenerateLiteralIfNeeded(nullptr, i);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(2u, id);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeInt 32 1
%2 = OpConstant %1 -23
)");
}

TEST_F(BuilderTest, Literal_I32_Dedup) {
  auto* i1 = create<ast::SintLiteral>(-23);
  auto* i2 = create<ast::SintLiteral>(-23);
  WrapInFunction(i1, i2);

  spirv::Builder& b = Build();

  ASSERT_NE(b.GenerateLiteralIfNeeded(nullptr, i1), 0u);
  ASSERT_NE(b.GenerateLiteralIfNeeded(nullptr, i2), 0u);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeInt 32 1
%2 = OpConstant %1 -23
)");
}

TEST_F(BuilderTest, Literal_U32) {
  auto* i = create<ast::UintLiteral>(23);
  WrapInFunction(i);

  spirv::Builder& b = Build();

  auto id = b.GenerateLiteralIfNeeded(nullptr, i);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(2u, id);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeInt 32 0
%2 = OpConstant %1 23
)");
}

TEST_F(BuilderTest, Literal_U32_Dedup) {
  auto* i1 = create<ast::UintLiteral>(23);
  auto* i2 = create<ast::UintLiteral>(23);
  WrapInFunction(i1, i2);

  spirv::Builder& b = Build();

  ASSERT_NE(b.GenerateLiteralIfNeeded(nullptr, i1), 0u);
  ASSERT_NE(b.GenerateLiteralIfNeeded(nullptr, i2), 0u);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeInt 32 0
%2 = OpConstant %1 23
)");
}

TEST_F(BuilderTest, Literal_F32) {
  auto* i = create<ast::FloatLiteral>(23.245f);
  WrapInFunction(i);

  spirv::Builder& b = Build();

  auto id = b.GenerateLiteralIfNeeded(nullptr, i);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(2u, id);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 23.2450008
)");
}

TEST_F(BuilderTest, Literal_F32_Dedup) {
  auto* i1 = create<ast::FloatLiteral>(23.245f);
  auto* i2 = create<ast::FloatLiteral>(23.245f);
  WrapInFunction(i1, i2);

  spirv::Builder& b = Build();

  ASSERT_NE(b.GenerateLiteralIfNeeded(nullptr, i1), 0u);
  ASSERT_NE(b.GenerateLiteralIfNeeded(nullptr, i2), 0u);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 23.2450008
)");
}

}  // namespace spirv
}  // namespace writer
}  // namespace tint
