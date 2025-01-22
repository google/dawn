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

std::string Preamble() {
    return R"(
  OpCapability Shader
  %glsl = OpExtInstImport "GLSL.std.450"
  OpMemoryModel Logical GLSL450
  OpEntryPoint GLCompute %100 "main"
  OpExecutionMode %100 LocalSize 1 1 1

  %void = OpTypeVoid
  %voidfn = OpTypeFunction %void

  %uint = OpTypeInt 32 0
  %int = OpTypeInt 32 1
  %float = OpTypeFloat 32

  %v2int = OpTypeVector %int 2
  %v2uint = OpTypeVector %uint 2
  %v2float = OpTypeVector %float 2
  %v3float = OpTypeVector %float 3
  %v4float = OpTypeVector %float 4
  %mat2v2float = OpTypeMatrix %v2float 2
  %mat3v3float = OpTypeMatrix %v3float 3
  %mat4v4float = OpTypeMatrix %v4float 4

  %int_10 = OpConstant %int 10
  %int_20 = OpConstant %int 20

  %uint_10 = OpConstant %uint 10
  %uint_20 = OpConstant %uint 20

  %float_50 = OpConstant %float 50
  %float_60 = OpConstant %float 60
  %float_70 = OpConstant %float 70

  %v2int_10_20 = OpConstantComposite %v2int %int_10 %int_20
  %v2uint_10_20 = OpConstantComposite %v2uint %uint_10 %uint_20

  %v2int_20_10 = OpConstantComposite %v2int %int_20 %int_10
  %v2uint_20_10 = OpConstantComposite %v2uint %uint_20 %uint_10

  %v2float_50_60 = OpConstantComposite %v2float %float_50 %float_60
  %v3float_50_60_70 = OpConstantComposite %v3float %float_50 %float_60 %float_70
  %v4float_50_50_50_50 = OpConstantComposite %v4float %float_50 %float_50 %float_50 %float_50

  %mat2v2float_50_60 = OpConstantComposite %mat2v2float %v2float_50_60 %v2float_50_60
  %mat3v3float_50_60_70 = OpConstantComposite %mat3v3float %v3float_50_60_70 %v3float_50_60_70 %v3float_50_60_70
  %mat4v4float_50_50_50_50 = OpConstantComposite %mat4v4float %v4float_50_50_50_50 %v4float_50_50_50_50 %v4float_50_50_50_50 %v4float_50_50_50_50

  %100 = OpFunction %void None %voidfn
  %entry = OpLabel
)";
}

