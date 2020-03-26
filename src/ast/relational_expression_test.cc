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

#include "src/ast/relational_expression.h"

#include <sstream>

#include "gtest/gtest.h"
#include "src/ast/identifier_expression.h"

namespace tint {
namespace ast {
namespace {

using RelationalExpressionTest = testing::Test;

TEST_F(RelationalExpressionTest, Creation) {
  auto lhs = std::make_unique<IdentifierExpression>("lhs");
  auto rhs = std::make_unique<IdentifierExpression>("rhs");

  auto lhs_ptr = lhs.get();
  auto rhs_ptr = rhs.get();

  RelationalExpression r(Relation::kEqual, std::move(lhs), std::move(rhs));
  EXPECT_EQ(r.lhs(), lhs_ptr);
  EXPECT_EQ(r.rhs(), rhs_ptr);
  EXPECT_EQ(r.relation(), Relation::kEqual);
}

TEST_F(RelationalExpressionTest, Creation_WithSource) {
  auto lhs = std::make_unique<IdentifierExpression>("lhs");
  auto rhs = std::make_unique<IdentifierExpression>("rhs");

  RelationalExpression r(Source{20, 2}, Relation::kEqual, std::move(lhs),
                         std::move(rhs));
  auto src = r.source();
  EXPECT_EQ(src.line, 20);
  EXPECT_EQ(src.column, 2);
}

TEST_F(RelationalExpressionTest, IsRelational) {
  RelationalExpression r;
  EXPECT_TRUE(r.IsRelational());
}

TEST_F(RelationalExpressionTest, IsValid) {
  auto lhs = std::make_unique<IdentifierExpression>("lhs");
  auto rhs = std::make_unique<IdentifierExpression>("rhs");

  RelationalExpression r(Relation::kEqual, std::move(lhs), std::move(rhs));
  EXPECT_TRUE(r.IsValid());
}

TEST_F(RelationalExpressionTest, IsValid_Null_LHS) {
  auto rhs = std::make_unique<IdentifierExpression>("rhs");

  RelationalExpression r;
  r.set_relation(Relation::kEqual);
  r.set_rhs(std::move(rhs));
  EXPECT_FALSE(r.IsValid());
}

TEST_F(RelationalExpressionTest, IsValid_Invalid_LHS) {
  auto lhs = std::make_unique<IdentifierExpression>("");
  auto rhs = std::make_unique<IdentifierExpression>("rhs");

  RelationalExpression r(Relation::kEqual, std::move(lhs), std::move(rhs));
  EXPECT_FALSE(r.IsValid());
}

TEST_F(RelationalExpressionTest, IsValid_Null_RHS) {
  auto lhs = std::make_unique<IdentifierExpression>("lhs");

  RelationalExpression r;
  r.set_relation(Relation::kEqual);
  r.set_lhs(std::move(lhs));
  EXPECT_FALSE(r.IsValid());
}

TEST_F(RelationalExpressionTest, IsValid_Invalid_RHS) {
  auto lhs = std::make_unique<IdentifierExpression>("lhs");
  auto rhs = std::make_unique<IdentifierExpression>("");

  RelationalExpression r(Relation::kEqual, std::move(lhs), std::move(rhs));
  EXPECT_FALSE(r.IsValid());
}

TEST_F(RelationalExpressionTest, IsValid_Relation_None) {
  auto lhs = std::make_unique<IdentifierExpression>("lhs");
  auto rhs = std::make_unique<IdentifierExpression>("rhs");

  RelationalExpression r(Relation::kNone, std::move(lhs), std::move(rhs));
  EXPECT_FALSE(r.IsValid());
}

TEST_F(RelationalExpressionTest, ToStr) {
  auto lhs = std::make_unique<IdentifierExpression>("lhs");
  auto rhs = std::make_unique<IdentifierExpression>("rhs");

  RelationalExpression r(Relation::kEqual, std::move(lhs), std::move(rhs));
  std::ostringstream out;
  r.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Relation{
    Identifier{lhs}
    equal
    Identifier{rhs}
  }
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
