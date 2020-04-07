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

#include "src/type_determiner.h"

#include <memory>
#include <utility>

#include "gtest/gtest.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/break_statement.h"
#include "src/ast/case_statement.h"
#include "src/ast/continue_statement.h"
#include "src/ast/else_statement.h"
#include "src/ast/float_literal.h"
#include "src/ast/if_statement.h"
#include "src/ast/int_literal.h"
#include "src/ast/loop_statement.h"
#include "src/ast/regardless_statement.h"
#include "src/ast/return_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/switch_statement.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/unless_statement.h"

namespace tint {
namespace {

class TypeDeterminerTest : public testing::Test {
 public:
  void SetUp() { td_ = std::make_unique<TypeDeterminer>(&ctx_); }

  TypeDeterminer* td() const { return td_.get(); }

 private:
  Context ctx_;
  std::unique_ptr<TypeDeterminer> td_;
};

TEST_F(TypeDeterminerTest, Stmt_Assign) {
  ast::type::F32Type f32;
  ast::type::I32Type i32;

  auto lhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 2));
  auto lhs_ptr = lhs.get();

  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.3f));
  auto rhs_ptr = rhs.get();

  ast::AssignmentStatement assign(std::move(lhs), std::move(rhs));

  EXPECT_TRUE(td()->DetermineResultType(&assign));
  ASSERT_NE(lhs_ptr->result_type(), nullptr);
  ASSERT_NE(rhs_ptr->result_type(), nullptr);

  EXPECT_TRUE(lhs_ptr->result_type()->IsI32());
  EXPECT_TRUE(rhs_ptr->result_type()->IsF32());
}

TEST_F(TypeDeterminerTest, Stmt_Break) {
  ast::type::I32Type i32;

  auto cond = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 2));
  auto cond_ptr = cond.get();

  ast::BreakStatement brk(ast::StatementCondition::kIf, std::move(cond));

  EXPECT_TRUE(td()->DetermineResultType(&brk));
  ASSERT_NE(cond_ptr->result_type(), nullptr);
  EXPECT_TRUE(cond_ptr->result_type()->IsI32());
}

TEST_F(TypeDeterminerTest, Stmt_Case) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  auto lhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 2));
  auto lhs_ptr = lhs.get();

  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.3f));
  auto rhs_ptr = rhs.get();

  ast::StatementList body;
  body.push_back(std::make_unique<ast::AssignmentStatement>(std::move(lhs),
                                                            std::move(rhs)));

  ast::CaseStatement cse(std::make_unique<ast::IntLiteral>(&i32, 3),
                         std::move(body));

  EXPECT_TRUE(td()->DetermineResultType(&cse));
  ASSERT_NE(lhs_ptr->result_type(), nullptr);
  ASSERT_NE(rhs_ptr->result_type(), nullptr);
  EXPECT_TRUE(lhs_ptr->result_type()->IsI32());
  EXPECT_TRUE(rhs_ptr->result_type()->IsF32());
}

TEST_F(TypeDeterminerTest, Stmt_Continue) {
  ast::type::I32Type i32;

  auto cond = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 2));
  auto cond_ptr = cond.get();

  ast::ContinueStatement stmt(ast::StatementCondition::kIf, std::move(cond));

  EXPECT_TRUE(td()->DetermineResultType(&stmt));
  ASSERT_NE(cond_ptr->result_type(), nullptr);
  EXPECT_TRUE(cond_ptr->result_type()->IsI32());
}

TEST_F(TypeDeterminerTest, Stmt_Else) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  auto lhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 2));
  auto lhs_ptr = lhs.get();

  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.3f));
  auto rhs_ptr = rhs.get();

  ast::StatementList body;
  body.push_back(std::make_unique<ast::AssignmentStatement>(std::move(lhs),
                                                            std::move(rhs)));

  ast::ElseStatement stmt(std::make_unique<ast::ScalarConstructorExpression>(
                              std::make_unique<ast::IntLiteral>(&i32, 3)),
                          std::move(body));

  EXPECT_TRUE(td()->DetermineResultType(&stmt));
  ASSERT_NE(stmt.condition()->result_type(), nullptr);
  ASSERT_NE(lhs_ptr->result_type(), nullptr);
  ASSERT_NE(rhs_ptr->result_type(), nullptr);
  EXPECT_TRUE(stmt.condition()->result_type()->IsI32());
  EXPECT_TRUE(lhs_ptr->result_type()->IsI32());
  EXPECT_TRUE(rhs_ptr->result_type()->IsF32());
}

