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
#include "src/ast/const_initializer_expression.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/int_literal.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type_initializer_expression.h"
#include "src/ast/unary_derivative_expression.h"
#include "src/ast/unary_method_expression.h"
#include "src/ast/unary_op_expression.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/type_manager.h"

namespace tint {
namespace reader {
namespace wgsl {

using ParserImplTest = testing::Test;

TEST_F(ParserImplTest, PrimaryExpression_Ident) {
  ParserImpl p{"a"};
  auto e = p.primary_expression();
  ASSERT_FALSE(p.has_error()) << p.error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsIdentifier());
  auto ident = e->AsIdentifier();
  ASSERT_EQ(ident->name().size(), 1);
  EXPECT_EQ(ident->name()[0], "a");
}

TEST_F(ParserImplTest, PrimaryExpression_Ident_WithNamespace) {
  ParserImpl p{"a::b::c::d"};
  auto e = p.primary_expression();
  ASSERT_FALSE(p.has_error()) << p.error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsIdentifier());
  auto ident = e->AsIdentifier();
  ASSERT_EQ(ident->name().size(), 4);
  EXPECT_EQ(ident->name()[0], "a");
  EXPECT_EQ(ident->name()[1], "b");
  EXPECT_EQ(ident->name()[2], "c");
  EXPECT_EQ(ident->name()[3], "d");
}

TEST_F(ParserImplTest, PrimaryExpression_Ident_MissingIdent) {
  ParserImpl p{"a::"};
  auto e = p.primary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:4: identifier expected");
}

TEST_F(ParserImplTest, PrimaryExpression_TypeDecl) {
  ParserImpl p{"vec4<i32>(1, 2, 3, 4))"};
  auto e = p.primary_expression();
  ASSERT_FALSE(p.has_error()) << p.error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsInitializer());
  ASSERT_TRUE(e->AsInitializer()->IsTypeInitializer());
  auto ty = e->AsInitializer()->AsTypeInitializer();

  ASSERT_EQ(ty->values().size(), 4);
  const auto& val = ty->values();
  ASSERT_TRUE(val[0]->IsInitializer());
  ASSERT_TRUE(val[0]->AsInitializer()->IsConstInitializer());
  auto ident = val[0]->AsInitializer()->AsConstInitializer();
  ASSERT_TRUE(ident->literal()->IsInt());
  EXPECT_EQ(ident->literal()->AsInt()->value(), 1);

  ASSERT_TRUE(val[1]->IsInitializer());
  ASSERT_TRUE(val[1]->AsInitializer()->IsConstInitializer());
  ident = val[1]->AsInitializer()->AsConstInitializer();
  ASSERT_TRUE(ident->literal()->IsInt());
  EXPECT_EQ(ident->literal()->AsInt()->value(), 2);

  ASSERT_TRUE(val[2]->IsInitializer());
  ASSERT_TRUE(val[2]->AsInitializer()->IsConstInitializer());
  ident = val[2]->AsInitializer()->AsConstInitializer();
  ASSERT_TRUE(ident->literal()->IsInt());
  EXPECT_EQ(ident->literal()->AsInt()->value(), 3);

  ASSERT_TRUE(val[3]->IsInitializer());
  ASSERT_TRUE(val[3]->AsInitializer()->IsConstInitializer());
  ident = val[3]->AsInitializer()->AsConstInitializer();
  ASSERT_TRUE(ident->literal()->IsInt());
  EXPECT_EQ(ident->literal()->AsInt()->value(), 4);
}

TEST_F(ParserImplTest, PrimaryExpression_TypeDecl_InvalidTypeDecl) {
  ParserImpl p{"vec4<if>(2., 3., 4., 5.)"};
  auto e = p.primary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:6: unable to determine subtype for vector");
}

TEST_F(ParserImplTest, PrimaryExpression_TypeDecl_MissingLeftParen) {
  ParserImpl p{"vec4<f32> 2., 3., 4., 5.)"};
  auto e = p.primary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:11: missing ( for type initializer");
}

TEST_F(ParserImplTest, PrimaryExpression_TypeDecl_MissingRightParen) {
  ParserImpl p{"vec4<f32>(2., 3., 4., 5."};
  auto e = p.primary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:25: missing ) for type initializer");
}

TEST_F(ParserImplTest, PrimaryExpression_TypeDecl_InvalidValue) {
  ParserImpl p{"i32(if(a) {})"};
  auto e = p.primary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:5: unable to parse argument expression");
}

TEST_F(ParserImplTest, PrimaryExpression_ConstLiteral_True) {
  ParserImpl p{"true"};
  auto e = p.primary_expression();
  ASSERT_FALSE(p.has_error());
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsInitializer());
  ASSERT_TRUE(e->AsInitializer()->IsConstInitializer());
  auto init = e->AsInitializer()->AsConstInitializer();
  ASSERT_TRUE(init->literal()->IsBool());
  EXPECT_TRUE(init->literal()->AsBool()->IsTrue());
}

