// Copyright 2021 The Tint Authors.
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

#include "src/transform/renamer.h"

#include <memory>

#include "gmock/gmock.h"
#include "src/transform/test_helper.h"

namespace tint {
namespace transform {
namespace {

using ::testing::ContainerEq;

using RenamerTest = TransformTest;

TEST_F(RenamerTest, EmptyModule) {
  auto* src = "";
  auto* expect = "";

  auto got = Run<Renamer>(src);

  EXPECT_EQ(expect, str(got));

  auto* data = got.data.Get<Renamer::Data>();

  ASSERT_EQ(data->remappings.size(), 0u);
}

TEST_F(RenamerTest, BasicModuleVertexIndex) {
  auto* src = R"(
[[builtin(vertex_index)]] var<in> vert_idx : u32;

fn test() -> u32 {
  return vert_idx;
}

[[stage(vertex)]]
fn entry() -> [[builtin(position)]] vec4<f32>  {
  test();
  return vec4<f32>();
}
)";

  auto* expect = R"(
[[builtin(vertex_index)]] var<in> tint_symbol : u32;

fn tint_symbol_1() -> u32 {
  return tint_symbol;
}

[[stage(vertex)]]
fn tint_symbol_2() -> [[builtin(position)]] vec4<f32> {
  tint_symbol_1();
  return vec4<f32>();
}
)";

  auto got = Run<Renamer>(src);

  EXPECT_EQ(expect, str(got));

  auto* data = got.data.Get<Renamer::Data>();

  ASSERT_NE(data, nullptr);
  Renamer::Data::Remappings expected_remappings = {
      {"vert_idx", "tint_symbol"},
      {"test", "tint_symbol_1"},
      {"entry", "tint_symbol_2"},
  };
  EXPECT_THAT(data->remappings, ContainerEq(expected_remappings));
}

TEST_F(RenamerTest, PreserveSwizzles) {
  auto* src = R"(
[[stage(vertex)]]
fn entry() -> [[builtin(position)]] vec4<f32> {
  var v : vec4<f32>;
  var rgba : f32;
  var xyzw : f32;
  return v.zyxw + v.rgab;
}
)";

  auto* expect = R"(
[[stage(vertex)]]
fn tint_symbol() -> [[builtin(position)]] vec4<f32> {
  var tint_symbol_1 : vec4<f32>;
  var tint_symbol_2 : f32;
  var tint_symbol_3 : f32;
  return (tint_symbol_1.zyxw + tint_symbol_1.rgab);
}
)";

  auto got = Run<Renamer>(src);

  EXPECT_EQ(expect, str(got));

  auto* data = got.data.Get<Renamer::Data>();

  ASSERT_NE(data, nullptr);
  Renamer::Data::Remappings expected_remappings = {
      {"entry", "tint_symbol"},
      {"v", "tint_symbol_1"},
      {"rgba", "tint_symbol_2"},
      {"xyzw", "tint_symbol_3"},
  };
  EXPECT_THAT(data->remappings, ContainerEq(expected_remappings));
}

TEST_F(RenamerTest, PreserveIntrinsics) {
  auto* src = R"(
[[stage(vertex)]]
fn entry() -> [[builtin(position)]] vec4<f32> {
  var blah : vec4<f32>;
  return abs(blah);
}
)";

  auto* expect = R"(
[[stage(vertex)]]
fn tint_symbol() -> [[builtin(position)]] vec4<f32> {
  var tint_symbol_1 : vec4<f32>;
  return abs(tint_symbol_1);
}
)";

  auto got = Run<Renamer>(src);

  EXPECT_EQ(expect, str(got));

  auto* data = got.data.Get<Renamer::Data>();

  ASSERT_NE(data, nullptr);
  Renamer::Data::Remappings expected_remappings = {
      {"entry", "tint_symbol"},
      {"blah", "tint_symbol_1"},
  };
  EXPECT_THAT(data->remappings, ContainerEq(expected_remappings));
}

