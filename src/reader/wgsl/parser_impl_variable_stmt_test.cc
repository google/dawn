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

#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, VariableStmt_VariableDecl) {
  auto p = parser("var a : i32;");
  auto e = p->variable_stmt();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e.value, nullptr);
  ASSERT_TRUE(e->Is<ast::VariableDeclStatement>());
  ASSERT_NE(e->variable(), nullptr);
  EXPECT_EQ(e->variable()->symbol(), p->builder().Symbols().Get("a"));

  ASSERT_EQ(e->source().range.begin.line, 1u);
  ASSERT_EQ(e->source().range.begin.column, 5u);
  ASSERT_EQ(e->source().range.end.line, 1u);
  ASSERT_EQ(e->source().range.end.column, 6u);

  EXPECT_EQ(e->variable()->constructor(), nullptr);
}

TEST_F(ParserImplTest, VariableStmt_VariableDecl_WithInit) {
  auto p = parser("var a : i32 = 1;");
  auto e = p->variable_stmt();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e.value, nullptr);
  ASSERT_TRUE(e->Is<ast::VariableDeclStatement>());
  ASSERT_NE(e->variable(), nullptr);
  EXPECT_EQ(e->variable()->symbol(), p->builder().Symbols().Get("a"));

  ASSERT_EQ(e->source().range.begin.line, 1u);
  ASSERT_EQ(e->source().range.begin.column, 5u);
  ASSERT_EQ(e->source().range.end.line, 1u);
  ASSERT_EQ(e->source().range.end.column, 6u);

  ASSERT_NE(e->variable()->constructor(), nullptr);
  EXPECT_TRUE(e->variable()->constructor()->Is<ast::ConstructorExpression>());
}

TEST_F(ParserImplTest, VariableStmt_VariableDecl_Invalid) {
  auto p = parser("var a : invalid;");
  auto e = p->variable_stmt();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_EQ(e.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:9: unknown constructed type 'invalid'");
}

TEST_F(ParserImplTest, VariableStmt_VariableDecl_ConstructorInvalid) {
  auto p = parser("var a : i32 = if(a) {}");
  auto e = p->variable_stmt();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_EQ(e.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:15: missing constructor for variable declaration");
}

TEST_F(ParserImplTest, VariableStmt_VariableDecl_ArrayInit) {
  auto p = parser("var a : array<i32> = array<i32>();");
  auto e = p->variable_stmt();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e.value, nullptr);
  ASSERT_TRUE(e->Is<ast::VariableDeclStatement>());
  ASSERT_NE(e->variable(), nullptr);
  EXPECT_EQ(e->variable()->symbol(), p->builder().Symbols().Get("a"));

  ASSERT_NE(e->variable()->constructor(), nullptr);
  EXPECT_TRUE(e->variable()->constructor()->Is<ast::ConstructorExpression>());
}

TEST_F(ParserImplTest, VariableStmt_VariableDecl_ArrayInit_NoSpace) {
  auto p = parser("var a : array<i32>=array<i32>();");
  auto e = p->variable_stmt();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e.value, nullptr);
  ASSERT_TRUE(e->Is<ast::VariableDeclStatement>());
  ASSERT_NE(e->variable(), nullptr);
  EXPECT_EQ(e->variable()->symbol(), p->builder().Symbols().Get("a"));

  ASSERT_NE(e->variable()->constructor(), nullptr);
  EXPECT_TRUE(e->variable()->constructor()->Is<ast::ConstructorExpression>());
}

TEST_F(ParserImplTest, VariableStmt_VariableDecl_VecInit) {
  auto p = parser("var a : vec2<i32> = vec2<i32>();");
  auto e = p->variable_stmt();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e.value, nullptr);
  ASSERT_TRUE(e->Is<ast::VariableDeclStatement>());
  ASSERT_NE(e->variable(), nullptr);
  EXPECT_EQ(e->variable()->symbol(), p->builder().Symbols().Get("a"));

  ASSERT_NE(e->variable()->constructor(), nullptr);
  EXPECT_TRUE(e->variable()->constructor()->Is<ast::ConstructorExpression>());
}

TEST_F(ParserImplTest, VariableStmt_VariableDecl_VecInit_NoSpace) {
  auto p = parser("var a : vec2<i32>=vec2<i32>();");
  auto e = p->variable_stmt();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e.value, nullptr);
  ASSERT_TRUE(e->Is<ast::VariableDeclStatement>());
  ASSERT_NE(e->variable(), nullptr);
  EXPECT_EQ(e->variable()->symbol(), p->builder().Symbols().Get("a"));

  ASSERT_NE(e->variable()->constructor(), nullptr);
  EXPECT_TRUE(e->variable()->constructor()->Is<ast::ConstructorExpression>());
}

TEST_F(ParserImplTest, VariableStmt_Let) {
  auto p = parser("let a : i32 = 1");
  auto e = p->variable_stmt();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e.value, nullptr);
  ASSERT_TRUE(e->Is<ast::VariableDeclStatement>());

  ASSERT_EQ(e->source().range.begin.line, 1u);
  ASSERT_EQ(e->source().range.begin.column, 5u);
  ASSERT_EQ(e->source().range.end.line, 1u);
  ASSERT_EQ(e->source().range.end.column, 6u);
}

TEST_F(ParserImplTest, VariableStmt_Let_InvalidVarIdent) {
  auto p = parser("let a : invalid = 1");
  auto e = p->variable_stmt();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_EQ(e.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:9: unknown constructed type 'invalid'");
}

TEST_F(ParserImplTest, VariableStmt_Let_MissingEqual) {
  auto p = parser("let a : i32 1");
  auto e = p->variable_stmt();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_EQ(e.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:13: expected '=' for let declaration");
}

TEST_F(ParserImplTest, VariableStmt_Let_MissingConstructor) {
  auto p = parser("let a : i32 =");
  auto e = p->variable_stmt();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_EQ(e.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:14: missing constructor for let declaration");
}

TEST_F(ParserImplTest, VariableStmt_Let_InvalidConstructor) {
  auto p = parser("let a : i32 = if (a) {}");
  auto e = p->variable_stmt();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_EQ(e.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:15: missing constructor for let declaration");
}

TEST_F(ParserImplTest, VariableStmt_Const) {
  auto p = parser("const a : i32 = 1");
  auto e = p->variable_stmt();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  EXPECT_EQ(
      p->builder().Diagnostics().str(),
      R"(test.wgsl:1:1 warning: use of deprecated language feature: use 'let' instead of 'const'
const a : i32 = 1
^^^^^
)");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
