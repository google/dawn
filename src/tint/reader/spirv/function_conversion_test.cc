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

  %void = OpTypeVoid
  %voidfn = OpTypeFunction %void

  %bool = OpTypeBool
  %uint = OpTypeInt 32 0
  %int = OpTypeInt 32 1
  %float = OpTypeFloat 32

  %true = OpConstantTrue %bool
  %false = OpConstantFalse %bool
  %v2bool = OpTypeVector %bool 2
  %v2bool_t_f = OpConstantComposite %v2bool %true %false

  %uint_10 = OpConstant %uint 10
  %uint_20 = OpConstant %uint 20
  %int_30 = OpConstant %int 30
  %int_40 = OpConstant %int 40
  %float_50 = OpConstant %float 50
  %float_60 = OpConstant %float 60

  %ptr_uint = OpTypePointer Function %uint
  %ptr_int = OpTypePointer Function %int
  %ptr_float = OpTypePointer Function %float

  %v2uint = OpTypeVector %uint 2
  %v2int = OpTypeVector %int 2
  %v2float = OpTypeVector %float 2

  %v2uint_10_20 = OpConstantComposite %v2uint %uint_10 %uint_20
  %v2uint_20_10 = OpConstantComposite %v2uint %uint_20 %uint_10
  %v2int_30_40 = OpConstantComposite %v2int %int_30 %int_40
  %v2int_40_30 = OpConstantComposite %v2int %int_40 %int_30
  %v2float_50_60 = OpConstantComposite %v2float %float_50 %float_60
  %v2float_60_50 = OpConstantComposite %v2float %float_60 %float_50
)";
}

using SpvUnaryConversionTest = SpvParserTestBase<::testing::Test>;

TEST_F(SpvUnaryConversionTest, Bitcast_Scalar) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpBitcast %uint %float_50
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_1 : u32 = bitcast<u32>(50.0f);"));
}

TEST_F(SpvUnaryConversionTest, Bitcast_Vector) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpBitcast %v2float %v2uint_10_20
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_1 : vec2f = bitcast<vec2f>(vec2u(10u, 20u));"));
}

TEST_F(SpvUnaryConversionTest, ConvertSToF_BadArg) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpConvertSToF %float %void
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_THAT(p->error(), HasSubstr("unhandled expression for ID 2\n%2 = OpTypeVoid"));
}

TEST_F(SpvUnaryConversionTest, ConvertUToF_BadArg) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpConvertUToF %float %void
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_THAT(p->error(), HasSubstr("unhandled expression for ID 2\n%2 = OpTypeVoid"));
}

TEST_F(SpvUnaryConversionTest, ConvertFToS_BadArg) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpConvertFToS %float %void
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_THAT(p->error(), HasSubstr("unhandled expression for ID 2\n%2 = OpTypeVoid"));
}

TEST_F(SpvUnaryConversionTest, ConvertFToU_BadArg) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpConvertFToU %float %void
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_THAT(p->error(), HasSubstr("unhandled expression for ID 2\n%2 = OpTypeVoid"));
}

TEST_F(SpvUnaryConversionTest, ConvertSToF_Scalar_BadArgType) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpConvertSToF %float %false
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_THAT(p->error(), HasSubstr("operand for conversion to floating point must be "
                                      "integral scalar or vector"));
}

TEST_F(SpvUnaryConversionTest, ConvertSToF_Vector_BadArgType) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpConvertSToF %v2float %v2bool_t_f
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_THAT(p->error(), HasSubstr("operand for conversion to floating point must be integral "
                                      "scalar or vector"));
}

TEST_F(SpvUnaryConversionTest, ConvertSToF_Scalar_FromSigned) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %int %int_30
     %1 = OpConvertSToF %float %30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_1 : f32 = f32(x_30);"));
}

TEST_F(SpvUnaryConversionTest, ConvertSToF_Scalar_FromUnsigned) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %uint %uint_10
     %1 = OpConvertSToF %float %30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_1 : f32 = f32(bitcast<i32>(x_30));"));
}

TEST_F(SpvUnaryConversionTest, ConvertSToF_Vector_FromSigned) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %v2int %v2int_30_40
     %1 = OpConvertSToF %v2float %30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_1 : vec2f = vec2f(x_30);"));
}

TEST_F(SpvUnaryConversionTest, ConvertSToF_Vector_FromUnsigned) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %v2uint %v2uint_10_20
     %1 = OpConvertSToF %v2float %30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_1 : vec2f = vec2f(bitcast<vec2i>(x_30));"));
}

TEST_F(SpvUnaryConversionTest, ConvertUToF_Scalar_BadArgType) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpConvertUToF %float %false
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_THAT(p->error(), HasSubstr("operand for conversion to floating point must be "
                                      "integral scalar or vector"));
}

TEST_F(SpvUnaryConversionTest, ConvertUToF_Vector_BadArgType) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpConvertUToF %v2float %v2bool_t_f
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_THAT(p->error(), HasSubstr("operand for conversion to floating point must be integral "
                                      "scalar or vector"));
}

TEST_F(SpvUnaryConversionTest, ConvertUToF_Scalar_FromSigned) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %int %int_30
     %1 = OpConvertUToF %float %30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_1 : f32 = f32(bitcast<u32>(x_30));"));
}

