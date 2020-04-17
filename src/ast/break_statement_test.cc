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

#include "src/ast/break_statement.h"

#include "gtest/gtest.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/statement_condition.h"

namespace tint {
namespace ast {
namespace {

using BreakStatementTest = testing::Test;

TEST_F(BreakStatementTest, Creation) {
  BreakStatement stmt;
  EXPECT_EQ(stmt.condition(), StatementCondition::kNone);
  EXPECT_EQ(stmt.conditional(), nullptr);
}

TEST_F(BreakStatementTest, CreationWithConditional) {
  auto expr = std::make_unique<IdentifierExpression>("expr");
  auto* expr_ptr = expr.get();

  BreakStatement stmt(StatementCondition::kIf, std::move(expr));
  EXPECT_EQ(stmt.condition(), StatementCondition::kIf);
  EXPECT_EQ(stmt.conditional(), expr_ptr);
}

TEST_F(BreakStatementTest, Creation_WithSource) {
  BreakStatement stmt(Source{20, 2});
  auto src = stmt.source();
  EXPECT_EQ(src.line, 20u);
  EXPECT_EQ(src.column, 2u);
}

TEST_F(BreakStatementTest, Creation_WithSourceAndCondition) {
  auto expr = std::make_unique<IdentifierExpression>("expr");

  BreakStatement stmt(Source{20, 2}, StatementCondition::kUnless,
                      std::move(expr));
  auto src = stmt.source();
  EXPECT_EQ(src.line, 20u);
  EXPECT_EQ(src.column, 2u);
}

TEST_F(BreakStatementTest, IsBreak) {
  BreakStatement stmt;
  EXPECT_TRUE(stmt.IsBreak());
}

TEST_F(BreakStatementTest, IsValid_WithoutCondition) {
  BreakStatement stmt;
  EXPECT_TRUE(stmt.IsValid());
}

TEST_F(BreakStatementTest, IsValid_WithCondition) {
  auto expr = std::make_unique<IdentifierExpression>("expr");
  BreakStatement stmt(StatementCondition::kIf, std::move(expr));
  EXPECT_TRUE(stmt.IsValid());
}

TEST_F(BreakStatementTest, IsValid_WithConditionAndConditionNone) {
  auto expr = std::make_unique<IdentifierExpression>("expr");
  BreakStatement stmt(StatementCondition::kNone, std::move(expr));
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(BreakStatementTest, IsValid_WithCondition_MissingConditional) {
  BreakStatement stmt;
  stmt.set_condition(StatementCondition::kIf);
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(BreakStatementTest, IsValid_InvalidConditional) {
  auto expr = std::make_unique<IdentifierExpression>("");
  BreakStatement stmt(StatementCondition::kIf, std::move(expr));
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(BreakStatementTest, ToStr_WithoutCondition) {
  BreakStatement stmt;
  std::ostringstream out;
  stmt.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Break{}
)");
}

TEST_F(BreakStatementTest, ToStr_WithCondition) {
  auto expr = std::make_unique<IdentifierExpression>("expr");
  BreakStatement stmt(StatementCondition::kUnless, std::move(expr));
  std::ostringstream out;
  stmt.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Break{
    unless
    Identifier{expr}
  }
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
