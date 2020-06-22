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
#include "src/ast/as_expression.h"
#include "src/ast/bool_literal.h"
#include "src/ast/cast_expression.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/unary_op_expression.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"
#include "src/type_manager.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, PrimaryExpression_Ident) {
  auto* p = parser("a");
  auto e = p->primary_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsIdentifier());
  auto* ident = e->AsIdentifier();
  EXPECT_EQ(ident->name(), "a");
}

TEST_F(ParserImplTest, PrimaryExpression_Ident_WithNamespace) {
  auto* p = parser("a::b::c::d");
  auto e = p->primary_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsIdentifier());
  auto* ident = e->AsIdentifier();
  EXPECT_EQ(ident->path(), "a::b::c");
  EXPECT_EQ(ident->name(), "d");
}

TEST_F(ParserImplTest, PrimaryExpression_Ident_MissingIdent) {
  auto* p = parser("a::");
  auto e = p->primary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:4: identifier expected");
}

TEST_F(ParserImplTest, PrimaryExpression_TypeDecl) {
  auto* p = parser("vec4<i32>(1, 2, 3, 4))");
  auto e = p->primary_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsConstructor());
  ASSERT_TRUE(e->AsConstructor()->IsTypeConstructor());
  auto* ty = e->AsConstructor()->AsTypeConstructor();

  ASSERT_EQ(ty->values().size(), 4u);
  const auto& val = ty->values();
  ASSERT_TRUE(val[0]->IsConstructor());
  ASSERT_TRUE(val[0]->AsConstructor()->IsScalarConstructor());
  auto* ident = val[0]->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(ident->literal()->IsSint());
  EXPECT_EQ(ident->literal()->AsSint()->value(), 1);

  ASSERT_TRUE(val[1]->IsConstructor());
  ASSERT_TRUE(val[1]->AsConstructor()->IsScalarConstructor());
  ident = val[1]->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(ident->literal()->IsSint());
  EXPECT_EQ(ident->literal()->AsSint()->value(), 2);

  ASSERT_TRUE(val[2]->IsConstructor());
  ASSERT_TRUE(val[2]->AsConstructor()->IsScalarConstructor());
  ident = val[2]->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(ident->literal()->IsSint());
  EXPECT_EQ(ident->literal()->AsSint()->value(), 3);

  ASSERT_TRUE(val[3]->IsConstructor());
  ASSERT_TRUE(val[3]->AsConstructor()->IsScalarConstructor());
  ident = val[3]->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(ident->literal()->IsSint());
  EXPECT_EQ(ident->literal()->AsSint()->value(), 4);
}

TEST_F(ParserImplTest, PrimaryExpression_TypeDecl_ZeroConstructor) {
  auto* p = parser("vec4<i32>()");
  auto e = p->primary_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsConstructor());
  ASSERT_TRUE(e->AsConstructor()->IsTypeConstructor());
  auto* ty = e->AsConstructor()->AsTypeConstructor();

  ASSERT_EQ(ty->values().size(), 0u);
}

TEST_F(ParserImplTest, PrimaryExpression_TypeDecl_InvalidTypeDecl) {
  auto* p = parser("vec4<if>(2., 3., 4., 5.)");
  auto e = p->primary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:6: unable to determine subtype for vector");
}

TEST_F(ParserImplTest, PrimaryExpression_TypeDecl_MissingLeftParen) {
  auto* p = parser("vec4<f32> 2., 3., 4., 5.)");
  auto e = p->primary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:11: missing ( for type constructor");
}

TEST_F(ParserImplTest, PrimaryExpression_TypeDecl_MissingRightParen) {
  auto* p = parser("vec4<f32>(2., 3., 4., 5.");
  auto e = p->primary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:25: missing ) for type constructor");
}

TEST_F(ParserImplTest, PrimaryExpression_TypeDecl_InvalidValue) {
  auto* p = parser("i32(if(a) {})");
  auto e = p->primary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:5: unable to parse argument expression");
}

TEST_F(ParserImplTest, PrimaryExpression_ConstLiteral_True) {
  auto* p = parser("true");
  auto e = p->primary_expression();
  ASSERT_FALSE(p->has_error());
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsConstructor());
  ASSERT_TRUE(e->AsConstructor()->IsScalarConstructor());
  auto* init = e->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(init->literal()->IsBool());
  EXPECT_TRUE(init->literal()->AsBool()->IsTrue());
}

