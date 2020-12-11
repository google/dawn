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

#include "src/ast/assignment_statement.h"

#include "src/ast/identifier_expression.h"
#include "src/ast/test_helper.h"

namespace tint {
namespace ast {
namespace {

using AssignmentStatementTest = TestHelper;

TEST_F(AssignmentStatementTest, Creation) {
  auto* lhs = create<IdentifierExpression>(mod.RegisterSymbol("lhs"), "lhs");
  auto* rhs = create<IdentifierExpression>(mod.RegisterSymbol("rhs"), "rhs");

  AssignmentStatement stmt(lhs, rhs);
  EXPECT_EQ(stmt.lhs(), lhs);
  EXPECT_EQ(stmt.rhs(), rhs);
}

TEST_F(AssignmentStatementTest, CreationWithSource) {
  auto* lhs = create<IdentifierExpression>(mod.RegisterSymbol("lhs"), "lhs");
  auto* rhs = create<IdentifierExpression>(mod.RegisterSymbol("rhs"), "rhs");

  AssignmentStatement stmt(Source{Source::Location{20, 2}}, lhs, rhs);
  auto src = stmt.source();
  EXPECT_EQ(src.range.begin.line, 20u);
  EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(AssignmentStatementTest, IsAssign) {
  auto* lhs = create<IdentifierExpression>(mod.RegisterSymbol("lhs"), "lhs");
  auto* rhs = create<IdentifierExpression>(mod.RegisterSymbol("rhs"), "rhs");

  AssignmentStatement stmt(lhs, rhs);
  EXPECT_TRUE(stmt.Is<AssignmentStatement>());
}

TEST_F(AssignmentStatementTest, IsValid) {
  auto* lhs = create<IdentifierExpression>(mod.RegisterSymbol("lhs"), "lhs");
  auto* rhs = create<IdentifierExpression>(mod.RegisterSymbol("rhs"), "rhs");

  AssignmentStatement stmt(lhs, rhs);
  EXPECT_TRUE(stmt.IsValid());
}

TEST_F(AssignmentStatementTest, IsValid_MissingLHS) {
  auto* rhs = create<IdentifierExpression>(mod.RegisterSymbol("rhs"), "rhs");

  AssignmentStatement stmt(nullptr, rhs);
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(AssignmentStatementTest, IsValid_MissingRHS) {
  auto* lhs = create<IdentifierExpression>(mod.RegisterSymbol("lhs"), "lhs");

  AssignmentStatement stmt(lhs, nullptr);
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(AssignmentStatementTest, IsValid_InvalidLHS) {
  auto* lhs = create<IdentifierExpression>(mod.RegisterSymbol(""), "");
  auto* rhs = create<IdentifierExpression>(mod.RegisterSymbol("rhs"), "rhs");
  AssignmentStatement stmt(lhs, rhs);
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(AssignmentStatementTest, IsValid_InvalidRHS) {
  auto* lhs = create<IdentifierExpression>(mod.RegisterSymbol("lhs"), "lhs");
  auto* rhs = create<IdentifierExpression>(mod.RegisterSymbol(""), "");
  AssignmentStatement stmt(lhs, rhs);
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(AssignmentStatementTest, ToStr) {
  auto* lhs = create<IdentifierExpression>(mod.RegisterSymbol("lhs"), "lhs");
  auto* rhs = create<IdentifierExpression>(mod.RegisterSymbol("rhs"), "rhs");

  AssignmentStatement stmt(lhs, rhs);
  std::ostringstream out;
  stmt.to_str(out, 2);

  EXPECT_EQ(demangle(out.str()), R"(  Assignment{
    Identifier[not set]{lhs}
    Identifier[not set]{rhs}
  }
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
