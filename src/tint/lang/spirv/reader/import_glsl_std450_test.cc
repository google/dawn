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

#include "src/tint/lang/spirv/reader/helper_test.h"

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

  %uint_10 = OpConstant %uint 10
  %uint_15 = OpConstant %uint 15
  %uint_20 = OpConstant %uint 20
  %int_30 = OpConstant %int 30
  %int_35 = OpConstant %int 35
  %int_40 = OpConstant %int 40
  %float_50 = OpConstant %float 50
  %float_60 = OpConstant %float 60
  %float_70 = OpConstant %float 70

  %v2uint = OpTypeVector %uint 2
  %v2int = OpTypeVector %int 2
  %v2float = OpTypeVector %float 2
  %v3float = OpTypeVector %float 3
  %v4float = OpTypeVector %float 4
  %mat2v2float = OpTypeMatrix %v2float 2
  %mat3v3float = OpTypeMatrix %v3float 3
  %mat4v4float = OpTypeMatrix %v4float 4

  %v2uint_10_20 = OpConstantComposite %v2uint %uint_10 %uint_20
  %v2uint_20_10 = OpConstantComposite %v2uint %uint_20 %uint_10
  %v2uint_15_15 = OpConstantComposite %v2uint %uint_15 %uint_15
  %v2int_30_40 = OpConstantComposite %v2int %int_30 %int_40
  %v2int_40_30 = OpConstantComposite %v2int %int_40 %int_30
  %v2int_35_35 = OpConstantComposite %v2int %int_35 %int_35
  %v2float_50_60 = OpConstantComposite %v2float %float_50 %float_60
  %v2float_60_50 = OpConstantComposite %v2float %float_60 %float_50
  %v2float_70_70 = OpConstantComposite %v2float %float_70 %float_70

  %v3float_50_60_70 = OpConstantComposite %v3float %float_50 %float_60 %float_70
  %v3float_60_70_50 = OpConstantComposite %v3float %float_60 %float_70 %float_50

  %v4float_50_50_50_50 = OpConstantComposite %v4float %float_50 %float_50 %float_50 %float_50

  %mat2v2float_50_60 = OpConstantComposite %mat2v2float %v2float_50_60 %v2float_50_60
  %mat3v3float_50_60_70 = OpConstantComposite %mat3v3float %v3float_50_60_70 %v3float_50_60_70 %v3float_50_60_70
  %mat4v4float_50_50_50_50 = OpConstantComposite %mat4v4float %v4float_50_50_50_50 %v4float_50_50_50_50 %v4float_50_50_50_50 %v4float_50_50_50_50

  %100 = OpFunction %void None %voidfn
  %entry = OpLabel
)";
}

struct GlslStd450Case {
    std::string opcode;
    std::string wgsl_func;
};
inline std::ostream& operator<<(std::ostream& out, GlslStd450Case c) {
    out << "GlslStd450Case(" << c.opcode << " " << c.wgsl_func << ")";
    return out;
}

// Nomenclature:
// Float = scalar float
// Floating = scalar float or vector-of-float
// Float3 = 3-element vector of float
// Int = scalar signed int
// Inting = scalar int or vector-of-int
// Uint = scalar unsigned int
// Uinting = scalar unsigned or vector-of-unsigned

using SpirvReaderTest_GlslStd450_Float_Floating = SpirvReaderTestWithParam<GlslStd450Case>;
using SpirvReaderTest_GlslStd450_Float_FloatingFloating = SpirvReaderTestWithParam<GlslStd450Case>;
using SpirvReaderTest_GlslStd450_Floating_Floating = SpirvReaderTestWithParam<GlslStd450Case>;
using SpirvReaderTest_GlslStd450_Floating_FloatingFloating =
    SpirvReaderTestWithParam<GlslStd450Case>;
using SpirvReaderTest_GlslStd450_Floating_FloatingFloatingFloating =
    SpirvReaderTestWithParam<GlslStd450Case>;
using SpirvReaderTest_GlslStd450_Floating_FloatingInting = SpirvReaderTestWithParam<GlslStd450Case>;
using SpirvReaderTest_GlslStd450_Float3_Float3Float3 = SpirvReaderTestWithParam<GlslStd450Case>;

using SpirvReaderTest_GlslStd450_Inting_Inting = SpirvReaderTestWithParam<GlslStd450Case>;
using SpirvReaderTest_GlslStd450_Inting_Inting_SignednessCoercing =
    SpirvReaderTestWithParam<GlslStd450Case>;
using SpirvReaderTest_GlslStd450_Inting_IntingInting = SpirvReaderTestWithParam<GlslStd450Case>;
using SpirvReaderTest_GlslStd450_Inting_IntingIntingInting =
    SpirvReaderTestWithParam<GlslStd450Case>;