TEST_F(TypeDeterminerTest, Stmt_If) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  auto else_lhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 2));
  auto else_lhs_ptr = else_lhs.get();

  auto else_rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.3f));
  auto else_rhs_ptr = else_rhs.get();

  ast::StatementList else_body;
  else_body.push_back(std::make_unique<ast::AssignmentStatement>(
      std::move(else_lhs), std::move(else_rhs)));

  auto else_stmt = std::make_unique<ast::ElseStatement>(
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::IntLiteral>(&i32, 3)),
      std::move(else_body));

  ast::ElseStatementList else_stmts;
  else_stmts.push_back(std::move(else_stmt));

  auto lhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 2));
  auto lhs_ptr = lhs.get();

  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.3f));
  auto rhs_ptr = rhs.get();

  ast::StatementList body;
  body.push_back(std::make_unique<ast::AssignmentStatement>(std::move(lhs),
                                                            std::move(rhs)));

  ast::IfStatement stmt(std::make_unique<ast::ScalarConstructorExpression>(
                            std::make_unique<ast::IntLiteral>(&i32, 3)),
                        std::move(body));
  stmt.set_else_statements(std::move(else_stmts));

  EXPECT_TRUE(td()->DetermineResultType(&stmt));
  ASSERT_NE(stmt.condition()->result_type(), nullptr);
  ASSERT_NE(else_lhs_ptr->result_type(), nullptr);
  ASSERT_NE(else_rhs_ptr->result_type(), nullptr);
  ASSERT_NE(lhs_ptr->result_type(), nullptr);
  ASSERT_NE(rhs_ptr->result_type(), nullptr);
  EXPECT_TRUE(stmt.condition()->result_type()->IsI32());
  EXPECT_TRUE(else_lhs_ptr->result_type()->IsI32());
  EXPECT_TRUE(else_rhs_ptr->result_type()->IsF32());
  EXPECT_TRUE(lhs_ptr->result_type()->IsI32());
  EXPECT_TRUE(rhs_ptr->result_type()->IsF32());
}

TEST_F(TypeDeterminerTest, Stmt_Loop) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  auto body_lhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 2));
  auto body_lhs_ptr = body_lhs.get();

  auto body_rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.3f));
  auto body_rhs_ptr = body_rhs.get();

  ast::StatementList body;
  body.push_back(std::make_unique<ast::AssignmentStatement>(
      std::move(body_lhs), std::move(body_rhs)));

  auto continuing_lhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 2));
  auto continuing_lhs_ptr = continuing_lhs.get();

  auto continuing_rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.3f));
  auto continuing_rhs_ptr = continuing_rhs.get();

  ast::StatementList continuing;
  continuing.push_back(std::make_unique<ast::AssignmentStatement>(
      std::move(continuing_lhs), std::move(continuing_rhs)));

  ast::LoopStatement stmt(std::move(body), std::move(continuing));

  EXPECT_TRUE(td()->DetermineResultType(&stmt));
  ASSERT_NE(body_lhs_ptr->result_type(), nullptr);
  ASSERT_NE(body_rhs_ptr->result_type(), nullptr);
  ASSERT_NE(continuing_lhs_ptr->result_type(), nullptr);
  ASSERT_NE(continuing_rhs_ptr->result_type(), nullptr);
  EXPECT_TRUE(body_lhs_ptr->result_type()->IsI32());
  EXPECT_TRUE(body_rhs_ptr->result_type()->IsF32());
  EXPECT_TRUE(continuing_lhs_ptr->result_type()->IsI32());
  EXPECT_TRUE(continuing_rhs_ptr->result_type()->IsF32());
}

