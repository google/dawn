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

#include "src/ast/else_statement.h"

#include "src/ast/bool_literal.h"
#include "src/ast/discard_statement.h"
#include "src/ast/if_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/test_helper.h"
#include "src/ast/type/bool_type.h"

namespace tint {
namespace ast {
namespace {

using ElseStatementTest = TestHelper;

TEST_F(ElseStatementTest, Creation) {
  type::Bool bool_type;
  auto* cond = create<ScalarConstructorExpression>(
      Source{}, create<BoolLiteral>(Source{}, &bool_type, true));
  auto* body =
      create<BlockStatement>(Source{}, StatementList{
                                           create<DiscardStatement>(Source{}),
                                       });
  auto* discard = body->get(0);

  ElseStatement e(Source{}, cond, body);
  EXPECT_EQ(e.condition(), cond);
  ASSERT_EQ(e.body()->size(), 1u);
  EXPECT_EQ(e.body()->get(0), discard);
}

TEST_F(ElseStatementTest, Creation_WithSource) {
  ElseStatement e(Source{Source::Location{20, 2}}, nullptr,
                  create<BlockStatement>(Source{}, StatementList{}));
  auto src = e.source();
  EXPECT_EQ(src.range.begin.line, 20u);
  EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(ElseStatementTest, IsElse) {
  ElseStatement e(Source{}, nullptr,
                  create<BlockStatement>(Source{}, StatementList{}));
  EXPECT_TRUE(e.Is<ElseStatement>());
}

TEST_F(ElseStatementTest, HasCondition) {
  type::Bool bool_type;
  auto* cond = create<ScalarConstructorExpression>(
      Source{}, create<BoolLiteral>(Source{}, &bool_type, true));
  ElseStatement e(Source{}, cond,
                  create<BlockStatement>(Source{}, StatementList{}));
  EXPECT_TRUE(e.HasCondition());
}

TEST_F(ElseStatementTest, HasContition_NullCondition) {
  ElseStatement e(Source{}, nullptr,
                  create<BlockStatement>(Source{}, StatementList{}));
  EXPECT_FALSE(e.HasCondition());
}

TEST_F(ElseStatementTest, IsValid) {
  ElseStatement e(Source{}, nullptr,
                  create<BlockStatement>(Source{}, StatementList{}));
  EXPECT_TRUE(e.IsValid());
}

TEST_F(ElseStatementTest, IsValid_WithBody) {
  auto* body =
      create<BlockStatement>(Source{}, StatementList{
                                           create<DiscardStatement>(Source{}),
                                       });
  ElseStatement e(Source{}, nullptr, body);
  EXPECT_TRUE(e.IsValid());
}

TEST_F(ElseStatementTest, IsValid_WithNullBodyStatement) {
  auto* body =
      create<BlockStatement>(Source{}, StatementList{
                                           create<DiscardStatement>(Source{}),
                                           nullptr,
                                       });
  ElseStatement e(Source{}, nullptr, body);
  EXPECT_FALSE(e.IsValid());
}

TEST_F(ElseStatementTest, IsValid_InvalidCondition) {
  auto* cond = create<ScalarConstructorExpression>(Source{}, nullptr);
  ElseStatement e(Source{}, cond,
                  create<BlockStatement>(Source{}, StatementList{}));
  EXPECT_FALSE(e.IsValid());
}

TEST_F(ElseStatementTest, IsValid_InvalidBodyStatement) {
  auto* body = create<BlockStatement>(
      Source{},
      StatementList{
          create<IfStatement>(Source{}, nullptr,
                              create<BlockStatement>(Source{}, StatementList{}),
                              ElseStatementList{}),
      });
  ElseStatement e(Source{}, nullptr, body);
  EXPECT_FALSE(e.IsValid());
}

TEST_F(ElseStatementTest, ToStr) {
  type::Bool bool_type;
  auto* cond = create<ScalarConstructorExpression>(
      Source{}, create<BoolLiteral>(Source{}, &bool_type, true));
  auto* body =
      create<BlockStatement>(Source{}, StatementList{
                                           create<DiscardStatement>(Source{}),
                                       });
  ElseStatement e(Source{}, cond, body);
  std::ostringstream out;
  e.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Else{
    (
      ScalarConstructor[not set]{true}
    )
    {
      Discard{}
    }
  }
)");
}

TEST_F(ElseStatementTest, ToStr_NoCondition) {
  auto* body =
      create<BlockStatement>(Source{}, StatementList{
                                           create<DiscardStatement>(Source{}),
                                       });
  ElseStatement e(Source{}, nullptr, body);
  std::ostringstream out;
  e.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Else{
    {
      Discard{}
    }
  }
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