TEST_F(SpvUnaryConversionTest, ConvertUToF_Scalar_FromUnsigned) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %uint %uint_10
     %1 = OpConvertUToF %float %30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_1 : f32 = f32(x_30);"));
}

TEST_F(SpvUnaryConversionTest, ConvertUToF_Vector_FromSigned) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %v2int %v2int_30_40
     %1 = OpConvertUToF %v2float %30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_1 : vec2f = vec2f(bitcast<vec2u>(x_30));"));
}

TEST_F(SpvUnaryConversionTest, ConvertUToF_Vector_FromUnsigned) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %v2uint %v2uint_10_20
     %1 = OpConvertUToF %v2float %30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_1 : vec2f = vec2f(x_30);"));
}

TEST_F(SpvUnaryConversionTest, ConvertFToS_Scalar_BadArgType) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpConvertFToS %int %uint_10
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_THAT(p->error(), HasSubstr("operand for conversion to signed integer must be floating "
                                      "point scalar or vector"));
}

TEST_F(SpvUnaryConversionTest, ConvertFToS_Vector_BadArgType) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpConvertFToS %v2float %v2bool_t_f
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_THAT(p->error(), HasSubstr("operand for conversion to signed integer must be floating "
                                      "point scalar or vector"));
}

TEST_F(SpvUnaryConversionTest, ConvertFToS_Scalar_ToSigned) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %float %float_50
     %1 = OpConvertFToS %int %30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_1 : i32 = i32(x_30);"));
}

TEST_F(SpvUnaryConversionTest, ConvertFToS_Scalar_ToUnsigned) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %float %float_50
     %1 = OpConvertFToS %uint %30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_1 : u32 = bitcast<u32>(i32(x_30));"));
}

TEST_F(SpvUnaryConversionTest, ConvertFToS_Vector_ToSigned) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %v2float %v2float_50_60
     %1 = OpConvertFToS %v2int %30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_1 : vec2i = vec2i(x_30);"));
}

TEST_F(SpvUnaryConversionTest, ConvertFToS_Vector_ToUnsigned) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %v2float %v2float_50_60
     %1 = OpConvertFToS %v2uint %30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_1 : vec2u = bitcast<vec2u>(vec2i(x_30));"));
}

TEST_F(SpvUnaryConversionTest, ConvertFToU_Scalar_BadArgType) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpConvertFToU %int %uint_10
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_THAT(p->error(), HasSubstr("operand for conversion to unsigned integer must be floating "
                                      "point scalar or vector"));
}

TEST_F(SpvUnaryConversionTest, ConvertFToU_Vector_BadArgType) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpConvertFToU %v2float %v2bool_t_f
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_THAT(p->error(), HasSubstr("operand for conversion to unsigned integer must be floating "
                                      "point scalar or vector"));
}

TEST_F(SpvUnaryConversionTest, ConvertFToU_Scalar_ToSigned_IsError) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %float %float_50
     %1 = OpConvertFToU %int %30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_FALSE(p->Parse());
    EXPECT_FALSE(p->success());
    EXPECT_THAT(p->error(), HasSubstr("Expected unsigned int scalar or vector "
                                      "type as Result Type: ConvertFToU"));
}

TEST_F(SpvUnaryConversionTest, ConvertFToU_Scalar_ToUnsigned) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %float %float_50
     %1 = OpConvertFToU %uint %30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_1 : u32 = u32(x_30);"));
}

TEST_F(SpvUnaryConversionTest, ConvertFToU_Vector_ToSigned_IsError) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %v2float %v2float_50_60
     %1 = OpConvertFToU %v2int %30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_FALSE(p->Parse());
    EXPECT_FALSE(p->success());
    EXPECT_THAT(p->error(), HasSubstr("Expected unsigned int scalar or vector "
                                      "type as Result Type: ConvertFToU"));
}

TEST_F(SpvUnaryConversionTest, ConvertFToU_Vector_ToUnsigned) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %v2float %v2float_50_60
     %1 = OpConvertFToU %v2uint %30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_1 : vec2u = vec2u(x_30);"));
}

TEST_F(SpvUnaryConversionTest, ConvertFToU_HoistedValue) {
    // From crbug.com/tint/804
    const auto assembly = Preamble() + R"(

%100 = OpFunction %void None %voidfn
%10 = OpLabel
OpBranch %30

%30 = OpLabel
OpLoopMerge %90 %80 None
OpBranchConditional %true %90 %40

%40 = OpLabel
OpSelectionMerge %50 None
OpBranchConditional %true %45 %50

%45 = OpLabel
; This value is hoisted
%600 = OpCopyObject %float %float_50
OpBranch %50

%50 = OpLabel
OpBranch %90

%80 = OpLabel ; unreachable continue target
%82 = OpConvertFToU %uint %600
OpBranch %30 ; backedge

%90 = OpLabel
OpReturn
OpFunctionEnd

  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_82 : u32 = u32(x_600);"));
}

// TODO(dneto): OpSConvert // only if multiple widths
// TODO(dneto): OpUConvert // only if multiple widths
// TODO(dneto): OpFConvert // only if multiple widths
// TODO(dneto): OpSatConvertSToU // Kernel (OpenCL), not in WebGPU
// TODO(dneto): OpSatConvertUToS // Kernel (OpenCL), not in WebGPU

}  // namespace
}  // namespace tint::reader::spirv
