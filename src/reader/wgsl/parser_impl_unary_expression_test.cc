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
#include "src/ast/const_initializer_expression.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/int_literal.h"
#include "src/ast/unary_derivative_expression.h"
#include "src/ast/unary_method_expression.h"
#include "src/ast/unary_op_expression.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, UnaryExpression_Postix) {
  auto p = parser("a[2]");
  auto e = p->unary_expression();
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
  auto init = ary->idx_expr()->AsInitializer()->AsConstInitializer();
  ASSERT_TRUE(init->literal()->IsInt());
  ASSERT_EQ(init->literal()->AsInt()->value(), 2);
}

TEST_F(ParserImplTest, UnaryExpression_Minus) {
  auto p = parser("- 1");
  auto e = p->unary_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsUnaryOp());

  auto u = e->AsUnaryOp();
  ASSERT_EQ(u->op(), ast::UnaryOp::kNegation);

  ASSERT_TRUE(u->expr()->IsInitializer());
  ASSERT_TRUE(u->expr()->AsInitializer()->IsConstInitializer());

  auto init = u->expr()->AsInitializer()->AsConstInitializer();
  ASSERT_TRUE(init->literal()->IsInt());
  EXPECT_EQ(init->literal()->AsInt()->value(), 1);
}

TEST_F(ParserImplTest, UnaryExpression_Minus_InvalidRHS) {
  auto p = parser("-if(a) {}");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:2: unable to parse right side of - expression");
}

TEST_F(ParserImplTest, UnaryExpression_Bang) {
  auto p = parser("!1");
  auto e = p->unary_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsUnaryOp());

  auto u = e->AsUnaryOp();
  ASSERT_EQ(u->op(), ast::UnaryOp::kNot);

  ASSERT_TRUE(u->expr()->IsInitializer());
  ASSERT_TRUE(u->expr()->AsInitializer()->IsConstInitializer());

  auto init = u->expr()->AsInitializer()->AsConstInitializer();
  ASSERT_TRUE(init->literal()->IsInt());
  EXPECT_EQ(init->literal()->AsInt()->value(), 1);
}

TEST_F(ParserImplTest, UnaryExpression_Bang_InvalidRHS) {
  auto p = parser("!if (a) {}");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:2: unable to parse right side of ! expression");
}

TEST_F(ParserImplTest, UnaryExpression_Any) {
  auto p = parser("any(a)");
  auto e = p->unary_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsUnaryMethod());

  auto u = e->AsUnaryMethod();
  ASSERT_EQ(u->op(), ast::UnaryMethod::kAny);
  ASSERT_EQ(u->params().size(), 1);
  ASSERT_TRUE(u->params()[0]->IsIdentifier());
  auto ident = u->params()[0]->AsIdentifier();
  ASSERT_EQ(ident->name().size(), 1);
  EXPECT_EQ(ident->name()[0], "a");
}

TEST_F(ParserImplTest, UnaryExpression_Any_MissingParenLeft) {
  auto p = parser("any a)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:5: missing ( for method call");
}

TEST_F(ParserImplTest, UnaryExpression_Any_MissingParenRight) {
  auto p = parser("any(a");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:6: missing ) for method call");
}

TEST_F(ParserImplTest, UnaryExpression_Any_MissingIdentifier) {
  auto p = parser("any()");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:5: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_Any_InvalidIdentifier) {
  auto p = parser("any(123)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:5: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_All) {
  auto p = parser("all(a)");
  auto e = p->unary_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsUnaryMethod());

  auto u = e->AsUnaryMethod();
  ASSERT_EQ(u->op(), ast::UnaryMethod::kAll);
  ASSERT_EQ(u->params().size(), 1);
  ASSERT_TRUE(u->params()[0]->IsIdentifier());
  auto ident = u->params()[0]->AsIdentifier();
  ASSERT_EQ(ident->name().size(), 1);
  EXPECT_EQ(ident->name()[0], "a");
}

TEST_F(ParserImplTest, UnaryExpression_All_MissingParenLeft) {
  auto p = parser("all a)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:5: missing ( for method call");
}

TEST_F(ParserImplTest, UnaryExpression_All_MissingParenRight) {
  auto p = parser("all(a");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:6: missing ) for method call");
}

