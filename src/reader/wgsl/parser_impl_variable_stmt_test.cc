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
#include "src/ast/statement.h"
#include "src/ast/variable_statement.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {

TEST_F(ParserImplTest, VariableStmt_VariableDecl) {
  auto p = parser("var a : i32;");
  auto e = p->variable_stmt();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsVariable());
  ASSERT_NE(e->variable(), nullptr);
  EXPECT_EQ(e->variable()->name(), "a");

  EXPECT_EQ(e->variable()->initializer(), nullptr);
}

TEST_F(ParserImplTest, VariableStmt_VariableDecl_WithInit) {
  auto p = parser("var a : i32 = 1;");
  auto e = p->variable_stmt();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsVariable());
  ASSERT_NE(e->variable(), nullptr);
  EXPECT_EQ(e->variable()->name(), "a");

  ASSERT_NE(e->variable()->initializer(), nullptr);
  EXPECT_TRUE(e->variable()->initializer()->IsInitializer());
}

TEST_F(ParserImplTest, VariableStmt_VariableDecl_Invalid) {
  auto p = parser("var a : invalid;");
  auto e = p->variable_stmt();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:9: unknown type alias 'invalid'");
}

TEST_F(ParserImplTest, VariableStmt_VariableDecl_InitializerInvalid) {
  auto p = parser("var a : i32 = if(a) {}");
  auto e = p->variable_stmt();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:15: missing initializer for variable declaration");
}

TEST_F(ParserImplTest, VariableStmt_Const) {
  auto p = parser("const a : i32 = 1");
  auto e = p->variable_stmt();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsVariable());
}

TEST_F(ParserImplTest, VariableStmt_Const_InvalidVarIdent) {
  auto p = parser("const a : invalid = 1");
  auto e = p->variable_stmt();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:11: unknown type alias 'invalid'");
}

TEST_F(ParserImplTest, VariableStmt_Const_MissingEqual) {
  auto p = parser("const a : i32 1");
  auto e = p->variable_stmt();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:15: missing = for constant declaration");
}

TEST_F(ParserImplTest, VariableStmt_Const_MissingInitializer) {
  auto p = parser("const a : i32 =");
  auto e = p->variable_stmt();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:16: missing initializer for const declaration");
}

TEST_F(ParserImplTest, VariableStmt_Const_InvalidInitializer) {
  auto p = parser("const a : i32 = if (a) {}");
  auto e = p->variable_stmt();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:17: missing initializer for const declaration");
}

}  // namespace wgsl
}  // namespace reader
}  // namespace tint