TEST_F(SpirvParserTest, GlslStd450_MatrixInverse_mat2x2) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %mat2v2float %glsl MatrixInverse %mat2v2float_50_60
     %2 = OpCopyObject %mat2v2float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat2x2<f32> = spirv.inverse mat2x2<f32>(vec2<f32>(50.0f, 60.0f))
    %3:mat2x2<f32> = let %2
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, GlslStd450_MatrixInverse_mat3x3) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %mat3v3float %glsl MatrixInverse %mat3v3float_50_60_70
     %2 = OpCopyObject %mat3v3float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat3x3<f32> = spirv.inverse mat3x3<f32>(vec3<f32>(50.0f, 60.0f, 70.0f))
    %3:mat3x3<f32> = let %2
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, GlslStd450_MatrixInverse_mat4x4) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %mat4v4float %glsl MatrixInverse %mat4v4float_50_50_50_50
     %2 = OpCopyObject %mat4v4float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat4x4<f32> = spirv.inverse mat4x4<f32>(vec4<f32>(50.0f))
    %3:mat4x4<f32> = let %2
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, GlslStd450_MatrixInverse_MultipleInScope) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %mat2v2float %glsl MatrixInverse %mat2v2float_50_60
     %2 = OpExtInst %mat2v2float %glsl MatrixInverse %mat2v2float_50_60
     %3 = OpCopyObject %mat2v2float %1
     %4 = OpCopyObject %mat2v2float %2
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat2x2<f32> = spirv.inverse mat2x2<f32>(vec2<f32>(50.0f, 60.0f))
    %3:mat2x2<f32> = spirv.inverse mat2x2<f32>(vec2<f32>(50.0f, 60.0f))
    %4:mat2x2<f32> = let %2
    %5:mat2x2<f32> = let %3
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, GlslStd450_SAbs_UnsignedToUnsigned) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %uint %glsl SAbs %uint_10
     %2 = OpExtInst %v2uint %glsl SAbs %v2uint_10_20
     %3 = OpCopyObject %uint %1
     %4 = OpCopyObject %v2uint %2
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.abs<u32> 10u
    %3:vec2<u32> = spirv.abs<u32> vec2<u32>(10u, 20u)
    %4:u32 = let %2
    %5:vec2<u32> = let %3
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, GlslStd450_SAbs_UnsignedToSigned) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %int %glsl SAbs %uint_10
     %2 = OpExtInst %v2int %glsl SAbs %v2uint_10_20
     %3 = OpCopyObject %int %1
     %4 = OpCopyObject %v2int %2
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.abs<i32> 10u
    %3:vec2<i32> = spirv.abs<i32> vec2<u32>(10u, 20u)
    %4:i32 = let %2
    %5:vec2<i32> = let %3
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, GlslStd450_SAbs_SignedToUnsigned) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %uint %glsl SAbs %int_10
     %2 = OpExtInst %v2uint %glsl SAbs %v2int_10_20
     %3 = OpCopyObject %uint %1
     %4 = OpCopyObject %v2uint %2
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.abs<u32> 10i
    %3:vec2<u32> = spirv.abs<u32> vec2<i32>(10i, 20i)
    %4:u32 = let %2
    %5:vec2<u32> = let %3
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, GlslStd450_SAbs_SignedToSigned) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %int %glsl SAbs %int_10
     %2 = OpExtInst %v2int %glsl SAbs %v2int_10_20
     %3 = OpCopyObject %int %1
     %4 = OpCopyObject %v2int %2
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.abs<i32> 10i
    %3:vec2<i32> = spirv.abs<i32> vec2<i32>(10i, 20i)
    %4:i32 = let %2
    %5:vec2<i32> = let %3
    ret
  }
}
)");
}

struct GlslStd450TwoParams {
    std::string spv_name;
    std::string ir_name;
};
[[maybe_unused]] inline std::ostream& operator<<(std::ostream& out, GlslStd450TwoParams c) {
    out << c.spv_name;
    return out;
}

using GlslStd450TwoParamTest = SpirvParserTestWithParam<GlslStd450TwoParams>;

TEST_P(GlslStd450TwoParamTest, UnsignedToUnsigned) {
    auto params = GetParam();

    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %uint %glsl )" +
                  params.spv_name + R"( %uint_10 %uint_10
     %2 = OpExtInst %v2uint %glsl )" +
                  params.spv_name + R"( %v2uint_10_20 %v2uint_20_10
     %3 = OpCopyObject %uint %1
     %4 = OpCopyObject %v2uint %2
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
                  params.ir_name + R"(<u32> 10u, 10u
    %3:vec2<u32> = spirv.)" +
                  params.ir_name + R"(<u32> vec2<u32>(10u, 20u), vec2<u32>(20u, 10u)
    %4:u32 = let %2
    %5:vec2<u32> = let %3
    ret
  }
}
)");
}

TEST_P(GlslStd450TwoParamTest, SignedToSigned) {
    auto params = GetParam();

    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %int %glsl )" +
                  params.spv_name + R"( %int_10 %int_10
     %2 = OpExtInst %v2int %glsl )" +
                  params.spv_name + R"( %v2int_10_20 %v2int_20_10
     %3 = OpCopyObject %int %1
     %4 = OpCopyObject %v2int %2
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
                  params.ir_name + R"(<i32> 10i, 10i
    %3:vec2<i32> = spirv.)" +
                  params.ir_name + R"(<i32> vec2<i32>(10i, 20i), vec2<i32>(20i, 10i)
    %4:i32 = let %2
    %5:vec2<i32> = let %3
    ret
  }
}
)");
}

