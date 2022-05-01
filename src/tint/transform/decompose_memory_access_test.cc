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

#include "src/tint/transform/decompose_memory_access.h"

#include "src/tint/transform/test_helper.h"

namespace tint::transform {
namespace {

using DecomposeMemoryAccessTest = TransformTest;

TEST_F(DecomposeMemoryAccessTest, ShouldRunEmptyModule) {
    auto* src = R"()";

    EXPECT_FALSE(ShouldRun<DecomposeMemoryAccess>(src));
}

TEST_F(DecomposeMemoryAccessTest, ShouldRunStorageBuffer) {
    auto* src = R"(
struct Buffer {
  i : i32,
};
@group(0) @binding(0) var<storage, read_write> sb : Buffer;
)";

    EXPECT_TRUE(ShouldRun<DecomposeMemoryAccess>(src));
}

TEST_F(DecomposeMemoryAccessTest, ShouldRunUniformBuffer) {
    auto* src = R"(
struct Buffer {
  i : i32,
};
@group(0) @binding(0) var<uniform> ub : Buffer;
)";

    EXPECT_TRUE(ShouldRun<DecomposeMemoryAccess>(src));
}

TEST_F(DecomposeMemoryAccessTest, SB_BasicLoad) {
    auto* src = R"(
struct SB {
  a : i32,
  b : u32,
  c : f32,
  d : vec2<i32>,
  e : vec2<u32>,
  f : vec2<f32>,
  g : vec3<i32>,
  h : vec3<u32>,
  i : vec3<f32>,
  j : vec4<i32>,
  k : vec4<u32>,
  l : vec4<f32>,
  m : mat2x2<f32>,
  n : mat2x3<f32>,
  o : mat2x4<f32>,
  p : mat3x2<f32>,
  q : mat3x3<f32>,
  r : mat3x4<f32>,
  s : mat4x2<f32>,
  t : mat4x3<f32>,
  u : mat4x4<f32>,
  v : array<vec3<f32>, 2>,
};

@group(0) @binding(0) var<storage, read_write> sb : SB;

@stage(compute) @workgroup_size(1)
fn main() {
  var a : i32 = sb.a;
  var b : u32 = sb.b;
  var c : f32 = sb.c;
  var d : vec2<i32> = sb.d;
  var e : vec2<u32> = sb.e;
  var f : vec2<f32> = sb.f;
  var g : vec3<i32> = sb.g;
  var h : vec3<u32> = sb.h;
  var i : vec3<f32> = sb.i;
  var j : vec4<i32> = sb.j;
  var k : vec4<u32> = sb.k;
  var l : vec4<f32> = sb.l;
  var m : mat2x2<f32> = sb.m;
  var n : mat2x3<f32> = sb.n;
  var o : mat2x4<f32> = sb.o;
  var p : mat3x2<f32> = sb.p;
  var q : mat3x3<f32> = sb.q;
  var r : mat3x4<f32> = sb.r;
  var s : mat4x2<f32> = sb.s;
  var t : mat4x3<f32> = sb.t;
  var u : mat4x4<f32> = sb.u;
  var v : array<vec3<f32>, 2> = sb.v;
}
)";

    auto* expect = R"(
