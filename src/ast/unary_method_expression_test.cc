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

#include "src/ast/unary_method_expression.h"

#include <memory>
#include <sstream>

#include "gtest/gtest.h"
#include "src/ast/identifier_expression.h"

namespace tint {
namespace ast {
namespace {

using UnaryMethodExpressionTest = testing::Test;

TEST_F(UnaryMethodExpressionTest, Creation) {
  ExpressionList params;
  params.push_back(std::make_unique<IdentifierExpression>("ident"));

  auto* ident_ptr = params[0].get();

  UnaryMethodExpression u(UnaryMethod::kAll, std::move(params));
  EXPECT_EQ(u.op(), UnaryMethod::kAll);
  ASSERT_EQ(u.params().size(), 1u);
  EXPECT_EQ(u.params()[0].get(), ident_ptr);
}

TEST_F(UnaryMethodExpressionTest, Creation_WithSource) {
  ExpressionList params;
  params.push_back(std::make_unique<IdentifierExpression>("ident"));

  UnaryMethodExpression u(Source{20, 2}, UnaryMethod::kAll, std::move(params));
  auto src = u.source();
  EXPECT_EQ(src.line, 20u);
  EXPECT_EQ(src.column, 2u);
}

TEST_F(UnaryMethodExpressionTest, IsUnaryMethod) {
  UnaryMethodExpression u;
  EXPECT_TRUE(u.IsUnaryMethod());
}

TEST_F(UnaryMethodExpressionTest, IsValid) {
  ExpressionList params;
  params.push_back(std::make_unique<IdentifierExpression>("ident"));

  UnaryMethodExpression u(UnaryMethod::kAll, std::move(params));
  EXPECT_TRUE(u.IsValid());
}

TEST_F(UnaryMethodExpressionTest, IsValid_NullParam) {
  ExpressionList params;
  params.push_back(std::make_unique<IdentifierExpression>("ident"));
  params.push_back(nullptr);

  UnaryMethodExpression u(UnaryMethod::kAll, std::move(params));
  EXPECT_FALSE(u.IsValid());
}

TEST_F(UnaryMethodExpressionTest, IsValid_InvalidParam) {
  ExpressionList params;
  params.push_back(std::make_unique<IdentifierExpression>(""));

  UnaryMethodExpression u(UnaryMethod::kAll, std::move(params));
  EXPECT_FALSE(u.IsValid());
}

TEST_F(UnaryMethodExpressionTest, IsValid_EmptyParams) {
  UnaryMethodExpression u;
  u.set_op(UnaryMethod::kAll);
  EXPECT_FALSE(u.IsValid());
}

TEST_F(UnaryMethodExpressionTest, ToStr) {
  ExpressionList params;
  params.push_back(std::make_unique<IdentifierExpression>("ident"));

  UnaryMethodExpression u(UnaryMethod::kAll, std::move(params));
  std::ostringstream out;
  u.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  UnaryMethod{
    all
    Identifier{ident}
  }
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