TEST_P(GlslStd450TwoParamTest, MixedToUnsigned) {
    auto params = GetParam();

    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %uint %glsl )" +
                  params.spv_name + R"( %int_10 %uint_10
     %2 = OpExtInst %v2uint %glsl )" +
                  params.spv_name + R"( %v2int_10_20 %v2uint_20_10
     %3 = OpCopyObject %uint %1
     %4 = OpCopyObject %v2uint %2
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
                  params.ir_name + R"(<u32> 10i, 10u
    %3:vec2<u32> = spirv.)" +
                  params.ir_name + R"(<u32> vec2<i32>(10i, 20i), vec2<u32>(20u, 10u)
    %4:u32 = let %2
    %5:vec2<u32> = let %3
    ret
  }
}
)");
}

TEST_P(GlslStd450TwoParamTest, MixedToSigned) {
    auto params = GetParam();

    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %int %glsl )" +
                  params.spv_name + R"( %uint_10 %int_10
     %2 = OpExtInst %v2int %glsl )" +
                  params.spv_name + R"( %v2uint_10_20 %v2int_20_10
     %3 = OpCopyObject %int %1
     %4 = OpCopyObject %v2int %2
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
                  params.ir_name + R"(<i32> 10u, 10i
    %3:vec2<i32> = spirv.)" +
                  params.ir_name + R"(<i32> vec2<u32>(10u, 20u), vec2<i32>(20i, 10i)
    %4:i32 = let %2
    %5:vec2<i32> = let %3
    ret
  }
}
)");
}

INSTANTIATE_TEST_SUITE_P(SpirvParser,
                         GlslStd450TwoParamTest,
                         ::testing::Values(GlslStd450TwoParams{"SMax", "smax"},
                                           GlslStd450TwoParams{"SMin", "smin"},
                                           GlslStd450TwoParams{"UMax", "umax"}));

TEST_F(SpirvParserTest, GlslStd450_SClamp_UnsignedToUnsigned) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %uint %glsl SClamp %uint_10 %uint_10 %uint_10
     %2 = OpExtInst %v2uint %glsl SClamp %v2uint_10_20 %v2uint_20_10 %v2uint_20_10
     %3 = OpCopyObject %uint %1
     %4 = OpCopyObject %v2uint %2
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.sclamp<u32> 10u, 10u, 10u
    %3:vec2<u32> = spirv.sclamp<u32> vec2<u32>(10u, 20u), vec2<u32>(20u, 10u), vec2<u32>(20u, 10u)
    %4:u32 = let %2
    %5:vec2<u32> = let %3
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, GlslStd450_SClamp_SignedToSigned) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %int %glsl SClamp %int_10 %int_10 %int_10
     %2 = OpExtInst %v2int %glsl SClamp %v2int_10_20 %v2int_20_10 %v2int_20_10
     %3 = OpCopyObject %int %1
     %4 = OpCopyObject %v2int %2
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.sclamp<i32> 10i, 10i, 10i
    %3:vec2<i32> = spirv.sclamp<i32> vec2<i32>(10i, 20i), vec2<i32>(20i, 10i), vec2<i32>(20i, 10i)
    %4:i32 = let %2
    %5:vec2<i32> = let %3
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, GlslStd450_SClamp_MixedToUnsigned) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %uint %glsl SClamp %int_10 %uint_10 %int_10
     %2 = OpExtInst %v2uint %glsl SClamp %v2int_10_20 %v2uint_20_10 %v2int_10_20
     %3 = OpCopyObject %uint %1
     %4 = OpCopyObject %v2uint %2
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.sclamp<u32> 10i, 10u, 10i
    %3:vec2<u32> = spirv.sclamp<u32> vec2<i32>(10i, 20i), vec2<u32>(20u, 10u), vec2<i32>(10i, 20i)
    %4:u32 = let %2
    %5:vec2<u32> = let %3
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, GlslStd450_SClamp_MixedToSigned) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %int %glsl SClamp %uint_10 %int_10 %uint_10
     %2 = OpExtInst %v2int %glsl SClamp %v2uint_10_20 %v2int_20_10 %v2uint_10_20
     %3 = OpCopyObject %int %1
     %4 = OpCopyObject %v2int %2
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.sclamp<i32> 10u, 10i, 10u
    %3:vec2<i32> = spirv.sclamp<i32> vec2<u32>(10u, 20u), vec2<i32>(20i, 10i), vec2<u32>(10u, 20u)
    %4:i32 = let %2
    %5:vec2<i32> = let %3
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::spirv::reader