using SpirvReaderTest_GlslStd450_Uinting_Uinting = SpirvReaderTestWithParam<GlslStd450Case>;
using SpirvReaderTest_GlslStd450_Uinting_UintingUinting = SpirvReaderTestWithParam<GlslStd450Case>;
using SpirvReaderTest_GlslStd450_Uinting_UintingUintingUinting =
    SpirvReaderTestWithParam<GlslStd450Case>;

TEST_P(SpirvReaderTest_GlslStd450_Float_Floating, Scalar) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %float %glsl )" +
                  GetParam().opcode + R"( %float_50
     %2 = OpCopyObject %float %1
     OpReturn
     OpFunctionEnd
  )",

              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = )" + GetParam().wgsl_func +
                  R"( 50.0f
    %3:f32 = let %2
    ret
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Float_Floating, Vector) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %float %glsl )" +
                  GetParam().opcode + R"( %v2float_50_60
     %2 = OpCopyObject %float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = )" + GetParam().wgsl_func +
                  R"( vec2<f32>(50.0f, 60.0f)
    %3:f32 = let %2
    ret
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Float_FloatingFloating, Scalar) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %float %glsl )" +
                  GetParam().opcode + R"( %float_50 %float_60
     %2 = OpCopyObject %float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = )" + GetParam().wgsl_func +
                  R"( 50.0f, 60.0f
    %3:f32 = let %2
    ret
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Float_FloatingFloating, Vector) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %float %glsl )" +
                  GetParam().opcode + R"( %v2float_50_60 %v2float_60_50
     %2 = OpCopyObject %float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = )" + GetParam().wgsl_func +
                  R"( vec2<f32>(50.0f, 60.0f), vec2<f32>(60.0f, 50.0f)
    %3:f32 = let %2
    ret
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Floating_Floating, Scalar) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %float %glsl )" +
                  GetParam().opcode + R"( %float_50
     %2 = OpCopyObject %float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = )" + GetParam().wgsl_func +
                  R"( 50.0f
    %3:f32 = let %2
    ret
  }
})");
}

