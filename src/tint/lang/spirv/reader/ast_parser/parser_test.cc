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