struct SB {
  a : i32,
  b : u32,
  c : f32,
  d : vec2<i32>,
  e : vec2<u32>,
  f : vec2<f32>,
  g : vec3<i32>,
  h : vec3<u32>,
  i : vec3<f32>,
  j : vec4<i32>,
  k : vec4<u32>,
  l : vec4<f32>,
  m : mat2x2<f32>,
  n : mat2x3<f32>,
  o : mat2x4<f32>,
  p : mat3x2<f32>,
  q : mat3x3<f32>,
  r : mat3x4<f32>,
  s : mat4x2<f32>,
  t : mat4x3<f32>,
  u : mat4x4<f32>,
  v : array<vec3<f32>, 2>,
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

@internal(intrinsic_load_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> i32

@internal(intrinsic_load_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_1(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> u32

@internal(intrinsic_load_storage_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_2(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> f32

@internal(intrinsic_load_storage_vec2_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_3(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> vec2<i32>

@internal(intrinsic_load_storage_vec2_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_4(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> vec2<u32>

@internal(intrinsic_load_storage_vec2_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_5(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> vec2<f32>

@internal(intrinsic_load_storage_vec3_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_6(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> vec3<i32>

@internal(intrinsic_load_storage_vec3_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_7(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> vec3<u32>

@internal(intrinsic_load_storage_vec3_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_8(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> vec3<f32>

@internal(intrinsic_load_storage_vec4_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_9(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> vec4<i32>

@internal(intrinsic_load_storage_vec4_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_10(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> vec4<u32>

@internal(intrinsic_load_storage_vec4_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_11(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> vec4<f32>

fn tint_symbol_12(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> mat2x2<f32> {
  return mat2x2<f32>(tint_symbol_5(buffer, (offset + 0u)), tint_symbol_5(buffer, (offset + 8u)));
}

fn tint_symbol_13(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> mat2x3<f32> {
  return mat2x3<f32>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 16u)));
}

fn tint_symbol_14(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> mat2x4<f32> {
  return mat2x4<f32>(tint_symbol_11(buffer, (offset + 0u)), tint_symbol_11(buffer, (offset + 16u)));
}

fn tint_symbol_15(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> mat3x2<f32> {
  return mat3x2<f32>(tint_symbol_5(buffer, (offset + 0u)), tint_symbol_5(buffer, (offset + 8u)), tint_symbol_5(buffer, (offset + 16u)));
}

fn tint_symbol_16(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> mat3x3<f32> {
  return mat3x3<f32>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 16u)), tint_symbol_8(buffer, (offset + 32u)));
}

fn tint_symbol_17(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> mat3x4<f32> {
  return mat3x4<f32>(tint_symbol_11(buffer, (offset + 0u)), tint_symbol_11(buffer, (offset + 16u)), tint_symbol_11(buffer, (offset + 32u)));
}

fn tint_symbol_18(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> mat4x2<f32> {
  return mat4x2<f32>(tint_symbol_5(buffer, (offset + 0u)), tint_symbol_5(buffer, (offset + 8u)), tint_symbol_5(buffer, (offset + 16u)), tint_symbol_5(buffer, (offset + 24u)));
}

fn tint_symbol_19(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> mat4x3<f32> {
  return mat4x3<f32>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 16u)), tint_symbol_8(buffer, (offset + 32u)), tint_symbol_8(buffer, (offset + 48u)));
}

fn tint_symbol_20(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> mat4x4<f32> {
  return mat4x4<f32>(tint_symbol_11(buffer, (offset + 0u)), tint_symbol_11(buffer, (offset + 16u)), tint_symbol_11(buffer, (offset + 32u)), tint_symbol_11(buffer, (offset + 48u)));
}

fn tint_symbol_21(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> array<vec3<f32>, 2u> {
  var arr : array<vec3<f32>, 2u>;
  for(var i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
    arr[i_1] = tint_symbol_8(buffer, (offset + (i_1 * 16u)));
  }
  return arr;
}

@stage(compute) @workgroup_size(1)
fn main() {
  var a : i32 = tint_symbol(sb, 0u);
  var b : u32 = tint_symbol_1(sb, 4u);
  var c : f32 = tint_symbol_2(sb, 8u);
  var d : vec2<i32> = tint_symbol_3(sb, 16u);
  var e : vec2<u32> = tint_symbol_4(sb, 24u);
  var f : vec2<f32> = tint_symbol_5(sb, 32u);
  var g : vec3<i32> = tint_symbol_6(sb, 48u);
  var h : vec3<u32> = tint_symbol_7(sb, 64u);
  var i : vec3<f32> = tint_symbol_8(sb, 80u);
  var j : vec4<i32> = tint_symbol_9(sb, 96u);
  var k : vec4<u32> = tint_symbol_10(sb, 112u);
  var l : vec4<f32> = tint_symbol_11(sb, 128u);
  var m : mat2x2<f32> = tint_symbol_12(sb, 144u);
  var n : mat2x3<f32> = tint_symbol_13(sb, 160u);
  var o : mat2x4<f32> = tint_symbol_14(sb, 192u);
  var p : mat3x2<f32> = tint_symbol_15(sb, 224u);
  var q : mat3x3<f32> = tint_symbol_16(sb, 256u);
  var r : mat3x4<f32> = tint_symbol_17(sb, 304u);
  var s : mat4x2<f32> = tint_symbol_18(sb, 352u);
  var t : mat4x3<f32> = tint_symbol_19(sb, 384u);
  var u : mat4x4<f32> = tint_symbol_20(sb, 448u);
  var v : array<vec3<f32>, 2> = tint_symbol_21(sb, 512u);
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, SB_BasicLoad_OutOfOrder) {
    auto* src = R"(
@stage(compute) @workgroup_size(1)
fn main() {
  var a : i32 = sb.a;
  var b : u32 = sb.b;
  var c : f32 = sb.c;
  var d : vec2<i32> = sb.d;
  var e : vec2<u32> = sb.e;
  var f : vec2<f32> = sb.f;
  var g : vec3<i32> = sb.g;
  var h : vec3<u32> = sb.h;
  var i : vec3<f32> = sb.i;
  var j : vec4<i32> = sb.j;
  var k : vec4<u32> = sb.k;
  var l : vec4<f32> = sb.l;
  var m : mat2x2<f32> = sb.m;
  var n : mat2x3<f32> = sb.n;
  var o : mat2x4<f32> = sb.o;
  var p : mat3x2<f32> = sb.p;
  var q : mat3x3<f32> = sb.q;
  var r : mat3x4<f32> = sb.r;
  var s : mat4x2<f32> = sb.s;
  var t : mat4x3<f32> = sb.t;
  var u : mat4x4<f32> = sb.u;
  var v : array<vec3<f32>, 2> = sb.v;
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  a : i32,
  b : u32,
  c : f32,
  d : vec2<i32>,
  e : vec2<u32>,
  f : vec2<f32>,
  g : vec3<i32>,
  h : vec3<u32>,
  i : vec3<f32>,
  j : vec4<i32>,
  k : vec4<u32>,
  l : vec4<f32>,
  m : mat2x2<f32>,
  n : mat2x3<f32>,
  o : mat2x4<f32>,
  p : mat3x2<f32>,
  q : mat3x3<f32>,
  r : mat3x4<f32>,
  s : mat4x2<f32>,
  t : mat4x3<f32>,
  u : mat4x4<f32>,
  v : array<vec3<f32>, 2>,
};
)";

    auto* expect = R"(
@internal(intrinsic_load_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> i32

@internal(intrinsic_load_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_1(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> u32

@internal(intrinsic_load_storage_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_2(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> f32

@internal(intrinsic_load_storage_vec2_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_3(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> vec2<i32>

@internal(intrinsic_load_storage_vec2_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_4(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> vec2<u32>

@internal(intrinsic_load_storage_vec2_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_5(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> vec2<f32>

@internal(intrinsic_load_storage_vec3_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_6(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> vec3<i32>

@internal(intrinsic_load_storage_vec3_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_7(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> vec3<u32>

@internal(intrinsic_load_storage_vec3_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_8(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> vec3<f32>

@internal(intrinsic_load_storage_vec4_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_9(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> vec4<i32>

@internal(intrinsic_load_storage_vec4_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_10(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> vec4<u32>

@internal(intrinsic_load_storage_vec4_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_11(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> vec4<f32>

fn tint_symbol_12(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> mat2x2<f32> {
  return mat2x2<f32>(tint_symbol_5(buffer, (offset + 0u)), tint_symbol_5(buffer, (offset + 8u)));
}

fn tint_symbol_13(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> mat2x3<f32> {
  return mat2x3<f32>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 16u)));
}

fn tint_symbol_14(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> mat2x4<f32> {
  return mat2x4<f32>(tint_symbol_11(buffer, (offset + 0u)), tint_symbol_11(buffer, (offset + 16u)));
}

fn tint_symbol_15(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> mat3x2<f32> {
  return mat3x2<f32>(tint_symbol_5(buffer, (offset + 0u)), tint_symbol_5(buffer, (offset + 8u)), tint_symbol_5(buffer, (offset + 16u)));
}

fn tint_symbol_16(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> mat3x3<f32> {
  return mat3x3<f32>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 16u)), tint_symbol_8(buffer, (offset + 32u)));
}

fn tint_symbol_17(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> mat3x4<f32> {
  return mat3x4<f32>(tint_symbol_11(buffer, (offset + 0u)), tint_symbol_11(buffer, (offset + 16u)), tint_symbol_11(buffer, (offset + 32u)));
}

fn tint_symbol_18(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> mat4x2<f32> {
  return mat4x2<f32>(tint_symbol_5(buffer, (offset + 0u)), tint_symbol_5(buffer, (offset + 8u)), tint_symbol_5(buffer, (offset + 16u)), tint_symbol_5(buffer, (offset + 24u)));
}

fn tint_symbol_19(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> mat4x3<f32> {
  return mat4x3<f32>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 16u)), tint_symbol_8(buffer, (offset + 32u)), tint_symbol_8(buffer, (offset + 48u)));
}

fn tint_symbol_20(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> mat4x4<f32> {
  return mat4x4<f32>(tint_symbol_11(buffer, (offset + 0u)), tint_symbol_11(buffer, (offset + 16u)), tint_symbol_11(buffer, (offset + 32u)), tint_symbol_11(buffer, (offset + 48u)));
}

fn tint_symbol_21(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> array<vec3<f32>, 2u> {
  var arr : array<vec3<f32>, 2u>;
  for(var i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
    arr[i_1] = tint_symbol_8(buffer, (offset + (i_1 * 16u)));
  }
  return arr;
}

@stage(compute) @workgroup_size(1)
fn main() {
  var a : i32 = tint_symbol(sb, 0u);
  var b : u32 = tint_symbol_1(sb, 4u);
  var c : f32 = tint_symbol_2(sb, 8u);
  var d : vec2<i32> = tint_symbol_3(sb, 16u);
  var e : vec2<u32> = tint_symbol_4(sb, 24u);
  var f : vec2<f32> = tint_symbol_5(sb, 32u);
  var g : vec3<i32> = tint_symbol_6(sb, 48u);
  var h : vec3<u32> = tint_symbol_7(sb, 64u);
  var i : vec3<f32> = tint_symbol_8(sb, 80u);
  var j : vec4<i32> = tint_symbol_9(sb, 96u);
  var k : vec4<u32> = tint_symbol_10(sb, 112u);
  var l : vec4<f32> = tint_symbol_11(sb, 128u);
  var m : mat2x2<f32> = tint_symbol_12(sb, 144u);
  var n : mat2x3<f32> = tint_symbol_13(sb, 160u);
  var o : mat2x4<f32> = tint_symbol_14(sb, 192u);
  var p : mat3x2<f32> = tint_symbol_15(sb, 224u);
  var q : mat3x3<f32> = tint_symbol_16(sb, 256u);
  var r : mat3x4<f32> = tint_symbol_17(sb, 304u);
  var s : mat4x2<f32> = tint_symbol_18(sb, 352u);
  var t : mat4x3<f32> = tint_symbol_19(sb, 384u);
  var u : mat4x4<f32> = tint_symbol_20(sb, 448u);
  var v : array<vec3<f32>, 2> = tint_symbol_21(sb, 512u);
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  a : i32,
  b : u32,
  c : f32,
  d : vec2<i32>,
  e : vec2<u32>,
  f : vec2<f32>,
  g : vec3<i32>,
  h : vec3<u32>,
  i : vec3<f32>,
  j : vec4<i32>,
  k : vec4<u32>,
  l : vec4<f32>,
  m : mat2x2<f32>,
  n : mat2x3<f32>,
  o : mat2x4<f32>,
  p : mat3x2<f32>,
  q : mat3x3<f32>,
  r : mat3x4<f32>,
  s : mat4x2<f32>,
  t : mat4x3<f32>,
  u : mat4x4<f32>,
  v : array<vec3<f32>, 2>,
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, UB_BasicLoad) {
    auto* src = R"(
struct UB {
  a : i32,
  b : u32,
  c : f32,
  d : vec2<i32>,
  e : vec2<u32>,
  f : vec2<f32>,
  g : vec3<i32>,
  h : vec3<u32>,
  i : vec3<f32>,
  j : vec4<i32>,
  k : vec4<u32>,
  l : vec4<f32>,
  m : mat2x2<f32>,
  n : mat2x3<f32>,
  o : mat2x4<f32>,
  p : mat3x2<f32>,
  q : mat3x3<f32>,
  r : mat3x4<f32>,
  s : mat4x2<f32>,
  t : mat4x3<f32>,
  u : mat4x4<f32>,
  v : array<vec3<f32>, 2>,
};

@group(0) @binding(0) var<uniform> ub : UB;

@stage(compute) @workgroup_size(1)
fn main() {
  var a : i32 = ub.a;
  var b : u32 = ub.b;
  var c : f32 = ub.c;
  var d : vec2<i32> = ub.d;
  var e : vec2<u32> = ub.e;
  var f : vec2<f32> = ub.f;
  var g : vec3<i32> = ub.g;
  var h : vec3<u32> = ub.h;
  var i : vec3<f32> = ub.i;
  var j : vec4<i32> = ub.j;
  var k : vec4<u32> = ub.k;
  var l : vec4<f32> = ub.l;
  var m : mat2x2<f32> = ub.m;
  var n : mat2x3<f32> = ub.n;
  var o : mat2x4<f32> = ub.o;
  var p : mat3x2<f32> = ub.p;
  var q : mat3x3<f32> = ub.q;
  var r : mat3x4<f32> = ub.r;
  var s : mat4x2<f32> = ub.s;
  var t : mat4x3<f32> = ub.t;
  var u : mat4x4<f32> = ub.u;
  var v : array<vec3<f32>, 2> = ub.v;
}
)";

    auto* expect = R"(
struct UB {
  a : i32,
  b : u32,
  c : f32,
  d : vec2<i32>,
  e : vec2<u32>,
  f : vec2<f32>,
  g : vec3<i32>,
  h : vec3<u32>,
  i : vec3<f32>,
  j : vec4<i32>,
  k : vec4<u32>,
  l : vec4<f32>,
  m : mat2x2<f32>,
  n : mat2x3<f32>,
  o : mat2x4<f32>,
  p : mat3x2<f32>,
  q : mat3x3<f32>,
  r : mat3x4<f32>,
  s : mat4x2<f32>,
  t : mat4x3<f32>,
  u : mat4x4<f32>,
  v : array<vec3<f32>, 2>,
}

@group(0) @binding(0) var<uniform> ub : UB;

@internal(intrinsic_load_uniform_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> i32

@internal(intrinsic_load_uniform_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_1(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> u32

@internal(intrinsic_load_uniform_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_2(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> f32

@internal(intrinsic_load_uniform_vec2_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_3(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> vec2<i32>

@internal(intrinsic_load_uniform_vec2_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_4(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> vec2<u32>

@internal(intrinsic_load_uniform_vec2_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_5(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> vec2<f32>

@internal(intrinsic_load_uniform_vec3_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_6(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> vec3<i32>

@internal(intrinsic_load_uniform_vec3_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_7(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> vec3<u32>

@internal(intrinsic_load_uniform_vec3_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_8(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> vec3<f32>

@internal(intrinsic_load_uniform_vec4_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_9(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> vec4<i32>

@internal(intrinsic_load_uniform_vec4_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_10(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> vec4<u32>

@internal(intrinsic_load_uniform_vec4_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_11(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> vec4<f32>

fn tint_symbol_12(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> mat2x2<f32> {
  return mat2x2<f32>(tint_symbol_5(buffer, (offset + 0u)), tint_symbol_5(buffer, (offset + 8u)));
}

fn tint_symbol_13(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> mat2x3<f32> {
  return mat2x3<f32>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 16u)));
}

fn tint_symbol_14(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> mat2x4<f32> {
  return mat2x4<f32>(tint_symbol_11(buffer, (offset + 0u)), tint_symbol_11(buffer, (offset + 16u)));
}

fn tint_symbol_15(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> mat3x2<f32> {
  return mat3x2<f32>(tint_symbol_5(buffer, (offset + 0u)), tint_symbol_5(buffer, (offset + 8u)), tint_symbol_5(buffer, (offset + 16u)));
}

fn tint_symbol_16(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> mat3x3<f32> {
  return mat3x3<f32>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 16u)), tint_symbol_8(buffer, (offset + 32u)));
}

fn tint_symbol_17(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> mat3x4<f32> {
  return mat3x4<f32>(tint_symbol_11(buffer, (offset + 0u)), tint_symbol_11(buffer, (offset + 16u)), tint_symbol_11(buffer, (offset + 32u)));
}

fn tint_symbol_18(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> mat4x2<f32> {
  return mat4x2<f32>(tint_symbol_5(buffer, (offset + 0u)), tint_symbol_5(buffer, (offset + 8u)), tint_symbol_5(buffer, (offset + 16u)), tint_symbol_5(buffer, (offset + 24u)));
}

fn tint_symbol_19(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> mat4x3<f32> {
  return mat4x3<f32>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 16u)), tint_symbol_8(buffer, (offset + 32u)), tint_symbol_8(buffer, (offset + 48u)));
}

fn tint_symbol_20(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> mat4x4<f32> {
  return mat4x4<f32>(tint_symbol_11(buffer, (offset + 0u)), tint_symbol_11(buffer, (offset + 16u)), tint_symbol_11(buffer, (offset + 32u)), tint_symbol_11(buffer, (offset + 48u)));
}

fn tint_symbol_21(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> array<vec3<f32>, 2u> {
  var arr : array<vec3<f32>, 2u>;
  for(var i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
    arr[i_1] = tint_symbol_8(buffer, (offset + (i_1 * 16u)));
  }
  return arr;
}

@stage(compute) @workgroup_size(1)
fn main() {
  var a : i32 = tint_symbol(ub, 0u);
  var b : u32 = tint_symbol_1(ub, 4u);
  var c : f32 = tint_symbol_2(ub, 8u);
  var d : vec2<i32> = tint_symbol_3(ub, 16u);
  var e : vec2<u32> = tint_symbol_4(ub, 24u);
  var f : vec2<f32> = tint_symbol_5(ub, 32u);
  var g : vec3<i32> = tint_symbol_6(ub, 48u);
  var h : vec3<u32> = tint_symbol_7(ub, 64u);
  var i : vec3<f32> = tint_symbol_8(ub, 80u);
  var j : vec4<i32> = tint_symbol_9(ub, 96u);
  var k : vec4<u32> = tint_symbol_10(ub, 112u);
  var l : vec4<f32> = tint_symbol_11(ub, 128u);
  var m : mat2x2<f32> = tint_symbol_12(ub, 144u);
  var n : mat2x3<f32> = tint_symbol_13(ub, 160u);
  var o : mat2x4<f32> = tint_symbol_14(ub, 192u);
  var p : mat3x2<f32> = tint_symbol_15(ub, 224u);
  var q : mat3x3<f32> = tint_symbol_16(ub, 256u);
  var r : mat3x4<f32> = tint_symbol_17(ub, 304u);
  var s : mat4x2<f32> = tint_symbol_18(ub, 352u);
  var t : mat4x3<f32> = tint_symbol_19(ub, 384u);
  var u : mat4x4<f32> = tint_symbol_20(ub, 448u);
  var v : array<vec3<f32>, 2> = tint_symbol_21(ub, 512u);
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, UB_BasicLoad_OutOfOrder) {
    auto* src = R"(
@stage(compute) @workgroup_size(1)
fn main() {
  var a : i32 = ub.a;
  var b : u32 = ub.b;
  var c : f32 = ub.c;
  var d : vec2<i32> = ub.d;
  var e : vec2<u32> = ub.e;
  var f : vec2<f32> = ub.f;
  var g : vec3<i32> = ub.g;
  var h : vec3<u32> = ub.h;
  var i : vec3<f32> = ub.i;
  var j : vec4<i32> = ub.j;
  var k : vec4<u32> = ub.k;
  var l : vec4<f32> = ub.l;
  var m : mat2x2<f32> = ub.m;
  var n : mat2x3<f32> = ub.n;
  var o : mat2x4<f32> = ub.o;
  var p : mat3x2<f32> = ub.p;
  var q : mat3x3<f32> = ub.q;
  var r : mat3x4<f32> = ub.r;
  var s : mat4x2<f32> = ub.s;
  var t : mat4x3<f32> = ub.t;
  var u : mat4x4<f32> = ub.u;
  var v : array<vec3<f32>, 2> = ub.v;
}

@group(0) @binding(0) var<uniform> ub : UB;

struct UB {
  a : i32,
  b : u32,
  c : f32,
  d : vec2<i32>,
  e : vec2<u32>,
  f : vec2<f32>,
  g : vec3<i32>,
  h : vec3<u32>,
  i : vec3<f32>,
  j : vec4<i32>,
  k : vec4<u32>,
  l : vec4<f32>,
  m : mat2x2<f32>,
  n : mat2x3<f32>,
  o : mat2x4<f32>,
  p : mat3x2<f32>,
  q : mat3x3<f32>,
  r : mat3x4<f32>,
  s : mat4x2<f32>,
  t : mat4x3<f32>,
  u : mat4x4<f32>,
  v : array<vec3<f32>, 2>,
};
)";

    auto* expect = R"(
@internal(intrinsic_load_uniform_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> i32

@internal(intrinsic_load_uniform_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_1(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> u32

@internal(intrinsic_load_uniform_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_2(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> f32

@internal(intrinsic_load_uniform_vec2_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_3(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> vec2<i32>

@internal(intrinsic_load_uniform_vec2_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_4(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> vec2<u32>

@internal(intrinsic_load_uniform_vec2_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_5(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> vec2<f32>

@internal(intrinsic_load_uniform_vec3_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_6(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> vec3<i32>

@internal(intrinsic_load_uniform_vec3_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_7(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> vec3<u32>

@internal(intrinsic_load_uniform_vec3_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_8(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> vec3<f32>

@internal(intrinsic_load_uniform_vec4_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_9(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> vec4<i32>

@internal(intrinsic_load_uniform_vec4_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_10(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> vec4<u32>

@internal(intrinsic_load_uniform_vec4_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_11(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> vec4<f32>

fn tint_symbol_12(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> mat2x2<f32> {
  return mat2x2<f32>(tint_symbol_5(buffer, (offset + 0u)), tint_symbol_5(buffer, (offset + 8u)));
}

fn tint_symbol_13(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> mat2x3<f32> {
  return mat2x3<f32>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 16u)));
}

fn tint_symbol_14(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> mat2x4<f32> {
  return mat2x4<f32>(tint_symbol_11(buffer, (offset + 0u)), tint_symbol_11(buffer, (offset + 16u)));
}

fn tint_symbol_15(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> mat3x2<f32> {
  return mat3x2<f32>(tint_symbol_5(buffer, (offset + 0u)), tint_symbol_5(buffer, (offset + 8u)), tint_symbol_5(buffer, (offset + 16u)));
}

fn tint_symbol_16(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> mat3x3<f32> {
  return mat3x3<f32>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 16u)), tint_symbol_8(buffer, (offset + 32u)));
}

fn tint_symbol_17(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> mat3x4<f32> {
  return mat3x4<f32>(tint_symbol_11(buffer, (offset + 0u)), tint_symbol_11(buffer, (offset + 16u)), tint_symbol_11(buffer, (offset + 32u)));
}

fn tint_symbol_18(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> mat4x2<f32> {
  return mat4x2<f32>(tint_symbol_5(buffer, (offset + 0u)), tint_symbol_5(buffer, (offset + 8u)), tint_symbol_5(buffer, (offset + 16u)), tint_symbol_5(buffer, (offset + 24u)));
}

fn tint_symbol_19(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> mat4x3<f32> {
  return mat4x3<f32>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 16u)), tint_symbol_8(buffer, (offset + 32u)), tint_symbol_8(buffer, (offset + 48u)));
}

fn tint_symbol_20(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> mat4x4<f32> {
  return mat4x4<f32>(tint_symbol_11(buffer, (offset + 0u)), tint_symbol_11(buffer, (offset + 16u)), tint_symbol_11(buffer, (offset + 32u)), tint_symbol_11(buffer, (offset + 48u)));
}

fn tint_symbol_21(@internal(disable_validation__ignore_constructible_function_parameter) buffer : UB, offset : u32) -> array<vec3<f32>, 2u> {
  var arr : array<vec3<f32>, 2u>;
  for(var i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
    arr[i_1] = tint_symbol_8(buffer, (offset + (i_1 * 16u)));
  }
  return arr;
}

@stage(compute) @workgroup_size(1)
fn main() {
  var a : i32 = tint_symbol(ub, 0u);
  var b : u32 = tint_symbol_1(ub, 4u);
  var c : f32 = tint_symbol_2(ub, 8u);
  var d : vec2<i32> = tint_symbol_3(ub, 16u);
  var e : vec2<u32> = tint_symbol_4(ub, 24u);
  var f : vec2<f32> = tint_symbol_5(ub, 32u);
  var g : vec3<i32> = tint_symbol_6(ub, 48u);
  var h : vec3<u32> = tint_symbol_7(ub, 64u);
  var i : vec3<f32> = tint_symbol_8(ub, 80u);
  var j : vec4<i32> = tint_symbol_9(ub, 96u);
  var k : vec4<u32> = tint_symbol_10(ub, 112u);
  var l : vec4<f32> = tint_symbol_11(ub, 128u);
  var m : mat2x2<f32> = tint_symbol_12(ub, 144u);
  var n : mat2x3<f32> = tint_symbol_13(ub, 160u);
  var o : mat2x4<f32> = tint_symbol_14(ub, 192u);
  var p : mat3x2<f32> = tint_symbol_15(ub, 224u);
  var q : mat3x3<f32> = tint_symbol_16(ub, 256u);
  var r : mat3x4<f32> = tint_symbol_17(ub, 304u);
  var s : mat4x2<f32> = tint_symbol_18(ub, 352u);
  var t : mat4x3<f32> = tint_symbol_19(ub, 384u);
  var u : mat4x4<f32> = tint_symbol_20(ub, 448u);
  var v : array<vec3<f32>, 2> = tint_symbol_21(ub, 512u);
}

@group(0) @binding(0) var<uniform> ub : UB;

struct UB {
  a : i32,
  b : u32,
  c : f32,
  d : vec2<i32>,
  e : vec2<u32>,
  f : vec2<f32>,
  g : vec3<i32>,
  h : vec3<u32>,
  i : vec3<f32>,
  j : vec4<i32>,
  k : vec4<u32>,
  l : vec4<f32>,
  m : mat2x2<f32>,
  n : mat2x3<f32>,
  o : mat2x4<f32>,
  p : mat3x2<f32>,
  q : mat3x3<f32>,
  r : mat3x4<f32>,
  s : mat4x2<f32>,
  t : mat4x3<f32>,
  u : mat4x4<f32>,
  v : array<vec3<f32>, 2>,
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, SB_BasicStore) {
    auto* src = R"(
struct SB {
  a : i32,
  b : u32,
  c : f32,
  d : vec2<i32>,
  e : vec2<u32>,
  f : vec2<f32>,
  g : vec3<i32>,
  h : vec3<u32>,
  i : vec3<f32>,
  j : vec4<i32>,
  k : vec4<u32>,
  l : vec4<f32>,
  m : mat2x2<f32>,
  n : mat2x3<f32>,
  o : mat2x4<f32>,
  p : mat3x2<f32>,
  q : mat3x3<f32>,
  r : mat3x4<f32>,
  s : mat4x2<f32>,
  t : mat4x3<f32>,
  u : mat4x4<f32>,
  v : array<vec3<f32>, 2>,
};

@group(0) @binding(0) var<storage, read_write> sb : SB;

@stage(compute) @workgroup_size(1)
fn main() {
  sb.a = i32();
  sb.b = u32();
  sb.c = f32();
  sb.d = vec2<i32>();
  sb.e = vec2<u32>();
  sb.f = vec2<f32>();
  sb.g = vec3<i32>();
  sb.h = vec3<u32>();
  sb.i = vec3<f32>();
  sb.j = vec4<i32>();
  sb.k = vec4<u32>();
  sb.l = vec4<f32>();
  sb.m = mat2x2<f32>();
  sb.n = mat2x3<f32>();
  sb.o = mat2x4<f32>();
  sb.p = mat3x2<f32>();
  sb.q = mat3x3<f32>();
  sb.r = mat3x4<f32>();
  sb.s = mat4x2<f32>();
  sb.t = mat4x3<f32>();
  sb.u = mat4x4<f32>();
  sb.v = array<vec3<f32>, 2>();
}
)";

    auto* expect = R"(
struct SB {
  a : i32,
  b : u32,
  c : f32,
  d : vec2<i32>,
  e : vec2<u32>,
  f : vec2<f32>,
  g : vec3<i32>,
  h : vec3<u32>,
  i : vec3<f32>,
  j : vec4<i32>,
  k : vec4<u32>,
  l : vec4<f32>,
  m : mat2x2<f32>,
  n : mat2x3<f32>,
  o : mat2x4<f32>,
  p : mat3x2<f32>,
  q : mat3x3<f32>,
  r : mat3x4<f32>,
  s : mat4x2<f32>,
  t : mat4x3<f32>,
  u : mat4x4<f32>,
  v : array<vec3<f32>, 2>,
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

@internal(intrinsic_store_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : i32)

@internal(intrinsic_store_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_1(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : u32)

@internal(intrinsic_store_storage_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_2(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : f32)

@internal(intrinsic_store_storage_vec2_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_3(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : vec2<i32>)

@internal(intrinsic_store_storage_vec2_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_4(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : vec2<u32>)

@internal(intrinsic_store_storage_vec2_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_5(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : vec2<f32>)

@internal(intrinsic_store_storage_vec3_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_6(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : vec3<i32>)

@internal(intrinsic_store_storage_vec3_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_7(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : vec3<u32>)

@internal(intrinsic_store_storage_vec3_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_8(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : vec3<f32>)

@internal(intrinsic_store_storage_vec4_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_9(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : vec4<i32>)

@internal(intrinsic_store_storage_vec4_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_10(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : vec4<u32>)

@internal(intrinsic_store_storage_vec4_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_11(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : vec4<f32>)

fn tint_symbol_12(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : mat2x2<f32>) {
  tint_symbol_5(buffer, (offset + 0u), value[0u]);
  tint_symbol_5(buffer, (offset + 8u), value[1u]);
}

fn tint_symbol_13(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : mat2x3<f32>) {
  tint_symbol_8(buffer, (offset + 0u), value[0u]);
  tint_symbol_8(buffer, (offset + 16u), value[1u]);
}

fn tint_symbol_14(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : mat2x4<f32>) {
  tint_symbol_11(buffer, (offset + 0u), value[0u]);
  tint_symbol_11(buffer, (offset + 16u), value[1u]);
}

fn tint_symbol_15(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : mat3x2<f32>) {
  tint_symbol_5(buffer, (offset + 0u), value[0u]);
  tint_symbol_5(buffer, (offset + 8u), value[1u]);
  tint_symbol_5(buffer, (offset + 16u), value[2u]);
}

fn tint_symbol_16(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : mat3x3<f32>) {
  tint_symbol_8(buffer, (offset + 0u), value[0u]);
  tint_symbol_8(buffer, (offset + 16u), value[1u]);
  tint_symbol_8(buffer, (offset + 32u), value[2u]);
}

fn tint_symbol_17(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : mat3x4<f32>) {
  tint_symbol_11(buffer, (offset + 0u), value[0u]);
  tint_symbol_11(buffer, (offset + 16u), value[1u]);
  tint_symbol_11(buffer, (offset + 32u), value[2u]);
}

fn tint_symbol_18(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : mat4x2<f32>) {
  tint_symbol_5(buffer, (offset + 0u), value[0u]);
  tint_symbol_5(buffer, (offset + 8u), value[1u]);
  tint_symbol_5(buffer, (offset + 16u), value[2u]);
  tint_symbol_5(buffer, (offset + 24u), value[3u]);
}

fn tint_symbol_19(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : mat4x3<f32>) {
  tint_symbol_8(buffer, (offset + 0u), value[0u]);
  tint_symbol_8(buffer, (offset + 16u), value[1u]);
  tint_symbol_8(buffer, (offset + 32u), value[2u]);
  tint_symbol_8(buffer, (offset + 48u), value[3u]);
}

fn tint_symbol_20(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : mat4x4<f32>) {
  tint_symbol_11(buffer, (offset + 0u), value[0u]);
  tint_symbol_11(buffer, (offset + 16u), value[1u]);
  tint_symbol_11(buffer, (offset + 32u), value[2u]);
  tint_symbol_11(buffer, (offset + 48u), value[3u]);
}

fn tint_symbol_21(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : array<vec3<f32>, 2u>) {
  var array = value;
  for(var i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
    tint_symbol_8(buffer, (offset + (i_1 * 16u)), array[i_1]);
  }
}

@stage(compute) @workgroup_size(1)
fn main() {
  tint_symbol(sb, 0u, i32());
  tint_symbol_1(sb, 4u, u32());
  tint_symbol_2(sb, 8u, f32());
  tint_symbol_3(sb, 16u, vec2<i32>());
  tint_symbol_4(sb, 24u, vec2<u32>());
  tint_symbol_5(sb, 32u, vec2<f32>());
  tint_symbol_6(sb, 48u, vec3<i32>());
  tint_symbol_7(sb, 64u, vec3<u32>());
  tint_symbol_8(sb, 80u, vec3<f32>());
  tint_symbol_9(sb, 96u, vec4<i32>());
  tint_symbol_10(sb, 112u, vec4<u32>());
  tint_symbol_11(sb, 128u, vec4<f32>());
  tint_symbol_12(sb, 144u, mat2x2<f32>());
  tint_symbol_13(sb, 160u, mat2x3<f32>());
  tint_symbol_14(sb, 192u, mat2x4<f32>());
  tint_symbol_15(sb, 224u, mat3x2<f32>());
  tint_symbol_16(sb, 256u, mat3x3<f32>());
  tint_symbol_17(sb, 304u, mat3x4<f32>());
  tint_symbol_18(sb, 352u, mat4x2<f32>());
  tint_symbol_19(sb, 384u, mat4x3<f32>());
  tint_symbol_20(sb, 448u, mat4x4<f32>());
  tint_symbol_21(sb, 512u, array<vec3<f32>, 2>());
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, SB_BasicStore_OutOfOrder) {
    auto* src = R"(
@stage(compute) @workgroup_size(1)
fn main() {
  sb.a = i32();
  sb.b = u32();
  sb.c = f32();
  sb.d = vec2<i32>();
  sb.e = vec2<u32>();
  sb.f = vec2<f32>();
  sb.g = vec3<i32>();
  sb.h = vec3<u32>();
  sb.i = vec3<f32>();
  sb.j = vec4<i32>();
  sb.k = vec4<u32>();
  sb.l = vec4<f32>();
  sb.m = mat2x2<f32>();
  sb.n = mat2x3<f32>();
  sb.o = mat2x4<f32>();
  sb.p = mat3x2<f32>();
  sb.q = mat3x3<f32>();
  sb.r = mat3x4<f32>();
  sb.s = mat4x2<f32>();
  sb.t = mat4x3<f32>();
  sb.u = mat4x4<f32>();
  sb.v = array<vec3<f32>, 2>();
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  a : i32,
  b : u32,
  c : f32,
  d : vec2<i32>,
  e : vec2<u32>,
  f : vec2<f32>,
  g : vec3<i32>,
  h : vec3<u32>,
  i : vec3<f32>,
  j : vec4<i32>,
  k : vec4<u32>,
  l : vec4<f32>,
  m : mat2x2<f32>,
  n : mat2x3<f32>,
  o : mat2x4<f32>,
  p : mat3x2<f32>,
  q : mat3x3<f32>,
  r : mat3x4<f32>,
  s : mat4x2<f32>,
  t : mat4x3<f32>,
  u : mat4x4<f32>,
  v : array<vec3<f32>, 2>,
};
)";

    auto* expect = R"(
@internal(intrinsic_store_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : i32)

@internal(intrinsic_store_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_1(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : u32)

@internal(intrinsic_store_storage_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_2(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : f32)

@internal(intrinsic_store_storage_vec2_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_3(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : vec2<i32>)

@internal(intrinsic_store_storage_vec2_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_4(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : vec2<u32>)

@internal(intrinsic_store_storage_vec2_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_5(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : vec2<f32>)

@internal(intrinsic_store_storage_vec3_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_6(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : vec3<i32>)

@internal(intrinsic_store_storage_vec3_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_7(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : vec3<u32>)

@internal(intrinsic_store_storage_vec3_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_8(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : vec3<f32>)

@internal(intrinsic_store_storage_vec4_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_9(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : vec4<i32>)

@internal(intrinsic_store_storage_vec4_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_10(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : vec4<u32>)

@internal(intrinsic_store_storage_vec4_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_11(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : vec4<f32>)

fn tint_symbol_12(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : mat2x2<f32>) {
  tint_symbol_5(buffer, (offset + 0u), value[0u]);
  tint_symbol_5(buffer, (offset + 8u), value[1u]);
}

fn tint_symbol_13(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : mat2x3<f32>) {
  tint_symbol_8(buffer, (offset + 0u), value[0u]);
  tint_symbol_8(buffer, (offset + 16u), value[1u]);
}

fn tint_symbol_14(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : mat2x4<f32>) {
  tint_symbol_11(buffer, (offset + 0u), value[0u]);
  tint_symbol_11(buffer, (offset + 16u), value[1u]);
}

fn tint_symbol_15(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : mat3x2<f32>) {
  tint_symbol_5(buffer, (offset + 0u), value[0u]);
  tint_symbol_5(buffer, (offset + 8u), value[1u]);
  tint_symbol_5(buffer, (offset + 16u), value[2u]);
}

fn tint_symbol_16(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : mat3x3<f32>) {
  tint_symbol_8(buffer, (offset + 0u), value[0u]);
  tint_symbol_8(buffer, (offset + 16u), value[1u]);
  tint_symbol_8(buffer, (offset + 32u), value[2u]);
}

fn tint_symbol_17(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : mat3x4<f32>) {
  tint_symbol_11(buffer, (offset + 0u), value[0u]);
  tint_symbol_11(buffer, (offset + 16u), value[1u]);
  tint_symbol_11(buffer, (offset + 32u), value[2u]);
}

fn tint_symbol_18(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : mat4x2<f32>) {
  tint_symbol_5(buffer, (offset + 0u), value[0u]);
  tint_symbol_5(buffer, (offset + 8u), value[1u]);
  tint_symbol_5(buffer, (offset + 16u), value[2u]);
  tint_symbol_5(buffer, (offset + 24u), value[3u]);
}

fn tint_symbol_19(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : mat4x3<f32>) {
  tint_symbol_8(buffer, (offset + 0u), value[0u]);
  tint_symbol_8(buffer, (offset + 16u), value[1u]);
  tint_symbol_8(buffer, (offset + 32u), value[2u]);
  tint_symbol_8(buffer, (offset + 48u), value[3u]);
}

fn tint_symbol_20(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : mat4x4<f32>) {
  tint_symbol_11(buffer, (offset + 0u), value[0u]);
  tint_symbol_11(buffer, (offset + 16u), value[1u]);
  tint_symbol_11(buffer, (offset + 32u), value[2u]);
  tint_symbol_11(buffer, (offset + 48u), value[3u]);
}

fn tint_symbol_21(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : array<vec3<f32>, 2u>) {
  var array = value;
  for(var i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
    tint_symbol_8(buffer, (offset + (i_1 * 16u)), array[i_1]);
  }
}

@stage(compute) @workgroup_size(1)
fn main() {
  tint_symbol(sb, 0u, i32());
  tint_symbol_1(sb, 4u, u32());
  tint_symbol_2(sb, 8u, f32());
  tint_symbol_3(sb, 16u, vec2<i32>());
  tint_symbol_4(sb, 24u, vec2<u32>());
  tint_symbol_5(sb, 32u, vec2<f32>());
  tint_symbol_6(sb, 48u, vec3<i32>());
  tint_symbol_7(sb, 64u, vec3<u32>());
  tint_symbol_8(sb, 80u, vec3<f32>());
  tint_symbol_9(sb, 96u, vec4<i32>());
  tint_symbol_10(sb, 112u, vec4<u32>());
  tint_symbol_11(sb, 128u, vec4<f32>());
  tint_symbol_12(sb, 144u, mat2x2<f32>());
  tint_symbol_13(sb, 160u, mat2x3<f32>());
  tint_symbol_14(sb, 192u, mat2x4<f32>());
  tint_symbol_15(sb, 224u, mat3x2<f32>());
  tint_symbol_16(sb, 256u, mat3x3<f32>());
  tint_symbol_17(sb, 304u, mat3x4<f32>());
  tint_symbol_18(sb, 352u, mat4x2<f32>());
  tint_symbol_19(sb, 384u, mat4x3<f32>());
  tint_symbol_20(sb, 448u, mat4x4<f32>());
  tint_symbol_21(sb, 512u, array<vec3<f32>, 2>());
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  a : i32,
  b : u32,
  c : f32,
  d : vec2<i32>,
  e : vec2<u32>,
  f : vec2<f32>,
  g : vec3<i32>,
  h : vec3<u32>,
  i : vec3<f32>,
  j : vec4<i32>,
  k : vec4<u32>,
  l : vec4<f32>,
  m : mat2x2<f32>,
  n : mat2x3<f32>,
  o : mat2x4<f32>,
  p : mat3x2<f32>,
  q : mat3x3<f32>,
  r : mat3x4<f32>,
  s : mat4x2<f32>,
  t : mat4x3<f32>,
  u : mat4x4<f32>,
  v : array<vec3<f32>, 2>,
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, LoadStructure) {
    auto* src = R"(
struct SB {
  a : i32,
  b : u32,
  c : f32,
  d : vec2<i32>,
  e : vec2<u32>,
  f : vec2<f32>,
  g : vec3<i32>,
  h : vec3<u32>,
  i : vec3<f32>,
  j : vec4<i32>,
  k : vec4<u32>,
  l : vec4<f32>,
  m : mat2x2<f32>,
  n : mat2x3<f32>,
  o : mat2x4<f32>,
  p : mat3x2<f32>,
  q : mat3x3<f32>,
  r : mat3x4<f32>,
  s : mat4x2<f32>,
  t : mat4x3<f32>,
  u : mat4x4<f32>,
  v : array<vec3<f32>, 2>,
};

@group(0) @binding(0) var<storage, read_write> sb : SB;

@stage(compute) @workgroup_size(1)
fn main() {
  var x : SB = sb;
}
)";

    auto* expect = R"(
struct SB {
  a : i32,
  b : u32,
  c : f32,
  d : vec2<i32>,
  e : vec2<u32>,
  f : vec2<f32>,
  g : vec3<i32>,
  h : vec3<u32>,
  i : vec3<f32>,
  j : vec4<i32>,
  k : vec4<u32>,
  l : vec4<f32>,
  m : mat2x2<f32>,
  n : mat2x3<f32>,
  o : mat2x4<f32>,
  p : mat3x2<f32>,
  q : mat3x3<f32>,
  r : mat3x4<f32>,
  s : mat4x2<f32>,
  t : mat4x3<f32>,
  u : mat4x4<f32>,
  v : array<vec3<f32>, 2>,
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

@internal(intrinsic_load_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_1(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> i32

@internal(intrinsic_load_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_2(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> u32

@internal(intrinsic_load_storage_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_3(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> f32

@internal(intrinsic_load_storage_vec2_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_4(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> vec2<i32>

@internal(intrinsic_load_storage_vec2_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_5(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> vec2<u32>

@internal(intrinsic_load_storage_vec2_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_6(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> vec2<f32>

@internal(intrinsic_load_storage_vec3_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_7(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> vec3<i32>

@internal(intrinsic_load_storage_vec3_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_8(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> vec3<u32>

@internal(intrinsic_load_storage_vec3_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_9(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> vec3<f32>

@internal(intrinsic_load_storage_vec4_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_10(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> vec4<i32>

@internal(intrinsic_load_storage_vec4_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_11(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> vec4<u32>

@internal(intrinsic_load_storage_vec4_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_12(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> vec4<f32>

fn tint_symbol_13(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> mat2x2<f32> {
  return mat2x2<f32>(tint_symbol_6(buffer, (offset + 0u)), tint_symbol_6(buffer, (offset + 8u)));
}

fn tint_symbol_14(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> mat2x3<f32> {
  return mat2x3<f32>(tint_symbol_9(buffer, (offset + 0u)), tint_symbol_9(buffer, (offset + 16u)));
}

fn tint_symbol_15(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> mat2x4<f32> {
  return mat2x4<f32>(tint_symbol_12(buffer, (offset + 0u)), tint_symbol_12(buffer, (offset + 16u)));
}

fn tint_symbol_16(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> mat3x2<f32> {
  return mat3x2<f32>(tint_symbol_6(buffer, (offset + 0u)), tint_symbol_6(buffer, (offset + 8u)), tint_symbol_6(buffer, (offset + 16u)));
}

fn tint_symbol_17(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> mat3x3<f32> {
  return mat3x3<f32>(tint_symbol_9(buffer, (offset + 0u)), tint_symbol_9(buffer, (offset + 16u)), tint_symbol_9(buffer, (offset + 32u)));
}

fn tint_symbol_18(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> mat3x4<f32> {
  return mat3x4<f32>(tint_symbol_12(buffer, (offset + 0u)), tint_symbol_12(buffer, (offset + 16u)), tint_symbol_12(buffer, (offset + 32u)));
}

fn tint_symbol_19(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> mat4x2<f32> {
  return mat4x2<f32>(tint_symbol_6(buffer, (offset + 0u)), tint_symbol_6(buffer, (offset + 8u)), tint_symbol_6(buffer, (offset + 16u)), tint_symbol_6(buffer, (offset + 24u)));
}

fn tint_symbol_20(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> mat4x3<f32> {
  return mat4x3<f32>(tint_symbol_9(buffer, (offset + 0u)), tint_symbol_9(buffer, (offset + 16u)), tint_symbol_9(buffer, (offset + 32u)), tint_symbol_9(buffer, (offset + 48u)));
}

fn tint_symbol_21(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> mat4x4<f32> {
  return mat4x4<f32>(tint_symbol_12(buffer, (offset + 0u)), tint_symbol_12(buffer, (offset + 16u)), tint_symbol_12(buffer, (offset + 32u)), tint_symbol_12(buffer, (offset + 48u)));
}

fn tint_symbol_22(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> array<vec3<f32>, 2u> {
  var arr : array<vec3<f32>, 2u>;
  for(var i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
    arr[i_1] = tint_symbol_9(buffer, (offset + (i_1 * 16u)));
  }
  return arr;
}

fn tint_symbol(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> SB {
  return SB(tint_symbol_1(buffer, (offset + 0u)), tint_symbol_2(buffer, (offset + 4u)), tint_symbol_3(buffer, (offset + 8u)), tint_symbol_4(buffer, (offset + 16u)), tint_symbol_5(buffer, (offset + 24u)), tint_symbol_6(buffer, (offset + 32u)), tint_symbol_7(buffer, (offset + 48u)), tint_symbol_8(buffer, (offset + 64u)), tint_symbol_9(buffer, (offset + 80u)), tint_symbol_10(buffer, (offset + 96u)), tint_symbol_11(buffer, (offset + 112u)), tint_symbol_12(buffer, (offset + 128u)), tint_symbol_13(buffer, (offset + 144u)), tint_symbol_14(buffer, (offset + 160u)), tint_symbol_15(buffer, (offset + 192u)), tint_symbol_16(buffer, (offset + 224u)), tint_symbol_17(buffer, (offset + 256u)), tint_symbol_18(buffer, (offset + 304u)), tint_symbol_19(buffer, (offset + 352u)), tint_symbol_20(buffer, (offset + 384u)), tint_symbol_21(buffer, (offset + 448u)), tint_symbol_22(buffer, (offset + 512u)));
}

@stage(compute) @workgroup_size(1)
fn main() {
  var x : SB = tint_symbol(sb, 0u);
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, LoadStructure_OutOfOrder) {
    auto* src = R"(
@stage(compute) @workgroup_size(1)
fn main() {
  var x : SB = sb;
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  a : i32,
  b : u32,
  c : f32,
  d : vec2<i32>,
  e : vec2<u32>,
  f : vec2<f32>,
  g : vec3<i32>,
  h : vec3<u32>,
  i : vec3<f32>,
  j : vec4<i32>,
  k : vec4<u32>,
  l : vec4<f32>,
  m : mat2x2<f32>,
  n : mat2x3<f32>,
  o : mat2x4<f32>,
  p : mat3x2<f32>,
  q : mat3x3<f32>,
  r : mat3x4<f32>,
  s : mat4x2<f32>,
  t : mat4x3<f32>,
  u : mat4x4<f32>,
  v : array<vec3<f32>, 2>,
};
)";

    auto* expect = R"(
@internal(intrinsic_load_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_1(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> i32

@internal(intrinsic_load_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_2(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> u32

@internal(intrinsic_load_storage_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_3(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> f32

@internal(intrinsic_load_storage_vec2_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_4(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> vec2<i32>

@internal(intrinsic_load_storage_vec2_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_5(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> vec2<u32>

@internal(intrinsic_load_storage_vec2_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_6(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> vec2<f32>

@internal(intrinsic_load_storage_vec3_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_7(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> vec3<i32>

@internal(intrinsic_load_storage_vec3_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_8(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> vec3<u32>

@internal(intrinsic_load_storage_vec3_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_9(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> vec3<f32>

@internal(intrinsic_load_storage_vec4_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_10(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> vec4<i32>

@internal(intrinsic_load_storage_vec4_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_11(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> vec4<u32>

@internal(intrinsic_load_storage_vec4_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_12(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> vec4<f32>

fn tint_symbol_13(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> mat2x2<f32> {
  return mat2x2<f32>(tint_symbol_6(buffer, (offset + 0u)), tint_symbol_6(buffer, (offset + 8u)));
}

fn tint_symbol_14(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> mat2x3<f32> {
  return mat2x3<f32>(tint_symbol_9(buffer, (offset + 0u)), tint_symbol_9(buffer, (offset + 16u)));
}

fn tint_symbol_15(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> mat2x4<f32> {
  return mat2x4<f32>(tint_symbol_12(buffer, (offset + 0u)), tint_symbol_12(buffer, (offset + 16u)));
}

fn tint_symbol_16(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> mat3x2<f32> {
  return mat3x2<f32>(tint_symbol_6(buffer, (offset + 0u)), tint_symbol_6(buffer, (offset + 8u)), tint_symbol_6(buffer, (offset + 16u)));
}

fn tint_symbol_17(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> mat3x3<f32> {
  return mat3x3<f32>(tint_symbol_9(buffer, (offset + 0u)), tint_symbol_9(buffer, (offset + 16u)), tint_symbol_9(buffer, (offset + 32u)));
}

fn tint_symbol_18(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> mat3x4<f32> {
  return mat3x4<f32>(tint_symbol_12(buffer, (offset + 0u)), tint_symbol_12(buffer, (offset + 16u)), tint_symbol_12(buffer, (offset + 32u)));
}

fn tint_symbol_19(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> mat4x2<f32> {
  return mat4x2<f32>(tint_symbol_6(buffer, (offset + 0u)), tint_symbol_6(buffer, (offset + 8u)), tint_symbol_6(buffer, (offset + 16u)), tint_symbol_6(buffer, (offset + 24u)));
}

fn tint_symbol_20(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> mat4x3<f32> {
  return mat4x3<f32>(tint_symbol_9(buffer, (offset + 0u)), tint_symbol_9(buffer, (offset + 16u)), tint_symbol_9(buffer, (offset + 32u)), tint_symbol_9(buffer, (offset + 48u)));
}

fn tint_symbol_21(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> mat4x4<f32> {
  return mat4x4<f32>(tint_symbol_12(buffer, (offset + 0u)), tint_symbol_12(buffer, (offset + 16u)), tint_symbol_12(buffer, (offset + 32u)), tint_symbol_12(buffer, (offset + 48u)));
}

fn tint_symbol_22(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> array<vec3<f32>, 2u> {
  var arr : array<vec3<f32>, 2u>;
  for(var i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
    arr[i_1] = tint_symbol_9(buffer, (offset + (i_1 * 16u)));
  }
  return arr;
}

fn tint_symbol(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> SB {
  return SB(tint_symbol_1(buffer, (offset + 0u)), tint_symbol_2(buffer, (offset + 4u)), tint_symbol_3(buffer, (offset + 8u)), tint_symbol_4(buffer, (offset + 16u)), tint_symbol_5(buffer, (offset + 24u)), tint_symbol_6(buffer, (offset + 32u)), tint_symbol_7(buffer, (offset + 48u)), tint_symbol_8(buffer, (offset + 64u)), tint_symbol_9(buffer, (offset + 80u)), tint_symbol_10(buffer, (offset + 96u)), tint_symbol_11(buffer, (offset + 112u)), tint_symbol_12(buffer, (offset + 128u)), tint_symbol_13(buffer, (offset + 144u)), tint_symbol_14(buffer, (offset + 160u)), tint_symbol_15(buffer, (offset + 192u)), tint_symbol_16(buffer, (offset + 224u)), tint_symbol_17(buffer, (offset + 256u)), tint_symbol_18(buffer, (offset + 304u)), tint_symbol_19(buffer, (offset + 352u)), tint_symbol_20(buffer, (offset + 384u)), tint_symbol_21(buffer, (offset + 448u)), tint_symbol_22(buffer, (offset + 512u)));
}

@stage(compute) @workgroup_size(1)
fn main() {
  var x : SB = tint_symbol(sb, 0u);
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  a : i32,
  b : u32,
  c : f32,
  d : vec2<i32>,
  e : vec2<u32>,
  f : vec2<f32>,
  g : vec3<i32>,
  h : vec3<u32>,
  i : vec3<f32>,
  j : vec4<i32>,
  k : vec4<u32>,
  l : vec4<f32>,
  m : mat2x2<f32>,
  n : mat2x3<f32>,
  o : mat2x4<f32>,
  p : mat3x2<f32>,
  q : mat3x3<f32>,
  r : mat3x4<f32>,
  s : mat4x2<f32>,
  t : mat4x3<f32>,
  u : mat4x4<f32>,
  v : array<vec3<f32>, 2>,
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, StoreStructure) {
    auto* src = R"(
struct SB {
  a : i32,
  b : u32,
  c : f32,
  d : vec2<i32>,
  e : vec2<u32>,
  f : vec2<f32>,
  g : vec3<i32>,
  h : vec3<u32>,
  i : vec3<f32>,
  j : vec4<i32>,
  k : vec4<u32>,
  l : vec4<f32>,
  m : mat2x2<f32>,
  n : mat2x3<f32>,
  o : mat2x4<f32>,
  p : mat3x2<f32>,
  q : mat3x3<f32>,
  r : mat3x4<f32>,
  s : mat4x2<f32>,
  t : mat4x3<f32>,
  u : mat4x4<f32>,
  v : array<vec3<f32>, 2>,
};

@group(0) @binding(0) var<storage, read_write> sb : SB;

@stage(compute) @workgroup_size(1)
fn main() {
  sb = SB();
}
)";

    auto* expect = R"(
struct SB {
  a : i32,
  b : u32,
  c : f32,
  d : vec2<i32>,
  e : vec2<u32>,
  f : vec2<f32>,
  g : vec3<i32>,
  h : vec3<u32>,
  i : vec3<f32>,
  j : vec4<i32>,
  k : vec4<u32>,
  l : vec4<f32>,
  m : mat2x2<f32>,
  n : mat2x3<f32>,
  o : mat2x4<f32>,
  p : mat3x2<f32>,
  q : mat3x3<f32>,
  r : mat3x4<f32>,
  s : mat4x2<f32>,
  t : mat4x3<f32>,
  u : mat4x4<f32>,
  v : array<vec3<f32>, 2>,
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

@internal(intrinsic_store_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_1(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : i32)

@internal(intrinsic_store_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_2(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : u32)

@internal(intrinsic_store_storage_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_3(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : f32)

@internal(intrinsic_store_storage_vec2_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_4(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : vec2<i32>)

@internal(intrinsic_store_storage_vec2_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_5(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : vec2<u32>)

@internal(intrinsic_store_storage_vec2_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_6(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : vec2<f32>)

@internal(intrinsic_store_storage_vec3_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_7(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : vec3<i32>)

@internal(intrinsic_store_storage_vec3_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_8(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : vec3<u32>)

@internal(intrinsic_store_storage_vec3_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_9(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : vec3<f32>)

@internal(intrinsic_store_storage_vec4_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_10(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : vec4<i32>)

@internal(intrinsic_store_storage_vec4_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_11(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : vec4<u32>)

@internal(intrinsic_store_storage_vec4_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_12(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : vec4<f32>)

fn tint_symbol_13(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : mat2x2<f32>) {
  tint_symbol_6(buffer, (offset + 0u), value[0u]);
  tint_symbol_6(buffer, (offset + 8u), value[1u]);
}

fn tint_symbol_14(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : mat2x3<f32>) {
  tint_symbol_9(buffer, (offset + 0u), value[0u]);
  tint_symbol_9(buffer, (offset + 16u), value[1u]);
}

fn tint_symbol_15(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : mat2x4<f32>) {
  tint_symbol_12(buffer, (offset + 0u), value[0u]);
  tint_symbol_12(buffer, (offset + 16u), value[1u]);
}

fn tint_symbol_16(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : mat3x2<f32>) {
  tint_symbol_6(buffer, (offset + 0u), value[0u]);
  tint_symbol_6(buffer, (offset + 8u), value[1u]);
  tint_symbol_6(buffer, (offset + 16u), value[2u]);
}

fn tint_symbol_17(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : mat3x3<f32>) {
  tint_symbol_9(buffer, (offset + 0u), value[0u]);
  tint_symbol_9(buffer, (offset + 16u), value[1u]);
  tint_symbol_9(buffer, (offset + 32u), value[2u]);
}

fn tint_symbol_18(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : mat3x4<f32>) {
  tint_symbol_12(buffer, (offset + 0u), value[0u]);
  tint_symbol_12(buffer, (offset + 16u), value[1u]);
  tint_symbol_12(buffer, (offset + 32u), value[2u]);
}

fn tint_symbol_19(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : mat4x2<f32>) {
  tint_symbol_6(buffer, (offset + 0u), value[0u]);
  tint_symbol_6(buffer, (offset + 8u), value[1u]);
  tint_symbol_6(buffer, (offset + 16u), value[2u]);
  tint_symbol_6(buffer, (offset + 24u), value[3u]);
}

fn tint_symbol_20(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : mat4x3<f32>) {
  tint_symbol_9(buffer, (offset + 0u), value[0u]);
  tint_symbol_9(buffer, (offset + 16u), value[1u]);
  tint_symbol_9(buffer, (offset + 32u), value[2u]);
  tint_symbol_9(buffer, (offset + 48u), value[3u]);
}

fn tint_symbol_21(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : mat4x4<f32>) {
  tint_symbol_12(buffer, (offset + 0u), value[0u]);
  tint_symbol_12(buffer, (offset + 16u), value[1u]);
  tint_symbol_12(buffer, (offset + 32u), value[2u]);
  tint_symbol_12(buffer, (offset + 48u), value[3u]);
}

fn tint_symbol_22(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : array<vec3<f32>, 2u>) {
  var array = value;
  for(var i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
    tint_symbol_9(buffer, (offset + (i_1 * 16u)), array[i_1]);
  }
}

fn tint_symbol(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : SB) {
  tint_symbol_1(buffer, (offset + 0u), value.a);
  tint_symbol_2(buffer, (offset + 4u), value.b);
  tint_symbol_3(buffer, (offset + 8u), value.c);
  tint_symbol_4(buffer, (offset + 16u), value.d);
  tint_symbol_5(buffer, (offset + 24u), value.e);
  tint_symbol_6(buffer, (offset + 32u), value.f);
  tint_symbol_7(buffer, (offset + 48u), value.g);
  tint_symbol_8(buffer, (offset + 64u), value.h);
  tint_symbol_9(buffer, (offset + 80u), value.i);
  tint_symbol_10(buffer, (offset + 96u), value.j);
  tint_symbol_11(buffer, (offset + 112u), value.k);
  tint_symbol_12(buffer, (offset + 128u), value.l);
  tint_symbol_13(buffer, (offset + 144u), value.m);
  tint_symbol_14(buffer, (offset + 160u), value.n);
  tint_symbol_15(buffer, (offset + 192u), value.o);
  tint_symbol_16(buffer, (offset + 224u), value.p);
  tint_symbol_17(buffer, (offset + 256u), value.q);
  tint_symbol_18(buffer, (offset + 304u), value.r);
  tint_symbol_19(buffer, (offset + 352u), value.s);
  tint_symbol_20(buffer, (offset + 384u), value.t);
  tint_symbol_21(buffer, (offset + 448u), value.u);
  tint_symbol_22(buffer, (offset + 512u), value.v);
}

@stage(compute) @workgroup_size(1)
fn main() {
  tint_symbol(sb, 0u, SB());
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, StoreStructure_OutOfOrder) {
    auto* src = R"(
@stage(compute) @workgroup_size(1)
fn main() {
  sb = SB();
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  a : i32,
  b : u32,
  c : f32,
  d : vec2<i32>,
  e : vec2<u32>,
  f : vec2<f32>,
  g : vec3<i32>,
  h : vec3<u32>,
  i : vec3<f32>,
  j : vec4<i32>,
  k : vec4<u32>,
  l : vec4<f32>,
  m : mat2x2<f32>,
  n : mat2x3<f32>,
  o : mat2x4<f32>,
  p : mat3x2<f32>,
  q : mat3x3<f32>,
  r : mat3x4<f32>,
  s : mat4x2<f32>,
  t : mat4x3<f32>,
  u : mat4x4<f32>,
  v : array<vec3<f32>, 2>,
};
)";

    auto* expect = R"(
@internal(intrinsic_store_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_1(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : i32)

@internal(intrinsic_store_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_2(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : u32)

@internal(intrinsic_store_storage_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_3(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : f32)

@internal(intrinsic_store_storage_vec2_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_4(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : vec2<i32>)

@internal(intrinsic_store_storage_vec2_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_5(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : vec2<u32>)

@internal(intrinsic_store_storage_vec2_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_6(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : vec2<f32>)

@internal(intrinsic_store_storage_vec3_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_7(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : vec3<i32>)

@internal(intrinsic_store_storage_vec3_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_8(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : vec3<u32>)

@internal(intrinsic_store_storage_vec3_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_9(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : vec3<f32>)

@internal(intrinsic_store_storage_vec4_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_10(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : vec4<i32>)

@internal(intrinsic_store_storage_vec4_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_11(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : vec4<u32>)

@internal(intrinsic_store_storage_vec4_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_12(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : vec4<f32>)

fn tint_symbol_13(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : mat2x2<f32>) {
  tint_symbol_6(buffer, (offset + 0u), value[0u]);
  tint_symbol_6(buffer, (offset + 8u), value[1u]);
}

fn tint_symbol_14(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : mat2x3<f32>) {
  tint_symbol_9(buffer, (offset + 0u), value[0u]);
  tint_symbol_9(buffer, (offset + 16u), value[1u]);
}

fn tint_symbol_15(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : mat2x4<f32>) {
  tint_symbol_12(buffer, (offset + 0u), value[0u]);
  tint_symbol_12(buffer, (offset + 16u), value[1u]);
}

fn tint_symbol_16(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : mat3x2<f32>) {
  tint_symbol_6(buffer, (offset + 0u), value[0u]);
  tint_symbol_6(buffer, (offset + 8u), value[1u]);
  tint_symbol_6(buffer, (offset + 16u), value[2u]);
}

fn tint_symbol_17(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : mat3x3<f32>) {
  tint_symbol_9(buffer, (offset + 0u), value[0u]);
  tint_symbol_9(buffer, (offset + 16u), value[1u]);
  tint_symbol_9(buffer, (offset + 32u), value[2u]);
}

fn tint_symbol_18(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : mat3x4<f32>) {
  tint_symbol_12(buffer, (offset + 0u), value[0u]);
  tint_symbol_12(buffer, (offset + 16u), value[1u]);
  tint_symbol_12(buffer, (offset + 32u), value[2u]);
}

fn tint_symbol_19(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : mat4x2<f32>) {
  tint_symbol_6(buffer, (offset + 0u), value[0u]);
  tint_symbol_6(buffer, (offset + 8u), value[1u]);
  tint_symbol_6(buffer, (offset + 16u), value[2u]);
  tint_symbol_6(buffer, (offset + 24u), value[3u]);
}

fn tint_symbol_20(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : mat4x3<f32>) {
  tint_symbol_9(buffer, (offset + 0u), value[0u]);
  tint_symbol_9(buffer, (offset + 16u), value[1u]);
  tint_symbol_9(buffer, (offset + 32u), value[2u]);
  tint_symbol_9(buffer, (offset + 48u), value[3u]);
}

fn tint_symbol_21(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : mat4x4<f32>) {
  tint_symbol_12(buffer, (offset + 0u), value[0u]);
  tint_symbol_12(buffer, (offset + 16u), value[1u]);
  tint_symbol_12(buffer, (offset + 32u), value[2u]);
  tint_symbol_12(buffer, (offset + 48u), value[3u]);
}

fn tint_symbol_22(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : array<vec3<f32>, 2u>) {
  var array = value;
  for(var i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
    tint_symbol_9(buffer, (offset + (i_1 * 16u)), array[i_1]);
  }
}

fn tint_symbol(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, value : SB) {
  tint_symbol_1(buffer, (offset + 0u), value.a);
  tint_symbol_2(buffer, (offset + 4u), value.b);
  tint_symbol_3(buffer, (offset + 8u), value.c);
  tint_symbol_4(buffer, (offset + 16u), value.d);
  tint_symbol_5(buffer, (offset + 24u), value.e);
  tint_symbol_6(buffer, (offset + 32u), value.f);
  tint_symbol_7(buffer, (offset + 48u), value.g);
  tint_symbol_8(buffer, (offset + 64u), value.h);
  tint_symbol_9(buffer, (offset + 80u), value.i);
  tint_symbol_10(buffer, (offset + 96u), value.j);
  tint_symbol_11(buffer, (offset + 112u), value.k);
  tint_symbol_12(buffer, (offset + 128u), value.l);
  tint_symbol_13(buffer, (offset + 144u), value.m);
  tint_symbol_14(buffer, (offset + 160u), value.n);
  tint_symbol_15(buffer, (offset + 192u), value.o);
  tint_symbol_16(buffer, (offset + 224u), value.p);
  tint_symbol_17(buffer, (offset + 256u), value.q);
  tint_symbol_18(buffer, (offset + 304u), value.r);
  tint_symbol_19(buffer, (offset + 352u), value.s);
  tint_symbol_20(buffer, (offset + 384u), value.t);
  tint_symbol_21(buffer, (offset + 448u), value.u);
  tint_symbol_22(buffer, (offset + 512u), value.v);
}

@stage(compute) @workgroup_size(1)
fn main() {
  tint_symbol(sb, 0u, SB());
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  a : i32,
  b : u32,
  c : f32,
  d : vec2<i32>,
  e : vec2<u32>,
  f : vec2<f32>,
  g : vec3<i32>,
  h : vec3<u32>,
  i : vec3<f32>,
  j : vec4<i32>,
  k : vec4<u32>,
  l : vec4<f32>,
  m : mat2x2<f32>,
  n : mat2x3<f32>,
  o : mat2x4<f32>,
  p : mat3x2<f32>,
  q : mat3x3<f32>,
  r : mat3x4<f32>,
  s : mat4x2<f32>,
  t : mat4x3<f32>,
  u : mat4x4<f32>,
  v : array<vec3<f32>, 2>,
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, ComplexStaticAccessChain) {
    auto* src = R"(
// sizeof(S1) == 32
// alignof(S1) == 16
struct S1 {
  a : i32,
  b : vec3<f32>,
  c : i32,
};

// sizeof(S2) == 116
// alignof(S2) == 16
struct S2 {
  a : i32,
  b : array<S1, 3>,
  c : i32,
};

struct SB {
  @size(128)
  a : i32,
  b : array<S2>,
};

@group(0) @binding(0) var<storage, read_write> sb : SB;

@stage(compute) @workgroup_size(1)
fn main() {
  var x : f32 = sb.b[4].b[1].b.z;
}
)";

    // sb.b[4].b[1].b.z
    //    ^  ^ ^  ^ ^ ^
    //    |  | |  | | |
    //  128  | |688 | 712
    //       | |    |
    //     640 656  704

    auto* expect = R"(
struct S1 {
  a : i32,
  b : vec3<f32>,
  c : i32,
}

struct S2 {
  a : i32,
  b : array<S1, 3>,
  c : i32,
}

struct SB {
  @size(128)
  a : i32,
  b : array<S2>,
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

@internal(intrinsic_load_storage_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> f32

@stage(compute) @workgroup_size(1)
fn main() {
  var x : f32 = tint_symbol(sb, 712u);
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, ComplexStaticAccessChain_OutOfOrder) {
    auto* src = R"(
@stage(compute) @workgroup_size(1)
fn main() {
  var x : f32 = sb.b[4].b[1].b.z;
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  @size(128)
  a : i32,
  b : array<S2>,
};

struct S2 {
  a : i32,
  b : array<S1, 3>,
  c : i32,
};

struct S1 {
  a : i32,
  b : vec3<f32>,
  c : i32,
};
)";

    // sb.b[4].b[1].b.z
    //    ^  ^ ^  ^ ^ ^
    //    |  | |  | | |
    //  128  | |688 | 712
    //       | |    |
    //     640 656  704

    auto* expect = R"(
@internal(intrinsic_load_storage_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> f32

@stage(compute) @workgroup_size(1)
fn main() {
  var x : f32 = tint_symbol(sb, 712u);
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  @size(128)
  a : i32,
  b : array<S2>,
}

struct S2 {
  a : i32,
  b : array<S1, 3>,
  c : i32,
}

struct S1 {
  a : i32,
  b : vec3<f32>,
  c : i32,
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, ComplexDynamicAccessChain) {
    auto* src = R"(
struct S1 {
  a : i32,
  b : vec3<f32>,
  c : i32,
};

struct S2 {
  a : i32,
  b : array<S1, 3>,
  c : i32,
};

struct SB {
  @size(128)
  a : i32,
  b : array<S2>,
};

@group(0) @binding(0) var<storage, read_write> sb : SB;

@stage(compute) @workgroup_size(1)
fn main() {
  var i : i32 = 4;
  var j : u32 = 1u;
  var k : i32 = 2;
  var x : f32 = sb.b[i].b[j].b[k];
}
)";

    auto* expect = R"(
struct S1 {
  a : i32,
  b : vec3<f32>,
  c : i32,
}

struct S2 {
  a : i32,
  b : array<S1, 3>,
  c : i32,
}

struct SB {
  @size(128)
  a : i32,
  b : array<S2>,
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

@internal(intrinsic_load_storage_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> f32

@stage(compute) @workgroup_size(1)
fn main() {
  var i : i32 = 4;
  var j : u32 = 1u;
  var k : i32 = 2;
  var x : f32 = tint_symbol(sb, (((((128u + (128u * u32(i))) + 16u) + (32u * j)) + 16u) + (4u * u32(k))));
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, ComplexDynamicAccessChain_OutOfOrder) {
    auto* src = R"(
@stage(compute) @workgroup_size(1)
fn main() {
  var i : i32 = 4;
  var j : u32 = 1u;
  var k : i32 = 2;
  var x : f32 = sb.b[i].b[j].b[k];
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  @size(128)
  a : i32,
  b : array<S2>
};

struct S2 {
  a : i32,
  b : array<S1, 3>,
  c : i32,
};

struct S1 {
  a : i32,
  b : vec3<f32>,
  c : i32,
};
)";

    auto* expect = R"(
@internal(intrinsic_load_storage_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> f32

@stage(compute) @workgroup_size(1)
fn main() {
  var i : i32 = 4;
  var j : u32 = 1u;
  var k : i32 = 2;
  var x : f32 = tint_symbol(sb, (((((128u + (128u * u32(i))) + 16u) + (32u * j)) + 16u) + (4u * u32(k))));
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  @size(128)
  a : i32,
  b : array<S2>,
}

struct S2 {
  a : i32,
  b : array<S1, 3>,
  c : i32,
}

struct S1 {
  a : i32,
  b : vec3<f32>,
  c : i32,
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, ComplexDynamicAccessChainWithAliases) {
    auto* src = R"(
struct S1 {
  a : i32,
  b : vec3<f32>,
  c : i32,
};

type A1 = S1;

type A1_Array = array<S1, 3>;

struct S2 {
  a : i32,
  b : A1_Array,
  c : i32,
};

type A2 = S2;

type A2_Array = array<S2>;

struct SB {
  @size(128)
  a : i32,
  b : A2_Array,
};

@group(0) @binding(0) var<storage, read_write> sb : SB;

@stage(compute) @workgroup_size(1)
fn main() {
  var i : i32 = 4;
  var j : u32 = 1u;
  var k : i32 = 2;
  var x : f32 = sb.b[i].b[j].b[k];
}
)";

    auto* expect = R"(
struct S1 {
  a : i32,
  b : vec3<f32>,
  c : i32,
}

type A1 = S1;

type A1_Array = array<S1, 3>;

struct S2 {
  a : i32,
  b : A1_Array,
  c : i32,
}

type A2 = S2;

type A2_Array = array<S2>;

struct SB {
  @size(128)
  a : i32,
  b : A2_Array,
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

@internal(intrinsic_load_storage_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> f32

@stage(compute) @workgroup_size(1)
fn main() {
  var i : i32 = 4;
  var j : u32 = 1u;
  var k : i32 = 2;
  var x : f32 = tint_symbol(sb, (((((128u + (128u * u32(i))) + 16u) + (32u * j)) + 16u) + (4u * u32(k))));
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, ComplexDynamicAccessChainWithAliases_OutOfOrder) {
    auto* src = R"(
@stage(compute) @workgroup_size(1)
fn main() {
  var i : i32 = 4;
  var j : u32 = 1u;
  var k : i32 = 2;
  var x : f32 = sb.b[i].b[j].b[k];
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  @size(128)
  a : i32,
  b : A2_Array,
};

type A2_Array = array<S2>;

type A2 = S2;

struct S2 {
  a : i32,
  b : A1_Array,
  c : i32,
};

type A1 = S1;

type A1_Array = array<S1, 3>;

struct S1 {
  a : i32,
  b : vec3<f32>,
  c : i32,
};
)";

    auto* expect = R"(
@internal(intrinsic_load_storage_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> f32

@stage(compute) @workgroup_size(1)
fn main() {
  var i : i32 = 4;
  var j : u32 = 1u;
  var k : i32 = 2;
  var x : f32 = tint_symbol(sb, (((((128u + (128u * u32(i))) + 16u) + (32u * j)) + 16u) + (4u * u32(k))));
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  @size(128)
  a : i32,
  b : A2_Array,
}

type A2_Array = array<S2>;

type A2 = S2;

struct S2 {
  a : i32,
  b : A1_Array,
  c : i32,
}

type A1 = S1;

type A1_Array = array<S1, 3>;

struct S1 {
  a : i32,
  b : vec3<f32>,
  c : i32,
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, StorageBufferAtomics) {
    auto* src = R"(
struct SB {
  padding : vec4<f32>,
  a : atomic<i32>,
  b : atomic<u32>,
};

@group(0) @binding(0) var<storage, read_write> sb : SB;

@stage(compute) @workgroup_size(1)
fn main() {
  atomicStore(&sb.a, 123);
  atomicLoad(&sb.a);
  atomicAdd(&sb.a, 123);
  atomicSub(&sb.a, 123);
  atomicMax(&sb.a, 123);
  atomicMin(&sb.a, 123);
  atomicAnd(&sb.a, 123);
  atomicOr(&sb.a, 123);
  atomicXor(&sb.a, 123);
  atomicExchange(&sb.a, 123);
  atomicCompareExchangeWeak(&sb.a, 123, 345);

  atomicStore(&sb.b, 123u);
  atomicLoad(&sb.b);
  atomicAdd(&sb.b, 123u);
  atomicSub(&sb.b, 123u);
  atomicMax(&sb.b, 123u);
  atomicMin(&sb.b, 123u);
  atomicAnd(&sb.b, 123u);
  atomicOr(&sb.b, 123u);
  atomicXor(&sb.b, 123u);
  atomicExchange(&sb.b, 123u);
  atomicCompareExchangeWeak(&sb.b, 123u, 345u);
}
)";

    auto* expect = R"(
struct SB {
  padding : vec4<f32>,
  a : atomic<i32>,
  b : atomic<u32>,
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

@internal(intrinsic_atomic_store_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : i32)

@internal(intrinsic_atomic_load_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_1(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> i32

@internal(intrinsic_atomic_add_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_2(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_sub_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_3(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_max_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_4(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_min_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_5(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_and_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_6(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_or_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_7(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_xor_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_8(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_exchange_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_9(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_compare_exchange_weak_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_10(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : i32, param_2 : i32) -> vec2<i32>

@internal(intrinsic_atomic_store_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_11(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : u32)

@internal(intrinsic_atomic_load_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_12(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> u32

@internal(intrinsic_atomic_add_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_13(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_sub_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_14(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_max_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_15(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_min_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_16(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_and_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_17(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_or_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_18(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_xor_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_19(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_exchange_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_20(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_compare_exchange_weak_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_21(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : u32, param_2 : u32) -> vec2<u32>

@stage(compute) @workgroup_size(1)
fn main() {
  tint_symbol(sb, 16u, 123);
  tint_symbol_1(sb, 16u);
  tint_symbol_2(sb, 16u, 123);
  tint_symbol_3(sb, 16u, 123);
  tint_symbol_4(sb, 16u, 123);
  tint_symbol_5(sb, 16u, 123);
  tint_symbol_6(sb, 16u, 123);
  tint_symbol_7(sb, 16u, 123);
  tint_symbol_8(sb, 16u, 123);
  tint_symbol_9(sb, 16u, 123);
  tint_symbol_10(sb, 16u, 123, 345);
  tint_symbol_11(sb, 20u, 123u);
  tint_symbol_12(sb, 20u);
  tint_symbol_13(sb, 20u, 123u);
  tint_symbol_14(sb, 20u, 123u);
  tint_symbol_15(sb, 20u, 123u);
  tint_symbol_16(sb, 20u, 123u);
  tint_symbol_17(sb, 20u, 123u);
  tint_symbol_18(sb, 20u, 123u);
  tint_symbol_19(sb, 20u, 123u);
  tint_symbol_20(sb, 20u, 123u);
  tint_symbol_21(sb, 20u, 123u, 345u);
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, StorageBufferAtomics_OutOfOrder) {
    auto* src = R"(
@stage(compute) @workgroup_size(1)
fn main() {
  atomicStore(&sb.a, 123);
  atomicLoad(&sb.a);
  atomicAdd(&sb.a, 123);
  atomicSub(&sb.a, 123);
  atomicMax(&sb.a, 123);
  atomicMin(&sb.a, 123);
  atomicAnd(&sb.a, 123);
  atomicOr(&sb.a, 123);
  atomicXor(&sb.a, 123);
  atomicExchange(&sb.a, 123);
  atomicCompareExchangeWeak(&sb.a, 123, 345);

  atomicStore(&sb.b, 123u);
  atomicLoad(&sb.b);
  atomicAdd(&sb.b, 123u);
  atomicSub(&sb.b, 123u);
  atomicMax(&sb.b, 123u);
  atomicMin(&sb.b, 123u);
  atomicAnd(&sb.b, 123u);
  atomicOr(&sb.b, 123u);
  atomicXor(&sb.b, 123u);
  atomicExchange(&sb.b, 123u);
  atomicCompareExchangeWeak(&sb.b, 123u, 345u);
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  padding : vec4<f32>,
  a : atomic<i32>,
  b : atomic<u32>,
};
)";

    auto* expect = R"(
@internal(intrinsic_atomic_store_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : i32)

@internal(intrinsic_atomic_load_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_1(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> i32

@internal(intrinsic_atomic_add_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_2(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_sub_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_3(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_max_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_4(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_min_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_5(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_and_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_6(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_or_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_7(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_xor_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_8(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_exchange_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_9(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_compare_exchange_weak_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_10(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : i32, param_2 : i32) -> vec2<i32>

@internal(intrinsic_atomic_store_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_11(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : u32)

@internal(intrinsic_atomic_load_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_12(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32) -> u32

@internal(intrinsic_atomic_add_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_13(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_sub_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_14(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_max_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_15(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_min_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_16(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_and_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_17(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_or_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_18(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_xor_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_19(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_exchange_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_20(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_compare_exchange_weak_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_21(@internal(disable_validation__ignore_constructible_function_parameter) buffer : SB, offset : u32, param_1 : u32, param_2 : u32) -> vec2<u32>

@stage(compute) @workgroup_size(1)
fn main() {
  tint_symbol(sb, 16u, 123);
  tint_symbol_1(sb, 16u);
  tint_symbol_2(sb, 16u, 123);
  tint_symbol_3(sb, 16u, 123);
  tint_symbol_4(sb, 16u, 123);
  tint_symbol_5(sb, 16u, 123);
  tint_symbol_6(sb, 16u, 123);
  tint_symbol_7(sb, 16u, 123);
  tint_symbol_8(sb, 16u, 123);
  tint_symbol_9(sb, 16u, 123);
  tint_symbol_10(sb, 16u, 123, 345);
  tint_symbol_11(sb, 20u, 123u);
  tint_symbol_12(sb, 20u);
  tint_symbol_13(sb, 20u, 123u);
  tint_symbol_14(sb, 20u, 123u);
  tint_symbol_15(sb, 20u, 123u);
  tint_symbol_16(sb, 20u, 123u);
  tint_symbol_17(sb, 20u, 123u);
  tint_symbol_18(sb, 20u, 123u);
  tint_symbol_19(sb, 20u, 123u);
  tint_symbol_20(sb, 20u, 123u);
  tint_symbol_21(sb, 20u, 123u, 345u);
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  padding : vec4<f32>,
  a : atomic<i32>,
  b : atomic<u32>,
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, WorkgroupBufferAtomics) {
    auto* src = R"(
struct S {
  padding : vec4<f32>,
  a : atomic<i32>,
  b : atomic<u32>,
}

var<workgroup> w : S;

@stage(compute) @workgroup_size(1)
fn main() {
  atomicStore(&(w.a), 123);
  atomicLoad(&(w.a));
  atomicAdd(&(w.a), 123);
  atomicSub(&(w.a), 123);
  atomicMax(&(w.a), 123);
  atomicMin(&(w.a), 123);
  atomicAnd(&(w.a), 123);
  atomicOr(&(w.a), 123);
  atomicXor(&(w.a), 123);
  atomicExchange(&(w.a), 123);
  atomicCompareExchangeWeak(&(w.a), 123, 345);
  atomicStore(&(w.b), 123u);
  atomicLoad(&(w.b));
  atomicAdd(&(w.b), 123u);
  atomicSub(&(w.b), 123u);
  atomicMax(&(w.b), 123u);
  atomicMin(&(w.b), 123u);
  atomicAnd(&(w.b), 123u);
  atomicOr(&(w.b), 123u);
  atomicXor(&(w.b), 123u);
  atomicExchange(&(w.b), 123u);
  atomicCompareExchangeWeak(&(w.b), 123u, 345u);
}
)";

    auto* expect = src;

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, WorkgroupBufferAtomics_OutOfOrder) {
    auto* src = R"(
@stage(compute) @workgroup_size(1)
fn main() {
  atomicStore(&(w.a), 123);
  atomicLoad(&(w.a));
  atomicAdd(&(w.a), 123);
  atomicSub(&(w.a), 123);
  atomicMax(&(w.a), 123);
  atomicMin(&(w.a), 123);
  atomicAnd(&(w.a), 123);
  atomicOr(&(w.a), 123);
  atomicXor(&(w.a), 123);
  atomicExchange(&(w.a), 123);
  atomicCompareExchangeWeak(&(w.a), 123, 345);
  atomicStore(&(w.b), 123u);
  atomicLoad(&(w.b));
  atomicAdd(&(w.b), 123u);
  atomicSub(&(w.b), 123u);
  atomicMax(&(w.b), 123u);
  atomicMin(&(w.b), 123u);
  atomicAnd(&(w.b), 123u);
  atomicOr(&(w.b), 123u);
  atomicXor(&(w.b), 123u);
  atomicExchange(&(w.b), 123u);
  atomicCompareExchangeWeak(&(w.b), 123u, 345u);
}

var<workgroup> w : S;

struct S {
  padding : vec4<f32>,
  a : atomic<i32>,
  b : atomic<u32>,
}
)";

    auto* expect = src;

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::transform
