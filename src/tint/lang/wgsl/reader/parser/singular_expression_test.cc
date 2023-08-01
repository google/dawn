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

#include "src/tint/lang/wgsl/ast/helper_test.h"
#include "src/tint/lang/wgsl/reader/parser/helper_test.h"

namespace tint::wgsl::reader {
namespace {

TEST_F(WGSLParserTest, SingularExpression_Array_ConstantIndex) {
    auto p = parser("a[1]");
    auto e = p->singular_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    ASSERT_TRUE(e->Is<ast::IndexAccessorExpression>());
    auto* idx = e->As<ast::IndexAccessorExpression>();

    ASSERT_TRUE(idx->object->Is<ast::IdentifierExpression>());
    auto* ident_expr = idx->object->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));

    ASSERT_TRUE(idx->index->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(idx->index->As<ast::IntLiteralExpression>()->value, 1);
    EXPECT_EQ(idx->index->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);
}

TEST_F(WGSLParserTest, SingularExpression_Array_ExpressionIndex) {
    auto p = parser("a[1 + b / 4]");
    auto e = p->singular_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    ASSERT_TRUE(e->Is<ast::IndexAccessorExpression>());
    auto* idx = e->As<ast::IndexAccessorExpression>();

    ASSERT_TRUE(idx->object->Is<ast::IdentifierExpression>());
    auto* ident_expr = idx->object->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));

    ASSERT_TRUE(idx->index->Is<ast::BinaryExpression>());

    EXPECT_EQ(e->source.range.begin.line, 1u);
    EXPECT_EQ(e->source.range.begin.column, 1u);
    EXPECT_EQ(e->source.range.end.line, 1u);
    EXPECT_EQ(e->source.range.end.column, 13u);
}

TEST_F(WGSLParserTest, SingularExpression_Array_MissingIndex) {
    auto p = parser("a[]");
    auto e = p->singular_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:3: unable to parse expression inside []");
}

TEST_F(WGSLParserTest, SingularExpression_Array_MissingRightBrace) {
    auto p = parser("a[1");
    auto e = p->singular_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:4: expected ']' for index accessor");
}

TEST_F(WGSLParserTest, SingularExpression_Array_InvalidIndex) {
    auto p = parser("a[if(a() {})]");
    auto e = p->singular_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:3: unable to parse expression inside []");
}

TEST_F(WGSLParserTest, SingularExpression_Call_Empty) {
    auto p = parser("a()");
    auto e = p->singular_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    ASSERT_TRUE(e->Is<ast::CallExpression>());
    auto* c = e->As<ast::CallExpression>();

    ast::CheckIdentifier(c->target, "a");

    EXPECT_EQ(c->args.Length(), 0u);
}

TEST_F(WGSLParserTest, SingularExpression_Call_WithArgs) {
    auto p = parser("test(1, b, 2 + 3 / b)");
    auto e = p->singular_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    ASSERT_TRUE(e->Is<ast::CallExpression>());
    auto* c = e->As<ast::CallExpression>();

    ast::CheckIdentifier(c->target, "test");

    EXPECT_EQ(c->args.Length(), 3u);
    EXPECT_TRUE(c->args[0]->Is<ast::IntLiteralExpression>());
    EXPECT_TRUE(c->args[1]->Is<ast::IdentifierExpression>());
    EXPECT_TRUE(c->args[2]->Is<ast::BinaryExpression>());
}

TEST_F(WGSLParserTest, SingularExpression_Call_TrailingComma) {
    auto p = parser("a(b, )");
    auto e = p->singular_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    ASSERT_NE(e.value, nullptr);

    ASSERT_TRUE(e->Is<ast::CallExpression>());
    auto* c = e->As<ast::CallExpression>();
    EXPECT_EQ(c->args.Length(), 1u);
}

TEST_F(WGSLParserTest, SingularExpression_Call_InvalidArg) {
    auto p = parser("a(if(a) {})");
    auto e = p->singular_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:3: expected expression for function call");
}

TEST_F(WGSLParserTest, SingularExpression_Call_MissingRightParen) {
    auto p = parser("a(");
    auto e = p->singular_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:3: expected expression for function call");
}

