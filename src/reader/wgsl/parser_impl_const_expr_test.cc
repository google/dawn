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
#include "src/ast/const_initializer_expression.h"
#include "src/ast/float_literal.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type_initializer_expression.h"
#include "src/reader/wgsl/parser_impl.h"

namespace tint {
namespace reader {
namespace wgsl {

using ParserImplTest = testing::Test;

TEST_F(ParserImplTest, ConstExpr_TypeDecl) {
  ParserImpl p{"vec2<f32>(1., 2.)"};
  auto e = p.const_expr();
  ASSERT_FALSE(p.has_error()) << p.error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsInitializer());
  ASSERT_TRUE(e->AsInitializer()->IsTypeInitializer());

  auto t = e->AsInitializer()->AsTypeInitializer();
  ASSERT_TRUE(t->type()->IsVector());
  EXPECT_EQ(t->type()->AsVector()->size(), 2);

  ASSERT_EQ(t->values().size(), 2);
  auto& v = t->values();

  ASSERT_TRUE(v[0]->IsInitializer());
  ASSERT_TRUE(v[0]->AsInitializer()->IsConstInitializer());
  auto c = v[0]->AsInitializer()->AsConstInitializer();
  ASSERT_TRUE(c->literal()->IsFloat());
  EXPECT_FLOAT_EQ(c->literal()->AsFloat()->value(), 1.);

  ASSERT_TRUE(v[1]->IsInitializer());
  ASSERT_TRUE(v[1]->AsInitializer()->IsConstInitializer());
  c = v[1]->AsInitializer()->AsConstInitializer();
  ASSERT_TRUE(c->literal()->IsFloat());
  EXPECT_FLOAT_EQ(c->literal()->AsFloat()->value(), 2.);
}

TEST_F(ParserImplTest, ConstExpr_TypeDecl_MissingRightParen) {
  ParserImpl p{"vec2<f32>(1., 2."};
  auto e = p.const_expr();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:17: missing ) for type initializer");
}

TEST_F(ParserImplTest, ConstExpr_TypeDecl_MissingLeftParen) {
  ParserImpl p{"vec2<f32> 1., 2.)"};
  auto e = p.const_expr();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:11: missing ( for type initializer");
}

TEST_F(ParserImplTest, ConstExpr_TypeDecl_HangingComma) {
  ParserImpl p{"vec2<f32>(1.,)"};
  auto e = p.const_expr();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:14: unable to parse const literal");
}

TEST_F(ParserImplTest, ConstExpr_TypeDecl_MissingComma) {
  ParserImpl p{"vec2<f32>(1. 2."};
  auto e = p.const_expr();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:14: missing ) for type initializer");
}

TEST_F(ParserImplTest, ConstExpr_MissingExpr) {
  ParserImpl p{"vec2<f32>()"};
  auto e = p.const_expr();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:11: unable to parse const literal");
}

TEST_F(ParserImplTest, ConstExpr_InvalidExpr) {
  ParserImpl p{"vec2<f32>(1., if(a) {})"};
  auto e = p.const_expr();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:15: unable to parse const literal");
}

TEST_F(ParserImplTest, ConstExpr_ConstLiteral) {
  ParserImpl p{"true"};
  auto e = p.const_expr();
  ASSERT_FALSE(p.has_error()) << p.error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsInitializer());
  ASSERT_TRUE(e->AsInitializer()->IsConstInitializer());
  auto c = e->AsInitializer()->AsConstInitializer();
  ASSERT_TRUE(c->literal()->IsBool());
  EXPECT_TRUE(c->literal()->AsBool()->IsTrue());
}

TEST_F(ParserImplTest, ConstExpr_ConstLiteral_Invalid) {
  ParserImpl p{"invalid"};
  auto e = p.const_expr();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:1: unknown type alias 'invalid'");
}

}  // namespace wgsl
}  // namespace reader
}  // namespace tint
