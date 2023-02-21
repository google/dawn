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
enable f16;

struct SB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_vec3_f16 : array<vec3<f16>, 2>,
};

@group(0) @binding(0) var<storage, read_write> sb : SB;

@compute @workgroup_size(1)
fn main() {
  var scalar_f32 : f32 = sb.scalar_f32;
  var scalar_i32 : i32 = sb.scalar_i32;
  var scalar_u32 : u32 = sb.scalar_u32;
  var scalar_f16 : f16 = sb.scalar_f16;
  var vec2_f32 : vec2<f32> = sb.vec2_f32;
  var vec2_i32 : vec2<i32> = sb.vec2_i32;
  var vec2_u32 : vec2<u32> = sb.vec2_u32;
  var vec2_f16 : vec2<f16> = sb.vec2_f16;
  var vec3_f32 : vec3<f32> = sb.vec3_f32;
  var vec3_i32 : vec3<i32> = sb.vec3_i32;
  var vec3_u32 : vec3<u32> = sb.vec3_u32;
  var vec3_f16 : vec3<f16> = sb.vec3_f16;
  var vec4_f32 : vec4<f32> = sb.vec4_f32;
  var vec4_i32 : vec4<i32> = sb.vec4_i32;
  var vec4_u32 : vec4<u32> = sb.vec4_u32;
  var vec4_f16 : vec4<f16> = sb.vec4_f16;
  var mat2x2_f32 : mat2x2<f32> = sb.mat2x2_f32;
  var mat2x3_f32 : mat2x3<f32> = sb.mat2x3_f32;
  var mat2x4_f32 : mat2x4<f32> = sb.mat2x4_f32;
  var mat3x2_f32 : mat3x2<f32> = sb.mat3x2_f32;
  var mat3x3_f32 : mat3x3<f32> = sb.mat3x3_f32;
  var mat3x4_f32 : mat3x4<f32> = sb.mat3x4_f32;
  var mat4x2_f32 : mat4x2<f32> = sb.mat4x2_f32;
  var mat4x3_f32 : mat4x3<f32> = sb.mat4x3_f32;
  var mat4x4_f32 : mat4x4<f32> = sb.mat4x4_f32;
  var mat2x2_f16 : mat2x2<f16> = sb.mat2x2_f16;
  var mat2x3_f16 : mat2x3<f16> = sb.mat2x3_f16;
  var mat2x4_f16 : mat2x4<f16> = sb.mat2x4_f16;
  var mat3x2_f16 : mat3x2<f16> = sb.mat3x2_f16;
  var mat3x3_f16 : mat3x3<f16> = sb.mat3x3_f16;
  var mat3x4_f16 : mat3x4<f16> = sb.mat3x4_f16;
  var mat4x2_f16 : mat4x2<f16> = sb.mat4x2_f16;
  var mat4x3_f16 : mat4x3<f16> = sb.mat4x3_f16;
  var mat4x4_f16 : mat4x4<f16> = sb.mat4x4_f16;
  var arr2_vec3_f32 : array<vec3<f32>, 2> = sb.arr2_vec3_f32;
  var arr2_vec3_f16 : array<vec3<f16>, 2> = sb.arr2_vec3_f16;
}
)";

    auto* expect = R"(
enable f16;

struct SB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_vec3_f16 : array<vec3<f16>, 2>,
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