TEST_P(SpirvReaderTest_GlslStd450_Floating_Floating, Vector) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %v2float %glsl )" +
                  GetParam().opcode + R"( %v2float_50_60
     %2 = OpCopyObject %v2float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = )" +
                  GetParam().wgsl_func +
                  R"( vec2<f32>(50.0f, 60.0f)
    %3:vec2<f32> = let %2
    ret
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Floating_FloatingFloating, Scalar) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %float %glsl )" +
                  GetParam().opcode + R"( %float_50 %float_60
     %2 = OpCopyObject %float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = )" + GetParam().wgsl_func +
                  R"( 50.0f, 60.0f
    %3:f32 = let %2
    ret
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Floating_FloatingFloating, Vector) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %v2float %glsl )" +
                  GetParam().opcode + R"( %v2float_50_60 %v2float_60_50
     %2 = OpCopyObject %v2float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = )" +
                  GetParam().wgsl_func +
                  R"( vec2<f32>(50.0f, 60.0f), vec2<f32>(60.0f, 50.0f)
    %3:vec2<f32> = let %2
    ret
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Floating_FloatingFloatingFloating, Scalar) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %float %glsl )" +
                  GetParam().opcode + R"( %float_50 %float_60 %float_70
     %2 = OpCopyObject %float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = )" + GetParam().wgsl_func +
                  R"( 50.0f, 60.0f, 70.0f
    %3:f32 = let %2
    ret
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Floating_FloatingFloatingFloating, Vector) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %v2float %glsl )" +
                  GetParam().opcode +
                  R"( %v2float_50_60 %v2float_60_50 %v2float_70_70
     %2 = OpCopyObject %v2float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = )" +
                  GetParam().wgsl_func +
                  R"( vec2<f32>(50.0f, 60.0f), vec2<f32>(60.0f, 50.0f), vec2<f32>(70.0f)
    %3:vec2<f32> = let %2
    ret
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Floating_FloatingInting, Scalar) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %float %glsl )" +
                  GetParam().opcode + R"( %float_50 %int_30
     %2 = OpCopyObject %float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = )" + GetParam().wgsl_func +
                  R"( 50.0f, 30i
    %3:f32 = let %2
    ret
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Floating_FloatingInting, Vector) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %v2float %glsl )" +
                  GetParam().opcode +
                  R"( %v2float_50_60 %v2int_30_40
     %2 = OpCopyObject %v2float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = )" +
                  GetParam().wgsl_func +
                  R"( vec2<f32>(50.0f, 60.0f), vec2<i32>(30i, 40i)
    %3:vec2<f32> = let %2
    ret
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Float3_Float3Float3, SpirvParser) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %v3float %glsl )" +
                  GetParam().opcode +
                  R"( %v3float_50_60_70 %v3float_60_70_50
     %2 = OpCopyObject %v3float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<f32> = )" +
                  GetParam().wgsl_func +
                  R"( vec3<f32>(50.0f, 60.0f, 70.0f), vec3<f32>(60.0f, 70.0f, 50.0f)
    %3:vec3<f32> = let %2
    ret
  }
}
)");
}

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReaderTest_GlslStd450_Float_Floating,
                         ::testing::Values(GlslStd450Case{"Length", "length"}));

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReaderTest_GlslStd450_Float_FloatingFloating,
                         ::testing::Values(GlslStd450Case{"Distance", "distance"}));

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReaderTest_GlslStd450_Floating_Floating,
                         ::testing::ValuesIn(std::vector<GlslStd450Case>{
                             {"Acos", "acos"},                //
                             {"Asin", "asin"},                //
                             {"Atan", "atan"},                //
                             {"Ceil", "ceil"},                //
                             {"Cos", "cos"},                  //
                             {"Cosh", "cosh"},                //
                             {"Degrees", "degrees"},          //
                             {"Exp", "exp"},                  //
                             {"Exp2", "exp2"},                //
                             {"FAbs", "abs"},                 //
                             {"FSign", "sign"},               //
                             {"Floor", "floor"},              //
                             {"Fract", "fract"},              //
                             {"InverseSqrt", "inverseSqrt"},  //
                             {"Log", "log"},                  //
                             {"Log2", "log2"},                //
                             {"Radians", "radians"},          //
                             {"Round", "round"},              //
                             {"RoundEven", "round"},          //
                             {"Sin", "sin"},                  //
                             {"Sinh", "sinh"},                //
                             {"Sqrt", "sqrt"},                //
                             {"Tan", "tan"},                  //
                             {"Tanh", "tanh"},                //
                             {"Trunc", "trunc"},              //
                         }));

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReaderTest_GlslStd450_Floating_FloatingFloating,
                         ::testing::ValuesIn(std::vector<GlslStd450Case>{
                             {"Atan2", "atan2"},
                             {"NMax", "max"},
                             {"NMin", "min"},
                             {"FMax", "max"},  // WGSL max promises more for NaN
                             {"FMin", "min"},  // WGSL min promises more for NaN
                             {"Pow", "pow"},
                             {"Step", "step"},
                         }));

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReaderTest_GlslStd450_Floating_FloatingInting,
                         ::testing::Values(GlslStd450Case{"Ldexp", "ldexp"}));
// For ldexp with unsigned second argument, see below.

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReaderTest_GlslStd450_Float3_Float3Float3,
                         ::testing::Values(GlslStd450Case{"Cross", "cross"}));

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReaderTest_GlslStd450_Floating_FloatingFloatingFloating,
                         ::testing::ValuesIn(std::vector<GlslStd450Case>{
                             {"NClamp", "clamp"},
                             {"FClamp", "clamp"},  // WGSL FClamp promises more for NaN
                             {"Fma", "fma"},
                             {"FMix", "mix"},
                             {"SmoothStep", "smoothstep"}}));

