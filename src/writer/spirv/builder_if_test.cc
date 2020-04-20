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

#include <memory>

#include "gtest/gtest.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/bool_literal.h"
#include "src/ast/else_statement.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/if_statement.h"
#include "src/ast/int_literal.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/i32_type.h"
#include "src/context.h"
#include "src/type_determiner.h"
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = testing::Test;

TEST_F(BuilderTest, If_Empty) {
  ast::type::BoolType bool_type;

  // if (true) {
  // }
  auto cond = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::BoolLiteral>(&bool_type, true));

  ast::IfStatement expr(std::move(cond), ast::StatementList{});

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});

  EXPECT_TRUE(b.GenerateIfStatement(&expr)) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeBool
%2 = OpConstantTrue %1
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(OpSelectionMerge %3 None
OpBranchConditional %2 %4 %3
%4 = OpLabel
OpBranch %3
%3 = OpLabel
)");
}

TEST_F(BuilderTest, If_WithStatements) {
  ast::type::BoolType bool_type;
  ast::type::I32Type i32;

  // if (true) {
  //   v = 2;
  // }
  auto var =
      std::make_unique<ast::Variable>("v", ast::StorageClass::kPrivate, &i32);

  ast::StatementList body;
  body.push_back(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("v"),
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::IntLiteral>(&i32, 2))));

  auto cond = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::BoolLiteral>(&bool_type, true));

  ast::IfStatement expr(std::move(cond), std::move(body));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();

  EXPECT_TRUE(b.GenerateIfStatement(&expr)) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%4 = OpTypeBool
%5 = OpConstantTrue %4
%8 = OpConstant %3 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(OpSelectionMerge %6 None
OpBranchConditional %5 %7 %6
%7 = OpLabel
OpStore %1 %8
OpBranch %6
%6 = OpLabel
)");
}

TEST_F(BuilderTest, If_WithElse) {
  ast::type::BoolType bool_type;
  ast::type::I32Type i32;

  // if (true) {
  //   v = 2;
  // } else {
  //   v = 3;
  // }
  auto var =
      std::make_unique<ast::Variable>("v", ast::StorageClass::kPrivate, &i32);

  ast::StatementList body;
  body.push_back(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("v"),
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::IntLiteral>(&i32, 2))));

  ast::StatementList else_body;
  else_body.push_back(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("v"),
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::IntLiteral>(&i32, 3))));

  ast::ElseStatementList else_stmts;
  else_stmts.push_back(
      std::make_unique<ast::ElseStatement>(std::move(else_body)));

  auto cond = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::BoolLiteral>(&bool_type, true));

  ast::IfStatement expr(std::move(cond), std::move(body));
  expr.set_else_statements(std::move(else_stmts));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();

  EXPECT_TRUE(b.GenerateIfStatement(&expr)) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%4 = OpTypeBool
%5 = OpConstantTrue %4
%9 = OpConstant %3 2
%10 = OpConstant %3 3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(OpSelectionMerge %6 None
OpBranchConditional %5 %7 %8
%7 = OpLabel
OpStore %1 %9
OpBranch %6
%8 = OpLabel
OpStore %1 %10
OpBranch %6
%6 = OpLabel
)");
}

TEST_F(BuilderTest, If_WithElseIf) {
  ast::type::BoolType bool_type;
  ast::type::I32Type i32;

  // if (true) {
  //   v = 2;
  // } elseif (true) {
  //   v = 3;
  // }
  auto var =
      std::make_unique<ast::Variable>("v", ast::StorageClass::kPrivate, &i32);

  ast::StatementList body;
  body.push_back(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("v"),
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::IntLiteral>(&i32, 2))));

  ast::StatementList else_body;
  else_body.push_back(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("v"),
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::IntLiteral>(&i32, 3))));

  auto else_cond = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::BoolLiteral>(&bool_type, true));

  ast::ElseStatementList else_stmts;
  else_stmts.push_back(std::make_unique<ast::ElseStatement>(
      std::move(else_cond), std::move(else_body)));

  auto cond = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::BoolLiteral>(&bool_type, true));

  ast::IfStatement expr(std::move(cond), std::move(body));
  expr.set_else_statements(std::move(else_stmts));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();

  EXPECT_TRUE(b.GenerateIfStatement(&expr)) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%4 = OpTypeBool
