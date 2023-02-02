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

TEST_F(ParserImplTest, MultiplicativeExpression_Parses_Multiply) {
    auto p = parser("a * b");
    auto lhs = p->unary_expression();
    auto e = p->expect_multiplicative_expression_post_unary_expression(lhs.value);
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

TEST_F(ParserImplTest, MultiplicativeExpression_Parses_Multiply_UnaryIndirect) {
    auto p = parser("a **b");
    auto lhs = p->unary_expression();
    auto e = p->expect_multiplicative_expression_post_unary_expression(lhs.value);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    ASSERT_TRUE(e->Is<ast::BinaryExpression>());
    auto* rel = e->As<ast::BinaryExpression>();
    EXPECT_EQ(ast::BinaryOp::kMultiply, rel->op);

    ASSERT_TRUE(rel->lhs->Is<ast::IdentifierExpression>());
    auto* ident_expr = rel->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));

    ASSERT_TRUE(rel->rhs->Is<ast::UnaryOpExpression>());
    auto* unary = rel->rhs->As<ast::UnaryOpExpression>();
    EXPECT_EQ(ast::UnaryOp::kIndirection, unary->op);

    ASSERT_TRUE(unary->expr->Is<ast::IdentifierExpression>());
    ident_expr = unary->expr->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("b"));
}

TEST_F(ParserImplTest, MultiplicativeExpression_Parses_Divide) {
    auto p = parser("a / b");
    auto lhs = p->unary_expression();
    auto e = p->expect_multiplicative_expression_post_unary_expression(lhs.value);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    ASSERT_TRUE(e->Is<ast::BinaryExpression>());
    auto* rel = e->As<ast::BinaryExpression>();
    EXPECT_EQ(ast::BinaryOp::kDivide, rel->op);

    ASSERT_TRUE(rel->lhs->Is<ast::IdentifierExpression>());
    auto* ident_expr = rel->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));

    ASSERT_TRUE(rel->rhs->Is<ast::IdentifierExpression>());
    ident_expr = rel->rhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("b"));
}

TEST_F(ParserImplTest, MultiplicativeExpression_Parses_Modulo) {
    auto p = parser("a % b");
    auto lhs = p->unary_expression();
    auto e = p->expect_multiplicative_expression_post_unary_expression(lhs.value);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    ASSERT_TRUE(e->Is<ast::BinaryExpression>());
    auto* rel = e->As<ast::BinaryExpression>();
    EXPECT_EQ(ast::BinaryOp::kModulo, rel->op);

    ASSERT_TRUE(rel->lhs->Is<ast::IdentifierExpression>());
    auto* ident_expr = rel->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));

    ASSERT_TRUE(rel->rhs->Is<ast::IdentifierExpression>());
    ident_expr = rel->rhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("b"));
}

TEST_F(ParserImplTest, MultiplicativeExpression_Parses_Grouping) {
    auto p = parser("a * b / c % d * e");
    auto lhs = p->unary_expression();
    auto e = p->expect_multiplicative_expression_post_unary_expression(lhs.value);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    ASSERT_TRUE(e->Is<ast::BinaryExpression>());
    // lhs: ((a * b) / c) % d
    // op: *
    // rhs: e
    auto* rel = e->As<ast::BinaryExpression>();
    EXPECT_EQ(ast::BinaryOp::kMultiply, rel->op);

    ASSERT_TRUE(rel->rhs->Is<ast::IdentifierExpression>());
    auto* ident_expr = rel->rhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("e"));

    ASSERT_TRUE(rel->lhs->Is<ast::BinaryExpression>());
    // lhs: (a * b) / c
    // op: %
    // rhs: d
    rel = rel->lhs->As<ast::BinaryExpression>();
    EXPECT_EQ(ast::BinaryOp::kModulo, rel->op);

    ASSERT_TRUE(rel->rhs->Is<ast::IdentifierExpression>());
    ident_expr = rel->rhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("d"));

    ASSERT_TRUE(rel->lhs->Is<ast::BinaryExpression>());
    // lhs: a * b
    // op: /
    // rhs: c
    rel = rel->lhs->As<ast::BinaryExpression>();
    EXPECT_EQ(ast::BinaryOp::kDivide, rel->op);

    ASSERT_TRUE(rel->rhs->Is<ast::IdentifierExpression>());
    ident_expr = rel->rhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("c"));

    ASSERT_TRUE(rel->lhs->Is<ast::BinaryExpression>());
    // lhs: a
    // op: *
    // rhs: b
    rel = rel->lhs->As<ast::BinaryExpression>();
    EXPECT_EQ(ast::BinaryOp::kMultiply, rel->op);

    ASSERT_TRUE(rel->lhs->Is<ast::IdentifierExpression>());
    ident_expr = rel->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));

    ASSERT_TRUE(rel->rhs->Is<ast::IdentifierExpression>());
    ident_expr = rel->rhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("b"));
}

TEST_F(ParserImplTest, MultiplicativeExpression_InvalidRHS) {
    auto p = parser("a * if (a) {}");
    auto lhs = p->unary_expression();
    auto e = p->expect_multiplicative_expression_post_unary_expression(lhs.value);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:5: unable to parse right side of * expression");
}

TEST_F(ParserImplTest, MultiplicativeExpression_NoMatch_ReturnsLHS) {
    auto p = parser("a + b");
    auto lhs = p->unary_expression();
    auto e = p->expect_multiplicative_expression_post_unary_expression(lhs.value);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    EXPECT_EQ(lhs.value, e.value);
}

}  // namespace
}  // namespace tint::reader::wgsl
