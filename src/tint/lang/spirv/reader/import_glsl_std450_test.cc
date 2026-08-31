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

std::string Preamble(const std::string& names = "") {
    return R"(
  OpCapability Shader
  OpCapability Float16
  %glsl = OpExtInstImport "GLSL.std.450"
  OpMemoryModel Logical GLSL450
  OpEntryPoint GLCompute %100 "main"
  OpExecutionMode %100 LocalSize 1 1 1
)" + names +
           R"(
  %void = OpTypeVoid
  %voidfn = OpTypeFunction %void

  %uint = OpTypeInt 32 0
  %int = OpTypeInt 32 1
  %float = OpTypeFloat 32
  %half = OpTypeFloat 16

  %uint_10 = OpConstant %uint 10
  %uint_15 = OpConstant %uint 15
  %uint_20 = OpConstant %uint 20
  %int_30 = OpConstant %int 30
  %int_35 = OpConstant %int 35
  %int_40 = OpConstant %int 40
  %float_50 = OpConstant %float 50
  %float_60 = OpConstant %float 60
  %float_70 = OpConstant %float 70
  %half_50 = OpConstant %half 50
  %half_60 = OpConstant %half 60
  %half_70 = OpConstant %half 70

  %v2uint = OpTypeVector %uint 2
  %v2int = OpTypeVector %int 2
  %v2float = OpTypeVector %float 2
  %v3float = OpTypeVector %float 3
  %v4float = OpTypeVector %float 4
  %v2half = OpTypeVector %half 2
  %v3half = OpTypeVector %half 3
  %v4half = OpTypeVector %half 4
  %mat2v2float = OpTypeMatrix %v2float 2
  %mat3v3float = OpTypeMatrix %v3float 3
  %mat4v4float = OpTypeMatrix %v4float 4
  %mat2v2half = OpTypeMatrix %v2half 2
  %mat3v3half = OpTypeMatrix %v3half 3
  %mat4v4half = OpTypeMatrix %v4half 4

  %modf_result_type = OpTypeStruct %float %float
  %modf_v2_result_type = OpTypeStruct %v2float %v2float
  %ptr_function_modf_result_type = OpTypePointer Function %modf_result_type
  %ptr_function_modf_v2_result_type = OpTypePointer Function %modf_v2_result_type

  %frexp_result_type_unsigned = OpTypeStruct %float %uint
  %frexp_result_type_signed = OpTypeStruct %float %int
  %frexp_v2_result_type_unsigned = OpTypeStruct %v2float %v2uint
  %frexp_v2_result_type_signed = OpTypeStruct %v2float %v2int
  %ptr_function_frexp_result_type_unsigned = OpTypePointer Function %frexp_result_type_unsigned
  %ptr_function_frexp_result_type_signed = OpTypePointer Function %frexp_result_type_signed
  %ptr_function_frexp_v2_result_type_unsigned = OpTypePointer Function %frexp_v2_result_type_unsigned
  %ptr_function_frexp_v2_result_type_signed = OpTypePointer Function %frexp_v2_result_type_signed

  %v2uint_10_20 = OpConstantComposite %v2uint %uint_10 %uint_20
  %v2uint_20_10 = OpConstantComposite %v2uint %uint_20 %uint_10
  %v2uint_15_15 = OpConstantComposite %v2uint %uint_15 %uint_15
  %v2int_30_40 = OpConstantComposite %v2int %int_30 %int_40
  %v2int_40_30 = OpConstantComposite %v2int %int_40 %int_30
  %v2int_35_35 = OpConstantComposite %v2int %int_35 %int_35
  %v2float_50_60 = OpConstantComposite %v2float %float_50 %float_60
  %v2float_60_50 = OpConstantComposite %v2float %float_60 %float_50
  %v2float_70_70 = OpConstantComposite %v2float %float_70 %float_70
  %v2half_50_60 = OpConstantComposite %v2half %half_50 %half_60

  %v3float_50_60_70 = OpConstantComposite %v3float %float_50 %float_60 %float_70
  %v3float_60_70_50 = OpConstantComposite %v3float %float_60 %float_70 %float_50
  %v3half_50_60_70 = OpConstantComposite %v3half %half_50 %half_60 %half_70

  %v4float_50_50_50_50 = OpConstantComposite %v4float %float_50 %float_50 %float_50 %float_50
  %v4half_50_50_50_50 = OpConstantComposite %v4half %half_50 %half_50 %half_50 %half_50

  %mat2v2float_50_60 = OpConstantComposite %mat2v2float %v2float_50_60 %v2float_50_60
  %mat3v3float_50_60_70 = OpConstantComposite %mat3v3float %v3float_50_60_70 %v3float_50_60_70 %v3float_50_60_70
  %mat4v4float_50_50_50_50 = OpConstantComposite %mat4v4float %v4float_50_50_50_50 %v4float_50_50_50_50 %v4float_50_50_50_50 %v4float_50_50_50_50

  %mat2v2half_50_60 = OpConstantComposite %mat2v2half %v2half_50_60 %v2half_50_60
  %mat3v3half_50_60_70 = OpConstantComposite %mat3v3half %v3half_50_60_70 %v3half_50_60_70 %v3half_50_60_70
  %mat4v4half_50_50_50_50 = OpConstantComposite %mat4v4half %v4half_50_50_50_50 %v4half_50_50_50_50 %v4half_50_50_50_50 %v4half_50_50_50_50

  %fn_float_float = OpTypeFunction %float %float
  %fn_float_v2float = OpTypeFunction %float %v2float
  %fn_float_v3float = OpTypeFunction %float %v3float
  %fn_float_v4float = OpTypeFunction %float %v4float
  %fn_v2float_v2float = OpTypeFunction %v2float %v2float
  %fn_v3float_v3float = OpTypeFunction %v3float %v3float
  %fn_v4float_v4float = OpTypeFunction %v4float %v4float
  %fn_int_int = OpTypeFunction %int %int
  %fn_v2int_v2int = OpTypeFunction %v2int %v2int
  %fn_uint_uint = OpTypeFunction %uint %uint
  %fn_v2uint_v2uint = OpTypeFunction %v2uint %v2uint

  %fn_float_float_float = OpTypeFunction %float %float %float
  %fn_float_v2float_v2float = OpTypeFunction %float %v2float %v2float
  %fn_v2float_v2float_v2float = OpTypeFunction %v2float %v2float %v2float
  %fn_v3float_v3float_v3float = OpTypeFunction %v3float %v3float %v3float
  %fn_float_float_int = OpTypeFunction %float %float %int
  %fn_v2float_v2float_v2int = OpTypeFunction %v2float %v2float %v2int

  %fn_float_float_float_float = OpTypeFunction %float %float %float %float
  %fn_v2float_v2float_v2float_v2float = OpTypeFunction %v2float %v2float %v2float %v2float

  %fn_uint_v2float = OpTypeFunction %uint %v2float
  %fn_uint_v4float = OpTypeFunction %uint %v4float
  %fn_v2float_uint = OpTypeFunction %v2float %uint
  %fn_v4float_uint = OpTypeFunction %v4float %uint

  %fn_float_mat2v2float = OpTypeFunction %float %mat2v2float
  %fn_float_mat3v3float = OpTypeFunction %float %mat3v3float
  %fn_float_mat4v4float = OpTypeFunction %float %mat4v4float
  %fn_half_mat2v2half = OpTypeFunction %half %mat2v2half
  %fn_half_mat3v3half = OpTypeFunction %half %mat3v3half
  %fn_half_mat4v4half = OpTypeFunction %half %mat4v4half

  %fn_modf_float = OpTypeFunction %modf_result_type %float
  %fn_modf_v2float = OpTypeFunction %modf_v2_result_type %v2float
  %fn_frexp_u_float = OpTypeFunction %frexp_result_type_unsigned %float
  %fn_frexp_s_float = OpTypeFunction %frexp_result_type_signed %float
  %fn_frexp_u_v2float = OpTypeFunction %frexp_v2_result_type_unsigned %v2float
  %fn_frexp_s_v2float = OpTypeFunction %frexp_v2_result_type_signed %v2float
  %fn_void_float = OpTypeFunction %void %float
  %fn_void_v2float = OpTypeFunction %void %v2float
  %fn_void_rectify = OpTypeFunction %void %int %v2int %uint %v2uint
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
using SpirvReaderTest_GlslStd450_Uinting_Uinting = SpirvReaderTestWithParam<GlslStd450Case>;

