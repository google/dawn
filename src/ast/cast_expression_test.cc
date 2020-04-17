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

#include "src/ast/cast_expression.h"

#include "gtest/gtest.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/type/f32_type.h"

namespace tint {
namespace ast {
namespace {

using CastExpressionTest = testing::Test;

TEST_F(CastExpressionTest, Creation) {
  type::F32Type f32;
  auto expr = std::make_unique<IdentifierExpression>("expr");
  auto* expr_ptr = expr.get();

  CastExpression c(&f32, std::move(expr));
  EXPECT_EQ(c.type(), &f32);
  EXPECT_EQ(c.expr(), expr_ptr);
}

TEST_F(CastExpressionTest, Creation_withSource) {
  type::F32Type f32;
  auto expr = std::make_unique<IdentifierExpression>("expr");

  CastExpression c(Source{20, 2}, &f32, std::move(expr));
  auto src = c.source();
  EXPECT_EQ(src.line, 20u);
  EXPECT_EQ(src.column, 2u);
}

TEST_F(CastExpressionTest, IsCast) {
  CastExpression c;
  EXPECT_TRUE(c.IsCast());
}

TEST_F(CastExpressionTest, IsValid) {
  type::F32Type f32;
  auto expr = std::make_unique<IdentifierExpression>("expr");

  CastExpression c(&f32, std::move(expr));
  EXPECT_TRUE(c.IsValid());
}

TEST_F(CastExpressionTest, IsValid_MissingType) {
  auto expr = std::make_unique<IdentifierExpression>("expr");

  CastExpression c;
  c.set_expr(std::move(expr));
  EXPECT_FALSE(c.IsValid());
}

TEST_F(CastExpressionTest, IsValid_MissingExpression) {
  type::F32Type f32;

  CastExpression c;
  c.set_type(&f32);
  EXPECT_FALSE(c.IsValid());
}

TEST_F(CastExpressionTest, IsValid_InvalidExpression) {
  type::F32Type f32;
  auto expr = std::make_unique<IdentifierExpression>("");
  CastExpression c(&f32, std::move(expr));
  EXPECT_FALSE(c.IsValid());
}

TEST_F(CastExpressionTest, ToStr) {
  type::F32Type f32;
  auto expr = std::make_unique<IdentifierExpression>("expr");

  CastExpression c(Source{20, 2}, &f32, std::move(expr));
  std::ostringstream out;
  c.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Cast<__f32>(
    Identifier{expr}
  )
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
