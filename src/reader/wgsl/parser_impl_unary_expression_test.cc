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

namespace tint {
namespace reader {
namespace wgsl {

using ParserImplTest = testing::Test;

TEST_F(ParserImplTest, UnaryExpression_Postix) {
  ParserImpl p{"a[2]"};
  auto e = p.unary_expression();
  ASSERT_FALSE(p.has_error()) << p.error();
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
  ParserImpl p{"- 1"};
  auto e = p.unary_expression();
  ASSERT_FALSE(p.has_error()) << p.error();
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
  ParserImpl p{"-if(a) {}"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:2: unable to parse right side of - expression");
}

TEST_F(ParserImplTest, UnaryExpression_Bang) {
  ParserImpl p{"!1"};
  auto e = p.unary_expression();
  ASSERT_FALSE(p.has_error()) << p.error();
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
  ParserImpl p{"!if (a) {}"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:2: unable to parse right side of ! expression");
}

TEST_F(ParserImplTest, UnaryExpression_Any) {
  ParserImpl p{"any(a)"};
  auto e = p.unary_expression();
  ASSERT_FALSE(p.has_error()) << p.error();
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
  ParserImpl p{"any a)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:5: missing ( for method call");
}

TEST_F(ParserImplTest, UnaryExpression_Any_MissingParenRight) {
  ParserImpl p{"any(a"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:6: missing ) for method call");
}

TEST_F(ParserImplTest, UnaryExpression_Any_MissingIdentifier) {
  ParserImpl p{"any()"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:5: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_Any_InvalidIdentifier) {
  ParserImpl p{"any(123)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:5: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_All) {
  ParserImpl p{"all(a)"};
  auto e = p.unary_expression();
  ASSERT_FALSE(p.has_error()) << p.error();
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
  ParserImpl p{"all a)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:5: missing ( for method call");
}

TEST_F(ParserImplTest, UnaryExpression_All_MissingParenRight) {
  ParserImpl p{"all(a"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:6: missing ) for method call");
}

TEST_F(ParserImplTest, UnaryExpression_All_MissingIdentifier) {
  ParserImpl p{"all()"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:5: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_All_InvalidIdentifier) {
  ParserImpl p{"all(123)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:5: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_IsNan) {
  ParserImpl p{"is_nan(a)"};
  auto e = p.unary_expression();
  ASSERT_FALSE(p.has_error()) << p.error();
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
  ParserImpl p{"is_nan a)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:8: missing ( for method call");
}

TEST_F(ParserImplTest, UnaryExpression_IsNan_MissingParenRight) {
  ParserImpl p{"is_nan(a"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:9: missing ) for method call");
}

TEST_F(ParserImplTest, UnaryExpression_IsNan_MissingIdentifier) {
  ParserImpl p{"is_nan()"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:8: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_IsNan_InvalidIdentifier) {
  ParserImpl p{"is_nan(123)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:8: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_IsInf) {
  ParserImpl p{"is_inf(a)"};
  auto e = p.unary_expression();
  ASSERT_FALSE(p.has_error()) << p.error();
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
  ParserImpl p{"is_inf a)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:8: missing ( for method call");
}

TEST_F(ParserImplTest, UnaryExpression_IsInf_MissingParenRight) {
  ParserImpl p{"is_inf(a"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:9: missing ) for method call");
}

TEST_F(ParserImplTest, UnaryExpression_IsInf_MissingIdentifier) {
  ParserImpl p{"is_inf()"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:8: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_IsInf_InvalidIdentifier) {
  ParserImpl p{"is_inf(123)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:8: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_IsFinite) {
  ParserImpl p{"is_finite(a)"};
  auto e = p.unary_expression();
  ASSERT_FALSE(p.has_error()) << p.error();
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
  ParserImpl p{"is_finite a)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:11: missing ( for method call");
}

TEST_F(ParserImplTest, UnaryExpression_IsFinite_MissingParenRight) {
  ParserImpl p{"is_finite(a"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:12: missing ) for method call");
}

TEST_F(ParserImplTest, UnaryExpression_IsFinite_MissingIdentifier) {
  ParserImpl p{"is_finite()"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:11: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_IsFinite_InvalidIdentifier) {
  ParserImpl p{"is_finite(123)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:11: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_IsNormal) {
  ParserImpl p{"is_normal(a)"};
  auto e = p.unary_expression();
  ASSERT_FALSE(p.has_error()) << p.error();
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
  ParserImpl p{"is_normal a)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:11: missing ( for method call");
}

TEST_F(ParserImplTest, UnaryExpression_IsNormal_MissingParenRight) {
  ParserImpl p{"is_normal(a"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:12: missing ) for method call");
}

TEST_F(ParserImplTest, UnaryExpression_IsNormal_MissingIdentifier) {
  ParserImpl p{"is_normal()"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:11: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_IsNormal_InvalidIdentifier) {
  ParserImpl p{"is_normal(123)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:11: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_Dot) {
  ParserImpl p{"dot(a, b)"};
  auto e = p.unary_expression();
  ASSERT_FALSE(p.has_error()) << p.error();
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
  ParserImpl p{"dot a, b)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:5: missing ( for method call");
}

TEST_F(ParserImplTest, UnaryExpression_Dot_MissingParenRight) {
  ParserImpl p{"dot(a, b"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:9: missing ) for method call");
}

TEST_F(ParserImplTest, UnaryExpression_Dot_MissingFirstIdentifier) {
  ParserImpl p{"dot(, a)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:5: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_Dot_MissingSecondIdentifier) {
  ParserImpl p{"dot(a, )"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:8: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_Dot_MissingComma) {
  ParserImpl p{"dot(a b)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:7: missing , for method call");
}

TEST_F(ParserImplTest, UnaryExpression_Dot_InvalidFirstIdentifier) {
  ParserImpl p{"dot(123, b)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:5: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_Dot_InvalidSecondIdentifier) {
  ParserImpl p{"dot(a, 123)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:8: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_OuterProduct) {
  ParserImpl p{"outer_product(a, b)"};
  auto e = p.unary_expression();
  ASSERT_FALSE(p.has_error()) << p.error();
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
  ParserImpl p{"outer_product a, b)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:15: missing ( for method call");
}

TEST_F(ParserImplTest, UnaryExpression_OuterProduct_MissingParenRight) {
  ParserImpl p{"outer_product(a, b"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:19: missing ) for method call");
}

TEST_F(ParserImplTest, UnaryExpression_OuterProduct_MissingFirstIdentifier) {
  ParserImpl p{"outer_product(, b)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:15: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_OuterProduct_MissingSecondIdentifier) {
  ParserImpl p{"outer_product(a, )"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:18: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_OuterProduct_MissingComma) {
  ParserImpl p{"outer_product(a b)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:17: missing , for method call");
}

TEST_F(ParserImplTest, UnaryExpression_OuterProduct_InvalidFirstIdentifier) {
  ParserImpl p{"outer_product(123, b)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:15: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_OuterProduct_InvalidSecondIdentifier) {
  ParserImpl p{"outer_product(a, 123)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:18: missing identifier for method call");
}

TEST_F(ParserImplTest, UnaryExpression_Dpdx_NoModifier) {
  ParserImpl p{"dpdx(a)"};
  auto e = p.unary_expression();
  ASSERT_FALSE(p.has_error()) << p.error();
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
  ParserImpl p{"dpdx<coarse>(a)"};
  auto e = p.unary_expression();
  ASSERT_FALSE(p.has_error()) << p.error();
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
  ParserImpl p{"dpdx coarse>(a)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:6: missing ( for derivative method");
}

TEST_F(ParserImplTest, UnaryExpression_Dpdx_InvalidModifier) {
  ParserImpl p{"dpdx<invalid>(a)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:6: unable to parse derivative modifier");
}

TEST_F(ParserImplTest, UnaryExpression_Dpdx_EmptyModifer) {
  ParserImpl p{"dpdx coarse>(a)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:6: missing ( for derivative method");
}

TEST_F(ParserImplTest, UnaryExpression_Dpdx_MissingGreaterThan) {
  ParserImpl p{"dpdx<coarse (a)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:13: missing > for derivative modifier");
}

TEST_F(ParserImplTest, UnaryExpression_Dpdx_MisisngLeftParen) {
  ParserImpl p{"dpdx<coarse>a)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:13: missing ( for derivative method");
}

TEST_F(ParserImplTest, UnaryExpression_Dpdx_MissingRightParen) {
  ParserImpl p{"dpdx<coarse>(a"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:15: missing ) for derivative method");
}

TEST_F(ParserImplTest, UnaryExpression_Dpdx_MissingIdentifier) {
  ParserImpl p{"dpdx<coarse>()"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:14: missing identifier for derivative method");
}

TEST_F(ParserImplTest, UnaryExpression_Dpdx_InvalidIdentifeir) {
  ParserImpl p{"dpdx<coarse>(12345)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:14: missing identifier for derivative method");
}

TEST_F(ParserImplTest, UnaryExpression_Dpdy_NoModifier) {
  ParserImpl p{"dpdy(a)"};
  auto e = p.unary_expression();
  ASSERT_FALSE(p.has_error()) << p.error();
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
  ParserImpl p{"dpdy<fine>(a)"};
  auto e = p.unary_expression();
  ASSERT_FALSE(p.has_error()) << p.error();
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
  ParserImpl p{"dpdy coarse>(a)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:6: missing ( for derivative method");
}

TEST_F(ParserImplTest, UnaryExpression_Dpdy_InvalidModifier) {
  ParserImpl p{"dpdy<invalid>(a)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:6: unable to parse derivative modifier");
}

TEST_F(ParserImplTest, UnaryExpression_Dpdy_EmptyModifer) {
  ParserImpl p{"dpdy coarse>(a)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:6: missing ( for derivative method");
}

TEST_F(ParserImplTest, UnaryExpression_Dpdy_MissingGreaterThan) {
  ParserImpl p{"dpdy<coarse (a)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:13: missing > for derivative modifier");
}

TEST_F(ParserImplTest, UnaryExpression_Dpdy_MisisngLeftParen) {
  ParserImpl p{"dpdy<coarse>a)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:13: missing ( for derivative method");
}

TEST_F(ParserImplTest, UnaryExpression_Dpdy_MissingRightParen) {
  ParserImpl p{"dpdy<coarse>(a"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:15: missing ) for derivative method");
}

TEST_F(ParserImplTest, UnaryExpression_Dpdy_MissingIdentifier) {
  ParserImpl p{"dpdy<coarse>()"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:14: missing identifier for derivative method");
}

TEST_F(ParserImplTest, UnaryExpression_Dpdy_InvalidIdentifeir) {
  ParserImpl p{"dpdy<coarse>(12345)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:14: missing identifier for derivative method");
}

TEST_F(ParserImplTest, UnaryExpression_Fwidth_NoModifier) {
  ParserImpl p{"fwidth(a)"};
  auto e = p.unary_expression();
  ASSERT_FALSE(p.has_error()) << p.error();
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
  ParserImpl p{"fwidth<coarse>(a)"};
  auto e = p.unary_expression();
  ASSERT_FALSE(p.has_error()) << p.error();
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
  ParserImpl p{"fwidth coarse>(a)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:8: missing ( for derivative method");
}

TEST_F(ParserImplTest, UnaryExpression_Fwidth_InvalidModifier) {
  ParserImpl p{"fwidth<invalid>(a)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:8: unable to parse derivative modifier");
}

TEST_F(ParserImplTest, UnaryExpression_Fwidth_EmptyModifer) {
  ParserImpl p{"fwidth coarse>(a)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:8: missing ( for derivative method");
}

TEST_F(ParserImplTest, UnaryExpression_Fwidth_MissingGreaterThan) {
  ParserImpl p{"fwidth<coarse (a)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:15: missing > for derivative modifier");
}

TEST_F(ParserImplTest, UnaryExpression_Fwidth_MisisngLeftParen) {
  ParserImpl p{"fwidth<coarse>a)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:15: missing ( for derivative method");
}

TEST_F(ParserImplTest, UnaryExpression_Fwidth_MissingRightParen) {
  ParserImpl p{"fwidth<coarse>(a"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:17: missing ) for derivative method");
}

TEST_F(ParserImplTest, UnaryExpression_Fwidth_MissingIdentifier) {
  ParserImpl p{"fwidth<coarse>()"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:16: missing identifier for derivative method");
}

TEST_F(ParserImplTest, UnaryExpression_Fwidht_InvalidIdentifeir) {
  ParserImpl p{"fwidth<coarse>(12345)"};
  auto e = p.unary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:16: missing identifier for derivative method");
}
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
