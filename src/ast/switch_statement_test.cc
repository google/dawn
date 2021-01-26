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

#include "src/ast/case_statement.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/test_helper.h"
#include "src/type/i32_type.h"

namespace tint {
namespace ast {
namespace {

using SwitchStatementTest = TestHelper;

TEST_F(SwitchStatementTest, Creation) {
  CaseSelectorList lit;
  lit.push_back(create<SintLiteral>(ty.i32(), 1));

  auto* ident = Expr("ident");
  CaseStatementList body;
  auto* case_stmt =
      create<CaseStatement>(lit, create<BlockStatement>(StatementList{}));
  body.push_back(case_stmt);

  auto* stmt = create<SwitchStatement>(ident, body);
  EXPECT_EQ(stmt->condition(), ident);
  ASSERT_EQ(stmt->body().size(), 1u);
  EXPECT_EQ(stmt->body()[0], case_stmt);
}

TEST_F(SwitchStatementTest, Creation_WithSource) {
  auto* ident = Expr("ident");

  auto* stmt = create<SwitchStatement>(Source{Source::Location{20, 2}}, ident,
                                       CaseStatementList());
  auto src = stmt->source();
  EXPECT_EQ(src.range.begin.line, 20u);
  EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(SwitchStatementTest, IsSwitch) {
  CaseSelectorList lit;
  lit.push_back(create<SintLiteral>(ty.i32(), 2));

  auto* ident = Expr("ident");
  CaseStatementList body;
  body.push_back(
      create<CaseStatement>(lit, create<BlockStatement>(StatementList{})));

  auto* stmt = create<SwitchStatement>(ident, body);
  EXPECT_TRUE(stmt->Is<SwitchStatement>());
}

TEST_F(SwitchStatementTest, IsValid) {
  CaseSelectorList lit;
  lit.push_back(create<SintLiteral>(ty.i32(), 2));

  auto* ident = Expr("ident");
  CaseStatementList body;
  body.push_back(
      create<CaseStatement>(lit, create<BlockStatement>(StatementList{})));

  auto* stmt = create<SwitchStatement>(ident, body);
  EXPECT_TRUE(stmt->IsValid());
}

TEST_F(SwitchStatementTest, IsValid_Null_Condition) {
  CaseSelectorList lit;
  lit.push_back(create<SintLiteral>(ty.i32(), 2));

  CaseStatementList body;
  body.push_back(
      create<CaseStatement>(lit, create<BlockStatement>(StatementList{})));

  auto* stmt = create<SwitchStatement>(nullptr, body);
  EXPECT_FALSE(stmt->IsValid());
}

TEST_F(SwitchStatementTest, IsValid_Invalid_Condition) {
  CaseSelectorList lit;
  lit.push_back(create<SintLiteral>(ty.i32(), 2));

  auto* ident = Expr("");
  CaseStatementList body;
  body.push_back(
      create<CaseStatement>(lit, create<BlockStatement>(StatementList{})));

  auto* stmt = create<SwitchStatement>(ident, body);
  EXPECT_FALSE(stmt->IsValid());
}

TEST_F(SwitchStatementTest, IsValid_Null_BodyStatement) {
  CaseSelectorList lit;
  lit.push_back(create<SintLiteral>(ty.i32(), 2));

  auto* ident = Expr("ident");
  CaseStatementList body;
  body.push_back(
      create<CaseStatement>(lit, create<BlockStatement>(StatementList{})));
  body.push_back(nullptr);

  auto* stmt = create<SwitchStatement>(ident, body);
  EXPECT_FALSE(stmt->IsValid());
}

TEST_F(SwitchStatementTest, IsValid_Invalid_BodyStatement) {
  auto* ident = Expr("ident");

  auto* case_body = create<BlockStatement>(StatementList{
      nullptr,
  });
  CaseStatementList body;
  body.push_back(create<CaseStatement>(CaseSelectorList{}, case_body));

  auto* stmt = create<SwitchStatement>(ident, body);
  EXPECT_FALSE(stmt->IsValid());
}

TEST_F(SwitchStatementTest, ToStr_Empty) {
  auto* ident = Expr("ident");

  auto* stmt = create<SwitchStatement>(ident, CaseStatementList{});
  std::ostringstream out;
  stmt->to_str(out, 2);
  EXPECT_EQ(demangle(out.str()), R"(  Switch{
    Identifier[not set]{ident}
    {
    }
  }
)");
}

TEST_F(SwitchStatementTest, ToStr) {
  CaseSelectorList lit;
  lit.push_back(create<SintLiteral>(ty.i32(), 2));

  auto* ident = Expr("ident");
  CaseStatementList body;
  body.push_back(
      create<CaseStatement>(lit, create<BlockStatement>(StatementList{})));

  auto* stmt = create<SwitchStatement>(ident, body);
  std::ostringstream out;
  stmt->to_str(out, 2);
  EXPECT_EQ(demangle(out.str()), R"(  Switch{
    Identifier[not set]{ident}
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
