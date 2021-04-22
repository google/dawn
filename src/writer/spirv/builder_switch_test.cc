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

#include "src/ast/fallthrough_statement.h"
#include "src/writer/spirv/spv_dump.h"
#include "src/writer/spirv/test_helper.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = TestHelper;

TEST_F(BuilderTest, Switch_Empty) {
  // switch (1) {
  //   default: {}
  // }

  auto* expr = Switch(1, DefaultCase());
  WrapInFunction(expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});

  EXPECT_TRUE(b.GenerateSwitchStatement(expr)) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%3 = OpConstant %2 1
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(OpSelectionMerge %1 None
OpSwitch %3 %4
%4 = OpLabel
OpBranch %1
%1 = OpLabel
)");
}

TEST_F(BuilderTest, Switch_WithCase) {
  // switch(a) {
  //   case 1:
  //     v = 1;
  //   case 2:
  //     v = 2;
  //   default: {}
  // }

  auto* v = Global("v", ty.i32(), ast::StorageClass::kPrivate);
  auto* a = Global("a", ty.i32(), ast::StorageClass::kPrivate);

  auto* expr = Switch("a", /**/
                      Case(Literal(1), Block(Assign("v", 1))),
                      Case(Literal(2), Block(Assign("v", 2))), DefaultCase());
  WrapInFunction(expr);

  auto* func = Func("a_func", {}, ty.void_(), ast::StatementList{},
                    ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
  ASSERT_TRUE(b.GenerateGlobalVariable(a)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_TRUE(b.GenerateSwitchStatement(expr)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(OpName %1 "v"
OpName %5 "a"
OpName %8 "a_func"
%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%5 = OpVariable %2 Private %4
%7 = OpTypeVoid
%6 = OpTypeFunction %7
%15 = OpConstant %3 1
%16 = OpConstant %3 2
%8 = OpFunction %7 None %6
%9 = OpLabel
%11 = OpLoad %3 %5
OpSelectionMerge %10 None
OpSwitch %11 %12 1 %13 2 %14
%13 = OpLabel
OpStore %1 %15
OpBranch %10
%14 = OpLabel
OpStore %1 %16
OpBranch %10
%12 = OpLabel
OpBranch %10
%10 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(BuilderTest, Switch_WithCase_Unsigned) {
  // switch(a) {
  //   case 1u:
  //     v = 1;
  //   case 2u:
  //     v = 2;
  //   default: {}
  // }

  auto* v = Global("v", ty.i32(), ast::StorageClass::kPrivate);
  auto* a = Global("a", ty.u32(), ast::StorageClass::kPrivate);

  auto* expr = Switch("a", Case(Literal(1u), Block(Assign("v", 1))),
                      Case(Literal(2u), Block(Assign("v", 2))), DefaultCase());

  WrapInFunction(expr);

  auto* func = Func("a_func", {}, ty.void_(), ast::StatementList{},
                    ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
  ASSERT_TRUE(b.GenerateGlobalVariable(a)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_TRUE(b.GenerateSwitchStatement(expr)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(OpName %1 "v"
OpName %5 "a"
OpName %11 "a_func"
%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%7 = OpTypeInt 32 0
%6 = OpTypePointer Private %7
%8 = OpConstantNull %7
%5 = OpVariable %6 Private %8
%10 = OpTypeVoid
%9 = OpTypeFunction %10
%18 = OpConstant %3 1
%19 = OpConstant %3 2
%11 = OpFunction %10 None %9
%12 = OpLabel
%14 = OpLoad %7 %5
OpSelectionMerge %13 None
OpSwitch %14 %15 1 %16 2 %17
%16 = OpLabel
OpStore %1 %18
OpBranch %13
%17 = OpLabel
OpStore %1 %19
OpBranch %13
%15 = OpLabel
OpBranch %13
%13 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(BuilderTest, Switch_WithDefault) {
  // switch(true) {
  //   default: {}
  //     v = 1;
  //  }

  auto* v = Global("v", ty.i32(), ast::StorageClass::kPrivate);
  auto* a = Global("a", ty.i32(), ast::StorageClass::kPrivate);

  auto* default_body = Block(Assign("v", 1));

  ast::CaseStatementList cases;
  cases.push_back(
      create<ast::CaseStatement>(ast::CaseSelectorList{}, default_body));

  auto* expr = create<ast::SwitchStatement>(Expr("a"), cases);

  WrapInFunction(expr);

  auto* func = Func("a_func", {}, ty.void_(), ast::StatementList{},
                    ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
  ASSERT_TRUE(b.GenerateGlobalVariable(a)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_TRUE(b.GenerateSwitchStatement(expr)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(OpName %1 "v"
OpName %5 "a"
OpName %8 "a_func"
%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%5 = OpVariable %2 Private %4
%7 = OpTypeVoid
%6 = OpTypeFunction %7
%13 = OpConstant %3 1
%8 = OpFunction %7 None %6
%9 = OpLabel
%11 = OpLoad %3 %5
OpSelectionMerge %10 None
OpSwitch %11 %12
%12 = OpLabel
OpStore %1 %13
OpBranch %10
%10 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(BuilderTest, Switch_WithCaseAndDefault) {
  // switch(a) {
  //   case 1:
  //      v = 1;
  //   case 2, 3:
  //      v = 2;
  //   default: {}
  //      v = 3;
  //  }

  auto* v = Global("v", ty.i32(), ast::StorageClass::kPrivate);
  auto* a = Global("a", ty.i32(), ast::StorageClass::kPrivate);

  auto* case_1_body = Block(Assign("v", Expr(1)));

  auto* case_2_body = Block(Assign("v", Expr(2)));

  auto* default_body = Block(Assign("v", Expr(3)));

  ast::CaseSelectorList selector_1;
  selector_1.push_back(Literal(1));

  ast::CaseSelectorList selector_2;
  selector_2.push_back(Literal(2));
  selector_2.push_back(Literal(3));

  ast::CaseStatementList cases;
  cases.push_back(create<ast::CaseStatement>(selector_1, case_1_body));
  cases.push_back(create<ast::CaseStatement>(selector_2, case_2_body));
  cases.push_back(
      create<ast::CaseStatement>(ast::CaseSelectorList{}, default_body));

  auto* expr = create<ast::SwitchStatement>(Expr("a"), cases);

  WrapInFunction(expr);

  auto* func = Func("a_func", {}, ty.void_(), ast::StatementList{},
                    ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
  ASSERT_TRUE(b.GenerateGlobalVariable(a)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_TRUE(b.GenerateSwitchStatement(expr)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(OpName %1 "v"
OpName %5 "a"
OpName %8 "a_func"
%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%5 = OpVariable %2 Private %4
%7 = OpTypeVoid
%6 = OpTypeFunction %7
%15 = OpConstant %3 1
%16 = OpConstant %3 2
%17 = OpConstant %3 3
%8 = OpFunction %7 None %6
%9 = OpLabel
%11 = OpLoad %3 %5
OpSelectionMerge %10 None
OpSwitch %11 %12 1 %13 2 %14 3 %14
%13 = OpLabel
OpStore %1 %15
OpBranch %10
%14 = OpLabel
OpStore %1 %16
OpBranch %10
%12 = OpLabel
OpStore %1 %17
OpBranch %10
%10 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(BuilderTest, Switch_CaseWithFallthrough) {
  // switch(a) {
  //   case 1:
  //      v = 1;
  //      fallthrough;
  //   case 2:
  //      v = 2;
  //   default: {}
  //      v = 3;
  //  }

  auto* v = Global("v", ty.i32(), ast::StorageClass::kPrivate);
  auto* a = Global("a", ty.i32(), ast::StorageClass::kPrivate);

  auto* case_1_body =
      Block(Assign("v", Expr(1)), create<ast::FallthroughStatement>());

  auto* case_2_body = Block(Assign("v", Expr(2)));

  auto* default_body = Block(Assign("v", Expr(3)));

  ast::CaseSelectorList selector_1;
  selector_1.push_back(Literal(1));

  ast::CaseSelectorList selector_2;
  selector_2.push_back(Literal(2));

  ast::CaseStatementList cases;
  cases.push_back(create<ast::CaseStatement>(selector_1, case_1_body));
  cases.push_back(create<ast::CaseStatement>(selector_2, case_2_body));
  cases.push_back(
      create<ast::CaseStatement>(ast::CaseSelectorList{}, default_body));

  auto* expr = create<ast::SwitchStatement>(Expr("a"), cases);

  WrapInFunction(expr);

  auto* func = Func("a_func", {}, ty.void_(), ast::StatementList{},
                    ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
  ASSERT_TRUE(b.GenerateGlobalVariable(a)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_TRUE(b.GenerateSwitchStatement(expr)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(OpName %1 "v"
OpName %5 "a"
OpName %8 "a_func"
%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%5 = OpVariable %2 Private %4
%7 = OpTypeVoid
%6 = OpTypeFunction %7
%15 = OpConstant %3 1
%16 = OpConstant %3 2
%17 = OpConstant %3 3
%8 = OpFunction %7 None %6
%9 = OpLabel
%11 = OpLoad %3 %5
OpSelectionMerge %10 None
OpSwitch %11 %12 1 %13 2 %14
%13 = OpLabel
OpStore %1 %15
OpBranch %14
%14 = OpLabel
OpStore %1 %16
OpBranch %10
%12 = OpLabel
OpStore %1 %17
OpBranch %10
%10 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(BuilderTest, Switch_WithNestedBreak) {
  // switch (a) {
  //   case 1:
  //     if (true) {
  //       break;
  //     }
  //     v = 1;
  //   default: {}
  // }

  auto* v = Global("v", ty.i32(), ast::StorageClass::kPrivate);
  auto* a = Global("a", ty.i32(), ast::StorageClass::kPrivate);

  auto* expr =
      Switch("a", /**/
             Case(Literal(1),
                  Block(/**/
                        If(Expr(true), Block(create<ast::BreakStatement>())),
                        Assign("v", 1))),
             DefaultCase());

  WrapInFunction(expr);

  auto* func = Func("a_func", {}, ty.void_(), ast::StatementList{},
                    ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
  ASSERT_TRUE(b.GenerateGlobalVariable(a)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_TRUE(b.GenerateSwitchStatement(expr)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(OpName %1 "v"
OpName %5 "a"
OpName %8 "a_func"
%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%5 = OpVariable %2 Private %4
%7 = OpTypeVoid
%6 = OpTypeFunction %7
%14 = OpTypeBool
%15 = OpConstantTrue %14
%18 = OpConstant %3 1
%8 = OpFunction %7 None %6
%9 = OpLabel
%11 = OpLoad %3 %5
OpSelectionMerge %10 None
OpSwitch %11 %12 1 %13
%13 = OpLabel
OpSelectionMerge %16 None
OpBranchConditional %15 %17 %16
%17 = OpLabel
OpBranch %10
%16 = OpLabel
OpStore %1 %18
OpBranch %10
%12 = OpLabel
OpBranch %10
%10 = OpLabel
OpReturn
OpFunctionEnd
)");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