TEST_F(ParserImplTest, PrimaryExpression_ParenExpr) {
  ParserImpl p{"(a == b)"};
  auto e = p.primary_expression();
  ASSERT_FALSE(p.has_error()) << p.error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsRelational());
}

TEST_F(ParserImplTest, PrimaryExpression_ParenExpr_MissingRightParen) {
  ParserImpl p{"(a == b"};
  auto e = p.primary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:8: expected )");
}

TEST_F(ParserImplTest, PrimaryExpression_ParenExpr_MissingExpr) {
  ParserImpl p{"()"};
  auto e = p.primary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:2: unable to parse expression");
}

TEST_F(ParserImplTest, PrimaryExpression_ParenExpr_InvalidExpr) {
  ParserImpl p{"(if (a) {})"};
  auto e = p.primary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:2: unable to parse expression");
}

TEST_F(ParserImplTest, PrimaryExpression_Cast) {
  auto tm = TypeManager::Instance();
  auto f32_type = tm->Get(std::make_unique<ast::type::F32Type>());

  ParserImpl p{"cast<f32>(1)"};
  auto e = p.primary_expression();
  ASSERT_FALSE(p.has_error()) << p.error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsCast());

  auto c = e->AsCast();
  ASSERT_EQ(c->type(), f32_type);

  ASSERT_TRUE(c->expr()->IsInitializer());
  ASSERT_TRUE(c->expr()->AsInitializer()->IsConstInitializer());

  TypeManager::Destroy();
}

TEST_F(ParserImplTest, PrimaryExpression_Cast_MissingGreaterThan) {
  ParserImpl p{"cast<f32(1)"};
  auto e = p.primary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:9: missing > for cast expression");
}

TEST_F(ParserImplTest, PrimaryExpression_Cast_MissingType) {
  ParserImpl p{"cast<>(1)"};
  auto e = p.primary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:6: missing type for cast expression");
}

TEST_F(ParserImplTest, PrimaryExpression_Cast_InvalidType) {
  ParserImpl p{"cast<invalid>(1)"};
  auto e = p.primary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:6: unknown type alias 'invalid'");
}

TEST_F(ParserImplTest, PrimaryExpression_Cast_MissingLeftParen) {
  ParserImpl p{"cast<f32>1)"};
  auto e = p.primary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:10: expected (");
}

TEST_F(ParserImplTest, PrimaryExpression_Cast_MissingRightParen) {
  ParserImpl p{"cast<f32>(1"};
  auto e = p.primary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:12: expected )");
}

TEST_F(ParserImplTest, PrimaryExpression_Cast_MissingExpression) {
  ParserImpl p{"cast<f32>()"};
  auto e = p.primary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:11: unable to parse expression");
}

TEST_F(ParserImplTest, PrimaryExpression_Cast_InvalidExpression) {
  ParserImpl p{"cast<f32>(if (a) {})"};
  auto e = p.primary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:11: unable to parse expression");
}

TEST_F(ParserImplTest, PrimaryExpression_As) {
  auto tm = TypeManager::Instance();
  auto f32_type = tm->Get(std::make_unique<ast::type::F32Type>());

  ParserImpl p{"as<f32>(1)"};
  auto e = p.primary_expression();
  ASSERT_FALSE(p.has_error()) << p.error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsAs());

  auto c = e->AsAs();
  ASSERT_EQ(c->type(), f32_type);

  ASSERT_TRUE(c->expr()->IsInitializer());
  ASSERT_TRUE(c->expr()->AsInitializer()->IsConstInitializer());

  TypeManager::Destroy();
}

TEST_F(ParserImplTest, PrimaryExpression_As_MissingGreaterThan) {
  ParserImpl p{"as<f32(1)"};
  auto e = p.primary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:7: missing > for as expression");
}

TEST_F(ParserImplTest, PrimaryExpression_As_MissingType) {
  ParserImpl p{"as<>(1)"};
  auto e = p.primary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:4: missing type for as expression");
}

TEST_F(ParserImplTest, PrimaryExpression_As_InvalidType) {
  ParserImpl p{"as<invalid>(1)"};
  auto e = p.primary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:4: unknown type alias 'invalid'");
}

TEST_F(ParserImplTest, PrimaryExpression_As_MissingLeftParen) {
  ParserImpl p{"as<f32>1)"};
  auto e = p.primary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:8: expected (");
}

TEST_F(ParserImplTest, PrimaryExpression_As_MissingRightParen) {
  ParserImpl p{"as<f32>(1"};
  auto e = p.primary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:10: expected )");
}

TEST_F(ParserImplTest, PrimaryExpression_As_MissingExpression) {
  ParserImpl p{"as<f32>()"};
  auto e = p.primary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:9: unable to parse expression");
}

TEST_F(ParserImplTest, PrimaryExpression_As_InvalidExpression) {
  ParserImpl p{"as<f32>(if (a) {})"};
  auto e = p.primary_expression();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:9: unable to parse expression");
}

}  // namespace wgsl
}  // namespace reader
}  // namespace tint
