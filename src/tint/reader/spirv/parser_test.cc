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

#include "src/tint/reader/spirv/parser.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/tint/reader/spirv/spirv_tools_helpers_test.h"

namespace tint::reader::spirv {
namespace {

using ParserTest = testing::Test;

TEST_F(ParserTest, DataEmpty) {
    std::vector<uint32_t> data;
    auto program = Parse(data);
    auto errs = diag::Formatter().format(program.Diagnostics());
    ASSERT_FALSE(program.IsValid()) << errs;
    EXPECT_EQ(errs, "error: line:0: Invalid SPIR-V magic number.\n");
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
    auto errs = diag::Formatter().format(program.Diagnostics());
    EXPECT_FALSE(program.IsValid()) << errs;
    EXPECT_THAT(errs, ::testing::HasSubstr("'dpdx' must only be called from uniform control flow"));
}

TEST_F(ParserTest, AllowNonUniformDerivatives_True) {
    auto spv = test::Assemble(kShaderWithNonUniformDerivative);
    Options options;
    options.allow_non_uniform_derivatives = true;
    auto program = Parse(spv, options);
    auto errs = diag::Formatter().format(program.Diagnostics());
    EXPECT_TRUE(program.IsValid()) << errs;
    EXPECT_EQ(program.Diagnostics().count(), 0u) << errs;
}

// TODO(dneto): uint32 vec, valid SPIR-V
// TODO(dneto): uint32 vec, invalid SPIR-V

}  // namespace
}  // namespace tint::reader::spirv
