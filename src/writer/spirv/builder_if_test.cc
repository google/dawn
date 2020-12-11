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
#include "src/ast/break_statement.h"
#include "src/ast/continue_statement.h"
#include "src/ast/else_statement.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/if_statement.h"
#include "src/ast/loop_statement.h"
#include "src/ast/return_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/i32_type.h"
#include "src/type_determiner.h"
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"
#include "src/writer/spirv/test_helper.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = TestHelper;

TEST_F(BuilderTest, If_Empty) {
  ast::type::Bool bool_type;

  // if (true) {
  // }
  auto* cond = create<ast::ScalarConstructorExpression>(
      create<ast::BoolLiteral>(&bool_type, true));

  ast::IfStatement expr(Source{}, cond, create<ast::BlockStatement>(),
                        ast::ElseStatementList{});

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

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
  ast::type::Bool bool_type;
  ast::type::I32 i32;

  // if (true) {
  //   v = 2;
  // }
  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "v",                             // name
                            ast::StorageClass::kPrivate,     // storage_class
                            &i32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations

  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("v"), "v"),
      create<ast::ScalarConstructorExpression>(
          create<ast::SintLiteral>(&i32, 2))));

  auto* cond = create<ast::ScalarConstructorExpression>(
      create<ast::BoolLiteral>(&bool_type, true));

  ast::IfStatement expr(Source{}, cond, body, ast::ElseStatementList{});

  td.RegisterVariableForTesting(var);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_TRUE(b.GenerateIfStatement(&expr)) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%5 = OpTypeBool
%6 = OpConstantTrue %5
%9 = OpConstant %3 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(OpSelectionMerge %7 None
OpBranchConditional %6 %8 %7
%8 = OpLabel
OpStore %1 %9
OpBranch %7
%7 = OpLabel
)");
}

TEST_F(BuilderTest, If_WithElse) {
  ast::type::Bool bool_type;
  ast::type::I32 i32;

  // if (true) {
  //   v = 2;
  // } else {
  //   v = 3;
  // }
  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "v",                             // name
                            ast::StorageClass::kPrivate,     // storage_class
                            &i32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations

  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("v"), "v"),
      create<ast::ScalarConstructorExpression>(
          create<ast::SintLiteral>(&i32, 2))));

  auto* else_body = create<ast::BlockStatement>();
  else_body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("v"), "v"),
      create<ast::ScalarConstructorExpression>(
          create<ast::SintLiteral>(&i32, 3))));

  auto* cond = create<ast::ScalarConstructorExpression>(
      create<ast::BoolLiteral>(&bool_type, true));

  ast::IfStatement expr(Source{}, cond, body,
                        {create<ast::ElseStatement>(else_body)});

  td.RegisterVariableForTesting(var);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_TRUE(b.GenerateIfStatement(&expr)) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%5 = OpTypeBool
%6 = OpConstantTrue %5
%10 = OpConstant %3 2
%11 = OpConstant %3 3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(OpSelectionMerge %7 None
OpBranchConditional %6 %8 %9
%8 = OpLabel
OpStore %1 %10
OpBranch %7
%9 = OpLabel
OpStore %1 %11
OpBranch %7
%7 = OpLabel
)");
}

TEST_F(BuilderTest, If_WithElseIf) {
  ast::type::Bool bool_type;
  ast::type::I32 i32;

  // if (true) {
  //   v = 2;
  // } elseif (true) {
  //   v = 3;
  // }
  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "v",                             // name
                            ast::StorageClass::kPrivate,     // storage_class
                            &i32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations

  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("v"), "v"),
      create<ast::ScalarConstructorExpression>(
          create<ast::SintLiteral>(&i32, 2))));

  auto* else_body = create<ast::BlockStatement>();
  else_body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("v"), "v"),
      create<ast::ScalarConstructorExpression>(
          create<ast::SintLiteral>(&i32, 3))));

  auto* else_cond = create<ast::ScalarConstructorExpression>(
      create<ast::BoolLiteral>(&bool_type, true));

  auto* cond = create<ast::ScalarConstructorExpression>(
      create<ast::BoolLiteral>(&bool_type, true));

  ast::IfStatement expr(Source{}, cond, body,
                        {create<ast::ElseStatement>(else_cond, else_body)});

  td.RegisterVariableForTesting(var);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_TRUE(b.GenerateIfStatement(&expr)) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%5 = OpTypeBool