@internal(intrinsic_load_storage_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> f32

@internal(intrinsic_load_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_1(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> i32

@internal(intrinsic_load_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_2(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> u32

@internal(intrinsic_load_storage_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_3(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> f16

@internal(intrinsic_load_storage_vec2_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_4(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec2<f32>

@internal(intrinsic_load_storage_vec2_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_5(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec2<i32>

@internal(intrinsic_load_storage_vec2_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_6(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec2<u32>

@internal(intrinsic_load_storage_vec2_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_7(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec2<f16>

@internal(intrinsic_load_storage_vec3_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_8(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec3<f32>

@internal(intrinsic_load_storage_vec3_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_9(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec3<i32>

@internal(intrinsic_load_storage_vec3_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_10(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec3<u32>

@internal(intrinsic_load_storage_vec3_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_11(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec3<f16>

@internal(intrinsic_load_storage_vec4_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_12(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec4<f32>

@internal(intrinsic_load_storage_vec4_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_13(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec4<i32>

@internal(intrinsic_load_storage_vec4_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_14(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec4<u32>

@internal(intrinsic_load_storage_vec4_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_15(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec4<f16>

fn tint_symbol_16(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat2x2<f32> {
  return mat2x2<f32>(tint_symbol_4(buffer, (offset + 0u)), tint_symbol_4(buffer, (offset + 8u)));
}

fn tint_symbol_17(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat2x3<f32> {
  return mat2x3<f32>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 16u)));
}

fn tint_symbol_18(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat2x4<f32> {
  return mat2x4<f32>(tint_symbol_12(buffer, (offset + 0u)), tint_symbol_12(buffer, (offset + 16u)));
}

fn tint_symbol_19(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat3x2<f32> {
  return mat3x2<f32>(tint_symbol_4(buffer, (offset + 0u)), tint_symbol_4(buffer, (offset + 8u)), tint_symbol_4(buffer, (offset + 16u)));
}

fn tint_symbol_20(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat3x3<f32> {
  return mat3x3<f32>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 16u)), tint_symbol_8(buffer, (offset + 32u)));
}

fn tint_symbol_21(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat3x4<f32> {
  return mat3x4<f32>(tint_symbol_12(buffer, (offset + 0u)), tint_symbol_12(buffer, (offset + 16u)), tint_symbol_12(buffer, (offset + 32u)));
}

fn tint_symbol_22(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat4x2<f32> {
  return mat4x2<f32>(tint_symbol_4(buffer, (offset + 0u)), tint_symbol_4(buffer, (offset + 8u)), tint_symbol_4(buffer, (offset + 16u)), tint_symbol_4(buffer, (offset + 24u)));
}

fn tint_symbol_23(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat4x3<f32> {
  return mat4x3<f32>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 16u)), tint_symbol_8(buffer, (offset + 32u)), tint_symbol_8(buffer, (offset + 48u)));
}

fn tint_symbol_24(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat4x4<f32> {
  return mat4x4<f32>(tint_symbol_12(buffer, (offset + 0u)), tint_symbol_12(buffer, (offset + 16u)), tint_symbol_12(buffer, (offset + 32u)), tint_symbol_12(buffer, (offset + 48u)));
}

fn tint_symbol_25(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat2x2<f16> {
  return mat2x2<f16>(tint_symbol_7(buffer, (offset + 0u)), tint_symbol_7(buffer, (offset + 4u)));
}

fn tint_symbol_26(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat2x3<f16> {
  return mat2x3<f16>(tint_symbol_11(buffer, (offset + 0u)), tint_symbol_11(buffer, (offset + 8u)));
}

fn tint_symbol_27(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat2x4<f16> {
  return mat2x4<f16>(tint_symbol_15(buffer, (offset + 0u)), tint_symbol_15(buffer, (offset + 8u)));
}

fn tint_symbol_28(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat3x2<f16> {
  return mat3x2<f16>(tint_symbol_7(buffer, (offset + 0u)), tint_symbol_7(buffer, (offset + 4u)), tint_symbol_7(buffer, (offset + 8u)));
}

fn tint_symbol_29(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat3x3<f16> {
  return mat3x3<f16>(tint_symbol_11(buffer, (offset + 0u)), tint_symbol_11(buffer, (offset + 8u)), tint_symbol_11(buffer, (offset + 16u)));
}

fn tint_symbol_30(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat3x4<f16> {
  return mat3x4<f16>(tint_symbol_15(buffer, (offset + 0u)), tint_symbol_15(buffer, (offset + 8u)), tint_symbol_15(buffer, (offset + 16u)));
}

fn tint_symbol_31(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat4x2<f16> {
  return mat4x2<f16>(tint_symbol_7(buffer, (offset + 0u)), tint_symbol_7(buffer, (offset + 4u)), tint_symbol_7(buffer, (offset + 8u)), tint_symbol_7(buffer, (offset + 12u)));
}

fn tint_symbol_32(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat4x3<f16> {
  return mat4x3<f16>(tint_symbol_11(buffer, (offset + 0u)), tint_symbol_11(buffer, (offset + 8u)), tint_symbol_11(buffer, (offset + 16u)), tint_symbol_11(buffer, (offset + 24u)));
}

fn tint_symbol_33(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat4x4<f16> {
  return mat4x4<f16>(tint_symbol_15(buffer, (offset + 0u)), tint_symbol_15(buffer, (offset + 8u)), tint_symbol_15(buffer, (offset + 16u)), tint_symbol_15(buffer, (offset + 24u)));
}

fn tint_symbol_34(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> array<vec3<f32>, 2u> {
  var arr : array<vec3<f32>, 2u>;
  for(var i = 0u; (i < 2u); i = (i + 1u)) {
    arr[i] = tint_symbol_8(buffer, (offset + (i * 16u)));
  }
  return arr;
}

fn tint_symbol_35(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> array<vec3<f16>, 2u> {
  var arr_1 : array<vec3<f16>, 2u>;
  for(var i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
    arr_1[i_1] = tint_symbol_11(buffer, (offset + (i_1 * 8u)));
  }
  return arr_1;
}

@compute @workgroup_size(1)
fn main() {
  var scalar_f32 : f32 = tint_symbol(&(sb), 0u);
  var scalar_i32 : i32 = tint_symbol_1(&(sb), 4u);
  var scalar_u32 : u32 = tint_symbol_2(&(sb), 8u);
  var scalar_f16 : f16 = tint_symbol_3(&(sb), 12u);
  var vec2_f32 : vec2<f32> = tint_symbol_4(&(sb), 16u);
  var vec2_i32 : vec2<i32> = tint_symbol_5(&(sb), 24u);
  var vec2_u32 : vec2<u32> = tint_symbol_6(&(sb), 32u);
  var vec2_f16 : vec2<f16> = tint_symbol_7(&(sb), 40u);
  var vec3_f32 : vec3<f32> = tint_symbol_8(&(sb), 48u);
  var vec3_i32 : vec3<i32> = tint_symbol_9(&(sb), 64u);
  var vec3_u32 : vec3<u32> = tint_symbol_10(&(sb), 80u);
  var vec3_f16 : vec3<f16> = tint_symbol_11(&(sb), 96u);
  var vec4_f32 : vec4<f32> = tint_symbol_12(&(sb), 112u);
  var vec4_i32 : vec4<i32> = tint_symbol_13(&(sb), 128u);
  var vec4_u32 : vec4<u32> = tint_symbol_14(&(sb), 144u);
  var vec4_f16 : vec4<f16> = tint_symbol_15(&(sb), 160u);
  var mat2x2_f32 : mat2x2<f32> = tint_symbol_16(&(sb), 168u);
  var mat2x3_f32 : mat2x3<f32> = tint_symbol_17(&(sb), 192u);
  var mat2x4_f32 : mat2x4<f32> = tint_symbol_18(&(sb), 224u);
  var mat3x2_f32 : mat3x2<f32> = tint_symbol_19(&(sb), 256u);
  var mat3x3_f32 : mat3x3<f32> = tint_symbol_20(&(sb), 288u);
  var mat3x4_f32 : mat3x4<f32> = tint_symbol_21(&(sb), 336u);
  var mat4x2_f32 : mat4x2<f32> = tint_symbol_22(&(sb), 384u);
  var mat4x3_f32 : mat4x3<f32> = tint_symbol_23(&(sb), 416u);
  var mat4x4_f32 : mat4x4<f32> = tint_symbol_24(&(sb), 480u);
  var mat2x2_f16 : mat2x2<f16> = tint_symbol_25(&(sb), 544u);
  var mat2x3_f16 : mat2x3<f16> = tint_symbol_26(&(sb), 552u);
  var mat2x4_f16 : mat2x4<f16> = tint_symbol_27(&(sb), 568u);
  var mat3x2_f16 : mat3x2<f16> = tint_symbol_28(&(sb), 584u);
  var mat3x3_f16 : mat3x3<f16> = tint_symbol_29(&(sb), 600u);
  var mat3x4_f16 : mat3x4<f16> = tint_symbol_30(&(sb), 624u);
  var mat4x2_f16 : mat4x2<f16> = tint_symbol_31(&(sb), 648u);
  var mat4x3_f16 : mat4x3<f16> = tint_symbol_32(&(sb), 664u);
  var mat4x4_f16 : mat4x4<f16> = tint_symbol_33(&(sb), 696u);
  var arr2_vec3_f32 : array<vec3<f32>, 2> = tint_symbol_34(&(sb), 736u);
  var arr2_vec3_f16 : array<vec3<f16>, 2> = tint_symbol_35(&(sb), 768u);
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, SB_BasicLoad_OutOfOrder) {
    auto* src = R"(
enable f16;

@compute @workgroup_size(1)
fn main() {
  var scalar_f32 : f32 = sb.scalar_f32;
  var scalar_i32 : i32 = sb.scalar_i32;
  var scalar_u32 : u32 = sb.scalar_u32;
  var scalar_f16 : f16 = sb.scalar_f16;
  var vec2_f32 : vec2<f32> = sb.vec2_f32;
  var vec2_i32 : vec2<i32> = sb.vec2_i32;
  var vec2_u32 : vec2<u32> = sb.vec2_u32;
  var vec2_f16 : vec2<f16> = sb.vec2_f16;
  var vec3_f32 : vec3<f32> = sb.vec3_f32;
  var vec3_i32 : vec3<i32> = sb.vec3_i32;
  var vec3_u32 : vec3<u32> = sb.vec3_u32;
  var vec3_f16 : vec3<f16> = sb.vec3_f16;
  var vec4_f32 : vec4<f32> = sb.vec4_f32;
  var vec4_i32 : vec4<i32> = sb.vec4_i32;
  var vec4_u32 : vec4<u32> = sb.vec4_u32;
  var vec4_f16 : vec4<f16> = sb.vec4_f16;
  var mat2x2_f32 : mat2x2<f32> = sb.mat2x2_f32;
  var mat2x3_f32 : mat2x3<f32> = sb.mat2x3_f32;
  var mat2x4_f32 : mat2x4<f32> = sb.mat2x4_f32;
  var mat3x2_f32 : mat3x2<f32> = sb.mat3x2_f32;
  var mat3x3_f32 : mat3x3<f32> = sb.mat3x3_f32;
  var mat3x4_f32 : mat3x4<f32> = sb.mat3x4_f32;
  var mat4x2_f32 : mat4x2<f32> = sb.mat4x2_f32;
  var mat4x3_f32 : mat4x3<f32> = sb.mat4x3_f32;
  var mat4x4_f32 : mat4x4<f32> = sb.mat4x4_f32;
  var mat2x2_f16 : mat2x2<f16> = sb.mat2x2_f16;
  var mat2x3_f16 : mat2x3<f16> = sb.mat2x3_f16;
  var mat2x4_f16 : mat2x4<f16> = sb.mat2x4_f16;
  var mat3x2_f16 : mat3x2<f16> = sb.mat3x2_f16;
  var mat3x3_f16 : mat3x3<f16> = sb.mat3x3_f16;
  var mat3x4_f16 : mat3x4<f16> = sb.mat3x4_f16;
  var mat4x2_f16 : mat4x2<f16> = sb.mat4x2_f16;
  var mat4x3_f16 : mat4x3<f16> = sb.mat4x3_f16;
  var mat4x4_f16 : mat4x4<f16> = sb.mat4x4_f16;
  var arr2_vec3_f32 : array<vec3<f32>, 2> = sb.arr2_vec3_f32;
  var arr2_vec3_f16 : array<vec3<f16>, 2> = sb.arr2_vec3_f16;
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_vec3_f16 : array<vec3<f16>, 2>,
};
)";

    auto* expect = R"(
enable f16;

@internal(intrinsic_load_storage_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> f32

@internal(intrinsic_load_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_1(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> i32

@internal(intrinsic_load_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_2(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> u32

@internal(intrinsic_load_storage_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_3(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> f16

@internal(intrinsic_load_storage_vec2_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_4(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec2<f32>

@internal(intrinsic_load_storage_vec2_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_5(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec2<i32>

@internal(intrinsic_load_storage_vec2_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_6(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec2<u32>

@internal(intrinsic_load_storage_vec2_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_7(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec2<f16>

@internal(intrinsic_load_storage_vec3_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_8(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec3<f32>

@internal(intrinsic_load_storage_vec3_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_9(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec3<i32>

@internal(intrinsic_load_storage_vec3_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_10(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec3<u32>

@internal(intrinsic_load_storage_vec3_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_11(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec3<f16>

@internal(intrinsic_load_storage_vec4_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_12(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec4<f32>

@internal(intrinsic_load_storage_vec4_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_13(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec4<i32>

@internal(intrinsic_load_storage_vec4_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_14(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec4<u32>

@internal(intrinsic_load_storage_vec4_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_15(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec4<f16>

fn tint_symbol_16(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat2x2<f32> {
  return mat2x2<f32>(tint_symbol_4(buffer, (offset + 0u)), tint_symbol_4(buffer, (offset + 8u)));
}

fn tint_symbol_17(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat2x3<f32> {
  return mat2x3<f32>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 16u)));
}

fn tint_symbol_18(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat2x4<f32> {
  return mat2x4<f32>(tint_symbol_12(buffer, (offset + 0u)), tint_symbol_12(buffer, (offset + 16u)));
}

fn tint_symbol_19(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat3x2<f32> {
  return mat3x2<f32>(tint_symbol_4(buffer, (offset + 0u)), tint_symbol_4(buffer, (offset + 8u)), tint_symbol_4(buffer, (offset + 16u)));
}

fn tint_symbol_20(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat3x3<f32> {
  return mat3x3<f32>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 16u)), tint_symbol_8(buffer, (offset + 32u)));
}

fn tint_symbol_21(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat3x4<f32> {
  return mat3x4<f32>(tint_symbol_12(buffer, (offset + 0u)), tint_symbol_12(buffer, (offset + 16u)), tint_symbol_12(buffer, (offset + 32u)));
}

fn tint_symbol_22(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat4x2<f32> {
  return mat4x2<f32>(tint_symbol_4(buffer, (offset + 0u)), tint_symbol_4(buffer, (offset + 8u)), tint_symbol_4(buffer, (offset + 16u)), tint_symbol_4(buffer, (offset + 24u)));
}

fn tint_symbol_23(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat4x3<f32> {
  return mat4x3<f32>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 16u)), tint_symbol_8(buffer, (offset + 32u)), tint_symbol_8(buffer, (offset + 48u)));
}

fn tint_symbol_24(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat4x4<f32> {
  return mat4x4<f32>(tint_symbol_12(buffer, (offset + 0u)), tint_symbol_12(buffer, (offset + 16u)), tint_symbol_12(buffer, (offset + 32u)), tint_symbol_12(buffer, (offset + 48u)));
}

fn tint_symbol_25(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat2x2<f16> {
  return mat2x2<f16>(tint_symbol_7(buffer, (offset + 0u)), tint_symbol_7(buffer, (offset + 4u)));
}

fn tint_symbol_26(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat2x3<f16> {
  return mat2x3<f16>(tint_symbol_11(buffer, (offset + 0u)), tint_symbol_11(buffer, (offset + 8u)));
}

fn tint_symbol_27(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat2x4<f16> {
  return mat2x4<f16>(tint_symbol_15(buffer, (offset + 0u)), tint_symbol_15(buffer, (offset + 8u)));
}

fn tint_symbol_28(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat3x2<f16> {
  return mat3x2<f16>(tint_symbol_7(buffer, (offset + 0u)), tint_symbol_7(buffer, (offset + 4u)), tint_symbol_7(buffer, (offset + 8u)));
}

fn tint_symbol_29(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat3x3<f16> {
  return mat3x3<f16>(tint_symbol_11(buffer, (offset + 0u)), tint_symbol_11(buffer, (offset + 8u)), tint_symbol_11(buffer, (offset + 16u)));
}

fn tint_symbol_30(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat3x4<f16> {
  return mat3x4<f16>(tint_symbol_15(buffer, (offset + 0u)), tint_symbol_15(buffer, (offset + 8u)), tint_symbol_15(buffer, (offset + 16u)));
}

fn tint_symbol_31(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat4x2<f16> {
  return mat4x2<f16>(tint_symbol_7(buffer, (offset + 0u)), tint_symbol_7(buffer, (offset + 4u)), tint_symbol_7(buffer, (offset + 8u)), tint_symbol_7(buffer, (offset + 12u)));
}

fn tint_symbol_32(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat4x3<f16> {
  return mat4x3<f16>(tint_symbol_11(buffer, (offset + 0u)), tint_symbol_11(buffer, (offset + 8u)), tint_symbol_11(buffer, (offset + 16u)), tint_symbol_11(buffer, (offset + 24u)));
}

fn tint_symbol_33(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat4x4<f16> {
  return mat4x4<f16>(tint_symbol_15(buffer, (offset + 0u)), tint_symbol_15(buffer, (offset + 8u)), tint_symbol_15(buffer, (offset + 16u)), tint_symbol_15(buffer, (offset + 24u)));
}

fn tint_symbol_34(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> array<vec3<f32>, 2u> {
  var arr : array<vec3<f32>, 2u>;
  for(var i = 0u; (i < 2u); i = (i + 1u)) {
    arr[i] = tint_symbol_8(buffer, (offset + (i * 16u)));
  }
  return arr;
}

fn tint_symbol_35(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> array<vec3<f16>, 2u> {
  var arr_1 : array<vec3<f16>, 2u>;
  for(var i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
    arr_1[i_1] = tint_symbol_11(buffer, (offset + (i_1 * 8u)));
  }
  return arr_1;
}

@compute @workgroup_size(1)
fn main() {
  var scalar_f32 : f32 = tint_symbol(&(sb), 0u);
  var scalar_i32 : i32 = tint_symbol_1(&(sb), 4u);
  var scalar_u32 : u32 = tint_symbol_2(&(sb), 8u);
  var scalar_f16 : f16 = tint_symbol_3(&(sb), 12u);
  var vec2_f32 : vec2<f32> = tint_symbol_4(&(sb), 16u);
  var vec2_i32 : vec2<i32> = tint_symbol_5(&(sb), 24u);
  var vec2_u32 : vec2<u32> = tint_symbol_6(&(sb), 32u);
  var vec2_f16 : vec2<f16> = tint_symbol_7(&(sb), 40u);
  var vec3_f32 : vec3<f32> = tint_symbol_8(&(sb), 48u);
  var vec3_i32 : vec3<i32> = tint_symbol_9(&(sb), 64u);
  var vec3_u32 : vec3<u32> = tint_symbol_10(&(sb), 80u);
  var vec3_f16 : vec3<f16> = tint_symbol_11(&(sb), 96u);
  var vec4_f32 : vec4<f32> = tint_symbol_12(&(sb), 112u);
  var vec4_i32 : vec4<i32> = tint_symbol_13(&(sb), 128u);
  var vec4_u32 : vec4<u32> = tint_symbol_14(&(sb), 144u);
  var vec4_f16 : vec4<f16> = tint_symbol_15(&(sb), 160u);
  var mat2x2_f32 : mat2x2<f32> = tint_symbol_16(&(sb), 168u);
  var mat2x3_f32 : mat2x3<f32> = tint_symbol_17(&(sb), 192u);
  var mat2x4_f32 : mat2x4<f32> = tint_symbol_18(&(sb), 224u);
  var mat3x2_f32 : mat3x2<f32> = tint_symbol_19(&(sb), 256u);
  var mat3x3_f32 : mat3x3<f32> = tint_symbol_20(&(sb), 288u);
  var mat3x4_f32 : mat3x4<f32> = tint_symbol_21(&(sb), 336u);
  var mat4x2_f32 : mat4x2<f32> = tint_symbol_22(&(sb), 384u);
  var mat4x3_f32 : mat4x3<f32> = tint_symbol_23(&(sb), 416u);
  var mat4x4_f32 : mat4x4<f32> = tint_symbol_24(&(sb), 480u);
  var mat2x2_f16 : mat2x2<f16> = tint_symbol_25(&(sb), 544u);
  var mat2x3_f16 : mat2x3<f16> = tint_symbol_26(&(sb), 552u);
  var mat2x4_f16 : mat2x4<f16> = tint_symbol_27(&(sb), 568u);
  var mat3x2_f16 : mat3x2<f16> = tint_symbol_28(&(sb), 584u);
  var mat3x3_f16 : mat3x3<f16> = tint_symbol_29(&(sb), 600u);
  var mat3x4_f16 : mat3x4<f16> = tint_symbol_30(&(sb), 624u);
  var mat4x2_f16 : mat4x2<f16> = tint_symbol_31(&(sb), 648u);
  var mat4x3_f16 : mat4x3<f16> = tint_symbol_32(&(sb), 664u);
  var mat4x4_f16 : mat4x4<f16> = tint_symbol_33(&(sb), 696u);
  var arr2_vec3_f32 : array<vec3<f32>, 2> = tint_symbol_34(&(sb), 736u);
  var arr2_vec3_f16 : array<vec3<f16>, 2> = tint_symbol_35(&(sb), 768u);
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_vec3_f16 : array<vec3<f16>, 2>,
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, UB_BasicLoad) {
    auto* src = R"(
enable f16;

struct UB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_mat4x2_f16 : array<mat4x2<f16>, 2>,
};

@group(0) @binding(0) var<uniform> ub : UB;

@compute @workgroup_size(1)
fn main() {
  var scalar_f32 : f32 = ub.scalar_f32;
  var scalar_i32 : i32 = ub.scalar_i32;
  var scalar_u32 : u32 = ub.scalar_u32;
  var scalar_f16 : f16 = ub.scalar_f16;
  var vec2_f32 : vec2<f32> = ub.vec2_f32;
  var vec2_i32 : vec2<i32> = ub.vec2_i32;
  var vec2_u32 : vec2<u32> = ub.vec2_u32;
  var vec2_f16 : vec2<f16> = ub.vec2_f16;
  var vec3_f32 : vec3<f32> = ub.vec3_f32;
  var vec3_i32 : vec3<i32> = ub.vec3_i32;
  var vec3_u32 : vec3<u32> = ub.vec3_u32;
  var vec3_f16 : vec3<f16> = ub.vec3_f16;
  var vec4_f32 : vec4<f32> = ub.vec4_f32;
  var vec4_i32 : vec4<i32> = ub.vec4_i32;
  var vec4_u32 : vec4<u32> = ub.vec4_u32;
  var vec4_f16 : vec4<f16> = ub.vec4_f16;
  var mat2x2_f32 : mat2x2<f32> = ub.mat2x2_f32;
  var mat2x3_f32 : mat2x3<f32> = ub.mat2x3_f32;
  var mat2x4_f32 : mat2x4<f32> = ub.mat2x4_f32;
  var mat3x2_f32 : mat3x2<f32> = ub.mat3x2_f32;
  var mat3x3_f32 : mat3x3<f32> = ub.mat3x3_f32;
  var mat3x4_f32 : mat3x4<f32> = ub.mat3x4_f32;
  var mat4x2_f32 : mat4x2<f32> = ub.mat4x2_f32;
  var mat4x3_f32 : mat4x3<f32> = ub.mat4x3_f32;
  var mat4x4_f32 : mat4x4<f32> = ub.mat4x4_f32;
  var mat2x2_f16 : mat2x2<f16> = ub.mat2x2_f16;
  var mat2x3_f16 : mat2x3<f16> = ub.mat2x3_f16;
  var mat2x4_f16 : mat2x4<f16> = ub.mat2x4_f16;
  var mat3x2_f16 : mat3x2<f16> = ub.mat3x2_f16;
  var mat3x3_f16 : mat3x3<f16> = ub.mat3x3_f16;
  var mat3x4_f16 : mat3x4<f16> = ub.mat3x4_f16;
  var mat4x2_f16 : mat4x2<f16> = ub.mat4x2_f16;
  var mat4x3_f16 : mat4x3<f16> = ub.mat4x3_f16;
  var mat4x4_f16 : mat4x4<f16> = ub.mat4x4_f16;
  var arr2_vec3_f32 : array<vec3<f32>, 2> = ub.arr2_vec3_f32;
  var arr2_mat4x2_f16 : array<mat4x2<f16>, 2> = ub.arr2_mat4x2_f16;
}
)";

    auto* expect = R"(
enable f16;

struct UB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_mat4x2_f16 : array<mat4x2<f16>, 2>,
}

@group(0) @binding(0) var<uniform> ub : UB;

@internal(intrinsic_load_uniform_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> f32

@internal(intrinsic_load_uniform_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_1(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> i32

@internal(intrinsic_load_uniform_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_2(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> u32

@internal(intrinsic_load_uniform_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_3(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> f16

@internal(intrinsic_load_uniform_vec2_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_4(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> vec2<f32>

@internal(intrinsic_load_uniform_vec2_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_5(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> vec2<i32>

@internal(intrinsic_load_uniform_vec2_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_6(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> vec2<u32>

@internal(intrinsic_load_uniform_vec2_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_7(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> vec2<f16>

@internal(intrinsic_load_uniform_vec3_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_8(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> vec3<f32>

@internal(intrinsic_load_uniform_vec3_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_9(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> vec3<i32>

@internal(intrinsic_load_uniform_vec3_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_10(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> vec3<u32>

@internal(intrinsic_load_uniform_vec3_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_11(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> vec3<f16>

@internal(intrinsic_load_uniform_vec4_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_12(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> vec4<f32>

@internal(intrinsic_load_uniform_vec4_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_13(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> vec4<i32>

@internal(intrinsic_load_uniform_vec4_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_14(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> vec4<u32>

@internal(intrinsic_load_uniform_vec4_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_15(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> vec4<f16>

fn tint_symbol_16(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> mat2x2<f32> {
  return mat2x2<f32>(tint_symbol_4(buffer, (offset + 0u)), tint_symbol_4(buffer, (offset + 8u)));
}

fn tint_symbol_17(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> mat2x3<f32> {
  return mat2x3<f32>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 16u)));
}

fn tint_symbol_18(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> mat2x4<f32> {
  return mat2x4<f32>(tint_symbol_12(buffer, (offset + 0u)), tint_symbol_12(buffer, (offset + 16u)));
}

fn tint_symbol_19(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> mat3x2<f32> {
  return mat3x2<f32>(tint_symbol_4(buffer, (offset + 0u)), tint_symbol_4(buffer, (offset + 8u)), tint_symbol_4(buffer, (offset + 16u)));
}

fn tint_symbol_20(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> mat3x3<f32> {
  return mat3x3<f32>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 16u)), tint_symbol_8(buffer, (offset + 32u)));
}

fn tint_symbol_21(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> mat3x4<f32> {
  return mat3x4<f32>(tint_symbol_12(buffer, (offset + 0u)), tint_symbol_12(buffer, (offset + 16u)), tint_symbol_12(buffer, (offset + 32u)));
}

fn tint_symbol_22(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> mat4x2<f32> {
  return mat4x2<f32>(tint_symbol_4(buffer, (offset + 0u)), tint_symbol_4(buffer, (offset + 8u)), tint_symbol_4(buffer, (offset + 16u)), tint_symbol_4(buffer, (offset + 24u)));
}

fn tint_symbol_23(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> mat4x3<f32> {
  return mat4x3<f32>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 16u)), tint_symbol_8(buffer, (offset + 32u)), tint_symbol_8(buffer, (offset + 48u)));
}

fn tint_symbol_24(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> mat4x4<f32> {
  return mat4x4<f32>(tint_symbol_12(buffer, (offset + 0u)), tint_symbol_12(buffer, (offset + 16u)), tint_symbol_12(buffer, (offset + 32u)), tint_symbol_12(buffer, (offset + 48u)));
}

fn tint_symbol_25(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> mat2x2<f16> {
  return mat2x2<f16>(tint_symbol_7(buffer, (offset + 0u)), tint_symbol_7(buffer, (offset + 4u)));
}

fn tint_symbol_26(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> mat2x3<f16> {
  return mat2x3<f16>(tint_symbol_11(buffer, (offset + 0u)), tint_symbol_11(buffer, (offset + 8u)));
}

fn tint_symbol_27(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> mat2x4<f16> {
  return mat2x4<f16>(tint_symbol_15(buffer, (offset + 0u)), tint_symbol_15(buffer, (offset + 8u)));
}

fn tint_symbol_28(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> mat3x2<f16> {
  return mat3x2<f16>(tint_symbol_7(buffer, (offset + 0u)), tint_symbol_7(buffer, (offset + 4u)), tint_symbol_7(buffer, (offset + 8u)));
}

fn tint_symbol_29(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> mat3x3<f16> {
  return mat3x3<f16>(tint_symbol_11(buffer, (offset + 0u)), tint_symbol_11(buffer, (offset + 8u)), tint_symbol_11(buffer, (offset + 16u)));
}

fn tint_symbol_30(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> mat3x4<f16> {
  return mat3x4<f16>(tint_symbol_15(buffer, (offset + 0u)), tint_symbol_15(buffer, (offset + 8u)), tint_symbol_15(buffer, (offset + 16u)));
}

fn tint_symbol_31(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> mat4x2<f16> {
  return mat4x2<f16>(tint_symbol_7(buffer, (offset + 0u)), tint_symbol_7(buffer, (offset + 4u)), tint_symbol_7(buffer, (offset + 8u)), tint_symbol_7(buffer, (offset + 12u)));
}

fn tint_symbol_32(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> mat4x3<f16> {
  return mat4x3<f16>(tint_symbol_11(buffer, (offset + 0u)), tint_symbol_11(buffer, (offset + 8u)), tint_symbol_11(buffer, (offset + 16u)), tint_symbol_11(buffer, (offset + 24u)));
}

fn tint_symbol_33(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> mat4x4<f16> {
  return mat4x4<f16>(tint_symbol_15(buffer, (offset + 0u)), tint_symbol_15(buffer, (offset + 8u)), tint_symbol_15(buffer, (offset + 16u)), tint_symbol_15(buffer, (offset + 24u)));
}

fn tint_symbol_34(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> array<vec3<f32>, 2u> {
  var arr : array<vec3<f32>, 2u>;
  for(var i = 0u; (i < 2u); i = (i + 1u)) {
    arr[i] = tint_symbol_8(buffer, (offset + (i * 16u)));
  }
  return arr;
}

fn tint_symbol_35(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> array<mat4x2<f16>, 2u> {
  var arr_1 : array<mat4x2<f16>, 2u>;
  for(var i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
    arr_1[i_1] = tint_symbol_31(buffer, (offset + (i_1 * 16u)));
  }
  return arr_1;
}

@compute @workgroup_size(1)
fn main() {
  var scalar_f32 : f32 = tint_symbol(&(ub), 0u);
  var scalar_i32 : i32 = tint_symbol_1(&(ub), 4u);
  var scalar_u32 : u32 = tint_symbol_2(&(ub), 8u);
  var scalar_f16 : f16 = tint_symbol_3(&(ub), 12u);
  var vec2_f32 : vec2<f32> = tint_symbol_4(&(ub), 16u);
  var vec2_i32 : vec2<i32> = tint_symbol_5(&(ub), 24u);
  var vec2_u32 : vec2<u32> = tint_symbol_6(&(ub), 32u);
  var vec2_f16 : vec2<f16> = tint_symbol_7(&(ub), 40u);
  var vec3_f32 : vec3<f32> = tint_symbol_8(&(ub), 48u);
  var vec3_i32 : vec3<i32> = tint_symbol_9(&(ub), 64u);
  var vec3_u32 : vec3<u32> = tint_symbol_10(&(ub), 80u);
  var vec3_f16 : vec3<f16> = tint_symbol_11(&(ub), 96u);
  var vec4_f32 : vec4<f32> = tint_symbol_12(&(ub), 112u);
  var vec4_i32 : vec4<i32> = tint_symbol_13(&(ub), 128u);
  var vec4_u32 : vec4<u32> = tint_symbol_14(&(ub), 144u);
  var vec4_f16 : vec4<f16> = tint_symbol_15(&(ub), 160u);
  var mat2x2_f32 : mat2x2<f32> = tint_symbol_16(&(ub), 168u);
  var mat2x3_f32 : mat2x3<f32> = tint_symbol_17(&(ub), 192u);
  var mat2x4_f32 : mat2x4<f32> = tint_symbol_18(&(ub), 224u);
  var mat3x2_f32 : mat3x2<f32> = tint_symbol_19(&(ub), 256u);
  var mat3x3_f32 : mat3x3<f32> = tint_symbol_20(&(ub), 288u);
  var mat3x4_f32 : mat3x4<f32> = tint_symbol_21(&(ub), 336u);
  var mat4x2_f32 : mat4x2<f32> = tint_symbol_22(&(ub), 384u);
  var mat4x3_f32 : mat4x3<f32> = tint_symbol_23(&(ub), 416u);
  var mat4x4_f32 : mat4x4<f32> = tint_symbol_24(&(ub), 480u);
  var mat2x2_f16 : mat2x2<f16> = tint_symbol_25(&(ub), 544u);
  var mat2x3_f16 : mat2x3<f16> = tint_symbol_26(&(ub), 552u);
  var mat2x4_f16 : mat2x4<f16> = tint_symbol_27(&(ub), 568u);
  var mat3x2_f16 : mat3x2<f16> = tint_symbol_28(&(ub), 584u);
  var mat3x3_f16 : mat3x3<f16> = tint_symbol_29(&(ub), 600u);
  var mat3x4_f16 : mat3x4<f16> = tint_symbol_30(&(ub), 624u);
  var mat4x2_f16 : mat4x2<f16> = tint_symbol_31(&(ub), 648u);
  var mat4x3_f16 : mat4x3<f16> = tint_symbol_32(&(ub), 664u);
  var mat4x4_f16 : mat4x4<f16> = tint_symbol_33(&(ub), 696u);
  var arr2_vec3_f32 : array<vec3<f32>, 2> = tint_symbol_34(&(ub), 736u);
  var arr2_mat4x2_f16 : array<mat4x2<f16>, 2> = tint_symbol_35(&(ub), 768u);
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, UB_BasicLoad_OutOfOrder) {
    auto* src = R"(
enable f16;

@compute @workgroup_size(1)
fn main() {
  var scalar_f32 : f32 = ub.scalar_f32;
  var scalar_i32 : i32 = ub.scalar_i32;
  var scalar_u32 : u32 = ub.scalar_u32;
  var scalar_f16 : f16 = ub.scalar_f16;
  var vec2_f32 : vec2<f32> = ub.vec2_f32;
  var vec2_i32 : vec2<i32> = ub.vec2_i32;
  var vec2_u32 : vec2<u32> = ub.vec2_u32;
  var vec2_f16 : vec2<f16> = ub.vec2_f16;
  var vec3_f32 : vec3<f32> = ub.vec3_f32;
  var vec3_i32 : vec3<i32> = ub.vec3_i32;
  var vec3_u32 : vec3<u32> = ub.vec3_u32;
  var vec3_f16 : vec3<f16> = ub.vec3_f16;
  var vec4_f32 : vec4<f32> = ub.vec4_f32;
  var vec4_i32 : vec4<i32> = ub.vec4_i32;
  var vec4_u32 : vec4<u32> = ub.vec4_u32;
  var vec4_f16 : vec4<f16> = ub.vec4_f16;
  var mat2x2_f32 : mat2x2<f32> = ub.mat2x2_f32;
  var mat2x3_f32 : mat2x3<f32> = ub.mat2x3_f32;
  var mat2x4_f32 : mat2x4<f32> = ub.mat2x4_f32;
  var mat3x2_f32 : mat3x2<f32> = ub.mat3x2_f32;
  var mat3x3_f32 : mat3x3<f32> = ub.mat3x3_f32;
  var mat3x4_f32 : mat3x4<f32> = ub.mat3x4_f32;
  var mat4x2_f32 : mat4x2<f32> = ub.mat4x2_f32;
  var mat4x3_f32 : mat4x3<f32> = ub.mat4x3_f32;
  var mat4x4_f32 : mat4x4<f32> = ub.mat4x4_f32;
  var mat2x2_f16 : mat2x2<f16> = ub.mat2x2_f16;
  var mat2x3_f16 : mat2x3<f16> = ub.mat2x3_f16;
  var mat2x4_f16 : mat2x4<f16> = ub.mat2x4_f16;
  var mat3x2_f16 : mat3x2<f16> = ub.mat3x2_f16;
  var mat3x3_f16 : mat3x3<f16> = ub.mat3x3_f16;
  var mat3x4_f16 : mat3x4<f16> = ub.mat3x4_f16;
  var mat4x2_f16 : mat4x2<f16> = ub.mat4x2_f16;
  var mat4x3_f16 : mat4x3<f16> = ub.mat4x3_f16;
  var mat4x4_f16 : mat4x4<f16> = ub.mat4x4_f16;
  var arr2_vec3_f32 : array<vec3<f32>, 2> = ub.arr2_vec3_f32;
  var arr2_mat4x2_f16 : array<mat4x2<f16>, 2> = ub.arr2_mat4x2_f16;
}

@group(0) @binding(0) var<uniform> ub : UB;

struct UB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_mat4x2_f16 : array<mat4x2<f16>, 2>,
};
)";

    auto* expect = R"(
enable f16;

@internal(intrinsic_load_uniform_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> f32

@internal(intrinsic_load_uniform_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_1(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> i32

@internal(intrinsic_load_uniform_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_2(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> u32

@internal(intrinsic_load_uniform_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_3(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> f16

@internal(intrinsic_load_uniform_vec2_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_4(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> vec2<f32>

@internal(intrinsic_load_uniform_vec2_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_5(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> vec2<i32>

@internal(intrinsic_load_uniform_vec2_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_6(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> vec2<u32>

@internal(intrinsic_load_uniform_vec2_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_7(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> vec2<f16>

@internal(intrinsic_load_uniform_vec3_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_8(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> vec3<f32>

@internal(intrinsic_load_uniform_vec3_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_9(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> vec3<i32>

@internal(intrinsic_load_uniform_vec3_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_10(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> vec3<u32>

@internal(intrinsic_load_uniform_vec3_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_11(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> vec3<f16>

@internal(intrinsic_load_uniform_vec4_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_12(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> vec4<f32>

@internal(intrinsic_load_uniform_vec4_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_13(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> vec4<i32>

@internal(intrinsic_load_uniform_vec4_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_14(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> vec4<u32>

@internal(intrinsic_load_uniform_vec4_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_15(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> vec4<f16>

fn tint_symbol_16(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> mat2x2<f32> {
  return mat2x2<f32>(tint_symbol_4(buffer, (offset + 0u)), tint_symbol_4(buffer, (offset + 8u)));
}

fn tint_symbol_17(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> mat2x3<f32> {
  return mat2x3<f32>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 16u)));
}

fn tint_symbol_18(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> mat2x4<f32> {
  return mat2x4<f32>(tint_symbol_12(buffer, (offset + 0u)), tint_symbol_12(buffer, (offset + 16u)));
}

fn tint_symbol_19(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> mat3x2<f32> {
  return mat3x2<f32>(tint_symbol_4(buffer, (offset + 0u)), tint_symbol_4(buffer, (offset + 8u)), tint_symbol_4(buffer, (offset + 16u)));
}

fn tint_symbol_20(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> mat3x3<f32> {
  return mat3x3<f32>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 16u)), tint_symbol_8(buffer, (offset + 32u)));
}

fn tint_symbol_21(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> mat3x4<f32> {
  return mat3x4<f32>(tint_symbol_12(buffer, (offset + 0u)), tint_symbol_12(buffer, (offset + 16u)), tint_symbol_12(buffer, (offset + 32u)));
}

fn tint_symbol_22(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> mat4x2<f32> {
  return mat4x2<f32>(tint_symbol_4(buffer, (offset + 0u)), tint_symbol_4(buffer, (offset + 8u)), tint_symbol_4(buffer, (offset + 16u)), tint_symbol_4(buffer, (offset + 24u)));
}

fn tint_symbol_23(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> mat4x3<f32> {
  return mat4x3<f32>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 16u)), tint_symbol_8(buffer, (offset + 32u)), tint_symbol_8(buffer, (offset + 48u)));
}

fn tint_symbol_24(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> mat4x4<f32> {
  return mat4x4<f32>(tint_symbol_12(buffer, (offset + 0u)), tint_symbol_12(buffer, (offset + 16u)), tint_symbol_12(buffer, (offset + 32u)), tint_symbol_12(buffer, (offset + 48u)));
}

fn tint_symbol_25(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> mat2x2<f16> {
  return mat2x2<f16>(tint_symbol_7(buffer, (offset + 0u)), tint_symbol_7(buffer, (offset + 4u)));
}

fn tint_symbol_26(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> mat2x3<f16> {
  return mat2x3<f16>(tint_symbol_11(buffer, (offset + 0u)), tint_symbol_11(buffer, (offset + 8u)));
}

fn tint_symbol_27(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> mat2x4<f16> {
  return mat2x4<f16>(tint_symbol_15(buffer, (offset + 0u)), tint_symbol_15(buffer, (offset + 8u)));
}

fn tint_symbol_28(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> mat3x2<f16> {
  return mat3x2<f16>(tint_symbol_7(buffer, (offset + 0u)), tint_symbol_7(buffer, (offset + 4u)), tint_symbol_7(buffer, (offset + 8u)));
}

fn tint_symbol_29(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> mat3x3<f16> {
  return mat3x3<f16>(tint_symbol_11(buffer, (offset + 0u)), tint_symbol_11(buffer, (offset + 8u)), tint_symbol_11(buffer, (offset + 16u)));
}

fn tint_symbol_30(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> mat3x4<f16> {
  return mat3x4<f16>(tint_symbol_15(buffer, (offset + 0u)), tint_symbol_15(buffer, (offset + 8u)), tint_symbol_15(buffer, (offset + 16u)));
}

fn tint_symbol_31(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> mat4x2<f16> {
  return mat4x2<f16>(tint_symbol_7(buffer, (offset + 0u)), tint_symbol_7(buffer, (offset + 4u)), tint_symbol_7(buffer, (offset + 8u)), tint_symbol_7(buffer, (offset + 12u)));
}

fn tint_symbol_32(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> mat4x3<f16> {
  return mat4x3<f16>(tint_symbol_11(buffer, (offset + 0u)), tint_symbol_11(buffer, (offset + 8u)), tint_symbol_11(buffer, (offset + 16u)), tint_symbol_11(buffer, (offset + 24u)));
}

fn tint_symbol_33(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> mat4x4<f16> {
  return mat4x4<f16>(tint_symbol_15(buffer, (offset + 0u)), tint_symbol_15(buffer, (offset + 8u)), tint_symbol_15(buffer, (offset + 16u)), tint_symbol_15(buffer, (offset + 24u)));
}

fn tint_symbol_34(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> array<vec3<f32>, 2u> {
  var arr : array<vec3<f32>, 2u>;
  for(var i = 0u; (i < 2u); i = (i + 1u)) {
    arr[i] = tint_symbol_8(buffer, (offset + (i * 16u)));
  }
  return arr;
}

fn tint_symbol_35(@internal(disable_validation__function_parameter) buffer : ptr<uniform, UB>, offset : u32) -> array<mat4x2<f16>, 2u> {
  var arr_1 : array<mat4x2<f16>, 2u>;
  for(var i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
    arr_1[i_1] = tint_symbol_31(buffer, (offset + (i_1 * 16u)));
  }
  return arr_1;
}

@compute @workgroup_size(1)
fn main() {
  var scalar_f32 : f32 = tint_symbol(&(ub), 0u);
  var scalar_i32 : i32 = tint_symbol_1(&(ub), 4u);
  var scalar_u32 : u32 = tint_symbol_2(&(ub), 8u);
  var scalar_f16 : f16 = tint_symbol_3(&(ub), 12u);
  var vec2_f32 : vec2<f32> = tint_symbol_4(&(ub), 16u);
  var vec2_i32 : vec2<i32> = tint_symbol_5(&(ub), 24u);
  var vec2_u32 : vec2<u32> = tint_symbol_6(&(ub), 32u);
  var vec2_f16 : vec2<f16> = tint_symbol_7(&(ub), 40u);
  var vec3_f32 : vec3<f32> = tint_symbol_8(&(ub), 48u);
  var vec3_i32 : vec3<i32> = tint_symbol_9(&(ub), 64u);
  var vec3_u32 : vec3<u32> = tint_symbol_10(&(ub), 80u);
  var vec3_f16 : vec3<f16> = tint_symbol_11(&(ub), 96u);
  var vec4_f32 : vec4<f32> = tint_symbol_12(&(ub), 112u);
  var vec4_i32 : vec4<i32> = tint_symbol_13(&(ub), 128u);
  var vec4_u32 : vec4<u32> = tint_symbol_14(&(ub), 144u);
  var vec4_f16 : vec4<f16> = tint_symbol_15(&(ub), 160u);
  var mat2x2_f32 : mat2x2<f32> = tint_symbol_16(&(ub), 168u);
  var mat2x3_f32 : mat2x3<f32> = tint_symbol_17(&(ub), 192u);
  var mat2x4_f32 : mat2x4<f32> = tint_symbol_18(&(ub), 224u);
  var mat3x2_f32 : mat3x2<f32> = tint_symbol_19(&(ub), 256u);
  var mat3x3_f32 : mat3x3<f32> = tint_symbol_20(&(ub), 288u);
  var mat3x4_f32 : mat3x4<f32> = tint_symbol_21(&(ub), 336u);
  var mat4x2_f32 : mat4x2<f32> = tint_symbol_22(&(ub), 384u);
  var mat4x3_f32 : mat4x3<f32> = tint_symbol_23(&(ub), 416u);
  var mat4x4_f32 : mat4x4<f32> = tint_symbol_24(&(ub), 480u);
  var mat2x2_f16 : mat2x2<f16> = tint_symbol_25(&(ub), 544u);
  var mat2x3_f16 : mat2x3<f16> = tint_symbol_26(&(ub), 552u);
  var mat2x4_f16 : mat2x4<f16> = tint_symbol_27(&(ub), 568u);
  var mat3x2_f16 : mat3x2<f16> = tint_symbol_28(&(ub), 584u);
  var mat3x3_f16 : mat3x3<f16> = tint_symbol_29(&(ub), 600u);
  var mat3x4_f16 : mat3x4<f16> = tint_symbol_30(&(ub), 624u);
  var mat4x2_f16 : mat4x2<f16> = tint_symbol_31(&(ub), 648u);
  var mat4x3_f16 : mat4x3<f16> = tint_symbol_32(&(ub), 664u);
  var mat4x4_f16 : mat4x4<f16> = tint_symbol_33(&(ub), 696u);
  var arr2_vec3_f32 : array<vec3<f32>, 2> = tint_symbol_34(&(ub), 736u);
  var arr2_mat4x2_f16 : array<mat4x2<f16>, 2> = tint_symbol_35(&(ub), 768u);
}

@group(0) @binding(0) var<uniform> ub : UB;

struct UB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_mat4x2_f16 : array<mat4x2<f16>, 2>,
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, SB_BasicStore) {
    auto* src = R"(
enable f16;

struct SB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_mat4x2_f16 : array<mat4x2<f16>, 2>,
};

@group(0) @binding(0) var<storage, read_write> sb : SB;

@compute @workgroup_size(1)
fn main() {
  sb.scalar_f32 = f32();
  sb.scalar_i32 = i32();
  sb.scalar_u32 = u32();
  sb.scalar_f16 = f16();
  sb.vec2_f32 = vec2<f32>();
  sb.vec2_i32 = vec2<i32>();
  sb.vec2_u32 = vec2<u32>();
  sb.vec2_f16 = vec2<f16>();
  sb.vec3_f32 = vec3<f32>();
  sb.vec3_i32 = vec3<i32>();
  sb.vec3_u32 = vec3<u32>();
  sb.vec3_f16 = vec3<f16>();
  sb.vec4_f32 = vec4<f32>();
  sb.vec4_i32 = vec4<i32>();
  sb.vec4_u32 = vec4<u32>();
  sb.vec4_f16 = vec4<f16>();
  sb.mat2x2_f32 = mat2x2<f32>();
  sb.mat2x3_f32 = mat2x3<f32>();
  sb.mat2x4_f32 = mat2x4<f32>();
  sb.mat3x2_f32 = mat3x2<f32>();
  sb.mat3x3_f32 = mat3x3<f32>();
  sb.mat3x4_f32 = mat3x4<f32>();
  sb.mat4x2_f32 = mat4x2<f32>();
  sb.mat4x3_f32 = mat4x3<f32>();
  sb.mat4x4_f32 = mat4x4<f32>();
  sb.mat2x2_f16 = mat2x2<f16>();
  sb.mat2x3_f16 = mat2x3<f16>();
  sb.mat2x4_f16 = mat2x4<f16>();
  sb.mat3x2_f16 = mat3x2<f16>();
  sb.mat3x3_f16 = mat3x3<f16>();
  sb.mat3x4_f16 = mat3x4<f16>();
  sb.mat4x2_f16 = mat4x2<f16>();
  sb.mat4x3_f16 = mat4x3<f16>();
  sb.mat4x4_f16 = mat4x4<f16>();
  sb.arr2_vec3_f32 = array<vec3<f32>, 2>();
  sb.arr2_mat4x2_f16 = array<mat4x2<f16>, 2>();
}
)";

    auto* expect = R"(
enable f16;

struct SB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_mat4x2_f16 : array<mat4x2<f16>, 2>,
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

@internal(intrinsic_store_storage_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : f32)

@internal(intrinsic_store_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_1(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : i32)

@internal(intrinsic_store_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_2(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : u32)

@internal(intrinsic_store_storage_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_3(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : f16)

@internal(intrinsic_store_storage_vec2_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_4(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec2<f32>)

@internal(intrinsic_store_storage_vec2_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_5(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec2<i32>)

@internal(intrinsic_store_storage_vec2_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_6(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec2<u32>)

@internal(intrinsic_store_storage_vec2_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_7(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec2<f16>)

@internal(intrinsic_store_storage_vec3_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_8(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec3<f32>)

@internal(intrinsic_store_storage_vec3_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_9(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec3<i32>)

@internal(intrinsic_store_storage_vec3_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_10(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec3<u32>)

@internal(intrinsic_store_storage_vec3_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_11(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec3<f16>)

@internal(intrinsic_store_storage_vec4_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_12(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec4<f32>)

@internal(intrinsic_store_storage_vec4_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_13(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec4<i32>)

@internal(intrinsic_store_storage_vec4_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_14(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec4<u32>)

@internal(intrinsic_store_storage_vec4_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_15(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec4<f16>)

fn tint_symbol_16(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat2x2<f32>) {
  tint_symbol_4(buffer, (offset + 0u), value[0u]);
  tint_symbol_4(buffer, (offset + 8u), value[1u]);
}

fn tint_symbol_17(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat2x3<f32>) {
  tint_symbol_8(buffer, (offset + 0u), value[0u]);
  tint_symbol_8(buffer, (offset + 16u), value[1u]);
}

fn tint_symbol_18(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat2x4<f32>) {
  tint_symbol_12(buffer, (offset + 0u), value[0u]);
  tint_symbol_12(buffer, (offset + 16u), value[1u]);
}

fn tint_symbol_19(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat3x2<f32>) {
  tint_symbol_4(buffer, (offset + 0u), value[0u]);
  tint_symbol_4(buffer, (offset + 8u), value[1u]);
  tint_symbol_4(buffer, (offset + 16u), value[2u]);
}

fn tint_symbol_20(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat3x3<f32>) {
  tint_symbol_8(buffer, (offset + 0u), value[0u]);
  tint_symbol_8(buffer, (offset + 16u), value[1u]);
  tint_symbol_8(buffer, (offset + 32u), value[2u]);
}

fn tint_symbol_21(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat3x4<f32>) {
  tint_symbol_12(buffer, (offset + 0u), value[0u]);
  tint_symbol_12(buffer, (offset + 16u), value[1u]);
  tint_symbol_12(buffer, (offset + 32u), value[2u]);
}

fn tint_symbol_22(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat4x2<f32>) {
  tint_symbol_4(buffer, (offset + 0u), value[0u]);
  tint_symbol_4(buffer, (offset + 8u), value[1u]);
  tint_symbol_4(buffer, (offset + 16u), value[2u]);
  tint_symbol_4(buffer, (offset + 24u), value[3u]);
}

fn tint_symbol_23(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat4x3<f32>) {
  tint_symbol_8(buffer, (offset + 0u), value[0u]);
  tint_symbol_8(buffer, (offset + 16u), value[1u]);
  tint_symbol_8(buffer, (offset + 32u), value[2u]);
  tint_symbol_8(buffer, (offset + 48u), value[3u]);
}

fn tint_symbol_24(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat4x4<f32>) {
  tint_symbol_12(buffer, (offset + 0u), value[0u]);
  tint_symbol_12(buffer, (offset + 16u), value[1u]);
  tint_symbol_12(buffer, (offset + 32u), value[2u]);
  tint_symbol_12(buffer, (offset + 48u), value[3u]);
}

fn tint_symbol_25(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat2x2<f16>) {
  tint_symbol_7(buffer, (offset + 0u), value[0u]);
  tint_symbol_7(buffer, (offset + 4u), value[1u]);
}

fn tint_symbol_26(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat2x3<f16>) {
  tint_symbol_11(buffer, (offset + 0u), value[0u]);
  tint_symbol_11(buffer, (offset + 8u), value[1u]);
}

fn tint_symbol_27(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat2x4<f16>) {
  tint_symbol_15(buffer, (offset + 0u), value[0u]);
  tint_symbol_15(buffer, (offset + 8u), value[1u]);
}

fn tint_symbol_28(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat3x2<f16>) {
  tint_symbol_7(buffer, (offset + 0u), value[0u]);
  tint_symbol_7(buffer, (offset + 4u), value[1u]);
  tint_symbol_7(buffer, (offset + 8u), value[2u]);
}

fn tint_symbol_29(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat3x3<f16>) {
  tint_symbol_11(buffer, (offset + 0u), value[0u]);
  tint_symbol_11(buffer, (offset + 8u), value[1u]);
  tint_symbol_11(buffer, (offset + 16u), value[2u]);
}

fn tint_symbol_30(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat3x4<f16>) {
  tint_symbol_15(buffer, (offset + 0u), value[0u]);
  tint_symbol_15(buffer, (offset + 8u), value[1u]);
  tint_symbol_15(buffer, (offset + 16u), value[2u]);
}

fn tint_symbol_31(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat4x2<f16>) {
  tint_symbol_7(buffer, (offset + 0u), value[0u]);
  tint_symbol_7(buffer, (offset + 4u), value[1u]);
  tint_symbol_7(buffer, (offset + 8u), value[2u]);
  tint_symbol_7(buffer, (offset + 12u), value[3u]);
}

fn tint_symbol_32(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat4x3<f16>) {
  tint_symbol_11(buffer, (offset + 0u), value[0u]);
  tint_symbol_11(buffer, (offset + 8u), value[1u]);
  tint_symbol_11(buffer, (offset + 16u), value[2u]);
  tint_symbol_11(buffer, (offset + 24u), value[3u]);
}

fn tint_symbol_33(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat4x4<f16>) {
  tint_symbol_15(buffer, (offset + 0u), value[0u]);
  tint_symbol_15(buffer, (offset + 8u), value[1u]);
  tint_symbol_15(buffer, (offset + 16u), value[2u]);
  tint_symbol_15(buffer, (offset + 24u), value[3u]);
}

fn tint_symbol_34(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : array<vec3<f32>, 2u>) {
  var array_1 = value;
  for(var i = 0u; (i < 2u); i = (i + 1u)) {
    tint_symbol_8(buffer, (offset + (i * 16u)), array_1[i]);
  }
}

fn tint_symbol_35(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : array<mat4x2<f16>, 2u>) {
  var array_2 = value;
  for(var i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
    tint_symbol_31(buffer, (offset + (i_1 * 16u)), array_2[i_1]);
  }
}

@compute @workgroup_size(1)
fn main() {
  tint_symbol(&(sb), 0u, f32());
  tint_symbol_1(&(sb), 4u, i32());
  tint_symbol_2(&(sb), 8u, u32());
  tint_symbol_3(&(sb), 12u, f16());
  tint_symbol_4(&(sb), 16u, vec2<f32>());
  tint_symbol_5(&(sb), 24u, vec2<i32>());
  tint_symbol_6(&(sb), 32u, vec2<u32>());
  tint_symbol_7(&(sb), 40u, vec2<f16>());
  tint_symbol_8(&(sb), 48u, vec3<f32>());
  tint_symbol_9(&(sb), 64u, vec3<i32>());
  tint_symbol_10(&(sb), 80u, vec3<u32>());
  tint_symbol_11(&(sb), 96u, vec3<f16>());
  tint_symbol_12(&(sb), 112u, vec4<f32>());
  tint_symbol_13(&(sb), 128u, vec4<i32>());
  tint_symbol_14(&(sb), 144u, vec4<u32>());
  tint_symbol_15(&(sb), 160u, vec4<f16>());
  tint_symbol_16(&(sb), 168u, mat2x2<f32>());
  tint_symbol_17(&(sb), 192u, mat2x3<f32>());
  tint_symbol_18(&(sb), 224u, mat2x4<f32>());
  tint_symbol_19(&(sb), 256u, mat3x2<f32>());
  tint_symbol_20(&(sb), 288u, mat3x3<f32>());
  tint_symbol_21(&(sb), 336u, mat3x4<f32>());
  tint_symbol_22(&(sb), 384u, mat4x2<f32>());
  tint_symbol_23(&(sb), 416u, mat4x3<f32>());
  tint_symbol_24(&(sb), 480u, mat4x4<f32>());
  tint_symbol_25(&(sb), 544u, mat2x2<f16>());
  tint_symbol_26(&(sb), 552u, mat2x3<f16>());
  tint_symbol_27(&(sb), 568u, mat2x4<f16>());
  tint_symbol_28(&(sb), 584u, mat3x2<f16>());
  tint_symbol_29(&(sb), 600u, mat3x3<f16>());
  tint_symbol_30(&(sb), 624u, mat3x4<f16>());
  tint_symbol_31(&(sb), 648u, mat4x2<f16>());
  tint_symbol_32(&(sb), 664u, mat4x3<f16>());
  tint_symbol_33(&(sb), 696u, mat4x4<f16>());
  tint_symbol_34(&(sb), 736u, array<vec3<f32>, 2>());
  tint_symbol_35(&(sb), 768u, array<mat4x2<f16>, 2>());
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, SB_BasicStore_OutOfOrder) {
    auto* src = R"(
enable f16;

@compute @workgroup_size(1)
fn main() {
  sb.scalar_f32 = f32();
  sb.scalar_i32 = i32();
  sb.scalar_u32 = u32();
  sb.scalar_f16 = f16();
  sb.vec2_f32 = vec2<f32>();
  sb.vec2_i32 = vec2<i32>();
  sb.vec2_u32 = vec2<u32>();
  sb.vec2_f16 = vec2<f16>();
  sb.vec3_f32 = vec3<f32>();
  sb.vec3_i32 = vec3<i32>();
  sb.vec3_u32 = vec3<u32>();
  sb.vec3_f16 = vec3<f16>();
  sb.vec4_f32 = vec4<f32>();
  sb.vec4_i32 = vec4<i32>();
  sb.vec4_u32 = vec4<u32>();
  sb.vec4_f16 = vec4<f16>();
  sb.mat2x2_f32 = mat2x2<f32>();
  sb.mat2x3_f32 = mat2x3<f32>();
  sb.mat2x4_f32 = mat2x4<f32>();
  sb.mat3x2_f32 = mat3x2<f32>();
  sb.mat3x3_f32 = mat3x3<f32>();
  sb.mat3x4_f32 = mat3x4<f32>();
  sb.mat4x2_f32 = mat4x2<f32>();
  sb.mat4x3_f32 = mat4x3<f32>();
  sb.mat4x4_f32 = mat4x4<f32>();
  sb.mat2x2_f16 = mat2x2<f16>();
  sb.mat2x3_f16 = mat2x3<f16>();
  sb.mat2x4_f16 = mat2x4<f16>();
  sb.mat3x2_f16 = mat3x2<f16>();
  sb.mat3x3_f16 = mat3x3<f16>();
  sb.mat3x4_f16 = mat3x4<f16>();
  sb.mat4x2_f16 = mat4x2<f16>();
  sb.mat4x3_f16 = mat4x3<f16>();
  sb.mat4x4_f16 = mat4x4<f16>();
  sb.arr2_vec3_f32 = array<vec3<f32>, 2>();
  sb.arr2_mat4x2_f16 = array<mat4x2<f16>, 2>();
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_mat4x2_f16 : array<mat4x2<f16>, 2>,
};
)";

    auto* expect = R"(
enable f16;

@internal(intrinsic_store_storage_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : f32)

@internal(intrinsic_store_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_1(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : i32)

@internal(intrinsic_store_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_2(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : u32)

@internal(intrinsic_store_storage_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_3(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : f16)

@internal(intrinsic_store_storage_vec2_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_4(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec2<f32>)

@internal(intrinsic_store_storage_vec2_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_5(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec2<i32>)

@internal(intrinsic_store_storage_vec2_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_6(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec2<u32>)

@internal(intrinsic_store_storage_vec2_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_7(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec2<f16>)

@internal(intrinsic_store_storage_vec3_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_8(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec3<f32>)

@internal(intrinsic_store_storage_vec3_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_9(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec3<i32>)

@internal(intrinsic_store_storage_vec3_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_10(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec3<u32>)

@internal(intrinsic_store_storage_vec3_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_11(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec3<f16>)

@internal(intrinsic_store_storage_vec4_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_12(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec4<f32>)

@internal(intrinsic_store_storage_vec4_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_13(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec4<i32>)

@internal(intrinsic_store_storage_vec4_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_14(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec4<u32>)

@internal(intrinsic_store_storage_vec4_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_15(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec4<f16>)

fn tint_symbol_16(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat2x2<f32>) {
  tint_symbol_4(buffer, (offset + 0u), value[0u]);
  tint_symbol_4(buffer, (offset + 8u), value[1u]);
}

fn tint_symbol_17(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat2x3<f32>) {
  tint_symbol_8(buffer, (offset + 0u), value[0u]);
  tint_symbol_8(buffer, (offset + 16u), value[1u]);
}

fn tint_symbol_18(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat2x4<f32>) {
  tint_symbol_12(buffer, (offset + 0u), value[0u]);
  tint_symbol_12(buffer, (offset + 16u), value[1u]);
}

fn tint_symbol_19(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat3x2<f32>) {
  tint_symbol_4(buffer, (offset + 0u), value[0u]);
  tint_symbol_4(buffer, (offset + 8u), value[1u]);
  tint_symbol_4(buffer, (offset + 16u), value[2u]);
}

fn tint_symbol_20(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat3x3<f32>) {
  tint_symbol_8(buffer, (offset + 0u), value[0u]);
  tint_symbol_8(buffer, (offset + 16u), value[1u]);
  tint_symbol_8(buffer, (offset + 32u), value[2u]);
}

fn tint_symbol_21(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat3x4<f32>) {
  tint_symbol_12(buffer, (offset + 0u), value[0u]);
  tint_symbol_12(buffer, (offset + 16u), value[1u]);
  tint_symbol_12(buffer, (offset + 32u), value[2u]);
}

fn tint_symbol_22(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat4x2<f32>) {
  tint_symbol_4(buffer, (offset + 0u), value[0u]);
  tint_symbol_4(buffer, (offset + 8u), value[1u]);
  tint_symbol_4(buffer, (offset + 16u), value[2u]);
  tint_symbol_4(buffer, (offset + 24u), value[3u]);
}

fn tint_symbol_23(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat4x3<f32>) {
  tint_symbol_8(buffer, (offset + 0u), value[0u]);
  tint_symbol_8(buffer, (offset + 16u), value[1u]);
  tint_symbol_8(buffer, (offset + 32u), value[2u]);
  tint_symbol_8(buffer, (offset + 48u), value[3u]);
}

fn tint_symbol_24(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat4x4<f32>) {
  tint_symbol_12(buffer, (offset + 0u), value[0u]);
  tint_symbol_12(buffer, (offset + 16u), value[1u]);
  tint_symbol_12(buffer, (offset + 32u), value[2u]);
  tint_symbol_12(buffer, (offset + 48u), value[3u]);
}

fn tint_symbol_25(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat2x2<f16>) {
  tint_symbol_7(buffer, (offset + 0u), value[0u]);
  tint_symbol_7(buffer, (offset + 4u), value[1u]);
}

fn tint_symbol_26(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat2x3<f16>) {
  tint_symbol_11(buffer, (offset + 0u), value[0u]);
  tint_symbol_11(buffer, (offset + 8u), value[1u]);
}

fn tint_symbol_27(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat2x4<f16>) {
  tint_symbol_15(buffer, (offset + 0u), value[0u]);
  tint_symbol_15(buffer, (offset + 8u), value[1u]);
}

fn tint_symbol_28(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat3x2<f16>) {
  tint_symbol_7(buffer, (offset + 0u), value[0u]);
  tint_symbol_7(buffer, (offset + 4u), value[1u]);
  tint_symbol_7(buffer, (offset + 8u), value[2u]);
}

fn tint_symbol_29(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat3x3<f16>) {
  tint_symbol_11(buffer, (offset + 0u), value[0u]);
  tint_symbol_11(buffer, (offset + 8u), value[1u]);
  tint_symbol_11(buffer, (offset + 16u), value[2u]);
}

fn tint_symbol_30(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat3x4<f16>) {
  tint_symbol_15(buffer, (offset + 0u), value[0u]);
  tint_symbol_15(buffer, (offset + 8u), value[1u]);
  tint_symbol_15(buffer, (offset + 16u), value[2u]);
}

fn tint_symbol_31(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat4x2<f16>) {
  tint_symbol_7(buffer, (offset + 0u), value[0u]);
  tint_symbol_7(buffer, (offset + 4u), value[1u]);
  tint_symbol_7(buffer, (offset + 8u), value[2u]);
  tint_symbol_7(buffer, (offset + 12u), value[3u]);
}

fn tint_symbol_32(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat4x3<f16>) {
  tint_symbol_11(buffer, (offset + 0u), value[0u]);
  tint_symbol_11(buffer, (offset + 8u), value[1u]);
  tint_symbol_11(buffer, (offset + 16u), value[2u]);
  tint_symbol_11(buffer, (offset + 24u), value[3u]);
}

fn tint_symbol_33(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat4x4<f16>) {
  tint_symbol_15(buffer, (offset + 0u), value[0u]);
  tint_symbol_15(buffer, (offset + 8u), value[1u]);
  tint_symbol_15(buffer, (offset + 16u), value[2u]);
  tint_symbol_15(buffer, (offset + 24u), value[3u]);
}

fn tint_symbol_34(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : array<vec3<f32>, 2u>) {
  var array_1 = value;
  for(var i = 0u; (i < 2u); i = (i + 1u)) {
    tint_symbol_8(buffer, (offset + (i * 16u)), array_1[i]);
  }
}

fn tint_symbol_35(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : array<mat4x2<f16>, 2u>) {
  var array_2 = value;
  for(var i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
    tint_symbol_31(buffer, (offset + (i_1 * 16u)), array_2[i_1]);
  }
}

@compute @workgroup_size(1)
fn main() {
  tint_symbol(&(sb), 0u, f32());
  tint_symbol_1(&(sb), 4u, i32());
  tint_symbol_2(&(sb), 8u, u32());
  tint_symbol_3(&(sb), 12u, f16());
  tint_symbol_4(&(sb), 16u, vec2<f32>());
  tint_symbol_5(&(sb), 24u, vec2<i32>());
  tint_symbol_6(&(sb), 32u, vec2<u32>());
  tint_symbol_7(&(sb), 40u, vec2<f16>());
  tint_symbol_8(&(sb), 48u, vec3<f32>());
  tint_symbol_9(&(sb), 64u, vec3<i32>());
  tint_symbol_10(&(sb), 80u, vec3<u32>());
  tint_symbol_11(&(sb), 96u, vec3<f16>());
  tint_symbol_12(&(sb), 112u, vec4<f32>());
  tint_symbol_13(&(sb), 128u, vec4<i32>());
  tint_symbol_14(&(sb), 144u, vec4<u32>());
  tint_symbol_15(&(sb), 160u, vec4<f16>());
  tint_symbol_16(&(sb), 168u, mat2x2<f32>());
  tint_symbol_17(&(sb), 192u, mat2x3<f32>());
  tint_symbol_18(&(sb), 224u, mat2x4<f32>());
  tint_symbol_19(&(sb), 256u, mat3x2<f32>());
  tint_symbol_20(&(sb), 288u, mat3x3<f32>());
  tint_symbol_21(&(sb), 336u, mat3x4<f32>());
  tint_symbol_22(&(sb), 384u, mat4x2<f32>());
  tint_symbol_23(&(sb), 416u, mat4x3<f32>());
  tint_symbol_24(&(sb), 480u, mat4x4<f32>());
  tint_symbol_25(&(sb), 544u, mat2x2<f16>());
  tint_symbol_26(&(sb), 552u, mat2x3<f16>());
  tint_symbol_27(&(sb), 568u, mat2x4<f16>());
  tint_symbol_28(&(sb), 584u, mat3x2<f16>());
  tint_symbol_29(&(sb), 600u, mat3x3<f16>());
  tint_symbol_30(&(sb), 624u, mat3x4<f16>());
  tint_symbol_31(&(sb), 648u, mat4x2<f16>());
  tint_symbol_32(&(sb), 664u, mat4x3<f16>());
  tint_symbol_33(&(sb), 696u, mat4x4<f16>());
  tint_symbol_34(&(sb), 736u, array<vec3<f32>, 2>());
  tint_symbol_35(&(sb), 768u, array<mat4x2<f16>, 2>());
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_mat4x2_f16 : array<mat4x2<f16>, 2>,
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, LoadStructure) {
    auto* src = R"(
enable f16;

struct SB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_mat4x2_f16 : array<mat4x2<f16>, 2>,
};

@group(0) @binding(0) var<storage, read_write> sb : SB;

@compute @workgroup_size(1)
fn main() {
  var x : SB = sb;
}
)";

    auto* expect = R"(
enable f16;

struct SB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_mat4x2_f16 : array<mat4x2<f16>, 2>,
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

@internal(intrinsic_load_storage_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_1(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> f32

@internal(intrinsic_load_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_2(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> i32

@internal(intrinsic_load_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_3(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> u32

@internal(intrinsic_load_storage_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_4(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> f16

@internal(intrinsic_load_storage_vec2_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_5(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec2<f32>

@internal(intrinsic_load_storage_vec2_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_6(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec2<i32>

@internal(intrinsic_load_storage_vec2_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_7(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec2<u32>

@internal(intrinsic_load_storage_vec2_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_8(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec2<f16>

@internal(intrinsic_load_storage_vec3_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_9(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec3<f32>

@internal(intrinsic_load_storage_vec3_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_10(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec3<i32>

@internal(intrinsic_load_storage_vec3_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_11(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec3<u32>

@internal(intrinsic_load_storage_vec3_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_12(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec3<f16>

@internal(intrinsic_load_storage_vec4_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_13(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec4<f32>

@internal(intrinsic_load_storage_vec4_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_14(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec4<i32>

@internal(intrinsic_load_storage_vec4_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_15(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec4<u32>

@internal(intrinsic_load_storage_vec4_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_16(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec4<f16>

fn tint_symbol_17(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat2x2<f32> {
  return mat2x2<f32>(tint_symbol_5(buffer, (offset + 0u)), tint_symbol_5(buffer, (offset + 8u)));
}

fn tint_symbol_18(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat2x3<f32> {
  return mat2x3<f32>(tint_symbol_9(buffer, (offset + 0u)), tint_symbol_9(buffer, (offset + 16u)));
}

fn tint_symbol_19(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat2x4<f32> {
  return mat2x4<f32>(tint_symbol_13(buffer, (offset + 0u)), tint_symbol_13(buffer, (offset + 16u)));
}

fn tint_symbol_20(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat3x2<f32> {
  return mat3x2<f32>(tint_symbol_5(buffer, (offset + 0u)), tint_symbol_5(buffer, (offset + 8u)), tint_symbol_5(buffer, (offset + 16u)));
}

fn tint_symbol_21(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat3x3<f32> {
  return mat3x3<f32>(tint_symbol_9(buffer, (offset + 0u)), tint_symbol_9(buffer, (offset + 16u)), tint_symbol_9(buffer, (offset + 32u)));
}

fn tint_symbol_22(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat3x4<f32> {
  return mat3x4<f32>(tint_symbol_13(buffer, (offset + 0u)), tint_symbol_13(buffer, (offset + 16u)), tint_symbol_13(buffer, (offset + 32u)));
}

fn tint_symbol_23(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat4x2<f32> {
  return mat4x2<f32>(tint_symbol_5(buffer, (offset + 0u)), tint_symbol_5(buffer, (offset + 8u)), tint_symbol_5(buffer, (offset + 16u)), tint_symbol_5(buffer, (offset + 24u)));
}

fn tint_symbol_24(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat4x3<f32> {
  return mat4x3<f32>(tint_symbol_9(buffer, (offset + 0u)), tint_symbol_9(buffer, (offset + 16u)), tint_symbol_9(buffer, (offset + 32u)), tint_symbol_9(buffer, (offset + 48u)));
}

fn tint_symbol_25(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat4x4<f32> {
  return mat4x4<f32>(tint_symbol_13(buffer, (offset + 0u)), tint_symbol_13(buffer, (offset + 16u)), tint_symbol_13(buffer, (offset + 32u)), tint_symbol_13(buffer, (offset + 48u)));
}

fn tint_symbol_26(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat2x2<f16> {
  return mat2x2<f16>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 4u)));
}

fn tint_symbol_27(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat2x3<f16> {
  return mat2x3<f16>(tint_symbol_12(buffer, (offset + 0u)), tint_symbol_12(buffer, (offset + 8u)));
}

fn tint_symbol_28(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat2x4<f16> {
  return mat2x4<f16>(tint_symbol_16(buffer, (offset + 0u)), tint_symbol_16(buffer, (offset + 8u)));
}

fn tint_symbol_29(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat3x2<f16> {
  return mat3x2<f16>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 4u)), tint_symbol_8(buffer, (offset + 8u)));
}

fn tint_symbol_30(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat3x3<f16> {
  return mat3x3<f16>(tint_symbol_12(buffer, (offset + 0u)), tint_symbol_12(buffer, (offset + 8u)), tint_symbol_12(buffer, (offset + 16u)));
}

fn tint_symbol_31(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat3x4<f16> {
  return mat3x4<f16>(tint_symbol_16(buffer, (offset + 0u)), tint_symbol_16(buffer, (offset + 8u)), tint_symbol_16(buffer, (offset + 16u)));
}

fn tint_symbol_32(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat4x2<f16> {
  return mat4x2<f16>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 4u)), tint_symbol_8(buffer, (offset + 8u)), tint_symbol_8(buffer, (offset + 12u)));
}

fn tint_symbol_33(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat4x3<f16> {
  return mat4x3<f16>(tint_symbol_12(buffer, (offset + 0u)), tint_symbol_12(buffer, (offset + 8u)), tint_symbol_12(buffer, (offset + 16u)), tint_symbol_12(buffer, (offset + 24u)));
}

fn tint_symbol_34(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat4x4<f16> {
  return mat4x4<f16>(tint_symbol_16(buffer, (offset + 0u)), tint_symbol_16(buffer, (offset + 8u)), tint_symbol_16(buffer, (offset + 16u)), tint_symbol_16(buffer, (offset + 24u)));
}

fn tint_symbol_35(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> array<vec3<f32>, 2u> {
  var arr : array<vec3<f32>, 2u>;
  for(var i = 0u; (i < 2u); i = (i + 1u)) {
    arr[i] = tint_symbol_9(buffer, (offset + (i * 16u)));
  }
  return arr;
}

fn tint_symbol_36(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> array<mat4x2<f16>, 2u> {
  var arr_1 : array<mat4x2<f16>, 2u>;
  for(var i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
    arr_1[i_1] = tint_symbol_32(buffer, (offset + (i_1 * 16u)));
  }
  return arr_1;
}

fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> SB {
  return SB(tint_symbol_1(buffer, (offset + 0u)), tint_symbol_2(buffer, (offset + 4u)), tint_symbol_3(buffer, (offset + 8u)), tint_symbol_4(buffer, (offset + 12u)), tint_symbol_5(buffer, (offset + 16u)), tint_symbol_6(buffer, (offset + 24u)), tint_symbol_7(buffer, (offset + 32u)), tint_symbol_8(buffer, (offset + 40u)), tint_symbol_9(buffer, (offset + 48u)), tint_symbol_10(buffer, (offset + 64u)), tint_symbol_11(buffer, (offset + 80u)), tint_symbol_12(buffer, (offset + 96u)), tint_symbol_13(buffer, (offset + 112u)), tint_symbol_14(buffer, (offset + 128u)), tint_symbol_15(buffer, (offset + 144u)), tint_symbol_16(buffer, (offset + 160u)), tint_symbol_17(buffer, (offset + 168u)), tint_symbol_18(buffer, (offset + 192u)), tint_symbol_19(buffer, (offset + 224u)), tint_symbol_20(buffer, (offset + 256u)), tint_symbol_21(buffer, (offset + 288u)), tint_symbol_22(buffer, (offset + 336u)), tint_symbol_23(buffer, (offset + 384u)), tint_symbol_24(buffer, (offset + 416u)), tint_symbol_25(buffer, (offset + 480u)), tint_symbol_26(buffer, (offset + 544u)), tint_symbol_27(buffer, (offset + 552u)), tint_symbol_28(buffer, (offset + 568u)), tint_symbol_29(buffer, (offset + 584u)), tint_symbol_30(buffer, (offset + 600u)), tint_symbol_31(buffer, (offset + 624u)), tint_symbol_32(buffer, (offset + 648u)), tint_symbol_33(buffer, (offset + 664u)), tint_symbol_34(buffer, (offset + 696u)), tint_symbol_35(buffer, (offset + 736u)), tint_symbol_36(buffer, (offset + 768u)));
}

@compute @workgroup_size(1)
fn main() {
  var x : SB = tint_symbol(&(sb), 0u);
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, LoadStructure_OutOfOrder) {
    auto* src = R"(
enable f16;

@compute @workgroup_size(1)
fn main() {
  var x : SB = sb;
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_mat4x2_f16 : array<mat4x2<f16>, 2>,
};
)";

    auto* expect = R"(
enable f16;

@internal(intrinsic_load_storage_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_1(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> f32

@internal(intrinsic_load_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_2(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> i32

@internal(intrinsic_load_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_3(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> u32

@internal(intrinsic_load_storage_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_4(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> f16

@internal(intrinsic_load_storage_vec2_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_5(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec2<f32>

@internal(intrinsic_load_storage_vec2_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_6(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec2<i32>

@internal(intrinsic_load_storage_vec2_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_7(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec2<u32>

@internal(intrinsic_load_storage_vec2_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_8(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec2<f16>

@internal(intrinsic_load_storage_vec3_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_9(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec3<f32>

@internal(intrinsic_load_storage_vec3_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_10(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec3<i32>

@internal(intrinsic_load_storage_vec3_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_11(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec3<u32>

@internal(intrinsic_load_storage_vec3_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_12(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec3<f16>

@internal(intrinsic_load_storage_vec4_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_13(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec4<f32>

@internal(intrinsic_load_storage_vec4_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_14(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec4<i32>

@internal(intrinsic_load_storage_vec4_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_15(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec4<u32>

@internal(intrinsic_load_storage_vec4_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_16(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> vec4<f16>

fn tint_symbol_17(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat2x2<f32> {
  return mat2x2<f32>(tint_symbol_5(buffer, (offset + 0u)), tint_symbol_5(buffer, (offset + 8u)));
}

fn tint_symbol_18(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat2x3<f32> {
  return mat2x3<f32>(tint_symbol_9(buffer, (offset + 0u)), tint_symbol_9(buffer, (offset + 16u)));
}

fn tint_symbol_19(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat2x4<f32> {
  return mat2x4<f32>(tint_symbol_13(buffer, (offset + 0u)), tint_symbol_13(buffer, (offset + 16u)));
}

fn tint_symbol_20(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat3x2<f32> {
  return mat3x2<f32>(tint_symbol_5(buffer, (offset + 0u)), tint_symbol_5(buffer, (offset + 8u)), tint_symbol_5(buffer, (offset + 16u)));
}

fn tint_symbol_21(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat3x3<f32> {
  return mat3x3<f32>(tint_symbol_9(buffer, (offset + 0u)), tint_symbol_9(buffer, (offset + 16u)), tint_symbol_9(buffer, (offset + 32u)));
}

fn tint_symbol_22(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat3x4<f32> {
  return mat3x4<f32>(tint_symbol_13(buffer, (offset + 0u)), tint_symbol_13(buffer, (offset + 16u)), tint_symbol_13(buffer, (offset + 32u)));
}

fn tint_symbol_23(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat4x2<f32> {
  return mat4x2<f32>(tint_symbol_5(buffer, (offset + 0u)), tint_symbol_5(buffer, (offset + 8u)), tint_symbol_5(buffer, (offset + 16u)), tint_symbol_5(buffer, (offset + 24u)));
}

fn tint_symbol_24(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat4x3<f32> {
  return mat4x3<f32>(tint_symbol_9(buffer, (offset + 0u)), tint_symbol_9(buffer, (offset + 16u)), tint_symbol_9(buffer, (offset + 32u)), tint_symbol_9(buffer, (offset + 48u)));
}

fn tint_symbol_25(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat4x4<f32> {
  return mat4x4<f32>(tint_symbol_13(buffer, (offset + 0u)), tint_symbol_13(buffer, (offset + 16u)), tint_symbol_13(buffer, (offset + 32u)), tint_symbol_13(buffer, (offset + 48u)));
}

fn tint_symbol_26(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat2x2<f16> {
  return mat2x2<f16>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 4u)));
}

fn tint_symbol_27(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat2x3<f16> {
  return mat2x3<f16>(tint_symbol_12(buffer, (offset + 0u)), tint_symbol_12(buffer, (offset + 8u)));
}

fn tint_symbol_28(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat2x4<f16> {
  return mat2x4<f16>(tint_symbol_16(buffer, (offset + 0u)), tint_symbol_16(buffer, (offset + 8u)));
}

fn tint_symbol_29(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat3x2<f16> {
  return mat3x2<f16>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 4u)), tint_symbol_8(buffer, (offset + 8u)));
}

fn tint_symbol_30(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat3x3<f16> {
  return mat3x3<f16>(tint_symbol_12(buffer, (offset + 0u)), tint_symbol_12(buffer, (offset + 8u)), tint_symbol_12(buffer, (offset + 16u)));
}

fn tint_symbol_31(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat3x4<f16> {
  return mat3x4<f16>(tint_symbol_16(buffer, (offset + 0u)), tint_symbol_16(buffer, (offset + 8u)), tint_symbol_16(buffer, (offset + 16u)));
}

fn tint_symbol_32(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat4x2<f16> {
  return mat4x2<f16>(tint_symbol_8(buffer, (offset + 0u)), tint_symbol_8(buffer, (offset + 4u)), tint_symbol_8(buffer, (offset + 8u)), tint_symbol_8(buffer, (offset + 12u)));
}

fn tint_symbol_33(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat4x3<f16> {
  return mat4x3<f16>(tint_symbol_12(buffer, (offset + 0u)), tint_symbol_12(buffer, (offset + 8u)), tint_symbol_12(buffer, (offset + 16u)), tint_symbol_12(buffer, (offset + 24u)));
}

fn tint_symbol_34(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> mat4x4<f16> {
  return mat4x4<f16>(tint_symbol_16(buffer, (offset + 0u)), tint_symbol_16(buffer, (offset + 8u)), tint_symbol_16(buffer, (offset + 16u)), tint_symbol_16(buffer, (offset + 24u)));
}

fn tint_symbol_35(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> array<vec3<f32>, 2u> {
  var arr : array<vec3<f32>, 2u>;
  for(var i = 0u; (i < 2u); i = (i + 1u)) {
    arr[i] = tint_symbol_9(buffer, (offset + (i * 16u)));
  }
  return arr;
}

fn tint_symbol_36(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> array<mat4x2<f16>, 2u> {
  var arr_1 : array<mat4x2<f16>, 2u>;
  for(var i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
    arr_1[i_1] = tint_symbol_32(buffer, (offset + (i_1 * 16u)));
  }
  return arr_1;
}

fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> SB {
  return SB(tint_symbol_1(buffer, (offset + 0u)), tint_symbol_2(buffer, (offset + 4u)), tint_symbol_3(buffer, (offset + 8u)), tint_symbol_4(buffer, (offset + 12u)), tint_symbol_5(buffer, (offset + 16u)), tint_symbol_6(buffer, (offset + 24u)), tint_symbol_7(buffer, (offset + 32u)), tint_symbol_8(buffer, (offset + 40u)), tint_symbol_9(buffer, (offset + 48u)), tint_symbol_10(buffer, (offset + 64u)), tint_symbol_11(buffer, (offset + 80u)), tint_symbol_12(buffer, (offset + 96u)), tint_symbol_13(buffer, (offset + 112u)), tint_symbol_14(buffer, (offset + 128u)), tint_symbol_15(buffer, (offset + 144u)), tint_symbol_16(buffer, (offset + 160u)), tint_symbol_17(buffer, (offset + 168u)), tint_symbol_18(buffer, (offset + 192u)), tint_symbol_19(buffer, (offset + 224u)), tint_symbol_20(buffer, (offset + 256u)), tint_symbol_21(buffer, (offset + 288u)), tint_symbol_22(buffer, (offset + 336u)), tint_symbol_23(buffer, (offset + 384u)), tint_symbol_24(buffer, (offset + 416u)), tint_symbol_25(buffer, (offset + 480u)), tint_symbol_26(buffer, (offset + 544u)), tint_symbol_27(buffer, (offset + 552u)), tint_symbol_28(buffer, (offset + 568u)), tint_symbol_29(buffer, (offset + 584u)), tint_symbol_30(buffer, (offset + 600u)), tint_symbol_31(buffer, (offset + 624u)), tint_symbol_32(buffer, (offset + 648u)), tint_symbol_33(buffer, (offset + 664u)), tint_symbol_34(buffer, (offset + 696u)), tint_symbol_35(buffer, (offset + 736u)), tint_symbol_36(buffer, (offset + 768u)));
}

@compute @workgroup_size(1)
fn main() {
  var x : SB = tint_symbol(&(sb), 0u);
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_mat4x2_f16 : array<mat4x2<f16>, 2>,
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, StoreStructure) {
    auto* src = R"(
enable f16;

struct SB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_mat4x2_f16 : array<mat4x2<f16>, 2>,
};

@group(0) @binding(0) var<storage, read_write> sb : SB;

@compute @workgroup_size(1)
fn main() {
  sb = SB();
}
)";

    auto* expect = R"(
enable f16;

struct SB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_mat4x2_f16 : array<mat4x2<f16>, 2>,
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

@internal(intrinsic_store_storage_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_1(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : f32)

@internal(intrinsic_store_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_2(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : i32)

@internal(intrinsic_store_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_3(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : u32)

@internal(intrinsic_store_storage_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_4(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : f16)

@internal(intrinsic_store_storage_vec2_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_5(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec2<f32>)

@internal(intrinsic_store_storage_vec2_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_6(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec2<i32>)

@internal(intrinsic_store_storage_vec2_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_7(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec2<u32>)

@internal(intrinsic_store_storage_vec2_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_8(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec2<f16>)

@internal(intrinsic_store_storage_vec3_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_9(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec3<f32>)

@internal(intrinsic_store_storage_vec3_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_10(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec3<i32>)

@internal(intrinsic_store_storage_vec3_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_11(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec3<u32>)

@internal(intrinsic_store_storage_vec3_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_12(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec3<f16>)

@internal(intrinsic_store_storage_vec4_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_13(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec4<f32>)

@internal(intrinsic_store_storage_vec4_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_14(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec4<i32>)

@internal(intrinsic_store_storage_vec4_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_15(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec4<u32>)

@internal(intrinsic_store_storage_vec4_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_16(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec4<f16>)

fn tint_symbol_17(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat2x2<f32>) {
  tint_symbol_5(buffer, (offset + 0u), value[0u]);
  tint_symbol_5(buffer, (offset + 8u), value[1u]);
}

fn tint_symbol_18(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat2x3<f32>) {
  tint_symbol_9(buffer, (offset + 0u), value[0u]);
  tint_symbol_9(buffer, (offset + 16u), value[1u]);
}

fn tint_symbol_19(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat2x4<f32>) {
  tint_symbol_13(buffer, (offset + 0u), value[0u]);
  tint_symbol_13(buffer, (offset + 16u), value[1u]);
}

fn tint_symbol_20(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat3x2<f32>) {
  tint_symbol_5(buffer, (offset + 0u), value[0u]);
  tint_symbol_5(buffer, (offset + 8u), value[1u]);
  tint_symbol_5(buffer, (offset + 16u), value[2u]);
}

fn tint_symbol_21(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat3x3<f32>) {
  tint_symbol_9(buffer, (offset + 0u), value[0u]);
  tint_symbol_9(buffer, (offset + 16u), value[1u]);
  tint_symbol_9(buffer, (offset + 32u), value[2u]);
}

fn tint_symbol_22(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat3x4<f32>) {
  tint_symbol_13(buffer, (offset + 0u), value[0u]);
  tint_symbol_13(buffer, (offset + 16u), value[1u]);
  tint_symbol_13(buffer, (offset + 32u), value[2u]);
}

fn tint_symbol_23(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat4x2<f32>) {
  tint_symbol_5(buffer, (offset + 0u), value[0u]);
  tint_symbol_5(buffer, (offset + 8u), value[1u]);
  tint_symbol_5(buffer, (offset + 16u), value[2u]);
  tint_symbol_5(buffer, (offset + 24u), value[3u]);
}

fn tint_symbol_24(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat4x3<f32>) {
  tint_symbol_9(buffer, (offset + 0u), value[0u]);
  tint_symbol_9(buffer, (offset + 16u), value[1u]);
  tint_symbol_9(buffer, (offset + 32u), value[2u]);
  tint_symbol_9(buffer, (offset + 48u), value[3u]);
}

fn tint_symbol_25(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat4x4<f32>) {
  tint_symbol_13(buffer, (offset + 0u), value[0u]);
  tint_symbol_13(buffer, (offset + 16u), value[1u]);
  tint_symbol_13(buffer, (offset + 32u), value[2u]);
  tint_symbol_13(buffer, (offset + 48u), value[3u]);
}

fn tint_symbol_26(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat2x2<f16>) {
  tint_symbol_8(buffer, (offset + 0u), value[0u]);
  tint_symbol_8(buffer, (offset + 4u), value[1u]);
}

fn tint_symbol_27(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat2x3<f16>) {
  tint_symbol_12(buffer, (offset + 0u), value[0u]);
  tint_symbol_12(buffer, (offset + 8u), value[1u]);
}

fn tint_symbol_28(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat2x4<f16>) {
  tint_symbol_16(buffer, (offset + 0u), value[0u]);
  tint_symbol_16(buffer, (offset + 8u), value[1u]);
}

fn tint_symbol_29(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat3x2<f16>) {
  tint_symbol_8(buffer, (offset + 0u), value[0u]);
  tint_symbol_8(buffer, (offset + 4u), value[1u]);
  tint_symbol_8(buffer, (offset + 8u), value[2u]);
}

fn tint_symbol_30(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat3x3<f16>) {
  tint_symbol_12(buffer, (offset + 0u), value[0u]);
  tint_symbol_12(buffer, (offset + 8u), value[1u]);
  tint_symbol_12(buffer, (offset + 16u), value[2u]);
}

fn tint_symbol_31(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat3x4<f16>) {
  tint_symbol_16(buffer, (offset + 0u), value[0u]);
  tint_symbol_16(buffer, (offset + 8u), value[1u]);
  tint_symbol_16(buffer, (offset + 16u), value[2u]);
}

fn tint_symbol_32(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat4x2<f16>) {
  tint_symbol_8(buffer, (offset + 0u), value[0u]);
  tint_symbol_8(buffer, (offset + 4u), value[1u]);
  tint_symbol_8(buffer, (offset + 8u), value[2u]);
  tint_symbol_8(buffer, (offset + 12u), value[3u]);
}

fn tint_symbol_33(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat4x3<f16>) {
  tint_symbol_12(buffer, (offset + 0u), value[0u]);
  tint_symbol_12(buffer, (offset + 8u), value[1u]);
  tint_symbol_12(buffer, (offset + 16u), value[2u]);
  tint_symbol_12(buffer, (offset + 24u), value[3u]);
}

fn tint_symbol_34(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat4x4<f16>) {
  tint_symbol_16(buffer, (offset + 0u), value[0u]);
  tint_symbol_16(buffer, (offset + 8u), value[1u]);
  tint_symbol_16(buffer, (offset + 16u), value[2u]);
  tint_symbol_16(buffer, (offset + 24u), value[3u]);
}

fn tint_symbol_35(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : array<vec3<f32>, 2u>) {
  var array_1 = value;
  for(var i = 0u; (i < 2u); i = (i + 1u)) {
    tint_symbol_9(buffer, (offset + (i * 16u)), array_1[i]);
  }
}

fn tint_symbol_36(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : array<mat4x2<f16>, 2u>) {
  var array_2 = value;
  for(var i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
    tint_symbol_32(buffer, (offset + (i_1 * 16u)), array_2[i_1]);
  }
}

fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : SB) {
  tint_symbol_1(buffer, (offset + 0u), value.scalar_f32);
  tint_symbol_2(buffer, (offset + 4u), value.scalar_i32);
  tint_symbol_3(buffer, (offset + 8u), value.scalar_u32);
  tint_symbol_4(buffer, (offset + 12u), value.scalar_f16);
  tint_symbol_5(buffer, (offset + 16u), value.vec2_f32);
  tint_symbol_6(buffer, (offset + 24u), value.vec2_i32);
  tint_symbol_7(buffer, (offset + 32u), value.vec2_u32);
  tint_symbol_8(buffer, (offset + 40u), value.vec2_f16);
  tint_symbol_9(buffer, (offset + 48u), value.vec3_f32);
  tint_symbol_10(buffer, (offset + 64u), value.vec3_i32);
  tint_symbol_11(buffer, (offset + 80u), value.vec3_u32);
  tint_symbol_12(buffer, (offset + 96u), value.vec3_f16);
  tint_symbol_13(buffer, (offset + 112u), value.vec4_f32);
  tint_symbol_14(buffer, (offset + 128u), value.vec4_i32);
  tint_symbol_15(buffer, (offset + 144u), value.vec4_u32);
  tint_symbol_16(buffer, (offset + 160u), value.vec4_f16);
  tint_symbol_17(buffer, (offset + 168u), value.mat2x2_f32);
  tint_symbol_18(buffer, (offset + 192u), value.mat2x3_f32);
  tint_symbol_19(buffer, (offset + 224u), value.mat2x4_f32);
  tint_symbol_20(buffer, (offset + 256u), value.mat3x2_f32);
  tint_symbol_21(buffer, (offset + 288u), value.mat3x3_f32);
  tint_symbol_22(buffer, (offset + 336u), value.mat3x4_f32);
  tint_symbol_23(buffer, (offset + 384u), value.mat4x2_f32);
  tint_symbol_24(buffer, (offset + 416u), value.mat4x3_f32);
  tint_symbol_25(buffer, (offset + 480u), value.mat4x4_f32);
  tint_symbol_26(buffer, (offset + 544u), value.mat2x2_f16);
  tint_symbol_27(buffer, (offset + 552u), value.mat2x3_f16);
  tint_symbol_28(buffer, (offset + 568u), value.mat2x4_f16);
  tint_symbol_29(buffer, (offset + 584u), value.mat3x2_f16);
  tint_symbol_30(buffer, (offset + 600u), value.mat3x3_f16);
  tint_symbol_31(buffer, (offset + 624u), value.mat3x4_f16);
  tint_symbol_32(buffer, (offset + 648u), value.mat4x2_f16);
  tint_symbol_33(buffer, (offset + 664u), value.mat4x3_f16);
  tint_symbol_34(buffer, (offset + 696u), value.mat4x4_f16);
  tint_symbol_35(buffer, (offset + 736u), value.arr2_vec3_f32);
  tint_symbol_36(buffer, (offset + 768u), value.arr2_mat4x2_f16);
}

@compute @workgroup_size(1)
fn main() {
  tint_symbol(&(sb), 0u, SB());
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, StoreStructure_OutOfOrder) {
    auto* src = R"(
enable f16;

@compute @workgroup_size(1)
fn main() {
  sb = SB();
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_mat4x2_f16 : array<mat4x2<f16>, 2>,
};
)";

    auto* expect = R"(
enable f16;

@internal(intrinsic_store_storage_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_1(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : f32)

@internal(intrinsic_store_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_2(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : i32)

@internal(intrinsic_store_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_3(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : u32)

@internal(intrinsic_store_storage_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_4(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : f16)

@internal(intrinsic_store_storage_vec2_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_5(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec2<f32>)

@internal(intrinsic_store_storage_vec2_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_6(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec2<i32>)

@internal(intrinsic_store_storage_vec2_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_7(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec2<u32>)

@internal(intrinsic_store_storage_vec2_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_8(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec2<f16>)

@internal(intrinsic_store_storage_vec3_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_9(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec3<f32>)

@internal(intrinsic_store_storage_vec3_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_10(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec3<i32>)

@internal(intrinsic_store_storage_vec3_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_11(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec3<u32>)

@internal(intrinsic_store_storage_vec3_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_12(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec3<f16>)

@internal(intrinsic_store_storage_vec4_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_13(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec4<f32>)

@internal(intrinsic_store_storage_vec4_i32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_14(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec4<i32>)

@internal(intrinsic_store_storage_vec4_u32) @internal(disable_validation__function_has_no_body)
fn tint_symbol_15(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec4<u32>)

@internal(intrinsic_store_storage_vec4_f16) @internal(disable_validation__function_has_no_body)
fn tint_symbol_16(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : vec4<f16>)

fn tint_symbol_17(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat2x2<f32>) {
  tint_symbol_5(buffer, (offset + 0u), value[0u]);
  tint_symbol_5(buffer, (offset + 8u), value[1u]);
}

fn tint_symbol_18(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat2x3<f32>) {
  tint_symbol_9(buffer, (offset + 0u), value[0u]);
  tint_symbol_9(buffer, (offset + 16u), value[1u]);
}

fn tint_symbol_19(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat2x4<f32>) {
  tint_symbol_13(buffer, (offset + 0u), value[0u]);
  tint_symbol_13(buffer, (offset + 16u), value[1u]);
}

fn tint_symbol_20(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat3x2<f32>) {
  tint_symbol_5(buffer, (offset + 0u), value[0u]);
  tint_symbol_5(buffer, (offset + 8u), value[1u]);
  tint_symbol_5(buffer, (offset + 16u), value[2u]);
}

fn tint_symbol_21(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat3x3<f32>) {
  tint_symbol_9(buffer, (offset + 0u), value[0u]);
  tint_symbol_9(buffer, (offset + 16u), value[1u]);
  tint_symbol_9(buffer, (offset + 32u), value[2u]);
}

fn tint_symbol_22(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat3x4<f32>) {
  tint_symbol_13(buffer, (offset + 0u), value[0u]);
  tint_symbol_13(buffer, (offset + 16u), value[1u]);
  tint_symbol_13(buffer, (offset + 32u), value[2u]);
}

fn tint_symbol_23(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat4x2<f32>) {
  tint_symbol_5(buffer, (offset + 0u), value[0u]);
  tint_symbol_5(buffer, (offset + 8u), value[1u]);
  tint_symbol_5(buffer, (offset + 16u), value[2u]);
  tint_symbol_5(buffer, (offset + 24u), value[3u]);
}

fn tint_symbol_24(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat4x3<f32>) {
  tint_symbol_9(buffer, (offset + 0u), value[0u]);
  tint_symbol_9(buffer, (offset + 16u), value[1u]);
  tint_symbol_9(buffer, (offset + 32u), value[2u]);
  tint_symbol_9(buffer, (offset + 48u), value[3u]);
}

fn tint_symbol_25(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat4x4<f32>) {
  tint_symbol_13(buffer, (offset + 0u), value[0u]);
  tint_symbol_13(buffer, (offset + 16u), value[1u]);
  tint_symbol_13(buffer, (offset + 32u), value[2u]);
  tint_symbol_13(buffer, (offset + 48u), value[3u]);
}

fn tint_symbol_26(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat2x2<f16>) {
  tint_symbol_8(buffer, (offset + 0u), value[0u]);
  tint_symbol_8(buffer, (offset + 4u), value[1u]);
}

fn tint_symbol_27(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat2x3<f16>) {
  tint_symbol_12(buffer, (offset + 0u), value[0u]);
  tint_symbol_12(buffer, (offset + 8u), value[1u]);
}

fn tint_symbol_28(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat2x4<f16>) {
  tint_symbol_16(buffer, (offset + 0u), value[0u]);
  tint_symbol_16(buffer, (offset + 8u), value[1u]);
}

fn tint_symbol_29(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat3x2<f16>) {
  tint_symbol_8(buffer, (offset + 0u), value[0u]);
  tint_symbol_8(buffer, (offset + 4u), value[1u]);
  tint_symbol_8(buffer, (offset + 8u), value[2u]);
}

fn tint_symbol_30(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat3x3<f16>) {
  tint_symbol_12(buffer, (offset + 0u), value[0u]);
  tint_symbol_12(buffer, (offset + 8u), value[1u]);
  tint_symbol_12(buffer, (offset + 16u), value[2u]);
}

fn tint_symbol_31(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat3x4<f16>) {
  tint_symbol_16(buffer, (offset + 0u), value[0u]);
  tint_symbol_16(buffer, (offset + 8u), value[1u]);
  tint_symbol_16(buffer, (offset + 16u), value[2u]);
}

fn tint_symbol_32(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat4x2<f16>) {
  tint_symbol_8(buffer, (offset + 0u), value[0u]);
  tint_symbol_8(buffer, (offset + 4u), value[1u]);
  tint_symbol_8(buffer, (offset + 8u), value[2u]);
  tint_symbol_8(buffer, (offset + 12u), value[3u]);
}

fn tint_symbol_33(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat4x3<f16>) {
  tint_symbol_12(buffer, (offset + 0u), value[0u]);
  tint_symbol_12(buffer, (offset + 8u), value[1u]);
  tint_symbol_12(buffer, (offset + 16u), value[2u]);
  tint_symbol_12(buffer, (offset + 24u), value[3u]);
}

fn tint_symbol_34(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : mat4x4<f16>) {
  tint_symbol_16(buffer, (offset + 0u), value[0u]);
  tint_symbol_16(buffer, (offset + 8u), value[1u]);
  tint_symbol_16(buffer, (offset + 16u), value[2u]);
  tint_symbol_16(buffer, (offset + 24u), value[3u]);
}

fn tint_symbol_35(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : array<vec3<f32>, 2u>) {
  var array_1 = value;
  for(var i = 0u; (i < 2u); i = (i + 1u)) {
    tint_symbol_9(buffer, (offset + (i * 16u)), array_1[i]);
  }
}

fn tint_symbol_36(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : array<mat4x2<f16>, 2u>) {
  var array_2 = value;
  for(var i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
    tint_symbol_32(buffer, (offset + (i_1 * 16u)), array_2[i_1]);
  }
}

fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, value : SB) {
  tint_symbol_1(buffer, (offset + 0u), value.scalar_f32);
  tint_symbol_2(buffer, (offset + 4u), value.scalar_i32);
  tint_symbol_3(buffer, (offset + 8u), value.scalar_u32);
  tint_symbol_4(buffer, (offset + 12u), value.scalar_f16);
  tint_symbol_5(buffer, (offset + 16u), value.vec2_f32);
  tint_symbol_6(buffer, (offset + 24u), value.vec2_i32);
  tint_symbol_7(buffer, (offset + 32u), value.vec2_u32);
  tint_symbol_8(buffer, (offset + 40u), value.vec2_f16);
  tint_symbol_9(buffer, (offset + 48u), value.vec3_f32);
  tint_symbol_10(buffer, (offset + 64u), value.vec3_i32);
  tint_symbol_11(buffer, (offset + 80u), value.vec3_u32);
  tint_symbol_12(buffer, (offset + 96u), value.vec3_f16);
  tint_symbol_13(buffer, (offset + 112u), value.vec4_f32);
  tint_symbol_14(buffer, (offset + 128u), value.vec4_i32);
  tint_symbol_15(buffer, (offset + 144u), value.vec4_u32);
  tint_symbol_16(buffer, (offset + 160u), value.vec4_f16);
  tint_symbol_17(buffer, (offset + 168u), value.mat2x2_f32);
  tint_symbol_18(buffer, (offset + 192u), value.mat2x3_f32);
  tint_symbol_19(buffer, (offset + 224u), value.mat2x4_f32);
  tint_symbol_20(buffer, (offset + 256u), value.mat3x2_f32);
  tint_symbol_21(buffer, (offset + 288u), value.mat3x3_f32);
  tint_symbol_22(buffer, (offset + 336u), value.mat3x4_f32);
  tint_symbol_23(buffer, (offset + 384u), value.mat4x2_f32);
  tint_symbol_24(buffer, (offset + 416u), value.mat4x3_f32);
  tint_symbol_25(buffer, (offset + 480u), value.mat4x4_f32);
  tint_symbol_26(buffer, (offset + 544u), value.mat2x2_f16);
  tint_symbol_27(buffer, (offset + 552u), value.mat2x3_f16);
  tint_symbol_28(buffer, (offset + 568u), value.mat2x4_f16);
  tint_symbol_29(buffer, (offset + 584u), value.mat3x2_f16);
  tint_symbol_30(buffer, (offset + 600u), value.mat3x3_f16);
  tint_symbol_31(buffer, (offset + 624u), value.mat3x4_f16);
  tint_symbol_32(buffer, (offset + 648u), value.mat4x2_f16);
  tint_symbol_33(buffer, (offset + 664u), value.mat4x3_f16);
  tint_symbol_34(buffer, (offset + 696u), value.mat4x4_f16);
  tint_symbol_35(buffer, (offset + 736u), value.arr2_vec3_f32);
  tint_symbol_36(buffer, (offset + 768u), value.arr2_mat4x2_f16);
}

@compute @workgroup_size(1)
fn main() {
  tint_symbol(&(sb), 0u, SB());
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_mat4x2_f16 : array<mat4x2<f16>, 2>,
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

@compute @workgroup_size(1)
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
fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> f32

@compute @workgroup_size(1)
fn main() {
  var x : f32 = tint_symbol(&(sb), 712u);
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, ComplexStaticAccessChain_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
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
fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> f32

@compute @workgroup_size(1)
fn main() {
  var x : f32 = tint_symbol(&(sb), 712u);
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

@compute @workgroup_size(1)
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
fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> f32

@compute @workgroup_size(1)
fn main() {
  var i : i32 = 4;
  var j : u32 = 1u;
  var k : i32 = 2;
  var x : f32 = tint_symbol(&(sb), (((((128u + (128u * u32(i))) + 16u) + (32u * j)) + 16u) + (4u * u32(k))));
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, ComplexDynamicAccessChain_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
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
fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> f32

@compute @workgroup_size(1)
fn main() {
  var i : i32 = 4;
  var j : u32 = 1u;
  var k : i32 = 2;
  var x : f32 = tint_symbol(&(sb), (((((128u + (128u * u32(i))) + 16u) + (32u * j)) + 16u) + (4u * u32(k))));
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

alias A1 = S1;

alias A1_Array = array<S1, 3>;

struct S2 {
  a : i32,
  b : A1_Array,
  c : i32,
};

alias A2 = S2;

alias A2_Array = array<S2>;

struct SB {
  @size(128)
  a : i32,
  b : A2_Array,
};

@group(0) @binding(0) var<storage, read_write> sb : SB;

@compute @workgroup_size(1)
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

alias A1 = S1;

alias A1_Array = array<S1, 3>;

struct S2 {
  a : i32,
  b : A1_Array,
  c : i32,
}

alias A2 = S2;

alias A2_Array = array<S2>;

struct SB {
  @size(128)
  a : i32,
  b : A2_Array,
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

@internal(intrinsic_load_storage_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> f32

@compute @workgroup_size(1)
fn main() {
  var i : i32 = 4;
  var j : u32 = 1u;
  var k : i32 = 2;
  var x : f32 = tint_symbol(&(sb), (((((128u + (128u * u32(i))) + 16u) + (32u * j)) + 16u) + (4u * u32(k))));
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, ComplexDynamicAccessChainWithAliases_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
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

alias A2_Array = array<S2>;

alias A2 = S2;

struct S2 {
  a : i32,
  b : A1_Array,
  c : i32,
};

alias A1 = S1;

alias A1_Array = array<S1, 3>;

struct S1 {
  a : i32,
  b : vec3<f32>,
  c : i32,
};
)";

    auto* expect = R"(
@internal(intrinsic_load_storage_f32) @internal(disable_validation__function_has_no_body)
fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> f32

@compute @workgroup_size(1)
fn main() {
  var i : i32 = 4;
  var j : u32 = 1u;
  var k : i32 = 2;
  var x : f32 = tint_symbol(&(sb), (((((128u + (128u * u32(i))) + 16u) + (32u * j)) + 16u) + (4u * u32(k))));
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  @size(128)
  a : i32,
  b : A2_Array,
}

alias A2_Array = array<S2>;

alias A2 = S2;

struct S2 {
  a : i32,
  b : A1_Array,
  c : i32,
}

alias A1 = S1;

alias A1_Array = array<S1, 3>;

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

@compute @workgroup_size(1)
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
fn tint_atomicStore(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : i32)

@internal(intrinsic_atomic_load_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_atomicLoad(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> i32

@internal(intrinsic_atomic_add_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_atomicAdd(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_sub_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_atomicSub(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_max_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_atomicMax(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_min_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_atomicMin(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_and_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_atomicAnd(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_or_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_atomicOr(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_xor_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_atomicXor(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_exchange_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_atomicExchange(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : i32) -> i32

struct atomic_compare_exchange_weak_ret_type {
  old_value : i32,
  exchanged : bool,
}

@internal(intrinsic_atomic_compare_exchange_weak_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_atomicCompareExchangeWeak(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : i32, param_2 : i32) -> atomic_compare_exchange_weak_ret_type

@internal(intrinsic_atomic_store_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_atomicStore_1(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : u32)

@internal(intrinsic_atomic_load_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_atomicLoad_1(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> u32

@internal(intrinsic_atomic_add_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_atomicAdd_1(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_sub_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_atomicSub_1(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_max_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_atomicMax_1(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_min_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_atomicMin_1(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_and_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_atomicAnd_1(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_or_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_atomicOr_1(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_xor_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_atomicXor_1(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_exchange_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_atomicExchange_1(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : u32) -> u32

struct atomic_compare_exchange_weak_ret_type_1 {
  old_value : u32,
  exchanged : bool,
}

@internal(intrinsic_atomic_compare_exchange_weak_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_atomicCompareExchangeWeak_1(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : u32, param_2 : u32) -> atomic_compare_exchange_weak_ret_type_1

@compute @workgroup_size(1)
fn main() {
  tint_atomicStore(&(sb), 16u, 123);
  tint_atomicLoad(&(sb), 16u);
  tint_atomicAdd(&(sb), 16u, 123);
  tint_atomicSub(&(sb), 16u, 123);
  tint_atomicMax(&(sb), 16u, 123);
  tint_atomicMin(&(sb), 16u, 123);
  tint_atomicAnd(&(sb), 16u, 123);
  tint_atomicOr(&(sb), 16u, 123);
  tint_atomicXor(&(sb), 16u, 123);
  tint_atomicExchange(&(sb), 16u, 123);
  tint_atomicCompareExchangeWeak(&(sb), 16u, 123, 345);
  tint_atomicStore_1(&(sb), 20u, 123u);
  tint_atomicLoad_1(&(sb), 20u);
  tint_atomicAdd_1(&(sb), 20u, 123u);
  tint_atomicSub_1(&(sb), 20u, 123u);
  tint_atomicMax_1(&(sb), 20u, 123u);
  tint_atomicMin_1(&(sb), 20u, 123u);
  tint_atomicAnd_1(&(sb), 20u, 123u);
  tint_atomicOr_1(&(sb), 20u, 123u);
  tint_atomicXor_1(&(sb), 20u, 123u);
  tint_atomicExchange_1(&(sb), 20u, 123u);
  tint_atomicCompareExchangeWeak_1(&(sb), 20u, 123u, 345u);
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, StorageBufferAtomics_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
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
fn tint_atomicStore(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : i32)

@internal(intrinsic_atomic_load_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_atomicLoad(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> i32

@internal(intrinsic_atomic_add_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_atomicAdd(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_sub_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_atomicSub(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_max_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_atomicMax(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_min_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_atomicMin(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_and_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_atomicAnd(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_or_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_atomicOr(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_xor_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_atomicXor(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_exchange_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_atomicExchange(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : i32) -> i32

struct atomic_compare_exchange_weak_ret_type {
  old_value : i32,
  exchanged : bool,
}

@internal(intrinsic_atomic_compare_exchange_weak_storage_i32) @internal(disable_validation__function_has_no_body)
fn tint_atomicCompareExchangeWeak(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : i32, param_2 : i32) -> atomic_compare_exchange_weak_ret_type

@internal(intrinsic_atomic_store_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_atomicStore_1(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : u32)

@internal(intrinsic_atomic_load_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_atomicLoad_1(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32) -> u32

@internal(intrinsic_atomic_add_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_atomicAdd_1(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_sub_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_atomicSub_1(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_max_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_atomicMax_1(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_min_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_atomicMin_1(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_and_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_atomicAnd_1(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_or_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_atomicOr_1(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_xor_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_atomicXor_1(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_exchange_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_atomicExchange_1(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : u32) -> u32

struct atomic_compare_exchange_weak_ret_type_1 {
  old_value : u32,
  exchanged : bool,
}

@internal(intrinsic_atomic_compare_exchange_weak_storage_u32) @internal(disable_validation__function_has_no_body)
fn tint_atomicCompareExchangeWeak_1(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read_write>, offset : u32, param_1 : u32, param_2 : u32) -> atomic_compare_exchange_weak_ret_type_1

@compute @workgroup_size(1)
fn main() {
  tint_atomicStore(&(sb), 16u, 123);
  tint_atomicLoad(&(sb), 16u);
  tint_atomicAdd(&(sb), 16u, 123);
  tint_atomicSub(&(sb), 16u, 123);
  tint_atomicMax(&(sb), 16u, 123);
  tint_atomicMin(&(sb), 16u, 123);
  tint_atomicAnd(&(sb), 16u, 123);
  tint_atomicOr(&(sb), 16u, 123);
  tint_atomicXor(&(sb), 16u, 123);
  tint_atomicExchange(&(sb), 16u, 123);
  tint_atomicCompareExchangeWeak(&(sb), 16u, 123, 345);
  tint_atomicStore_1(&(sb), 20u, 123u);
  tint_atomicLoad_1(&(sb), 20u);
  tint_atomicAdd_1(&(sb), 20u, 123u);
  tint_atomicSub_1(&(sb), 20u, 123u);
  tint_atomicMax_1(&(sb), 20u, 123u);
  tint_atomicMin_1(&(sb), 20u, 123u);
  tint_atomicAnd_1(&(sb), 20u, 123u);
  tint_atomicOr_1(&(sb), 20u, 123u);
  tint_atomicXor_1(&(sb), 20u, 123u);
  tint_atomicExchange_1(&(sb), 20u, 123u);
  tint_atomicCompareExchangeWeak_1(&(sb), 20u, 123u, 345u);
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

@compute @workgroup_size(1)
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
@compute @workgroup_size(1)
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