TEST_P(SpirvReaderTest_GlslStd450_Float_Floating, Scalar) {
    EXPECT_IR(Preamble(R"(  OpName %foo "foo"
  OpName %x "x"
)") + R"(
     %foo = OpFunction %float None %fn_float_float
     %x = OpFunctionParameter %float
     %foo_entry = OpLabel
     %1 = OpExtInst %float %glsl )" +
                  GetParam().opcode + R"( %x
     OpReturnValue %1
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %res = OpFunctionCall %float %foo %float_50
     OpReturn
     OpFunctionEnd
  )",

              R"(
%foo = func(%x:f32):f32 {
  $B1: {
    %3:f32 = )" + GetParam().wgsl_func +
                  R"( %x
    ret %3
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Float_Floating, Vector) {
    EXPECT_IR(Preamble(R"(  OpName %foo "foo"
  OpName %x "x"
)") + R"(
     %foo = OpFunction %float None %fn_float_v2float
     %x = OpFunctionParameter %v2float
     %foo_entry = OpLabel
     %1 = OpExtInst %float %glsl )" +
                  GetParam().opcode + R"( %x
     OpReturnValue %1
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %res = OpFunctionCall %float %foo %v2float_50_60
     OpReturn
     OpFunctionEnd
  )",
              R"(
%foo = func(%x:vec2<f32>):f32 {
  $B1: {
    %3:f32 = )" + GetParam().wgsl_func +
                  R"( %x
    ret %3
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Float_FloatingFloating, Scalar) {
    EXPECT_IR(Preamble(R"(  OpName %foo "foo"
  OpName %x "x"
  OpName %y "y"
)") + R"(
     %foo = OpFunction %float None %fn_float_float_float
     %x = OpFunctionParameter %float
     %y = OpFunctionParameter %float
     %foo_entry = OpLabel
     %1 = OpExtInst %float %glsl )" +
                  GetParam().opcode + R"( %x %y
     OpReturnValue %1
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %res = OpFunctionCall %float %foo %float_50 %float_60
     OpReturn
     OpFunctionEnd
  )",

              R"(
%foo = func(%x:f32, %y:f32):f32 {
  $B1: {
    %4:f32 = )" + GetParam().wgsl_func +
                  R"( %x, %y
    ret %4
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Float_FloatingFloating, Vector) {
    EXPECT_IR(Preamble(R"(  OpName %foo "foo"
  OpName %x "x"
  OpName %y "y"
)") + R"(
     %foo = OpFunction %float None %fn_float_v2float_v2float
     %x = OpFunctionParameter %v2float
     %y = OpFunctionParameter %v2float
     %foo_entry = OpLabel
     %1 = OpExtInst %float %glsl )" +
                  GetParam().opcode + R"( %x %y
     OpReturnValue %1
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %res = OpFunctionCall %float %foo %v2float_50_60 %v2float_60_50
     OpReturn
     OpFunctionEnd
  )",
              R"(
%foo = func(%x:vec2<f32>, %y:vec2<f32>):f32 {
  $B1: {
    %4:f32 = )" + GetParam().wgsl_func +
                  R"( %x, %y
    ret %4
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Floating_Floating, Scalar) {
    EXPECT_IR(Preamble(R"(  OpName %foo "foo"
  OpName %x "x"
)") + R"(
     %foo = OpFunction %float None %fn_float_float
     %x = OpFunctionParameter %float
     %foo_entry = OpLabel
     %1 = OpExtInst %float %glsl )" +
                  GetParam().opcode + R"( %x
     OpReturnValue %1
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %res = OpFunctionCall %float %foo %float_50
     OpReturn
     OpFunctionEnd
  )",

              R"(
%foo = func(%x:f32):f32 {
  $B1: {
    %3:f32 = )" + GetParam().wgsl_func +
                  R"( %x
    ret %3
  }
})");
}

TEST_P(SpirvReaderTest_GlslStd450_Floating_Floating, Vector) {
    EXPECT_IR(Preamble(R"(  OpName %foo "foo"
  OpName %x "x"
)") + R"(
     %foo = OpFunction %v2float None %fn_v2float_v2float
     %x = OpFunctionParameter %v2float
     %foo_entry = OpLabel
     %1 = OpExtInst %v2float %glsl )" +
                  GetParam().opcode + R"( %x
     OpReturnValue %1
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %res = OpFunctionCall %v2float %foo %v2float_50_60
     OpReturn
     OpFunctionEnd
  )",
              R"(
%foo = func(%x:vec2<f32>):vec2<f32> {
  $B1: {
    %3:vec2<f32> = )" +
                  GetParam().wgsl_func +
                  R"( %x
    ret %3
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Floating_FloatingFloating, Scalar) {
    EXPECT_IR(Preamble(R"(  OpName %foo "foo"
  OpName %x "x"
  OpName %y "y"
)") + R"(
     %foo = OpFunction %float None %fn_float_float_float
     %x = OpFunctionParameter %float
     %y = OpFunctionParameter %float
     %foo_entry = OpLabel
     %1 = OpExtInst %float %glsl )" +
                  GetParam().opcode + R"( %x %y
     OpReturnValue %1
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %res = OpFunctionCall %float %foo %float_50 %float_60
     OpReturn
     OpFunctionEnd
  )",

              R"(
%foo = func(%x:f32, %y:f32):f32 {
  $B1: {
    %4:f32 = )" + GetParam().wgsl_func +
                  R"( %x, %y
    ret %4
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Floating_FloatingFloating, Vector) {
    EXPECT_IR(Preamble(R"(  OpName %foo "foo"
  OpName %x "x"
  OpName %y "y"
)") + R"(
     %foo = OpFunction %v2float None %fn_v2float_v2float_v2float
     %x = OpFunctionParameter %v2float
     %y = OpFunctionParameter %v2float
     %foo_entry = OpLabel
     %1 = OpExtInst %v2float %glsl )" +
                  GetParam().opcode + R"( %x %y
     OpReturnValue %1
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %res = OpFunctionCall %v2float %foo %v2float_50_60 %v2float_60_50
     OpReturn
     OpFunctionEnd
  )",
              R"(
%foo = func(%x:vec2<f32>, %y:vec2<f32>):vec2<f32> {
  $B1: {
    %4:vec2<f32> = )" +
                  GetParam().wgsl_func +
                  R"( %x, %y
    ret %4
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Floating_FloatingFloatingFloating, Scalar) {
    EXPECT_IR(Preamble(R"(  OpName %foo "foo"
  OpName %x "x"
  OpName %y "y"
  OpName %z "z"
)") + R"(
     %foo = OpFunction %float None %fn_float_float_float_float
     %x = OpFunctionParameter %float
     %y = OpFunctionParameter %float
     %z = OpFunctionParameter %float
     %foo_entry = OpLabel
     %1 = OpExtInst %float %glsl )" +
                  GetParam().opcode + R"( %x %y %z
     OpReturnValue %1
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %res = OpFunctionCall %float %foo %float_50 %float_60 %float_70
     OpReturn
     OpFunctionEnd
  )",

              R"(
%foo = func(%x:f32, %y:f32, %z:f32):f32 {
  $B1: {
    %5:f32 = )" + GetParam().wgsl_func +
                  R"( %x, %y, %z
    ret %5
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Floating_FloatingFloatingFloating, Vector) {
    EXPECT_IR(Preamble(R"(  OpName %foo "foo"
  OpName %x "x"
  OpName %y "y"
  OpName %z "z"
)") + R"(
     %foo = OpFunction %v2float None %fn_v2float_v2float_v2float_v2float
     %x = OpFunctionParameter %v2float
     %y = OpFunctionParameter %v2float
     %z = OpFunctionParameter %v2float
     %foo_entry = OpLabel
     %1 = OpExtInst %v2float %glsl )" +
                  GetParam().opcode +
                  R"( %x %y %z
     OpReturnValue %1
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %res = OpFunctionCall %v2float %foo %v2float_50_60 %v2float_60_50 %v2float_70_70
     OpReturn
     OpFunctionEnd
  )",
              R"(
%foo = func(%x:vec2<f32>, %y:vec2<f32>, %z:vec2<f32>):vec2<f32> {
  $B1: {
    %5:vec2<f32> = )" +
                  GetParam().wgsl_func +
                  R"( %x, %y, %z
    ret %5
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Floating_FloatingInting, Scalar) {
    EXPECT_IR(Preamble(R"(  OpName %foo "foo"
  OpName %x "x"
  OpName %y "y"
)") + R"(
     %foo = OpFunction %float None %fn_float_float_int
     %x = OpFunctionParameter %float
     %y = OpFunctionParameter %int
     %foo_entry = OpLabel
     %1 = OpExtInst %float %glsl )" +
                  GetParam().opcode + R"( %x %y
     OpReturnValue %1
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %res = OpFunctionCall %float %foo %float_50 %int_30
     OpReturn
     OpFunctionEnd
  )",

              R"(
%foo = func(%x:f32, %y:i32):f32 {
  $B1: {
    %4:f32 = )" + GetParam().wgsl_func +
                  R"( %x, %y
    ret %4
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Floating_FloatingInting, Vector) {
    EXPECT_IR(Preamble(R"(  OpName %foo "foo"
  OpName %x "x"
  OpName %y "y"
)") + R"(
     %foo = OpFunction %v2float None %fn_v2float_v2float_v2int
     %x = OpFunctionParameter %v2float
     %y = OpFunctionParameter %v2int
     %foo_entry = OpLabel
     %1 = OpExtInst %v2float %glsl )" +
                  GetParam().opcode +
                  R"( %x %y
     OpReturnValue %1
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %res = OpFunctionCall %v2float %foo %v2float_50_60 %v2int_30_40
     OpReturn
     OpFunctionEnd
  )",
              R"(
%foo = func(%x:vec2<f32>, %y:vec2<i32>):vec2<f32> {
  $B1: {
    %4:vec2<f32> = )" +
                  GetParam().wgsl_func +
                  R"( %x, %y
    ret %4
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Float3_Float3Float3, SpirvParser) {
    EXPECT_IR(Preamble(R"(  OpName %foo "foo"
  OpName %x "x"
  OpName %y "y"
)") + R"(
     %foo = OpFunction %v3float None %fn_v3float_v3float_v3float
     %x = OpFunctionParameter %v3float
     %y = OpFunctionParameter %v3float
     %foo_entry = OpLabel
     %1 = OpExtInst %v3float %glsl )" +
                  GetParam().opcode +
                  R"( %x %y
     OpReturnValue %1
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %res = OpFunctionCall %v3float %foo %v3float_50_60_70 %v3float_60_70_50
     OpReturn
     OpFunctionEnd
  )",
              R"(
%foo = func(%x:vec3<f32>, %y:vec3<f32>):vec3<f32> {
  $B1: {
    %4:vec3<f32> = )" +
                  GetParam().wgsl_func +
                  R"( %x, %y
    ret %4
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
                             {"Acosh", "acosh"},              //
                             {"Asin", "asin"},                //
                             {"Asinh", "asinh"},              //
                             {"Atan", "atan"},                //
                             {"Atanh", "atanh"},              //
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
    EXPECT_IR(Preamble(R"(  OpName %foo "foo"
  OpName %x "x"
)") + R"(
     %foo = OpFunction %int None %fn_int_int
     %x = OpFunctionParameter %int
     %foo_entry = OpLabel
     %1 = OpExtInst %int %glsl )" +
                  GetParam().opcode +
                  R"( %x
     OpReturnValue %1
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %res = OpFunctionCall %int %foo %int_30
     OpReturn
     OpFunctionEnd
  )",
              R"(
%foo = func(%x:i32):i32 {
  $B1: {
    %3:i32 = )" + GetParam().wgsl_func +
                  R"( %x
    ret %3
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Inting_Inting, Vector) {
    EXPECT_IR(Preamble(R"(  OpName %foo "foo"
  OpName %x "x"
)") + R"(
     %foo = OpFunction %v2int None %fn_v2int_v2int
     %x = OpFunctionParameter %v2int
     %foo_entry = OpLabel
     %1 = OpExtInst %v2int %glsl )" +
                  GetParam().opcode +
                  R"( %x
     OpReturnValue %1
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %res = OpFunctionCall %v2int %foo %v2int_30_40
     OpReturn
     OpFunctionEnd
  )",
              R"(
%foo = func(%x:vec2<i32>):vec2<i32> {
  $B1: {
    %3:vec2<i32> = )" +
                  GetParam().wgsl_func +
                  R"( %x
    ret %3
  }
}
)");
}

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReaderTest_GlslStd450_Inting_Inting,
                         ::testing::Values(GlslStd450Case{"FindILsb", "firstTrailingBit"},
                                           GlslStd450Case{"FindSMsb", "firstLeadingBit"}));

TEST_P(SpirvReaderTest_GlslStd450_Uinting_Uinting, Scalar) {
    EXPECT_IR(Preamble(R"(  OpName %foo "foo"
  OpName %x "x"
)") + R"(
     %foo = OpFunction %uint None %fn_uint_uint
     %x = OpFunctionParameter %uint
     %foo_entry = OpLabel
     %1 = OpExtInst %uint %glsl )" +
                  GetParam().opcode +
                  R"( %x
     OpReturnValue %1
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %res = OpFunctionCall %uint %foo %uint_10
     OpReturn
     OpFunctionEnd
  )",
              R"(
%foo = func(%x:u32):u32 {
  $B1: {
    %3:u32 = )" + GetParam().wgsl_func +
                  R"( %x
    ret %3
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Uinting_Uinting, Vector) {
    EXPECT_IR(Preamble(R"(  OpName %foo "foo"
  OpName %x "x"
)") + R"(
     %foo = OpFunction %v2uint None %fn_v2uint_v2uint
     %x = OpFunctionParameter %v2uint
     %foo_entry = OpLabel
     %1 = OpExtInst %v2uint %glsl )" +
                  GetParam().opcode +
                  R"( %x
     OpReturnValue %1
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %res = OpFunctionCall %v2uint %foo %v2uint_10_20
     OpReturn
     OpFunctionEnd
  )",
              R"(
%foo = func(%x:vec2<u32>):vec2<u32> {
  $B1: {
    %3:vec2<u32> = )" +
                  GetParam().wgsl_func +
                  R"( %x
    ret %3
  }
}
)");
}

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReaderTest_GlslStd450_Uinting_Uinting,
                         ::testing::Values(GlslStd450Case{"FindILsb", "firstTrailingBit"},
                                           GlslStd450Case{"FindUMsb", "firstLeadingBit"}));

// Test Normalize.  WGSL does not have a scalar form of the normalize builtin.
// So we have to test it separately, as it does not fit the patterns tested
// above.

TEST_F(SpirvReaderTest, Normalize_Scalar) {
    // Scalar normalize maps to sign.
    EXPECT_IR(Preamble(R"(  OpName %foo "foo"
  OpName %x "x"
)") + R"(
     %foo = OpFunction %float None %fn_float_float
     %x = OpFunctionParameter %float
     %foo_entry = OpLabel
     %1 = OpExtInst %float %glsl Normalize %x
     OpReturnValue %1
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %res = OpFunctionCall %float %foo %float_50
     OpReturn
     OpFunctionEnd
  )",
              R"(
%foo = func(%x:f32):f32 {
  $B1: {
    %3:f32 = sign %x
    ret %3
  }
}
)");
}

TEST_F(SpirvReaderTest, Normalize_Vector2) {
    EXPECT_IR(Preamble(R"(  OpName %foo "foo"
  OpName %x "x"
)") + R"(
     %foo = OpFunction %v2float None %fn_v2float_v2float
     %x = OpFunctionParameter %v2float
     %foo_entry = OpLabel
     %1 = OpExtInst %v2float %glsl Normalize %x
     OpReturnValue %1
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %res = OpFunctionCall %v2float %foo %v2float_50_60
     OpReturn
     OpFunctionEnd
  )",
              R"(
%foo = func(%x:vec2<f32>):vec2<f32> {
  $B1: {
    %3:vec2<f32> = normalize %x
    ret %3
  }
}
)");
}

TEST_F(SpirvReaderTest, Normalize_Vector3) {
    EXPECT_IR(Preamble(R"(  OpName %foo "foo"
  OpName %x "x"
)") + R"(
     %foo = OpFunction %v3float None %fn_v3float_v3float
     %x = OpFunctionParameter %v3float
     %foo_entry = OpLabel
     %1 = OpExtInst %v3float %glsl Normalize %x
     OpReturnValue %1
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %res = OpFunctionCall %v3float %foo %v3float_50_60_70
     OpReturn
     OpFunctionEnd
  )",
              R"(
%foo = func(%x:vec3<f32>):vec3<f32> {
  $B1: {
    %3:vec3<f32> = normalize %x
    ret %3
  }
}
)");
}

TEST_F(SpirvReaderTest, Normalize_Vector4) {
    EXPECT_IR(Preamble(R"(  OpName %foo "foo"
  OpName %x "x"
)") + R"(
     %foo = OpFunction %v4float None %fn_v4float_v4float
     %x = OpFunctionParameter %v4float
     %foo_entry = OpLabel
     %1 = OpExtInst %v4float %glsl Normalize %x
     OpReturnValue %1
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %res = OpFunctionCall %v4float %foo %v4float_50_50_50_50
     OpReturn
     OpFunctionEnd
  )",
              R"(
%foo = func(%x:vec4<f32>):vec4<f32> {
  $B1: {
    %3:vec4<f32> = normalize %x
    ret %3
  }
}
)");
}

TEST_F(SpirvReaderTest, RectifyOperandsAndResult_FindUMsb) {
    // Check signedness conversion of arguments and results.
    //   SPIR-V signed arg -> cast arg to unsigned
    //      signed result -> cast result to signed
    //      unsigned result -> keep it
    //
    //   SPIR-V unsigned arg -> keep it
    //      signed result -> cast result to signed
    //      unsigned result -> keep it
    EXPECT_IR(Preamble(R"(  OpName %foo "foo"
  OpName %a "a"
  OpName %b "b"
  OpName %c "c"
  OpName %d "d"
)") + R"(
     %foo = OpFunction %void None %fn_void_rectify
     %a = OpFunctionParameter %int
     %b = OpFunctionParameter %v2int
     %c = OpFunctionParameter %uint
     %d = OpFunctionParameter %v2uint
     %foo_entry = OpLabel

     ; signed arg
     ;    signed result
     %1 = OpExtInst %int %glsl FindUMsb %a
     %2 = OpExtInst %v2int %glsl FindUMsb %b

     ; signed arg
     ;    unsigned result
     %3 = OpExtInst %uint %glsl FindUMsb %a
     %4 = OpExtInst %v2uint %glsl FindUMsb %b

     ; unsigned arg
     ;    signed result
     %5 = OpExtInst %int %glsl FindUMsb %c
     %6 = OpExtInst %v2int %glsl FindUMsb %d

     ; unsigned arg
     ;    unsigned result
     %7 = OpExtInst %uint %glsl FindUMsb %c
     %8 = OpExtInst %v2uint %glsl FindUMsb %d
     OpReturn
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %res = OpFunctionCall %void %foo %int_30 %v2int_30_40 %uint_10 %v2uint_10_20
     OpReturn
     OpFunctionEnd
  )",
              R"(
%foo = func(%a:i32, %b:vec2<i32>, %c:u32, %d:vec2<u32>):void {
  $B1: {
    %6:u32 = bitcast<u32> %a
    %7:u32 = firstLeadingBit %6
    %8:i32 = bitcast<i32> %7
    %9:vec2<u32> = bitcast<vec2<u32>> %b
    %10:vec2<u32> = firstLeadingBit %9
    %11:vec2<i32> = bitcast<vec2<i32>> %10
    %12:u32 = bitcast<u32> %a
    %13:u32 = firstLeadingBit %12
    %14:vec2<u32> = bitcast<vec2<u32>> %b
    %15:vec2<u32> = firstLeadingBit %14
    %16:u32 = firstLeadingBit %c
    %17:i32 = bitcast<i32> %16
    %18:vec2<u32> = firstLeadingBit %d
    %19:vec2<i32> = bitcast<vec2<i32>> %18
    %20:u32 = firstLeadingBit %c
    %21:vec2<u32> = firstLeadingBit %d
    ret
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
    auto fn_ty = param.vec_size == 2 ? "%fn_uint_v2float" : "%fn_uint_v4float";
    auto in_ty = param.vec_size == 2 ? "%v2float" : "%v4float";
    auto in_val = param.vec_size == 2 ? "%v2float_50_60" : "%v4float_50_50_50_50";
    auto wgsl_in_ty = "vec" + std::to_string(param.vec_size) + "<f32>";

    EXPECT_IR(Preamble(R"(  OpName %foo "foo"
  OpName %x "x"
)") + R"(
  %foo = OpFunction %uint None )" +
                  fn_ty + R"(
  %x = OpFunctionParameter )" +
                  in_ty + R"(
  %foo_entry = OpLabel
  %1 = OpExtInst %uint %glsl )" +
                  param.opcode + R"( %x
  OpReturnValue %1
  OpFunctionEnd

  %100 = OpFunction %void None %voidfn
  %entry = OpLabel
  %res = OpFunctionCall %uint %foo )" +
                  in_val + R"(
  OpReturn
  OpFunctionEnd
  )",
              R"(
%foo = func(%x:)" +
                  wgsl_in_ty + R"():u32 {
  $B1: {
    %3:u32 = )" + param.wgsl_func +
                  R"( %x
    ret %3
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
    auto fn_ty = param.vec_size == 2 ? "%fn_v2float_uint" : "%fn_v4float_uint";
    auto wgsl_type = "vec" + std::to_string(param.vec_size) + "<f32>";

    EXPECT_IR(Preamble(R"(  OpName %foo "foo"
  OpName %x "x"
)") + R"(
  %foo = OpFunction )" +
                  type + R"( None )" + fn_ty + R"(
  %x = OpFunctionParameter %uint
  %foo_entry = OpLabel
  %1 = OpExtInst )" +
                  type + std::string(" %glsl ") + param.opcode + R"( %x
  OpReturnValue %1
  OpFunctionEnd

  %100 = OpFunction %void None %voidfn
  %entry = OpLabel
  %res = OpFunctionCall )" +
                  type + R"( %foo %uint_10
  OpReturn
  OpFunctionEnd
  )",
              R"(
%foo = func(%x:u32):)" +
                  wgsl_type + R"( {
  $B1: {
    %3:)" + wgsl_type +
                  " = " + param.wgsl_func +
                  R"( %x
    ret %3
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

struct DeterminantData {
    std::string in;
    std::string in_type;
    std::string out;
    std::string ty;
    std::string ty_name;
    std::string fn_type;
};

[[maybe_unused]] inline std::ostream& operator<<(std::ostream& out, DeterminantData c) {
    out << "Determinant(" << c.in << ")";
    return out;
}

using SpirvReaderTest_GlslStd450_Determinant = SpirvReaderTestWithParam<DeterminantData>;

TEST_P(SpirvReaderTest_GlslStd450_Determinant, Test) {
    auto param = GetParam();

    EXPECT_IR(Preamble(R"(  OpName %foo "foo"
  OpName %x "x"
)") + R"(
     %foo = OpFunction %)" +
                  param.ty_name + R"( None %)" + param.fn_type + R"(
     %x = OpFunctionParameter %)" +
                  param.in_type + R"(
     %foo_entry = OpLabel
     %1 = OpExtInst %)" +
                  param.ty_name + R"( %glsl Determinant %x
     OpReturnValue %1
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %res = OpFunctionCall %)" +
                  param.ty_name + R"( %foo %)" + param.in + R"(
     OpReturn
     OpFunctionEnd
  )",
              R"(
%foo = func(%x:)" +
                  param.out + R"():)" + param.ty + R"( {
  $B1: {
    %3:)" + param.ty +
                  R"( = determinant %x
    ret %3
  }
}
)");
}

