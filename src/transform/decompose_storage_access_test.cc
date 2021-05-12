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

#include "src/transform/decompose_storage_access.h"

#include "src/transform/test_helper.h"

namespace tint {
namespace transform {
namespace {

using DecomposeStorageAccessTest = TransformTest;

TEST_F(DecomposeStorageAccessTest, BasicLoad) {
  auto* src = R"(
[[block]]
struct SB {
  a : i32;
  b : u32;
  c : f32;
  d : vec2<i32>;
  e : vec2<u32>;
  f : vec2<f32>;
  g : vec3<i32>;
  h : vec3<u32>;
  i : vec3<f32>;
  j : vec4<i32>;
  k : vec4<u32>;
  l : vec4<f32>;
  m : mat2x2<f32>;
  n : mat2x3<f32>;
  o : mat2x4<f32>;
  p : mat3x2<f32>;
  q : mat3x3<f32>;
  r : mat3x4<f32>;
  s : mat4x2<f32>;
  t : mat4x3<f32>;
  u : mat4x4<f32>;
  v : array<vec3<f32>, 2>;
};

[[group(0), binding(0)]] var<storage> sb : [[access(read_write)]] SB;

[[stage(compute)]]
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
[[block]]
struct SB {
  a : i32;
  b : u32;
  c : f32;
  d : vec2<i32>;
  e : vec2<u32>;
  f : vec2<f32>;
  g : vec3<i32>;
  h : vec3<u32>;
  i : vec3<f32>;
  j : vec4<i32>;
  k : vec4<u32>;
  l : vec4<f32>;
  m : mat2x2<f32>;
  n : mat2x3<f32>;
  o : mat2x4<f32>;
  p : mat3x2<f32>;
  q : mat3x3<f32>;
  r : mat3x4<f32>;
  s : mat4x2<f32>;
  t : mat4x3<f32>;
  u : mat4x4<f32>;
  v : array<vec3<f32>, 2>;
};

[[internal(intrinsic_load_i32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol(buffer : [[access(read_write)]] SB, offset : u32) -> i32

[[internal(intrinsic_load_u32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_1(buffer : [[access(read_write)]] SB, offset : u32) -> u32

[[internal(intrinsic_load_f32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_2(buffer : [[access(read_write)]] SB, offset : u32) -> f32

[[internal(intrinsic_load_vec2_i32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_3(buffer : [[access(read_write)]] SB, offset : u32) -> vec2<i32>

[[internal(intrinsic_load_vec2_u32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_4(buffer : [[access(read_write)]] SB, offset : u32) -> vec2<u32>

[[internal(intrinsic_load_vec2_f32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_5(buffer : [[access(read_write)]] SB, offset : u32) -> vec2<f32>

[[internal(intrinsic_load_vec3_i32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_6(buffer : [[access(read_write)]] SB, offset : u32) -> vec3<i32>

[[internal(intrinsic_load_vec3_u32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_7(buffer : [[access(read_write)]] SB, offset : u32) -> vec3<u32>

[[internal(intrinsic_load_vec3_f32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_8(buffer : [[access(read_write)]] SB, offset : u32) -> vec3<f32>

[[internal(intrinsic_load_vec4_i32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_9(buffer : [[access(read_write)]] SB, offset : u32) -> vec4<i32>

[[internal(intrinsic_load_vec4_u32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_10(buffer : [[access(read_write)]] SB, offset : u32) -> vec4<u32>

[[internal(intrinsic_load_vec4_f32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_11(buffer : [[access(read_write)]] SB, offset : u32) -> vec4<f32>

fn tint_symbol_12(buffer : [[access(read_write)]] SB, offset : u32) -> mat2x2<f32> {
  return mat2x2<f32>(tint_symbol_5(buffer, (offset + 0u)), tint_symbol_5(buffer, (offset + 8u)));
}

fn tint_symbol_13(buffer : [[access(read_write)]] SB, offset : u32) -> mat2x3<f32> {
  return mat2x3<f32>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 16u)));
}

fn tint_symbol_14(buffer : [[access(read_write)]] SB, offset : u32) -> mat2x4<f32> {
  return mat2x4<f32>(tint_symbol_11(buffer, (offset + 0u)), tint_symbol_11(buffer, (offset + 16u)));
}

fn tint_symbol_15(buffer : [[access(read_write)]] SB, offset : u32) -> mat3x2<f32> {
  return mat3x2<f32>(tint_symbol_5(buffer, (offset + 0u)), tint_symbol_5(buffer, (offset + 8u)), tint_symbol_5(buffer, (offset + 16u)));
}

fn tint_symbol_16(buffer : [[access(read_write)]] SB, offset : u32) -> mat3x3<f32> {
  return mat3x3<f32>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 16u)), tint_symbol_8(buffer, (offset + 32u)));
}

fn tint_symbol_17(buffer : [[access(read_write)]] SB, offset : u32) -> mat3x4<f32> {
  return mat3x4<f32>(tint_symbol_11(buffer, (offset + 0u)), tint_symbol_11(buffer, (offset + 16u)), tint_symbol_11(buffer, (offset + 32u)));
}

fn tint_symbol_18(buffer : [[access(read_write)]] SB, offset : u32) -> mat4x2<f32> {
  return mat4x2<f32>(tint_symbol_5(buffer, (offset + 0u)), tint_symbol_5(buffer, (offset + 8u)), tint_symbol_5(buffer, (offset + 16u)), tint_symbol_5(buffer, (offset + 24u)));
}

fn tint_symbol_19(buffer : [[access(read_write)]] SB, offset : u32) -> mat4x3<f32> {
  return mat4x3<f32>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 16u)), tint_symbol_8(buffer, (offset + 32u)), tint_symbol_8(buffer, (offset + 48u)));
}

fn tint_symbol_20(buffer : [[access(read_write)]] SB, offset : u32) -> mat4x4<f32> {
  return mat4x4<f32>(tint_symbol_11(buffer, (offset + 0u)), tint_symbol_11(buffer, (offset + 16u)), tint_symbol_11(buffer, (offset + 32u)), tint_symbol_11(buffer, (offset + 48u)));
}

fn tint_symbol_21(buffer : [[access(read_write)]] SB, offset : u32) -> array<vec3<f32>, 2> {
  return array<vec3<f32>, 2>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 16u)));
}

[[group(0), binding(0)]] var<storage> sb : [[access(read_write)]] SB;

[[stage(compute)]]
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

  auto got = Run<DecomposeStorageAccess>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStorageAccessTest, BasicStore) {
  auto* src = R"(
[[block]]
struct SB {
  a : i32;
  b : u32;
  c : f32;
  d : vec2<i32>;
  e : vec2<u32>;
  f : vec2<f32>;
  g : vec3<i32>;
  h : vec3<u32>;
  i : vec3<f32>;
  j : vec4<i32>;
  k : vec4<u32>;
  l : vec4<f32>;
  m : mat2x2<f32>;
  n : mat2x3<f32>;
  o : mat2x4<f32>;
  p : mat3x2<f32>;
  q : mat3x3<f32>;
  r : mat3x4<f32>;
  s : mat4x2<f32>;
  t : mat4x3<f32>;
  u : mat4x4<f32>;
  v : array<vec3<f32>, 2>;
};

[[group(0), binding(0)]] var<storage> sb : [[access(read_write)]] SB;

[[stage(compute)]]
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
[[block]]
struct SB {
  a : i32;
  b : u32;
  c : f32;
  d : vec2<i32>;
  e : vec2<u32>;
  f : vec2<f32>;
  g : vec3<i32>;
  h : vec3<u32>;
  i : vec3<f32>;
  j : vec4<i32>;
  k : vec4<u32>;
  l : vec4<f32>;
  m : mat2x2<f32>;
  n : mat2x3<f32>;
  o : mat2x4<f32>;
  p : mat3x2<f32>;
  q : mat3x3<f32>;
  r : mat3x4<f32>;
  s : mat4x2<f32>;
  t : mat4x3<f32>;
  u : mat4x4<f32>;
  v : array<vec3<f32>, 2>;
};

[[internal(intrinsic_store_i32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol(buffer : [[access(read_write)]] SB, offset : u32, value : i32)

[[internal(intrinsic_store_u32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_1(buffer : [[access(read_write)]] SB, offset : u32, value : u32)

[[internal(intrinsic_store_f32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_2(buffer : [[access(read_write)]] SB, offset : u32, value : f32)

[[internal(intrinsic_store_vec2_u32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_3(buffer : [[access(read_write)]] SB, offset : u32, value : vec2<i32>)

[[internal(intrinsic_store_vec2_f32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_4(buffer : [[access(read_write)]] SB, offset : u32, value : vec2<u32>)

[[internal(intrinsic_store_vec2_i32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_5(buffer : [[access(read_write)]] SB, offset : u32, value : vec2<f32>)

[[internal(intrinsic_store_vec3_u32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_6(buffer : [[access(read_write)]] SB, offset : u32, value : vec3<i32>)

[[internal(intrinsic_store_vec3_f32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_7(buffer : [[access(read_write)]] SB, offset : u32, value : vec3<u32>)

[[internal(intrinsic_store_vec3_i32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_8(buffer : [[access(read_write)]] SB, offset : u32, value : vec3<f32>)

[[internal(intrinsic_store_vec4_u32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_9(buffer : [[access(read_write)]] SB, offset : u32, value : vec4<i32>)

[[internal(intrinsic_store_vec4_f32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_10(buffer : [[access(read_write)]] SB, offset : u32, value : vec4<u32>)

[[internal(intrinsic_store_vec4_i32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_11(buffer : [[access(read_write)]] SB, offset : u32, value : vec4<f32>)

fn tint_symbol_12(buffer : [[access(read_write)]] SB, offset : u32, value : mat2x2<f32>) {
  tint_symbol_5(buffer, (offset + 0u), value[0u]);
  tint_symbol_5(buffer, (offset + 8u), value[1u]);
}

fn tint_symbol_13(buffer : [[access(read_write)]] SB, offset : u32, value : mat2x3<f32>) {
  tint_symbol_8(buffer, (offset + 0u), value[0u]);
  tint_symbol_8(buffer, (offset + 16u), value[1u]);
}

fn tint_symbol_14(buffer : [[access(read_write)]] SB, offset : u32, value : mat2x4<f32>) {
  tint_symbol_11(buffer, (offset + 0u), value[0u]);
  tint_symbol_11(buffer, (offset + 16u), value[1u]);
}

fn tint_symbol_15(buffer : [[access(read_write)]] SB, offset : u32, value : mat3x2<f32>) {
  tint_symbol_5(buffer, (offset + 0u), value[0u]);
  tint_symbol_5(buffer, (offset + 8u), value[1u]);
  tint_symbol_5(buffer, (offset + 16u), value[2u]);
}

fn tint_symbol_16(buffer : [[access(read_write)]] SB, offset : u32, value : mat3x3<f32>) {
  tint_symbol_8(buffer, (offset + 0u), value[0u]);
  tint_symbol_8(buffer, (offset + 16u), value[1u]);
  tint_symbol_8(buffer, (offset + 32u), value[2u]);
}

fn tint_symbol_17(buffer : [[access(read_write)]] SB, offset : u32, value : mat3x4<f32>) {
  tint_symbol_11(buffer, (offset + 0u), value[0u]);
  tint_symbol_11(buffer, (offset + 16u), value[1u]);
  tint_symbol_11(buffer, (offset + 32u), value[2u]);
}

fn tint_symbol_18(buffer : [[access(read_write)]] SB, offset : u32, value : mat4x2<f32>) {
  tint_symbol_5(buffer, (offset + 0u), value[0u]);
  tint_symbol_5(buffer, (offset + 8u), value[1u]);
  tint_symbol_5(buffer, (offset + 16u), value[2u]);
  tint_symbol_5(buffer, (offset + 24u), value[3u]);
}

fn tint_symbol_19(buffer : [[access(read_write)]] SB, offset : u32, value : mat4x3<f32>) {
  tint_symbol_8(buffer, (offset + 0u), value[0u]);
  tint_symbol_8(buffer, (offset + 16u), value[1u]);
  tint_symbol_8(buffer, (offset + 32u), value[2u]);
  tint_symbol_8(buffer, (offset + 48u), value[3u]);
}

fn tint_symbol_20(buffer : [[access(read_write)]] SB, offset : u32, value : mat4x4<f32>) {
  tint_symbol_11(buffer, (offset + 0u), value[0u]);
  tint_symbol_11(buffer, (offset + 16u), value[1u]);
  tint_symbol_11(buffer, (offset + 32u), value[2u]);
  tint_symbol_11(buffer, (offset + 48u), value[3u]);
}

fn tint_symbol_21(buffer : [[access(read_write)]] SB, offset : u32, value : array<vec3<f32>, 2>) {
  tint_symbol_8(buffer, (offset + 0u), value[0u]);
  tint_symbol_8(buffer, (offset + 16u), value[1u]);
}

[[group(0), binding(0)]] var<storage> sb : [[access(read_write)]] SB;

[[stage(compute)]]
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

  auto got = Run<DecomposeStorageAccess>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStorageAccessTest, LoadStructure) {
  auto* src = R"(
[[block]]
struct SB {
  a : i32;
  b : u32;
  c : f32;
  d : vec2<i32>;
  e : vec2<u32>;
  f : vec2<f32>;
  g : vec3<i32>;
  h : vec3<u32>;
  i : vec3<f32>;
  j : vec4<i32>;
  k : vec4<u32>;
  l : vec4<f32>;
  m : mat2x2<f32>;
  n : mat2x3<f32>;
  o : mat2x4<f32>;
  p : mat3x2<f32>;
  q : mat3x3<f32>;
  r : mat3x4<f32>;
  s : mat4x2<f32>;
  t : mat4x3<f32>;
  u : mat4x4<f32>;
  v : array<vec3<f32>, 2>;
};

[[group(0), binding(0)]] var<storage> sb : [[access(read_write)]] SB;

[[stage(compute)]]
fn main() {
  var x : SB = sb;
}
)";

  auto* expect = R"(
[[block]]
struct SB {
  a : i32;
  b : u32;
  c : f32;
  d : vec2<i32>;
  e : vec2<u32>;
  f : vec2<f32>;
  g : vec3<i32>;
  h : vec3<u32>;
  i : vec3<f32>;
  j : vec4<i32>;
  k : vec4<u32>;
  l : vec4<f32>;
  m : mat2x2<f32>;
  n : mat2x3<f32>;
  o : mat2x4<f32>;
  p : mat3x2<f32>;
  q : mat3x3<f32>;
  r : mat3x4<f32>;
  s : mat4x2<f32>;
  t : mat4x3<f32>;
  u : mat4x4<f32>;
  v : array<vec3<f32>, 2>;
};

[[internal(intrinsic_load_i32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol(buffer : [[access(read_write)]] SB, offset : u32) -> i32

[[internal(intrinsic_load_u32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_1(buffer : [[access(read_write)]] SB, offset : u32) -> u32

[[internal(intrinsic_load_f32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_2(buffer : [[access(read_write)]] SB, offset : u32) -> f32

[[internal(intrinsic_load_vec2_i32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_3(buffer : [[access(read_write)]] SB, offset : u32) -> vec2<i32>

[[internal(intrinsic_load_vec2_u32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_4(buffer : [[access(read_write)]] SB, offset : u32) -> vec2<u32>

[[internal(intrinsic_load_vec2_f32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_5(buffer : [[access(read_write)]] SB, offset : u32) -> vec2<f32>

[[internal(intrinsic_load_vec3_i32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_6(buffer : [[access(read_write)]] SB, offset : u32) -> vec3<i32>

[[internal(intrinsic_load_vec3_u32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_7(buffer : [[access(read_write)]] SB, offset : u32) -> vec3<u32>

[[internal(intrinsic_load_vec3_f32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_8(buffer : [[access(read_write)]] SB, offset : u32) -> vec3<f32>

[[internal(intrinsic_load_vec4_i32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_9(buffer : [[access(read_write)]] SB, offset : u32) -> vec4<i32>

[[internal(intrinsic_load_vec4_u32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_10(buffer : [[access(read_write)]] SB, offset : u32) -> vec4<u32>

[[internal(intrinsic_load_vec4_f32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_11(buffer : [[access(read_write)]] SB, offset : u32) -> vec4<f32>

fn tint_symbol_12(buffer : [[access(read_write)]] SB, offset : u32) -> mat2x2<f32> {
  return mat2x2<f32>(tint_symbol_5(buffer, (offset + 0u)), tint_symbol_5(buffer, (offset + 8u)));
}

fn tint_symbol_13(buffer : [[access(read_write)]] SB, offset : u32) -> mat2x3<f32> {
  return mat2x3<f32>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 16u)));
}

fn tint_symbol_14(buffer : [[access(read_write)]] SB, offset : u32) -> mat2x4<f32> {
  return mat2x4<f32>(tint_symbol_11(buffer, (offset + 0u)), tint_symbol_11(buffer, (offset + 16u)));
}

fn tint_symbol_15(buffer : [[access(read_write)]] SB, offset : u32) -> mat3x2<f32> {
  return mat3x2<f32>(tint_symbol_5(buffer, (offset + 0u)), tint_symbol_5(buffer, (offset + 8u)), tint_symbol_5(buffer, (offset + 16u)));
}

fn tint_symbol_16(buffer : [[access(read_write)]] SB, offset : u32) -> mat3x3<f32> {
  return mat3x3<f32>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 16u)), tint_symbol_8(buffer, (offset + 32u)));
}

fn tint_symbol_17(buffer : [[access(read_write)]] SB, offset : u32) -> mat3x4<f32> {
  return mat3x4<f32>(tint_symbol_11(buffer, (offset + 0u)), tint_symbol_11(buffer, (offset + 16u)), tint_symbol_11(buffer, (offset + 32u)));
}

fn tint_symbol_18(buffer : [[access(read_write)]] SB, offset : u32) -> mat4x2<f32> {
  return mat4x2<f32>(tint_symbol_5(buffer, (offset + 0u)), tint_symbol_5(buffer, (offset + 8u)), tint_symbol_5(buffer, (offset + 16u)), tint_symbol_5(buffer, (offset + 24u)));
}

fn tint_symbol_19(buffer : [[access(read_write)]] SB, offset : u32) -> mat4x3<f32> {
  return mat4x3<f32>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 16u)), tint_symbol_8(buffer, (offset + 32u)), tint_symbol_8(buffer, (offset + 48u)));
}

fn tint_symbol_20(buffer : [[access(read_write)]] SB, offset : u32) -> mat4x4<f32> {
  return mat4x4<f32>(tint_symbol_11(buffer, (offset + 0u)), tint_symbol_11(buffer, (offset + 16u)), tint_symbol_11(buffer, (offset + 32u)), tint_symbol_11(buffer, (offset + 48u)));
}

fn tint_symbol_21(buffer : [[access(read_write)]] SB, offset : u32) -> array<vec3<f32>, 2> {
  return array<vec3<f32>, 2>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 16u)));
}

fn tint_symbol_22(buffer : [[access(read_write)]] SB, offset : u32) -> SB {
  return SB(tint_symbol(buffer, (offset + 0u)), tint_symbol_1(buffer, (offset + 4u)), tint_symbol_2(buffer, (offset + 8u)), tint_symbol_3(buffer, (offset + 16u)), tint_symbol_4(buffer, (offset + 24u)), tint_symbol_5(buffer, (offset + 32u)), tint_symbol_6(buffer, (offset + 48u)), tint_symbol_7(buffer, (offset + 64u)), tint_symbol_8(buffer, (offset + 80u)), tint_symbol_9(buffer, (offset + 96u)), tint_symbol_10(buffer, (offset + 112u)), tint_symbol_11(buffer, (offset + 128u)), tint_symbol_12(buffer, (offset + 144u)), tint_symbol_13(buffer, (offset + 160u)), tint_symbol_14(buffer, (offset + 192u)), tint_symbol_15(buffer, (offset + 224u)), tint_symbol_16(buffer, (offset + 256u)), tint_symbol_17(buffer, (offset + 304u)), tint_symbol_18(buffer, (offset + 352u)), tint_symbol_19(buffer, (offset + 384u)), tint_symbol_20(buffer, (offset + 448u)), tint_symbol_21(buffer, (offset + 512u)));
}

[[group(0), binding(0)]] var<storage> sb : [[access(read_write)]] SB;

[[stage(compute)]]
fn main() {
  var x : SB = tint_symbol_22(sb, 0u);
}
)";

  auto got = Run<DecomposeStorageAccess>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStorageAccessTest, StoreStructure) {
  auto* src = R"(
[[block]]
struct SB {
  a : i32;
  b : u32;
  c : f32;
  d : vec2<i32>;
  e : vec2<u32>;
  f : vec2<f32>;
  g : vec3<i32>;
  h : vec3<u32>;
  i : vec3<f32>;
  j : vec4<i32>;
  k : vec4<u32>;
  l : vec4<f32>;
  m : mat2x2<f32>;
  n : mat2x3<f32>;
  o : mat2x4<f32>;
  p : mat3x2<f32>;
  q : mat3x3<f32>;
  r : mat3x4<f32>;
  s : mat4x2<f32>;
  t : mat4x3<f32>;
  u : mat4x4<f32>;
  v : array<vec3<f32>, 2>;
};

[[group(0), binding(0)]] var<storage> sb : [[access(read_write)]] SB;

[[stage(compute)]]
fn main() {
  sb = SB();
}
)";

  auto* expect = R"(
[[block]]
struct SB {
  a : i32;
  b : u32;
  c : f32;
  d : vec2<i32>;
  e : vec2<u32>;
  f : vec2<f32>;
  g : vec3<i32>;
  h : vec3<u32>;
  i : vec3<f32>;
  j : vec4<i32>;
  k : vec4<u32>;
  l : vec4<f32>;
  m : mat2x2<f32>;
  n : mat2x3<f32>;
  o : mat2x4<f32>;
  p : mat3x2<f32>;
  q : mat3x3<f32>;
  r : mat3x4<f32>;
  s : mat4x2<f32>;
  t : mat4x3<f32>;
  u : mat4x4<f32>;
  v : array<vec3<f32>, 2>;
};

[[internal(intrinsic_store_i32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol(buffer : [[access(read_write)]] SB, offset : u32, value : i32)

[[internal(intrinsic_store_u32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_1(buffer : [[access(read_write)]] SB, offset : u32, value : u32)

[[internal(intrinsic_store_f32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_2(buffer : [[access(read_write)]] SB, offset : u32, value : f32)

[[internal(intrinsic_store_vec2_u32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_3(buffer : [[access(read_write)]] SB, offset : u32, value : vec2<i32>)

[[internal(intrinsic_store_vec2_f32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_4(buffer : [[access(read_write)]] SB, offset : u32, value : vec2<u32>)

[[internal(intrinsic_store_vec2_i32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_5(buffer : [[access(read_write)]] SB, offset : u32, value : vec2<f32>)

[[internal(intrinsic_store_vec3_u32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_6(buffer : [[access(read_write)]] SB, offset : u32, value : vec3<i32>)

[[internal(intrinsic_store_vec3_f32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_7(buffer : [[access(read_write)]] SB, offset : u32, value : vec3<u32>)

[[internal(intrinsic_store_vec3_i32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_8(buffer : [[access(read_write)]] SB, offset : u32, value : vec3<f32>)

[[internal(intrinsic_store_vec4_u32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_9(buffer : [[access(read_write)]] SB, offset : u32, value : vec4<i32>)

[[internal(intrinsic_store_vec4_f32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_10(buffer : [[access(read_write)]] SB, offset : u32, value : vec4<u32>)

[[internal(intrinsic_store_vec4_i32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol_11(buffer : [[access(read_write)]] SB, offset : u32, value : vec4<f32>)

fn tint_symbol_12(buffer : [[access(read_write)]] SB, offset : u32, value : mat2x2<f32>) {
  tint_symbol_5(buffer, (offset + 0u), value[0u]);
  tint_symbol_5(buffer, (offset + 8u), value[1u]);
}

fn tint_symbol_13(buffer : [[access(read_write)]] SB, offset : u32, value : mat2x3<f32>) {
  tint_symbol_8(buffer, (offset + 0u), value[0u]);
  tint_symbol_8(buffer, (offset + 16u), value[1u]);
}

fn tint_symbol_14(buffer : [[access(read_write)]] SB, offset : u32, value : mat2x4<f32>) {
  tint_symbol_11(buffer, (offset + 0u), value[0u]);
  tint_symbol_11(buffer, (offset + 16u), value[1u]);
}

fn tint_symbol_15(buffer : [[access(read_write)]] SB, offset : u32, value : mat3x2<f32>) {
  tint_symbol_5(buffer, (offset + 0u), value[0u]);
  tint_symbol_5(buffer, (offset + 8u), value[1u]);
  tint_symbol_5(buffer, (offset + 16u), value[2u]);
}

fn tint_symbol_16(buffer : [[access(read_write)]] SB, offset : u32, value : mat3x3<f32>) {
  tint_symbol_8(buffer, (offset + 0u), value[0u]);
  tint_symbol_8(buffer, (offset + 16u), value[1u]);
  tint_symbol_8(buffer, (offset + 32u), value[2u]);
}

fn tint_symbol_17(buffer : [[access(read_write)]] SB, offset : u32, value : mat3x4<f32>) {
  tint_symbol_11(buffer, (offset + 0u), value[0u]);
  tint_symbol_11(buffer, (offset + 16u), value[1u]);
  tint_symbol_11(buffer, (offset + 32u), value[2u]);
}

fn tint_symbol_18(buffer : [[access(read_write)]] SB, offset : u32, value : mat4x2<f32>) {
  tint_symbol_5(buffer, (offset + 0u), value[0u]);
  tint_symbol_5(buffer, (offset + 8u), value[1u]);
  tint_symbol_5(buffer, (offset + 16u), value[2u]);
  tint_symbol_5(buffer, (offset + 24u), value[3u]);
}

fn tint_symbol_19(buffer : [[access(read_write)]] SB, offset : u32, value : mat4x3<f32>) {
  tint_symbol_8(buffer, (offset + 0u), value[0u]);
  tint_symbol_8(buffer, (offset + 16u), value[1u]);
  tint_symbol_8(buffer, (offset + 32u), value[2u]);
  tint_symbol_8(buffer, (offset + 48u), value[3u]);
}

fn tint_symbol_20(buffer : [[access(read_write)]] SB, offset : u32, value : mat4x4<f32>) {
  tint_symbol_11(buffer, (offset + 0u), value[0u]);
  tint_symbol_11(buffer, (offset + 16u), value[1u]);
  tint_symbol_11(buffer, (offset + 32u), value[2u]);
  tint_symbol_11(buffer, (offset + 48u), value[3u]);
}

fn tint_symbol_21(buffer : [[access(read_write)]] SB, offset : u32, value : array<vec3<f32>, 2>) {
  tint_symbol_8(buffer, (offset + 0u), value[0u]);
  tint_symbol_8(buffer, (offset + 16u), value[1u]);
}

fn tint_symbol_22(buffer : [[access(read_write)]] SB, offset : u32, value : SB) {
  tint_symbol(buffer, (offset + 0u), value.a);
  tint_symbol_1(buffer, (offset + 4u), value.b);
  tint_symbol_2(buffer, (offset + 8u), value.c);
  tint_symbol_3(buffer, (offset + 16u), value.d);
  tint_symbol_4(buffer, (offset + 24u), value.e);
  tint_symbol_5(buffer, (offset + 32u), value.f);
  tint_symbol_6(buffer, (offset + 48u), value.g);
  tint_symbol_7(buffer, (offset + 64u), value.h);
  tint_symbol_8(buffer, (offset + 80u), value.i);
  tint_symbol_9(buffer, (offset + 96u), value.j);
  tint_symbol_10(buffer, (offset + 112u), value.k);
  tint_symbol_11(buffer, (offset + 128u), value.l);
  tint_symbol_12(buffer, (offset + 144u), value.m);
  tint_symbol_13(buffer, (offset + 160u), value.n);
  tint_symbol_14(buffer, (offset + 192u), value.o);
  tint_symbol_15(buffer, (offset + 224u), value.p);
  tint_symbol_16(buffer, (offset + 256u), value.q);
  tint_symbol_17(buffer, (offset + 304u), value.r);
  tint_symbol_18(buffer, (offset + 352u), value.s);
  tint_symbol_19(buffer, (offset + 384u), value.t);
  tint_symbol_20(buffer, (offset + 448u), value.u);
  tint_symbol_21(buffer, (offset + 512u), value.v);
}

[[group(0), binding(0)]] var<storage> sb : [[access(read_write)]] SB;

[[stage(compute)]]
fn main() {
  tint_symbol_22(sb, 0u, SB());
}
)";

  auto got = Run<DecomposeStorageAccess>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStorageAccessTest, ComplexStaticAccessChain) {
  auto* src = R"(
struct S1 {
  a : i32;
  b : vec3<f32>;
  c : i32;
};

struct S2 {
  a : i32;
  b : [[stride(32)]] array<S1, 3>;
  c : i32;
};

[[block]]
struct SB {
  [[size(128)]]
  a : i32;
  b : [[stride(256)]] array<S2>;
};

[[group(0), binding(0)]] var<storage> sb : [[access(read_write)]] SB;

[[stage(compute)]]
fn main() {
  var x : f32 = sb.b[4].b[1].b.z;
}
)";

  // sb.b[4].b[1].b.z
  //    ^  ^ ^  ^ ^ ^
  //    |  | |  | | |
  //  128  | |1200| 1224
  //       | |    |
  //    1152 1168 1216

  auto* expect = R"(
struct S1 {
  a : i32;
  b : vec3<f32>;
  c : i32;
};

struct S2 {
  a : i32;
  b : [[stride(32)]] array<S1, 3>;
  c : i32;
};

[[block]]
struct SB {
  [[size(128)]]
  a : i32;
  b : [[stride(256)]] array<S2>;
};

[[internal(intrinsic_load_f32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol(buffer : [[access(read_write)]] SB, offset : u32) -> f32

[[group(0), binding(0)]] var<storage> sb : [[access(read_write)]] SB;

[[stage(compute)]]
fn main() {
  var x : f32 = tint_symbol(sb, 1224u);
}
)";

  auto got = Run<DecomposeStorageAccess>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStorageAccessTest, ComplexDynamicAccessChain) {
  auto* src = R"(
struct S1 {
  a : i32;
  b : vec3<f32>;
  c : i32;
};

struct S2 {
  a : i32;
  b : [[stride(32)]] array<S1, 3>;
  c : i32;
};

[[block]]
struct SB {
  [[size(128)]]
  a : i32;
  b : [[stride(256)]] array<S2>;
};

[[group(0), binding(0)]] var<storage> sb : [[access(read_write)]] SB;

[[stage(compute)]]
fn main() {
  var i : i32 = 4;
  var j : u32 = 1u;
  var k : i32 = 2;
  var x : f32 = sb.b[i].b[j].b[k];
}
)";

  auto* expect = R"(
struct S1 {
  a : i32;
  b : vec3<f32>;
  c : i32;
};

struct S2 {
  a : i32;
  b : [[stride(32)]] array<S1, 3>;
  c : i32;
};

[[block]]
struct SB {
  [[size(128)]]
  a : i32;
  b : [[stride(256)]] array<S2>;
};

[[internal(intrinsic_load_f32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol(buffer : [[access(read_write)]] SB, offset : u32) -> f32

[[group(0), binding(0)]] var<storage> sb : [[access(read_write)]] SB;

[[stage(compute)]]
fn main() {
  var i : i32 = 4;
  var j : u32 = 1u;
  var k : i32 = 2;
  var x : f32 = tint_symbol(sb, (((((128u + (256u * u32(i))) + 16u) + (32u * j)) + 16u) + (4u * u32(k))));
}
)";

  auto got = Run<DecomposeStorageAccess>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStorageAccessTest, ComplexDynamicAccessChainWithAliases) {
  auto* src = R"(
struct S1 {
  a : i32;
  b : vec3<f32>;
  c : i32;
};

type A1 = S1;

type A1_Array = [[stride(32)]] array<S1, 3>;

struct S2 {
  a : i32;
  b : A1_Array;
  c : i32;
};

type A2 = S2;

type A2_Array = [[stride(256)]] array<S2>;

[[block]]
struct SB {
  [[size(128)]]
  a : i32;
  b : A2_Array;
};

[[group(0), binding(0)]] var<storage> sb : [[access(read_write)]] SB;

[[stage(compute)]]
fn main() {
  var i : i32 = 4;
  var j : u32 = 1u;
  var k : i32 = 2;
  var x : f32 = sb.b[i].b[j].b[k];
}
)";

  auto* expect = R"(
struct S1 {
  a : i32;
  b : vec3<f32>;
  c : i32;
};

type A1 = S1;

type A1_Array = [[stride(32)]] array<S1, 3>;

struct S2 {
  a : i32;
  b : A1_Array;
  c : i32;
};

type A2 = S2;

type A2_Array = [[stride(256)]] array<S2>;

[[block]]
struct SB {
  [[size(128)]]
  a : i32;
  b : A2_Array;
};

[[internal(intrinsic_load_f32), internal(disable_validation__function_has_no_body)]]
fn tint_symbol(buffer : [[access(read_write)]] SB, offset : u32) -> f32

[[group(0), binding(0)]] var<storage> sb : [[access(read_write)]] SB;

[[stage(compute)]]
fn main() {
  var i : i32 = 4;
  var j : u32 = 1u;
  var k : i32 = 2;
  var x : f32 = tint_symbol(sb, (((((128u + (256u * u32(i))) + 16u) + (32u * j)) + 16u) + (4u * u32(k))));
}
)";

  auto got = Run<DecomposeStorageAccess>(src);

  EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace transform
}  // namespace tint
