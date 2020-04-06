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

#include "src/ast/case_statement.h"

#include "gtest/gtest.h"
#include "src/ast/bool_literal.h"
#include "src/ast/if_statement.h"
#include "src/ast/nop_statement.h"
#include "src/ast/type/bool_type.h"

namespace tint {
namespace ast {
namespace {

using CaseStatementTest = testing::Test;

TEST_F(CaseStatementTest, Creation) {
  ast::type::BoolType bool_type;
  auto b = std::make_unique<BoolLiteral>(&bool_type, true);
  StatementList stmts;
  stmts.push_back(std::make_unique<NopStatement>());

  auto bool_ptr = b.get();
  auto nop_ptr = stmts[0].get();

  CaseStatement c(std::move(b), std::move(stmts));
  EXPECT_EQ(c.condition(), bool_ptr);
  ASSERT_EQ(c.body().size(), 1);
  EXPECT_EQ(c.body()[0].get(), nop_ptr);
}

TEST_F(CaseStatementTest, Creation_WithSource) {
  ast::type::BoolType bool_type;
  auto b = std::make_unique<BoolLiteral>(&bool_type, true);
  StatementList stmts;
  stmts.push_back(std::make_unique<NopStatement>());

  CaseStatement c(Source{20, 2}, std::move(b), std::move(stmts));
  auto src = c.source();
  EXPECT_EQ(src.line, 20);
  EXPECT_EQ(src.column, 2);
}

TEST_F(CaseStatementTest, IsDefault_WithoutCondition) {
  StatementList stmts;
  stmts.push_back(std::make_unique<NopStatement>());

  CaseStatement c;
  c.set_body(std::move(stmts));
  EXPECT_TRUE(c.IsDefault());
}

TEST_F(CaseStatementTest, IsDefault_WithCondition) {
  ast::type::BoolType bool_type;
  auto b = std::make_unique<BoolLiteral>(&bool_type, true);
  CaseStatement c;
  c.set_condition(std::move(b));
  EXPECT_FALSE(c.IsDefault());
}

TEST_F(CaseStatementTest, IsCase) {
  CaseStatement c;
  EXPECT_TRUE(c.IsCase());
}

TEST_F(CaseStatementTest, IsValid) {
  CaseStatement c;
  EXPECT_TRUE(c.IsValid());
}

TEST_F(CaseStatementTest, IsValid_NullBodyStatement) {
  ast::type::BoolType bool_type;
  auto b = std::make_unique<BoolLiteral>(&bool_type, true);
  StatementList stmts;
  stmts.push_back(std::make_unique<NopStatement>());
  stmts.push_back(nullptr);

  CaseStatement c(std::move(b), std::move(stmts));
  EXPECT_FALSE(c.IsValid());
}

TEST_F(CaseStatementTest, IsValid_InvalidBodyStatement) {
  ast::type::BoolType bool_type;
  auto b = std::make_unique<BoolLiteral>(&bool_type, true);
  StatementList stmts;
  stmts.push_back(std::make_unique<IfStatement>());

  CaseStatement c(std::move(b), std::move(stmts));
  EXPECT_FALSE(c.IsValid());
}

TEST_F(CaseStatementTest, ToStr_WithCondition) {
  ast::type::BoolType bool_type;
  auto b = std::make_unique<BoolLiteral>(&bool_type, true);
  StatementList stmts;
  stmts.push_back(std::make_unique<NopStatement>());
  CaseStatement c(std::move(b), std::move(stmts));

  std::ostringstream out;
  c.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Case true{
    Nop{}
  }
)");
}

TEST_F(CaseStatementTest, ToStr_WithoutCondition) {
  StatementList stmts;
  stmts.push_back(std::make_unique<NopStatement>());
  CaseStatement c(nullptr, std::move(stmts));

  std::ostringstream out;
  c.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Default{
    Nop{}
  }
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
