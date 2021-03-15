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

#include "src/transform/msl.h"

#include <string>

#include "src/transform/test_helper.h"

namespace tint {
namespace transform {
namespace {

using MslReservedKeywordTest = TransformTestWithParam<std::string>;

TEST_F(MslReservedKeywordTest, Basic) {
  auto* src = R"(
struct class {
  delete : i32;
};

[[stage(fragment)]]
fn main() -> void {
  var foo : i32;
  var half : f32;
  var half1 : f32;
  var half2 : f32;
  var _tint_half2 : f32;
}
)";

  auto* expect = R"(
struct _tint_class {
  _tint_delete : i32;
};

[[stage(fragment)]]
fn _tint_main() -> void {
  var foo : i32;
  var _tint_half : f32;
  var half1 : f32;
  var _tint_half2_0 : f32;
  var _tint_half2 : f32;
}
)";

  auto got = Run<Msl>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_P(MslReservedKeywordTest, Keywords) {
  auto keyword = GetParam();

  auto src = R"(
[[stage(fragment)]]
fn main() -> void {
  var )" + keyword +
             R"( : i32;
}
)";

  auto expect = R"(
[[stage(fragment)]]
fn _tint_main() -> void {
  var _tint_)" + keyword +
                R"( : i32;
}
)";

  auto got = Run<Msl>(src);

  EXPECT_EQ(expect, str(got));
}
INSTANTIATE_TEST_SUITE_P(MslReservedKeywordTest,
                         MslReservedKeywordTest,
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

using MslEntryPointIOTest = TransformTest;

TEST_F(MslEntryPointIOTest, HandleEntryPointIOTypes_Parameters) {
  auto* src = R"(
[[stage(fragment)]]
fn frag_main([[builtin(frag_coord)]] coord : vec4<f32>,
             [[location(1)]] loc1 : f32,
             [[location(2)]] loc2 : vec4<u32>) -> void {
  var col : f32 = (coord.x * loc1);
}
)";

  auto* expect = R"(
struct tint_symbol_3 {
  [[location(1)]]
  loc1 : f32;
  [[location(2)]]
  loc2 : vec4<u32>;
};

[[stage(fragment)]]
fn frag_main(tint_symbol_4 : tint_symbol_3, [[builtin(frag_coord)]] coord : vec4<f32>) -> void {
  const loc1 : f32 = tint_symbol_4.loc1;
  const loc2 : vec4<u32> = tint_symbol_4.loc2;
  var col : f32 = (coord.x * loc1);
}
)";

  auto got = Run<Msl>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(MslEntryPointIOTest, HandleEntryPointIOTypes_OnlyBuiltinParameters) {
  // Expect no change.
  auto* src = R"(
[[stage(fragment)]]
fn frag_main([[builtin(frag_coord)]] coord : vec4<f32>) -> void {
}
)";

  auto got = Transform<Msl>(src);

  EXPECT_EQ(src, str(got));
}

TEST_F(MslEntryPointIOTest, HandleEntryPointIOTypes_Parameters_EmptyBody) {
  auto* src = R"(
[[stage(fragment)]]
fn frag_main([[builtin(frag_coord)]] coord : vec4<f32>,
             [[location(1)]] loc1 : f32,
             [[location(2)]] loc2 : vec4<u32>) -> void {
}
)";

  auto* expect = R"(
struct tint_symbol_3 {
  [[location(1)]]
  loc1 : f32;
  [[location(2)]]
  loc2 : vec4<u32>;
};

[[stage(fragment)]]
fn frag_main(tint_symbol_4 : tint_symbol_3, [[builtin(frag_coord)]] coord : vec4<f32>) -> void {
  const loc1 : f32 = tint_symbol_4.loc1;
  const loc2 : vec4<u32> = tint_symbol_4.loc2;
}
)";

  auto got = Run<Msl>(src);

  EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace transform
}  // namespace tint
