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

#include "src/transform/hlsl.h"

#include <string>

#include "src/transform/test_helper.h"

namespace tint {
namespace transform {
namespace {

using HlslTest = TransformTest;

TEST_F(HlslTest, PromoteArrayInitializerToConstVar_Basic) {
  auto* src = R"(
[[stage(vertex)]]
fn main() {
  var f0 : f32 = 1.0;
  var f1 : f32 = 2.0;
  var f2 : f32 = 3.0;
  var f3 : f32 = 4.0;
  var i : f32 = array<f32, 4>(f0, f1, f2, f3)[2];
}
)";

  auto* expect = R"(
[[stage(vertex)]]
fn main() {
  var f0 : f32 = 1.0;
  var f1 : f32 = 2.0;
  var f2 : f32 = 3.0;
  var f3 : f32 = 4.0;
  let tint_symbol : array<f32, 4> = array<f32, 4>(f0, f1, f2, f3);
  var i : f32 = tint_symbol[2];
}
)";

  auto got = Run<Hlsl>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(HlslTest, PromoteStructureInitializerToConstVar_Basic) {
  auto* src = R"(
struct S {
  a : i32;
  b : f32;
  c : vec3<f32>;
};

[[stage(vertex)]]
fn main() {
  var x : f32 = S(1, 2.0, vec3<f32>()).b;
}
)";

  auto* expect = R"(
struct S {
  a : i32;
  b : f32;
  c : vec3<f32>;
};

[[stage(vertex)]]
fn main() {
  let tint_symbol : S = S(1, 2.0, vec3<f32>());
  var x : f32 = tint_symbol.b;
}
)";

  auto got = Run<Hlsl>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(HlslTest, PromoteArrayInitializerToConstVar_ArrayInArray) {
  auto* src = R"(
[[stage(vertex)]]
fn main() {
  var i : f32 = array<array<f32, 2>, 2>(array<f32, 2>(1.0, 2.0), array<f32, 2>(3.0, 4.0))[0][1];
}
)";

  auto* expect = R"(
[[stage(vertex)]]
fn main() {
  let tint_symbol : array<f32, 2> = array<f32, 2>(1.0, 2.0);
  let tint_symbol_1 : array<f32, 2> = array<f32, 2>(3.0, 4.0);
  let tint_symbol_2 : array<array<f32, 2>, 2> = array<array<f32, 2>, 2>(tint_symbol, tint_symbol_1);
  var i : f32 = tint_symbol_2[0][1];
}
)";

  auto got = Run<Hlsl>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(HlslTest, PromoteStructureInitializerToConstVar_Nested) {
  auto* src = R"(
struct S1 {
  a : i32;
};

struct S2 {
  a : i32;
  b : S1;
  c : i32;
};

struct S3 {
  a : S2;
};

[[stage(vertex)]]
fn main() {
  var x : i32 = S3(S2(1, S1(2), 3)).a.b.a;
}
)";

  auto* expect = R"(
struct S1 {
  a : i32;
};

struct S2 {
  a : i32;
  b : S1;
  c : i32;
};

struct S3 {
  a : S2;
};

[[stage(vertex)]]
fn main() {
  let tint_symbol : S1 = S1(2);
  let tint_symbol_1 : S2 = S2(1, tint_symbol, 3);
  let tint_symbol_2 : S3 = S3(tint_symbol_1);
  var x : i32 = tint_symbol_2.a.b.a;
}
)";

  auto got = Run<Hlsl>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(HlslTest, PromoteInitializerToConstVar_Mixed) {
  auto* src = R"(
struct S1 {
  a : i32;
};

struct S2 {
  a : array<S1, 3>;
};

[[stage(vertex)]]
fn main() {
  var x : i32 = S2(array<S1, 3>(S1(1), S1(2), S1(3))).a[1].a;
}
)";

  auto* expect = R"(
struct S1 {
  a : i32;
};

struct S2 {
  a : array<S1, 3>;
};

