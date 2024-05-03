// Copyright 2024 The Dawn & Tint Authors
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

struct BinaryCase {
    std::string spirv_type;
    std::string spirv_opcode;
    std::string ir;
};
std::string PrintBuiltinCase(testing::TestParamInfo<BinaryCase> bc) {
    return bc.param.spirv_opcode + "_" + bc.param.spirv_type;
}

using BinaryTest = SpirvParserTestWithParam<BinaryCase>;

TEST_P(BinaryTest, All) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %u32 = OpTypeInt 32 0
        %f16 = OpTypeFloat 16
        %f32 = OpTypeFloat 32
      %vec3i = OpTypeVector %i32 3
      %vec4u = OpTypeVector %u32 4
      %vec3h = OpTypeVector %f16 3
      %vec4f = OpTypeVector %f32 4
    %ep_type = OpTypeFunction %void
    %fn_type = OpTypeFunction %)" +
                  params.spirv_type + " %" + params.spirv_type + " %" + params.spirv_type + R"(
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd

        %foo = OpFunction %)" +
                  params.spirv_type + " " + R"( None %fn_type
        %lhs = OpFunctionParameter %)" +
                  params.spirv_type + " " + R"(
        %rhs = OpFunctionParameter %)" +
                  params.spirv_type + " " + R"(
  %foo_start = OpLabel
     %result = )" +
                  params.spirv_opcode + R"( %)" + params.spirv_type + " " + R"( %lhs %rhs
               OpReturnValue %result
               OpFunctionEnd
)",
              R"(
  $B2: {
    )" + params.ir +
                  R"(
    ret %5
  }
)");
}

INSTANTIATE_TEST_SUITE_P(SpirvParser,
                         BinaryTest,
                         testing::Values(
                             // OpFAdd
                             BinaryCase{
                                 "f16",
                                 "OpFAdd",
                                 "%5:f16 = add %3, %4",
                             },
                             BinaryCase{
                                 "f32",
                                 "OpFAdd",
                                 "%5:f32 = add %3, %4",
                             },
                             BinaryCase{
                                 "vec3h",
                                 "OpFAdd",
                                 "%5:vec3<f16> = add %3, %4",
                             },
                             BinaryCase{
                                 "vec4f",
                                 "OpFAdd",
                                 "%5:vec4<f32> = add %3, %4",
                             },

                             // OpFMul
                             BinaryCase{
                                 "f16",
                                 "OpFMul",
                                 "%5:f16 = mul %3, %4",
                             },
                             BinaryCase{
                                 "f32",
                                 "OpFMul",
                                 "%5:f32 = mul %3, %4",
                             },
                             BinaryCase{
                                 "vec3h",
                                 "OpFMul",
                                 "%5:vec3<f16> = mul %3, %4",
                             },
                             BinaryCase{
                                 "vec4f",
                                 "OpFMul",
                                 "%5:vec4<f32> = mul %3, %4",
                             },

                             // OpIAdd
                             BinaryCase{
                                 "i32",
                                 "OpIAdd",
                                 "%5:i32 = add %3, %4",
                             },
                             BinaryCase{
                                 "u32",
                                 "OpIAdd",
                                 "%5:u32 = add %3, %4",
                             },
                             BinaryCase{
                                 "vec3i",
                                 "OpIAdd",
                                 "%5:vec3<i32> = add %3, %4",
                             },
                             BinaryCase{
                                 "vec4u",
                                 "OpIAdd",
                                 "%5:vec4<u32> = add %3, %4",
                             }),
                         PrintBuiltinCase);

}  // namespace
}  // namespace tint::spirv::reader