TEST_F(TypeDeterminerTest, Stmt_Regardless) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  auto lhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 2));
  auto lhs_ptr = lhs.get();

  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.3f));
  auto rhs_ptr = rhs.get();

  ast::StatementList body;
  body.push_back(std::make_unique<ast::AssignmentStatement>(std::move(lhs),
                                                            std::move(rhs)));

  ast::RegardlessStatement regardless(
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::IntLiteral>(&i32, 3)),
      std::move(body));

  EXPECT_TRUE(td()->DetermineResultType(&regardless));
  ASSERT_NE(regardless.condition()->result_type(), nullptr);
  ASSERT_NE(lhs_ptr->result_type(), nullptr);
  ASSERT_NE(rhs_ptr->result_type(), nullptr);
  EXPECT_TRUE(regardless.condition()->result_type()->IsI32());
  EXPECT_TRUE(lhs_ptr->result_type()->IsI32());
  EXPECT_TRUE(rhs_ptr->result_type()->IsF32());
}

TEST_F(TypeDeterminerTest, Stmt_Return) {
  ast::type::I32Type i32;

  auto cond = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 2));
  auto cond_ptr = cond.get();

  ast::ReturnStatement ret(std::move(cond));

  EXPECT_TRUE(td()->DetermineResultType(&ret));
  ASSERT_NE(cond_ptr->result_type(), nullptr);
  EXPECT_TRUE(cond_ptr->result_type()->IsI32());
}

TEST_F(TypeDeterminerTest, Stmt_Switch) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  auto lhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 2));
  auto lhs_ptr = lhs.get();

  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.3f));
  auto rhs_ptr = rhs.get();

  ast::StatementList body;
  body.push_back(std::make_unique<ast::AssignmentStatement>(std::move(lhs),
                                                            std::move(rhs)));

  ast::CaseStatementList cases;
  cases.push_back(std::make_unique<ast::CaseStatement>(
      std::make_unique<ast::IntLiteral>(&i32, 3), std::move(body)));

  ast::SwitchStatement stmt(std::make_unique<ast::ScalarConstructorExpression>(
                                std::make_unique<ast::IntLiteral>(&i32, 2)),
                            std::move(cases));

  EXPECT_TRUE(td()->DetermineResultType(&stmt)) << td()->error();
  ASSERT_NE(stmt.condition()->result_type(), nullptr);
  ASSERT_NE(lhs_ptr->result_type(), nullptr);
  ASSERT_NE(rhs_ptr->result_type(), nullptr);

  EXPECT_TRUE(stmt.condition()->result_type()->IsI32());
  EXPECT_TRUE(lhs_ptr->result_type()->IsI32());
  EXPECT_TRUE(rhs_ptr->result_type()->IsF32());
}

TEST_F(TypeDeterminerTest, Stmt_Unless) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  auto lhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 2));
  auto lhs_ptr = lhs.get();

  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.3f));
  auto rhs_ptr = rhs.get();

  ast::StatementList body;
  body.push_back(std::make_unique<ast::AssignmentStatement>(std::move(lhs),
                                                            std::move(rhs)));

  ast::UnlessStatement unless(
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::IntLiteral>(&i32, 3)),
      std::move(body));

  EXPECT_TRUE(td()->DetermineResultType(&unless));
  ASSERT_NE(unless.condition()->result_type(), nullptr);
  ASSERT_NE(lhs_ptr->result_type(), nullptr);
  ASSERT_NE(rhs_ptr->result_type(), nullptr);
  EXPECT_TRUE(unless.condition()->result_type()->IsI32());
  EXPECT_TRUE(lhs_ptr->result_type()->IsI32());
  EXPECT_TRUE(rhs_ptr->result_type()->IsF32());
}

TEST_F(TypeDeterminerTest, Expr_Constructor_Scalar) {
  ast::type::F32Type f32;
  ast::ScalarConstructorExpression s(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f));

  EXPECT_TRUE(td()->DetermineResultType(&s));
  ASSERT_NE(s.result_type(), nullptr);
  EXPECT_TRUE(s.result_type()->IsF32());
}

TEST_F(TypeDeterminerTest, Expr_Constructor_Type) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  ast::TypeConstructorExpression tc(&vec, std::move(vals));

  EXPECT_TRUE(td()->DetermineResultType(&tc));
  ASSERT_NE(tc.result_type(), nullptr);
  ASSERT_TRUE(tc.result_type()->IsVector());
  EXPECT_TRUE(tc.result_type()->AsVector()->type()->IsF32());
  EXPECT_EQ(tc.result_type()->AsVector()->size(), 3);
}

}  // namespace
}  // namespace tint
