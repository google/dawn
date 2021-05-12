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

#include "src/writer/spirv/spv_dump.h"
#include "src/writer/spirv/test_helper.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = TestHelper;

TEST_F(BuilderTest, If_Empty) {
  // if (true) {
  // }
  auto* cond = Expr(true);

  auto* expr =
      create<ast::IfStatement>(cond, Block(), ast::ElseStatementList{});
  WrapInFunction(expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});

  EXPECT_TRUE(b.GenerateIfStatement(expr)) << b.error();
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

TEST_F(BuilderTest, If_Empty_OutsideFunction_IsError) {
  // Outside a function.
  // if (true) {
  // }
  auto* cond = Expr(true);

  ast::ElseStatementList elses;
  auto* block = Block();
  auto* expr = create<ast::IfStatement>(cond, block, elses);
  WrapInFunction(expr);

  spirv::Builder& b = Build();

  EXPECT_FALSE(b.GenerateIfStatement(expr)) << b.error();
  EXPECT_TRUE(b.has_error());
  EXPECT_EQ(b.error(),
            "Internal error: trying to add SPIR-V instruction 247 outside a "
            "function");
}

TEST_F(BuilderTest, If_WithStatements) {
  // if (true) {
  //   v = 2;
  // }

  auto* var = Global("v", ty.i32(), ast::StorageClass::kPrivate);
  auto* body = Block(Assign("v", 2));
  auto* expr =
      create<ast::IfStatement>(Expr(true), body, ast::ElseStatementList{});
  WrapInFunction(expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_TRUE(b.GenerateIfStatement(expr)) << b.error();
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
  // if (true) {
  //   v = 2;
  // } else {
  //   v = 3;
  // }

  auto* var = Global("v", ty.i32(), ast::StorageClass::kPrivate);
  auto* body = Block(Assign("v", 2));
  auto* else_body = Block(Assign("v", 3));

  auto* expr = create<ast::IfStatement>(
      Expr(true), body,
      ast::ElseStatementList{create<ast::ElseStatement>(nullptr, else_body)});
  WrapInFunction(expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_TRUE(b.GenerateIfStatement(expr)) << b.error();
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
  // if (true) {
  //   v = 2;
  // } elseif (true) {
  //   v = 3;
  // }

  auto* var = Global("v", ty.i32(), ast::StorageClass::kPrivate);
  auto* body = Block(Assign("v", 2));
  auto* else_body = Block(Assign("v", 3));

  auto* expr = create<ast::IfStatement>(
      Expr(true), body,
      ast::ElseStatementList{
          create<ast::ElseStatement>(Expr(true), else_body),
      });
  WrapInFunction(expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_TRUE(b.GenerateIfStatement(expr)) << b.error();
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
  // if (true) {
  //   v = 2;
  // } elseif (true) {
  //   v = 3;
  // } elseif (false) {
  //   v = 4;
  // } else {
  //   v = 5;
  // }

  auto* var = Global("v", ty.i32(), ast::StorageClass::kPrivate);
  auto* body = Block(Assign("v", 2));
  auto* elseif_1_body = Block(Assign("v", 3));
  auto* elseif_2_body = Block(Assign("v", 4));
  auto* else_body = Block(Assign("v", 5));

  auto* expr = create<ast::IfStatement>(
      Expr(true), body,
      ast::ElseStatementList{
          create<ast::ElseStatement>(Expr(true), elseif_1_body),
          create<ast::ElseStatement>(Expr(false), elseif_2_body),
          create<ast::ElseStatement>(nullptr, else_body),
      });
  WrapInFunction(expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_TRUE(b.GenerateIfStatement(expr)) << b.error();
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
  // loop {
  //   if (true) {
  //     break;
  //   }
  // }

  auto* if_body = Block(create<ast::BreakStatement>());

  auto* if_stmt =
      create<ast::IfStatement>(Expr(true), if_body, ast::ElseStatementList{});

  auto* loop_body = Block(if_stmt);

  auto* expr = Loop(loop_body, Block());
  WrapInFunction(expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});

  EXPECT_TRUE(b.GenerateLoopStatement(expr)) << b.error();
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
  // loop {
  //   if (true) {
  //   } else {
  //     break;
  //   }
  // }
  auto* else_body = Block(create<ast::BreakStatement>());

  auto* if_stmt = create<ast::IfStatement>(
      Expr(true), Block(),
      ast::ElseStatementList{create<ast::ElseStatement>(nullptr, else_body)});

  auto* loop_body = Block(if_stmt);

  auto* expr = Loop(loop_body, Block());
  WrapInFunction(expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});

  EXPECT_TRUE(b.GenerateLoopStatement(expr)) << b.error();
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
  // loop {
  //   if (true) {
  //     continue;
  //   }
  // }
  auto* if_body = Block(create<ast::ContinueStatement>());

  auto* if_stmt =
      create<ast::IfStatement>(Expr(true), if_body, ast::ElseStatementList{});

  auto* loop_body = Block(if_stmt);

  auto* expr = Loop(loop_body, Block());
  WrapInFunction(expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});

  EXPECT_TRUE(b.GenerateLoopStatement(expr)) << b.error();
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
  // loop {
  //   if (true) {
  //   } else {
  //     continue;
  //   }
  // }
  auto* else_body = Block(create<ast::ContinueStatement>());

  auto* if_stmt = create<ast::IfStatement>(
      Expr(true), Block(),
      ast::ElseStatementList{create<ast::ElseStatement>(nullptr, else_body)});

  auto* loop_body = Block(if_stmt);

  auto* expr = Loop(loop_body, Block());
  WrapInFunction(expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});

  EXPECT_TRUE(b.GenerateLoopStatement(expr)) << b.error();
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
  // if (true) {
  //   return;
  // }
  auto* if_body = Block(Return());

  auto* expr =
      create<ast::IfStatement>(Expr(true), if_body, ast::ElseStatementList{});
  WrapInFunction(expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});

  EXPECT_TRUE(b.GenerateIfStatement(expr)) << b.error();
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
  // if (true) {
  //   return false;
  // }
  // return true;
  auto* if_body = Block(Return(false));
  auto* expr = If(Expr(true), if_body);
  Func("test", {}, ty.bool_(), {expr, Return(true)}, {});
  spirv::Builder& b = Build();

  b.push_function(Function{});

  EXPECT_TRUE(b.GenerateIfStatement(expr)) << b.error();
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

  auto* var = Global("a", ty.bool_(), ast::StorageClass::kPrivate);

  auto* expr =
      create<ast::IfStatement>(Expr("a"), Block(), ast::ElseStatementList{});
  WrapInFunction(expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_TRUE(b.GenerateIfStatement(expr)) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeBool
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%5 = OpLoad %3 %1
OpSelectionMerge %6 None
OpBranchConditional %5 %7 %6
%7 = OpLabel
OpBranch %6
%6 = OpLabel
)");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
