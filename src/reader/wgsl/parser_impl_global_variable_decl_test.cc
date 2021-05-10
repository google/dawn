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

TEST_F(ParserImplTest, GlobalVariableDecl_WithoutConstructor) {
  auto p = parser("var<private> a : f32");
  auto decos = p->decoration_list();
  EXPECT_FALSE(decos.errored);
  EXPECT_FALSE(decos.matched);
  auto e = p->global_variable_decl(decos.value);
  ASSERT_FALSE(p->has_error()) << p->error();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  ASSERT_NE(e.value, nullptr);

  EXPECT_EQ(e->symbol(), p->builder().Symbols().Get("a"));
  EXPECT_TRUE(e->type()->Is<ast::F32>());
  EXPECT_EQ(e->declared_storage_class(), ast::StorageClass::kPrivate);

  EXPECT_EQ(e->source().range.begin.line, 1u);
  EXPECT_EQ(e->source().range.begin.column, 14u);
  EXPECT_EQ(e->source().range.end.line, 1u);
  EXPECT_EQ(e->source().range.end.column, 15u);

  ASSERT_EQ(e->constructor(), nullptr);
}

TEST_F(ParserImplTest, GlobalVariableDecl_WithConstructor) {
  auto p = parser("var<private> a : f32 = 1.");
  auto decos = p->decoration_list();
  EXPECT_FALSE(decos.errored);
  EXPECT_FALSE(decos.matched);
  auto e = p->global_variable_decl(decos.value);
  ASSERT_FALSE(p->has_error()) << p->error();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  ASSERT_NE(e.value, nullptr);

  EXPECT_EQ(e->symbol(), p->builder().Symbols().Get("a"));
  EXPECT_TRUE(e->type()->Is<ast::F32>());
  EXPECT_EQ(e->declared_storage_class(), ast::StorageClass::kPrivate);

  EXPECT_EQ(e->source().range.begin.line, 1u);
  EXPECT_EQ(e->source().range.begin.column, 14u);
  EXPECT_EQ(e->source().range.end.line, 1u);
  EXPECT_EQ(e->source().range.end.column, 15u);

  ASSERT_NE(e->constructor(), nullptr);
  ASSERT_TRUE(e->constructor()->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(e->constructor()->Is<ast::ScalarConstructorExpression>());
}

TEST_F(ParserImplTest, GlobalVariableDecl_WithDecoration) {
  auto p = parser("[[binding(2), group(1)]] var<uniform> a : f32");
  auto decos = p->decoration_list();
  EXPECT_FALSE(decos.errored);
  EXPECT_TRUE(decos.matched);
  auto e = p->global_variable_decl(decos.value);
  ASSERT_FALSE(p->has_error()) << p->error();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  ASSERT_NE(e.value, nullptr);

  EXPECT_EQ(e->symbol(), p->builder().Symbols().Get("a"));
  ASSERT_NE(e->type(), nullptr);
  EXPECT_TRUE(e->type()->Is<ast::F32>());
  EXPECT_EQ(e->declared_storage_class(), ast::StorageClass::kUniform);

  EXPECT_EQ(e->source().range.begin.line, 1u);
  EXPECT_EQ(e->source().range.begin.column, 39u);
  EXPECT_EQ(e->source().range.end.line, 1u);
  EXPECT_EQ(e->source().range.end.column, 40u);

  ASSERT_EQ(e->constructor(), nullptr);

  auto& decorations = e->decorations();
  ASSERT_EQ(decorations.size(), 2u);
  ASSERT_TRUE(decorations[0]->Is<ast::BindingDecoration>());
  ASSERT_TRUE(decorations[1]->Is<ast::GroupDecoration>());
}

TEST_F(ParserImplTest, GlobalVariableDecl_WithDecoration_MulitpleGroups) {
  auto p = parser("[[binding(2)]] [[group(1)]] var<uniform> a : f32");
  auto decos = p->decoration_list();
  EXPECT_FALSE(decos.errored);
  EXPECT_TRUE(decos.matched);

  auto e = p->global_variable_decl(decos.value);
  ASSERT_FALSE(p->has_error()) << p->error();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  ASSERT_NE(e.value, nullptr);

  EXPECT_EQ(e->symbol(), p->builder().Symbols().Get("a"));
  ASSERT_NE(e->type(), nullptr);
  EXPECT_TRUE(e->type()->Is<ast::F32>());
  EXPECT_EQ(e->declared_storage_class(), ast::StorageClass::kUniform);

  EXPECT_EQ(e->source().range.begin.line, 1u);
  EXPECT_EQ(e->source().range.begin.column, 42u);
  EXPECT_EQ(e->source().range.end.line, 1u);
  EXPECT_EQ(e->source().range.end.column, 43u);

  ASSERT_EQ(e->constructor(), nullptr);

  auto& decorations = e->decorations();
  ASSERT_EQ(decorations.size(), 2u);
  ASSERT_TRUE(decorations[0]->Is<ast::BindingDecoration>());
  ASSERT_TRUE(decorations[1]->Is<ast::GroupDecoration>());
}

TEST_F(ParserImplTest, GlobalVariableDecl_InvalidDecoration) {
  auto p = parser("[[binding()]] var<uniform> a : f32");
  auto decos = p->decoration_list();
  EXPECT_TRUE(decos.errored);
  EXPECT_FALSE(decos.matched);

  auto e = p->global_variable_decl(decos.value);
  EXPECT_FALSE(e.errored);
  EXPECT_TRUE(e.matched);
  EXPECT_NE(e.value, nullptr);

  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(),
            "1:11: expected signed integer literal for binding decoration");
}

