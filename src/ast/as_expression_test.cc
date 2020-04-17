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

#include "src/ast/as_expression.h"

#include "gtest/gtest.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/type/f32_type.h"

namespace tint {
namespace ast {
namespace {

using AsExpressionTest = testing::Test;

TEST_F(AsExpressionTest, Create) {
  type::F32Type f32;
  auto expr = std::make_unique<IdentifierExpression>("expr");

  auto* expr_ptr = expr.get();

  AsExpression exp(&f32, std::move(expr));
  ASSERT_EQ(exp.type(), &f32);
  ASSERT_EQ(exp.expr(), expr_ptr);
}

TEST_F(AsExpressionTest, CreateWithSource) {
  type::F32Type f32;
  auto expr = std::make_unique<IdentifierExpression>("expr");

  AsExpression exp(Source{20, 2}, &f32, std::move(expr));
  auto src = exp.source();
  EXPECT_EQ(src.line, 20u);
  EXPECT_EQ(src.column, 2u);
}

TEST_F(AsExpressionTest, IsAs) {
  AsExpression exp;
  EXPECT_TRUE(exp.IsAs());
}

TEST_F(AsExpressionTest, IsValid) {
  type::F32Type f32;
  auto expr = std::make_unique<IdentifierExpression>("expr");

  AsExpression exp(&f32, std::move(expr));
  EXPECT_TRUE(exp.IsValid());
}

TEST_F(AsExpressionTest, IsValid_MissingType) {
  auto expr = std::make_unique<IdentifierExpression>("expr");

  AsExpression exp;
  exp.set_expr(std::move(expr));
  EXPECT_FALSE(exp.IsValid());
}

TEST_F(AsExpressionTest, IsValid_MissingExpr) {
  type::F32Type f32;

  AsExpression exp;
  exp.set_type(&f32);
  EXPECT_FALSE(exp.IsValid());
}

TEST_F(AsExpressionTest, IsValid_InvalidExpr) {
  type::F32Type f32;
  auto expr = std::make_unique<IdentifierExpression>("");
  AsExpression e(&f32, std::move(expr));
  EXPECT_FALSE(e.IsValid());
}

TEST_F(AsExpressionTest, ToStr) {
  type::F32Type f32;
  auto expr = std::make_unique<IdentifierExpression>("expr");

  AsExpression exp(&f32, std::move(expr));
  std::ostringstream out;
  exp.to_str(out, 2);

  EXPECT_EQ(out.str(), R"(  As<__f32>{
    Identifier{expr}
  }
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
