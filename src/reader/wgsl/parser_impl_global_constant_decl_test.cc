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
#include "src/ast/variable.h"
#include "src/ast/variable_decoration.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"
#include "src/type/f32_type.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, GlobalConstantDecl) {
  auto p = parser("const a : f32 = 1.");
  auto e = p->global_constant_decl();
  EXPECT_FALSE(p->has_error()) << p->error();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  ASSERT_NE(e.value, nullptr);

  EXPECT_TRUE(e->is_const());
  EXPECT_EQ(e->symbol(), p->builder().Symbols().Get("a"));
  ASSERT_NE(e->type(), nullptr);
  EXPECT_TRUE(e->type()->Is<type::F32>());

  EXPECT_EQ(e->source().range.begin.line, 1u);
  EXPECT_EQ(e->source().range.begin.column, 7u);
  EXPECT_EQ(e->source().range.end.line, 1u);
  EXPECT_EQ(e->source().range.end.column, 8u);

  ASSERT_NE(e->constructor(), nullptr);
  EXPECT_TRUE(e->constructor()->Is<ast::ConstructorExpression>());
}

TEST_F(ParserImplTest, GlobalConstantDecl_MissingEqual) {
  auto p = parser("const a: f32 1.");
  auto e = p->global_constant_decl();
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(e.errored);
  EXPECT_FALSE(e.matched);
  EXPECT_EQ(e.value, nullptr);
  EXPECT_EQ(p->error(), "1:14: expected '=' for constant declaration");
}

TEST_F(ParserImplTest, GlobalConstantDecl_InvalidVariable) {
  auto p = parser("const a: invalid = 1.");
  auto e = p->global_constant_decl();
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(e.errored);
  EXPECT_FALSE(e.matched);
  EXPECT_EQ(e.value, nullptr);
  EXPECT_EQ(p->error(), "1:10: unknown constructed type 'invalid'");
}

TEST_F(ParserImplTest, GlobalConstantDecl_InvalidExpression) {
  auto p = parser("const a: f32 = if (a) {}");
  auto e = p->global_constant_decl();
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(e.errored);
  EXPECT_FALSE(e.matched);
  EXPECT_EQ(e.value, nullptr);
  EXPECT_EQ(p->error(), "1:16: unable to parse const literal");
}

TEST_F(ParserImplTest, GlobalConstantDecl_MissingExpression) {
  auto p = parser("const a: f32 =");
  auto e = p->global_constant_decl();
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(e.errored);
  EXPECT_FALSE(e.matched);
  EXPECT_EQ(e.value, nullptr);
  EXPECT_EQ(p->error(), "1:15: unable to parse const literal");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
