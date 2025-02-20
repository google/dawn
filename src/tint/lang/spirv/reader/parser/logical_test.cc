// Copyright 2025 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/reader/parser/helper_test.h"

namespace tint::spirv::reader {
namespace {

struct SpirvLogicalParam {
    std::string spv_name;
    std::string wgsl_name;
};
[[maybe_unused]] inline std::ostream& operator<<(std::ostream& out, SpirvLogicalParam c) {
    out << c.spv_name;
    return out;
}

using SpirvParser_LogicalTest = SpirvParserTestWithParam<SpirvLogicalParam>;
TEST_P(SpirvParser_LogicalTest, FOrd_Scalar) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
        %f32 = OpTypeFloat 32
        %one = OpConstant %f32 1
        %two = OpConstant %f32 2
     %v2bool = OpTypeVector %bool 2
    %v2float = OpTypeVector %f32 2
      %v2one = OpConstantComposite %v2float %one %one
      %v2two = OpConstantComposite %v2float %two %two
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = OpFOrd)" +
                  params.spv_name + R"( %bool %one %two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = )" + params.wgsl_name +
                  R"( 1.0f, 2.0f
    ret
  }
}
)");
}

TEST_P(SpirvParser_LogicalTest, FOrd_Vector) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
        %f32 = OpTypeFloat 32
        %one = OpConstant %f32 1
        %two = OpConstant %f32 2
     %v2bool = OpTypeVector %bool 2
    %v2float = OpTypeVector %f32 2
      %v2one = OpConstantComposite %v2float %one %one
      %v2two = OpConstantComposite %v2float %two %two
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = OpFOrd)" +
                  params.spv_name + R"( %v2bool %v2one %v2two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<bool> = )" +
                  params.wgsl_name + R"( vec2<f32>(1.0f), vec2<f32>(2.0f)
    ret
  }
}
)");
}

INSTANTIATE_TEST_SUITE_P(SpirvParser,
                         SpirvParser_LogicalTest,
                         testing::Values(SpirvLogicalParam{"Equal", "eq"},
                                         SpirvLogicalParam{"NotEqual", "neq"},
                                         SpirvLogicalParam{"GreaterThan", "gt"},
                                         SpirvLogicalParam{"GreaterThanEqual", "gte"},
                                         SpirvLogicalParam{"LessThan", "lt"},
                                         SpirvLogicalParam{"LessThanEqual", "lte"}));

}  // namespace
}  // namespace tint::spirv::reader
