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

#include "src/ast/return_statement.h"

#include <sstream>

#include "gtest/gtest.h"
#include "src/ast/identifier_expression.h"

namespace tint {
namespace ast {
namespace {

using ReturnStatementTest = testing::Test;

TEST_F(ReturnStatementTest, Creation) {
  auto expr = std::make_unique<IdentifierExpression>("expr");
  auto* expr_ptr = expr.get();

  ReturnStatement r(std::move(expr));
  EXPECT_EQ(r.value(), expr_ptr);
}

TEST_F(ReturnStatementTest, Creation_WithSource) {
  ReturnStatement r(Source{20, 2});
  auto src = r.source();
  EXPECT_EQ(src.line, 20u);
  EXPECT_EQ(src.column, 2u);
}

TEST_F(ReturnStatementTest, IsReturn) {
  ReturnStatement r;
  EXPECT_TRUE(r.IsReturn());
}

TEST_F(ReturnStatementTest, HasValue_WithoutValue) {
  ReturnStatement r;
  EXPECT_FALSE(r.has_value());
}

TEST_F(ReturnStatementTest, HasValue_WithValue) {
  auto expr = std::make_unique<IdentifierExpression>("expr");
  ReturnStatement r(std::move(expr));
  EXPECT_TRUE(r.has_value());
}

TEST_F(ReturnStatementTest, IsValid_WithoutValue) {
  ReturnStatement r;
  EXPECT_TRUE(r.IsValid());
}

TEST_F(ReturnStatementTest, IsValid_WithValue) {
  auto expr = std::make_unique<IdentifierExpression>("expr");
  ReturnStatement r(std::move(expr));
  EXPECT_TRUE(r.IsValid());
}

TEST_F(ReturnStatementTest, IsValid_InvalidValue) {
  auto expr = std::make_unique<IdentifierExpression>("");
  ReturnStatement r(std::move(expr));
  EXPECT_FALSE(r.IsValid());
}

TEST_F(ReturnStatementTest, ToStr_WithValue) {
  auto expr = std::make_unique<IdentifierExpression>("expr");
  ReturnStatement r(std::move(expr));
  std::ostringstream out;
  r.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Return{
    {
      Identifier{expr}
    }
  }
)");
}

TEST_F(ReturnStatementTest, ToStr_WithoutValue) {
  ReturnStatement r;
  std::ostringstream out;
  r.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Return{}
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
