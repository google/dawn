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
   OpEntryPoint Fragment %100 "main"
   OpExecutionMode %100 OriginUpperLeft
)";
}

std::string CommonTypes() {
    return R"(
  %void = OpTypeVoid
  %voidfn = OpTypeFunction %void

  %bool = OpTypeBool
  %uint = OpTypeInt 32 0
  %int = OpTypeInt 32 1
  %float = OpTypeFloat 32

  %v2bool = OpTypeVector %bool 2
  %v2uint = OpTypeVector %uint 2
  %v2int = OpTypeVector %int 2
  %v2float = OpTypeVector %float 2
)";
}

using SpvParserTestMiscInstruction = SpvParserTest;

TEST_F(SpvParserTestMiscInstruction, OpUndef_BeforeFunction_Scalar) {
    const auto assembly = Preamble() + CommonTypes() + R"(
     %1 = OpUndef %bool
     %2 = OpUndef %uint
     %3 = OpUndef %int
     %4 = OpUndef %float

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %11 = OpCopyObject %bool %1
     %12 = OpCopyObject %uint %2
     %13 = OpCopyObject %int %3
     %14 = OpCopyObject %float %4
     OpReturn
     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr(R"(let x_11 : bool = false;
let x_12 : u32 = 0u;
let x_13 : i32 = 0i;
let x_14 : f32 = 0.0f;
return;
)"));
}

TEST_F(SpvParserTestMiscInstruction, OpUndef_BeforeFunction_Vector) {
    const auto assembly = Preamble() + CommonTypes() + R"(
     %4 = OpUndef %v2bool
     %1 = OpUndef %v2uint
     %2 = OpUndef %v2int
     %3 = OpUndef %v2float

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel

     %14 = OpCopyObject %v2bool %4
     %11 = OpCopyObject %v2uint %1
     %12 = OpCopyObject %v2int %2
     %13 = OpCopyObject %v2float %3
     OpReturn
     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr(R"(let x_14 : vec2<bool> = vec2<bool>();
let x_11 : vec2u = vec2u();
let x_12 : vec2i = vec2i();
let x_13 : vec2f = vec2f();
)"));
}

TEST_F(SpvParserTestMiscInstruction, OpUndef_InFunction_Scalar) {
    const auto assembly = Preamble() + CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpUndef %bool
     %2 = OpUndef %uint
     %3 = OpUndef %int
     %4 = OpUndef %float

     %11 = OpCopyObject %bool %1
     %12 = OpCopyObject %uint %2
     %13 = OpCopyObject %int %3
     %14 = OpCopyObject %float %4
     OpReturn
     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr(R"(let x_11 : bool = false;
let x_12 : u32 = 0u;
let x_13 : i32 = 0i;
let x_14 : f32 = 0.0f;
return;
)"));
}

TEST_F(SpvParserTestMiscInstruction, OpUndef_InFunction_Vector) {
    const auto assembly = Preamble() + CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpUndef %v2uint
     %2 = OpUndef %v2int
     %3 = OpUndef %v2float

     %11 = OpCopyObject %v2uint %1
     %12 = OpCopyObject %v2int %2
     %13 = OpCopyObject %v2float %3
     OpReturn
     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr(R"(let x_11 : vec2u = vec2u();
let x_12 : vec2i = vec2i();
let x_13 : vec2f = vec2f();
)"));
}

TEST_F(SpvParserTestMiscInstruction, OpUndef_InFunction_Matrix) {
    const auto assembly = Preamble() + CommonTypes() + R"(
     %mat = OpTypeMatrix %v2float 2

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpUndef %mat

     %11 = OpCopyObject %mat %1
     OpReturn
     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_11 : mat2x2f = mat2x2f();"));
}

TEST_F(SpvParserTestMiscInstruction, OpUndef_InFunction_Array) {
    const auto assembly = Preamble() + CommonTypes() + R"(
     %uint_2 = OpConstant %uint 2
     %arr = OpTypeArray %uint %uint_2

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpUndef %arr

     %11 = OpCopyObject %arr %1
     OpReturn
     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_11 : array<u32, 2u> = array<u32, 2u>();"));
}

TEST_F(SpvParserTestMiscInstruction, OpUndef_InFunction_Struct) {
    const auto assembly = Preamble() + CommonTypes() + R"(
     %strct = OpTypeStruct %bool %uint %int %float

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpUndef %strct

     %11 = OpCopyObject %strct %1
     OpReturn
     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_11 : S = S(false, 0u, 0i, 0.0f);"));
}

TEST_F(SpvParserTestMiscInstruction, OpNop) {
    const auto assembly = Preamble() + CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     OpNop
     OpReturn
     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error() << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_EQ(test::ToString(p->program(), ast_body), "return;\n");
}

// Test swizzle generation.

struct SwizzleCase {
    uint32_t index;
    std::string expected_expr;
    std::string expected_error;
};
using SpvParserSwizzleTest = SpvParserTestBase<::testing::TestWithParam<SwizzleCase>>;

TEST_P(SpvParserSwizzleTest, Sample) {
    // We need a function so we can get a FunctionEmitter.
    const auto assembly = Preamble() + CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     OpReturn
     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);

    auto* result = fe.Swizzle(GetParam().index);
    if (GetParam().expected_error.empty()) {
        Program program(p->program());
        EXPECT_TRUE(fe.success());
        ASSERT_NE(result, nullptr);
        auto got = test::ToString(program, result);
        EXPECT_EQ(got, GetParam().expected_expr);
    } else {
        EXPECT_EQ(result, nullptr);
        EXPECT_FALSE(fe.success());
        EXPECT_EQ(p->error(), GetParam().expected_error);
    }
}

INSTANTIATE_TEST_SUITE_P(ValidIndex,
                         SpvParserSwizzleTest,
                         ::testing::ValuesIn(std::vector<SwizzleCase>{
                             {0, "x", ""},
                             {1, "y", ""},
                             {2, "z", ""},
                             {3, "w", ""},
                             {4, "", "vector component index is larger than 3: 4"},
                             {99999, "", "vector component index is larger than 3: 99999"}}));

// TODO(dneto): OpSizeof : requires Kernel (OpenCL)

}  // namespace
}  // namespace tint::reader::spirv
