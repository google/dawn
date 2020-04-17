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
#include "src/ast/variable_decl_statement.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, VariableStmt_VariableDecl) {
  auto* p = parser("var a : i32;");
  auto e = p->variable_stmt();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsVariableDecl());
  ASSERT_NE(e->variable(), nullptr);
  EXPECT_EQ(e->variable()->name(), "a");

  EXPECT_EQ(e->variable()->constructor(), nullptr);
}

TEST_F(ParserImplTest, VariableStmt_VariableDecl_WithInit) {
  auto* p = parser("var a : i32 = 1;");
  auto e = p->variable_stmt();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsVariableDecl());
  ASSERT_NE(e->variable(), nullptr);
  EXPECT_EQ(e->variable()->name(), "a");

  ASSERT_NE(e->variable()->constructor(), nullptr);
  EXPECT_TRUE(e->variable()->constructor()->IsConstructor());
}

TEST_F(ParserImplTest, VariableStmt_VariableDecl_Invalid) {
  auto* p = parser("var a : invalid;");
  auto e = p->variable_stmt();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:9: unknown type alias 'invalid'");
}

TEST_F(ParserImplTest, VariableStmt_VariableDecl_ConstructorInvalid) {
  auto* p = parser("var a : i32 = if(a) {}");
  auto e = p->variable_stmt();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:15: missing constructor for variable declaration");
}

TEST_F(ParserImplTest, VariableStmt_Const) {
  auto* p = parser("const a : i32 = 1");
  auto e = p->variable_stmt();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsVariableDecl());
}

TEST_F(ParserImplTest, VariableStmt_Const_InvalidVarIdent) {
  auto* p = parser("const a : invalid = 1");
  auto e = p->variable_stmt();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:11: unknown type alias 'invalid'");
}

TEST_F(ParserImplTest, VariableStmt_Const_MissingEqual) {
  auto* p = parser("const a : i32 1");
  auto e = p->variable_stmt();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:15: missing = for constant declaration");
}

TEST_F(ParserImplTest, VariableStmt_Const_MissingConstructor) {
  auto* p = parser("const a : i32 =");
  auto e = p->variable_stmt();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:16: missing constructor for const declaration");
}

TEST_F(ParserImplTest, VariableStmt_Const_InvalidConstructor) {
  auto* p = parser("const a : i32 = if (a) {}");
  auto e = p->variable_stmt();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:17: missing constructor for const declaration");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
