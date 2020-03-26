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
#include "src/ast/call_expression.h"
#include "src/ast/const_initializer_expression.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/int_literal.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/unary_derivative_expression.h"
#include "src/ast/unary_method_expression.h"
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
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);

  ASSERT_TRUE(e->IsArrayAccessor());
  auto ary = e->AsArrayAccessor();

  ASSERT_TRUE(ary->array()->IsIdentifier());
  auto ident = ary->array()->AsIdentifier();
  ASSERT_EQ(ident->name().size(), 1);
  EXPECT_EQ(ident->name()[0], "a");

  ASSERT_TRUE(ary->idx_expr()->IsInitializer());
  ASSERT_TRUE(ary->idx_expr()->AsInitializer()->IsConstInitializer());
  auto c = ary->idx_expr()->AsInitializer()->AsConstInitializer();
  ASSERT_TRUE(c->literal()->IsInt());
  EXPECT_EQ(c->literal()->AsInt()->value(), 1);
}

TEST_F(ParserImplTest, PostfixExpression_Array_ExpressionIndex) {
  auto p = parser("a[1 + b / 4]");
  auto e = p->postfix_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);

  ASSERT_TRUE(e->IsArrayAccessor());
  auto ary = e->AsArrayAccessor();

  ASSERT_TRUE(ary->array()->IsIdentifier());
  auto ident = ary->array()->AsIdentifier();
  ASSERT_EQ(ident->name().size(), 1);
  EXPECT_EQ(ident->name()[0], "a");

  ASSERT_TRUE(ary->idx_expr()->IsRelational());
}

TEST_F(ParserImplTest, PostfixExpression_Array_MissingIndex) {
  auto p = parser("a[]");
  auto e = p->postfix_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:3: unable to parse expression inside []");
}

TEST_F(ParserImplTest, PostfixExpression_Array_MissingRightBrace) {
  auto p = parser("a[1");
  auto e = p->postfix_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:4: missing ] for array accessor");
}

TEST_F(ParserImplTest, PostfixExpression_Array_InvalidIndex) {
  auto p = parser("a[if(a() {})]");
  auto e = p->postfix_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:3: unable to parse expression inside []");
}

TEST_F(ParserImplTest, PostfixExpression_Call_Empty) {
  auto p = parser("a()");
  auto e = p->postfix_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);

  ASSERT_TRUE(e->IsCall());
  auto c = e->AsCall();

  ASSERT_TRUE(c->func()->IsIdentifier());
  auto func = c->func()->AsIdentifier();
  ASSERT_EQ(func->name().size(), 1);
  EXPECT_EQ(func->name()[0], "a");

  EXPECT_EQ(c->params().size(), 0);
}

TEST_F(ParserImplTest, PostfixExpression_Call_WithArgs) {
  auto p = parser("std::test(1, b, 2 + 3 / b)");
  auto e = p->postfix_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);

  ASSERT_TRUE(e->IsCall());
  auto c = e->AsCall();

  ASSERT_TRUE(c->func()->IsIdentifier());
  auto func = c->func()->AsIdentifier();
  ASSERT_EQ(func->name().size(), 2);
  EXPECT_EQ(func->name()[0], "std");
  EXPECT_EQ(func->name()[1], "test");

  EXPECT_EQ(c->params().size(), 3);
  EXPECT_TRUE(c->params()[0]->IsInitializer());
  EXPECT_TRUE(c->params()[1]->IsIdentifier());
  EXPECT_TRUE(c->params()[2]->IsRelational());
}

TEST_F(ParserImplTest, PostfixExpression_Call_InvalidArg) {
  auto p = parser("a(if(a) {})");
  auto e = p->postfix_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:3: unable to parse argument expression");
}

TEST_F(ParserImplTest, PostfixExpression_Call_HangingComma) {
  auto p = parser("a(b, )");
  auto e = p->postfix_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:6: unable to parse argument expression after comma");
}

TEST_F(ParserImplTest, PostfixExpression_Call_MissingRightParen) {
  auto p = parser("a(");
  auto e = p->postfix_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:3: missing ) for call expression");
}

TEST_F(ParserImplTest, PostfixExpression_MemberAccessor) {
  auto p = parser("a.b");
  auto e = p->postfix_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsMemberAccessor());

  auto m = e->AsMemberAccessor();
  ASSERT_TRUE(m->structure()->IsIdentifier());
  ASSERT_EQ(m->structure()->AsIdentifier()->name().size(), 1);
  EXPECT_EQ(m->structure()->AsIdentifier()->name()[0], "a");

  ASSERT_TRUE(m->member()->IsIdentifier());
  ASSERT_EQ(m->member()->AsIdentifier()->name().size(), 1);
  EXPECT_EQ(m->member()->AsIdentifier()->name()[0], "b");
}

TEST_F(ParserImplTest, PostfixExpression_MemberAccesssor_InvalidIdent) {
  auto p = parser("a.if");
  auto e = p->postfix_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:3: missing identifier for member accessor");
}

TEST_F(ParserImplTest, PostfixExpression_MemberAccessor_MissingIdent) {
  auto p = parser("a.");
  auto e = p->postfix_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:3: missing identifier for member accessor");
}

TEST_F(ParserImplTest, PostfixExpression_NonMatch_returnLHS) {
  auto p = parser("a b");
  auto e = p->postfix_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsIdentifier());
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
