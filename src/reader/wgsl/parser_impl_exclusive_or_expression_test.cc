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
#include "src/ast/binary_expression.h"
#include "src/ast/bool_literal.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, ExclusiveOrExpression_Parses) {
  auto* p = parser("a ^ true");
  auto e = p->exclusive_or_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);

  ASSERT_TRUE(e->IsBinary());
  auto* rel = e->AsBinary();
  EXPECT_EQ(ast::BinaryOp::kXor, rel->op());

  ASSERT_TRUE(rel->lhs()->IsIdentifier());
  auto* ident = rel->lhs()->AsIdentifier();
  EXPECT_EQ(ident->name(), "a");

  ASSERT_TRUE(rel->rhs()->IsConstructor());
  ASSERT_TRUE(rel->rhs()->AsConstructor()->IsScalarConstructor());
  auto* init = rel->rhs()->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(init->literal()->IsBool());
  ASSERT_TRUE(init->literal()->AsBool()->IsTrue());
}

TEST_F(ParserImplTest, ExclusiveOrExpression_InvalidLHS) {
  auto* p = parser("if (a) {} ^ true");
  auto e = p->exclusive_or_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_EQ(e, nullptr);
}

TEST_F(ParserImplTest, ExclusiveOrExpression_InvalidRHS) {
  auto* p = parser("true ^ if (a) {}");
  auto e = p->exclusive_or_expression();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:8: unable to parse right side of ^ expression");
}

TEST_F(ParserImplTest, ExclusiveOrExpression_NoOr_ReturnsLHS) {
  auto* p = parser("a true");
  auto e = p->exclusive_or_expression();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsIdentifier());
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
