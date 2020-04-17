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

#include "src/ast/assignment_statement.h"

#include "gtest/gtest.h"
#include "src/ast/identifier_expression.h"

namespace tint {
namespace ast {
namespace {

using AssignmentStatementTest = testing::Test;

TEST_F(AssignmentStatementTest, Creation) {
  auto lhs = std::make_unique<ast::IdentifierExpression>("lhs");
  auto rhs = std::make_unique<ast::IdentifierExpression>("rhs");

  auto* lhs_ptr = lhs.get();
  auto* rhs_ptr = rhs.get();

  AssignmentStatement stmt(std::move(lhs), std::move(rhs));
  EXPECT_EQ(stmt.lhs(), lhs_ptr);
  EXPECT_EQ(stmt.rhs(), rhs_ptr);
}

TEST_F(AssignmentStatementTest, CreationWithSource) {
  auto lhs = std::make_unique<ast::IdentifierExpression>("lhs");
  auto rhs = std::make_unique<ast::IdentifierExpression>("rhs");

  AssignmentStatement stmt(Source{20, 2}, std::move(lhs), std::move(rhs));
  auto src = stmt.source();
  EXPECT_EQ(src.line, 20u);
  EXPECT_EQ(src.column, 2u);
}

TEST_F(AssignmentStatementTest, IsAssign) {
  auto lhs = std::make_unique<ast::IdentifierExpression>("lhs");
  auto rhs = std::make_unique<ast::IdentifierExpression>("rhs");

  AssignmentStatement stmt(std::move(lhs), std::move(rhs));
  EXPECT_TRUE(stmt.IsAssign());
}

TEST_F(AssignmentStatementTest, IsValid) {
  auto lhs = std::make_unique<ast::IdentifierExpression>("lhs");
  auto rhs = std::make_unique<ast::IdentifierExpression>("rhs");

  AssignmentStatement stmt(std::move(lhs), std::move(rhs));
  EXPECT_TRUE(stmt.IsValid());
}

TEST_F(AssignmentStatementTest, IsValid_MissingLHS) {
  auto rhs = std::make_unique<ast::IdentifierExpression>("rhs");

  AssignmentStatement stmt;
  stmt.set_rhs(std::move(rhs));
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(AssignmentStatementTest, IsValid_MissingRHS) {
  auto lhs = std::make_unique<ast::IdentifierExpression>("lhs");

  AssignmentStatement stmt;
  stmt.set_lhs(std::move(lhs));
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(AssignmentStatementTest, IsValid_InvalidLHS) {
  auto lhs = std::make_unique<ast::IdentifierExpression>("");
  auto rhs = std::make_unique<ast::IdentifierExpression>("rhs");
  AssignmentStatement stmt(std::move(lhs), std::move(rhs));
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(AssignmentStatementTest, IsValid_InvalidRHS) {
  auto lhs = std::make_unique<ast::IdentifierExpression>("lhs");
  auto rhs = std::make_unique<ast::IdentifierExpression>("");
  AssignmentStatement stmt(std::move(lhs), std::move(rhs));
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(AssignmentStatementTest, ToStr) {
  auto lhs = std::make_unique<ast::IdentifierExpression>("lhs");
  auto rhs = std::make_unique<ast::IdentifierExpression>("rhs");

  AssignmentStatement stmt(std::move(lhs), std::move(rhs));
  std::ostringstream out;
  stmt.to_str(out, 2);

  EXPECT_EQ(out.str(), R"(  Assignment{
    Identifier{lhs}
    Identifier{rhs}
  }
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
