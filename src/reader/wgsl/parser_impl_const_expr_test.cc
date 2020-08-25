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
#include "src/ast/bool_literal.h"
#include "src/ast/float_literal.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type_constructor_expression.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, ConstExpr_TypeDecl) {
  auto* p = parser("vec2<f32>(1., 2.)");
  auto e = p->const_expr();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsConstructor());
  ASSERT_TRUE(e->AsConstructor()->IsTypeConstructor());

  auto* t = e->AsConstructor()->AsTypeConstructor();
  ASSERT_TRUE(t->type()->IsVector());
  EXPECT_EQ(t->type()->AsVector()->size(), 2u);

  ASSERT_EQ(t->values().size(), 2u);
  auto& v = t->values();

  ASSERT_TRUE(v[0]->IsConstructor());
  ASSERT_TRUE(v[0]->AsConstructor()->IsScalarConstructor());
  auto* c = v[0]->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(c->literal()->IsFloat());
  EXPECT_FLOAT_EQ(c->literal()->AsFloat()->value(), 1.);

  ASSERT_TRUE(v[1]->IsConstructor());
  ASSERT_TRUE(v[1]->AsConstructor()->IsScalarConstructor());
  c = v[1]->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(c->literal()->IsFloat());
  EXPECT_FLOAT_EQ(c->literal()->AsFloat()->value(), 2.);
}

TEST_F(ParserImplTest, ConstExpr_TypeDecl_MissingRightParen) {
  auto* p = parser("vec2<f32>(1., 2.");
  auto e = p->const_expr();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:17: missing ) for type constructor");
}

TEST_F(ParserImplTest, ConstExpr_TypeDecl_MissingLeftParen) {
  auto* p = parser("vec2<f32> 1., 2.)");
  auto e = p->const_expr();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:11: missing ( for type constructor");
}

TEST_F(ParserImplTest, ConstExpr_TypeDecl_HangingComma) {
  auto* p = parser("vec2<f32>(1.,)");
  auto e = p->const_expr();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:14: unable to parse const literal");
}

TEST_F(ParserImplTest, ConstExpr_TypeDecl_MissingComma) {
  auto* p = parser("vec2<f32>(1. 2.");
  auto e = p->const_expr();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:14: missing ) for type constructor");
}

TEST_F(ParserImplTest, ConstExpr_MissingExpr) {
  auto* p = parser("vec2<f32>()");
  auto e = p->const_expr();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:11: unable to parse const literal");
}

TEST_F(ParserImplTest, ConstExpr_InvalidExpr) {
  auto* p = parser("vec2<f32>(1., if(a) {})");
  auto e = p->const_expr();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:15: unable to parse const literal");
}

TEST_F(ParserImplTest, ConstExpr_ConstLiteral) {
  auto* p = parser("true");
  auto e = p->const_expr();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsConstructor());
  ASSERT_TRUE(e->AsConstructor()->IsScalarConstructor());
  auto* c = e->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(c->literal()->IsBool());
  EXPECT_TRUE(c->literal()->AsBool()->IsTrue());
}

TEST_F(ParserImplTest, ConstExpr_ConstLiteral_Invalid) {
  auto* p = parser("invalid");
  auto e = p->const_expr();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:1: unknown type alias 'invalid'");
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
  auto* p = parser(out.str());
  auto e = p->const_expr();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:517: max const_expr depth reached");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