TEST_F(ParserImplTest, UnaryExpression_All_MissingIdentifier) {
  auto p = parser("all()");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:5: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_All_InvalidIdentifier) {
  auto p = parser("all(123)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:5: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_IsNan) {
  auto p = parser("is_nan(a)");
  auto e = p->unary_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsUnaryMethod());

  auto u = e->AsUnaryMethod();
  ASSERT_EQ(u->op(), ast::UnaryMethod::kIsNan);
  ASSERT_EQ(u->params().size(), 1);
  ASSERT_TRUE(u->params()[0]->IsIdentifier());
  auto ident = u->params()[0]->AsIdentifier();
  ASSERT_EQ(ident->name().size(), 1);
  EXPECT_EQ(ident->name()[0], "a");
}

TEST_F(ParserImplTest, UnaryExpression_IsNan_MissingParenLeft) {
  auto p = parser("is_nan a)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:8: missing ( for method call");
}

TEST_F(ParserImplTest, UnaryExpression_IsNan_MissingParenRight) {
  auto p = parser("is_nan(a");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:9: missing ) for method call");
}

TEST_F(ParserImplTest, UnaryExpression_IsNan_MissingIdentifier) {
  auto p = parser("is_nan()");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:8: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_IsNan_InvalidIdentifier) {
  auto p = parser("is_nan(123)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:8: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_IsInf) {
  auto p = parser("is_inf(a)");
  auto e = p->unary_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsUnaryMethod());

  auto u = e->AsUnaryMethod();
  ASSERT_EQ(u->op(), ast::UnaryMethod::kIsInf);
  ASSERT_EQ(u->params().size(), 1);
  ASSERT_TRUE(u->params()[0]->IsIdentifier());
  auto ident = u->params()[0]->AsIdentifier();
  ASSERT_EQ(ident->name().size(), 1);
  EXPECT_EQ(ident->name()[0], "a");
}

TEST_F(ParserImplTest, UnaryExpression_IsInf_MissingParenLeft) {
  auto p = parser("is_inf a)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:8: missing ( for method call");
}

TEST_F(ParserImplTest, UnaryExpression_IsInf_MissingParenRight) {
  auto p = parser("is_inf(a");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:9: missing ) for method call");
}

TEST_F(ParserImplTest, UnaryExpression_IsInf_MissingIdentifier) {
  auto p = parser("is_inf()");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:8: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_IsInf_InvalidIdentifier) {
  auto p = parser("is_inf(123)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:8: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_IsFinite) {
  auto p = parser("is_finite(a)");
  auto e = p->unary_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsUnaryMethod());

  auto u = e->AsUnaryMethod();
  ASSERT_EQ(u->op(), ast::UnaryMethod::kIsFinite);
  ASSERT_EQ(u->params().size(), 1);
  ASSERT_TRUE(u->params()[0]->IsIdentifier());
  auto ident = u->params()[0]->AsIdentifier();
  ASSERT_EQ(ident->name().size(), 1);
  EXPECT_EQ(ident->name()[0], "a");
}

TEST_F(ParserImplTest, UnaryExpression_IsFinite_MissingParenLeft) {
  auto p = parser("is_finite a)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:11: missing ( for method call");
}

TEST_F(ParserImplTest, UnaryExpression_IsFinite_MissingParenRight) {
  auto p = parser("is_finite(a");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:12: missing ) for method call");
}

TEST_F(ParserImplTest, UnaryExpression_IsFinite_MissingIdentifier) {
  auto p = parser("is_finite()");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:11: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_IsFinite_InvalidIdentifier) {
  auto p = parser("is_finite(123)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:11: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_IsNormal) {
  auto p = parser("is_normal(a)");
  auto e = p->unary_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsUnaryMethod());

  auto u = e->AsUnaryMethod();
  ASSERT_EQ(u->op(), ast::UnaryMethod::kIsNormal);
  ASSERT_EQ(u->params().size(), 1);
  ASSERT_TRUE(u->params()[0]->IsIdentifier());
  auto ident = u->params()[0]->AsIdentifier();
  ASSERT_EQ(ident->name().size(), 1);
  EXPECT_EQ(ident->name()[0], "a");
}

TEST_F(ParserImplTest, UnaryExpression_IsNormal_MissingParenLeft) {
  auto p = parser("is_normal a)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:11: missing ( for method call");
}