TEST_F(ParserImplTest, PrimaryExpression_ParenExpr) {
  auto* p = parser("(a == b)");
  auto e = p->primary_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsBinary());
}

TEST_F(ParserImplTest, PrimaryExpression_ParenExpr_MissingRightParen) {
  auto* p = parser("(a == b");
  auto e = p->primary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:8: expected )");
}

TEST_F(ParserImplTest, PrimaryExpression_ParenExpr_MissingExpr) {
  auto* p = parser("()");
  auto e = p->primary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:2: unable to parse expression");
}

TEST_F(ParserImplTest, PrimaryExpression_ParenExpr_InvalidExpr) {
  auto* p = parser("(if (a) {})");
  auto e = p->primary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:2: unable to parse expression");
}

TEST_F(ParserImplTest, PrimaryExpression_Cast) {
  auto* f32_type = tm()->Get(std::make_unique<ast::type::F32Type>());

  auto* p = parser("cast<f32>(1)");
  auto e = p->primary_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsCast());

  auto* c = e->AsCast();
  ASSERT_EQ(c->type(), f32_type);

  ASSERT_TRUE(c->expr()->IsConstructor());
  ASSERT_TRUE(c->expr()->AsConstructor()->IsScalarConstructor());
}

TEST_F(ParserImplTest, PrimaryExpression_Cast_MissingGreaterThan) {
  auto* p = parser("cast<f32(1)");
  auto e = p->primary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:9: missing > for cast expression");
}

TEST_F(ParserImplTest, PrimaryExpression_Cast_MissingType) {
  auto* p = parser("cast<>(1)");
  auto e = p->primary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:6: missing type for cast expression");
}

TEST_F(ParserImplTest, PrimaryExpression_Cast_InvalidType) {
  auto* p = parser("cast<invalid>(1)");
  auto e = p->primary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:6: unknown type alias 'invalid'");
}

TEST_F(ParserImplTest, PrimaryExpression_Cast_MissingLeftParen) {
  auto* p = parser("cast<f32>1)");
  auto e = p->primary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:10: expected (");
}

TEST_F(ParserImplTest, PrimaryExpression_Cast_MissingRightParen) {
  auto* p = parser("cast<f32>(1");
  auto e = p->primary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:12: expected )");
}

TEST_F(ParserImplTest, PrimaryExpression_Cast_MissingExpression) {
  auto* p = parser("cast<f32>()");
  auto e = p->primary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:11: unable to parse expression");
}

TEST_F(ParserImplTest, PrimaryExpression_Cast_InvalidExpression) {
  auto* p = parser("cast<f32>(if (a) {})");
  auto e = p->primary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:11: unable to parse expression");
}

TEST_F(ParserImplTest, PrimaryExpression_As) {
  auto* f32_type = tm()->Get(std::make_unique<ast::type::F32Type>());

  auto* p = parser("as<f32>(1)");
  auto e = p->primary_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsAs());

  auto* c = e->AsAs();
  ASSERT_EQ(c->type(), f32_type);

  ASSERT_TRUE(c->expr()->IsConstructor());
  ASSERT_TRUE(c->expr()->AsConstructor()->IsScalarConstructor());
}

TEST_F(ParserImplTest, PrimaryExpression_As_MissingGreaterThan) {
  auto* p = parser("as<f32(1)");
  auto e = p->primary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:7: missing > for as expression");
}

TEST_F(ParserImplTest, PrimaryExpression_As_MissingType) {
  auto* p = parser("as<>(1)");
  auto e = p->primary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:4: missing type for as expression");
}

TEST_F(ParserImplTest, PrimaryExpression_As_InvalidType) {
  auto* p = parser("as<invalid>(1)");
  auto e = p->primary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:4: unknown type alias 'invalid'");
}

TEST_F(ParserImplTest, PrimaryExpression_As_MissingLeftParen) {
  auto* p = parser("as<f32>1)");
  auto e = p->primary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:8: expected (");
}

TEST_F(ParserImplTest, PrimaryExpression_As_MissingRightParen) {
  auto* p = parser("as<f32>(1");
  auto e = p->primary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:10: expected )");
}

TEST_F(ParserImplTest, PrimaryExpression_As_MissingExpression) {
  auto* p = parser("as<f32>()");
  auto e = p->primary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:9: unable to parse expression");
}

TEST_F(ParserImplTest, PrimaryExpression_As_InvalidExpression) {
  auto* p = parser("as<f32>(if (a) {})");
  auto e = p->primary_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:9: unable to parse expression");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