TEST_P(SpirvReaderTest_GlslStd450_Inting_Inting, Scalar) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %int %glsl )" +
                  GetParam().opcode +
                  R"( %int_30
     %2 = OpCopyObject %int %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = )" + GetParam().wgsl_func +
                  R"( 30i
    %3:i32 = let %2
    ret
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Inting_Inting_SignednessCoercing, DISABLED_Scalar_UnsignedArg) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %int %glsl )" +
                  GetParam().opcode +
                  R"( %uint_10
     %2 = OpCopyObject %int %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    let x_1 = )" + GetParam().wgsl_func +
                  R"((bitcast<i32>(u1));
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Inting_Inting_SignednessCoercing,
       DISABLED_Scalar_UnsignedResult) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %uint %glsl )" +
                  GetParam().opcode +
                  R"( %int_30
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    let x_1 = bitcast<u32>()" +
                  GetParam().wgsl_func + R"((i1));
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Inting_Inting, Vector) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %v2int %glsl )" +
                  GetParam().opcode +
                  R"( %v2int_30_40
     %2 = OpCopyObject %v2int %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = )" +
                  GetParam().wgsl_func +
                  R"( vec2<i32>(30i, 40i)
    %3:vec2<i32> = let %2
    ret
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Inting_Inting_SignednessCoercing, DISABLED_Vector_UnsignedArg) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %v2int %glsl )" +
                  GetParam().opcode +
                  R"( %v2uint_10_20
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    let x_1 = )" + GetParam().wgsl_func +
                  R"((bitcast<vec2i>(v2u1));
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Inting_Inting_SignednessCoercing,
       DISABLED_Vector_UnsignedResult) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %v2uint %glsl )" +
                  GetParam().opcode +
                  R"( %v2int_30_40
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    let x_1 = bitcast<vec2u>()" +
                  GetParam().wgsl_func + R"((v2i1));
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Inting_IntingInting, Scalar) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %int %glsl )" +
                  GetParam().opcode +
                  R"( %int_30 %int_35
     %2 = OpCopyObject %int %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = )" + GetParam().wgsl_func +
                  R"( 30i, 35i
    %3:i32 = let %2
    ret
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Inting_IntingInting, Vector) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %v2int %glsl )" +
                  GetParam().opcode +
                  R"( %v2int_30_40 %v2int_40_30
     %2 = OpCopyObject %v2int %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = )" +
                  GetParam().wgsl_func +
                  R"( vec2<i32>(30i, 40i), vec2<i32>(40i, 30i)
    %3:vec2<i32> = let %2
    ret
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Inting_IntingIntingInting, Scalar) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %int %glsl )" +
                  GetParam().opcode +
                  R"( %int_30 %int_35 %int_40
     %2 = OpCopyObject %int %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = )" + GetParam().wgsl_func +
                  R"( 30i, 35i, 40i
    %3:i32 = let %2
    ret
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Inting_IntingIntingInting, Vector) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %v2int %glsl )" +
                  GetParam().opcode +
                  R"( %v2int_30_40 %v2int_40_30 %v2int_35_35
     %2 = OpCopyObject %v2int %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = )" +
                  GetParam().wgsl_func +
                  R"( vec2<i32>(30i, 40i), vec2<i32>(40i, 30i), vec2<i32>(35i)
    %3:vec2<i32> = let %2
    ret
  }
}
)");
}

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReaderTest_GlslStd450_Inting_Inting,
                         ::testing::Values(GlslStd450Case{"SAbs", "abs"},
                                           GlslStd450Case{"FindILsb", "firstTrailingBit"},
                                           GlslStd450Case{"FindSMsb", "firstLeadingBit"},
                                           GlslStd450Case{"SSign", "sign"}));

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReaderTest_GlslStd450_Inting_Inting_SignednessCoercing,
                         ::testing::Values(GlslStd450Case{"SSign", "sign"}));

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReaderTest_GlslStd450_Inting_IntingInting,
                         ::testing::Values(GlslStd450Case{"SMax", "max"},
                                           GlslStd450Case{"SMin", "min"}));

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReaderTest_GlslStd450_Inting_IntingIntingInting,
                         ::testing::Values(GlslStd450Case{"SClamp", "clamp"}));

TEST_P(SpirvReaderTest_GlslStd450_Uinting_Uinting, Scalar) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %uint %glsl )" +
                  GetParam().opcode +
                  R"( %uint_10
     %2 = OpCopyObject %uint %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = )" + GetParam().wgsl_func +
                  R"( 10u
    %3:u32 = let %2
    ret
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Uinting_Uinting, Vector) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %v2uint %glsl )" +
                  GetParam().opcode +
                  R"( %v2uint_10_20
     %2 = OpCopyObject %v2uint %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = )" +
                  GetParam().wgsl_func +
                  R"( vec2<u32>(10u, 20u)
    %3:vec2<u32> = let %2
    ret
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Uinting_UintingUinting, Scalar) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %uint %glsl )" +
                  GetParam().opcode + R"( %uint_10 %uint_15
     %2 = OpCopyObject %uint %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = )" + GetParam().wgsl_func +
                  R"( 10u, 15u
    %3:u32 = let %2
    ret
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Uinting_UintingUinting, Vector) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %v2uint %glsl )" +
                  GetParam().opcode +
                  R"( %v2uint_10_20 %v2uint_20_10
     %2 = OpCopyObject %v2uint %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = )" +
                  GetParam().wgsl_func +
                  R"( vec2<u32>(10u, 20u), vec2<u32>(20u, 10u)
    %3:vec2<u32> = let %2
    ret
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Uinting_UintingUintingUinting, Scalar) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %uint %glsl )" +
                  GetParam().opcode + R"( %uint_10 %uint_15 %uint_20
     %2 = OpCopyObject %uint %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = )" + GetParam().wgsl_func +
                  R"( 10u, 15u, 20u
    %3:u32 = let %2
    ret
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Uinting_UintingUintingUinting, Vector) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %v2uint %glsl )" +
                  GetParam().opcode +
                  R"( %v2uint_10_20 %v2uint_20_10 %v2uint_15_15
     %2 = OpCopyObject %v2uint %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = )" +
                  GetParam().wgsl_func +
                  R"( vec2<u32>(10u, 20u), vec2<u32>(20u, 10u), vec2<u32>(15u)
    %3:vec2<u32> = let %2
    ret
  }
}
)");
}

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReaderTest_GlslStd450_Uinting_Uinting,
                         ::testing::Values(GlslStd450Case{"FindILsb", "firstTrailingBit"},
                                           GlslStd450Case{"FindUMsb", "firstLeadingBit"}));

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReaderTest_GlslStd450_Uinting_UintingUinting,
                         ::testing::Values(GlslStd450Case{"UMax", "max"},
                                           GlslStd450Case{"UMin", "min"}));

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReaderTest_GlslStd450_Uinting_UintingUintingUinting,
                         ::testing::Values(GlslStd450Case{"UClamp", "clamp"}));

