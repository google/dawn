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

#include "src/ast/binary_expression.h"

#include <sstream>

#include "gtest/gtest.h"
#include "src/ast/identifier_expression.h"

namespace tint {
namespace ast {
namespace {

using BinaryExpressionTest = testing::Test;

TEST_F(BinaryExpressionTest, Creation) {
  auto lhs = std::make_unique<IdentifierExpression>("lhs");
  auto rhs = std::make_unique<IdentifierExpression>("rhs");

  auto* lhs_ptr = lhs.get();
  auto* rhs_ptr = rhs.get();

  BinaryExpression r(BinaryOp::kEqual, std::move(lhs), std::move(rhs));
  EXPECT_EQ(r.lhs(), lhs_ptr);
  EXPECT_EQ(r.rhs(), rhs_ptr);
  EXPECT_EQ(r.op(), BinaryOp::kEqual);
}

TEST_F(BinaryExpressionTest, Creation_WithSource) {
  auto lhs = std::make_unique<IdentifierExpression>("lhs");
  auto rhs = std::make_unique<IdentifierExpression>("rhs");

  BinaryExpression r(Source{20, 2}, BinaryOp::kEqual, std::move(lhs),
                     std::move(rhs));
  auto src = r.source();
  EXPECT_EQ(src.line, 20u);
  EXPECT_EQ(src.column, 2u);
}

TEST_F(BinaryExpressionTest, IsBinaryal) {
  BinaryExpression r;
  EXPECT_TRUE(r.IsBinary());
}

TEST_F(BinaryExpressionTest, IsValid) {
  auto lhs = std::make_unique<IdentifierExpression>("lhs");
  auto rhs = std::make_unique<IdentifierExpression>("rhs");

  BinaryExpression r(BinaryOp::kEqual, std::move(lhs), std::move(rhs));
  EXPECT_TRUE(r.IsValid());
}

TEST_F(BinaryExpressionTest, IsValid_Null_LHS) {
  auto rhs = std::make_unique<IdentifierExpression>("rhs");

  BinaryExpression r;
  r.set_op(BinaryOp::kEqual);
  r.set_rhs(std::move(rhs));
  EXPECT_FALSE(r.IsValid());
}

TEST_F(BinaryExpressionTest, IsValid_Invalid_LHS) {
  auto lhs = std::make_unique<IdentifierExpression>("");
  auto rhs = std::make_unique<IdentifierExpression>("rhs");

  BinaryExpression r(BinaryOp::kEqual, std::move(lhs), std::move(rhs));
  EXPECT_FALSE(r.IsValid());
}

TEST_F(BinaryExpressionTest, IsValid_Null_RHS) {
  auto lhs = std::make_unique<IdentifierExpression>("lhs");

  BinaryExpression r;
  r.set_op(BinaryOp::kEqual);
  r.set_lhs(std::move(lhs));
  EXPECT_FALSE(r.IsValid());
}

TEST_F(BinaryExpressionTest, IsValid_Invalid_RHS) {
  auto lhs = std::make_unique<IdentifierExpression>("lhs");
  auto rhs = std::make_unique<IdentifierExpression>("");

  BinaryExpression r(BinaryOp::kEqual, std::move(lhs), std::move(rhs));
  EXPECT_FALSE(r.IsValid());
}

TEST_F(BinaryExpressionTest, IsValid_Binary_None) {
  auto lhs = std::make_unique<IdentifierExpression>("lhs");
  auto rhs = std::make_unique<IdentifierExpression>("rhs");

  BinaryExpression r(BinaryOp::kNone, std::move(lhs), std::move(rhs));
  EXPECT_FALSE(r.IsValid());
}

TEST_F(BinaryExpressionTest, ToStr) {
  auto lhs = std::make_unique<IdentifierExpression>("lhs");
  auto rhs = std::make_unique<IdentifierExpression>("rhs");

  BinaryExpression r(BinaryOp::kEqual, std::move(lhs), std::move(rhs));
  std::ostringstream out;
  r.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Binary{
    Identifier{lhs}
    equal
    Identifier{rhs}
  }
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
