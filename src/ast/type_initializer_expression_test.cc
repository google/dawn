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

#include "src/ast/type_initializer_expression.h"

#include <sstream>

#include "gtest/gtest.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/vector_type.h"

namespace tint {
namespace ast {
namespace {

using TypeInitializerExpressionTest = testing::Test;

TEST_F(TypeInitializerExpressionTest, Creation) {
  type::F32Type f32;
  std::vector<std::unique_ptr<Expression>> expr;
  expr.push_back(std::make_unique<IdentifierExpression>("expr"));
  auto expr_ptr = expr[0].get();

  TypeInitializerExpression t(&f32, std::move(expr));
  EXPECT_EQ(t.type(), &f32);
  ASSERT_EQ(t.values().size(), 1);
  EXPECT_EQ(t.values()[0].get(), expr_ptr);
}

TEST_F(TypeInitializerExpressionTest, Creation_WithSource) {
  type::F32Type f32;
  std::vector<std::unique_ptr<Expression>> expr;
  expr.push_back(std::make_unique<IdentifierExpression>("expr"));

  TypeInitializerExpression t(Source{20, 2}, &f32, std::move(expr));
  auto src = t.source();
  EXPECT_EQ(src.line, 20);
  EXPECT_EQ(src.column, 2);
}

TEST_F(TypeInitializerExpressionTest, IsTypeInitializer) {
  TypeInitializerExpression t;
  EXPECT_TRUE(t.IsTypeInitializer());
}

TEST_F(TypeInitializerExpressionTest, IsValid) {
  type::F32Type f32;
  std::vector<std::unique_ptr<Expression>> expr;
  expr.push_back(std::make_unique<IdentifierExpression>("expr"));

  TypeInitializerExpression t(&f32, std::move(expr));
  EXPECT_TRUE(t.IsValid());
}

TEST_F(TypeInitializerExpressionTest, IsValid_NullType) {
  std::vector<std::unique_ptr<Expression>> expr;
  expr.push_back(std::make_unique<IdentifierExpression>("expr"));

  TypeInitializerExpression t;
  t.set_values(std::move(expr));
  EXPECT_FALSE(t.IsValid());
}

TEST_F(TypeInitializerExpressionTest, IsValid_NullValue) {
  type::F32Type f32;
  std::vector<std::unique_ptr<Expression>> expr;
  expr.push_back(std::make_unique<IdentifierExpression>("expr"));
  expr.push_back(nullptr);

  TypeInitializerExpression t(&f32, std::move(expr));
  EXPECT_FALSE(t.IsValid());
}

TEST_F(TypeInitializerExpressionTest, IsValid_InvalidValue) {
  type::F32Type f32;
  std::vector<std::unique_ptr<Expression>> expr;
  expr.push_back(std::make_unique<IdentifierExpression>(""));

  TypeInitializerExpression t(&f32, std::move(expr));
  EXPECT_FALSE(t.IsValid());
}

TEST_F(TypeInitializerExpressionTest, IsValid_EmptyValue) {
  type::F32Type f32;
  std::vector<std::unique_ptr<Expression>> expr;

  TypeInitializerExpression t(&f32, std::move(expr));
  EXPECT_FALSE(t.IsValid());
}

TEST_F(TypeInitializerExpressionTest, ToStr) {
  type::F32Type f32;
  type::VectorType vec(&f32, 3);
  std::vector<std::unique_ptr<Expression>> expr;
  expr.push_back(std::make_unique<IdentifierExpression>("expr_1"));
  expr.push_back(std::make_unique<IdentifierExpression>("expr_2"));
  expr.push_back(std::make_unique<IdentifierExpression>("expr_3"));

  TypeInitializerExpression t(&vec, std::move(expr));
  std::ostringstream out;
  t.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  TypeInitializer{
    __vec_3__f32
    Identifier{expr_1}
    Identifier{expr_2}
    Identifier{expr_3}
  }
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
