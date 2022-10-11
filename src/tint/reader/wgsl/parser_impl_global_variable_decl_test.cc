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

#include "src/tint/reader/wgsl/parser_impl_test_helper.h"

namespace tint::reader::wgsl {
namespace {

TEST_F(ParserImplTest, GlobalVariableDecl_WithoutConstructor) {
    auto p = parser("var<private> a : f32");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(attrs.errored);
    EXPECT_FALSE(attrs.matched);
    auto e = p->global_variable_decl(attrs.value);
    ASSERT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    auto* var = e.value->As<ast::Var>();
    ASSERT_NE(var, nullptr);

    EXPECT_EQ(var->symbol, p->builder().Symbols().Get("a"));
    EXPECT_TRUE(var->type->Is<ast::F32>());
    EXPECT_EQ(var->declared_address_space, ast::AddressSpace::kPrivate);

    EXPECT_EQ(var->source.range.begin.line, 1u);
    EXPECT_EQ(var->source.range.begin.column, 14u);
    EXPECT_EQ(var->source.range.end.line, 1u);
    EXPECT_EQ(var->source.range.end.column, 15u);

    ASSERT_EQ(var->constructor, nullptr);
}

TEST_F(ParserImplTest, GlobalVariableDecl_WithConstructor) {
    auto p = parser("var<private> a : f32 = 1.");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(attrs.errored);
    EXPECT_FALSE(attrs.matched);
    auto e = p->global_variable_decl(attrs.value);
    ASSERT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    auto* var = e.value->As<ast::Var>();
    ASSERT_NE(var, nullptr);

    EXPECT_EQ(var->symbol, p->builder().Symbols().Get("a"));
    EXPECT_TRUE(var->type->Is<ast::F32>());
    EXPECT_EQ(var->declared_address_space, ast::AddressSpace::kPrivate);

    EXPECT_EQ(var->source.range.begin.line, 1u);
    EXPECT_EQ(var->source.range.begin.column, 14u);
    EXPECT_EQ(var->source.range.end.line, 1u);
    EXPECT_EQ(var->source.range.end.column, 15u);

    ASSERT_NE(var->constructor, nullptr);
    ASSERT_TRUE(var->constructor->Is<ast::FloatLiteralExpression>());
}

TEST_F(ParserImplTest, GlobalVariableDecl_WithAttribute) {
    auto p = parser("@binding(2) @group(1) var<uniform> a : f32");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(attrs.errored);
    EXPECT_TRUE(attrs.matched);
    auto e = p->global_variable_decl(attrs.value);
    ASSERT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    auto* var = e.value->As<ast::Var>();
    ASSERT_NE(var, nullptr);

    EXPECT_EQ(var->symbol, p->builder().Symbols().Get("a"));
    ASSERT_NE(var->type, nullptr);
    EXPECT_TRUE(var->type->Is<ast::F32>());
    EXPECT_EQ(var->declared_address_space, ast::AddressSpace::kUniform);

    EXPECT_EQ(var->source.range.begin.line, 1u);
    EXPECT_EQ(var->source.range.begin.column, 36u);
    EXPECT_EQ(var->source.range.end.line, 1u);
    EXPECT_EQ(var->source.range.end.column, 37u);

    ASSERT_EQ(var->constructor, nullptr);

    auto& attributes = var->attributes;
    ASSERT_EQ(attributes.Length(), 2u);
    ASSERT_TRUE(attributes[0]->Is<ast::BindingAttribute>());
    ASSERT_TRUE(attributes[1]->Is<ast::GroupAttribute>());
}

TEST_F(ParserImplTest, GlobalVariableDecl_WithAttribute_MulitpleGroups) {
    auto p = parser("@binding(2) @group(1) var<uniform> a : f32");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(attrs.errored);
    EXPECT_TRUE(attrs.matched);

    auto e = p->global_variable_decl(attrs.value);
    ASSERT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    auto* var = e.value->As<ast::Var>();
    ASSERT_NE(var, nullptr);

    EXPECT_EQ(var->symbol, p->builder().Symbols().Get("a"));
    ASSERT_NE(var->type, nullptr);
    EXPECT_TRUE(var->type->Is<ast::F32>());
    EXPECT_EQ(var->declared_address_space, ast::AddressSpace::kUniform);

    EXPECT_EQ(var->source.range.begin.line, 1u);
    EXPECT_EQ(var->source.range.begin.column, 36u);
    EXPECT_EQ(var->source.range.end.line, 1u);
    EXPECT_EQ(var->source.range.end.column, 37u);

    ASSERT_EQ(var->constructor, nullptr);

    auto& attributes = var->attributes;
    ASSERT_EQ(attributes.Length(), 2u);
    ASSERT_TRUE(attributes[0]->Is<ast::BindingAttribute>());
    ASSERT_TRUE(attributes[1]->Is<ast::GroupAttribute>());
}

TEST_F(ParserImplTest, GlobalVariableDecl_InvalidAttribute) {
    auto p = parser("@binding() var<uniform> a : f32");
    auto attrs = p->attribute_list();
    EXPECT_TRUE(attrs.errored);
    EXPECT_FALSE(attrs.matched);

    auto e = p->global_variable_decl(attrs.value);
    EXPECT_FALSE(e.errored);
    EXPECT_TRUE(e.matched);
    EXPECT_NE(e.value, nullptr);

    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:10: expected signed integer literal for binding attribute");
}

TEST_F(ParserImplTest, GlobalVariableDecl_InvalidConstExpr) {
    auto p = parser("var<private> a : f32 = if (a) {}");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(attrs.errored);
    EXPECT_FALSE(attrs.matched);
    auto e = p->global_variable_decl(attrs.value);
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:24: missing initializer for 'var' declaration");
}

TEST_F(ParserImplTest, GlobalVariableDecl_InvalidVariableDecl) {
    auto p = parser("var<invalid> a : f32;");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(attrs.errored);
    EXPECT_FALSE(attrs.matched);
    auto e = p->global_variable_decl(attrs.value);
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), R"(1:5: expected address space for variable declaration
Possible values: 'function', 'private', 'push_constant', 'storage', 'uniform', 'workgroup')");
}

}  // namespace
}  // namespace tint::reader::wgsl
