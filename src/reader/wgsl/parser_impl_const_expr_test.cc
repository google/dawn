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

#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, ConstExpr_TypeDecl) {
  auto p = parser("vec2<f32>(1., 2.)");
  auto e = p->expect_const_expr();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(e.errored);
  ASSERT_TRUE(e->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(e->Is<ast::TypeConstructorExpression>());

  auto* t = e->As<ast::TypeConstructorExpression>();
  ASSERT_TRUE(t->type()->Is<ast::Vector>());
  EXPECT_EQ(t->type()->As<ast::Vector>()->size(), 2u);

  ASSERT_EQ(t->values().size(), 2u);
  auto& v = t->values();

  ASSERT_TRUE(v[0]->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(v[0]->Is<ast::ScalarConstructorExpression>());
  auto* c = v[0]->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(c->literal()->Is<ast::FloatLiteral>());
  EXPECT_FLOAT_EQ(c->literal()->As<ast::FloatLiteral>()->value(), 1.);

  ASSERT_TRUE(v[1]->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(v[1]->Is<ast::ScalarConstructorExpression>());
  c = v[1]->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(c->literal()->Is<ast::FloatLiteral>());
  EXPECT_FLOAT_EQ(c->literal()->As<ast::FloatLiteral>()->value(), 2.);
}

TEST_F(ParserImplTest, ConstExpr_TypeDecl_Empty) {
  auto p = parser("vec2<f32>()");
  auto e = p->expect_const_expr();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(e.errored);
  ASSERT_TRUE(e->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(e->Is<ast::TypeConstructorExpression>());

  auto* t = e->As<ast::TypeConstructorExpression>();
  ASSERT_TRUE(t->type()->Is<ast::Vector>());
  EXPECT_EQ(t->type()->As<ast::Vector>()->size(), 2u);

  ASSERT_EQ(t->values().size(), 0u);
}

TEST_F(ParserImplTest, ConstExpr_TypeDecl_TrailingComma) {
  auto p = parser("vec2<f32>(1., 2.,)");
  auto e = p->expect_const_expr();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(e.errored);
  ASSERT_TRUE(e->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(e->Is<ast::TypeConstructorExpression>());

  auto* t = e->As<ast::TypeConstructorExpression>();
  ASSERT_TRUE(t->type()->Is<ast::Vector>());
  EXPECT_EQ(t->type()->As<ast::Vector>()->size(), 2u);

  ASSERT_EQ(t->values().size(), 2u);
  ASSERT_TRUE(t->values()[0]->Is<ast::ScalarConstructorExpression>());
  ASSERT_TRUE(t->values()[1]->Is<ast::ScalarConstructorExpression>());
}

TEST_F(ParserImplTest, ConstExpr_TypeDecl_MissingRightParen) {
  auto p = parser("vec2<f32>(1., 2.");
  auto e = p->expect_const_expr();
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(e.errored);
  ASSERT_EQ(e.value, nullptr);
  EXPECT_EQ(p->error(), "1:17: expected ')' for type constructor");
}

TEST_F(ParserImplTest, ConstExpr_TypeDecl_MissingLeftParen) {
  auto p = parser("vec2<f32> 1., 2.)");
  auto e = p->expect_const_expr();
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(e.errored);
  ASSERT_EQ(e.value, nullptr);
  EXPECT_EQ(p->error(), "1:11: expected '(' for type constructor");
}

TEST_F(ParserImplTest, ConstExpr_TypeDecl_MissingComma) {
  auto p = parser("vec2<f32>(1. 2.");
  auto e = p->expect_const_expr();
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(e.errored);
  ASSERT_EQ(e.value, nullptr);
  EXPECT_EQ(p->error(), "1:14: expected ')' for type constructor");
}

TEST_F(ParserImplTest, ConstExpr_InvalidExpr) {
  auto p = parser("vec2<f32>(1., if(a) {})");
  auto e = p->expect_const_expr();
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(e.errored);
  ASSERT_EQ(e.value, nullptr);
  EXPECT_EQ(p->error(), "1:15: unable to parse constant literal");
}

TEST_F(ParserImplTest, ConstExpr_ConstLiteral) {
  auto p = parser("true");
  auto e = p->expect_const_expr();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(e.errored);
  ASSERT_NE(e.value, nullptr);
  ASSERT_TRUE(e->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(e->Is<ast::ScalarConstructorExpression>());
  auto* c = e->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(c->literal()->Is<ast::BoolLiteral>());
  EXPECT_TRUE(c->literal()->As<ast::BoolLiteral>()->IsTrue());
}

TEST_F(ParserImplTest, ConstExpr_ConstLiteral_Invalid) {
  auto p = parser("invalid");
  auto e = p->expect_const_expr();
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(e.errored);
  ASSERT_EQ(e.value, nullptr);
  EXPECT_EQ(p->error(), "1:1: unknown constructed type 'invalid'");
}

TEST_F(ParserImplTest, ConstExpr_Recursion) {
  std::stringstream out;
  for (size_t i = 0; i < 200; i++) {
    out << "f32(";
  }
  out << "1.0";
  for (size_t i = 0; i < 200; i++) {
    out << ")";
  }
  auto p = parser(out.str());
  auto e = p->expect_const_expr();
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(e.errored);
  ASSERT_EQ(e.value, nullptr);
  EXPECT_EQ(p->error(), "1:517: maximum parser recursive depth reached");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
