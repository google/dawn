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

#include "src/ast/override_decoration.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, GlobalConstantDecl) {
  auto p = parser("let a : f32 = 1.");
  auto decos = p->decoration_list();
  EXPECT_FALSE(decos.errored);
  EXPECT_FALSE(decos.matched);
  auto e = p->global_constant_decl(decos.value);
  EXPECT_FALSE(p->has_error()) << p->error();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  ASSERT_NE(e.value, nullptr);

  EXPECT_TRUE(e->is_const);
  EXPECT_EQ(e->symbol, p->builder().Symbols().Get("a"));
  ASSERT_NE(e->type, nullptr);
  EXPECT_TRUE(e->type->Is<ast::F32>());

  EXPECT_EQ(e->source.range.begin.line, 1u);
  EXPECT_EQ(e->source.range.begin.column, 5u);
  EXPECT_EQ(e->source.range.end.line, 1u);
  EXPECT_EQ(e->source.range.end.column, 6u);

  ASSERT_NE(e->constructor, nullptr);
  EXPECT_TRUE(e->constructor->Is<ast::LiteralExpression>());

  EXPECT_FALSE(
      ast::HasDecoration<ast::OverrideDecoration>(e.value->decorations));
}

TEST_F(ParserImplTest, GlobalConstantDecl_Inferred) {
  auto p = parser("let a = 1.");
  auto decos = p->decoration_list();
  EXPECT_FALSE(decos.errored);
  EXPECT_FALSE(decos.matched);
  auto e = p->global_constant_decl(decos.value);
  EXPECT_FALSE(p->has_error()) << p->error();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  ASSERT_NE(e.value, nullptr);

  EXPECT_TRUE(e->is_const);
  EXPECT_EQ(e->symbol, p->builder().Symbols().Get("a"));
  EXPECT_EQ(e->type, nullptr);

  EXPECT_EQ(e->source.range.begin.line, 1u);
  EXPECT_EQ(e->source.range.begin.column, 5u);
  EXPECT_EQ(e->source.range.end.line, 1u);
  EXPECT_EQ(e->source.range.end.column, 6u);

  ASSERT_NE(e->constructor, nullptr);
  EXPECT_TRUE(e->constructor->Is<ast::LiteralExpression>());

  EXPECT_FALSE(
      ast::HasDecoration<ast::OverrideDecoration>(e.value->decorations));
}

TEST_F(ParserImplTest, GlobalConstantDecl_InvalidExpression) {
  auto p = parser("let a : f32 = if (a) {}");
  auto decos = p->decoration_list();
  EXPECT_FALSE(decos.errored);
  EXPECT_FALSE(decos.matched);
  auto e = p->global_constant_decl(decos.value);
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(e.errored);
  EXPECT_FALSE(e.matched);
  EXPECT_EQ(e.value, nullptr);
  EXPECT_EQ(p->error(), "1:15: invalid type for const_expr");
}

TEST_F(ParserImplTest, GlobalConstantDecl_MissingExpression) {
  auto p = parser("let a : f32 =");
  auto decos = p->decoration_list();
  EXPECT_FALSE(decos.errored);
  EXPECT_FALSE(decos.matched);
  auto e = p->global_constant_decl(decos.value);
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(e.errored);
  EXPECT_FALSE(e.matched);
  EXPECT_EQ(e.value, nullptr);
  EXPECT_EQ(p->error(), "1:14: unable to parse const_expr");
}

TEST_F(ParserImplTest, GlobalConstantDec_Override_WithId) {
  auto p = parser("[[override(7)]] let a : f32 = 1.");
  auto decos = p->decoration_list();
  EXPECT_FALSE(decos.errored);
  EXPECT_TRUE(decos.matched);

  auto e = p->global_constant_decl(decos.value);
  EXPECT_FALSE(p->has_error()) << p->error();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  ASSERT_NE(e.value, nullptr);

  EXPECT_TRUE(e->is_const);
  EXPECT_EQ(e->symbol, p->builder().Symbols().Get("a"));
  ASSERT_NE(e->type, nullptr);
  EXPECT_TRUE(e->type->Is<ast::F32>());

  EXPECT_EQ(e->source.range.begin.line, 1u);
  EXPECT_EQ(e->source.range.begin.column, 21u);
  EXPECT_EQ(e->source.range.end.line, 1u);
  EXPECT_EQ(e->source.range.end.column, 22u);

  ASSERT_NE(e->constructor, nullptr);
  EXPECT_TRUE(e->constructor->Is<ast::LiteralExpression>());

  auto* override_deco =
      ast::GetDecoration<ast::OverrideDecoration>(e.value->decorations);
  ASSERT_NE(override_deco, nullptr);
  EXPECT_TRUE(override_deco->has_value);
  EXPECT_EQ(override_deco->value, 7u);
}

TEST_F(ParserImplTest, GlobalConstantDec_Override_WithoutId) {
  auto p = parser("[[override]] let a : f32 = 1.");
  auto decos = p->decoration_list();
  EXPECT_FALSE(decos.errored);
  EXPECT_TRUE(decos.matched);

  auto e = p->global_constant_decl(decos.value);
  EXPECT_FALSE(p->has_error()) << p->error();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  ASSERT_NE(e.value, nullptr);

  EXPECT_TRUE(e->is_const);
  EXPECT_EQ(e->symbol, p->builder().Symbols().Get("a"));
  ASSERT_NE(e->type, nullptr);
  EXPECT_TRUE(e->type->Is<ast::F32>());

  EXPECT_EQ(e->source.range.begin.line, 1u);
  EXPECT_EQ(e->source.range.begin.column, 18u);
  EXPECT_EQ(e->source.range.end.line, 1u);
  EXPECT_EQ(e->source.range.end.column, 19u);

  ASSERT_NE(e->constructor, nullptr);
  EXPECT_TRUE(e->constructor->Is<ast::LiteralExpression>());

  auto* override_deco =
      ast::GetDecoration<ast::OverrideDecoration>(e.value->decorations);
  ASSERT_NE(override_deco, nullptr);
  EXPECT_FALSE(override_deco->has_value);
}

TEST_F(ParserImplTest, GlobalConstantDec_Override_MissingId) {
  auto p = parser("[[override()]] let a : f32 = 1.");
  auto decos = p->decoration_list();
  EXPECT_TRUE(decos.errored);
  EXPECT_FALSE(decos.matched);

  auto e = p->global_constant_decl(decos.value);
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  ASSERT_NE(e.value, nullptr);

  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(),
            "1:12: expected signed integer literal for override decoration");
}

TEST_F(ParserImplTest, GlobalConstantDec_Override_InvalidId) {
  auto p = parser("[[override(-7)]] let a : f32 = 1.");
  auto decos = p->decoration_list();
  EXPECT_TRUE(decos.errored);
  EXPECT_FALSE(decos.matched);

  auto e = p->global_constant_decl(decos.value);
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  ASSERT_NE(e.value, nullptr);

  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:12: override decoration must be positive");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