TEST_F(ParserImplTest, UnaryExpression_IsNormal_MissingParenRight) {
  auto p = parser("is_normal(a");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:12: missing ) for method call");
}

TEST_F(ParserImplTest, UnaryExpression_IsNormal_MissingIdentifier) {
  auto p = parser("is_normal()");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:11: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_IsNormal_InvalidIdentifier) {
  auto p = parser("is_normal(123)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:11: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_Dot) {
  auto p = parser("dot(a, b)");
  auto e = p->unary_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsUnaryMethod());

  auto u = e->AsUnaryMethod();
  ASSERT_EQ(u->op(), ast::UnaryMethod::kDot);
  ASSERT_EQ(u->params().size(), 2);
  ASSERT_TRUE(u->params()[0]->IsIdentifier());
  auto ident = u->params()[0]->AsIdentifier();
  ASSERT_EQ(ident->name().size(), 1);
  EXPECT_EQ(ident->name()[0], "a");

  ASSERT_TRUE(u->params()[1]->IsIdentifier());
  ident = u->params()[1]->AsIdentifier();
  ASSERT_EQ(ident->name().size(), 1);
  EXPECT_EQ(ident->name()[0], "b");
}

TEST_F(ParserImplTest, UnaryExpression_Dot_MissingParenLeft) {
  auto p = parser("dot a, b)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:5: missing ( for method call");
}

TEST_F(ParserImplTest, UnaryExpression_Dot_MissingParenRight) {
  auto p = parser("dot(a, b");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:9: missing ) for method call");
}

