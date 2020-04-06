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

#include "src/ast/unless_statement.h"

#include "gtest/gtest.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/if_statement.h"
#include "src/ast/nop_statement.h"

namespace tint {
namespace ast {
namespace {

using UnlessStatementTest = testing::Test;

TEST_F(UnlessStatementTest, Creation) {
  auto ident = std::make_unique<IdentifierExpression>("ident");
  StatementList body;
  body.push_back(std::make_unique<NopStatement>());

  auto ident_ptr = ident.get();
  auto nop_ptr = body[0].get();

  UnlessStatement u(std::move(ident), std::move(body));
  EXPECT_EQ(u.condition(), ident_ptr);
  ASSERT_EQ(u.body().size(), 1);
  EXPECT_EQ(u.body()[0].get(), nop_ptr);
}

TEST_F(UnlessStatementTest, Creation_WithSource) {
  auto ident = std::make_unique<IdentifierExpression>("ident");
  StatementList body;
  body.push_back(std::make_unique<NopStatement>());

  UnlessStatement u(Source{20, 2}, std::move(ident), std::move(body));
  auto src = u.source();
  EXPECT_EQ(src.line, 20);
  EXPECT_EQ(src.column, 2);
}

TEST_F(UnlessStatementTest, IsUnless) {
  UnlessStatement stmt;
  EXPECT_TRUE(stmt.IsUnless());
}

TEST_F(UnlessStatementTest, IsValid) {
  auto ident = std::make_unique<IdentifierExpression>("ident");
  StatementList body;
  body.push_back(std::make_unique<NopStatement>());

  UnlessStatement u(std::move(ident), std::move(body));
  EXPECT_TRUE(u.IsValid());
}

TEST_F(UnlessStatementTest, IsValid_NullCondition) {
  StatementList body;
  body.push_back(std::make_unique<NopStatement>());

  UnlessStatement u;
  u.set_body(std::move(body));
  EXPECT_FALSE(u.IsValid());
}

TEST_F(UnlessStatementTest, IsValid_InvalidCondition) {
  auto ident = std::make_unique<IdentifierExpression>("");
  StatementList body;
  body.push_back(std::make_unique<NopStatement>());

  UnlessStatement u(std::move(ident), std::move(body));
  EXPECT_FALSE(u.IsValid());
}

TEST_F(UnlessStatementTest, IsValid_NullBodyStatement) {
  auto ident = std::make_unique<IdentifierExpression>("ident");
  StatementList body;
  body.push_back(std::make_unique<NopStatement>());
  body.push_back(nullptr);

  UnlessStatement u(std::move(ident), std::move(body));
  EXPECT_FALSE(u.IsValid());
}

TEST_F(UnlessStatementTest, IsValid_InvalidBodyStatement) {
  auto ident = std::make_unique<IdentifierExpression>("ident");
  StatementList body;
  body.push_back(std::make_unique<NopStatement>());
  body.push_back(std::make_unique<IfStatement>());

  UnlessStatement u(std::move(ident), std::move(body));
  EXPECT_FALSE(u.IsValid());
}

TEST_F(UnlessStatementTest, ToStr) {
  auto ident = std::make_unique<IdentifierExpression>("ident");
  StatementList body;
  body.push_back(std::make_unique<NopStatement>());

  UnlessStatement u(std::move(ident), std::move(body));
  std::ostringstream out;
  u.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Unless{
    (
      Identifier{ident}
    )
    {
      Nop{}
    }
  }
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
