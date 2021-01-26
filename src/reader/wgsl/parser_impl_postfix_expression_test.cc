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
#include "src/ast/array_accessor_expression.h"
#include "src/ast/binary_expression.h"
#include "src/ast/call_expression.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/unary_op_expression.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, PostfixExpression_Array_ConstantIndex) {
  auto p = parser("a[1]");
  auto e = p->postfix_expression();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e.value, nullptr);

  ASSERT_TRUE(e->Is<ast::ArrayAccessorExpression>());
  auto* ary = e->As<ast::ArrayAccessorExpression>();

  ASSERT_TRUE(ary->array()->Is<ast::IdentifierExpression>());
  auto* ident = ary->array()->As<ast::IdentifierExpression>();
  EXPECT_EQ(ident->symbol(), p->get_program().RegisterSymbol("a"));

  ASSERT_TRUE(ary->idx_expr()->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(ary->idx_expr()->Is<ast::ScalarConstructorExpression>());
  auto* c = ary->idx_expr()->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(c->literal()->Is<ast::SintLiteral>());
  EXPECT_EQ(c->literal()->As<ast::SintLiteral>()->value(), 1);
}

TEST_F(ParserImplTest, PostfixExpression_Array_ExpressionIndex) {
  auto p = parser("a[1 + b / 4]");
  auto e = p->postfix_expression();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e.value, nullptr);

  ASSERT_TRUE(e->Is<ast::ArrayAccessorExpression>());
  auto* ary = e->As<ast::ArrayAccessorExpression>();

  ASSERT_TRUE(ary->array()->Is<ast::IdentifierExpression>());
  auto* ident = ary->array()->As<ast::IdentifierExpression>();
  EXPECT_EQ(ident->symbol(), p->get_program().RegisterSymbol("a"));

  ASSERT_TRUE(ary->idx_expr()->Is<ast::BinaryExpression>());
}

TEST_F(ParserImplTest, PostfixExpression_Array_MissingIndex) {
  auto p = parser("a[]");
  auto e = p->postfix_expression();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_EQ(e.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:3: unable to parse expression inside []");
}

TEST_F(ParserImplTest, PostfixExpression_Array_MissingRightBrace) {
  auto p = parser("a[1");
  auto e = p->postfix_expression();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_EQ(e.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:4: expected ']' for array accessor");
}

TEST_F(ParserImplTest, PostfixExpression_Array_InvalidIndex) {
  auto p = parser("a[if(a() {})]");
  auto e = p->postfix_expression();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_EQ(e.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:3: unable to parse expression inside []");
}

TEST_F(ParserImplTest, PostfixExpression_Call_Empty) {
  auto p = parser("a()");
  auto e = p->postfix_expression();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e.value, nullptr);

  ASSERT_TRUE(e->Is<ast::CallExpression>());
  auto* c = e->As<ast::CallExpression>();

  ASSERT_TRUE(c->func()->Is<ast::IdentifierExpression>());
  auto* func = c->func()->As<ast::IdentifierExpression>();
  EXPECT_EQ(func->symbol(), p->get_program().RegisterSymbol("a"));

  EXPECT_EQ(c->params().size(), 0u);
}

TEST_F(ParserImplTest, PostfixExpression_Call_WithArgs) {
  auto p = parser("test(1, b, 2 + 3 / b)");
  auto e = p->postfix_expression();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e.value, nullptr);

  ASSERT_TRUE(e->Is<ast::CallExpression>());
  auto* c = e->As<ast::CallExpression>();

  ASSERT_TRUE(c->func()->Is<ast::IdentifierExpression>());
  auto* func = c->func()->As<ast::IdentifierExpression>();
  EXPECT_EQ(func->symbol(), p->get_program().RegisterSymbol("test"));

  EXPECT_EQ(c->params().size(), 3u);
  EXPECT_TRUE(c->params()[0]->Is<ast::ConstructorExpression>());
  EXPECT_TRUE(c->params()[1]->Is<ast::IdentifierExpression>());
  EXPECT_TRUE(c->params()[2]->Is<ast::BinaryExpression>());
}

TEST_F(ParserImplTest, PostfixExpression_Call_InvalidArg) {
  auto p = parser("a(if(a) {})");
  auto e = p->postfix_expression();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_EQ(e.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:3: unable to parse argument expression");
}

TEST_F(ParserImplTest, PostfixExpression_Call_HangingComma) {
  auto p = parser("a(b, )");
  auto e = p->postfix_expression();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_EQ(e.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:6: unable to parse argument expression after comma");
}

TEST_F(ParserImplTest, PostfixExpression_Call_MissingRightParen) {
  auto p = parser("a(");
  auto e = p->postfix_expression();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_EQ(e.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:3: expected ')' for call expression");
}

TEST_F(ParserImplTest, PostfixExpression_MemberAccessor) {
  auto p = parser("a.b");
  auto e = p->postfix_expression();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e.value, nullptr);
  ASSERT_TRUE(e->Is<ast::MemberAccessorExpression>());

  auto* m = e->As<ast::MemberAccessorExpression>();
  ASSERT_TRUE(m->structure()->Is<ast::IdentifierExpression>());
  EXPECT_EQ(m->structure()->As<ast::IdentifierExpression>()->symbol(),
            p->get_program().RegisterSymbol("a"));

  ASSERT_TRUE(m->member()->Is<ast::IdentifierExpression>());
  EXPECT_EQ(m->member()->As<ast::IdentifierExpression>()->symbol(),
            p->get_program().RegisterSymbol("b"));
}

TEST_F(ParserImplTest, PostfixExpression_MemberAccesssor_InvalidIdent) {
  auto p = parser("a.if");
  auto e = p->postfix_expression();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_EQ(e.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:3: expected identifier for member accessor");
}

TEST_F(ParserImplTest, PostfixExpression_MemberAccessor_MissingIdent) {
  auto p = parser("a.");
  auto e = p->postfix_expression();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_EQ(e.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:3: expected identifier for member accessor");
}

TEST_F(ParserImplTest, PostfixExpression_NonMatch_returnLHS) {
  auto p = parser("a b");
  auto e = p->postfix_expression();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e.value, nullptr);
  ASSERT_TRUE(e->Is<ast::IdentifierExpression>());
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
