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

#include "src/ast/discard_statement.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/test_helper.h"

namespace tint {
namespace ast {
namespace {

using IfStatementTest = TestHelper;

TEST_F(IfStatementTest, Creation) {
  auto* cond = create<IdentifierExpression>(Source{},
                                            mod.RegisterSymbol("cond"), "cond");
  auto* body = create<BlockStatement>(
      Source{}, StatementList{create<DiscardStatement>(Source{})});
  IfStatement stmt(Source{Source::Location{20, 2}}, cond, body,
                   ElseStatementList{});
  auto src = stmt.source();
  EXPECT_EQ(src.range.begin.line, 20u);
  EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(IfStatementTest, IsIf) {
  IfStatement stmt(Source{}, nullptr,
                   create<BlockStatement>(Source{}, StatementList{}),
                   ElseStatementList{});
  EXPECT_TRUE(stmt.Is<IfStatement>());
}

TEST_F(IfStatementTest, IsValid) {
  auto* cond = create<IdentifierExpression>(Source{},
                                            mod.RegisterSymbol("cond"), "cond");
  auto* body = create<BlockStatement>(
      Source{}, StatementList{create<DiscardStatement>(Source{})});
  IfStatement stmt(Source{}, cond, body, ElseStatementList{});
  EXPECT_TRUE(stmt.IsValid());
}

TEST_F(IfStatementTest, IsValid_WithElseStatements) {
  auto* cond = create<IdentifierExpression>(Source{},
                                            mod.RegisterSymbol("cond"), "cond");
  auto* body = create<BlockStatement>(
      Source{}, StatementList{create<DiscardStatement>(Source{})});
  IfStatement stmt(Source{}, cond, body,
                   {
                       create<ElseStatement>(
                           Source{},
                           create<IdentifierExpression>(
                               Source{}, mod.RegisterSymbol("Ident"), "Ident"),
                           create<BlockStatement>(Source{}, StatementList{})),
                       create<ElseStatement>(
                           Source{}, nullptr,
                           create<BlockStatement>(Source{}, StatementList{})),
                   });
  EXPECT_TRUE(stmt.IsValid());
}

TEST_F(IfStatementTest, IsValid_MissingCondition) {
  auto* body = create<BlockStatement>(
      Source{}, StatementList{create<DiscardStatement>(Source{})});
  IfStatement stmt(Source{}, nullptr, body, ElseStatementList{});
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(IfStatementTest, IsValid_InvalidCondition) {
  auto* cond =
      create<IdentifierExpression>(Source{}, mod.RegisterSymbol(""), "");
  auto* body = create<BlockStatement>(
      Source{}, StatementList{create<DiscardStatement>(Source{})});
  IfStatement stmt(Source{}, cond, body, ElseStatementList{});
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(IfStatementTest, IsValid_NullBodyStatement) {
  auto* cond = create<IdentifierExpression>(Source{},
                                            mod.RegisterSymbol("cond"), "cond");
  auto* body =
      create<BlockStatement>(Source{}, StatementList{
                                           create<DiscardStatement>(Source{}),
                                           nullptr,
                                       });
  IfStatement stmt(Source{}, cond, body, ElseStatementList{});
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(IfStatementTest, IsValid_InvalidBodyStatement) {
  auto* cond = create<IdentifierExpression>(Source{},
                                            mod.RegisterSymbol("cond"), "cond");
  auto* body = create<BlockStatement>(
      Source{},
      StatementList{
          create<DiscardStatement>(Source{}),
          create<IfStatement>(Source{}, nullptr,
                              create<BlockStatement>(Source{}, StatementList{}),
                              ast::ElseStatementList{}),
      });
  IfStatement stmt(Source{}, cond, body, ElseStatementList{});
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(IfStatementTest, IsValid_NullElseStatement) {
  auto* cond = create<IdentifierExpression>(Source{},
                                            mod.RegisterSymbol("cond"), "cond");
  auto* body = create<BlockStatement>(
      Source{}, StatementList{create<DiscardStatement>(Source{})});
  IfStatement stmt(Source{}, cond, body,
                   {
                       create<ElseStatement>(
                           Source{},
                           create<IdentifierExpression>(
                               Source{}, mod.RegisterSymbol("Ident"), "Ident"),
                           create<BlockStatement>(Source{}, StatementList{})),
                       create<ElseStatement>(
                           Source{}, nullptr,
                           create<BlockStatement>(Source{}, StatementList{})),
                       nullptr,
                   });
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(IfStatementTest, IsValid_InvalidElseStatement) {
  auto* cond = create<IdentifierExpression>(Source{},
                                            mod.RegisterSymbol("cond"), "cond");
  auto* body = create<BlockStatement>(
      Source{}, StatementList{create<DiscardStatement>(Source{})});
  IfStatement stmt(Source{}, cond, body,
                   {
                       create<ElseStatement>(
                           Source{},
                           create<IdentifierExpression>(
                               Source{}, mod.RegisterSymbol(""), ""),
                           create<BlockStatement>(Source{}, StatementList{})),
                   });
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(IfStatementTest, IsValid_MultipleElseWiththoutCondition) {
  auto* cond = create<IdentifierExpression>(Source{},
                                            mod.RegisterSymbol("cond"), "cond");
  auto* body = create<BlockStatement>(
      Source{}, StatementList{create<DiscardStatement>(Source{})});
  IfStatement stmt(Source{}, cond, body,
                   {
                       create<ElseStatement>(
                           Source{}, nullptr,
                           create<BlockStatement>(Source{}, StatementList{})),
                       create<ElseStatement>(
                           Source{}, nullptr,
                           create<BlockStatement>(Source{}, StatementList{})),
                   });
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(IfStatementTest, IsValid_ElseNotLast) {
  auto* cond = create<IdentifierExpression>(Source{},
                                            mod.RegisterSymbol("cond"), "cond");
  auto* body = create<BlockStatement>(
      Source{}, StatementList{create<DiscardStatement>(Source{})});
  IfStatement stmt(Source{}, cond, body,
                   {
                       create<ElseStatement>(
                           Source{}, nullptr,
                           create<BlockStatement>(Source{}, StatementList{})),
                       create<ElseStatement>(
                           Source{},
                           create<IdentifierExpression>(
                               Source{}, mod.RegisterSymbol("Ident"), "Ident"),
                           create<BlockStatement>(Source{}, StatementList{})),
                   });
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(IfStatementTest, ToStr) {
  auto* cond = create<IdentifierExpression>(Source{},
                                            mod.RegisterSymbol("cond"), "cond");
  auto* body = create<BlockStatement>(
      Source{}, StatementList{create<DiscardStatement>(Source{})});
  IfStatement stmt(Source{}, cond, body, ElseStatementList{});

  std::ostringstream out;
  stmt.to_str(out, 2);
  EXPECT_EQ(demangle(out.str()), R"(  If{
    (
      Identifier[not set]{cond}
    )
    {
      Discard{}
    }
  }
)");
}

TEST_F(IfStatementTest, ToStr_WithElseStatements) {
  auto* cond = create<IdentifierExpression>(Source{},
                                            mod.RegisterSymbol("cond"), "cond");
  auto* body = create<BlockStatement>(
      Source{}, StatementList{create<DiscardStatement>(Source{})});
  auto* else_if_body = create<BlockStatement>(
      Source{}, StatementList{create<DiscardStatement>(Source{})});
  auto* else_body = create<BlockStatement>(
      Source{}, StatementList{create<DiscardStatement>(Source{}),
                              create<DiscardStatement>(Source{})});
  IfStatement stmt(Source{}, cond, body,
                   {
                       create<ElseStatement>(
                           Source{},
                           create<IdentifierExpression>(
                               Source{}, mod.RegisterSymbol("ident"), "ident"),
                           else_if_body),
                       create<ElseStatement>(Source{}, nullptr, else_body),
                   });

  std::ostringstream out;
  stmt.to_str(out, 2);
  EXPECT_EQ(demangle(out.str()), R"(  If{
    (
      Identifier[not set]{cond}
    )
    {
      Discard{}
    }
  }
  Else{
    (
      Identifier[not set]{ident}
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
