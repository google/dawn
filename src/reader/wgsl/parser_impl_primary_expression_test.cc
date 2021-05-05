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

#include "src/ast/bitcast_expression.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, PrimaryExpression_Ident) {
  auto p = parser("a");
  auto e = p->primary_expression();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e.value, nullptr);
  ASSERT_TRUE(e->Is<ast::IdentifierExpression>());
  auto* ident = e->As<ast::IdentifierExpression>();
  EXPECT_EQ(ident->symbol(), p->builder().Symbols().Get("a"));
}

TEST_F(ParserImplTest, PrimaryExpression_TypeDecl) {
  auto p = parser("vec4<i32>(1, 2, 3, 4))");
  auto e = p->primary_expression();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e.value, nullptr);
  ASSERT_TRUE(e->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(e->Is<ast::TypeConstructorExpression>());
  auto* ty = e->As<ast::TypeConstructorExpression>();

  ASSERT_EQ(ty->values().size(), 4u);
  const auto& val = ty->values();
  ASSERT_TRUE(val[0]->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(val[0]->Is<ast::ScalarConstructorExpression>());
  auto* ident = val[0]->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(ident->literal()->Is<ast::SintLiteral>());
  EXPECT_EQ(ident->literal()->As<ast::SintLiteral>()->value(), 1);

  ASSERT_TRUE(val[1]->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(val[1]->Is<ast::ScalarConstructorExpression>());
  ident = val[1]->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(ident->literal()->Is<ast::SintLiteral>());
  EXPECT_EQ(ident->literal()->As<ast::SintLiteral>()->value(), 2);

  ASSERT_TRUE(val[2]->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(val[2]->Is<ast::ScalarConstructorExpression>());
  ident = val[2]->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(ident->literal()->Is<ast::SintLiteral>());
  EXPECT_EQ(ident->literal()->As<ast::SintLiteral>()->value(), 3);

  ASSERT_TRUE(val[3]->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(val[3]->Is<ast::ScalarConstructorExpression>());
  ident = val[3]->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(ident->literal()->Is<ast::SintLiteral>());
  EXPECT_EQ(ident->literal()->As<ast::SintLiteral>()->value(), 4);
}

TEST_F(ParserImplTest, PrimaryExpression_TypeDecl_ZeroConstructor) {
  auto p = parser("vec4<i32>()");
  auto e = p->primary_expression();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e.value, nullptr);
  ASSERT_TRUE(e->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(e->Is<ast::TypeConstructorExpression>());
  auto* ty = e->As<ast::TypeConstructorExpression>();

  ASSERT_EQ(ty->values().size(), 0u);
}

TEST_F(ParserImplTest, PrimaryExpression_TypeDecl_InvalidTypeDecl) {
  auto p = parser("vec4<if>(2., 3., 4., 5.)");
  auto e = p->primary_expression();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_EQ(e.value, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:6: invalid type for vector");
}

TEST_F(ParserImplTest, PrimaryExpression_TypeDecl_MissingLeftParen) {
  auto p = parser("vec4<f32> 2., 3., 4., 5.)");
  auto e = p->primary_expression();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_EQ(e.value, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:11: expected '(' for type constructor");
}

TEST_F(ParserImplTest, PrimaryExpression_TypeDecl_MissingRightParen) {
  auto p = parser("vec4<f32>(2., 3., 4., 5.");
  auto e = p->primary_expression();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_EQ(e.value, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:25: expected ')' for type constructor");
}

TEST_F(ParserImplTest, PrimaryExpression_TypeDecl_InvalidValue) {
  auto p = parser("i32(if(a) {})");
  auto e = p->primary_expression();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_EQ(e.value, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:5: expected ')' for type constructor");
}

TEST_F(ParserImplTest, PrimaryExpression_TypeDecl_StructConstructor_Empty) {
  auto p = parser(R"(
  struct S { a : i32; b : f32; };
  S()
  )");

  p->expect_global_decl();
  ASSERT_FALSE(p->has_error()) << p->error();

  auto e = p->primary_expression();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e.value, nullptr);
  ASSERT_TRUE(e->Is<ast::TypeConstructorExpression>());

  auto* constructor = e->As<ast::TypeConstructorExpression>();
  ASSERT_TRUE(constructor->type()->Is<ast::TypeName>());
  EXPECT_EQ(constructor->type()->As<ast::TypeName>()->name(),
            p->builder().Symbols().Get("S"));

  auto values = constructor->values();
  ASSERT_EQ(values.size(), 0u);
}

TEST_F(ParserImplTest, PrimaryExpression_TypeDecl_StructConstructor_NotEmpty) {
  auto p = parser(R"(
  struct S { a : i32; b : f32; };
  S(1u, 2.0)
  )");

  p->expect_global_decl();
  ASSERT_FALSE(p->has_error()) << p->error();

  auto e = p->primary_expression();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e.value, nullptr);
  ASSERT_TRUE(e->Is<ast::TypeConstructorExpression>());

  auto* constructor = e->As<ast::TypeConstructorExpression>();
  ASSERT_TRUE(constructor->type()->Is<ast::TypeName>());
  EXPECT_EQ(constructor->type()->As<ast::TypeName>()->name(),
            p->builder().Symbols().Get("S"));

  auto values = constructor->values();
  ASSERT_EQ(values.size(), 2u);

  ASSERT_TRUE(values[0]->Is<ast::ScalarConstructorExpression>());
  auto* val0 = values[0]->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(val0->literal()->Is<ast::UintLiteral>());
  EXPECT_EQ(val0->literal()->As<ast::UintLiteral>()->value(), 1u);

  ASSERT_TRUE(values[1]->Is<ast::ScalarConstructorExpression>());
  auto* val1 = values[1]->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(val1->literal()->Is<ast::FloatLiteral>());
  EXPECT_EQ(val1->literal()->As<ast::FloatLiteral>()->value(), 2.f);
}

TEST_F(ParserImplTest, PrimaryExpression_ConstLiteral_True) {
  auto p = parser("true");
  auto e = p->primary_expression();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e.value, nullptr);
  ASSERT_TRUE(e->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(e->Is<ast::ScalarConstructorExpression>());
  auto* init = e->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(init->literal()->Is<ast::BoolLiteral>());
  EXPECT_TRUE(init->literal()->As<ast::BoolLiteral>()->IsTrue());
}

TEST_F(ParserImplTest, PrimaryExpression_ParenExpr) {
  auto p = parser("(a == b)");
  auto e = p->primary_expression();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e.value, nullptr);
  ASSERT_TRUE(e->Is<ast::BinaryExpression>());
}

TEST_F(ParserImplTest, PrimaryExpression_ParenExpr_MissingRightParen) {
  auto p = parser("(a == b");
  auto e = p->primary_expression();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_EQ(e.value, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:8: expected ')'");
}

TEST_F(ParserImplTest, PrimaryExpression_ParenExpr_MissingExpr) {
  auto p = parser("()");
  auto e = p->primary_expression();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_EQ(e.value, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:2: unable to parse expression");
}

TEST_F(ParserImplTest, PrimaryExpression_ParenExpr_InvalidExpr) {
  auto p = parser("(if (a) {})");
  auto e = p->primary_expression();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_EQ(e.value, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:2: unable to parse expression");
}

TEST_F(ParserImplTest, PrimaryExpression_Cast) {
  auto p = parser("f32(1)");

  auto e = p->primary_expression();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e.value, nullptr);
  ASSERT_TRUE(e->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(e->Is<ast::TypeConstructorExpression>());

  auto* c = e->As<ast::TypeConstructorExpression>();
  ASSERT_TRUE(c->type()->Is<ast::F32>());
  ASSERT_EQ(c->values().size(), 1u);

  ASSERT_TRUE(c->values()[0]->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(c->values()[0]->Is<ast::ScalarConstructorExpression>());
}

TEST_F(ParserImplTest, PrimaryExpression_Bitcast) {
  auto p = parser("bitcast<f32>(1)");

  auto e = p->primary_expression();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e.value, nullptr);
  ASSERT_TRUE(e->Is<ast::BitcastExpression>());

  auto* c = e->As<ast::BitcastExpression>();
  ASSERT_TRUE(c->type()->Is<ast::F32>());
  ASSERT_TRUE(c->expr()->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(c->expr()->Is<ast::ScalarConstructorExpression>());
}

TEST_F(ParserImplTest, PrimaryExpression_Bitcast_MissingGreaterThan) {
  auto p = parser("bitcast<f32(1)");
  auto e = p->primary_expression();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_EQ(e.value, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:12: expected '>' for bitcast expression");
}

TEST_F(ParserImplTest, PrimaryExpression_Bitcast_MissingType) {
  auto p = parser("bitcast<>(1)");
  auto e = p->primary_expression();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_EQ(e.value, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:9: invalid type for bitcast expression");
}

TEST_F(ParserImplTest, PrimaryExpression_Bitcast_InvalidType) {
  auto p = parser("bitcast<invalid>(1)");
  auto e = p->primary_expression();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_EQ(e.value, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:9: unknown constructed type 'invalid'");
}

TEST_F(ParserImplTest, PrimaryExpression_Bitcast_MissingLeftParen) {
  auto p = parser("bitcast<f32>1)");
  auto e = p->primary_expression();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_EQ(e.value, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:13: expected '('");
}

TEST_F(ParserImplTest, PrimaryExpression_Bitcast_MissingRightParen) {
  auto p = parser("bitcast<f32>(1");
  auto e = p->primary_expression();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_EQ(e.value, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:15: expected ')'");
}

TEST_F(ParserImplTest, PrimaryExpression_Bitcast_MissingExpression) {
  auto p = parser("bitcast<f32>()");
  auto e = p->primary_expression();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_EQ(e.value, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:14: unable to parse expression");
}

TEST_F(ParserImplTest, PrimaryExpression_bitcast_InvalidExpression) {
  auto p = parser("bitcast<f32>(if (a) {})");
  auto e = p->primary_expression();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_EQ(e.value, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:14: unable to parse expression");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
