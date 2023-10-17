// Copyright 2020 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/lang/spirv/reader/ast_parser/parse.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/tint/lang/spirv/reader/ast_parser/spirv_tools_helpers_test.h"

namespace tint::spirv::reader::ast_parser {
namespace {

using ParserTest = testing::Test;

TEST_F(ParserTest, DataEmpty) {
    std::vector<uint32_t> data;
    auto program = Parse(data, {});
    auto errs = program.Diagnostics().str();
    ASSERT_FALSE(program.IsValid()) << errs;
    EXPECT_EQ(errs, "error: line:0: Invalid SPIR-V magic number.");
}

constexpr auto kShaderWithNonUniformDerivative = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %foo "foo" %x
               OpExecutionMode %foo OriginUpperLeft
               OpDecorate %x Location 0
      %float = OpTypeFloat 32
%_ptr_Input_float = OpTypePointer Input %float
          %x = OpVariable %_ptr_Input_float Input
       %void = OpTypeVoid
    %float_0 = OpConstantNull %float
       %bool = OpTypeBool
  %func_type = OpTypeFunction %void
        %foo = OpFunction %void None %func_type
  %foo_start = OpLabel
    %x_value = OpLoad %float %x
  %condition = OpFOrdGreaterThan %bool %x_value %float_0
               OpSelectionMerge %merge None
               OpBranchConditional %condition %true_branch %merge
%true_branch = OpLabel
     %result = OpDPdx %float %x_value
               OpBranch %merge
      %merge = OpLabel
               OpReturn
               OpFunctionEnd
)";

TEST_F(ParserTest, AllowNonUniformDerivatives_False) {
    auto spv = test::Assemble(kShaderWithNonUniformDerivative);
    Options options;
    options.allow_non_uniform_derivatives = false;
    auto program = Parse(spv, options);
    auto errs = program.Diagnostics().str();
    EXPECT_FALSE(program.IsValid()) << errs;
    EXPECT_THAT(errs, ::testing::HasSubstr("'dpdx' must only be called from uniform control flow"));
}

TEST_F(ParserTest, AllowNonUniformDerivatives_True) {
    auto spv = test::Assemble(kShaderWithNonUniformDerivative);
    Options options;
    options.allow_non_uniform_derivatives = true;
    auto program = Parse(spv, options);
    auto errs = program.Diagnostics().str();
    EXPECT_TRUE(program.IsValid()) << errs;
    EXPECT_EQ(program.Diagnostics().count(), 0u) << errs;
}

constexpr auto kShaderWithReadWriteStorageTexture = R"(
               OpCapability Shader
               OpCapability StorageImageExtendedFormats
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %100 "main"
               OpExecutionMode %100 LocalSize 8 8 1
               OpSource HLSL 600
               OpName %type_2d_image "type.2d.image"
               OpName %RWTexture2D "RWTexture2D"
               OpName %100 "main"
               OpDecorate %RWTexture2D DescriptorSet 0
               OpDecorate %RWTexture2D Binding 0
      %float = OpTypeFloat 32
    %float_0 = OpConstant %float 0
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %v2uint = OpTypeVector %uint 2
      %coord = OpConstantComposite %v2uint %uint_1 %uint_1
%type_2d_image = OpTypeImage %float 2D 2 0 0 2 Rgba32f
%_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
       %void = OpTypeVoid
         %20 = OpTypeFunction %void
%RWTexture2D = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
        %100 = OpFunction %void None %20
         %22 = OpLabel
         %30 = OpLoad %type_2d_image %RWTexture2D
         %31 = OpImageRead %v4float %30 %coord None
         %32 = OpFAdd %v4float %31 %31
               OpImageWrite %30 %coord %32 None
               OpReturn
               OpFunctionEnd
  )";

TEST_F(ParserTest, AllowChromiumExtensions_False) {
    auto spv = test::Assemble(kShaderWithReadWriteStorageTexture);
    Options options;
    options.allow_chromium_extensions = false;
    auto program = Parse(spv, options);
    auto errs = program.Diagnostics().str();
    EXPECT_FALSE(program.IsValid()) << errs;
    EXPECT_THAT(errs,
                ::testing::HasSubstr(
                    "error: module requires chromium_experimental_read_write_storage_texture, but "
                    "'allow-chromium-extensions' was not passed"));
}

TEST_F(ParserTest, AllowChromiumExtensions_True) {
    auto spv = test::Assemble(kShaderWithReadWriteStorageTexture);
    Options options;
    options.allow_chromium_extensions = true;
    auto program = Parse(spv, options);
    auto errs = program.Diagnostics().str();
    EXPECT_TRUE(program.IsValid()) << errs;
    EXPECT_EQ(program.Diagnostics().count(), 0u) << errs;
}

// TODO(dneto): uint32 vec, valid SPIR-V
// TODO(dneto): uint32 vec, invalid SPIR-V

}  // namespace
}  // namespace tint::spirv::reader::ast_parser