TEST_F(ParserImplTest, GlobalVariableDecl_InvalidConstExpr) {
  auto p = parser("var<private> a : f32 = if (a) {}");
  auto decos = p->decoration_list();
  EXPECT_FALSE(decos.errored);
  EXPECT_FALSE(decos.matched);
  auto e = p->global_variable_decl(decos.value);
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(e.errored);
  EXPECT_FALSE(e.matched);
  EXPECT_EQ(e.value, nullptr);
  EXPECT_EQ(p->error(), "1:24: unable to parse constant literal");
}

TEST_F(ParserImplTest, GlobalVariableDecl_InvalidVariableDecl) {
  auto p = parser("var<invalid> a : f32;");
  auto decos = p->decoration_list();
  EXPECT_FALSE(decos.errored);
  EXPECT_FALSE(decos.matched);
  auto e = p->global_variable_decl(decos.value);
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(e.errored);
  EXPECT_FALSE(e.matched);
  EXPECT_EQ(e.value, nullptr);
  EXPECT_EQ(p->error(), "1:5: invalid storage class for variable decoration");
}

TEST_F(ParserImplTest, GlobalVariableDecl_StorageClassIn_Deprecated) {
  auto p = parser("[[location(0)]] var<in> a : f32");
  auto f = p->function_header();
  auto decos = p->decoration_list();
  EXPECT_FALSE(decos.errored);
  EXPECT_TRUE(decos.matched);
  auto e = p->global_variable_decl(decos.value);
  ASSERT_FALSE(p->has_error()) << p->error();

  EXPECT_EQ(e->symbol(), p->builder().Symbols().Get("a"));
  EXPECT_TRUE(e->type()->Is<ast::F32>());
  EXPECT_EQ(e->declared_storage_class(), ast::StorageClass::kInput);

  EXPECT_EQ(
      p->builder().Diagnostics().str(),
      R"(test.wgsl:1:21 warning: use of deprecated language feature: use an entry point parameter instead of a variable in the `in` storage class
[[location(0)]] var<in> a : f32
                    ^^
)");
}

TEST_F(ParserImplTest, GlobalVariableDecl_StorageClassOut_Deprecated) {
  auto p = parser("[[location(0)]] var<out> a : f32");
  auto f = p->function_header();
  auto decos = p->decoration_list();
  EXPECT_FALSE(decos.errored);
  EXPECT_TRUE(decos.matched);
  auto e = p->global_variable_decl(decos.value);
  ASSERT_FALSE(p->has_error()) << p->error();

  EXPECT_EQ(e->symbol(), p->builder().Symbols().Get("a"));
  EXPECT_TRUE(e->type()->Is<ast::F32>());
  EXPECT_EQ(e->declared_storage_class(), ast::StorageClass::kOutput);

  EXPECT_EQ(
      p->builder().Diagnostics().str(),
      R"(test.wgsl:1:21 warning: use of deprecated language feature: use an entry point return value instead of a variable in the `out` storage class
[[location(0)]] var<out> a : f32
                    ^^^
)");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
