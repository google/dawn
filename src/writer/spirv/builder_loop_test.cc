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

  ast::LoopStatement expr(create<ast::BlockStatement>(),
                          create<ast::BlockStatement>());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();
  b.push_function(Function{});

  EXPECT_TRUE(b.GenerateLoopStatement(&expr)) << b.error();
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
  ast::type::I32 i32;

  // loop {
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

  ast::LoopStatement expr(body, create<ast::BlockStatement>());

  td.RegisterVariableForTesting(var);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_TRUE(b.GenerateLoopStatement(&expr)) << b.error();
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

  auto* continuing = create<ast::BlockStatement>();
  continuing->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("v"), "v"),
      create<ast::ScalarConstructorExpression>(
          create<ast::SintLiteral>(&i32, 3))));
  ast::LoopStatement expr(body, continuing);

  td.RegisterVariableForTesting(var);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_TRUE(b.GenerateLoopStatement(&expr)) << b.error();
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
  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::ContinueStatement>());

  ast::LoopStatement expr(body, create<ast::BlockStatement>());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});

  EXPECT_TRUE(b.GenerateLoopStatement(&expr)) << b.error();
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
  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::BreakStatement>());

  ast::LoopStatement expr(body, create<ast::BlockStatement>());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});

  EXPECT_TRUE(b.GenerateLoopStatement(&expr)) << b.error();
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
