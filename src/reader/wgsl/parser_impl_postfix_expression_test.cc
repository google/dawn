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
  auto* p = parser("a[1]");
  auto e = p->postfix_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);

  ASSERT_TRUE(e->IsArrayAccessor());
  auto* ary = e->AsArrayAccessor();

  ASSERT_TRUE(ary->array()->IsIdentifier());
  auto* ident = ary->array()->AsIdentifier();
  EXPECT_EQ(ident->name(), "a");

  ASSERT_TRUE(ary->idx_expr()->IsConstructor());
  ASSERT_TRUE(ary->idx_expr()->AsConstructor()->IsScalarConstructor());
  auto* c = ary->idx_expr()->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(c->literal()->IsSint());
  EXPECT_EQ(c->literal()->AsSint()->value(), 1);
}

TEST_F(ParserImplTest, PostfixExpression_Array_ExpressionIndex) {
  auto* p = parser("a[1 + b / 4]");
  auto e = p->postfix_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);

  ASSERT_TRUE(e->IsArrayAccessor());
  auto* ary = e->AsArrayAccessor();

  ASSERT_TRUE(ary->array()->IsIdentifier());
  auto* ident = ary->array()->AsIdentifier();
  EXPECT_EQ(ident->name(), "a");

  ASSERT_TRUE(ary->idx_expr()->IsBinary());
}

TEST_F(ParserImplTest, PostfixExpression_Array_MissingIndex) {
  auto* p = parser("a[]");
  auto e = p->postfix_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:3: unable to parse expression inside []");
}

TEST_F(ParserImplTest, PostfixExpression_Array_MissingRightBrace) {
  auto* p = parser("a[1");
  auto e = p->postfix_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:4: missing ] for array accessor");
}

TEST_F(ParserImplTest, PostfixExpression_Array_InvalidIndex) {
  auto* p = parser("a[if(a() {})]");
  auto e = p->postfix_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:3: unable to parse expression inside []");
}

TEST_F(ParserImplTest, PostfixExpression_Call_Empty) {
  auto* p = parser("a()");
  auto e = p->postfix_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);

  ASSERT_TRUE(e->IsCall());
  auto* c = e->AsCall();

  ASSERT_TRUE(c->func()->IsIdentifier());
  auto* func = c->func()->AsIdentifier();
  EXPECT_EQ(func->name(), "a");

  EXPECT_EQ(c->params().size(), 0u);
}

TEST_F(ParserImplTest, PostfixExpression_Call_WithArgs) {
  auto* p = parser("std::test(1, b, 2 + 3 / b)");
  auto e = p->postfix_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);

  ASSERT_TRUE(e->IsCall());
  auto* c = e->AsCall();

  ASSERT_TRUE(c->func()->IsIdentifier());
  auto* func = c->func()->AsIdentifier();
  EXPECT_EQ(func->path(), "std");
  EXPECT_EQ(func->name(), "test");

  EXPECT_EQ(c->params().size(), 3u);
  EXPECT_TRUE(c->params()[0]->IsConstructor());
  EXPECT_TRUE(c->params()[1]->IsIdentifier());
  EXPECT_TRUE(c->params()[2]->IsBinary());
}

TEST_F(ParserImplTest, PostfixExpression_Call_InvalidArg) {
  auto* p = parser("a(if(a) {})");
  auto e = p->postfix_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:3: unable to parse argument expression");
}

TEST_F(ParserImplTest, PostfixExpression_Call_HangingComma) {
  auto* p = parser("a(b, )");
  auto e = p->postfix_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:6: unable to parse argument expression after comma");
}

TEST_F(ParserImplTest, PostfixExpression_Call_MissingRightParen) {
  auto* p = parser("a(");
  auto e = p->postfix_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:3: missing ) for call expression");
}

TEST_F(ParserImplTest, PostfixExpression_MemberAccessor) {
  auto* p = parser("a.b");
  auto e = p->postfix_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsMemberAccessor());

  auto* m = e->AsMemberAccessor();
  ASSERT_TRUE(m->structure()->IsIdentifier());
  EXPECT_EQ(m->structure()->AsIdentifier()->name(), "a");

  ASSERT_TRUE(m->member()->IsIdentifier());
  EXPECT_EQ(m->member()->AsIdentifier()->name(), "b");
}

TEST_F(ParserImplTest, PostfixExpression_MemberAccesssor_InvalidIdent) {
  auto* p = parser("a.if");
  auto e = p->postfix_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:3: missing identifier for member accessor");
}

TEST_F(ParserImplTest, PostfixExpression_MemberAccessor_MissingIdent) {
  auto* p = parser("a.");
  auto e = p->postfix_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:3: missing identifier for member accessor");
}

TEST_F(ParserImplTest, PostfixExpression_NonMatch_returnLHS) {
  auto* p = parser("a b");
  auto e = p->postfix_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsIdentifier());
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