%6 = OpConstantTrue %5
%10 = OpConstant %3 2
%13 = OpConstant %3 3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(OpSelectionMerge %7 None
OpBranchConditional %6 %8 %9
%8 = OpLabel
OpStore %1 %10
OpBranch %7
%9 = OpLabel
OpSelectionMerge %11 None
OpBranchConditional %6 %12 %11
%12 = OpLabel
OpStore %1 %13
OpBranch %11
%11 = OpLabel
OpBranch %7
%7 = OpLabel
)");
}

TEST_F(BuilderTest, If_WithMultiple) {
  ast::type::Bool bool_type;
  ast::type::I32 i32;

  // if (true) {
  //   v = 2;
  // } elseif (true) {
  //   v = 3;
  // } elseif (false) {
  //   v = 4;
  // } else {
  //   v = 5;
  // }
  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "v",                             // name
                            ast::StorageClass::kPrivate,     // storage_class
                            &i32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations

  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("v"), "v"),
      create<ast::ScalarConstructorExpression>(
          create<ast::SintLiteral>(&i32, 2))));
  auto* elseif_1_body = create<ast::BlockStatement>();
  elseif_1_body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("v"), "v"),
      create<ast::ScalarConstructorExpression>(
          create<ast::SintLiteral>(&i32, 3))));
  auto* elseif_2_body = create<ast::BlockStatement>();
  elseif_2_body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("v"), "v"),
      create<ast::ScalarConstructorExpression>(
          create<ast::SintLiteral>(&i32, 4))));
  auto* else_body = create<ast::BlockStatement>();
  else_body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("v"), "v"),
      create<ast::ScalarConstructorExpression>(
          create<ast::SintLiteral>(&i32, 5))));

  auto* elseif_1_cond = create<ast::ScalarConstructorExpression>(
      create<ast::BoolLiteral>(&bool_type, true));
  auto* elseif_2_cond = create<ast::ScalarConstructorExpression>(
      create<ast::BoolLiteral>(&bool_type, false));

  auto* cond = create<ast::ScalarConstructorExpression>(
      create<ast::BoolLiteral>(&bool_type, true));

  ast::IfStatement expr(
      Source{}, cond, body,
      {
          create<ast::ElseStatement>(elseif_1_cond, elseif_1_body),
          create<ast::ElseStatement>(elseif_2_cond, elseif_2_body),
          create<ast::ElseStatement>(else_body),
      });

  td.RegisterVariableForTesting(var);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_TRUE(b.GenerateIfStatement(&expr)) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%5 = OpTypeBool
%6 = OpConstantTrue %5
%10 = OpConstant %3 2
%14 = OpConstant %3 3
%15 = OpConstantFalse %5
%19 = OpConstant %3 4
%20 = OpConstant %3 5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(OpSelectionMerge %7 None
OpBranchConditional %6 %8 %9
%8 = OpLabel
OpStore %1 %10
OpBranch %7
%9 = OpLabel
OpSelectionMerge %11 None
OpBranchConditional %6 %12 %13
%12 = OpLabel
OpStore %1 %14
OpBranch %11
%13 = OpLabel
OpSelectionMerge %16 None
OpBranchConditional %15 %17 %18
%17 = OpLabel
OpStore %1 %19
OpBranch %16
%18 = OpLabel
OpStore %1 %20
OpBranch %16
%16 = OpLabel
OpBranch %11
%11 = OpLabel
OpBranch %7
%7 = OpLabel
)");
}

