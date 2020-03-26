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

#include "src/ast/const_initializer_expression.h"

#include "gtest/gtest.h"
#include "src/ast/bool_literal.h"

namespace tint {
namespace ast {
namespace {

using ConstInitializerExpressionTest = testing::Test;

TEST_F(ConstInitializerExpressionTest, Creation) {
  auto b = std::make_unique<BoolLiteral>(true);
  auto b_ptr = b.get();
  ConstInitializerExpression c(std::move(b));
  EXPECT_EQ(c.literal(), b_ptr);
}

TEST_F(ConstInitializerExpressionTest, Creation_WithSource) {
  auto b = std::make_unique<BoolLiteral>(true);
  ConstInitializerExpression c(Source{20, 2}, std::move(b));
  auto src = c.source();
  EXPECT_EQ(src.line, 20);
  EXPECT_EQ(src.column, 2);
}

TEST_F(ConstInitializerExpressionTest, IsValid) {
  auto b = std::make_unique<BoolLiteral>(true);
  ConstInitializerExpression c(std::move(b));
  EXPECT_TRUE(c.IsValid());
}

TEST_F(ConstInitializerExpressionTest, IsValid_MissingLiteral) {
  ConstInitializerExpression c;
  EXPECT_FALSE(c.IsValid());
}

TEST_F(ConstInitializerExpressionTest, ToStr) {
  auto b = std::make_unique<BoolLiteral>(true);
  ConstInitializerExpression c(std::move(b));
  std::ostringstream out;
  c.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  ConstInitializer{true}
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