TEST_F(ParserImplTest, UnaryExpression_Dot_MissingFirstIdentifier) {
  auto p = parser("dot(, a)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:5: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_Dot_MissingSecondIdentifier) {
  auto p = parser("dot(a, )");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:8: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_Dot_MissingComma) {
  auto p = parser("dot(a b)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:7: missing , for method call");
}

TEST_F(ParserImplTest, UnaryExpression_Dot_InvalidFirstIdentifier) {
  auto p = parser("dot(123, b)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:5: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_Dot_InvalidSecondIdentifier) {
  auto p = parser("dot(a, 123)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:8: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_OuterProduct) {
  auto p = parser("outer_product(a, b)");
  auto e = p->unary_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsUnaryMethod());

  auto u = e->AsUnaryMethod();
  ASSERT_EQ(u->op(), ast::UnaryMethod::kOuterProduct);
  ASSERT_EQ(u->params().size(), 2);
  ASSERT_TRUE(u->params()[0]->IsIdentifier());
  auto ident = u->params()[0]->AsIdentifier();
  ASSERT_EQ(ident->name().size(), 1);
  EXPECT_EQ(ident->name()[0], "a");

  ASSERT_TRUE(u->params()[1]->IsIdentifier());
  ident = u->params()[1]->AsIdentifier();
  ASSERT_EQ(ident->name().size(), 1);
  EXPECT_EQ(ident->name()[0], "b");
}

TEST_F(ParserImplTest, UnaryExpression_OuterProduct_MissingParenLeft) {
  auto p = parser("outer_product a, b)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:15: missing ( for method call");
}

TEST_F(ParserImplTest, UnaryExpression_OuterProduct_MissingParenRight) {
  auto p = parser("outer_product(a, b");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:19: missing ) for method call");
}

TEST_F(ParserImplTest, UnaryExpression_OuterProduct_MissingFirstIdentifier) {
  auto p = parser("outer_product(, b)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:15: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_OuterProduct_MissingSecondIdentifier) {
  auto p = parser("outer_product(a, )");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:18: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_OuterProduct_MissingComma) {
  auto p = parser("outer_product(a b)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:17: missing , for method call");
}

TEST_F(ParserImplTest, UnaryExpression_OuterProduct_InvalidFirstIdentifier) {
  auto p = parser("outer_product(123, b)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:15: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_OuterProduct_InvalidSecondIdentifier) {
  auto p = parser("outer_product(a, 123)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:18: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_Dpdx_NoModifier) {
  auto p = parser("dpdx(a)");
  auto e = p->unary_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsUnaryDerivative());

  auto deriv = e->AsUnaryDerivative();
  EXPECT_EQ(deriv->op(), ast::UnaryDerivative::kDpdx);
  EXPECT_EQ(deriv->modifier(), ast::DerivativeModifier::kNone);

  ASSERT_NE(deriv->param(), nullptr);
  ASSERT_TRUE(deriv->param()->IsIdentifier());
  auto ident = deriv->param()->AsIdentifier();
  ASSERT_EQ(ident->name().size(), 1);
  EXPECT_EQ(ident->name()[0], "a");
}

TEST_F(ParserImplTest, UnaryExpression_Dpdx_WithModifier) {
  auto p = parser("dpdx<coarse>(a)");
  auto e = p->unary_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsUnaryDerivative());

  auto deriv = e->AsUnaryDerivative();
  EXPECT_EQ(deriv->op(), ast::UnaryDerivative::kDpdx);
  EXPECT_EQ(deriv->modifier(), ast::DerivativeModifier::kCoarse);

  ASSERT_NE(deriv->param(), nullptr);
  ASSERT_TRUE(deriv->param()->IsIdentifier());
  auto ident = deriv->param()->AsIdentifier();
  ASSERT_EQ(ident->name().size(), 1);
  EXPECT_EQ(ident->name()[0], "a");
}

TEST_F(ParserImplTest, UnaryExpression_Dpdx_MissingLessThan) {
  auto p = parser("dpdx coarse>(a)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:6: missing ( for derivative method");
}

TEST_F(ParserImplTest, UnaryExpression_Dpdx_InvalidModifier) {
  auto p = parser("dpdx<invalid>(a)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:6: unable to parse derivative modifier");
}

TEST_F(ParserImplTest, UnaryExpression_Dpdx_EmptyModifer) {
  auto p = parser("dpdx coarse>(a)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:6: missing ( for derivative method");
}

TEST_F(ParserImplTest, UnaryExpression_Dpdx_MissingGreaterThan) {
  auto p = parser("dpdx<coarse (a)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:13: missing > for derivative modifier");
}

TEST_F(ParserImplTest, UnaryExpression_Dpdx_MisisngLeftParen) {
  auto p = parser("dpdx<coarse>a)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:13: missing ( for derivative method");
}

TEST_F(ParserImplTest, UnaryExpression_Dpdx_MissingRightParen) {
  auto p = parser("dpdx<coarse>(a");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:15: missing ) for derivative method");
}

TEST_F(ParserImplTest, UnaryExpression_Dpdx_MissingIdentifier) {
  auto p = parser("dpdx<coarse>()");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:14: missing identifier for derivative method");
}

TEST_F(ParserImplTest, UnaryExpression_Dpdx_InvalidIdentifeir) {
  auto p = parser("dpdx<coarse>(12345)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:14: missing identifier for derivative method");
}

TEST_F(ParserImplTest, UnaryExpression_Dpdy_NoModifier) {
  auto p = parser("dpdy(a)");
  auto e = p->unary_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsUnaryDerivative());

  auto deriv = e->AsUnaryDerivative();
  EXPECT_EQ(deriv->op(), ast::UnaryDerivative::kDpdy);
  EXPECT_EQ(deriv->modifier(), ast::DerivativeModifier::kNone);

  ASSERT_NE(deriv->param(), nullptr);
  ASSERT_TRUE(deriv->param()->IsIdentifier());
  auto ident = deriv->param()->AsIdentifier();
  ASSERT_EQ(ident->name().size(), 1);
  EXPECT_EQ(ident->name()[0], "a");
}

TEST_F(ParserImplTest, UnaryExpression_Dpdy_WithModifier) {
  auto p = parser("dpdy<fine>(a)");
  auto e = p->unary_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsUnaryDerivative());

  auto deriv = e->AsUnaryDerivative();
  EXPECT_EQ(deriv->op(), ast::UnaryDerivative::kDpdy);
  EXPECT_EQ(deriv->modifier(), ast::DerivativeModifier::kFine);

  ASSERT_NE(deriv->param(), nullptr);
  ASSERT_TRUE(deriv->param()->IsIdentifier());
  auto ident = deriv->param()->AsIdentifier();
  ASSERT_EQ(ident->name().size(), 1);
  EXPECT_EQ(ident->name()[0], "a");
}

TEST_F(ParserImplTest, UnaryExpression_Dpdy_MissingLessThan) {
  auto p = parser("dpdy coarse>(a)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:6: missing ( for derivative method");
}

