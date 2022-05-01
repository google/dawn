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

#include "gmock/gmock.h"
#include "src/tint/reader/spirv/function.h"
#include "src/tint/reader/spirv/parser_impl_test_helper.h"
#include "src/tint/reader/spirv/spirv_tools_helpers_test.h"

namespace tint::reader::spirv {
namespace {

using ::testing::Eq;
using ::testing::HasSubstr;

std::string Preamble() {
    return R"(
     OpCapability Shader
     OpMemoryModel Logical Simple
     OpEntryPoint Fragment %100 "x_100"
     OpExecutionMode %100 OriginUpperLeft
)";
}

TEST_F(SpvParserTest, EmitStatement_VoidCallNoParams) {
    auto p = parser(test::Assemble(Preamble() + R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void

     %50 = OpFunction %void None %voidfn
     %entry_50 = OpLabel
     OpReturn
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpFunctionCall %void %50
     OpReturn
     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error();
    const auto got = test::ToString(p->program());
    const char* expect = R"(fn x_50() {
  return;
}

fn x_100_1() {
  x_50();
  return;
}

@stage(fragment)
fn x_100() {
  x_100_1();
}
)";
    EXPECT_EQ(expect, got);
}

TEST_F(SpvParserTest, EmitStatement_ScalarCallNoParams) {
    auto p = parser(test::Assemble(Preamble() + R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %uint = OpTypeInt 32 0
     %uintfn = OpTypeFunction %uint
     %val = OpConstant %uint 42

     %50 = OpFunction %uint None %uintfn
     %entry_50 = OpLabel
     OpReturnValue %val
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpFunctionCall %uint %50
     OpReturn
     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    ast::StatementList f100;
    {
        auto fe = p->function_emitter(100);
        EXPECT_TRUE(fe.EmitBody()) << p->error();
        f100 = fe.ast_body();
    }
    ast::StatementList f50;
    {
        auto fe = p->function_emitter(50);
        EXPECT_TRUE(fe.EmitBody()) << p->error();
        f50 = fe.ast_body();
    }
    auto program = p->program();
    EXPECT_THAT(test::ToString(program, f100), HasSubstr("let x_1 : u32 = x_50();\nreturn;"));
    EXPECT_THAT(test::ToString(program, f50), HasSubstr("return 42u;"));
}

TEST_F(SpvParserTest, EmitStatement_ScalarCallNoParamsUsedTwice) {
    auto p = parser(test::Assemble(Preamble() + R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %uint = OpTypeInt 32 0
     %uintfn = OpTypeFunction %uint
     %val = OpConstant %uint 42
     %ptr_uint = OpTypePointer Function %uint

     %50 = OpFunction %uint None %uintfn
     %entry_50 = OpLabel
     OpReturnValue %val
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %10 = OpVariable %ptr_uint Function
     %1 = OpFunctionCall %uint %50
     OpStore %10 %1
     OpStore %10 %1
     OpReturn
     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    ast::StatementList f100;
    {
        auto fe = p->function_emitter(100);
        EXPECT_TRUE(fe.EmitBody()) << p->error();
        f100 = fe.ast_body();
    }
    ast::StatementList f50;
    {
        auto fe = p->function_emitter(50);
        EXPECT_TRUE(fe.EmitBody()) << p->error();
        f50 = fe.ast_body();
    }
    auto program = p->program();
    EXPECT_EQ(test::ToString(program, f100), R"(var x_10 : u32;
let x_1 : u32 = x_50();
x_10 = x_1;
x_10 = x_1;
return;
)");
    EXPECT_THAT(test::ToString(program, f50), HasSubstr("return 42u;"));
}

TEST_F(SpvParserTest, EmitStatement_CallWithParams) {
    auto p = parser(test::Assemble(Preamble() + R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %uint = OpTypeInt 32 0
     %uintfn_uint_uint = OpTypeFunction %uint %uint %uint
     %val = OpConstant %uint 42
     %val2 = OpConstant %uint 84

     %50 = OpFunction %uint None %uintfn_uint_uint
     %51 = OpFunctionParameter %uint
     %52 = OpFunctionParameter %uint
     %entry_50 = OpLabel
     %sum = OpIAdd %uint %51 %52
     OpReturnValue %sum
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpFunctionCall %uint %50 %val %val2
     OpReturn
     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error();
    EXPECT_TRUE(p->error().empty());
    const auto program_ast_str = test::ToString(p->program());
    const std::string expected = R"(fn x_50(x_51 : u32, x_52 : u32) -> u32 {
  return (x_51 + x_52);
}

fn x_100_1() {
  let x_1 : u32 = x_50(42u, 84u);
  return;
}

@stage(fragment)
fn x_100() {
  x_100_1();
}
)";
    EXPECT_EQ(program_ast_str, expected);
}

}  // namespace
}  // namespace tint::reader::spirv
