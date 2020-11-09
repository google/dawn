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
#include "src/ast/identifier_expression.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/unary_op_expression.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, UnaryExpression_Postix) {
  auto* p = parser("a[2]");
  auto e = p->unary_expression();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e.value, nullptr);

  ASSERT_TRUE(e.value->IsArrayAccessor());
  auto* ary = e.value->AsArrayAccessor();
  ASSERT_TRUE(ary->array()->IsIdentifier());
  auto* ident = ary->array()->AsIdentifier();
  EXPECT_EQ(ident->name(), "a");

  ASSERT_TRUE(ary->idx_expr()->IsConstructor());
  ASSERT_TRUE(ary->idx_expr()->AsConstructor()->IsScalarConstructor());
  auto* init = ary->idx_expr()->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(init->literal()->IsSint());
  ASSERT_EQ(init->literal()->AsSint()->value(), 2);
}

TEST_F(ParserImplTest, UnaryExpression_Minus) {
  auto* p = parser("- 1");
  auto e = p->unary_expression();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e.value, nullptr);
  ASSERT_TRUE(e.value->IsUnaryOp());

  auto* u = e.value->AsUnaryOp();
  ASSERT_EQ(u->op(), ast::UnaryOp::kNegation);

  ASSERT_TRUE(u->expr()->IsConstructor());
  ASSERT_TRUE(u->expr()->AsConstructor()->IsScalarConstructor());

  auto* init = u->expr()->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(init->literal()->IsSint());
  EXPECT_EQ(init->literal()->AsSint()->value(), 1);
}

TEST_F(ParserImplTest, UnaryExpression_Minus_InvalidRHS) {
  auto* p = parser("-if(a) {}");
  auto e = p->unary_expression();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_EQ(e.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:2: unable to parse right side of - expression");
}

TEST_F(ParserImplTest, UnaryExpression_Bang) {
  auto* p = parser("!1");
  auto e = p->unary_expression();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e.value, nullptr);
  ASSERT_TRUE(e.value->IsUnaryOp());

  auto* u = e.value->AsUnaryOp();
  ASSERT_EQ(u->op(), ast::UnaryOp::kNot);

  ASSERT_TRUE(u->expr()->IsConstructor());
  ASSERT_TRUE(u->expr()->AsConstructor()->IsScalarConstructor());

  auto* init = u->expr()->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(init->literal()->IsSint());
  EXPECT_EQ(init->literal()->AsSint()->value(), 1);
}

TEST_F(ParserImplTest, UnaryExpression_Bang_InvalidRHS) {
  auto* p = parser("!if (a) {}");
  auto e = p->unary_expression();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_EQ(e.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:2: unable to parse right side of ! expression");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