INSTANTIATE_TEST_SUITE_P(
    SpirvReader,
    SpirvReaderTest_GlslStd450_Determinant,
    ::testing::Values(DeterminantData{"mat2v2float_50_60", "mat2v2float", "mat2x2<f32>", "f32",
                                      "float", "fn_float_mat2v2float"},
                      DeterminantData{"mat3v3float_50_60_70", "mat3v3float", "mat3x3<f32>", "f32",
                                      "float", "fn_float_mat3v3float"},
                      DeterminantData{"mat4v4float_50_50_50_50", "mat4v4float", "mat4x4<f32>",
                                      "f32", "float", "fn_float_mat4v4float"},
                      DeterminantData{"mat2v2half_50_60", "mat2v2half", "mat2x2<f16>", "f16",
                                      "half", "fn_half_mat2v2half"},
                      DeterminantData{"mat3v3half_50_60_70", "mat3v3half", "mat3x3<f16>", "f16",
                                      "half", "fn_half_mat3v3half"},
                      DeterminantData{"mat4v4half_50_50_50_50", "mat4v4half", "mat4x4<f16>", "f16",
                                      "half", "fn_half_mat4v4half"}));

TEST_F(SpirvReaderTest, ModfStruct_Store) {
    EXPECT_IR(Preamble(R"(  OpName %foo "foo"
  OpName %x "x"
)") + R"(
     %foo = OpFunction %void None %fn_void_float
     %x = OpFunctionParameter %float
     %foo_entry = OpLabel
     %1 = OpVariable %ptr_function_modf_result_type Function
     %2 = OpExtInst %modf_result_type %glsl ModfStruct %x
     OpStore %1 %2
     OpReturn
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %res = OpFunctionCall %void %foo %float_50
     OpReturn
     OpFunctionEnd
  )",
              R"(
tint_symbol_2 = struct @align(4) {
  tint_symbol:f32 @offset(0)
  tint_symbol_1:f32 @offset(4)
}

__modf_result_f32 = struct @align(4) {
  fract:f32 @offset(0)
  whole:f32 @offset(4)
}

%foo = func(%x:f32):void {
  $B1: {
    %3:ptr<function, tint_symbol_2, read_write> = var undef
    %4:__modf_result_f32 = modf %x
    %5:f32 = access %4, 0u
    %6:f32 = access %4, 1u
    %7:tint_symbol_2 = construct %5, %6
    store %3, %7
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, ModfStruct_Scalar) {
    EXPECT_IR(Preamble(R"(  OpName %foo "foo"
  OpName %x "x"
)") + R"(
     %foo = OpFunction %float None %fn_float_float
     %x = OpFunctionParameter %float
     %foo_entry = OpLabel
     %1 = OpExtInst %modf_result_type %glsl ModfStruct %x
     %2 = OpCompositeExtract %float %1 0
     OpReturnValue %2
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %res = OpFunctionCall %float %foo %float_50
     OpReturn
     OpFunctionEnd
  )",
              R"(
tint_symbol_2 = struct @align(4) {
  tint_symbol:f32 @offset(0)
  tint_symbol_1:f32 @offset(4)
}

__modf_result_f32 = struct @align(4) {
  fract:f32 @offset(0)
  whole:f32 @offset(4)
}

%foo = func(%x:f32):f32 {
  $B1: {
    %3:__modf_result_f32 = modf %x
    %4:f32 = access %3, 0u
    %5:f32 = access %3, 1u
    %6:tint_symbol_2 = construct %4, %5
    %7:f32 = access %6, 0u
    ret %7
  }
}
)");
}

TEST_F(SpirvReaderTest, ModfStruct_Vector) {
    EXPECT_IR(Preamble(R"(  OpName %foo "foo"
  OpName %x "x"
)") + R"(
     %foo = OpFunction %v2float None %fn_v2float_v2float
     %x = OpFunctionParameter %v2float
     %foo_entry = OpLabel
     %1 = OpExtInst %modf_v2_result_type %glsl ModfStruct %x
     %2 = OpCompositeExtract %v2float %1 0
     OpReturnValue %2
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %res = OpFunctionCall %v2float %foo %v2float_50_60
     OpReturn
     OpFunctionEnd
  )",
              R"(
tint_symbol_2 = struct @align(8) {
  tint_symbol:vec2<f32> @offset(0)
  tint_symbol_1:vec2<f32> @offset(8)
}

__modf_result_vec2_f32 = struct @align(8) {
  fract:vec2<f32> @offset(0)
  whole:vec2<f32> @offset(8)
}

%foo = func(%x:vec2<f32>):vec2<f32> {
  $B1: {
    %3:__modf_result_vec2_f32 = modf %x
    %4:vec2<f32> = access %3, 0u
    %5:vec2<f32> = access %3, 1u
    %6:tint_symbol_2 = construct %4, %5
    %7:vec2<f32> = access %6, 0u
    ret %7
  }
}
)");
}

TEST_F(SpirvReaderTest, FrexpStruct_Store) {
    EXPECT_IR(Preamble(R"(  OpName %foo "foo"
  OpName %x "x"
)") + R"(
     %foo = OpFunction %void None %fn_void_float
     %x = OpFunctionParameter %float
     %foo_entry = OpLabel
     %1 = OpVariable %ptr_function_frexp_result_type_unsigned Function
     %2 = OpExtInst %frexp_result_type_unsigned %glsl FrexpStruct %x
     OpStore %1 %2
     OpReturn
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %res = OpFunctionCall %void %foo %float_50
     OpReturn
     OpFunctionEnd
  )",
              R"(
tint_symbol_2 = struct @align(4) {
  tint_symbol:f32 @offset(0)
  tint_symbol_1:u32 @offset(4)
}

__frexp_result_f32 = struct @align(4) {
  fract:f32 @offset(0)
  exp:i32 @offset(4)
}

%foo = func(%x:f32):void {
  $B1: {
    %3:ptr<function, tint_symbol_2, read_write> = var undef
    %4:__frexp_result_f32 = frexp %x
    %5:f32 = access %4, 0u
    %6:i32 = access %4, 1u
    %7:u32 = bitcast<u32> %6
    %8:tint_symbol_2 = construct %5, %7
    store %3, %8
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, FrexpStruct_ScalarUnsigned) {
    EXPECT_IR(Preamble(R"(  OpName %foo "foo"
  OpName %x "x"
)") + R"(
     %foo = OpFunction %void None %fn_void_float
     %x = OpFunctionParameter %float
     %foo_entry = OpLabel
     %1 = OpExtInst %frexp_result_type_unsigned %glsl FrexpStruct %x
     %2 = OpCompositeExtract %float %1 0
     %3 = OpCompositeExtract %uint %1 1
     OpReturn
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %res = OpFunctionCall %void %foo %float_50
     OpReturn
     OpFunctionEnd
  )",
              R"(
tint_symbol_2 = struct @align(4) {
  tint_symbol:f32 @offset(0)
  tint_symbol_1:u32 @offset(4)
}

__frexp_result_f32 = struct @align(4) {
  fract:f32 @offset(0)
  exp:i32 @offset(4)
}

%foo = func(%x:f32):void {
  $B1: {
    %3:__frexp_result_f32 = frexp %x
    %4:f32 = access %3, 0u
    %5:i32 = access %3, 1u
    %6:u32 = bitcast<u32> %5
    %7:tint_symbol_2 = construct %4, %6
    %8:f32 = access %7, 0u
    %9:u32 = access %7, 1u
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, FrexpStruct_ScalarSigned) {
    EXPECT_IR(Preamble(R"(  OpName %foo "foo"
  OpName %x "x"
)") + R"(
     %foo = OpFunction %void None %fn_void_float
     %x = OpFunctionParameter %float
     %foo_entry = OpLabel
     %1 = OpExtInst %frexp_result_type_signed %glsl FrexpStruct %x
     %2 = OpCompositeExtract %float %1 0
     %3 = OpCompositeExtract %int %1 1
     OpReturn
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %res = OpFunctionCall %void %foo %float_50
     OpReturn
     OpFunctionEnd
  )",
              R"(
tint_symbol_2 = struct @align(4) {
  tint_symbol:f32 @offset(0)
  tint_symbol_1:i32 @offset(4)
}

__frexp_result_f32 = struct @align(4) {
  fract:f32 @offset(0)
  exp:i32 @offset(4)
}

%foo = func(%x:f32):void {
  $B1: {
    %3:__frexp_result_f32 = frexp %x
    %4:f32 = access %3, 0u
    %5:i32 = access %3, 1u
    %6:tint_symbol_2 = construct %4, %5
    %7:f32 = access %6, 0u
    %8:i32 = access %6, 1u
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, FrexpStruct_VectorUnsigned) {
    EXPECT_IR(Preamble(R"(  OpName %foo "foo"
  OpName %x "x"
)") + R"(
     %foo = OpFunction %void None %fn_void_v2float
     %x = OpFunctionParameter %v2float
     %foo_entry = OpLabel
     %1 = OpExtInst %frexp_v2_result_type_unsigned %glsl FrexpStruct %x
     %2 = OpCompositeExtract %v2float %1 0
     %3 = OpCompositeExtract %v2uint %1 1
     OpReturn
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %res = OpFunctionCall %void %foo %v2float_50_60
     OpReturn
     OpFunctionEnd
  )",
              R"(
tint_symbol_2 = struct @align(8) {
  tint_symbol:vec2<f32> @offset(0)
  tint_symbol_1:vec2<u32> @offset(8)
}

__frexp_result_vec2_f32 = struct @align(8) {
  fract:vec2<f32> @offset(0)
  exp:vec2<i32> @offset(8)
}

%foo = func(%x:vec2<f32>):void {
  $B1: {
    %3:__frexp_result_vec2_f32 = frexp %x
    %4:vec2<f32> = access %3, 0u
    %5:vec2<i32> = access %3, 1u
    %6:vec2<u32> = bitcast<vec2<u32>> %5
    %7:tint_symbol_2 = construct %4, %6
    %8:vec2<f32> = access %7, 0u
    %9:vec2<u32> = access %7, 1u
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, FrexpStruct_VectorSigned) {
    EXPECT_IR(Preamble(R"(  OpName %foo "foo"
  OpName %x "x"
)") + R"(
     %foo = OpFunction %void None %fn_void_v2float
     %x = OpFunctionParameter %v2float
     %foo_entry = OpLabel
     %1 = OpExtInst %frexp_v2_result_type_signed %glsl FrexpStruct %x
     %2 = OpCompositeExtract %v2float %1 0
     %3 = OpCompositeExtract %v2int %1 1
     OpReturn
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %res = OpFunctionCall %void %foo %v2float_50_60
     OpReturn
     OpFunctionEnd
  )",
              R"(
tint_symbol_2 = struct @align(8) {
  tint_symbol:vec2<f32> @offset(0)
  tint_symbol_1:vec2<i32> @offset(8)
}

__frexp_result_vec2_f32 = struct @align(8) {
  fract:vec2<f32> @offset(0)
  exp:vec2<i32> @offset(8)
}

%foo = func(%x:vec2<f32>):void {
  $B1: {
    %3:__frexp_result_vec2_f32 = frexp %x
    %4:vec2<f32> = access %3, 0u
    %5:vec2<i32> = access %3, 1u
    %6:tint_symbol_2 = construct %4, %5
    %7:vec2<f32> = access %6, 0u
    %8:vec2<i32> = access %6, 1u
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::spirv::reader
