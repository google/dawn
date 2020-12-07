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
#include "src/ast/case_statement.h"
#include "src/ast/fallthrough_statement.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/if_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/switch_statement.h"
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

TEST_F(BuilderTest, Switch_Empty) {
  ast::type::I32 i32;

  // switch (1) {
  // }
  auto* cond = create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1));

  ast::SwitchStatement expr(cond, ast::CaseStatementList{});

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});

  EXPECT_TRUE(b.GenerateSwitchStatement(&expr)) << b.error();
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
  ast::type::I32 i32;

  // switch(a) {
  //   case 1:
  //     v = 1;
  //   case 2:
  //     v = 2;
  // }

  auto* v =
      create<ast::Variable>(Source{}, "v", ast::StorageClass::kPrivate, &i32);
  auto* a =
      create<ast::Variable>(Source{}, "a", ast::StorageClass::kPrivate, &i32);

  auto* case_1_body = create<ast::BlockStatement>();
  case_1_body->append(
      create<ast::AssignmentStatement>(create<ast::IdentifierExpression>("v"),
                                       create<ast::ScalarConstructorExpression>(
                                           create<ast::SintLiteral>(&i32, 1))));

  auto* case_2_body = create<ast::BlockStatement>();
  case_2_body->append(
      create<ast::AssignmentStatement>(create<ast::IdentifierExpression>("v"),
                                       create<ast::ScalarConstructorExpression>(
                                           create<ast::SintLiteral>(&i32, 2))));

  ast::CaseSelectorList selector_1;
  selector_1.push_back(create<ast::SintLiteral>(&i32, 1));

  ast::CaseSelectorList selector_2;
  selector_2.push_back(create<ast::SintLiteral>(&i32, 2));

  ast::CaseStatementList cases;
  cases.push_back(create<ast::CaseStatement>(selector_1, case_1_body));
  cases.push_back(create<ast::CaseStatement>(selector_2, case_2_body));

  ast::SwitchStatement expr(create<ast::IdentifierExpression>("a"), cases);

  td.RegisterVariableForTesting(v);
  td.RegisterVariableForTesting(a);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func(Source{}, "a_func", {}, &i32,
                     create<ast::BlockStatement>(),
                     ast::FunctionDecorationList{});

  ASSERT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
  ASSERT_TRUE(b.GenerateGlobalVariable(a)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_TRUE(b.GenerateSwitchStatement(&expr)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(OpName %1 "v"
OpName %5 "a"
OpName %7 "a_func"
%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%5 = OpVariable %2 Private %4
%6 = OpTypeFunction %3
%14 = OpConstant %3 1
%15 = OpConstant %3 2
%7 = OpFunction %3 None %6
%8 = OpLabel
%10 = OpLoad %3 %5
OpSelectionMerge %9 None
OpSwitch %10 %11 1 %12 2 %13
%12 = OpLabel
OpStore %1 %14
OpBranch %9
%13 = OpLabel
OpStore %1 %15
OpBranch %9
%11 = OpLabel
OpBranch %9
%9 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(BuilderTest, Switch_WithDefault) {
  ast::type::I32 i32;

  // switch(true) {
  //   default:
  //     v = 1;
  //  }

  auto* v =
      create<ast::Variable>(Source{}, "v", ast::StorageClass::kPrivate, &i32);
  auto* a =
      create<ast::Variable>(Source{}, "a", ast::StorageClass::kPrivate, &i32);

  auto* default_body = create<ast::BlockStatement>();
  default_body->append(
      create<ast::AssignmentStatement>(create<ast::IdentifierExpression>("v"),
                                       create<ast::ScalarConstructorExpression>(
                                           create<ast::SintLiteral>(&i32, 1))));

  ast::CaseStatementList cases;
  cases.push_back(create<ast::CaseStatement>(default_body));

  ast::SwitchStatement expr(create<ast::IdentifierExpression>("a"), cases);

  td.RegisterVariableForTesting(v);
  td.RegisterVariableForTesting(a);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func(Source{}, "a_func", {}, &i32,
                     create<ast::BlockStatement>(),
                     ast::FunctionDecorationList{});

  ASSERT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
  ASSERT_TRUE(b.GenerateGlobalVariable(a)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_TRUE(b.GenerateSwitchStatement(&expr)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(OpName %1 "v"
OpName %5 "a"
OpName %7 "a_func"
%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%5 = OpVariable %2 Private %4
%6 = OpTypeFunction %3
%12 = OpConstant %3 1
%7 = OpFunction %3 None %6
%8 = OpLabel
%10 = OpLoad %3 %5
OpSelectionMerge %9 None
OpSwitch %10 %11
%11 = OpLabel
OpStore %1 %12
OpBranch %9
%9 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(BuilderTest, Switch_WithCaseAndDefault) {
  ast::type::I32 i32;

  // switch(a) {
  //   case 1:
  //      v = 1;
  //   case 2, 3:
  //      v = 2;
  //   default:
  //      v = 3;
  //  }

  auto* v =
      create<ast::Variable>(Source{}, "v", ast::StorageClass::kPrivate, &i32);
  auto* a =
      create<ast::Variable>(Source{}, "a", ast::StorageClass::kPrivate, &i32);

  auto* case_1_body = create<ast::BlockStatement>();
  case_1_body->append(
      create<ast::AssignmentStatement>(create<ast::IdentifierExpression>("v"),
                                       create<ast::ScalarConstructorExpression>(
                                           create<ast::SintLiteral>(&i32, 1))));

  auto* case_2_body = create<ast::BlockStatement>();
  case_2_body->append(
      create<ast::AssignmentStatement>(create<ast::IdentifierExpression>("v"),
                                       create<ast::ScalarConstructorExpression>(
                                           create<ast::SintLiteral>(&i32, 2))));

  auto* default_body = create<ast::BlockStatement>();
  default_body->append(
      create<ast::AssignmentStatement>(create<ast::IdentifierExpression>("v"),
                                       create<ast::ScalarConstructorExpression>(
                                           create<ast::SintLiteral>(&i32, 3))));

  ast::CaseSelectorList selector_1;
  selector_1.push_back(create<ast::SintLiteral>(&i32, 1));

  ast::CaseSelectorList selector_2;
  selector_2.push_back(create<ast::SintLiteral>(&i32, 2));
  selector_2.push_back(create<ast::SintLiteral>(&i32, 3));

  ast::CaseStatementList cases;
  cases.push_back(create<ast::CaseStatement>(selector_1, case_1_body));
  cases.push_back(create<ast::CaseStatement>(selector_2, case_2_body));
  cases.push_back(create<ast::CaseStatement>(default_body));

  ast::SwitchStatement expr(create<ast::IdentifierExpression>("a"), cases);

  td.RegisterVariableForTesting(v);
  td.RegisterVariableForTesting(a);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func(Source{}, "a_func", {}, &i32,
                     create<ast::BlockStatement>(),
                     ast::FunctionDecorationList{});

  ASSERT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
  ASSERT_TRUE(b.GenerateGlobalVariable(a)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_TRUE(b.GenerateSwitchStatement(&expr)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(OpName %1 "v"
OpName %5 "a"
OpName %7 "a_func"
%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%5 = OpVariable %2 Private %4
%6 = OpTypeFunction %3
%14 = OpConstant %3 1
%15 = OpConstant %3 2
%16 = OpConstant %3 3
%7 = OpFunction %3 None %6
%8 = OpLabel
%10 = OpLoad %3 %5
OpSelectionMerge %9 None
OpSwitch %10 %11 1 %12 2 %13 3 %13
%12 = OpLabel
OpStore %1 %14
OpBranch %9
%13 = OpLabel
OpStore %1 %15
OpBranch %9
%11 = OpLabel
OpStore %1 %16
OpBranch %9
%9 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(BuilderTest, Switch_CaseWithFallthrough) {
  ast::type::I32 i32;

  // switch(a) {
  //   case 1:
  //      v = 1;
  //      fallthrough;
  //   case 2:
  //      v = 2;
  //   default:
  //      v = 3;
  //  }

  auto* v =
      create<ast::Variable>(Source{}, "v", ast::StorageClass::kPrivate, &i32);
  auto* a =
      create<ast::Variable>(Source{}, "a", ast::StorageClass::kPrivate, &i32);

  auto* case_1_body = create<ast::BlockStatement>();
  case_1_body->append(
      create<ast::AssignmentStatement>(create<ast::IdentifierExpression>("v"),
                                       create<ast::ScalarConstructorExpression>(
                                           create<ast::SintLiteral>(&i32, 1))));
  case_1_body->append(create<ast::FallthroughStatement>());

  auto* case_2_body = create<ast::BlockStatement>();
  case_2_body->append(
      create<ast::AssignmentStatement>(create<ast::IdentifierExpression>("v"),
                                       create<ast::ScalarConstructorExpression>(
                                           create<ast::SintLiteral>(&i32, 2))));

  auto* default_body = create<ast::BlockStatement>();
  default_body->append(
      create<ast::AssignmentStatement>(create<ast::IdentifierExpression>("v"),
                                       create<ast::ScalarConstructorExpression>(
                                           create<ast::SintLiteral>(&i32, 3))));

  ast::CaseSelectorList selector_1;
  selector_1.push_back(create<ast::SintLiteral>(&i32, 1));

  ast::CaseSelectorList selector_2;
  selector_2.push_back(create<ast::SintLiteral>(&i32, 2));

  ast::CaseStatementList cases;
  cases.push_back(create<ast::CaseStatement>(selector_1, case_1_body));
  cases.push_back(create<ast::CaseStatement>(selector_2, case_2_body));
  cases.push_back(create<ast::CaseStatement>(default_body));

  ast::SwitchStatement expr(create<ast::IdentifierExpression>("a"), cases);

  td.RegisterVariableForTesting(v);
  td.RegisterVariableForTesting(a);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func(Source{}, "a_func", {}, &i32,
                     create<ast::BlockStatement>(),
                     ast::FunctionDecorationList{});

  ASSERT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
  ASSERT_TRUE(b.GenerateGlobalVariable(a)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_TRUE(b.GenerateSwitchStatement(&expr)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(OpName %1 "v"
OpName %5 "a"
OpName %7 "a_func"
%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%5 = OpVariable %2 Private %4
%6 = OpTypeFunction %3
%14 = OpConstant %3 1
%15 = OpConstant %3 2
%16 = OpConstant %3 3
%7 = OpFunction %3 None %6
%8 = OpLabel
%10 = OpLoad %3 %5
OpSelectionMerge %9 None
OpSwitch %10 %11 1 %12 2 %13
%12 = OpLabel
OpStore %1 %14
OpBranch %13
%13 = OpLabel
OpStore %1 %15
OpBranch %9
%11 = OpLabel
OpStore %1 %16
OpBranch %9
%9 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(BuilderTest, Switch_CaseFallthroughLastStatement) {
  ast::type::I32 i32;

  // switch(a) {
  //   case 1:
  //      v = 1;
  //      fallthrough;
  //  }

  auto* v =
      create<ast::Variable>(Source{}, "v", ast::StorageClass::kPrivate, &i32);
  auto* a =
      create<ast::Variable>(Source{}, "a", ast::StorageClass::kPrivate, &i32);

  auto* case_1_body = create<ast::BlockStatement>();
  case_1_body->append(
      create<ast::AssignmentStatement>(create<ast::IdentifierExpression>("v"),
                                       create<ast::ScalarConstructorExpression>(
                                           create<ast::SintLiteral>(&i32, 1))));
  case_1_body->append(create<ast::FallthroughStatement>());

  ast::CaseSelectorList selector_1;
  selector_1.push_back(create<ast::SintLiteral>(&i32, 1));

  ast::CaseStatementList cases;
  cases.push_back(create<ast::CaseStatement>(selector_1, case_1_body));

  ast::SwitchStatement expr(create<ast::IdentifierExpression>("a"), cases);

  td.RegisterVariableForTesting(v);
  td.RegisterVariableForTesting(a);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func(Source{}, "a_func", {}, &i32,
                     create<ast::BlockStatement>(),
                     ast::FunctionDecorationList{});

  ASSERT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
  ASSERT_TRUE(b.GenerateGlobalVariable(a)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_FALSE(b.GenerateSwitchStatement(&expr)) << b.error();
  EXPECT_EQ(b.error(), "fallthrough of last case statement is disallowed");
}

TEST_F(BuilderTest, Switch_WithNestedBreak) {
  ast::type::I32 i32;
  ast::type::Bool bool_type;

  // switch (a) {
  //   case 1:
  //     if (true) {
  //       break;
  //     }
  //     v = 1;
  //  }

  auto* v =
      create<ast::Variable>(Source{}, "v", ast::StorageClass::kPrivate, &i32);
  auto* a =
      create<ast::Variable>(Source{}, "a", ast::StorageClass::kPrivate, &i32);

  auto* if_body = create<ast::BlockStatement>();
  if_body->append(create<ast::BreakStatement>());

  auto* case_1_body = create<ast::BlockStatement>();
  case_1_body->append(
      create<ast::IfStatement>(Source{},
                               create<ast::ScalarConstructorExpression>(
                                   create<ast::BoolLiteral>(&bool_type, true)),
                               if_body, ast::ElseStatementList{}));

  case_1_body->append(
      create<ast::AssignmentStatement>(create<ast::IdentifierExpression>("v"),
                                       create<ast::ScalarConstructorExpression>(
                                           create<ast::SintLiteral>(&i32, 1))));

  ast::CaseSelectorList selector_1;
  selector_1.push_back(create<ast::SintLiteral>(&i32, 1));

  ast::CaseStatementList cases;
  cases.push_back(create<ast::CaseStatement>(selector_1, case_1_body));

  ast::SwitchStatement expr(create<ast::IdentifierExpression>("a"), cases);

  td.RegisterVariableForTesting(v);
  td.RegisterVariableForTesting(a);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func(Source{}, "a_func", {}, &i32,
                     create<ast::BlockStatement>(),
                     ast::FunctionDecorationList{});

  ASSERT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
  ASSERT_TRUE(b.GenerateGlobalVariable(a)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_TRUE(b.GenerateSwitchStatement(&expr)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(OpName %1 "v"
OpName %5 "a"
OpName %7 "a_func"
%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%5 = OpVariable %2 Private %4
%6 = OpTypeFunction %3
%13 = OpTypeBool
%14 = OpConstantTrue %13
%17 = OpConstant %3 1
%7 = OpFunction %3 None %6
%8 = OpLabel
%10 = OpLoad %3 %5
OpSelectionMerge %9 None
OpSwitch %10 %11 1 %12
%12 = OpLabel
OpSelectionMerge %15 None
OpBranchConditional %14 %16 %15
%16 = OpLabel
OpBranch %9
%15 = OpLabel
OpStore %1 %17
OpBranch %9
%11 = OpLabel
OpBranch %9
%9 = OpLabel
OpReturn
OpFunctionEnd
)");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