// Test Normalize.  WGSL does not have a scalar form of the normalize builtin.
// So we have to test it separately, as it does not fit the patterns tested
// above.

TEST_F(SpirvReaderTest, Normalize_Scalar) {
    // Scalar normalize maps to sign.
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %float %glsl Normalize %float_50
     %2 = OpCopyObject %float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = sign 50.0f
    %3:f32 = let %2
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, Normalize_Vector2) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %v2float %glsl Normalize %v2float_50_60
     %2 = OpCopyObject %v2float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = normalize vec2<f32>(50.0f, 60.0f)
    %3:vec2<f32> = let %2
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, Normalize_Vector3) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %v3float %glsl Normalize %v3float_50_60_70
     %2 = OpCopyObject %v3float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<f32> = normalize vec3<f32>(50.0f, 60.0f, 70.0f)
    %3:vec3<f32> = let %2
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, Normalize_Vector4) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %v4float %glsl Normalize %v4float_50_50_50_50
     %2 = OpCopyObject %v4float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec4<f32> = normalize vec4<f32>(50.0f)
    %3:vec4<f32> = let %2
    ret
  }
}
)");
}

// Check that we convert signedness of operands and result type.
// This is needed for each of the integer-based extended instructions.

TEST_F(SpirvReaderTest, DISABLED_RectifyOperandsAndResult_SAbs) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %uint %glsl SAbs %uint_10
     %2 = OpExtInst %v2uint %glsl SAbs %v2uint_10_20
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    let x_1 = bitcast<u32>(abs(bitcast<i32>(u1)));
    let x_2 = bitcast<vec2u>(abs(bitcast<vec2i>(v2u1)));
  }
}
)");
}

TEST_F(SpirvReaderTest, DISABLED_RectifyOperandsAndResult_SMax) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %uint %glsl SMax %uint_10 %uint_15
     %2 = OpExtInst %v2uint %glsl SMax %v2uint_10_20 %v2uint_20_10
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    let x_1 = bitcast<u32>(max(bitcast<i32>(u1), bitcast<i32>(u2)));
    let x_2 = bitcast<vec2u>(max(bitcast<vec2i>(v2u1), bitcast<vec2i>(v2u2)));
  }
}
)");
}

TEST_F(SpirvReaderTest, DISABLED_RectifyOperandsAndResult_SMin) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %uint %glsl SMin %uint_10 %uint_15
     %2 = OpExtInst %v2uint %glsl SMin %v2uint_10_20 %v2uint_20_10
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    let x_1 = bitcast<u32>(min(bitcast<i32>(u1), bitcast<i32>(u2)));
    let x_2 = bitcast<vec2u>(min(bitcast<vec2i>(v2u1), bitcast<vec2i>(v2u2)));
  }
}
)");
}

TEST_F(SpirvReaderTest, DISABLED_RectifyOperandsAndResult_SClamp) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %uint %glsl SClamp %uint_10 %int_35 %uint_20
     %2 = OpExtInst %v2uint %glsl SClamp %v2uint_10_20 %v2int_40_30 %v2uint_15_15
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    let x_1 = bitcast<u32>(clamp(bitcast<i32>(u1), i2, bitcast<i32>(u3)));
    let x_2 = bitcast<vec2u>(clamp(bitcast<vec2i>(v2u1), v2i2, bitcast<vec2i>(v2u3)));
  }
}
)");
}

