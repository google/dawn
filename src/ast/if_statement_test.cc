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

#include "gtest/gtest-spi.h"
#include "src/ast/discard_statement.h"
#include "src/ast/test_helper.h"

namespace tint {
namespace ast {
namespace {

using IfStatementTest = TestHelper;

TEST_F(IfStatementTest, Creation) {
  auto* cond = Expr("cond");
  auto* stmt = create<IfStatement>(Source{Source::Location{20, 2}}, cond,
                                   Block(create<DiscardStatement>()),
                                   ElseStatementList{});
  auto src = stmt->source();
  EXPECT_EQ(src.range.begin.line, 20u);
  EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(IfStatementTest, IsIf) {
  auto* stmt = create<IfStatement>(Expr(true), Block(), ElseStatementList{});
  EXPECT_TRUE(stmt->Is<IfStatement>());
}

TEST_F(IfStatementTest, Assert_Null_Condition) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b;
        b.create<IfStatement>(nullptr, b.Block(), ElseStatementList{});
      },
      "internal compiler error");
}

TEST_F(IfStatementTest, Assert_Null_Body) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b;
        b.create<IfStatement>(b.Expr(true), nullptr, ElseStatementList{});
      },
      "internal compiler error");
}

TEST_F(IfStatementTest, Assert_Null_ElseStatement) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b;
        auto* body = b.create<BlockStatement>(StatementList{});
        b.create<IfStatement>(b.Expr(true), body, ElseStatementList{nullptr});
      },
      "internal compiler error");
}

TEST_F(IfStatementTest, Assert_DifferentProgramID_Cond) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b1;
        ProgramBuilder b2;
        b1.create<IfStatement>(b2.Expr(true), b1.Block(), ElseStatementList{});
      },
      "internal compiler error");
}

TEST_F(IfStatementTest, Assert_DifferentProgramID_Body) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b1;
        ProgramBuilder b2;
        b1.create<IfStatement>(b1.Expr(true), b2.Block(), ElseStatementList{});
      },
      "internal compiler error");
}

TEST_F(IfStatementTest, Assert_DifferentProgramID_ElseStatement) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b1;
        ProgramBuilder b2;
        b1.create<IfStatement>(
            b1.Expr(true), b1.Block(),
            ElseStatementList{
                b2.create<ElseStatement>(b2.Expr("ident"), b2.Block()),
            });
      },
      "internal compiler error");
}

TEST_F(IfStatementTest, ToStr) {
  auto* cond = Expr("cond");
  auto* stmt = create<IfStatement>(cond, Block(create<DiscardStatement>()),
                                   ElseStatementList{});

  EXPECT_EQ(str(stmt), R"(If{
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
  auto* cond = Expr("cond");
  auto* body = Block(create<DiscardStatement>());
  auto* else_if_body = Block(create<DiscardStatement>());
  auto* else_body =
      Block(create<DiscardStatement>(), create<DiscardStatement>());
  auto* stmt = create<IfStatement>(
      cond, body,
      ElseStatementList{
          create<ElseStatement>(Expr("ident"), else_if_body),
          create<ElseStatement>(nullptr, else_body),
      });

  EXPECT_EQ(str(stmt), R"(If{
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