TEST_F(BuilderTest, If_WithBreak) {
  ast::type::Bool bool_type;
  // loop {
  //   if (true) {
  //     break;
  //   }
  // }
  auto* cond = create<ast::ScalarConstructorExpression>(
      create<ast::BoolLiteral>(&bool_type, true));

  auto* if_body = create<ast::BlockStatement>();
  if_body->append(create<ast::BreakStatement>());

  auto* if_stmt = create<ast::IfStatement>(Source{}, cond, if_body,
                                           ast::ElseStatementList{});

  auto* loop_body = create<ast::BlockStatement>();
  loop_body->append(if_stmt);

  ast::LoopStatement expr(loop_body, create<ast::BlockStatement>());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});

  EXPECT_TRUE(b.GenerateLoopStatement(&expr)) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeBool
%6 = OpConstantTrue %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(OpBranch %1
%1 = OpLabel
OpLoopMerge %2 %3 None
OpBranch %4
%4 = OpLabel
OpSelectionMerge %7 None
OpBranchConditional %6 %8 %7
%8 = OpLabel
OpBranch %2
%7 = OpLabel
OpBranch %3
%3 = OpLabel
OpBranch %1
%2 = OpLabel
)");
}

TEST_F(BuilderTest, If_WithElseBreak) {
  ast::type::Bool bool_type;
  // loop {
  //   if (true) {
  //   } else {
  //     break;
  //   }
  // }
  auto* cond = create<ast::ScalarConstructorExpression>(
      create<ast::BoolLiteral>(&bool_type, true));

  auto* else_body = create<ast::BlockStatement>();
  else_body->append(create<ast::BreakStatement>());

  auto* if_stmt = create<ast::IfStatement>(
      Source{}, cond, create<ast::BlockStatement>(),
      ast::ElseStatementList{create<ast::ElseStatement>(else_body)});

  auto* loop_body = create<ast::BlockStatement>();
  loop_body->append(if_stmt);

  ast::LoopStatement expr(loop_body, create<ast::BlockStatement>());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});

  EXPECT_TRUE(b.GenerateLoopStatement(&expr)) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeBool
%6 = OpConstantTrue %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(OpBranch %1
%1 = OpLabel
OpLoopMerge %2 %3 None
OpBranch %4
%4 = OpLabel
OpSelectionMerge %7 None
OpBranchConditional %6 %8 %9
%8 = OpLabel
OpBranch %7
%9 = OpLabel
OpBranch %2
%7 = OpLabel
OpBranch %3
%3 = OpLabel
OpBranch %1
%2 = OpLabel
)");
}

TEST_F(BuilderTest, If_WithContinue) {
  ast::type::Bool bool_type;
  // loop {
  //   if (true) {
  //     continue;
  //   }
  // }
  auto* cond = create<ast::ScalarConstructorExpression>(
      create<ast::BoolLiteral>(&bool_type, true));

  auto* if_body = create<ast::BlockStatement>();
  if_body->append(create<ast::ContinueStatement>());

  auto* if_stmt = create<ast::IfStatement>(Source{}, cond, if_body,
                                           ast::ElseStatementList{});

  auto* loop_body = create<ast::BlockStatement>();
  loop_body->append(if_stmt);

  ast::LoopStatement expr(loop_body, create<ast::BlockStatement>());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});

  EXPECT_TRUE(b.GenerateLoopStatement(&expr)) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeBool
%6 = OpConstantTrue %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(OpBranch %1
%1 = OpLabel
OpLoopMerge %2 %3 None
OpBranch %4
%4 = OpLabel
OpSelectionMerge %7 None
OpBranchConditional %6 %8 %7
%8 = OpLabel
OpBranch %3
%7 = OpLabel
OpBranch %3
%3 = OpLabel
OpBranch %1
%2 = OpLabel
)");
}