TEST_F(SpirvReaderTest, DISABLED_RectifyOperandsAndResult_UMax) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %int %glsl UMax %int_30 %int_35
     %2 = OpExtInst %v2int %glsl UMax %v2int_30_40 %v2int_40_30
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    let x_1 = bitcast<i32>(max(bitcast<u32>(i1), bitcast<u32>(i2)));
    let x_2 = bitcast<vec2i>(max(bitcast<vec2u>(v2i1), bitcast<vec2u>(v2i2)));
  }
}
)");
}

TEST_F(SpirvReaderTest, DISABLED_RectifyOperandsAndResult_UMin) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %int %glsl UMin %int_30 %int_35
     %2 = OpExtInst %v2int %glsl UMin %v2int_30_40 %v2int_40_30
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    let x_1 = bitcast<i32>(min(bitcast<u32>(i1), bitcast<u32>(i2)));
    let x_2 = bitcast<vec2i>(min(bitcast<vec2u>(v2i1), bitcast<vec2u>(v2i2)));
  }
}
)");
}

TEST_F(SpirvReaderTest, DISABLED_RectifyOperandsAndResult_UClamp) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %int %glsl UClamp %int_30 %uint_15 %int_40
     %2 = OpExtInst %v2int %glsl UClamp %v2int_30_40 %v2uint_20_10 %v2int_35_35
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    let x_1 = bitcast<i32>(clamp(bitcast<u32>(i1), u2, bitcast<u32>(i3)));
    let x_2 = bitcast<vec2i>(clamp(bitcast<vec2u>(v2i1), v2u2, bitcast<vec2u>(v2i3)));
  }
}
)");
}

TEST_F(SpirvReaderTest, DISABLED_RectifyOperandsAndResult_FindILsb) {
    // Check conversion of:
    //   signed results to unsigned result to match first arg.
    //   unsigned results to signed result to match first arg.
    // This is the first extended instruction we've supported which goes both
    // ways.
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %uint %glsl FindILsb %int_30
     %2 = OpExtInst %v2uint %glsl FindILsb %v2int_30_40
     %3 = OpExtInst %int %glsl FindILsb %uint_10
     %4 = OpExtInst %v2int %glsl FindILsb %v2uint_10_20
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    let x_1 = bitcast<u32>(firstTrailingBit(i1));
    let x_2 = bitcast<vec2u>(firstTrailingBit(v2i1));
    let x_3 = bitcast<i32>(firstTrailingBit(u1));
    let x_4 = bitcast<vec2i>(firstTrailingBit(v2u1));
  }
}
)");
}

TEST_F(SpirvReaderTest, DISABLED_RectifyOperandsAndResult_FindSMsb) {
    // Check signedness conversion of arguments and results.
    //   SPIR-V signed arg -> keep it
    //      signed result -> keep it
    //      unsigned result -> cast result to unsigned
    //
    //   SPIR-V unsigned arg -> cast it to signed
    //      signed result -> keept it
    //      unsigned result -> cast result to unsigned
    EXPECT_IR(Preamble() + R"(
     ; signed arg
     ;    signed result
     %1 = OpExtInst %int %glsl FindSMsb %int_30
     %2 = OpExtInst %v2int %glsl FindSMsb %v2int_30_40

     ; signed arg
     ;    unsigned result
     %3 = OpExtInst %uint %glsl FindSMsb %int_30
     %4 = OpExtInst %v2uint %glsl FindSMsb %v2int_30_40

     ; unsigned arg
     ;    signed result
     %5 = OpExtInst %int %glsl FindSMsb %uint_10
     %6 = OpExtInst %v2int %glsl FindSMsb %v2uint_10_20

     ; unsigned arg
     ;    unsigned result
     %7 = OpExtInst %uint %glsl FindSMsb %uint_10
     %8 = OpExtInst %v2uint %glsl FindSMsb %v2uint_10_20
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    let x_1 = firstLeadingBit(i1);
    let x_2 = firstLeadingBit(v2i1);
    let x_3 = bitcast<u32>(firstLeadingBit(i1));
    let x_4 = bitcast<vec2u>(firstLeadingBit(v2i1));
    let x_5 = firstLeadingBit(bitcast<i32>(u1));
    let x_6 = firstLeadingBit(bitcast<vec2i>(v2u1));
    let x_7 = bitcast<u32>(firstLeadingBit(bitcast<i32>(u1)));
    let x_8 = bitcast<vec2u>(firstLeadingBit(bitcast<vec2i>(v2u1)));
  }
}
)");
}

