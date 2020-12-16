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
#include "src/ast/break_statement.h"
#include "src/ast/continue_statement.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/loop_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
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

TEST_F(BuilderTest, Loop_Empty) {
  // loop {
  // }

  ast::LoopStatement loop(
      Source{}, create<ast::BlockStatement>(Source{}, ast::StatementList{}),
      create<ast::BlockStatement>(Source{}, ast::StatementList{}));

  ASSERT_TRUE(td.DetermineResultType(&loop)) << td.error();
  b.push_function(Function{});

  EXPECT_TRUE(b.GenerateLoopStatement(&loop)) << b.error();
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(OpBranch %1
%1 = OpLabel
OpLoopMerge %2 %3 None
OpBranch %4
%4 = OpLabel
OpBranch %3
%3 = OpLabel
OpBranch %1
%2 = OpLabel
)");
}

TEST_F(BuilderTest, Loop_WithoutContinuing) {
  // loop {
  //   v = 2;
  // }

  auto* var = Var("v", ast::StorageClass::kPrivate, ty.i32);
  auto* body = create<ast::BlockStatement>(
      Source{}, ast::StatementList{create<ast::AssignmentStatement>(
                    Source{}, Expr("v"), Expr(2))});

  ast::LoopStatement loop(
      Source{}, body,
      create<ast::BlockStatement>(Source{}, ast::StatementList{}));

  td.RegisterVariableForTesting(var);
  ASSERT_TRUE(td.DetermineResultType(&loop)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_TRUE(b.GenerateLoopStatement(&loop)) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%9 = OpConstant %3 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(OpBranch %5
%5 = OpLabel
OpLoopMerge %6 %7 None
OpBranch %8
%8 = OpLabel
OpStore %1 %9
OpBranch %7
%7 = OpLabel
OpBranch %5
%6 = OpLabel
)");
}

TEST_F(BuilderTest, Loop_WithContinuing) {
  ast::type::I32 i32;
  // loop {
  //   a = 2;
  //   continuing {
  //     a = 3;
  //   }
  // }

  auto* var = Var("v", ast::StorageClass::kPrivate, ty.i32);
  auto* body = create<ast::BlockStatement>(
      Source{}, ast::StatementList{create<ast::AssignmentStatement>(
                    Source{}, Expr("v"), Expr(2))});
  auto* continuing = create<ast::BlockStatement>(
      Source{}, ast::StatementList{create<ast::AssignmentStatement>(
                    Source{}, Expr("v"), Expr(3))});

  ast::LoopStatement loop(Source{}, body, continuing);

  td.RegisterVariableForTesting(var);
  ASSERT_TRUE(td.DetermineResultType(&loop)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_TRUE(b.GenerateLoopStatement(&loop)) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%9 = OpConstant %3 2
%10 = OpConstant %3 3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(OpBranch %5
%5 = OpLabel
OpLoopMerge %6 %7 None
OpBranch %8
%8 = OpLabel
OpStore %1 %9
OpBranch %7
%7 = OpLabel
OpStore %1 %10
OpBranch %5
%6 = OpLabel
)");
}

TEST_F(BuilderTest, Loop_WithContinue) {
  // loop {
  //   continue;
  // }
  auto* body = create<ast::BlockStatement>(
      Source{}, ast::StatementList{
                    create<ast::ContinueStatement>(Source{}),
                });
  ast::LoopStatement loop(
      Source{}, body,
      create<ast::BlockStatement>(Source{}, ast::StatementList{}));

  ASSERT_TRUE(td.DetermineResultType(&loop)) << td.error();

  b.push_function(Function{});

  EXPECT_TRUE(b.GenerateLoopStatement(&loop)) << b.error();
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(OpBranch %1
%1 = OpLabel
OpLoopMerge %2 %3 None
OpBranch %4
%4 = OpLabel
OpBranch %3
%3 = OpLabel
OpBranch %1
%2 = OpLabel
)");
}

TEST_F(BuilderTest, Loop_WithBreak) {
  // loop {
  //   break;
  // }
  auto* body = create<ast::BlockStatement>(
      Source{}, ast::StatementList{
                    create<ast::BreakStatement>(Source{}),
                });
  ast::LoopStatement loop(
      Source{}, body,
      create<ast::BlockStatement>(Source{}, ast::StatementList{}));

  ASSERT_TRUE(td.DetermineResultType(&loop)) << td.error();

  b.push_function(Function{});

  EXPECT_TRUE(b.GenerateLoopStatement(&loop)) << b.error();
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(OpBranch %1
%1 = OpLabel
OpLoopMerge %2 %3 None
OpBranch %4
%4 = OpLabel
OpBranch %2
%3 = OpLabel
OpBranch %1
%2 = OpLabel
)");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
