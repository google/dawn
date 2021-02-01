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
  auto decos = p->decoration_list();
  EXPECT_FALSE(decos.errored);
  EXPECT_FALSE(decos.matched);
  auto e = p->global_constant_decl(decos.value);
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

  EXPECT_FALSE(e.value->HasConstantIdDecoration());
}

TEST_F(ParserImplTest, GlobalConstantDecl_MissingEqual) {
  auto p = parser("const a: f32 1.");
  auto decos = p->decoration_list();
  EXPECT_FALSE(decos.errored);
  EXPECT_FALSE(decos.matched);
  auto e = p->global_constant_decl(decos.value);
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(e.errored);
  EXPECT_FALSE(e.matched);
  EXPECT_EQ(e.value, nullptr);
  EXPECT_EQ(p->error(), "1:14: expected '=' for constant declaration");
}

TEST_F(ParserImplTest, GlobalConstantDecl_InvalidVariable) {
  auto p = parser("const a: invalid = 1.");
  auto decos = p->decoration_list();
  EXPECT_FALSE(decos.errored);
  EXPECT_FALSE(decos.matched);
  auto e = p->global_constant_decl(decos.value);
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(e.errored);
  EXPECT_FALSE(e.matched);
  EXPECT_EQ(e.value, nullptr);
  EXPECT_EQ(p->error(), "1:10: unknown constructed type 'invalid'");
}

TEST_F(ParserImplTest, GlobalConstantDecl_InvalidExpression) {
  auto p = parser("const a: f32 = if (a) {}");
  auto decos = p->decoration_list();
  EXPECT_FALSE(decos.errored);
  EXPECT_FALSE(decos.matched);
  auto e = p->global_constant_decl(decos.value);
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(e.errored);
  EXPECT_FALSE(e.matched);
  EXPECT_EQ(e.value, nullptr);
  EXPECT_EQ(p->error(), "1:16: unable to parse const literal");
}

TEST_F(ParserImplTest, GlobalConstantDecl_MissingExpression) {
  auto p = parser("const a: f32 =");
  auto decos = p->decoration_list();
  EXPECT_FALSE(decos.errored);
  EXPECT_FALSE(decos.matched);
  auto e = p->global_constant_decl(decos.value);
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(e.errored);
  EXPECT_FALSE(e.matched);
  EXPECT_EQ(e.value, nullptr);
  EXPECT_EQ(p->error(), "1:15: unable to parse const literal");
}

TEST_F(ParserImplTest, GlobalConstantDec_ConstantId) {
  auto p = parser("[[constant_id(7)]] const a : f32 = 1.");
  auto decos = p->decoration_list();
  EXPECT_FALSE(decos.errored);
  EXPECT_TRUE(decos.matched);

  auto e = p->global_constant_decl(decos.value);
  EXPECT_FALSE(p->has_error()) << p->error();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  ASSERT_NE(e.value, nullptr);

  EXPECT_TRUE(e->is_const());
  EXPECT_EQ(e->symbol(), p->builder().Symbols().Get("a"));
  ASSERT_NE(e->type(), nullptr);
  EXPECT_TRUE(e->type()->Is<type::F32>());

  EXPECT_EQ(e->source().range.begin.line, 1u);
  EXPECT_EQ(e->source().range.begin.column, 26u);
  EXPECT_EQ(e->source().range.end.line, 1u);
  EXPECT_EQ(e->source().range.end.column, 27u);

  ASSERT_NE(e->constructor(), nullptr);
  EXPECT_TRUE(e->constructor()->Is<ast::ConstructorExpression>());

  EXPECT_TRUE(e.value->HasConstantIdDecoration());
  EXPECT_EQ(e.value->constant_id(), 7u);
}

TEST_F(ParserImplTest, GlobalConstantDec_ConstantId_Missing) {
  auto p = parser("[[constant_id()]] const a : f32 = 1.");
  auto decos = p->decoration_list();
  EXPECT_TRUE(decos.errored);
  EXPECT_FALSE(decos.matched);

  auto e = p->global_constant_decl(decos.value);
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  ASSERT_NE(e.value, nullptr);

  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(),
            "1:15: expected signed integer literal for constant_id decoration");
}

TEST_F(ParserImplTest, GlobalConstantDec_ConstantId_Invalid) {
  auto p = parser("[[constant_id(-7)]] const a : f32 = 1.");
  auto decos = p->decoration_list();
  EXPECT_TRUE(decos.errored);
  EXPECT_FALSE(decos.matched);

  auto e = p->global_constant_decl(decos.value);
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  ASSERT_NE(e.value, nullptr);

  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:15: constant_id decoration must be positive");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
