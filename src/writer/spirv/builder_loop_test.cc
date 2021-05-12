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

TEST_F(BuilderTest, Loop_Empty) {
  // loop {
  // }

  auto* loop = Loop(Block(), Block());
  WrapInFunction(loop);

  spirv::Builder& b = Build();

  b.push_function(Function{});

  EXPECT_TRUE(b.GenerateLoopStatement(loop)) << b.error();
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

  auto* var = Global("v", ty.i32(), ast::StorageClass::kPrivate);
  auto* body = Block(Assign("v", 2));

  auto* loop = Loop(body, Block());
  WrapInFunction(loop);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_TRUE(b.GenerateLoopStatement(loop)) << b.error();
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
  // loop {
  //   a = 2;
  //   continuing {
  //     a = 3;
  //   }
  // }

  auto* var = Global("v", ty.i32(), ast::StorageClass::kPrivate);
  auto* body = Block(Assign("v", 2));
  auto* continuing = Block(Assign("v", 3));

  auto* loop = Loop(body, continuing);
  WrapInFunction(loop);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_TRUE(b.GenerateLoopStatement(loop)) << b.error();
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

TEST_F(BuilderTest, Loop_WithBodyVariableAccessInContinuing) {
  // loop {
  //   var a : i32;
  //   continuing {
  //     a = 3;
  //   }
  // }

  auto* var = Var("a", ty.i32());
  auto* var_decl = WrapInStatement(var);
  auto* body = Block(var_decl);
  auto* continuing = Block(Assign("a", 3));

  auto* loop = Loop(body, continuing);
  WrapInFunction(loop);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  EXPECT_TRUE(b.GenerateLoopStatement(loop)) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%7 = OpTypeInt 32 1
%6 = OpTypePointer Function %7
%8 = OpConstantNull %7
%9 = OpConstant %7 3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(OpBranch %1
%1 = OpLabel
OpLoopMerge %2 %3 None
OpBranch %4
%4 = OpLabel
OpBranch %3
%3 = OpLabel
OpStore %5 %9
OpBranch %1
%2 = OpLabel
)");
}

TEST_F(BuilderTest, Loop_WithContinue) {
  // loop {
  //   continue;
  // }
  auto* body = Block(create<ast::ContinueStatement>());
  auto* loop = Loop(body, Block());
  WrapInFunction(loop);

  spirv::Builder& b = Build();

  b.push_function(Function{});

  EXPECT_TRUE(b.GenerateLoopStatement(loop)) << b.error();
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
  auto* body = Block(create<ast::BreakStatement>());
  auto* loop = Loop(body, Block());
  WrapInFunction(loop);

  spirv::Builder& b = Build();

  b.push_function(Function{});

  EXPECT_TRUE(b.GenerateLoopStatement(loop)) << b.error();
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