TEST_F(SpirvReaderTest, DISABLED_RectifyOperandsAndResult_FindUMsb) {
    // Check signedness conversion of arguments and results.
    //   SPIR-V signed arg -> cast arg to unsigned
    //      signed result -> cast result to signed
    //      unsigned result -> keep it
    //
    //   SPIR-V unsigned arg -> keep it
    //      signed result -> cast result to signed
    //      unsigned result -> keep it
    EXPECT_IR(Preamble() + R"(
     ; signed arg
     ;    signed result
     %1 = OpExtInst %int %glsl FindUMsb %int_30
     %2 = OpExtInst %v2int %glsl FindUMsb %v2int_30_40

     ; signed arg
     ;    unsigned result
     %3 = OpExtInst %uint %glsl FindUMsb %int_30
     %4 = OpExtInst %v2uint %glsl FindUMsb %v2int_30_40

     ; unsigned arg
     ;    signed result
     %5 = OpExtInst %int %glsl FindUMsb %uint_10
     %6 = OpExtInst %v2int %glsl FindUMsb %v2uint_10_20

     ; unsigned arg
     ;    unsigned result
     %7 = OpExtInst %uint %glsl FindUMsb %uint_10
     %8 = OpExtInst %v2uint %glsl FindUMsb %v2uint_10_20
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    let x_1 = bitcast<i32>(firstLeadingBit(bitcast<u32>(i1)));
    let x_2 = bitcast<vec2i>(firstLeadingBit(bitcast<vec2u>(v2i1)));
    let x_3 = firstLeadingBit(bitcast<u32>(i1));
    let x_4 = firstLeadingBit(bitcast<vec2u>(v2i1));
    let x_5 = bitcast<i32>(firstLeadingBit(u1));
    let x_6 = bitcast<vec2i>(firstLeadingBit(v2u1));
    let x_7 = firstLeadingBit(u1);
    let x_8 = firstLeadingBit(v2u1);
  }
}
)");
}

struct DataPackingCase {
    std::string opcode;
    std::string wgsl_func;
    uint32_t vec_size;
};

inline std::ostream& operator<<(std::ostream& out, DataPackingCase c) {
    out << "DataPacking(" << c.opcode << ")";
    return out;
}

using SpirvReaderTest_GlslStd450_DataPacking = SpirvReaderTestWithParam<DataPackingCase>;

TEST_P(SpirvReaderTest_GlslStd450_DataPacking, Valid) {
    auto param = GetParam();
    EXPECT_IR(Preamble() + R"(
  %1 = OpExtInst %uint %glsl )" +
                  param.opcode +
                  (param.vec_size == 2 ? " %v2float_50_60" : " %v4float_50_50_50_50") + R"(
  %2 = OpCopyObject %uint %1
  OpReturn
  OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = )" + param.wgsl_func +
                  " vec" + std::to_string(param.vec_size) + "<f32>(50.0f" +
                  (param.vec_size == 4 ? "" : ", 60.0f") + R"()
    %3:u32 = let %2
    ret
  }
}
)");
}

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReaderTest_GlslStd450_DataPacking,
                         ::testing::ValuesIn(std::vector<DataPackingCase>{
                             {"PackSnorm4x8", "pack4x8snorm", 4},
                             {"PackUnorm4x8", "pack4x8unorm", 4},
                             {"PackSnorm2x16", "pack2x16snorm", 2},
                             {"PackUnorm2x16", "pack2x16unorm", 2},
                             {"PackHalf2x16", "pack2x16float", 2}}));

using SpirvReaderTest_GlslStd450_DataUnpacking = SpirvReaderTestWithParam<DataPackingCase>;

TEST_P(SpirvReaderTest_GlslStd450_DataUnpacking, Valid) {
    auto param = GetParam();
    auto type = param.vec_size == 2 ? "%v2float" : "%v4float";
    auto wgsl_type = "vec" + std::to_string(param.vec_size) + "<f32>";

    EXPECT_IR(Preamble() + R"(
  %1 = OpExtInst )" +
                  type + std::string(" %glsl ") + param.opcode + R"( %uint_10
  %2 = OpCopyObject )" +
                  type + R"( %1
  OpReturn
  OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:)" + wgsl_type +
                  " = " + param.wgsl_func +
                  R"( 10u
    %3:)" + wgsl_type +
                  R"( = let %2
    ret
  }
}
)");
}

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReaderTest_GlslStd450_DataUnpacking,
                         ::testing::ValuesIn(std::vector<DataPackingCase>{
                             {"UnpackSnorm4x8", "unpack4x8snorm", 4},
                             {"UnpackUnorm4x8", "unpack4x8unorm", 4},
                             {"UnpackSnorm2x16", "unpack2x16snorm", 2},
                             {"UnpackUnorm2x16", "unpack2x16unorm", 2},
                             {"UnpackHalf2x16", "unpack2x16float", 2}}));

