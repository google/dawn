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

#include "src/ast/unary_op_expression.h"

#include <sstream>

#include "gtest/gtest.h"
#include "src/ast/identifier_expression.h"

namespace tint {
namespace ast {
namespace {

using UnaryOpExpressionTest = testing::Test;

TEST_F(UnaryOpExpressionTest, Creation) {
  auto ident = std::make_unique<IdentifierExpression>("ident");
  auto* ident_ptr = ident.get();

  UnaryOpExpression u(UnaryOp::kNot, std::move(ident));
  EXPECT_EQ(u.op(), UnaryOp::kNot);
  EXPECT_EQ(u.expr(), ident_ptr);
}

TEST_F(UnaryOpExpressionTest, Creation_WithSource) {
  auto ident = std::make_unique<IdentifierExpression>("ident");
  UnaryOpExpression u(Source{20, 2}, UnaryOp::kNot, std::move(ident));
  auto src = u.source();
  EXPECT_EQ(src.line, 20u);
  EXPECT_EQ(src.column, 2u);
}

TEST_F(UnaryOpExpressionTest, IsUnaryOp) {
  UnaryOpExpression u;
  EXPECT_TRUE(u.IsUnaryOp());
}

TEST_F(UnaryOpExpressionTest, IsValid) {
  auto ident = std::make_unique<IdentifierExpression>("ident");
  UnaryOpExpression u(UnaryOp::kNot, std::move(ident));
  EXPECT_TRUE(u.IsValid());
}

TEST_F(UnaryOpExpressionTest, IsValid_NullExpression) {
  UnaryOpExpression u;
  u.set_op(UnaryOp::kNot);
  EXPECT_FALSE(u.IsValid());
}

TEST_F(UnaryOpExpressionTest, IsValid_InvalidExpression) {
  auto ident = std::make_unique<IdentifierExpression>("");
  UnaryOpExpression u(UnaryOp::kNot, std::move(ident));
  EXPECT_FALSE(u.IsValid());
}

TEST_F(UnaryOpExpressionTest, ToStr) {
  auto ident = std::make_unique<IdentifierExpression>("ident");
  UnaryOpExpression u(UnaryOp::kNot, std::move(ident));
  std::ostringstream out;
  u.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  UnaryOp{
    not
    Identifier{ident}
  }
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
