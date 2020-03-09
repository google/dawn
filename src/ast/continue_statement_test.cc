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

#include "src/ast/continue_statement.h"

#include "gtest/gtest.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/statement_condition.h"

namespace tint {
namespace ast {

using ContinueStatementTest = testing::Test;

TEST_F(ContinueStatementTest, Creation) {
  ContinueStatement stmt;
  EXPECT_EQ(stmt.condition(), StatementCondition::kNone);
  EXPECT_EQ(stmt.conditional(), nullptr);
}

TEST_F(ContinueStatementTest, CreationWithConditional) {
  auto expr = std::make_unique<IdentifierExpression>("expr");
  auto expr_ptr = expr.get();

  ContinueStatement stmt(StatementCondition::kIf, std::move(expr));
  EXPECT_EQ(stmt.condition(), StatementCondition::kIf);
  EXPECT_EQ(stmt.conditional(), expr_ptr);
}

TEST_F(ContinueStatementTest, Creation_WithSource) {
  ContinueStatement stmt(Source{20, 2});
  auto src = stmt.source();
  EXPECT_EQ(src.line, 20);
  EXPECT_EQ(src.column, 2);
}

TEST_F(ContinueStatementTest, Creation_WithSourceAndCondition) {
  auto expr = std::make_unique<IdentifierExpression>("expr");

  ContinueStatement stmt(Source{20, 2}, StatementCondition::kUnless,
                         std::move(expr));
  auto src = stmt.source();
  EXPECT_EQ(src.line, 20);
  EXPECT_EQ(src.column, 2);
}

TEST_F(ContinueStatementTest, IsContinue) {
  ContinueStatement stmt;
  EXPECT_TRUE(stmt.IsContinue());
}

TEST_F(ContinueStatementTest, IsValid_WithoutCondition) {
  ContinueStatement stmt;
  EXPECT_TRUE(stmt.IsValid());
}

TEST_F(ContinueStatementTest, IsValid_WithCondition) {
  auto expr = std::make_unique<IdentifierExpression>("expr");
  ContinueStatement stmt(StatementCondition::kIf, std::move(expr));
  EXPECT_TRUE(stmt.IsValid());
}

TEST_F(ContinueStatementTest, IsValid_InvalidConditional) {
  auto expr = std::make_unique<IdentifierExpression>("");
  ContinueStatement stmt(StatementCondition::kIf, std::move(expr));
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(ContinueStatementTest, IsValid_NoneConditionWithConditional) {
  auto expr = std::make_unique<IdentifierExpression>("expr");
  ContinueStatement stmt(StatementCondition::kNone, std::move(expr));
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(ContinueStatementTest, IsValid_WithCondition_MissingConditional) {
  ContinueStatement stmt;
  stmt.set_condition(StatementCondition::kIf);
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(ContinueStatementTest, ToStr_WithoutCondition) {
  ContinueStatement stmt;
  std::ostringstream out;
  stmt.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Continue{}
)");
}

TEST_F(ContinueStatementTest, ToStr_WithCondition) {
  auto expr = std::make_unique<IdentifierExpression>("expr");
  ContinueStatement stmt(StatementCondition::kUnless, std::move(expr));
  std::ostringstream out;
  stmt.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Continue{
    unless
    Identifier{expr}
  }
)");
}

}  // namespace ast
}  // namespace tint