TEST_F(ParserImplTest, UnaryExpression_Dpdy_InvalidModifier) {
  auto p = parser("dpdy<invalid>(a)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:6: unable to parse derivative modifier");
}

TEST_F(ParserImplTest, UnaryExpression_Dpdy_EmptyModifer) {
  auto p = parser("dpdy coarse>(a)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:6: missing ( for derivative method");
}

TEST_F(ParserImplTest, UnaryExpression_Dpdy_MissingGreaterThan) {
  auto p = parser("dpdy<coarse (a)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:13: missing > for derivative modifier");
}

TEST_F(ParserImplTest, UnaryExpression_Dpdy_MisisngLeftParen) {
  auto p = parser("dpdy<coarse>a)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:13: missing ( for derivative method");
}

TEST_F(ParserImplTest, UnaryExpression_Dpdy_MissingRightParen) {
  auto p = parser("dpdy<coarse>(a");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:15: missing ) for derivative method");
}

TEST_F(ParserImplTest, UnaryExpression_Dpdy_MissingIdentifier) {
  auto p = parser("dpdy<coarse>()");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:14: missing identifier for derivative method");
}

TEST_F(ParserImplTest, UnaryExpression_Dpdy_InvalidIdentifeir) {
  auto p = parser("dpdy<coarse>(12345)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:14: missing identifier for derivative method");
}

TEST_F(ParserImplTest, UnaryExpression_Fwidth_NoModifier) {
  auto p = parser("fwidth(a)");
  auto e = p->unary_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsUnaryDerivative());

  auto deriv = e->AsUnaryDerivative();
  EXPECT_EQ(deriv->op(), ast::UnaryDerivative::kFwidth);
  EXPECT_EQ(deriv->modifier(), ast::DerivativeModifier::kNone);

  ASSERT_NE(deriv->param(), nullptr);
  ASSERT_TRUE(deriv->param()->IsIdentifier());
  auto ident = deriv->param()->AsIdentifier();
  ASSERT_EQ(ident->name().size(), 1);
  EXPECT_EQ(ident->name()[0], "a");
}

TEST_F(ParserImplTest, UnaryExpression_Fwidth_WithModifier) {
  auto p = parser("fwidth<coarse>(a)");
  auto e = p->unary_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsUnaryDerivative());

  auto deriv = e->AsUnaryDerivative();
  EXPECT_EQ(deriv->op(), ast::UnaryDerivative::kFwidth);
  EXPECT_EQ(deriv->modifier(), ast::DerivativeModifier::kCoarse);

  ASSERT_NE(deriv->param(), nullptr);
  ASSERT_TRUE(deriv->param()->IsIdentifier());
  auto ident = deriv->param()->AsIdentifier();
  ASSERT_EQ(ident->name().size(), 1);
  EXPECT_EQ(ident->name()[0], "a");
}

TEST_F(ParserImplTest, UnaryExpression_Fwidth_MissingLessThan) {
  auto p = parser("fwidth coarse>(a)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:8: missing ( for derivative method");
}

TEST_F(ParserImplTest, UnaryExpression_Fwidth_InvalidModifier) {
  auto p = parser("fwidth<invalid>(a)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:8: unable to parse derivative modifier");
}

TEST_F(ParserImplTest, UnaryExpression_Fwidth_EmptyModifer) {
  auto p = parser("fwidth coarse>(a)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:8: missing ( for derivative method");
}

TEST_F(ParserImplTest, UnaryExpression_Fwidth_MissingGreaterThan) {
  auto p = parser("fwidth<coarse (a)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:15: missing > for derivative modifier");
}

TEST_F(ParserImplTest, UnaryExpression_Fwidth_MisisngLeftParen) {
  auto p = parser("fwidth<coarse>a)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:15: missing ( for derivative method");
}

TEST_F(ParserImplTest, UnaryExpression_Fwidth_MissingRightParen) {
  auto p = parser("fwidth<coarse>(a");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:17: missing ) for derivative method");
}

TEST_F(ParserImplTest, UnaryExpression_Fwidth_MissingIdentifier) {
  auto p = parser("fwidth<coarse>()");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:16: missing identifier for derivative method");
}

TEST_F(ParserImplTest, UnaryExpression_Fwidht_InvalidIdentifeir) {
  auto p = parser("fwidth<coarse>(12345)");
  auto e = p->unary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:16: missing identifier for derivative method");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