TEST_F(RenamerTest, AttemptSymbolCollision) {
  auto* src = R"(
[[stage(vertex)]]
fn entry() -> [[builtin(position)]] vec4<f32> {
  var tint_symbol : vec4<f32>;
  var tint_symbol_2 : vec4<f32>;
  var tint_symbol_4 : vec4<f32>;
  return tint_symbol + tint_symbol_2 + tint_symbol_4;
}
)";

  auto* expect = R"(
[[stage(vertex)]]
fn tint_symbol() -> [[builtin(position)]] vec4<f32> {
  var tint_symbol_1 : vec4<f32>;
  var tint_symbol_2 : vec4<f32>;
  var tint_symbol_3 : vec4<f32>;
  return ((tint_symbol_1 + tint_symbol_2) + tint_symbol_3);
}
)";

  auto got = Run<Renamer>(src);

  EXPECT_EQ(expect, str(got));

  auto* data = got.data.Get<Renamer::Data>();

  ASSERT_NE(data, nullptr);
  Renamer::Data::Remappings expected_remappings = {
      {"entry", "tint_symbol"},
      {"tint_symbol", "tint_symbol_1"},
      {"tint_symbol_2", "tint_symbol_2"},
      {"tint_symbol_4", "tint_symbol_3"},
  };
  EXPECT_THAT(data->remappings, ContainerEq(expected_remappings));
}

using RenamerTestHlsl = TransformTestWithParam<std::string>;
using RenamerTestMsl = TransformTestWithParam<std::string>;

TEST_P(RenamerTestHlsl, Keywords) {
  auto keyword = GetParam();

  auto src = R"(
[[stage(fragment)]]
fn frag_main() {
  var )" + keyword +
             R"( : i32;
}
)";

  auto* expect = R"(
[[stage(fragment)]]
fn frag_main() {
  var tint_symbol : i32;
}
)";

  Renamer::Config config{Renamer::Target::kHlslKeywords};
  auto got = Run(src, std::make_unique<Renamer>(config));

  EXPECT_EQ(expect, str(got));
}

TEST_P(RenamerTestMsl, Keywords) {
  auto keyword = GetParam();

  auto src = R"(
[[stage(fragment)]]
fn frag_main() {
  var )" + keyword +
             R"( : i32;
}
)";

  auto* expect = R"(
[[stage(fragment)]]
fn frag_main() {
  var tint_symbol : i32;
}
)";

  Renamer::Config config{Renamer::Target::kMslKeywords};
  auto got = Run(src, std::make_unique<Renamer>(config));

  EXPECT_EQ(expect, str(got));
}