TEST_F(WGSLParserTest, SingularExpression_MemberAccessor) {
    auto p = parser("a.b");
    auto e = p->singular_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::MemberAccessorExpression>());

    auto* m = e->As<ast::MemberAccessorExpression>();
    ASSERT_TRUE(m->object->Is<ast::IdentifierExpression>());
    EXPECT_EQ(m->object->As<ast::IdentifierExpression>()->identifier->symbol,
              p->builder().Symbols().Get("a"));

    EXPECT_EQ(m->member->symbol, p->builder().Symbols().Get("b"));

    EXPECT_EQ(e->source.range.begin.line, 1u);
    EXPECT_EQ(e->source.range.begin.column, 1u);
    EXPECT_EQ(e->source.range.end.line, 1u);
    EXPECT_EQ(e->source.range.end.column, 4u);
}

TEST_F(WGSLParserTest, SingularExpression_MemberAccesssor_InvalidIdent) {
    auto p = parser("a.if");
    auto e = p->singular_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:3: expected identifier for member accessor");
}

TEST_F(WGSLParserTest, SingularExpression_MemberAccessor_MissingIdent) {
    auto p = parser("a.");
    auto e = p->singular_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:3: expected identifier for member accessor");
}

TEST_F(WGSLParserTest, SingularExpression_NonMatch_returnLHS) {
    auto p = parser("a b");
    auto e = p->singular_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::IdentifierExpression>());
}

TEST_F(WGSLParserTest, SingularExpression_Array_NestedIndexAccessor) {
    auto p = parser("a[b[c]]");
    auto e = p->singular_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    const auto* outer_accessor = e->As<ast::IndexAccessorExpression>();
    ASSERT_TRUE(outer_accessor);

    EXPECT_EQ(outer_accessor->source.range.begin.line, 1u);
    EXPECT_EQ(outer_accessor->source.range.begin.column, 1u);
    EXPECT_EQ(outer_accessor->source.range.end.line, 1u);
    EXPECT_EQ(outer_accessor->source.range.end.column, 8u);

    const auto* outer_object = outer_accessor->object->As<ast::IdentifierExpression>();
    ASSERT_TRUE(outer_object);
    EXPECT_EQ(outer_object->identifier->symbol, p->builder().Symbols().Get("a"));

    EXPECT_EQ(outer_object->source.range.begin.line, 1u);
    EXPECT_EQ(outer_object->source.range.begin.column, 1u);
    EXPECT_EQ(outer_object->source.range.end.line, 1u);
    EXPECT_EQ(outer_object->source.range.end.column, 2u);

    const auto* inner_accessor = outer_accessor->index->As<ast::IndexAccessorExpression>();
    ASSERT_TRUE(inner_accessor);

    EXPECT_EQ(inner_accessor->source.range.begin.line, 1u);
    EXPECT_EQ(inner_accessor->source.range.begin.column, 3u);
    EXPECT_EQ(inner_accessor->source.range.end.line, 1u);
    EXPECT_EQ(inner_accessor->source.range.end.column, 7u);

    const auto* inner_object = inner_accessor->object->As<ast::IdentifierExpression>();
    ASSERT_TRUE(inner_object);
    EXPECT_EQ(inner_object->identifier->symbol, p->builder().Symbols().Get("b"));

    EXPECT_EQ(inner_object->source.range.begin.line, 1u);
    EXPECT_EQ(inner_object->source.range.begin.column, 3u);
    EXPECT_EQ(inner_object->source.range.end.line, 1u);
    EXPECT_EQ(inner_object->source.range.end.column, 4u);

    const auto* index_expr = inner_accessor->index->As<ast::IdentifierExpression>();
    ASSERT_TRUE(index_expr);
    EXPECT_EQ(index_expr->identifier->symbol, p->builder().Symbols().Get("c"));

    EXPECT_EQ(index_expr->source.range.begin.line, 1u);
    EXPECT_EQ(index_expr->source.range.begin.column, 5u);
    EXPECT_EQ(index_expr->source.range.end.line, 1u);
    EXPECT_EQ(index_expr->source.range.end.column, 6u);
}

}  // namespace
}  // namespace tint::wgsl::reader
