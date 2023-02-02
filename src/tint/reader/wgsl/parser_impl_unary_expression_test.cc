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

#include "src/tint/ast/unary_op_expression.h"
#include "src/tint/reader/wgsl/parser_impl_test_helper.h"

namespace tint::reader::wgsl {
namespace {

TEST_F(ParserImplTest, UnaryExpression_Postix) {
    auto p = parser("a[2]");
    auto e = p->unary_expression();
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
    ASSERT_EQ(idx->index->As<ast::IntLiteralExpression>()->value, 2);
    ASSERT_EQ(idx->index->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);
}

TEST_F(ParserImplTest, UnaryExpression_Minus) {
    auto p = parser("- 1");
    auto e = p->unary_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::UnaryOpExpression>());

    auto* u = e->As<ast::UnaryOpExpression>();
    ASSERT_EQ(u->op, ast::UnaryOp::kNegation);

    ASSERT_TRUE(u->expr->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(u->expr->As<ast::IntLiteralExpression>()->value, 1);
    ASSERT_EQ(u->expr->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);
}

TEST_F(ParserImplTest, UnaryExpression_AddressOf) {
    auto p = parser("&x");
    auto e = p->unary_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::UnaryOpExpression>());

    auto* u = e->As<ast::UnaryOpExpression>();
    EXPECT_EQ(u->op, ast::UnaryOp::kAddressOf);
    EXPECT_TRUE(u->expr->Is<ast::IdentifierExpression>());
}

TEST_F(ParserImplTest, UnaryExpression_Dereference) {
    auto p = parser("*x");
    auto e = p->unary_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::UnaryOpExpression>());

    auto* u = e->As<ast::UnaryOpExpression>();
    EXPECT_EQ(u->op, ast::UnaryOp::kIndirection);
    EXPECT_TRUE(u->expr->Is<ast::IdentifierExpression>());
}

TEST_F(ParserImplTest, UnaryExpression_AddressOf_Precedence) {
    auto p = parser("&x.y");
    auto e = p->unary_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::UnaryOpExpression>());

    auto* u = e->As<ast::UnaryOpExpression>();
    EXPECT_EQ(u->op, ast::UnaryOp::kAddressOf);
    EXPECT_TRUE(u->expr->Is<ast::MemberAccessorExpression>());
}

TEST_F(ParserImplTest, UnaryExpression_Dereference_Precedence) {
    auto p = parser("*x.y");
    auto e = p->unary_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::UnaryOpExpression>());

    auto* u = e->As<ast::UnaryOpExpression>();
    EXPECT_EQ(u->op, ast::UnaryOp::kIndirection);
    EXPECT_TRUE(u->expr->Is<ast::MemberAccessorExpression>());
}

TEST_F(ParserImplTest, UnaryExpression_Minus_InvalidRHS) {
    auto p = parser("-if(a) {}");
    auto e = p->unary_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:2: unable to parse right side of - expression");
}

TEST_F(ParserImplTest, UnaryExpression_Bang) {
    auto p = parser("!1");
    auto e = p->unary_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::UnaryOpExpression>());

    auto* u = e->As<ast::UnaryOpExpression>();
    ASSERT_EQ(u->op, ast::UnaryOp::kNot);

    ASSERT_TRUE(u->expr->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(u->expr->As<ast::IntLiteralExpression>()->value, 1);
    ASSERT_EQ(u->expr->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);
}

TEST_F(ParserImplTest, UnaryExpression_Bang_InvalidRHS) {
    auto p = parser("!if (a) {}");
    auto e = p->unary_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:2: unable to parse right side of ! expression");
}

TEST_F(ParserImplTest, UnaryExpression_Tilde) {
    auto p = parser("~1");
    auto e = p->unary_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::UnaryOpExpression>());

    auto* u = e->As<ast::UnaryOpExpression>();
    ASSERT_EQ(u->op, ast::UnaryOp::kComplement);

    ASSERT_TRUE(u->expr->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(u->expr->As<ast::IntLiteralExpression>()->value, 1);
    ASSERT_EQ(u->expr->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);
}

TEST_F(ParserImplTest, UnaryExpression_PrefixPlusPlus) {
    auto p = parser("++a");
    auto e = p->unary_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(),
              "1:1: prefix increment and decrement operators are reserved for a "
              "future WGSL version");
}

TEST_F(ParserImplTest, UnaryExpression_PrefixMinusMinus) {
    auto p = parser("--a");
    auto e = p->unary_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(),
              "1:1: prefix increment and decrement operators are reserved for a "
              "future WGSL version");
}

}  // namespace
}  // namespace tint::reader::wgsl