%5 = OpConstantTrue %4
%9 = OpConstant %3 2
%12 = OpConstant %3 3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(OpSelectionMerge %6 None
OpBranchConditional %5 %7 %8
%7 = OpLabel
OpStore %1 %9
OpBranch %6
%8 = OpLabel
OpSelectionMerge %10 None
OpBranchConditional %5 %11 %10
%11 = OpLabel
OpStore %1 %12
OpBranch %10
%10 = OpLabel
OpBranch %6
%6 = OpLabel
)");
}

TEST_F(BuilderTest, If_WithMultiple) {
  ast::type::BoolType bool_type;
  ast::type::I32Type i32;

  // if (true) {
  //   v = 2;
  // } elseif (true) {
  //   v = 3;
  // } elseif (false) {
  //   v = 4;
  // } else {
  //   v = 5;
  // }
  auto var =
      std::make_unique<ast::Variable>("v", ast::StorageClass::kPrivate, &i32);

  ast::StatementList body;
  body.push_back(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("v"),
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::IntLiteral>(&i32, 2))));
  ast::StatementList elseif_1_body;
  elseif_1_body.push_back(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("v"),
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::IntLiteral>(&i32, 3))));
  ast::StatementList elseif_2_body;
  elseif_2_body.push_back(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("v"),
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::IntLiteral>(&i32, 4))));
  ast::StatementList else_body;
  else_body.push_back(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("v"),
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::IntLiteral>(&i32, 5))));

  auto elseif_1_cond = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::BoolLiteral>(&bool_type, true));
  auto elseif_2_cond = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::BoolLiteral>(&bool_type, false));

  ast::ElseStatementList else_stmts;
  else_stmts.push_back(std::make_unique<ast::ElseStatement>(
      std::move(elseif_1_cond), std::move(elseif_1_body)));
  else_stmts.push_back(std::make_unique<ast::ElseStatement>(
      std::move(elseif_2_cond), std::move(elseif_2_body)));
  else_stmts.push_back(
      std::make_unique<ast::ElseStatement>(std::move(else_body)));

  auto cond = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::BoolLiteral>(&bool_type, true));

  ast::IfStatement expr(std::move(cond), std::move(body));
  expr.set_else_statements(std::move(else_stmts));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();

  EXPECT_TRUE(b.GenerateIfStatement(&expr)) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%4 = OpTypeBool
%5 = OpConstantTrue %4
%9 = OpConstant %3 2
%13 = OpConstant %3 3
%14 = OpConstantFalse %4
%18 = OpConstant %3 4
%19 = OpConstant %3 5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(OpSelectionMerge %6 None
OpBranchConditional %5 %7 %8
%7 = OpLabel
OpStore %1 %9
OpBranch %6
%8 = OpLabel
OpSelectionMerge %10 None
OpBranchConditional %5 %11 %12
%11 = OpLabel
OpStore %1 %13
OpBranch %10
%12 = OpLabel
OpSelectionMerge %15 None
OpBranchConditional %14 %16 %17
%16 = OpLabel
OpStore %1 %18
OpBranch %15
%17 = OpLabel
OpStore %1 %19
OpBranch %15
%15 = OpLabel
OpBranch %10
%10 = OpLabel
OpBranch %6
%6 = OpLabel
)");
}

TEST_F(BuilderTest, DISABLED_If_WithBreak) {
  // if (a) {
  //   break;
  // }
}

TEST_F(BuilderTest, DISABLED_If_WithContinue) {
  // if (a) {
  //   continue;
  // }
}

TEST_F(BuilderTest, DISABLED_IF_WithReturn) {
  // if (a) {
  //   return;
  // }
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