TEST_F(SpirvReaderTest, DISABLED_GlslStd450_Refract_Scalar) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %float %glsl Refract %float_50 %float_60 %float_70
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    let x_1 = refract(vec2f(f1, 0.0f), vec2f(f2, 0.0f), f3).x;
  }
}
)");
}

TEST_F(SpirvReaderTest, DISABLED_GlslStd450_Refract_Vector) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %v2float %glsl Refract %v2float_50_60 %v2float_60_50 %float_70
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    let x_1 = refract(v2f1, v2f2, f3);
  }
}
)");
}

TEST_F(SpirvReaderTest, DISABLED_GlslStd450_FaceForward_Scalar) {
    // The %99 sum only has one use.  Ensure it is evaluated only once by
    // making a let-declaration for it, since it is the normal operand to
    // the builtin function, and code generation uses it twice.
    EXPECT_IR(Preamble() + R"(
     %99 = OpFAdd %float %float_50 %float_50 ; normal operand has only one use
     %1 = OpExtInst %float %glsl FaceForward %99 %float_60 %float_70
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    let x_1 = select(-(x_99), x_99, ((f2 * f3) < 0.0f));
  }
}
)");
}

TEST_F(SpirvReaderTest, DISABLED_GlslStd450_FaceForward_Vector) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %v2float %glsl FaceForward %v2float_50_60 %v2float_60_50 %v2float_70_70
     %2 = OpCopyObject %v2float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = faceForward vec2<f32>(50.0f, 60.0f), vec2<f32>(60.0f, 50.0f), vec2<f32>(70.0f)
    %2:vec2<f32> = let %2
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, DISABLED_GlslStd450_Reflect_Scalar) {
    EXPECT_IR(Preamble() + R"(
     %98 = OpFAdd %float %float_50 %float_50 ; has only one use
     %99 = OpFAdd %float %float_60 %float_60 ; has only one use
     %1 = OpExtInst %float %glsl Reflect %98 %99
     %2 = OpCopyObject %float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = (x_98 - (2.0f * (x_99 * (x_99 * x_98))));
    %3:f32 = let %2
  }
}
)");
}

TEST_F(SpirvReaderTest, DISABLED_GlslStd450_Reflect_Vector) {
    EXPECT_IR(Preamble() + R"(
     %98 = OpFAdd %v2float %v2float_50_60 %v2float_50_60
     %99 = OpFAdd %v2float %v2float_60_50 %v2float_60_50
     %1 = OpExtInst %v2float %glsl Reflect %98 %99
     %2 = OpCopyObject %v2float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = add vec2<f32>(50.0f, 60.0f), vec2<f32>(50.0f, 60.0f)
    %3:vec2<f32> = add vec2<f32>(60.0f, 50.0f), vec2<f32>(60.0f, 50.0f)
    %4:vec2<f32> = reflect %2, %3
    %5:vec2<f32> = let %4
    ret
  }
}
)");
}

// For ldexp with signed second argument, see above.
TEST_F(SpirvReaderTest, DISABLED_GlslStd450_Ldexp_Scalar_Float_Uint) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %float %glsl Ldexp %float_50 %uint_10
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    let x_1 = ldexp(f1, i32(u1));
  }
}
)");
}

TEST_F(SpirvReaderTest, DISABLED_GlslStd450_Ldexp_Vector_Floatvec_Uintvec) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %v2float %glsl Ldexp %v2float_50_60 %v2uint_10_20
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    let x_1 = ldexp(v2f1, vec2i(v2u1));
  }
}
)");
}

struct DeterminantData {
    std::string in;
    std::string out;
};

inline std::ostream& operator<<(std::ostream& out, DeterminantData c) {
    out << "Determinant(" << c.in << ")";
    return out;
}

using SpirvReaderTest_GlslStd450_Determinant = SpirvReaderTestWithParam<DeterminantData>;

TEST_P(SpirvReaderTest_GlslStd450_Determinant, DISABLED_Test) {
    auto param = GetParam();

    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %float %glsl Determinant %)" +
                  param.in + R"(
     %2 = OpCopyObject %float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = determinant )" +
                  param.out + R"(
    %3:f32 = let %2
    ret
  }
}
)");
}

INSTANTIATE_TEST_SUITE_P(
    SpirvReader,
    SpirvReaderTest_GlslStd450_Determinant,
    ::testing::Values(DeterminantData{"m2x2f1", "mat2x2<f32>(vec2<f32>(50.0f, 60.0f))"},
                      DeterminantData{"m3x3f1", "mat3x3<f32>(vec3<f32>(50.0f, 60.0f, 70.0f))"},
                      DeterminantData{"m4x4f1", "mat4x4<f32>(vec4<f32>(50.0f))"}));

}  // namespace
}  // namespace tint::spirv::reader