TEST_F(BuilderTest, If_WithElseContinue) {
  ast::type::Bool bool_type;
  // loop {
  //   if (true) {
  //   } else {
  //     continue;
  //   }
  // }
  auto* cond = create<ast::ScalarConstructorExpression>(
      create<ast::BoolLiteral>(&bool_type, true));

  auto* else_body = create<ast::BlockStatement>();
  else_body->append(create<ast::ContinueStatement>());

  auto* if_stmt = create<ast::IfStatement>(
      Source{}, cond, create<ast::BlockStatement>(),
      ast::ElseStatementList{create<ast::ElseStatement>(else_body)});

  auto* loop_body = create<ast::BlockStatement>();
  loop_body->append(if_stmt);

  ast::LoopStatement expr(loop_body, create<ast::BlockStatement>());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});

  EXPECT_TRUE(b.GenerateLoopStatement(&expr)) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeBool
%6 = OpConstantTrue %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(OpBranch %1
%1 = OpLabel
OpLoopMerge %2 %3 None
OpBranch %4
%4 = OpLabel
OpSelectionMerge %7 None
OpBranchConditional %6 %8 %9
%8 = OpLabel
OpBranch %7
%9 = OpLabel
OpBranch %3
%7 = OpLabel
OpBranch %3
%3 = OpLabel
OpBranch %1
%2 = OpLabel
)");
}

TEST_F(BuilderTest, If_WithReturn) {
  ast::type::Bool bool_type;
  // if (true) {
  //   return;
  // }
  auto* cond = create<ast::ScalarConstructorExpression>(
      create<ast::BoolLiteral>(&bool_type, true));

  auto* if_body = create<ast::BlockStatement>();
  if_body->append(create<ast::ReturnStatement>(Source{}));

  ast::IfStatement expr(Source{}, cond, if_body, ast::ElseStatementList{});

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});

  EXPECT_TRUE(b.GenerateIfStatement(&expr)) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeBool
%2 = OpConstantTrue %1
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(OpSelectionMerge %3 None
OpBranchConditional %2 %4 %3
%4 = OpLabel
OpReturn
%3 = OpLabel
)");
}

TEST_F(BuilderTest, If_WithReturnValue) {
  ast::type::Bool bool_type;
  // if (true) {
  //   return false;
  // }
  auto* cond = create<ast::ScalarConstructorExpression>(
      create<ast::BoolLiteral>(&bool_type, true));
  auto* cond2 = create<ast::ScalarConstructorExpression>(
      create<ast::BoolLiteral>(&bool_type, false));

  auto* if_body = create<ast::BlockStatement>();
  if_body->append(create<ast::ReturnStatement>(Source{}, cond2));

  ast::IfStatement expr(Source{}, cond, if_body, ast::ElseStatementList{});

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});

  EXPECT_TRUE(b.GenerateIfStatement(&expr)) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeBool
%2 = OpConstantTrue %1
%5 = OpConstantFalse %1
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(OpSelectionMerge %3 None
OpBranchConditional %2 %4 %3
%4 = OpLabel
OpReturnValue %5
%3 = OpLabel
)");
}

TEST_F(BuilderTest, If_WithLoad_Bug327) {
  // var a : bool;
  // if (a) {
  // }

  ast::type::Bool bool_type;
  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "a",                             // name
                            ast::StorageClass::kFunction,    // storage_class
                            &bool_type,                      // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  td.RegisterVariableForTesting(var);

  ast::IfStatement expr(
      Source{},
      create<ast::IdentifierExpression>(mod->RegisterSymbol("a"), "a"),
      create<ast::BlockStatement>(), ast::ElseStatementList{});

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_TRUE(b.GenerateIfStatement(&expr)) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeBool
%2 = OpTypePointer Function %3
%1 = OpVariable %2 Function
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%4 = OpLoad %3 %1
OpSelectionMerge %5 None
OpBranchConditional %4 %6 %5
%6 = OpLabel
OpBranch %5
%5 = OpLabel
)");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
