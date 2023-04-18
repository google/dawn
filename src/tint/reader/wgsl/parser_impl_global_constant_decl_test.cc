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

#include "src/tint/ast/id_attribute.h"
#include "src/tint/ast/test_helper.h"
#include "src/tint/reader/wgsl/parser_impl_test_helper.h"

namespace tint::reader::wgsl {
namespace {

TEST_F(ParserImplTest, GlobalLetDecl) {
    auto p = parser("let a : f32 = 1.");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(attrs.errored);
    EXPECT_FALSE(attrs.matched);
    auto e = p->global_constant_decl(attrs.value);
    EXPECT_TRUE(p->has_error());
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(p->error(), "1:1: module-scope 'let' is invalid, use 'const'");
}

TEST_F(ParserImplTest, GlobalConstDecl) {
    auto p = parser("const a : f32 = 1.");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(attrs.errored);
    EXPECT_FALSE(attrs.matched);
    auto e = p->global_constant_decl(attrs.value);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    auto* c = e.value->As<ast::Const>();
    ASSERT_NE(c, nullptr);

    EXPECT_EQ(c->name->symbol, p->builder().Symbols().Get("a"));
    ASSERT_NE(c->type, nullptr);
    ast::CheckIdentifier(c->type, "f32");

    EXPECT_EQ(c->source.range.begin.line, 1u);
    EXPECT_EQ(c->source.range.begin.column, 7u);
    EXPECT_EQ(c->source.range.end.line, 1u);
    EXPECT_EQ(c->source.range.end.column, 8u);

    ASSERT_NE(c->initializer, nullptr);
    EXPECT_TRUE(c->initializer->Is<ast::LiteralExpression>());
}

TEST_F(ParserImplTest, GlobalConstDecl_Inferred) {
    auto p = parser("const a = 1.");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(attrs.errored);
    EXPECT_FALSE(attrs.matched);
    auto e = p->global_constant_decl(attrs.value);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    auto* c = e.value->As<ast::Const>();
    ASSERT_NE(c, nullptr);

    EXPECT_EQ(c->name->symbol, p->builder().Symbols().Get("a"));
    EXPECT_EQ(c->type, nullptr);

    EXPECT_EQ(c->source.range.begin.line, 1u);
    EXPECT_EQ(c->source.range.begin.column, 7u);
    EXPECT_EQ(c->source.range.end.line, 1u);
    EXPECT_EQ(c->source.range.end.column, 8u);

    ASSERT_NE(c->initializer, nullptr);
    EXPECT_TRUE(c->initializer->Is<ast::LiteralExpression>());
}

TEST_F(ParserImplTest, GlobalConstDecl_InvalidExpression) {
    auto p = parser("const a : f32 = if (a) {}");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(attrs.errored);
    EXPECT_FALSE(attrs.matched);
    auto e = p->global_constant_decl(attrs.value);
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:17: missing initializer for 'const' declaration");
}

TEST_F(ParserImplTest, GlobalConstDecl_MissingExpression) {
    auto p = parser("const a : f32 =");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(attrs.errored);
    EXPECT_FALSE(attrs.matched);
    auto e = p->global_constant_decl(attrs.value);
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:16: missing initializer for 'const' declaration");
}

TEST_F(ParserImplTest, GlobalOverrideDecl_WithId) {
    auto p = parser("@id(7) override a : f32 = 1.");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(attrs.errored);
    EXPECT_TRUE(attrs.matched);

    auto e = p->global_constant_decl(attrs.value);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    auto* override = e.value->As<ast::Override>();
    ASSERT_NE(override, nullptr);

    EXPECT_EQ(override->name->symbol, p->builder().Symbols().Get("a"));
    ASSERT_NE(override->type, nullptr);
    ast::CheckIdentifier(override->type, "f32");

    EXPECT_EQ(override->source.range.begin.line, 1u);
    EXPECT_EQ(override->source.range.begin.column, 17u);
    EXPECT_EQ(override->source.range.end.line, 1u);
    EXPECT_EQ(override->source.range.end.column, 18u);

    ASSERT_NE(override->initializer, nullptr);
    EXPECT_TRUE(override->initializer->Is<ast::LiteralExpression>());

    auto* override_attr = ast::GetAttribute<ast::IdAttribute>(override->attributes);
    ASSERT_NE(override_attr, nullptr);
    EXPECT_TRUE(override_attr->expr->Is<ast::IntLiteralExpression>());
}

TEST_F(ParserImplTest, GlobalOverrideDecl_WithId_TrailingComma) {
    auto p = parser("@id(7,) override a : f32 = 1.");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(attrs.errored);
    EXPECT_TRUE(attrs.matched);

    auto e = p->global_constant_decl(attrs.value);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    auto* override = e.value->As<ast::Override>();
    ASSERT_NE(override, nullptr);

    EXPECT_EQ(override->name->symbol, p->builder().Symbols().Get("a"));
    ASSERT_NE(override->type, nullptr);
    ast::CheckIdentifier(override->type, "f32");

    EXPECT_EQ(override->source.range.begin.line, 1u);
    EXPECT_EQ(override->source.range.begin.column, 18u);
    EXPECT_EQ(override->source.range.end.line, 1u);
    EXPECT_EQ(override->source.range.end.column, 19u);

    ASSERT_NE(override->initializer, nullptr);
    EXPECT_TRUE(override->initializer->Is<ast::LiteralExpression>());

    auto* override_attr = ast::GetAttribute<ast::IdAttribute>(override->attributes);
    ASSERT_NE(override_attr, nullptr);
    EXPECT_TRUE(override_attr->expr->Is<ast::IntLiteralExpression>());
}

TEST_F(ParserImplTest, GlobalOverrideDecl_WithoutId) {
    auto p = parser("override a : f32 = 1.");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(attrs.errored);
    EXPECT_FALSE(attrs.matched);

    auto e = p->global_constant_decl(attrs.value);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    auto* override = e.value->As<ast::Override>();
    ASSERT_NE(override, nullptr);

    EXPECT_EQ(override->name->symbol, p->builder().Symbols().Get("a"));
    ASSERT_NE(override->type, nullptr);
    ast::CheckIdentifier(override->type, "f32");

    EXPECT_EQ(override->source.range.begin.line, 1u);
    EXPECT_EQ(override->source.range.begin.column, 10u);
    EXPECT_EQ(override->source.range.end.line, 1u);
    EXPECT_EQ(override->source.range.end.column, 11u);

    ASSERT_NE(override->initializer, nullptr);
    EXPECT_TRUE(override->initializer->Is<ast::LiteralExpression>());

    auto* id_attr = ast::GetAttribute<ast::IdAttribute>(override->attributes);
    ASSERT_EQ(id_attr, nullptr);
}

}  // namespace
}  // namespace tint::reader::wgsl
