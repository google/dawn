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

TEST_F(ParserImplTest, ShiftExpression_PostUnary_Parses_ShiftLeft) {
    auto p = parser("a << true");
    auto lhs = p->unary_expression();
    auto e = p->expect_shift_expression_post_unary_expression(lhs.value);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    EXPECT_EQ(e->source.range.begin.line, 1u);
    EXPECT_EQ(e->source.range.begin.column, 3u);
    EXPECT_EQ(e->source.range.end.line, 1u);
    EXPECT_EQ(e->source.range.end.column, 5u);

    ASSERT_TRUE(e->Is<ast::BinaryExpression>());
    auto* rel = e->As<ast::BinaryExpression>();
    EXPECT_EQ(ast::BinaryOp::kShiftLeft, rel->op);

    ASSERT_TRUE(rel->lhs->Is<ast::IdentifierExpression>());
    auto* ident_expr = rel->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));

    ASSERT_TRUE(rel->rhs->Is<ast::BoolLiteralExpression>());
    ASSERT_TRUE(rel->rhs->As<ast::BoolLiteralExpression>()->value);
}

TEST_F(ParserImplTest, ShiftExpression_PostUnary_Parses_ShiftRight) {
    auto p = parser("a >> true");
    auto lhs = p->unary_expression();
    auto e = p->expect_shift_expression_post_unary_expression(lhs.value);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    EXPECT_EQ(e->source.range.begin.line, 1u);
    EXPECT_EQ(e->source.range.begin.column, 3u);
    EXPECT_EQ(e->source.range.end.line, 1u);
    EXPECT_EQ(e->source.range.end.column, 5u);

    ASSERT_TRUE(e->Is<ast::BinaryExpression>());
    auto* rel = e->As<ast::BinaryExpression>();
    EXPECT_EQ(ast::BinaryOp::kShiftRight, rel->op);

    ASSERT_TRUE(rel->lhs->Is<ast::IdentifierExpression>());
    auto* ident_expr = rel->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));

    ASSERT_TRUE(rel->rhs->Is<ast::BoolLiteralExpression>());
    ASSERT_TRUE(rel->rhs->As<ast::BoolLiteralExpression>()->value);
}

TEST_F(ParserImplTest, ShiftExpression_PostUnary_Parses_Additive) {
    auto p = parser("a + b");
    auto lhs = p->unary_expression();
    auto e = p->expect_shift_expression_post_unary_expression(lhs.value);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    ASSERT_TRUE(e->Is<ast::BinaryExpression>());
    auto* rel = e->As<ast::BinaryExpression>();
    EXPECT_EQ(ast::BinaryOp::kAdd, rel->op);

    ASSERT_TRUE(rel->lhs->Is<ast::IdentifierExpression>());
    auto* ident_expr = rel->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));

    ASSERT_TRUE(rel->rhs->Is<ast::IdentifierExpression>());
    ident_expr = rel->rhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("b"));
}

TEST_F(ParserImplTest, ShiftExpression_PostUnary_Parses_Multiplicative) {
    auto p = parser("a * b");
    auto lhs = p->unary_expression();
    auto e = p->expect_shift_expression_post_unary_expression(lhs.value);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    ASSERT_TRUE(e->Is<ast::BinaryExpression>());
    auto* rel = e->As<ast::BinaryExpression>();
    EXPECT_EQ(ast::BinaryOp::kMultiply, rel->op);

    ASSERT_TRUE(rel->lhs->Is<ast::IdentifierExpression>());
    auto* ident_expr = rel->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));

    ASSERT_TRUE(rel->rhs->Is<ast::IdentifierExpression>());
    ident_expr = rel->rhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("b"));
}

TEST_F(ParserImplTest, ShiftExpression_PostUnary_InvalidSpaceLeft) {
    auto p = parser("a < < true");
    auto lhs = p->unary_expression();
    auto e = p->expect_shift_expression_post_unary_expression(lhs.value);
    EXPECT_FALSE(e.errored);
    ASSERT_NE(e.value, nullptr);
    EXPECT_FALSE(e.value->Is<ast::BinaryExpression>());
}

TEST_F(ParserImplTest, ShiftExpression_PostUnary_InvalidSpaceRight) {
    auto p = parser("a > > true");
    auto lhs = p->unary_expression();
    auto e = p->expect_shift_expression_post_unary_expression(lhs.value);
    EXPECT_FALSE(e.errored);
    ASSERT_NE(e.value, nullptr);
    EXPECT_FALSE(e.value->Is<ast::BinaryExpression>());
}

TEST_F(ParserImplTest, ShiftExpression_PostUnary_InvalidRHS) {
    auto p = parser("a << if (a) {}");
    auto lhs = p->unary_expression();
    auto e = p->expect_shift_expression_post_unary_expression(lhs.value);
    EXPECT_TRUE(e.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:6: unable to parse right side of << expression");
}

TEST_F(ParserImplTest, ShiftExpression_PostUnary_NoOr_ReturnsLHS) {
    auto p = parser("a true");
    auto lhs = p->unary_expression();
    auto e = p->expect_shift_expression_post_unary_expression(lhs.value);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_EQ(lhs.value, e.value);
}

TEST_F(ParserImplTest, ShiftExpression_Parses) {
    auto p = parser("a << true");
    auto e = p->shift_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    ASSERT_TRUE(e->Is<ast::BinaryExpression>());
    auto* rel = e->As<ast::BinaryExpression>();
    EXPECT_EQ(ast::BinaryOp::kShiftLeft, rel->op);

    ASSERT_TRUE(rel->lhs->Is<ast::IdentifierExpression>());
    auto* ident_expr = rel->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));

    ASSERT_TRUE(rel->rhs->Is<ast::BoolLiteralExpression>());
    ASSERT_TRUE(rel->rhs->As<ast::BoolLiteralExpression>()->value);
}

TEST_F(ParserImplTest, ShiftExpression_Invalid_Unary) {
    auto p = parser("if >> true");
    auto e = p->shift_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_EQ(e.value, nullptr);
}

TEST_F(ParserImplTest, ShiftExpression_Inavlid_ShiftExpressionPostUnary) {
    auto p = parser("a * if (a) {}");
    auto e = p->shift_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_TRUE(p->has_error());
    ASSERT_EQ(e.value, nullptr);

    EXPECT_EQ(p->error(), "1:5: unable to parse right side of * expression");
}

}  // namespace
}  // namespace tint::reader::wgsl
