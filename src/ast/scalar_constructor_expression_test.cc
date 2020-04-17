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

#include "src/ast/scalar_constructor_expression.h"

#include "gtest/gtest.h"
#include "src/ast/bool_literal.h"
#include "src/ast/type/bool_type.h"

namespace tint {
namespace ast {
namespace {

using ScalarConstructorExpressionTest = testing::Test;

TEST_F(ScalarConstructorExpressionTest, Creation) {
  ast::type::BoolType bool_type;
  auto b = std::make_unique<BoolLiteral>(&bool_type, true);
  auto* b_ptr = b.get();
  ScalarConstructorExpression c(std::move(b));
  EXPECT_EQ(c.literal(), b_ptr);
}

TEST_F(ScalarConstructorExpressionTest, Creation_WithSource) {
  ast::type::BoolType bool_type;
  auto b = std::make_unique<BoolLiteral>(&bool_type, true);
  ScalarConstructorExpression c(Source{20, 2}, std::move(b));
  auto src = c.source();
  EXPECT_EQ(src.line, 20u);
  EXPECT_EQ(src.column, 2u);
}

TEST_F(ScalarConstructorExpressionTest, IsValid) {
  ast::type::BoolType bool_type;
  auto b = std::make_unique<BoolLiteral>(&bool_type, true);
  ScalarConstructorExpression c(std::move(b));
  EXPECT_TRUE(c.IsValid());
}

TEST_F(ScalarConstructorExpressionTest, IsValid_MissingLiteral) {
  ScalarConstructorExpression c;
  EXPECT_FALSE(c.IsValid());
}

TEST_F(ScalarConstructorExpressionTest, ToStr) {
  ast::type::BoolType bool_type;
  auto b = std::make_unique<BoolLiteral>(&bool_type, true);
  ScalarConstructorExpression c(std::move(b));
  std::ostringstream out;
  c.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  ScalarConstructor{true}
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
