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

#include "gtest/gtest.h"
#include "src/ast/bool_literal.h"
#include "src/ast/if_statement.h"
#include "src/ast/nop_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/type/bool_type.h"

namespace tint {
namespace ast {
namespace {

using ElseStatementTest = testing::Test;

TEST_F(ElseStatementTest, Creation) {
  ast::type::BoolType bool_type;
  auto cond = std::make_unique<ScalarConstructorExpression>(
      std::make_unique<BoolLiteral>(&bool_type, true));
  std::vector<std::unique_ptr<Statement>> body;
  body.push_back(std::make_unique<NopStatement>());

  auto cond_ptr = cond.get();
  auto nop_ptr = body[0].get();

  ElseStatement e(std::move(cond), std::move(body));
  EXPECT_EQ(e.condition(), cond_ptr);
  ASSERT_EQ(e.body().size(), 1);
  EXPECT_EQ(e.body()[0].get(), nop_ptr);
}

TEST_F(ElseStatementTest, Creation_WithSource) {
  ElseStatement e(Source{20, 2}, {});
  auto src = e.source();
  EXPECT_EQ(src.line, 20);
  EXPECT_EQ(src.column, 2);
}

TEST_F(ElseStatementTest, IsElse) {
  ElseStatement e;
  EXPECT_TRUE(e.IsElse());
}

TEST_F(ElseStatementTest, HasCondition) {
  ast::type::BoolType bool_type;
  auto cond = std::make_unique<ScalarConstructorExpression>(
      std::make_unique<BoolLiteral>(&bool_type, true));
  ElseStatement e(std::move(cond), {});
  EXPECT_TRUE(e.HasCondition());
}

TEST_F(ElseStatementTest, HasContition_NullCondition) {
  ElseStatement e;
  EXPECT_FALSE(e.HasCondition());
}

TEST_F(ElseStatementTest, IsValid) {
  ElseStatement e;
  EXPECT_TRUE(e.IsValid());
}

TEST_F(ElseStatementTest, IsValid_WithBody) {
  std::vector<std::unique_ptr<Statement>> body;
  body.push_back(std::make_unique<NopStatement>());

  ElseStatement e(std::move(body));
  EXPECT_TRUE(e.IsValid());
}

TEST_F(ElseStatementTest, IsValid_WithNullBodyStatement) {
  std::vector<std::unique_ptr<Statement>> body;
  body.push_back(std::make_unique<NopStatement>());
  body.push_back(nullptr);

  ElseStatement e(std::move(body));
  EXPECT_FALSE(e.IsValid());
}

TEST_F(ElseStatementTest, IsValid_InvalidCondition) {
  auto cond = std::make_unique<ScalarConstructorExpression>();
  ElseStatement e(std::move(cond), {});
  EXPECT_FALSE(e.IsValid());
}

TEST_F(ElseStatementTest, IsValid_InvalidBodyStatement) {
  std::vector<std::unique_ptr<Statement>> body;
  body.push_back(std::make_unique<IfStatement>());

  ElseStatement e(std::move(body));
  EXPECT_FALSE(e.IsValid());
}

TEST_F(ElseStatementTest, ToStr) {
  ast::type::BoolType bool_type;
  auto cond = std::make_unique<ScalarConstructorExpression>(
      std::make_unique<BoolLiteral>(&bool_type, true));
  std::vector<std::unique_ptr<Statement>> body;
  body.push_back(std::make_unique<NopStatement>());

  ElseStatement e(std::move(cond), std::move(body));
  std::ostringstream out;
  e.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Else{
    (
      ScalarConstructor{true}
    )
    {
      Nop{}
    }
  }
)");
}

TEST_F(ElseStatementTest, ToStr_NoCondition) {
  std::vector<std::unique_ptr<Statement>> body;
  body.push_back(std::make_unique<NopStatement>());

  ElseStatement e(std::move(body));
  std::ostringstream out;
  e.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Else{
    {
      Nop{}
    }
  }
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
