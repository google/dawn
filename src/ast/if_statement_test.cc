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

#include "src/ast/if_statement.h"

#include "gtest/gtest.h"
#include "src/ast/discard_statement.h"
#include "src/ast/identifier_expression.h"

namespace tint {
namespace ast {
namespace {

using IfStatementTest = testing::Test;

TEST_F(IfStatementTest, Creation) {
  auto cond = std::make_unique<IdentifierExpression>("cond");
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<DiscardStatement>());

  auto* cond_ptr = cond.get();
  auto* stmt_ptr = body->get(0);

  IfStatement stmt(std::move(cond), std::move(body));
  EXPECT_EQ(stmt.condition(), cond_ptr);
  ASSERT_EQ(stmt.body()->size(), 1u);
  EXPECT_EQ(stmt.body()->get(0), stmt_ptr);
}

TEST_F(IfStatementTest, Creation_WithSource) {
  auto cond = std::make_unique<IdentifierExpression>("cond");
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<DiscardStatement>());

  IfStatement stmt(Source{20, 2}, std::move(cond), std::move(body));
  auto src = stmt.source();
  EXPECT_EQ(src.line, 20u);
  EXPECT_EQ(src.column, 2u);
}

TEST_F(IfStatementTest, IsIf) {
  IfStatement stmt;
  EXPECT_TRUE(stmt.IsIf());
}

TEST_F(IfStatementTest, IsValid) {
  auto cond = std::make_unique<IdentifierExpression>("cond");
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<DiscardStatement>());

  IfStatement stmt(std::move(cond), std::move(body));
  EXPECT_TRUE(stmt.IsValid());
}

TEST_F(IfStatementTest, IsValid_WithElseStatements) {
  auto cond = std::make_unique<IdentifierExpression>("cond");
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<DiscardStatement>());

  ElseStatementList else_stmts;
  else_stmts.push_back(std::make_unique<ElseStatement>());
  else_stmts[0]->set_condition(std::make_unique<IdentifierExpression>("Ident"));
  else_stmts.push_back(std::make_unique<ElseStatement>());

  IfStatement stmt(std::move(cond), std::move(body));
  stmt.set_else_statements(std::move(else_stmts));
  EXPECT_TRUE(stmt.IsValid());
}

TEST_F(IfStatementTest, IsValid_MissingCondition) {
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<DiscardStatement>());

  IfStatement stmt(nullptr, std::move(body));
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(IfStatementTest, IsValid_InvalidCondition) {
  auto cond = std::make_unique<IdentifierExpression>("");
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<DiscardStatement>());

  IfStatement stmt(std::move(cond), std::move(body));
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(IfStatementTest, IsValid_NullBodyStatement) {
  auto cond = std::make_unique<IdentifierExpression>("cond");
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<DiscardStatement>());
  body->append(nullptr);

  IfStatement stmt(std::move(cond), std::move(body));
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(IfStatementTest, IsValid_InvalidBodyStatement) {
  auto cond = std::make_unique<IdentifierExpression>("cond");
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<DiscardStatement>());
  body->append(std::make_unique<IfStatement>());

  IfStatement stmt(std::move(cond), std::move(body));
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(IfStatementTest, IsValid_NullElseStatement) {
  auto cond = std::make_unique<IdentifierExpression>("cond");
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<DiscardStatement>());

  ElseStatementList else_stmts;
  else_stmts.push_back(std::make_unique<ElseStatement>());
  else_stmts[0]->set_condition(std::make_unique<IdentifierExpression>("Ident"));
  else_stmts.push_back(std::make_unique<ElseStatement>());
  else_stmts.push_back(nullptr);

  IfStatement stmt(std::move(cond), std::move(body));
  stmt.set_else_statements(std::move(else_stmts));
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(IfStatementTest, IsValid_InvalidElseStatement) {
  auto cond = std::make_unique<IdentifierExpression>("cond");
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<DiscardStatement>());

  ElseStatementList else_stmts;
  else_stmts.push_back(std::make_unique<ElseStatement>());
  else_stmts[0]->set_condition(std::make_unique<IdentifierExpression>(""));

  IfStatement stmt(std::move(cond), std::move(body));
  stmt.set_else_statements(std::move(else_stmts));
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(IfStatementTest, IsValid_MultipleElseWiththoutCondition) {
  auto cond = std::make_unique<IdentifierExpression>("cond");
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<DiscardStatement>());

  ElseStatementList else_stmts;
  else_stmts.push_back(std::make_unique<ElseStatement>());
  else_stmts.push_back(std::make_unique<ElseStatement>());

  IfStatement stmt(std::move(cond), std::move(body));
  stmt.set_else_statements(std::move(else_stmts));
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(IfStatementTest, IsValid_ElseNotLast) {
  auto cond = std::make_unique<IdentifierExpression>("cond");
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<DiscardStatement>());

  ElseStatementList else_stmts;
  else_stmts.push_back(std::make_unique<ElseStatement>());
  else_stmts.push_back(std::make_unique<ElseStatement>());
  else_stmts[1]->set_condition(std::make_unique<IdentifierExpression>("ident"));

  IfStatement stmt(std::move(cond), std::move(body));
  stmt.set_else_statements(std::move(else_stmts));
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(IfStatementTest, ToStr) {
  auto cond = std::make_unique<IdentifierExpression>("cond");
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<DiscardStatement>());

  IfStatement stmt(std::move(cond), std::move(body));

  std::ostringstream out;
  stmt.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  If{
    (
      Identifier{cond}
    )
    {
      Discard{}
    }
  }
)");
}

TEST_F(IfStatementTest, ToStr_WithElseStatements) {
  auto cond = std::make_unique<IdentifierExpression>("cond");
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<DiscardStatement>());

  auto else_if_body = std::make_unique<BlockStatement>();
  else_if_body->append(std::make_unique<DiscardStatement>());

  auto else_body = std::make_unique<BlockStatement>();
  else_body->append(std::make_unique<DiscardStatement>());
  else_body->append(std::make_unique<DiscardStatement>());

  ElseStatementList else_stmts;
  else_stmts.push_back(std::make_unique<ElseStatement>());
  else_stmts[0]->set_condition(std::make_unique<IdentifierExpression>("ident"));
  else_stmts[0]->set_body(std::move(else_if_body));
  else_stmts.push_back(std::make_unique<ElseStatement>());
  else_stmts[1]->set_body(std::move(else_body));

  IfStatement stmt(std::move(cond), std::move(body));
  stmt.set_else_statements(std::move(else_stmts));

  std::ostringstream out;
  stmt.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  If{
    (
      Identifier{cond}
    )
    {
      Discard{}
    }
  }
  Else{
    (
      Identifier{ident}
    )
    {
      Discard{}
    }
  }
  Else{
    {
      Discard{}
      Discard{}
    }
  }
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