INSTANTIATE_TEST_SUITE_P(RenamerTestHlsl,
                         RenamerTestHlsl,
                         testing::Values("AddressU",
                                         "AddressV",
                                         "AddressW",
                                         "AllMemoryBarrier",
                                         "AllMemoryBarrierWithGroupSync",
                                         "AppendStructuredBuffer",
                                         "BINORMAL",
                                         "BLENDINDICES",
                                         "BLENDWEIGHT",
                                         "BlendState",
                                         "BorderColor",
                                         "Buffer",
                                         "ByteAddressBuffer",
                                         "COLOR",
                                         "CheckAccessFullyMapped",
                                         "ComparisonFunc",
                                         "CompileShader",
                                         "ComputeShader",
                                         "ConsumeStructuredBuffer",
                                         "D3DCOLORtoUBYTE4",
                                         "DEPTH",
                                         "DepthStencilState",
                                         "DepthStencilView",
                                         "DeviceMemoryBarrier",
                                         "DeviceMemroyBarrierWithGroupSync",
                                         "DomainShader",
                                         "EvaluateAttributeAtCentroid",
                                         "EvaluateAttributeAtSample",
                                         "EvaluateAttributeSnapped",
                                         "FOG",
                                         "Filter",
                                         "GeometryShader",
                                         "GetRenderTargetSampleCount",
                                         "GetRenderTargetSamplePosition",
                                         "GroupMemoryBarrier",
                                         "GroupMemroyBarrierWithGroupSync",
                                         "Hullshader",
                                         "InputPatch",
                                         "InterlockedAdd",
                                         "InterlockedAnd",
                                         "InterlockedCompareExchange",
                                         "InterlockedCompareStore",
                                         "InterlockedExchange",
                                         "InterlockedMax",
                                         "InterlockedMin",
                                         "InterlockedOr",
                                         "InterlockedXor",
                                         "LineStream",
                                         "MaxAnisotropy",
                                         "MaxLOD",
                                         "MinLOD",
                                         "MipLODBias",
                                         "NORMAL",
                                         "NULL",
                                         "Normal",
                                         "OutputPatch",
                                         "POSITION",
                                         "POSITIONT",
                                         "PSIZE",
                                         "PixelShader",
                                         "PointStream",
                                         "Process2DQuadTessFactorsAvg",
                                         "Process2DQuadTessFactorsMax",
                                         "Process2DQuadTessFactorsMin",
                                         "ProcessIsolineTessFactors",
                                         "ProcessQuadTessFactorsAvg",
                                         "ProcessQuadTessFactorsMax",
                                         "ProcessQuadTessFactorsMin",
                                         "ProcessTriTessFactorsAvg",
                                         "ProcessTriTessFactorsMax",
                                         "ProcessTriTessFactorsMin",
                                         "RWBuffer",
                                         "RWByteAddressBuffer",
                                         "RWStructuredBuffer",
                                         "RWTexture1D",
                                         "RWTexture2D",
                                         "RWTexture2DArray",
                                         "RWTexture3D",
                                         "RasterizerState",
                                         "RenderTargetView",
                                         "SV_ClipDistance",
                                         "SV_Coverage",
                                         "SV_CullDistance",
                                         "SV_Depth",
                                         "SV_DepthGreaterEqual",
                                         "SV_DepthLessEqual",
                                         "SV_DispatchThreadID",
                                         "SV_DomainLocation",
                                         "SV_GSInstanceID",
                                         "SV_GroupID",
                                         "SV_GroupIndex",
                                         "SV_GroupThreadID",
                                         "SV_InnerCoverage",
                                         "SV_InsideTessFactor",
                                         "SV_InstanceID",
                                         "SV_IsFrontFace",
                                         "SV_OutputControlPointID",
                                         "SV_Position",
                                         "SV_PrimitiveID",
                                         "SV_RenderTargetArrayIndex",
                                         "SV_SampleIndex",
                                         "SV_StencilRef",
                                         "SV_Target",
                                         "SV_TessFactor",
                                         "SV_VertexArrayIndex",
                                         "SV_VertexID",
                                         "Sampler",
                                         "Sampler1D",
                                         "Sampler2D",
                                         "Sampler3D",
                                         "SamplerCUBE",
                                         "StructuredBuffer",
                                         "TANGENT",
                                         "TESSFACTOR",
                                         "TEXCOORD",
                                         "Texcoord",
                                         "Texture",
                                         "Texture1D",
                                         "Texture2D",
                                         "Texture2DArray",
                                         "Texture2DMS",
                                         "Texture2DMSArray",
                                         "Texture3D",
                                         "TextureCube",
                                         "TextureCubeArray",
                                         "TriangleStream",
                                         "VFACE",
                                         "VPOS",
                                         "VertexShader",
                                         "abort",
                                         // "abs",  // WGSL intrinsic
                                         // "acos",  // WGSL intrinsic
                                         // "all",  // WGSL intrinsic
                                         "allow_uav_condition",
                                         // "any",  // WGSL intrinsic
                                         "asdouble",
                                         "asfloat",
                                         // "asin",  // WGSL intrinsic
                                         "asint",
                                         // "asm",  // WGSL keyword
                                         "asm_fragment",
                                         "asuint",
                                         // "atan",  // WGSL intrinsic
                                         // "atan2",  // WGSL intrinsic
                                         "auto",
                                         // "bool",  // WGSL keyword
                                         "bool1",
                                         "bool1x1",
                                         "bool1x2",
                                         "bool1x3",
                                         "bool1x4",
                                         "bool2",
                                         "bool2x1",
                                         "bool2x2",
                                         "bool2x3",
                                         "bool2x4",
                                         "bool3",
                                         "bool3x1",
                                         "bool3x2",
                                         "bool3x3",
                                         "bool3x4",
                                         "bool4",
                                         "bool4x1",
                                         "bool4x2",
                                         "bool4x3",
                                         "bool4x4",
                                         "branch",
                                         // "break",  // WGSL keyword
                                         // "call",  // WGSL intrinsic
                                         // "case",  // WGSL keyword
                                         "catch",
                                         "cbuffer",
                                         // "ceil",  // WGSL intrinsic
                                         "centroid",
                                         "char",
                                         // "clamp",  // WGSL intrinsic
                                         "class",
                                         "clip",
                                         "column_major",
                                         "compile_fragment",
                                         // "const",  // WGSL keyword
                                         "const_cast",
                                         // "continue",  // WGSL keyword
                                         // "cos",  // WGSL intrinsic
                                         // "cosh",  // WGSL intrinsic
                                         "countbits",
                                         // "cross",  // WGSL intrinsic
                                         "ddx",
                                         "ddx_coarse",
                                         "ddx_fine",
                                         "ddy",
                                         "ddy_coarse",
                                         "ddy_fine",
                                         "degrees",
                                         "delete",
                                         // "determinant",  // WGSL intrinsic
                                         // "discard",  // WGSL keyword
                                         // "distance",  // WGSL intrinsic
                                         // "do",  // WGSL keyword
                                         // "dot",  // WGSL intrinsic
                                         "double",
                                         "double1",
                                         "double1x1",
                                         "double1x2",
                                         "double1x3",
                                         "double1x4",
                                         "double2",
                                         "double2x1",
                                         "double2x2",
                                         "double2x3",
                                         "double2x4",
                                         "double3",
                                         "double3x1",
                                         "double3x2",
                                         "double3x3",
                                         "double3x4",
                                         "double4",
                                         "double4x1",
                                         "double4x2",
                                         "double4x3",
                                         "double4x4",
                                         "dst",
                                         "dword",
                                         "dword1",
                                         "dword1x1",
                                         "dword1x2",
                                         "dword1x3",
                                         "dword1x4",
                                         "dword2",
                                         "dword2x1",
                                         "dword2x2",
                                         "dword2x3",
                                         "dword2x4",
                                         "dword3",
                                         "dword3x1",
                                         "dword3x2",
                                         "dword3x3",
                                         "dword3x4",
                                         "dword4",
                                         "dword4x1",
                                         "dword4x2",
                                         "dword4x3",
                                         "dword4x4",
                                         "dynamic_cast",
                                         // "else",  // WGSL keyword
                                         // "enum",  // WGSL keyword
                                         "errorf",
                                         // "exp",  // WGSL intrinsic
                                         // "exp2",  // WGSL intrinsic
                                         "explicit",
                                         "export",
                                         "extern",
                                         "f16to32",
                                         "f32tof16",
                                         // "faceforward",  // WGSL intrinsic
                                         // "false",  // WGSL keyword
                                         "fastopt",
                                         "firstbithigh",
                                         "firstbitlow",
                                         "flatten",
                                         "float",
                                         "float1",
                                         "float1x1",
                                         "float1x2",
                                         "float1x3",
                                         "float1x4",
                                         "float2",
                                         "float2x1",
                                         "float2x2",
                                         "float2x3",
                                         "float2x4",
                                         "float3",
                                         "float3x1",
                                         "float3x2",
                                         "float3x3",
                                         "float3x4",
                                         "float4",
                                         "float4x1",
                                         "float4x2",
                                         "float4x3",
                                         "float4x4",
                                         // "floor",  // WGSL intrinsic
                                         // "fma",  // WGSL intrinsic
                                         "fmod",
                                         // "for",  // WGSL keyword
                                         "forcecase",
                                         "frac",
                                         // "frexp",  // WGSL intrinsic
                                         "friend",
                                         // "fwidth",  // WGSL intrinsic
                                         "fxgroup",
                                         "goto",
                                         "groupshared",
                                         "half",
                                         "half1",
                                         "half1x1",
                                         "half1x2",
                                         "half1x3",
                                         "half1x4",
                                         "half2",
                                         "half2x1",
                                         "half2x2",
                                         "half2x3",
                                         "half2x4",
                                         "half3",
                                         "half3x1",
                                         "half3x2",
                                         "half3x3",
                                         "half3x4",
                                         "half4",
                                         "half4x1",
                                         "half4x2",
                                         "half4x3",
                                         "half4x4",
                                         // "if",  // WGSL keyword
                                         // "in",  // WGSL keyword
                                         "inline",
                                         "inout",
                                         "int",
                                         "int1",
                                         "int1x1",
                                         "int1x2",
                                         "int1x3",
                                         "int1x4",
                                         "int2",
                                         "int2x1",
                                         "int2x2",
                                         "int2x3",
                                         "int2x4",
                                         "int3",
                                         "int3x1",
                                         "int3x2",
                                         "int3x3",
                                         "int3x4",
                                         "int4",
                                         "int4x1",
                                         "int4x2",
                                         "int4x3",
                                         "int4x4",
                                         "interface",
                                         "isfinite",
                                         "isinf",
                                         "isnan",
                                         // "ldexp",  // WGSL intrinsic
                                         // "length",  // WGSL intrinsic
                                         "lerp",
                                         "lineadj",
                                         "linear",
                                         "lit",
                                         // "log",  // WGSL intrinsic
                                         "log10",
                                         // "log2",  // WGSL intrinsic
                                         "long",
                                         // "loop",  // WGSL keyword
                                         "mad",
                                         "matrix",
                                         // "max",  // WGSL intrinsic
                                         // "min",  // WGSL intrinsic
                                         "min10float",
                                         "min10float1",
                                         "min10float1x1",
                                         "min10float1x2",
                                         "min10float1x3",
                                         "min10float1x4",
                                         "min10float2",
                                         "min10float2x1",
                                         "min10float2x2",
                                         "min10float2x3",
                                         "min10float2x4",
                                         "min10float3",
                                         "min10float3x1",
                                         "min10float3x2",
                                         "min10float3x3",
                                         "min10float3x4",
                                         "min10float4",
                                         "min10float4x1",
                                         "min10float4x2",
                                         "min10float4x3",
                                         "min10float4x4",
                                         "min12int",
                                         "min12int1",
                                         "min12int1x1",
                                         "min12int1x2",
                                         "min12int1x3",
                                         "min12int1x4",
                                         "min12int2",
                                         "min12int2x1",
                                         "min12int2x2",
                                         "min12int2x3",
                                         "min12int2x4",
                                         "min12int3",
                                         "min12int3x1",
                                         "min12int3x2",
                                         "min12int3x3",
                                         "min12int3x4",
                                         "min12int4",
                                         "min12int4x1",
                                         "min12int4x2",
                                         "min12int4x3",
                                         "min12int4x4",
                                         "min16float",
                                         "min16float1",
                                         "min16float1x1",
                                         "min16float1x2",
                                         "min16float1x3",
                                         "min16float1x4",
                                         "min16float2",
                                         "min16float2x1",
                                         "min16float2x2",
                                         "min16float2x3",
                                         "min16float2x4",
                                         "min16float3",
                                         "min16float3x1",
                                         "min16float3x2",
                                         "min16float3x3",
                                         "min16float3x4",
                                         "min16float4",
                                         "min16float4x1",
                                         "min16float4x2",
                                         "min16float4x3",
                                         "min16float4x4",
                                         "min16int",
                                         "min16int1",
                                         "min16int1x1",
                                         "min16int1x2",
                                         "min16int1x3",
                                         "min16int1x4",
                                         "min16int2",
                                         "min16int2x1",
                                         "min16int2x2",
                                         "min16int2x3",
                                         "min16int2x4",
                                         "min16int3",
                                         "min16int3x1",
                                         "min16int3x2",
                                         "min16int3x3",
                                         "min16int3x4",
                                         "min16int4",
                                         "min16int4x1",
                                         "min16int4x2",
                                         "min16int4x3",
                                         "min16int4x4",
                                         "min16uint",
                                         "min16uint1",
                                         "min16uint1x1",
                                         "min16uint1x2",
                                         "min16uint1x3",
                                         "min16uint1x4",
                                         "min16uint2",
                                         "min16uint2x1",
                                         "min16uint2x2",
                                         "min16uint2x3",
                                         "min16uint2x4",
                                         "min16uint3",
                                         "min16uint3x1",
                                         "min16uint3x2",
                                         "min16uint3x3",
                                         "min16uint3x4",
                                         "min16uint4",
                                         "min16uint4x1",
                                         "min16uint4x2",
                                         "min16uint4x3",
                                         "min16uint4x4",
                                         // "modf",  // WGSL intrinsic
                                         "msad4",
                                         "mul",
                                         "mutable",
                                         "namespace",
                                         "new",
                                         "nointerpolation",
                                         "noise",
                                         "noperspective",
                                         // "normalize",  // WGSL intrinsic
                                         "numthreads",
                                         "operator",
                                         // "out",  // WGSL keyword
                                         "packoffset",
                                         "pass",
                                         "pixelfragment",
                                         "pixelshader",
                                         "point",
                                         // "pow",  // WGSL intrinsic
                                         "precise",
                                         "printf",
                                         // "private",  // WGSL keyword
                                         "protected",
                                         "public",
                                         "radians",
                                         "rcp",
                                         // "reflect",  // WGSL intrinsic
                                         "refract",
                                         "register",
                                         "reinterpret_cast",
                                         // "return",  // WGSL keyword
                                         // "reversebits",  // WGSL intrinsic
                                         // "round",  // WGSL intrinsic
                                         "row_major",
                                         "rsqrt",
                                         "sample",
                                         "sampler1D",
                                         "sampler2D",
                                         "sampler3D",
                                         "samplerCUBE",
                                         "sampler_state",
                                         "saturate",
                                         "shared",
                                         "short",
                                         // "sign",  // WGSL intrinsic
                                         "signed",
                                         // "sin",  // WGSL intrinsic
                                         "sincos",
                                         // "sinh",  // WGSL intrinsic
                                         "sizeof",
                                         // "smoothstep",  // WGSL intrinsic
                                         "snorm",
                                         // "sqrt",  // WGSL intrinsic
                                         "stateblock",
                                         "stateblock_state",
                                         "static",
                                         "static_cast",
                                         // "step",  // WGSL intrinsic
                                         "string",
                                         // "struct",  // WGSL keyword
                                         // "switch",  // WGSL keyword
                                         // "tan",  // WGSL intrinsic
                                         // "tanh",  // WGSL intrinsic
                                         "tbuffer",
                                         "technique",
                                         "technique10",
                                         "technique11",
                                         "template",
                                         "tex1D",
                                         "tex1Dbias",
                                         "tex1Dgrad",
                                         "tex1Dlod",
                                         "tex1Dproj",
                                         "tex2D",
                                         "tex2Dbias",
                                         "tex2Dgrad",
                                         "tex2Dlod",
                                         "tex2Dproj",
                                         "tex3D",
                                         "tex3Dbias",
                                         "tex3Dgrad",
                                         "tex3Dlod",
                                         "tex3Dproj",
                                         "texCUBE",
                                         "texCUBEbias",
                                         "texCUBEgrad",
                                         "texCUBElod",
                                         "texCUBEproj",
                                         "texture",
                                         "texture1D",
                                         "texture1DArray",
                                         "texture2D",
                                         "texture2DArray",
                                         "texture2DMS",
                                         "texture2DMSArray",
                                         "texture3D",
                                         "textureCube",
                                         "textureCubeArray",
                                         "this",
                                         "throw",
                                         "transpose",
                                         "triangle",
                                         "triangleadj",
                                         // "true",  // WGSL keyword
                                         // "trunc",  // WGSL intrinsic
                                         "try",
                                         // "typedef",  // WGSL keyword
                                         "typename",
                                         "uint",
                                         "uint1",
                                         "uint1x1",
                                         "uint1x2",
                                         "uint1x3",
                                         "uint1x4",
                                         "uint2",
                                         "uint2x1",
                                         "uint2x2",
                                         "uint2x3",
                                         "uint2x4",
                                         "uint3",
                                         "uint3x1",
                                         "uint3x2",
                                         "uint3x3",
                                         "uint3x4",
                                         "uint4",
                                         "uint4x1",
                                         "uint4x2",
                                         "uint4x3",
                                         "uint4x4",
                                         // "uniform",  // WGSL keyword
                                         "union",
                                         "unorm",
                                         "unroll",
                                         "unsigned",
                                         "using",
                                         "vector",
                                         "vertexfragment",
                                         "vertexshader",
                                         "virtual",
                                         // "void",  // WGSL keyword
                                         "volatile",
                                         "while"));