[[stage(vertex)]]
fn main() {
  let tint_symbol : S1 = S1(1);
  let tint_symbol_1 : S1 = S1(2);
  let tint_symbol_2 : S1 = S1(3);
  let tint_symbol_3 : array<S1, 3> = array<S1, 3>(tint_symbol, tint_symbol_1, tint_symbol_2);
  let tint_symbol_4 : S2 = S2(tint_symbol_3);
  var x : i32 = tint_symbol_4.a[1].a;
}
)";

  auto got = Run<Hlsl>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(HlslTest, PromoteInitializerToConstVar_NoChangeOnVarDecl) {
  auto* src = R"(
struct S {
  a : i32;
  b : f32;
  c : i32;
};

[[stage(vertex)]]
fn main() {
  var local_arr : array<f32, 4> = array<f32, 4>(0.0, 1.0, 2.0, 3.0);
  var local_str : S = S(1, 2.0, 3);
}

let module_arr : array<f32, 4> = array<f32, 4>(0.0, 1.0, 2.0, 3.0);

let module_str : S = S(1, 2.0, 3);
)";

  auto* expect = src;

  auto got = Run<Hlsl>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(HlslTest, PromoteArrayInitializerToConstVar_Bug406) {
  // See crbug.com/tint/406
  auto* src = R"(
[[block]]
struct Uniforms {
  transform : mat2x2<f32>;
};

[[group(0), binding(0)]] var<uniform> ubo : Uniforms;

[[builtin(vertex_index)]] var<in> vertex_index : u32;

[[builtin(position)]] var<out> position : vec4<f32>;

[[stage(vertex)]]
fn main() {
  let transform : mat2x2<f32> = ubo.transform;
  var coord : vec2<f32> = array<vec2<f32>, 3>(
      vec2<f32>(-1.0,  1.0),
      vec2<f32>( 1.0,  1.0),
      vec2<f32>(-1.0, -1.0)
  )[vertex_index];
  position = vec4<f32>(transform * coord, 0.0, 1.0);
}
)";

  auto* expect = R"(
[[block]]
struct Uniforms {
  transform : mat2x2<f32>;
};

[[group(0), binding(0)]] var<uniform> ubo : Uniforms;

[[builtin(vertex_index)]] var<in> vertex_index : u32;

[[builtin(position)]] var<out> position : vec4<f32>;

[[stage(vertex)]]
fn main() {
  let transform : mat2x2<f32> = ubo.transform;
  let tint_symbol : array<vec2<f32>, 3> = array<vec2<f32>, 3>(vec2<f32>(-1.0, 1.0), vec2<f32>(1.0, 1.0), vec2<f32>(-1.0, -1.0));
  var coord : vec2<f32> = tint_symbol[vertex_index];
  position = vec4<f32>((transform * coord), 0.0, 1.0);
}
)";

  auto got = Run<Hlsl>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(HlslTest, AddEmptyEntryPoint) {
  auto* src = R"()";

  auto* expect = R"(
[[stage(vertex)]]
fn _tint_unused_entry_point() {
}
)";

  auto got = Run<Hlsl>(src);

  EXPECT_EQ(expect, str(got));
}

using HlslReservedKeywordTest = TransformTestWithParam<std::string>;

TEST_P(HlslReservedKeywordTest, Keywords) {
  auto keyword = GetParam();

  auto src = R"(
[[stage(fragment)]]
fn main() {
  var )" + keyword +
             R"( : i32;
}
)";

  auto expect = R"(
[[stage(fragment)]]
fn main() {
  var tint_)" + keyword +
                R"( : i32;
}
)";

  auto got = Run<Hlsl>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_P(HlslReservedKeywordTest, AttemptSymbolCollision) {
  auto keyword = GetParam();

  auto src = R"(
[[stage(fragment)]]
fn main() {
  var tint_)" +
             keyword +
             R"( : i32;
  var tint_)" +
             keyword +
             R"(_1 : i32;
  var )" + keyword +
             R"( : i32;
  var tint_)" +
             keyword +
             R"(_2 : i32;
  var tint_)" +
             keyword +
             R"(_3 : i32;
}
)";

  auto expect = R"(
[[stage(fragment)]]
fn main() {
  var tint_)" + keyword +
                R"( : i32;
  var tint_)" + keyword +
                R"(_1 : i32;
  var tint_)" + keyword +
                R"(_2 : i32;
  var tint_)" + keyword +
                R"(_2_1 : i32;
  var tint_)" + keyword +
                R"(_3 : i32;
}
)";

  auto got = Run<Hlsl>(src);

  EXPECT_EQ(expect, str(got));
}

INSTANTIATE_TEST_SUITE_P(HlslReservedKeywordTest,
                         HlslReservedKeywordTest,
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

}  // namespace
}  // namespace transform
}  // namespace tint
