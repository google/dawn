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

#include "src/ast/switch_statement.h"

#include <sstream>

#include "gtest/gtest.h"
#include "src/ast/case_statement.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/type/i32_type.h"

namespace tint {
namespace ast {
namespace {

using SwitchStatementTest = testing::Test;

TEST_F(SwitchStatementTest, Creation) {
  ast::type::I32Type i32;

  CaseSelectorList lit;
  lit.push_back(std::make_unique<SintLiteral>(&i32, 1));

  auto ident = std::make_unique<IdentifierExpression>("ident");
  CaseStatementList body;
  body.push_back(std::make_unique<CaseStatement>(
      std::move(lit), std::make_unique<ast::BlockStatement>()));

  auto* ident_ptr = ident.get();
  auto* case_ptr = body[0].get();

  SwitchStatement stmt(std::move(ident), std::move(body));
  EXPECT_EQ(stmt.condition(), ident_ptr);
  ASSERT_EQ(stmt.body().size(), 1u);
  EXPECT_EQ(stmt.body()[0].get(), case_ptr);
}

TEST_F(SwitchStatementTest, Creation_WithSource) {
  auto ident = std::make_unique<IdentifierExpression>("ident");

  SwitchStatement stmt(Source{20, 2}, std::move(ident), CaseStatementList());
  auto src = stmt.source();
  EXPECT_EQ(src.line, 20u);
  EXPECT_EQ(src.column, 2u);
}

TEST_F(SwitchStatementTest, IsSwitch) {
  SwitchStatement stmt;
  EXPECT_TRUE(stmt.IsSwitch());
}

TEST_F(SwitchStatementTest, IsValid) {
  ast::type::I32Type i32;

  CaseSelectorList lit;
  lit.push_back(std::make_unique<SintLiteral>(&i32, 2));

  auto ident = std::make_unique<IdentifierExpression>("ident");
  CaseStatementList body;
  body.push_back(std::make_unique<CaseStatement>(
      std::move(lit), std::make_unique<ast::BlockStatement>()));

  SwitchStatement stmt(std::move(ident), std::move(body));
  EXPECT_TRUE(stmt.IsValid());
}

TEST_F(SwitchStatementTest, IsValid_Null_Condition) {
  ast::type::I32Type i32;

  CaseSelectorList lit;
  lit.push_back(std::make_unique<SintLiteral>(&i32, 2));

  CaseStatementList body;
  body.push_back(std::make_unique<CaseStatement>(
      std::move(lit), std::make_unique<ast::BlockStatement>()));

  SwitchStatement stmt;
  stmt.set_body(std::move(body));
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(SwitchStatementTest, IsValid_Invalid_Condition) {
  ast::type::I32Type i32;

  CaseSelectorList lit;
  lit.push_back(std::make_unique<SintLiteral>(&i32, 2));

  auto ident = std::make_unique<IdentifierExpression>("");
  CaseStatementList body;
  body.push_back(std::make_unique<CaseStatement>(
      std::move(lit), std::make_unique<ast::BlockStatement>()));

  SwitchStatement stmt(std::move(ident), std::move(body));
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(SwitchStatementTest, IsValid_Null_BodyStatement) {
  ast::type::I32Type i32;

  CaseSelectorList lit;
  lit.push_back(std::make_unique<SintLiteral>(&i32, 2));

  auto ident = std::make_unique<IdentifierExpression>("ident");
  CaseStatementList body;
  body.push_back(std::make_unique<CaseStatement>(
      std::move(lit), std::make_unique<ast::BlockStatement>()));
  body.push_back(nullptr);

  SwitchStatement stmt(std::move(ident), std::move(body));
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(SwitchStatementTest, IsValid_Invalid_BodyStatement) {
  auto ident = std::make_unique<IdentifierExpression>("ident");

  auto case_body = std::make_unique<ast::BlockStatement>();
  case_body->append(nullptr);

  CaseStatementList body;
  body.push_back(std::make_unique<CaseStatement>(CaseSelectorList{},
                                                 std::move(case_body)));

  SwitchStatement stmt(std::move(ident), std::move(body));
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(SwitchStatementTest, ToStr_Empty) {
  auto ident = std::make_unique<IdentifierExpression>("ident");

  SwitchStatement stmt(std::move(ident), {});
  std::ostringstream out;
  stmt.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Switch{
    Identifier{ident}
    {
    }
  }
)");
}

TEST_F(SwitchStatementTest, ToStr) {
  ast::type::I32Type i32;

  CaseSelectorList lit;
  lit.push_back(std::make_unique<SintLiteral>(&i32, 2));

  auto ident = std::make_unique<IdentifierExpression>("ident");
  CaseStatementList body;
  body.push_back(std::make_unique<CaseStatement>(
      std::move(lit), std::make_unique<ast::BlockStatement>()));

  SwitchStatement stmt(std::move(ident), std::move(body));
  std::ostringstream out;
  stmt.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Switch{
    Identifier{ident}
    {
      Case 2{
      }
    }
  }
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