INSTANTIATE_TEST_SUITE_P(RenamerTestMsl,
                         RenamerTestMsl,
                         testing::Values(
                             // c++14 spec
                             "alignas",
                             "alignof",
                             "and",
                             "and_eq",
                             // "asm",  // Also reserved in WGSL
                             "auto",
                             "bitand",
                             "bitor",
                             // "bool",   // Also used in WGSL
                             // "break",  // Also used in WGSL
                             // "case",   // Also used in WGSL
                             "catch",
                             "char",
                             "char16_t",
                             "char32_t",
                             "class",
                             "compl",
                             // "const",     // Also used in WGSL
                             "const_cast",
                             "constexpr",
                             // "continue",  // Also used in WGSL
                             "decltype",
                             // "default",   // Also used in WGSL
                             "delete",
                             // "do",  // Also used in WGSL
                             "double",
                             "dynamic_cast",
                             // "else",  // Also used in WGSL
                             // "enum",  // Also used in WGSL
                             "explicit",
                             "extern",
                             // "false",  // Also used in WGSL
                             "final",
                             "float",
                             // "for",  // Also used in WGSL
                             "friend",
                             "goto",
                             // "if",  // Also used in WGSL
                             "inline",
                             "int",
                             "long",
                             "mutable",
                             "namespace",
                             "new",
                             "noexcept",
                             "not",
                             "not_eq",
                             "nullptr",
                             "operator",
                             "or",
                             "or_eq",
                             "override",
                             // "private",  // Also used in WGSL
                             "protected",
                             "public",
                             "register",
                             "reinterpret_cast",
                             // "return",  // Also used in WGSL
                             "short",
                             "signed",
                             "sizeof",
                             "static",
                             "static_assert",
                             "static_cast",
                             // "struct",  // Also used in WGSL
                             // "switch",  // Also used in WGSL
                             "template",
                             "this",
                             "thread_local",
                             "throw",
                             // "true",  // Also used in WGSL
                             "try",
                             // "typedef",  // Also used in WGSL
                             "typeid",
                             "typename",
                             "union",
                             "unsigned",
                             "using",
                             "virtual",
                             // "void",  // Also used in WGSL
                             "volatile",
                             "wchar_t",
                             "while",
                             "xor",
                             "xor_eq",

                             // Metal Spec
                             "access",
                             // "array",  // Also used in WGSL
                             "array_ref",
                             "as_type",
                             "atomic",
                             "atomic_bool",
                             "atomic_int",
                             "atomic_uint",
                             "bool2",
                             "bool3",
                             "bool4",
                             "buffer",
                             "char2",
                             "char3",
                             "char4",
                             "const_reference",
                             "constant",
                             "depth2d",
                             "depth2d_array",
                             "depth2d_ms",
                             "depth2d_ms_array",
                             "depthcube",
                             "depthcube_array",
                             "device",
                             "discard_fragment",
                             "float2",
                             "float2x2",
                             "float2x3",
                             "float2x4",
                             "float3",
                             "float3x2",
                             "float3x3",
                             "float3x4",
                             "float4",
                             "float4x2",
                             "float4x3",
                             "float4x4",
                             "fragment",
                             "half",
                             "half2",
                             "half2x2",
                             "half2x3",
                             "half2x4",
                             "half3",
                             "half3x2",
                             "half3x3",
                             "half3x4",
                             "half4",
                             "half4x2",
                             "half4x3",
                             "half4x4",
                             "imageblock",
                             "int16_t",
                             "int2",
                             "int3",
                             "int32_t",
                             "int4",
                             "int64_t",
                             "int8_t",
                             "kernel",
                             "long2",
                             "long3",
                             "long4",
                             "main",   // No functions called main
                             "metal",  // The namespace
                             "packed_bool2",
                             "packed_bool3",
                             "packed_bool4",
                             "packed_char2",
                             "packed_char3",
                             "packed_char4",
                             "packed_float2",
                             "packed_float3",
                             "packed_float4",
                             "packed_half2",
                             "packed_half3",
                             "packed_half4",
                             "packed_int2",
                             "packed_int3",
                             "packed_int4",
                             "packed_short2",
                             "packed_short3",
                             "packed_short4",
                             "packed_uchar2",
                             "packed_uchar3",
                             "packed_uchar4",
                             "packed_uint2",
                             "packed_uint3",
                             "packed_uint4",
                             "packed_ushort2",
                             "packed_ushort3",
                             "packed_ushort4",
                             "patch_control_point",
                             "ptrdiff_t",
                             "r16snorm",
                             "r16unorm",
                             // "r8unorm",  // Also used in WGSL
                             "reference",
                             "rg11b10f",
                             "rg16snorm",
                             "rg16unorm",
                             // "rg8snorm",  // Also used in WGSL
                             // "rg8unorm",  // Also used in WGSL
                             "rgb10a2",
                             "rgb9e5",
                             "rgba16snorm",
                             "rgba16unorm",
                             // "rgba8snorm",  // Also used in WGSL
                             // "rgba8unorm",  // Also used in WGSL
                             // "sampler",  // Also used in WGSL
                             "short2",
                             "short3",
                             "short4",
                             "size_t",
                             "srgba8unorm",
                             "texture",
                             "texture1d",
                             "texture1d_array",
                             "texture2d",
                             "texture2d_array",
                             "texture2d_ms",
                             "texture2d_ms_array",
                             "texture3d",
                             "texture_buffer",
                             "texturecube",
                             "texturecube_array",
                             "thread",
                             "threadgroup",
                             "threadgroup_imageblock",
                             "uchar",
                             "uchar2",
                             "uchar3",
                             "uchar4",
                             "uint",
                             "uint16_t",
                             "uint2",
                             "uint3",
                             "uint32_t",
                             "uint4",
                             "uint64_t",
                             "uint8_t",
                             "ulong2",
                             "ulong3",
                             "ulong4",
                             // "uniform",  // Also used in WGSL
                             "ushort",
                             "ushort2",
                             "ushort3",
                             "ushort4",
                             "vec",
                             "vertex"));

}  // namespace
}  // namespace transform
}  // namespace tint
