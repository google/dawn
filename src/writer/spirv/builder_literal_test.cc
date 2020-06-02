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

#include "gtest/gtest.h"
#include "spirv/unified1/spirv.h"
#include "src/ast/bool_literal.h"
#include "src/ast/float_literal.h"
#include "src/ast/literal.h"
#include "src/ast/sint_literal.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/uint_literal.h"
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"

namespace tint {
namespace writer {
namespace spirv {

using BuilderTest = testing::Test;

TEST_F(BuilderTest, Literal_Bool_True) {
  ast::type::BoolType bool_type;
  ast::BoolLiteral b_true(&bool_type, true);

  ast::Module mod;
  Builder b(&mod);
  auto id = b.GenerateLiteralIfNeeded(&b_true);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(2u, id);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeBool
%2 = OpConstantTrue %1
)");
}

TEST_F(BuilderTest, Literal_Bool_False) {
  ast::type::BoolType bool_type;
  ast::BoolLiteral b_false(&bool_type, false);

  ast::Module mod;
  Builder b(&mod);
  auto id = b.GenerateLiteralIfNeeded(&b_false);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(2u, id);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeBool
%2 = OpConstantFalse %1
)");
}

TEST_F(BuilderTest, Literal_Bool_Dedup) {
  ast::type::BoolType bool_type;
  ast::BoolLiteral b_true(&bool_type, true);
  ast::BoolLiteral b_false(&bool_type, false);

  ast::Module mod;
  Builder b(&mod);
  ASSERT_NE(b.GenerateLiteralIfNeeded(&b_true), 0u);
  ASSERT_FALSE(b.has_error()) << b.error();
  ASSERT_NE(b.GenerateLiteralIfNeeded(&b_false), 0u);
  ASSERT_FALSE(b.has_error()) << b.error();
  ASSERT_NE(b.GenerateLiteralIfNeeded(&b_true), 0u);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeBool
%2 = OpConstantTrue %1
%3 = OpConstantFalse %1
)");
}

TEST_F(BuilderTest, Literal_I32) {
  ast::type::I32Type i32;
  ast::SintLiteral i(&i32, -23);

  ast::Module mod;
  Builder b(&mod);
  auto id = b.GenerateLiteralIfNeeded(&i);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(2u, id);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeInt 32 1
%2 = OpConstant %1 -23
)");
}

TEST_F(BuilderTest, Literal_I32_Dedup) {
  ast::type::I32Type i32;
  ast::SintLiteral i1(&i32, -23);
  ast::SintLiteral i2(&i32, -23);

  ast::Module mod;
  Builder b(&mod);
  ASSERT_NE(b.GenerateLiteralIfNeeded(&i1), 0u);
  ASSERT_NE(b.GenerateLiteralIfNeeded(&i2), 0u);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeInt 32 1
%2 = OpConstant %1 -23
)");
}

TEST_F(BuilderTest, Literal_U32) {
  ast::type::U32Type u32;
  ast::UintLiteral i(&u32, 23);

  ast::Module mod;
  Builder b(&mod);
  auto id = b.GenerateLiteralIfNeeded(&i);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(2u, id);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeInt 32 0
%2 = OpConstant %1 23
)");
}

TEST_F(BuilderTest, Literal_U32_Dedup) {
  ast::type::U32Type u32;
  ast::UintLiteral i1(&u32, 23);
  ast::UintLiteral i2(&u32, 23);

  ast::Module mod;
  Builder b(&mod);
  ASSERT_NE(b.GenerateLiteralIfNeeded(&i1), 0u);
  ASSERT_NE(b.GenerateLiteralIfNeeded(&i2), 0u);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeInt 32 0
%2 = OpConstant %1 23
)");
}

TEST_F(BuilderTest, Literal_F32) {
  ast::type::F32Type f32;
  ast::FloatLiteral i(&f32, 23.245f);

  ast::Module mod;
  Builder b(&mod);
  auto id = b.GenerateLiteralIfNeeded(&i);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(2u, id);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 23.2450008
)");
}

TEST_F(BuilderTest, Literal_F32_Dedup) {
  ast::type::F32Type f32;
  ast::FloatLiteral i1(&f32, 23.245f);
  ast::FloatLiteral i2(&f32, 23.245f);

  ast::Module mod;
  Builder b(&mod);
  ASSERT_NE(b.GenerateLiteralIfNeeded(&i1), 0u);
  ASSERT_NE(b.GenerateLiteralIfNeeded(&i2), 0u);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 23.2450008
)");
}

}  // namespace spirv
}  // namespace writer
}  // namespace tint
