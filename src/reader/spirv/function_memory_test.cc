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

#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "src/reader/spirv/function.h"
#include "src/reader/spirv/parser_impl.h"
#include "src/reader/spirv/parser_impl_test_helper.h"
#include "src/reader/spirv/spirv_tools_helpers_test.h"

namespace tint {
namespace reader {
namespace spirv {
namespace {

using ::testing::HasSubstr;

TEST_F(SpvParserTest, EmitStatement_StoreBoolConst) {
  auto p = parser(test::Assemble(R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %ty = OpTypeBool
     %true = OpConstantTrue %ty
     %false = OpConstantFalse %ty
     %null = OpConstantNull %ty
     %ptr_ty = OpTypePointer Function %ty
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpVariable %ptr_ty Function
     OpStore %1 %true
     OpStore %1 %false
     OpStore %1 %null
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Assignment{
  Identifier{x_1}
  ScalarConstructor{true}
}
Assignment{
  Identifier{x_1}
  ScalarConstructor{false}
}
Assignment{
  Identifier{x_1}
  ScalarConstructor{false}
})"));
}

TEST_F(SpvParserTest, EmitStatement_StoreUintConst) {
  auto p = parser(test::Assemble(R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %ty = OpTypeInt 32 0
     %val = OpConstant %ty 42
     %null = OpConstantNull %ty
     %ptr_ty = OpTypePointer Function %ty
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpVariable %ptr_ty Function
     OpStore %1 %val
     OpStore %1 %null
     OpReturn
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody());
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Assignment{
  Identifier{x_1}
  ScalarConstructor{42}
}
Assignment{
  Identifier{x_1}
  ScalarConstructor{0}
})"));
}

TEST_F(SpvParserTest, EmitStatement_StoreIntConst) {
  auto p = parser(test::Assemble(R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %ty = OpTypeInt 32 1
     %val = OpConstant %ty 42
     %null = OpConstantNull %ty
     %ptr_ty = OpTypePointer Function %ty
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpVariable %ptr_ty Function
     OpStore %1 %val
     OpStore %1 %null
     OpReturn
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody());
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Assignment{
  Identifier{x_1}
  ScalarConstructor{42}
}
Assignment{
  Identifier{x_1}
  ScalarConstructor{0}
})"));
}

TEST_F(SpvParserTest, EmitStatement_StoreFloatConst) {
  auto p = parser(test::Assemble(R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %ty = OpTypeFloat 32
     %val = OpConstant %ty 42
     %null = OpConstantNull %ty
     %ptr_ty = OpTypePointer Function %ty
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpVariable %ptr_ty Function
     OpStore %1 %val
     OpStore %1 %null
     OpReturn
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody());
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Assignment{
  Identifier{x_1}
  ScalarConstructor{42.000000}
}
Assignment{
  Identifier{x_1}
  ScalarConstructor{0.000000}
})"));
}

TEST_F(SpvParserTest, EmitStatement_LoadBool) {
  auto p = parser(test::Assemble(R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %ty = OpTypeBool
     %true = OpConstantTrue %ty
     %false = OpConstantFalse %ty
     %null = OpConstantNull %ty
     %ptr_ty = OpTypePointer Function %ty
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpVariable %ptr_ty Function %true
     %2 = OpLoad %ty %1
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(
  Variable{
    x_2
    none
    __bool
    {
      Identifier{x_1}
    }
  })"));
}

TEST_F(SpvParserTest, EmitStatement_LoadScalar) {
  auto p = parser(test::Assemble(R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %ty = OpTypeInt 32 0
     %ty_42 = OpConstant %ty 42
     %ptr_ty = OpTypePointer Function %ty
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpVariable %ptr_ty Function %ty_42
     %2 = OpLoad %ty %1
     %3 = OpLoad %ty %1
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(VariableDeclStatement{
  Variable{
    x_2
    none
    __u32
    {
      Identifier{x_1}
    }
  }
}
VariableDeclStatement{
  Variable{
    x_3
    none
    __u32
    {
      Identifier{x_1}
    }
  }
})"));
}

TEST_F(SpvParserTest, EmitStatement_UseLoadedScalarTwice) {
  auto p = parser(test::Assemble(R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %ty = OpTypeInt 32 0
     %ty_42 = OpConstant %ty 42
     %ptr_ty = OpTypePointer Function %ty
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpVariable %ptr_ty Function %ty_42
     %2 = OpLoad %ty %1
     OpStore %1 %2
     OpStore %1 %2
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(VariableDeclStatement{
  Variable{
    x_2
    none
    __u32
    {
      Identifier{x_1}
    }
  }
}
Assignment{
  Identifier{x_1}
  Identifier{x_2}
}
Assignment{
  Identifier{x_1}
  Identifier{x_2}
}
)"));
}

TEST_F(SpvParserTest, EmitStatement_StoreToModuleScopeVar) {
  auto p = parser(test::Assemble(R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %ty = OpTypeInt 32 0
     %val = OpConstant %ty 42
     %ptr_ty = OpTypePointer Workgroup %ty
     %1 = OpVariable %ptr_ty Workgroup
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     OpStore %1 %val
     OpReturn
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody());
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Assignment{
  Identifier{x_1}
  ScalarConstructor{42}
})"));
}

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
