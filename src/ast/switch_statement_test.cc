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
#include "src/ast/type/i32_type.h"

namespace tint {
namespace ast {
namespace {

using SwitchStatementTest = TestHelper;

TEST_F(SwitchStatementTest, Creation) {
  type::I32 i32;

  CaseSelectorList lit;
  lit.push_back(create<SintLiteral>(Source{}, &i32, 1));

  auto* ident = create<IdentifierExpression>(
      Source{}, mod.RegisterSymbol("ident"), "ident");
  CaseStatementList body;
  auto* case_stmt = create<CaseStatement>(
      Source{}, lit, create<BlockStatement>(Source{}, StatementList{}));
  body.push_back(case_stmt);

  SwitchStatement stmt(Source{}, ident, body);
  EXPECT_EQ(stmt.condition(), ident);
  ASSERT_EQ(stmt.body().size(), 1u);
  EXPECT_EQ(stmt.body()[0], case_stmt);
}

TEST_F(SwitchStatementTest, Creation_WithSource) {
  auto* ident = create<IdentifierExpression>(
      Source{}, mod.RegisterSymbol("ident"), "ident");

  SwitchStatement stmt(Source{Source::Location{20, 2}}, ident,
                       CaseStatementList());
  auto src = stmt.source();
  EXPECT_EQ(src.range.begin.line, 20u);
  EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(SwitchStatementTest, IsSwitch) {
  type::I32 i32;

  CaseSelectorList lit;
  lit.push_back(create<SintLiteral>(Source{}, &i32, 2));

  auto* ident = create<IdentifierExpression>(
      Source{}, mod.RegisterSymbol("ident"), "ident");
  CaseStatementList body;
  body.push_back(create<CaseStatement>(
      Source{}, lit, create<BlockStatement>(Source{}, StatementList{})));

  SwitchStatement stmt(Source{}, ident, body);
  EXPECT_TRUE(stmt.Is<SwitchStatement>());
}

TEST_F(SwitchStatementTest, IsValid) {
  type::I32 i32;

  CaseSelectorList lit;
  lit.push_back(create<SintLiteral>(Source{}, &i32, 2));

  auto* ident = create<IdentifierExpression>(
      Source{}, mod.RegisterSymbol("ident"), "ident");
  CaseStatementList body;
  body.push_back(create<CaseStatement>(
      Source{}, lit, create<BlockStatement>(Source{}, StatementList{})));

  SwitchStatement stmt(Source{}, ident, body);
  EXPECT_TRUE(stmt.IsValid());
}

TEST_F(SwitchStatementTest, IsValid_Null_Condition) {
  type::I32 i32;

  CaseSelectorList lit;
  lit.push_back(create<SintLiteral>(Source{}, &i32, 2));

  CaseStatementList body;
  body.push_back(create<CaseStatement>(
      Source{}, lit, create<BlockStatement>(Source{}, StatementList{})));

  SwitchStatement stmt(Source{}, nullptr, body);
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(SwitchStatementTest, IsValid_Invalid_Condition) {
  type::I32 i32;

  CaseSelectorList lit;
  lit.push_back(create<SintLiteral>(Source{}, &i32, 2));

  auto* ident =
      create<IdentifierExpression>(Source{}, mod.RegisterSymbol(""), "");
  CaseStatementList body;
  body.push_back(create<CaseStatement>(
      Source{}, lit, create<BlockStatement>(Source{}, StatementList{})));

  SwitchStatement stmt(Source{}, ident, body);
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(SwitchStatementTest, IsValid_Null_BodyStatement) {
  type::I32 i32;

  CaseSelectorList lit;
  lit.push_back(create<SintLiteral>(Source{}, &i32, 2));

  auto* ident = create<IdentifierExpression>(
      Source{}, mod.RegisterSymbol("ident"), "ident");
  CaseStatementList body;
  body.push_back(create<CaseStatement>(
      Source{}, lit, create<BlockStatement>(Source{}, StatementList{})));
  body.push_back(nullptr);

  SwitchStatement stmt(Source{}, ident, body);
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(SwitchStatementTest, IsValid_Invalid_BodyStatement) {
  auto* ident = create<IdentifierExpression>(
      Source{}, mod.RegisterSymbol("ident"), "ident");

  auto* case_body = create<BlockStatement>(Source{}, StatementList{
                                                         nullptr,
                                                     });
  CaseStatementList body;
  body.push_back(
      create<CaseStatement>(Source{}, CaseSelectorList{}, case_body));

  SwitchStatement stmt(Source{}, ident, body);
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(SwitchStatementTest, ToStr_Empty) {
  auto* ident = create<IdentifierExpression>(
      Source{}, mod.RegisterSymbol("ident"), "ident");

  SwitchStatement stmt(Source{}, ident, {});
  std::ostringstream out;
  stmt.to_str(out, 2);
  EXPECT_EQ(demangle(out.str()), R"(  Switch{
    Identifier[not set]{ident}
    {
    }
  }
)");
}

TEST_F(SwitchStatementTest, ToStr) {
  type::I32 i32;

  CaseSelectorList lit;
  lit.push_back(create<SintLiteral>(Source{}, &i32, 2));

  auto* ident = create<IdentifierExpression>(
      Source{}, mod.RegisterSymbol("ident"), "ident");
  CaseStatementList body;
  body.push_back(create<CaseStatement>(
      Source{}, lit, create<BlockStatement>(Source{}, StatementList{})));

  SwitchStatement stmt(Source{}, ident, body);
  std::ostringstream out;
  stmt.to_str(out, 2);
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
