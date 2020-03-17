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

#include "src/ast/regardless_statement.h"

#include <sstream>

#include "gtest/gtest.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/if_statement.h"
#include "src/ast/kill_statement.h"

namespace tint {
namespace ast {

using RegardlessStatementTest = testing::Test;

TEST_F(RegardlessStatementTest, Creation) {
  auto ident = std::make_unique<IdentifierExpression>("ident");
  std::vector<std::unique_ptr<Statement>> stmts;
  stmts.push_back(std::make_unique<KillStatement>());

  auto ident_ptr = ident.get();
  auto kill_ptr = stmts[0].get();

  RegardlessStatement r(std::move(ident), std::move(stmts));
  EXPECT_EQ(r.condition(), ident_ptr);
  ASSERT_EQ(r.body().size(), 1);
  EXPECT_EQ(r.body()[0].get(), kill_ptr);
}

TEST_F(RegardlessStatementTest, Creation_WithSource) {
  auto ident = std::make_unique<IdentifierExpression>("ident");
  std::vector<std::unique_ptr<Statement>> stmts;
  stmts.push_back(std::make_unique<KillStatement>());

  RegardlessStatement r(Source{20, 2}, std::move(ident), std::move(stmts));
  auto src = r.source();
  EXPECT_EQ(src.line, 20);
  EXPECT_EQ(src.column, 2);
}

TEST_F(RegardlessStatementTest, IsRegardless) {
  RegardlessStatement r;
  EXPECT_TRUE(r.IsRegardless());
}

TEST_F(RegardlessStatementTest, IsValid) {
  auto ident = std::make_unique<IdentifierExpression>("ident");
  std::vector<std::unique_ptr<Statement>> stmts;
  stmts.push_back(std::make_unique<KillStatement>());

  RegardlessStatement r(std::move(ident), std::move(stmts));
  EXPECT_TRUE(r.IsValid());
}

TEST_F(RegardlessStatementTest, IsValid_NullCondition) {
  std::vector<std::unique_ptr<Statement>> stmts;
  stmts.push_back(std::make_unique<KillStatement>());

  RegardlessStatement r;
  r.set_body(std::move(stmts));
  EXPECT_FALSE(r.IsValid());
}

TEST_F(RegardlessStatementTest, IsValid_InvalidCondition) {
  auto ident = std::make_unique<IdentifierExpression>("");
  std::vector<std::unique_ptr<Statement>> stmts;
  stmts.push_back(std::make_unique<KillStatement>());

  RegardlessStatement r(std::move(ident), std::move(stmts));
  EXPECT_FALSE(r.IsValid());
}

TEST_F(RegardlessStatementTest, IsValid_NullBodyStatement) {
  auto ident = std::make_unique<IdentifierExpression>("ident");
  std::vector<std::unique_ptr<Statement>> stmts;
  stmts.push_back(std::make_unique<KillStatement>());
  stmts.push_back(nullptr);

  RegardlessStatement r;
  r.set_condition(std::move(ident));
  r.set_body(std::move(stmts));
  EXPECT_FALSE(r.IsValid());
}

TEST_F(RegardlessStatementTest, IsValid_InvalidBodyStatement) {
  auto ident = std::make_unique<IdentifierExpression>("ident");
  std::vector<std::unique_ptr<Statement>> stmts;
  stmts.push_back(std::make_unique<KillStatement>());
  stmts.push_back(std::make_unique<IfStatement>());

  RegardlessStatement r(std::move(ident), std::move(stmts));
  EXPECT_FALSE(r.IsValid());
}

TEST_F(RegardlessStatementTest, ToStr) {
  auto ident = std::make_unique<IdentifierExpression>("ident");
  std::vector<std::unique_ptr<Statement>> stmts;
  stmts.push_back(std::make_unique<KillStatement>());

  RegardlessStatement r(std::move(ident), std::move(stmts));
  std::ostringstream out;
  r.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Regardless{
    Identifier{ident}
    {
      Kill{}
    }
  }
)");
}

}  // namespace ast
}  // namespace tint
