// Copyright 2022 The Tint Authors.
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

#include "src/tint/transform/packed_vec3.h"

#include <string>
#include <utility>
#include <vector>

#include "src/tint/ast/module.h"
#include "src/tint/program_builder.h"
#include "src/tint/sem/struct.h"
#include "src/tint/sem/variable.h"
#include "src/tint/transform/test_helper.h"
#include "src/tint/type/array.h"
#include "src/tint/utils/string.h"

namespace tint::transform {
namespace {

using PackedVec3Test = TransformTest;

TEST_F(PackedVec3Test, ShouldRun_EmptyModule) {
    auto* src = R"()";

    EXPECT_FALSE(ShouldRun<PackedVec3>(src));
}

TEST_F(PackedVec3Test, ShouldRun_NoHostShareableVec3s) {
    auto* src = R"(
struct S {
  v : vec3<f32>,
  m : mat3x3<f32>,
  a : array<vec3<f32>, 4>,
}

var<private> p_s : S;
var<private> p_v : vec3<f32>;
var<private> p_m : mat3x3<f32>;
var<private> p_a : array<vec3<f32>, 4>;

var<workgroup> w_s : S;
var<workgroup> w_v : vec3<f32>;
var<workgroup> w_m : mat3x3<f32>;
var<workgroup> w_a : array<vec3<f32>, 4>;

fn f() {
  var f_s : S;
  var f_v : vec3<f32>;
  var f_m : mat3x3<f32>;
  var f_a : array<vec3<f32>, 4>;
}
)";

    EXPECT_FALSE(ShouldRun<PackedVec3>(src));
}

TEST_F(PackedVec3Test, ShouldRun_Vec4Vec2) {
    auto* src = R"(
struct S {
  v4 : vec4<f32>,
  v2 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> Ps : S; // Host sharable
@group(0) @binding(1) var<uniform> Pv4 : vec4<f32>; // Host sharable
@group(0) @binding(2) var<uniform> Pv2 : vec2<f32>; // Host sharable
)";

    EXPECT_FALSE(ShouldRun<PackedVec3>(src));
}

TEST_F(PackedVec3Test, ShouldRun_OtherMatrices) {
    auto* src = R"(
struct S {
  m2x2 : mat2x2<f32>,
  m2x4 : mat2x4<f32>,
  m3x2 : mat3x2<f32>,
  m3x4 : mat3x4<f32>,
  m4x2 : mat4x2<f32>,
  m4x4 : mat4x4<f32>,
}

@group(0) @binding(0) var<uniform> Ps : S; // Host sharable
@group(0) @binding(1) var<uniform> Pm2x2 : mat2x2<f32>; // Host sharable
@group(0) @binding(2) var<uniform> Pm2x4 : mat2x4<f32>; // Host sharable
@group(0) @binding(3) var<uniform> Pm3x2 : mat3x2<f32>; // Host sharable
@group(0) @binding(4) var<uniform> Pm3x4 : mat3x4<f32>; // Host sharable
@group(0) @binding(5) var<uniform> Pm4x2 : mat4x2<f32>; // Host sharable
@group(0) @binding(6) var<uniform> Pm4x4 : mat4x4<f32>; // Host sharable
)";

    EXPECT_FALSE(ShouldRun<PackedVec3>(src));
}

TEST_F(PackedVec3Test, ShouldRun_ArrayOfNonVec3) {
    auto* src = R"(
struct S {
  arr_v : array<vec2<f32>, 4>,
  arr_m : array<mat3x2<f32>, 4>,
}

@group(0) @binding(0) var<storage> Ps : S; // Host sharable
@group(0) @binding(1) var<storage> Parr_v : array<vec2<f32>, 4>; // Host sharable
@group(0) @binding(2) var<storage> Parr_m : array<mat3x2<f32>, 4>; // Host sharable
)";

    EXPECT_FALSE(ShouldRun<PackedVec3>(src));
}

TEST_F(PackedVec3Test, ShouldRun_HostSharable_Vec3) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> P : vec3<f32>; // Host sharable
)";

    EXPECT_TRUE(ShouldRun<PackedVec3>(src));
}

TEST_F(PackedVec3Test, ShouldRun_HostSharable_Mat3x3) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> P : mat3x3<f32>; // Host sharable
)";

    EXPECT_TRUE(ShouldRun<PackedVec3>(src));
}

TEST_F(PackedVec3Test, ShouldRun_HostSharable_ArrayOfVec3) {
    auto* src = R"(
@group(0) @binding(0) var<storage> P : array<vec3<f32>>; // Host sharable
)";

    EXPECT_TRUE(ShouldRun<PackedVec3>(src));
}

TEST_F(PackedVec3Test, ShouldRun_HostSharable_ArrayOfMat3x3) {
    auto* src = R"(
@group(0) @binding(0) var<storage> P : array<mat3x3<f32>>; // Host sharable
)";

    EXPECT_TRUE(ShouldRun<PackedVec3>(src));
}

TEST_F(PackedVec3Test, ShouldRun_HostSharableStruct_Vec3) {
    auto* src = R"(
struct S {
  v : vec3<f32>,
}

@group(0) @binding(0) var<uniform> P : S; // Host sharable
)";

    EXPECT_TRUE(ShouldRun<PackedVec3>(src));
}

TEST_F(PackedVec3Test, ShouldRun_HostSharableStruct_Mat3x3) {
    auto* src = R"(
struct S {
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<uniform> P : S; // Host sharable
)";

    EXPECT_TRUE(ShouldRun<PackedVec3>(src));
}

TEST_F(PackedVec3Test, ShouldRun_HostSharableStruct_ArrayOfVec3) {
    auto* src = R"(
struct S {
  a : array<vec3<f32>, 4>,
}

@group(0) @binding(0) var<uniform> P : S; // Host sharable
)";

    EXPECT_TRUE(ShouldRun<PackedVec3>(src));
}

TEST_F(PackedVec3Test, ShouldRun_HostSharableStruct_ArrayOfMat3x3) {
    auto* src = R"(
struct S {
  a : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<uniform> P : S; // Host sharable
)";

    EXPECT_TRUE(ShouldRun<PackedVec3>(src));
}

TEST_F(PackedVec3Test, Vec3_ReadVector) {
    auto* src = R"(
@group(0) @binding(0) var<storage> v : vec3<f32>;

fn f() {
  let x = v;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

@group(0) @binding(0) var<storage> v : __packed_vec3<f32>;

fn f() {
  let x = vec3<f32>(v);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, Vec3_ReadComponent_MemberAccessChain) {
    auto* src = R"(
@group(0) @binding(0) var<storage> v : vec3<f32>;

fn f() {
  let x = v.yz.x;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

@group(0) @binding(0) var<storage> v : __packed_vec3<f32>;

fn f() {
  let x = vec3<f32>(v).yz.x;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, Vec3_ReadComponent_IndexAccessor) {
    auto* src = R"(
@group(0) @binding(0) var<storage> v : vec3<f32>;

fn f() {
  let x = v[1];
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

@group(0) @binding(0) var<storage> v : __packed_vec3<f32>;

fn f() {
  let x = v[1];
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, Vec3_WriteVector_ValueRHS) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> v : vec3<f32>;

fn f() {
  v = vec3(1.23);
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

@group(0) @binding(0) var<storage, read_write> v : __packed_vec3<f32>;

fn f() {
  v = __packed_vec3<f32>(vec3(1.22999999999999998224));
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, Vec3_WriteVector_RefRHS) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> v : vec3<f32>;
@group(0) @binding(1) var<uniform> in : vec3<f32>;

fn f() {
  v = in;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

@group(0) @binding(0) var<storage, read_write> v : __packed_vec3<f32>;

@group(0) @binding(1) var<uniform> in : __packed_vec3<f32>;

fn f() {
  v = in;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, Vec3_WriteComponent_MemberAccessor) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> v : vec3<f32>;

fn f() {
  v.y = 1.23;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

@group(0) @binding(0) var<storage, read_write> v : __packed_vec3<f32>;

fn f() {
  v.y = 1.22999999999999998224;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, Vec3_WriteComponent_IndexAccessor) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> v : vec3<f32>;

fn f() {
  v[1] = 1.23;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

@group(0) @binding(0) var<storage, read_write> v : __packed_vec3<f32>;

fn f() {
  v[1] = 1.22999999999999998224;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ArrayOfVec3_ReadArray) {
    auto* src = R"(
@group(0) @binding(0) var<storage> arr : array<vec3<f32>, 4>;

fn f() {
  let x = arr;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

fn tint_unpack_vec3_in_composite(in : array<tint_packed_vec3_f32_array_element, 4u>) -> array<vec3<f32>, 4u> {
  var result : array<vec3<f32>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

@group(0) @binding(0) var<storage> arr : array<tint_packed_vec3_f32_array_element, 4u>;

fn f() {
  let x = tint_unpack_vec3_in_composite(arr);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ArrayOfVec3_ReadVector) {
    auto* src = R"(
@group(0) @binding(0) var<storage> arr : array<vec3<f32>, 4>;

fn f() {
  let x = arr[0];
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

@group(0) @binding(0) var<storage> arr : array<tint_packed_vec3_f32_array_element, 4u>;

fn f() {
  let x = vec3<f32>(arr[0].elements);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ArrayOfVec3_ReadComponent_MemberAccessor) {
    auto* src = R"(
@group(0) @binding(0) var<storage> arr : array<vec3<f32>, 4>;

fn f() {
  let x = arr[0].y;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

@group(0) @binding(0) var<storage> arr : array<tint_packed_vec3_f32_array_element, 4u>;

fn f() {
  let x = arr[0].elements.y;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ArrayOfVec3_ReadComponent_IndexAccessor) {
    auto* src = R"(
@group(0) @binding(0) var<storage> arr : array<vec3<f32>, 4>;

fn f() {
  let x = arr[0][1];
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

@group(0) @binding(0) var<storage> arr : array<tint_packed_vec3_f32_array_element, 4u>;

fn f() {
  let x = arr[0].elements[1];
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ArrayOfVec3_WriteArray_ValueRHS) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> arr : array<vec3<f32>, 2>;

fn f() {
  arr = array(vec3(1.5, 2.5, 3.5), vec3(4.5, 5.5, 6.5));
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

fn tint_pack_vec3_in_composite(in : array<vec3<f32>, 2u>) -> array<tint_packed_vec3_f32_array_element, 2u> {
  var result : array<tint_packed_vec3_f32_array_element, 2u>;
  for(var i : u32; (i < 2u); i = (i + 1)) {
    result[i] = tint_packed_vec3_f32_array_element(__packed_vec3<f32>(in[i]));
  }
  return result;
}

@group(0) @binding(0) var<storage, read_write> arr : array<tint_packed_vec3_f32_array_element, 2u>;

fn f() {
  arr = tint_pack_vec3_in_composite(array(vec3(1.5, 2.5, 3.5), vec3(4.5, 5.5, 6.5)));
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ArrayOfVec3_WriteArray_RefRHS) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> arr : array<vec3<f32>, 2>;
@group(0) @binding(1) var<uniform> in : array<vec3<f32>, 2>;

fn f() {
  arr = in;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

@group(0) @binding(0) var<storage, read_write> arr : array<tint_packed_vec3_f32_array_element, 2u>;

@group(0) @binding(1) var<uniform> in : array<tint_packed_vec3_f32_array_element, 2u>;

fn f() {
  arr = in;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ArrayOfVec3_WriteVector_ValueRHS) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> arr : array<vec3<f32>, 4>;

fn f() {
  arr[0] = vec3(1.23);
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

@group(0) @binding(0) var<storage, read_write> arr : array<tint_packed_vec3_f32_array_element, 4u>;

fn f() {
  arr[0].elements = __packed_vec3<f32>(vec3(1.22999999999999998224));
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ArrayOfVec3_WriteVector_RefRHS) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> arr : array<vec3<f32>, 4>;
@group(0) @binding(1) var<uniform> in_arr : array<vec3<f32>, 4>;
@group(0) @binding(2) var<uniform> in_vec : vec3<f32>;

fn f() {
  arr[0] = in_arr[0];
  arr[1] = in_vec;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

@group(0) @binding(0) var<storage, read_write> arr : array<tint_packed_vec3_f32_array_element, 4u>;

@group(0) @binding(1) var<uniform> in_arr : array<tint_packed_vec3_f32_array_element, 4u>;

@group(0) @binding(2) var<uniform> in_vec : __packed_vec3<f32>;

fn f() {
  arr[0].elements = in_arr[0].elements;
  arr[1].elements = in_vec;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ArrayOfVec3_WriteComponent_MemberAccessor) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> arr : array<vec3<f32>, 4>;

fn f() {
  arr[0].y = 1.23;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

@group(0) @binding(0) var<storage, read_write> arr : array<tint_packed_vec3_f32_array_element, 4u>;

fn f() {
  arr[0].elements.y = 1.22999999999999998224;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ArrayOfVec3_WriteComponent_IndexAccessor) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> arr : array<vec3<f32>, 4>;

fn f() {
  arr[0][1] = 1.23;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

@group(0) @binding(0) var<storage, read_write> arr : array<tint_packed_vec3_f32_array_element, 4u>;

fn f() {
  arr[0].elements[1] = 1.22999999999999998224;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, Matrix_ReadMatrix) {
    auto* src = R"(
@group(0) @binding(0) var<storage> m : mat3x3<f32>;

fn f() {
  let x = m;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

fn tint_unpack_vec3_in_composite(in : array<tint_packed_vec3_f32_array_element, 3u>) -> mat3x3<f32> {
  var result : mat3x3<f32>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

@group(0) @binding(0) var<storage> m : array<tint_packed_vec3_f32_array_element, 3u>;

fn f() {
  let x = tint_unpack_vec3_in_composite(m);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, Matrix_ReadColumn) {
    auto* src = R"(
@group(0) @binding(0) var<storage> m : mat3x3<f32>;

fn f() {
  let x = m[1];
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

@group(0) @binding(0) var<storage> m : array<tint_packed_vec3_f32_array_element, 3u>;

fn f() {
  let x = vec3<f32>(m[1].elements);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, Matrix_ReadComponent_MemberAccessChain) {
    auto* src = R"(
@group(0) @binding(0) var<storage> m : mat3x3<f32>;

fn f() {
  let x = m[1].yz.x;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

@group(0) @binding(0) var<storage> m : array<tint_packed_vec3_f32_array_element, 3u>;

fn f() {
  let x = vec3<f32>(m[1].elements).yz.x;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, Matrix_ReadComponent_IndexAccessor) {
    auto* src = R"(
@group(0) @binding(0) var<storage> m : mat3x3<f32>;

fn f() {
  let x = m[2][1];
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

@group(0) @binding(0) var<storage> m : array<tint_packed_vec3_f32_array_element, 3u>;

fn f() {
  let x = m[2].elements[1];
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, Matrix_WriteMatrix_ValueRHS) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> m : mat3x3<f32>;

fn f() {
  m = mat3x3(1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5);
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

fn tint_pack_vec3_in_composite(in : mat3x3<f32>) -> array<tint_packed_vec3_f32_array_element, 3u> {
  var result : array<tint_packed_vec3_f32_array_element, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = tint_packed_vec3_f32_array_element(__packed_vec3<f32>(in[i]));
  }
  return result;
}

@group(0) @binding(0) var<storage, read_write> m : array<tint_packed_vec3_f32_array_element, 3u>;

fn f() {
  m = tint_pack_vec3_in_composite(mat3x3(1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5));
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, Matrix_WriteMatrix_RefRHS) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> m : mat3x3<f32>;
@group(0) @binding(1) var<uniform> in : mat3x3<f32>;

fn f() {
  m = in;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

@group(0) @binding(0) var<storage, read_write> m : array<tint_packed_vec3_f32_array_element, 3u>;

@group(0) @binding(1) var<uniform> in : array<tint_packed_vec3_f32_array_element, 3u>;

fn f() {
  m = in;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, Matrix_WriteColumn_ValueRHS) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> m : mat3x3<f32>;

fn f() {
  m[1] = vec3(1.23);
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

@group(0) @binding(0) var<storage, read_write> m : array<tint_packed_vec3_f32_array_element, 3u>;

fn f() {
  m[1].elements = __packed_vec3<f32>(vec3(1.22999999999999998224));
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, Matrix_WriteColumn_RefRHS) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> m : mat3x3<f32>;
@group(0) @binding(1) var<uniform> in_mat : mat3x3<f32>;
@group(0) @binding(1) var<uniform> in_vec : vec3<f32>;

fn f() {
  m[0] = in_mat[0];
  m[1] = in_vec;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

@group(0) @binding(0) var<storage, read_write> m : array<tint_packed_vec3_f32_array_element, 3u>;

@group(0) @binding(1) var<uniform> in_mat : array<tint_packed_vec3_f32_array_element, 3u>;

@group(0) @binding(1) var<uniform> in_vec : __packed_vec3<f32>;

fn f() {
  m[0].elements = in_mat[0].elements;
  m[1].elements = in_vec;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, Matrix_WriteComponent_MemberAccessor) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> m : mat3x3<f32>;

fn f() {
  m[1].y = 1.23;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

@group(0) @binding(0) var<storage, read_write> m : array<tint_packed_vec3_f32_array_element, 3u>;

fn f() {
  m[1].elements.y = 1.22999999999999998224;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, Matrix_WriteComponent_IndexAccessor) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> m : mat3x3<f32>;

fn f() {
  m[1][2] = 1.23;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

@group(0) @binding(0) var<storage, read_write> m : array<tint_packed_vec3_f32_array_element, 3u>;

fn f() {
  m[1].elements[2] = 1.22999999999999998224;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ArrayOfMatrix_ReadArray) {
    auto* src = R"(
@group(0) @binding(0) var<storage> arr : array<mat3x3<f32>, 4>;

fn f() {
  let x = arr;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

fn tint_unpack_vec3_in_composite(in : array<tint_packed_vec3_f32_array_element, 3u>) -> mat3x3<f32> {
  var result : mat3x3<f32>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_1(in : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>) -> array<mat3x3<f32>, 4u> {
  var result : array<mat3x3<f32>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_unpack_vec3_in_composite(in[i]);
  }
  return result;
}

@group(0) @binding(0) var<storage> arr : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>;

fn f() {
  let x = tint_unpack_vec3_in_composite_1(arr);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ArrayOfMatrix_ReadMatrix) {
    auto* src = R"(
@group(0) @binding(0) var<storage> arr : array<mat3x3<f32>, 4>;

fn f() {
  let x = arr[0];
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

fn tint_unpack_vec3_in_composite(in : array<tint_packed_vec3_f32_array_element, 3u>) -> mat3x3<f32> {
  var result : mat3x3<f32>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

@group(0) @binding(0) var<storage> arr : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>;

fn f() {
  let x = tint_unpack_vec3_in_composite(arr[0]);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ArrayOfMatrix_ReadColumn) {
    auto* src = R"(
@group(0) @binding(0) var<storage> arr : array<mat3x3<f32>, 4>;

fn f() {
  let x = arr[0][1];
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

@group(0) @binding(0) var<storage> arr : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>;

fn f() {
  let x = vec3<f32>(arr[0][1].elements);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ArrayOfMatrix_ReadComponent_MemberAccessor) {
    auto* src = R"(
@group(0) @binding(0) var<storage> arr : array<mat3x3<f32>, 4>;

fn f() {
  let x = arr[0][1].y;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

@group(0) @binding(0) var<storage> arr : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>;

fn f() {
  let x = arr[0][1].elements.y;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ArrayOfMatrix_ReadComponent_IndexAccessor) {
    auto* src = R"(
@group(0) @binding(0) var<storage> arr : array<mat3x3<f32>, 4>;

fn f() {
  let x = arr[0][1][2];
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

@group(0) @binding(0) var<storage> arr : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>;

fn f() {
  let x = arr[0][1].elements[2];
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ArrayOfMatrix_WriteArray_ValueRHS) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> arr : array<mat3x3<f32>, 2>;

fn f() {
  arr = array(mat3x3<f32>(), mat3x3(1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5));
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

fn tint_pack_vec3_in_composite(in : mat3x3<f32>) -> array<tint_packed_vec3_f32_array_element, 3u> {
  var result : array<tint_packed_vec3_f32_array_element, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = tint_packed_vec3_f32_array_element(__packed_vec3<f32>(in[i]));
  }
  return result;
}

fn tint_pack_vec3_in_composite_1(in : array<mat3x3<f32>, 2u>) -> array<array<tint_packed_vec3_f32_array_element, 3u>, 2u> {
  var result : array<array<tint_packed_vec3_f32_array_element, 3u>, 2u>;
  for(var i : u32; (i < 2u); i = (i + 1)) {
    result[i] = tint_pack_vec3_in_composite(in[i]);
  }
  return result;
}

@group(0) @binding(0) var<storage, read_write> arr : array<array<tint_packed_vec3_f32_array_element, 3u>, 2u>;

fn f() {
  arr = tint_pack_vec3_in_composite_1(array(mat3x3<f32>(), mat3x3(1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5)));
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ArrayOfMatrix_WriteArray_RefRHS) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> arr : array<mat3x3<f32>, 2>;
@group(0) @binding(1) var<uniform> in : array<mat3x3<f32>, 2>;

fn f() {
  arr = in;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

@group(0) @binding(0) var<storage, read_write> arr : array<array<tint_packed_vec3_f32_array_element, 3u>, 2u>;

@group(0) @binding(1) var<uniform> in : array<array<tint_packed_vec3_f32_array_element, 3u>, 2u>;

fn f() {
  arr = in;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ArrayOfMatrix_WriteMatrix_ValueRHS) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> arr : array<mat3x3<f32>, 4>;

fn f() {
  arr[0] = mat3x3(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9);
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

fn tint_pack_vec3_in_composite(in : mat3x3<f32>) -> array<tint_packed_vec3_f32_array_element, 3u> {
  var result : array<tint_packed_vec3_f32_array_element, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = tint_packed_vec3_f32_array_element(__packed_vec3<f32>(in[i]));
  }
  return result;
}

@group(0) @binding(0) var<storage, read_write> arr : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>;

fn f() {
  arr[0] = tint_pack_vec3_in_composite(mat3x3(1.10000000000000008882, 2.20000000000000017764, 3.29999999999999982236, 4.40000000000000035527, 5.5, 6.59999999999999964473, 7.70000000000000017764, 8.80000000000000071054, 9.90000000000000035527));
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ArrayOfMatrix_WriteMatrix_RefRHS) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> arr : array<mat3x3<f32>, 4>;
@group(0) @binding(1) var<uniform> in_arr : array<mat3x3<f32>, 4>;
@group(0) @binding(2) var<uniform> in_mat : mat3x3<f32>;

fn f() {
  arr[0] = in_arr[0];
  arr[1] = in_mat;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

@group(0) @binding(0) var<storage, read_write> arr : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>;

@group(0) @binding(1) var<uniform> in_arr : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>;

@group(0) @binding(2) var<uniform> in_mat : array<tint_packed_vec3_f32_array_element, 3u>;

fn f() {
  arr[0] = in_arr[0];
  arr[1] = in_mat;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ArrayOfMatrix_WriteVector_ValueRHS) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> arr : array<mat3x3<f32>, 4>;

fn f() {
  arr[0][1] = vec3(1.23);
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

@group(0) @binding(0) var<storage, read_write> arr : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>;

fn f() {
  arr[0][1].elements = __packed_vec3<f32>(vec3(1.22999999999999998224));
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ArrayOfMatrix_WriteVector_RefRHS) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> arr : array<mat3x3<f32>, 4>;
@group(0) @binding(1) var<uniform> in_arr : array<mat3x3<f32>, 4>;
@group(0) @binding(2) var<uniform> in_mat : mat3x3<f32>;
@group(0) @binding(3) var<uniform> in_vec : vec3<f32>;

fn f() {
  arr[0][0] = arr[0][1];
  arr[0][1] = in_mat[2];
  arr[0][2] = in_vec;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

@group(0) @binding(0) var<storage, read_write> arr : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>;

@group(0) @binding(1) var<uniform> in_arr : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>;

@group(0) @binding(2) var<uniform> in_mat : array<tint_packed_vec3_f32_array_element, 3u>;

@group(0) @binding(3) var<uniform> in_vec : __packed_vec3<f32>;

fn f() {
  arr[0][0].elements = arr[0][1].elements;
  arr[0][1].elements = in_mat[2].elements;
  arr[0][2].elements = in_vec;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ArrayOfMatrix_WriteComponent_MemberAccessor) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> arr : array<mat3x3<f32>, 4>;

fn f() {
  arr[0][1].y = 1.23;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

@group(0) @binding(0) var<storage, read_write> arr : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>;

fn f() {
  arr[0][1].elements.y = 1.22999999999999998224;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ArrayOfMatrix_WriteComponent_IndexAccessor) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> arr : array<mat3x3<f32>, 4>;

fn f() {
  arr[0][1][2] = 1.23;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

@group(0) @binding(0) var<storage, read_write> arr : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>;

fn f() {
  arr[0][1].elements[2] = 1.22999999999999998224;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_Vec3_ReadStruct) {
    auto* src = R"(
struct S {
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let x = P;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct S_tint_packed_vec3 {
  @align(16)
  v : __packed_vec3<f32>,
}

fn tint_unpack_vec3_in_composite(in : S_tint_packed_vec3) -> S {
  var result : S;
  result.v = vec3<f32>(in.v);
  return result;
}

struct S {
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage> P : S_tint_packed_vec3;

fn f() {
  let x = tint_unpack_vec3_in_composite(P);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_Vec3_ReadVector) {
    auto* src = R"(
struct S {
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let x = P.v;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct S_tint_packed_vec3 {
  @align(16)
  v : __packed_vec3<f32>,
}

struct S {
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage> P : S_tint_packed_vec3;

fn f() {
  let x = vec3<f32>(P.v);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_Vec3_ReadComponent_MemberAccessChain) {
    auto* src = R"(
struct S {
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let x = P.v.yz.x;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct S_tint_packed_vec3 {
  @align(16)
  v : __packed_vec3<f32>,
}

struct S {
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage> P : S_tint_packed_vec3;

fn f() {
  let x = vec3<f32>(P.v).yz.x;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_Vec3_ReadComponent_IndexAccessor) {
    auto* src = R"(
struct S {
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let x = P.v[1];
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct S_tint_packed_vec3 {
  @align(16)
  v : __packed_vec3<f32>,
}

struct S {
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage> P : S_tint_packed_vec3;

fn f() {
  let x = P.v[1];
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_Vec3_WriteStruct_ValueRHS) {
    auto* src = R"(
struct S {
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage, read_write> P : S;

fn f() {
  P = S(vec3(1.23));
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct S_tint_packed_vec3 {
  @align(16)
  v : __packed_vec3<f32>,
}

fn tint_pack_vec3_in_composite(in : S) -> S_tint_packed_vec3 {
  var result : S_tint_packed_vec3;
  result.v = __packed_vec3<f32>(in.v);
  return result;
}

struct S {
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage, read_write> P : S_tint_packed_vec3;

fn f() {
  P = tint_pack_vec3_in_composite(S(vec3(1.22999999999999998224)));
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_Vec3_WriteStruct_RefRHS) {
    auto* src = R"(
struct S {
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage, read_write> P : S;
@group(0) @binding(1) var<uniform> in : S;

fn f() {
  P = in;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct S_tint_packed_vec3 {
  @align(16)
  v : __packed_vec3<f32>,
}

struct S {
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage, read_write> P : S_tint_packed_vec3;

@group(0) @binding(1) var<uniform> in : S_tint_packed_vec3;

fn f() {
  P = in;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_Vec3_WriteVector_ValueRHS) {
    auto* src = R"(
struct S {
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage, read_write> P : S;

fn f() {
  P.v = vec3(1.23);
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct S_tint_packed_vec3 {
  @align(16)
  v : __packed_vec3<f32>,
}

struct S {
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage, read_write> P : S_tint_packed_vec3;

fn f() {
  P.v = __packed_vec3<f32>(vec3(1.22999999999999998224));
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_Vec3_WriteVector_RefRHS) {
    auto* src = R"(
struct S {
  v1 : vec3<f32>,
  v2 : vec3<f32>,
}

@group(0) @binding(0) var<storage, read_write> P : S;
@group(0) @binding(1) var<uniform> in_str : S;
@group(0) @binding(2) var<uniform> in_vec : vec3<f32>;

fn f() {
  P.v1 = in_str.v1;
  P.v2 = in_vec;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct S_tint_packed_vec3 {
  @align(16)
  v1 : __packed_vec3<f32>,
  @align(16)
  v2 : __packed_vec3<f32>,
}

struct S {
  v1 : vec3<f32>,
  v2 : vec3<f32>,
}

@group(0) @binding(0) var<storage, read_write> P : S_tint_packed_vec3;

@group(0) @binding(1) var<uniform> in_str : S_tint_packed_vec3;

@group(0) @binding(2) var<uniform> in_vec : __packed_vec3<f32>;

fn f() {
  P.v1 = in_str.v1;
  P.v2 = in_vec;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_Vec3_WriteComponent_MemberAccessor) {
    auto* src = R"(
struct S {
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage, read_write> P : S;

fn f() {
  P.v.y = 1.23;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct S_tint_packed_vec3 {
  @align(16)
  v : __packed_vec3<f32>,
}

struct S {
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage, read_write> P : S_tint_packed_vec3;

fn f() {
  P.v.y = 1.22999999999999998224;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_Vec3_WriteComponent_IndexAccessor) {
    auto* src = R"(
struct S {
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage, read_write> P : S;

fn f() {
  P.v[1] = 1.23;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct S_tint_packed_vec3 {
  @align(16)
  v : __packed_vec3<f32>,
}

struct S {
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage, read_write> P : S_tint_packed_vec3;

fn f() {
  P.v[1] = 1.22999999999999998224;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_ArrayOfVec3_ReadStruct) {
    auto* src = R"(
struct S {
  arr : array<vec3<f32>, 4>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let x = P;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr : array<tint_packed_vec3_f32_array_element, 4u>,
}

fn tint_unpack_vec3_in_composite(in : array<tint_packed_vec3_f32_array_element, 4u>) -> array<vec3<f32>, 4u> {
  var result : array<vec3<f32>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_1(in : S_tint_packed_vec3) -> S {
  var result : S;
  result.arr = tint_unpack_vec3_in_composite(in.arr);
  return result;
}

struct S {
  arr : array<vec3<f32>, 4>,
}

@group(0) @binding(0) var<storage> P : S_tint_packed_vec3;

fn f() {
  let x = tint_unpack_vec3_in_composite_1(P);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_ArrayOfVec3_ReadArray) {
    auto* src = R"(
struct S {
  arr : array<vec3<f32>, 4>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let x = P.arr;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr : array<tint_packed_vec3_f32_array_element, 4u>,
}

fn tint_unpack_vec3_in_composite(in : array<tint_packed_vec3_f32_array_element, 4u>) -> array<vec3<f32>, 4u> {
  var result : array<vec3<f32>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

struct S {
  arr : array<vec3<f32>, 4>,
}

@group(0) @binding(0) var<storage> P : S_tint_packed_vec3;

fn f() {
  let x = tint_unpack_vec3_in_composite(P.arr);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_ArrayOfVec3_ReadVector) {
    auto* src = R"(
struct S {
  arr : array<vec3<f32>, 4>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let x = P.arr[0];
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr : array<tint_packed_vec3_f32_array_element, 4u>,
}

struct S {
  arr : array<vec3<f32>, 4>,
}

@group(0) @binding(0) var<storage> P : S_tint_packed_vec3;

fn f() {
  let x = vec3<f32>(P.arr[0].elements);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_ArrayOfVec3_ReadComponent_MemberAccessor) {
    auto* src = R"(
struct S {
  arr : array<vec3<f32>, 4>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let x = P.arr[0].y;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr : array<tint_packed_vec3_f32_array_element, 4u>,
}

struct S {
  arr : array<vec3<f32>, 4>,
}

@group(0) @binding(0) var<storage> P : S_tint_packed_vec3;

fn f() {
  let x = P.arr[0].elements.y;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_ArrayOfVec3_ReadComponent_IndexAccessor) {
    auto* src = R"(
struct S {
  arr : array<vec3<f32>, 4>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let x = P.arr[0][1];
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr : array<tint_packed_vec3_f32_array_element, 4u>,
}

struct S {
  arr : array<vec3<f32>, 4>,
}

@group(0) @binding(0) var<storage> P : S_tint_packed_vec3;

fn f() {
  let x = P.arr[0].elements[1];
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_ArrayOfVec3_WriteStruct_ValueRHS) {
    auto* src = R"(
struct S {
  arr : array<vec3<f32>, 2>,
}

@group(0) @binding(0) var<storage, read_write> P : S;

fn f() {
  P = S(array(vec3(1.5, 4.5, 7.5), vec3(9.5, 6.5, 3.5)));
}
)";

    auto* expect =
        R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr : array<tint_packed_vec3_f32_array_element, 2u>,
}

fn tint_pack_vec3_in_composite(in : array<vec3<f32>, 2u>) -> array<tint_packed_vec3_f32_array_element, 2u> {
  var result : array<tint_packed_vec3_f32_array_element, 2u>;
  for(var i : u32; (i < 2u); i = (i + 1)) {
    result[i] = tint_packed_vec3_f32_array_element(__packed_vec3<f32>(in[i]));
  }
  return result;
}

fn tint_pack_vec3_in_composite_1(in : S) -> S_tint_packed_vec3 {
  var result : S_tint_packed_vec3;
  result.arr = tint_pack_vec3_in_composite(in.arr);
  return result;
}

struct S {
  arr : array<vec3<f32>, 2>,
}

@group(0) @binding(0) var<storage, read_write> P : S_tint_packed_vec3;

fn f() {
  P = tint_pack_vec3_in_composite_1(S(array(vec3(1.5, 4.5, 7.5), vec3(9.5, 6.5, 3.5))));
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_ArrayOfVec3_WriteStruct_RefRHS) {
    auto* src = R"(
struct S {
  arr : array<vec3<f32>, 2>,
}

@group(0) @binding(0) var<storage, read_write> P : S;
@group(0) @binding(1) var<uniform> in : S;

fn f() {
  P = in;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr : array<tint_packed_vec3_f32_array_element, 2u>,
}

struct S {
  arr : array<vec3<f32>, 2>,
}

@group(0) @binding(0) var<storage, read_write> P : S_tint_packed_vec3;

@group(0) @binding(1) var<uniform> in : S_tint_packed_vec3;

fn f() {
  P = in;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_ArrayOfVec3_WriteArray_ValueRHS) {
    auto* src = R"(
struct S {
  arr : array<vec3<f32>, 2>,
}

@group(0) @binding(0) var<storage, read_write> P : S;

fn f() {
  P.arr = array(vec3(1.5, 4.5, 7.5), vec3(9.5, 6.5, 3.5));
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr : array<tint_packed_vec3_f32_array_element, 2u>,
}

fn tint_pack_vec3_in_composite(in : array<vec3<f32>, 2u>) -> array<tint_packed_vec3_f32_array_element, 2u> {
  var result : array<tint_packed_vec3_f32_array_element, 2u>;
  for(var i : u32; (i < 2u); i = (i + 1)) {
    result[i] = tint_packed_vec3_f32_array_element(__packed_vec3<f32>(in[i]));
  }
  return result;
}

struct S {
  arr : array<vec3<f32>, 2>,
}

@group(0) @binding(0) var<storage, read_write> P : S_tint_packed_vec3;

fn f() {
  P.arr = tint_pack_vec3_in_composite(array(vec3(1.5, 4.5, 7.5), vec3(9.5, 6.5, 3.5)));
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_ArrayOfVec3_WriteArray_RefRHS) {
    auto* src = R"(
struct S {
  arr1 : array<vec3<f32>, 2>,
  arr2 : array<vec3<f32>, 2>,
}

@group(0) @binding(0) var<storage, read_write> P : S;
@group(0) @binding(1) var<uniform> in_str : S;
@group(0) @binding(2) var<uniform> in_arr : array<vec3<f32>, 2>;

fn f() {
  P.arr1 = in_str.arr1;
  P.arr2 = in_arr;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr1 : array<tint_packed_vec3_f32_array_element, 2u>,
  @align(16)
  arr2 : array<tint_packed_vec3_f32_array_element, 2u>,
}

struct S {
  arr1 : array<vec3<f32>, 2>,
  arr2 : array<vec3<f32>, 2>,
}

@group(0) @binding(0) var<storage, read_write> P : S_tint_packed_vec3;

@group(0) @binding(1) var<uniform> in_str : S_tint_packed_vec3;

@group(0) @binding(2) var<uniform> in_arr : array<tint_packed_vec3_f32_array_element, 2u>;

fn f() {
  P.arr1 = in_str.arr1;
  P.arr2 = in_arr;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_ArrayOfVec3_WriteVector_ValueRHS) {
    auto* src = R"(
struct S {
  arr : array<vec3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> P : S;

fn f() {
  P.arr[0] = vec3(1.23);
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr : array<tint_packed_vec3_f32_array_element, 4u>,
}

struct S {
  arr : array<vec3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> P : S_tint_packed_vec3;

fn f() {
  P.arr[0].elements = __packed_vec3<f32>(vec3(1.22999999999999998224));
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_ArrayOfVec3_WriteVector_RefRHS) {
    auto* src = R"(
struct S {
  arr : array<vec3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> P : S;
@group(0) @binding(1) var<uniform> in_str : S;
@group(0) @binding(2) var<uniform> in_arr : array<vec3<f32>, 4>;
@group(0) @binding(3) var<uniform> in_vec : vec3<f32>;

fn f() {
  P.arr[0] = in_str.arr[0];
  P.arr[1] = in_arr[1];
  P.arr[2] = in_vec;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr : array<tint_packed_vec3_f32_array_element, 4u>,
}

struct S {
  arr : array<vec3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> P : S_tint_packed_vec3;

@group(0) @binding(1) var<uniform> in_str : S_tint_packed_vec3;

@group(0) @binding(2) var<uniform> in_arr : array<tint_packed_vec3_f32_array_element, 4u>;

@group(0) @binding(3) var<uniform> in_vec : __packed_vec3<f32>;

fn f() {
  P.arr[0].elements = in_str.arr[0].elements;
  P.arr[1].elements = in_arr[1].elements;
  P.arr[2].elements = in_vec;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_ArrayOfVec3_WriteComponent_MemberAccessor) {
    auto* src = R"(
struct S {
  arr : array<vec3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> P : S;

fn f() {
  P.arr[0].y = 1.23;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr : array<tint_packed_vec3_f32_array_element, 4u>,
}

struct S {
  arr : array<vec3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> P : S_tint_packed_vec3;

fn f() {
  P.arr[0].elements.y = 1.22999999999999998224;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_ArrayOfVec3_WriteComponent_IndexAccessor) {
    auto* src = R"(
struct S {
  arr : array<vec3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> P : S;

fn f() {
  P.arr[0][1] = 1.23;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr : array<tint_packed_vec3_f32_array_element, 4u>,
}

struct S {
  arr : array<vec3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> P : S_tint_packed_vec3;

fn f() {
  P.arr[0].elements[1] = 1.22999999999999998224;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_Matrix_ReadStruct) {
    auto* src = R"(
struct S {
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let x = P;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  m : array<tint_packed_vec3_f32_array_element, 3u>,
}

fn tint_unpack_vec3_in_composite(in : array<tint_packed_vec3_f32_array_element, 3u>) -> mat3x3<f32> {
  var result : mat3x3<f32>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_1(in : S_tint_packed_vec3) -> S {
  var result : S;
  result.m = tint_unpack_vec3_in_composite(in.m);
  return result;
}

struct S {
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<storage> P : S_tint_packed_vec3;

fn f() {
  let x = tint_unpack_vec3_in_composite_1(P);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_Matrix_ReadMatrix) {
    auto* src = R"(
struct S {
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let x = P.m;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  m : array<tint_packed_vec3_f32_array_element, 3u>,
}

fn tint_unpack_vec3_in_composite(in : array<tint_packed_vec3_f32_array_element, 3u>) -> mat3x3<f32> {
  var result : mat3x3<f32>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

struct S {
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<storage> P : S_tint_packed_vec3;

fn f() {
  let x = tint_unpack_vec3_in_composite(P.m);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_Matrix_ReadColumn) {
    auto* src = R"(
struct S {
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let x = P.m[1];
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  m : array<tint_packed_vec3_f32_array_element, 3u>,
}

struct S {
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<storage> P : S_tint_packed_vec3;

fn f() {
  let x = vec3<f32>(P.m[1].elements);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_Matrix_ReadComponent_MemberAccessChain) {
    auto* src = R"(
struct S {
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let x = P.m[1].yz.x;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  m : array<tint_packed_vec3_f32_array_element, 3u>,
}

struct S {
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<storage> P : S_tint_packed_vec3;

fn f() {
  let x = vec3<f32>(P.m[1].elements).yz.x;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_Matrix_ReadComponent_IndexAccessor) {
    auto* src = R"(
struct S {
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let x = P.m[2][1];
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  m : array<tint_packed_vec3_f32_array_element, 3u>,
}

struct S {
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<storage> P : S_tint_packed_vec3;

fn f() {
  let x = P.m[2].elements[1];
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_Matrix_WriteStruct_ValueRHS) {
    auto* src = R"(
struct S {
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<storage, read_write> P : S;

fn f() {
  P = S(mat3x3(1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5));
}
)";

    auto* expect =
        R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  m : array<tint_packed_vec3_f32_array_element, 3u>,
}

fn tint_pack_vec3_in_composite(in : mat3x3<f32>) -> array<tint_packed_vec3_f32_array_element, 3u> {
  var result : array<tint_packed_vec3_f32_array_element, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = tint_packed_vec3_f32_array_element(__packed_vec3<f32>(in[i]));
  }
  return result;
}

fn tint_pack_vec3_in_composite_1(in : S) -> S_tint_packed_vec3 {
  var result : S_tint_packed_vec3;
  result.m = tint_pack_vec3_in_composite(in.m);
  return result;
}

struct S {
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<storage, read_write> P : S_tint_packed_vec3;

fn f() {
  P = tint_pack_vec3_in_composite_1(S(mat3x3(1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5)));
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_Matrix_WriteStruct_RefRHS) {
    auto* src = R"(
struct S {
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<storage, read_write> P : S;
@group(0) @binding(1) var<uniform> in : S;

fn f() {
  P = in;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  m : array<tint_packed_vec3_f32_array_element, 3u>,
}

struct S {
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<storage, read_write> P : S_tint_packed_vec3;

@group(0) @binding(1) var<uniform> in : S_tint_packed_vec3;

fn f() {
  P = in;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_Matrix_WriteMatrix_ValueRHS) {
    auto* src = R"(
struct S {
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<storage, read_write> P : S;

fn f() {
  P.m = mat3x3(1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5);
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  m : array<tint_packed_vec3_f32_array_element, 3u>,
}

fn tint_pack_vec3_in_composite(in : mat3x3<f32>) -> array<tint_packed_vec3_f32_array_element, 3u> {
  var result : array<tint_packed_vec3_f32_array_element, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = tint_packed_vec3_f32_array_element(__packed_vec3<f32>(in[i]));
  }
  return result;
}

struct S {
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<storage, read_write> P : S_tint_packed_vec3;

fn f() {
  P.m = tint_pack_vec3_in_composite(mat3x3(1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5));
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_Matrix_WriteMatrix_RefRHS) {
    auto* src = R"(
struct S {
  m1 : mat3x3<f32>,
  m2 : mat3x3<f32>,
}

@group(0) @binding(0) var<storage, read_write> P : S;
@group(0) @binding(1) var<uniform> in_str : S;
@group(0) @binding(2) var<uniform> in_mat : mat3x3<f32>;

fn f() {
  P.m1 = in_str.m1;
  P.m2 = in_mat;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  m1 : array<tint_packed_vec3_f32_array_element, 3u>,
  @align(16)
  m2 : array<tint_packed_vec3_f32_array_element, 3u>,
}

struct S {
  m1 : mat3x3<f32>,
  m2 : mat3x3<f32>,
}

@group(0) @binding(0) var<storage, read_write> P : S_tint_packed_vec3;

@group(0) @binding(1) var<uniform> in_str : S_tint_packed_vec3;

@group(0) @binding(2) var<uniform> in_mat : array<tint_packed_vec3_f32_array_element, 3u>;

fn f() {
  P.m1 = in_str.m1;
  P.m2 = in_mat;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_Matrix_WriteColumn_ValueRHS) {
    auto* src = R"(
struct S {
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<storage, read_write> P : S;

fn f() {
  P.m[1] = vec3(1.23);
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  m : array<tint_packed_vec3_f32_array_element, 3u>,
}

struct S {
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<storage, read_write> P : S_tint_packed_vec3;

fn f() {
  P.m[1].elements = __packed_vec3<f32>(vec3(1.22999999999999998224));
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_Matrix_WriteColumn_RefRHS) {
    auto* src = R"(
struct S {
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<storage, read_write> P : S;
@group(0) @binding(1) var<uniform> in_str : S;
@group(0) @binding(2) var<uniform> in_mat : mat3x3<f32>;
@group(0) @binding(3) var<uniform> in_vec : vec3<f32>;

fn f() {
  P.m[0] = in_str.m[0];
  P.m[1] = in_mat[1];
  P.m[2] = in_vec;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  m : array<tint_packed_vec3_f32_array_element, 3u>,
}

struct S {
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<storage, read_write> P : S_tint_packed_vec3;

@group(0) @binding(1) var<uniform> in_str : S_tint_packed_vec3;

@group(0) @binding(2) var<uniform> in_mat : array<tint_packed_vec3_f32_array_element, 3u>;

@group(0) @binding(3) var<uniform> in_vec : __packed_vec3<f32>;

fn f() {
  P.m[0].elements = in_str.m[0].elements;
  P.m[1].elements = in_mat[1].elements;
  P.m[2].elements = in_vec;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_Matrix_WriteComponent_MemberAccessor) {
    auto* src = R"(
struct S {
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<storage, read_write> P : S;

fn f() {
  P.m[1].y = 1.23;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  m : array<tint_packed_vec3_f32_array_element, 3u>,
}

struct S {
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<storage, read_write> P : S_tint_packed_vec3;

fn f() {
  P.m[1].elements.y = 1.22999999999999998224;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_Matrix_WriteComponent_IndexAccessor) {
    auto* src = R"(
struct S {
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<storage, read_write> P : S;

fn f() {
  P.m[1][2] = 1.23;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  m : array<tint_packed_vec3_f32_array_element, 3u>,
}

struct S {
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<storage, read_write> P : S_tint_packed_vec3;

fn f() {
  P.m[1].elements[2] = 1.22999999999999998224;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_ArrayOfMatrix_ReadStruct) {
    auto* src = R"(
struct S {
  arr : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let x = P;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>,
}

fn tint_unpack_vec3_in_composite(in : array<tint_packed_vec3_f32_array_element, 3u>) -> mat3x3<f32> {
  var result : mat3x3<f32>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_1(in : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>) -> array<mat3x3<f32>, 4u> {
  var result : array<mat3x3<f32>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_unpack_vec3_in_composite(in[i]);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_2(in : S_tint_packed_vec3) -> S {
  var result : S;
  result.arr = tint_unpack_vec3_in_composite_1(in.arr);
  return result;
}

struct S {
  arr : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<storage> P : S_tint_packed_vec3;

fn f() {
  let x = tint_unpack_vec3_in_composite_2(P);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_ArrayOfMatrix_ReadArray) {
    auto* src = R"(
struct S {
  arr : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let x = P.arr;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>,
}

fn tint_unpack_vec3_in_composite(in : array<tint_packed_vec3_f32_array_element, 3u>) -> mat3x3<f32> {
  var result : mat3x3<f32>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_1(in : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>) -> array<mat3x3<f32>, 4u> {
  var result : array<mat3x3<f32>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_unpack_vec3_in_composite(in[i]);
  }
  return result;
}

struct S {
  arr : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<storage> P : S_tint_packed_vec3;

fn f() {
  let x = tint_unpack_vec3_in_composite_1(P.arr);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_ArrayOfMatrix_ReadMatrix) {
    auto* src = R"(
struct S {
  arr : array<mat3x3<f32>, 4u>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let x = P.arr[0];
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>,
}

fn tint_unpack_vec3_in_composite(in : array<tint_packed_vec3_f32_array_element, 3u>) -> mat3x3<f32> {
  var result : mat3x3<f32>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

struct S {
  arr : array<mat3x3<f32>, 4u>,
}

@group(0) @binding(0) var<storage> P : S_tint_packed_vec3;

fn f() {
  let x = tint_unpack_vec3_in_composite(P.arr[0]);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_ArrayOfMatrix_ReadColumn) {
    auto* src = R"(
struct S {
  arr : array<mat3x3<f32>, 4u>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let x = P.arr[0][1];
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>,
}

struct S {
  arr : array<mat3x3<f32>, 4u>,
}

@group(0) @binding(0) var<storage> P : S_tint_packed_vec3;

fn f() {
  let x = vec3<f32>(P.arr[0][1].elements);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_ArrayOfMatrix_ReadComponent_MemberAccessor) {
    auto* src = R"(
struct S {
  arr : array<mat3x3<f32>, 4u>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let x = P.arr[0][1].y;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>,
}

struct S {
  arr : array<mat3x3<f32>, 4u>,
}

@group(0) @binding(0) var<storage> P : S_tint_packed_vec3;

fn f() {
  let x = P.arr[0][1].elements.y;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_ArrayOfMatrix_ReadComponent_IndexAccessor) {
    auto* src = R"(
struct S {
  arr : array<mat3x3<f32>, 4u>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let x = P.arr[0][1][2];
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>,
}

struct S {
  arr : array<mat3x3<f32>, 4u>,
}

@group(0) @binding(0) var<storage> P : S_tint_packed_vec3;

fn f() {
  let x = P.arr[0][1].elements[2];
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_ArrayOfMatrix_WriteStruct_ValueRHS) {
    auto* src = R"(
struct S {
  arr : array<mat3x3<f32>, 2>,
}

@group(0) @binding(0) var<storage, read_write> P : S;

fn f() {
  P = S(array(mat3x3<f32>(), mat3x3(1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5)));
}
)";

    auto* expect =
        R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr : array<array<tint_packed_vec3_f32_array_element, 3u>, 2u>,
}

fn tint_pack_vec3_in_composite(in : mat3x3<f32>) -> array<tint_packed_vec3_f32_array_element, 3u> {
  var result : array<tint_packed_vec3_f32_array_element, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = tint_packed_vec3_f32_array_element(__packed_vec3<f32>(in[i]));
  }
  return result;
}

fn tint_pack_vec3_in_composite_1(in : array<mat3x3<f32>, 2u>) -> array<array<tint_packed_vec3_f32_array_element, 3u>, 2u> {
  var result : array<array<tint_packed_vec3_f32_array_element, 3u>, 2u>;
  for(var i : u32; (i < 2u); i = (i + 1)) {
    result[i] = tint_pack_vec3_in_composite(in[i]);
  }
  return result;
}

fn tint_pack_vec3_in_composite_2(in : S) -> S_tint_packed_vec3 {
  var result : S_tint_packed_vec3;
  result.arr = tint_pack_vec3_in_composite_1(in.arr);
  return result;
}

struct S {
  arr : array<mat3x3<f32>, 2>,
}

@group(0) @binding(0) var<storage, read_write> P : S_tint_packed_vec3;

fn f() {
  P = tint_pack_vec3_in_composite_2(S(array(mat3x3<f32>(), mat3x3(1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5))));
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_ArrayOfMatrix_WriteStruct_RefRHS) {
    auto* src = R"(
struct S {
  arr : array<mat3x3<f32>, 2>,
}

@group(0) @binding(0) var<storage, read_write> P : S;
@group(0) @binding(1) var<uniform> in : S;

fn f() {
  P = in;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr : array<array<tint_packed_vec3_f32_array_element, 3u>, 2u>,
}

struct S {
  arr : array<mat3x3<f32>, 2>,
}

@group(0) @binding(0) var<storage, read_write> P : S_tint_packed_vec3;

@group(0) @binding(1) var<uniform> in : S_tint_packed_vec3;

fn f() {
  P = in;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_ArrayOfMatrix_WriteArray_ValueRHS) {
    auto* src = R"(
struct S {
  arr : array<mat3x3<f32>, 2>,
}

@group(0) @binding(0) var<storage, read_write> P : S;

fn f() {
  P.arr = array(mat3x3<f32>(), mat3x3(1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5));
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr : array<array<tint_packed_vec3_f32_array_element, 3u>, 2u>,
}

fn tint_pack_vec3_in_composite(in : mat3x3<f32>) -> array<tint_packed_vec3_f32_array_element, 3u> {
  var result : array<tint_packed_vec3_f32_array_element, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = tint_packed_vec3_f32_array_element(__packed_vec3<f32>(in[i]));
  }
  return result;
}

fn tint_pack_vec3_in_composite_1(in : array<mat3x3<f32>, 2u>) -> array<array<tint_packed_vec3_f32_array_element, 3u>, 2u> {
  var result : array<array<tint_packed_vec3_f32_array_element, 3u>, 2u>;
  for(var i : u32; (i < 2u); i = (i + 1)) {
    result[i] = tint_pack_vec3_in_composite(in[i]);
  }
  return result;
}

struct S {
  arr : array<mat3x3<f32>, 2>,
}

@group(0) @binding(0) var<storage, read_write> P : S_tint_packed_vec3;

fn f() {
  P.arr = tint_pack_vec3_in_composite_1(array(mat3x3<f32>(), mat3x3(1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5)));
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_ArrayOfMatrix_WriteArray_RefRHS) {
    auto* src = R"(
struct S {
  arr1 : array<mat3x3<f32>, 2>,
  arr2 : array<mat3x3<f32>, 2>,
}

@group(0) @binding(0) var<storage, read_write> P : S;
@group(0) @binding(1) var<uniform> in_str : S;
@group(0) @binding(2) var<uniform> in_arr : array<mat3x3<f32>, 2>;

fn f() {
  P.arr1 = in_str.arr1;
  P.arr2 = in_arr;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr1 : array<array<tint_packed_vec3_f32_array_element, 3u>, 2u>,
  @align(16)
  arr2 : array<array<tint_packed_vec3_f32_array_element, 3u>, 2u>,
}

struct S {
  arr1 : array<mat3x3<f32>, 2>,
  arr2 : array<mat3x3<f32>, 2>,
}

@group(0) @binding(0) var<storage, read_write> P : S_tint_packed_vec3;

@group(0) @binding(1) var<uniform> in_str : S_tint_packed_vec3;

@group(0) @binding(2) var<uniform> in_arr : array<array<tint_packed_vec3_f32_array_element, 3u>, 2u>;

fn f() {
  P.arr1 = in_str.arr1;
  P.arr2 = in_arr;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_ArrayOfMatrix_WriteMatrix_ValueRHS) {
    auto* src = R"(
struct S {
  arr : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> P : S;

fn f() {
  P.arr[0] = mat3x3(1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5);
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>,
}

fn tint_pack_vec3_in_composite(in : mat3x3<f32>) -> array<tint_packed_vec3_f32_array_element, 3u> {
  var result : array<tint_packed_vec3_f32_array_element, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = tint_packed_vec3_f32_array_element(__packed_vec3<f32>(in[i]));
  }
  return result;
}

struct S {
  arr : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> P : S_tint_packed_vec3;

fn f() {
  P.arr[0] = tint_pack_vec3_in_composite(mat3x3(1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5));
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_ArrayOfMatrix_WriteMatrix_RefRHS) {
    auto* src = R"(
struct S {
  arr : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> P : S;
@group(0) @binding(1) var<uniform> in_str : S;
@group(0) @binding(2) var<uniform> in_arr : array<mat3x3<f32>, 4>;
@group(0) @binding(3) var<uniform> in_mat : mat3x3<f32>;

fn f() {
  P.arr[0] = in_str.arr[0];
  P.arr[1] = in_arr[1];
  P.arr[2] = in_mat;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>,
}

struct S {
  arr : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> P : S_tint_packed_vec3;

@group(0) @binding(1) var<uniform> in_str : S_tint_packed_vec3;

@group(0) @binding(2) var<uniform> in_arr : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>;

@group(0) @binding(3) var<uniform> in_mat : array<tint_packed_vec3_f32_array_element, 3u>;

fn f() {
  P.arr[0] = in_str.arr[0];
  P.arr[1] = in_arr[1];
  P.arr[2] = in_mat;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_ArrayOfMatrix_WriteVector_ValueRHS) {
    auto* src = R"(
struct S {
  arr : array<mat3x3<f32>, 4u>,
}

@group(0) @binding(0) var<storage, read_write> P : S;

fn f() {
  P.arr[0][1] = vec3(1.23);
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>,
}

struct S {
  arr : array<mat3x3<f32>, 4u>,
}

@group(0) @binding(0) var<storage, read_write> P : S_tint_packed_vec3;

fn f() {
  P.arr[0][1].elements = __packed_vec3<f32>(vec3(1.22999999999999998224));
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_ArrayOfMatrix_WriteVector_RefRHS) {
    auto* src = R"(
struct S {
  arr : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> P : S;
@group(0) @binding(1) var<uniform> in_str : S;
@group(0) @binding(2) var<uniform> in_arr : array<mat3x3<f32>, 4>;
@group(0) @binding(3) var<uniform> in_mat : mat3x3<f32>;
@group(0) @binding(4) var<uniform> in_vec : vec3<f32>;

fn f() {
  P.arr[0][0] = in_str.arr[0][1];
  P.arr[1][1] = in_arr[3][2];
  P.arr[2][2] = in_mat[1];
  P.arr[3][0] = in_vec;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>,
}

struct S {
  arr : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> P : S_tint_packed_vec3;

@group(0) @binding(1) var<uniform> in_str : S_tint_packed_vec3;

@group(0) @binding(2) var<uniform> in_arr : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>;

@group(0) @binding(3) var<uniform> in_mat : array<tint_packed_vec3_f32_array_element, 3u>;

@group(0) @binding(4) var<uniform> in_vec : __packed_vec3<f32>;

fn f() {
  P.arr[0][0].elements = in_str.arr[0][1].elements;
  P.arr[1][1].elements = in_arr[3][2].elements;
  P.arr[2][2].elements = in_mat[1].elements;
  P.arr[3][0].elements = in_vec;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_ArrayOfMatrix_WriteComponent_MemberAccessor) {
    auto* src = R"(
struct S {
  arr : array<mat3x3<f32>, 4u>,
}

@group(0) @binding(0) var<storage, read_write> P : S;

fn f() {
  P.arr[0][1].y = 1.23;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>,
}

struct S {
  arr : array<mat3x3<f32>, 4u>,
}

@group(0) @binding(0) var<storage, read_write> P : S_tint_packed_vec3;

fn f() {
  P.arr[0][1].elements.y = 1.22999999999999998224;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_ArrayOfMatrix_WriteComponent_IndexAccessor) {
    auto* src = R"(
struct S {
  arr : array<mat3x3<f32>, 4u>,
}

@group(0) @binding(0) var<storage, read_write> P : S;

fn f() {
  P.arr[0][1][2] = 1.23;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>,
}

struct S {
  arr : array<mat3x3<f32>, 4u>,
}

@group(0) @binding(0) var<storage, read_write> P : S_tint_packed_vec3;

fn f() {
  P.arr[0][1].elements[2] = 1.22999999999999998224;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_ExistingMemberAttributes) {
    auto* src = R"(
struct S {
  @align(32) @size(32) v : vec3<f32>,
  @align(64) @size(64) arr : array<vec3<f32>, 4>,
  @align(128) @size(128) x : u32,
}

@group(0) @binding(0) var<uniform> P : S;

fn f() {
  let x = P.v[0];
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(32) @size(32)
  v : __packed_vec3<f32>,
  @align(64) @size(64)
  arr : array<tint_packed_vec3_f32_array_element, 4u>,
  @align(128) @size(128)
  x : u32,
}

struct S {
  @align(32) @size(32)
  v : vec3<f32>,
  @align(64) @size(64)
  arr : array<vec3<f32>, 4>,
  @align(128) @size(128)
  x : u32,
}

@group(0) @binding(0) var<uniform> P : S_tint_packed_vec3;

fn f() {
  let x = P.v[0];
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_ExistingMemberAttributes_SizeMatchesUnpackedVec3) {
    // Test that the type we replace a vec3 with is not larger than it should be.
    auto* src = R"(
struct S {
  @size(12) v : vec3<f32>,
  @size(64) arr : array<vec3<f32>, 4>,
}

@group(0) @binding(0) var<uniform> P : S;

fn f() {
  let x = P.v[0];
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @size(12) @align(16)
  v : __packed_vec3<f32>,
  @size(64) @align(16)
  arr : array<tint_packed_vec3_f32_array_element, 4u>,
}

struct S {
  @size(12)
  v : vec3<f32>,
  @size(64)
  arr : array<vec3<f32>, 4>,
}

@group(0) @binding(0) var<uniform> P : S_tint_packed_vec3;

fn f() {
  let x = P.v[0];
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_ExistingMemberAttributes_AlignTooSmall) {
    // Test that we add an @align() attribute when the new alignment of the packed vec3 struct would
    // be too small.
    auto* src = R"(
struct S {
  a : u32,
  v : vec3<f32>,
  b : u32,
  arr : array<vec3<f32>, 4>,
}

@group(0) @binding(0) var<uniform> P : S;

fn f() {
  let x = P.v[0];
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  a : u32,
  @align(16)
  v : __packed_vec3<f32>,
  b : u32,
  @align(16)
  arr : array<tint_packed_vec3_f32_array_element, 4u>,
}

struct S {
  a : u32,
  v : vec3<f32>,
  b : u32,
  arr : array<vec3<f32>, 4>,
}

@group(0) @binding(0) var<uniform> P : S_tint_packed_vec3;

fn f() {
  let x = P.v[0];
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructMember_ExistingMemberAttributes_ExplicitOffset) {
    // Test that the we do not add an @align attribute if @offset is present.

    // struct S {
    //   a : u32,
    //   @offset(32) v : vec3<f32>,
    //   b : u32,
    //   @offset(128) arr : array<vec3<f32>, 4>,
    // }
    //
    // @group(0) @binding(0) var<uniform> P : S;
    ProgramBuilder b;
    b.Structure("S", utils::Vector{
                         b.Member("a", b.ty.u32()),
                         b.Member("v", b.ty.vec3<f32>(), utils::Vector{b.MemberOffset(AInt(32))}),
                         b.Member("b", b.ty.u32()),
                         b.Member("arr", b.ty.array(b.ty.vec3<f32>(), b.Expr(AInt(4))),
                                  utils::Vector{b.MemberOffset(AInt(128))}),
                     });
    b.GlobalVar("P", builtin::AddressSpace::kStorage, b.ty("S"),
                utils::Vector{b.Group(AInt(0)), b.Binding(AInt(0))});
    Program src(std::move(b));

    auto* expect =
        R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  a : u32,
  @size(28)
  padding : u32,
  /* @offset(32) */
  v : __packed_vec3<f32>,
  b : u32,
  @size(80)
  padding_1 : u32,
  /* @offset(128) */
  arr : array<tint_packed_vec3_f32_array_element, 4u>,
}

struct S {
  a : u32,
  @size(16)
  padding_2 : u32,
  /* @offset(32) */
  v : vec3<f32>,
  b : u32,
  @size(80)
  padding_3 : u32,
  /* @offset(128) */
  arr : array<vec3<f32>, 4>,
}

@group(0) @binding(0) var<storage> P : S_tint_packed_vec3;
)";

    DataMap data;
    auto got = Run<PackedVec3>(std::move(src), data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructValueConstructor_ViaIndexAccessor) {
    auto* src = R"(
struct S {
  a : vec3<f32>,
  b : vec3<f32>,
  arr : array<vec3<f32>, 4>,
}

@group(0) @binding(0) var<storage> s : S;

fn f() {
  let value_arr : array<vec3<f32>, 4> = array<vec3<f32>, 4>();
  let x = S(value_arr[0], s.arr[0], value_arr);
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  a : __packed_vec3<f32>,
  @align(16)
  b : __packed_vec3<f32>,
  @align(16)
  arr : array<tint_packed_vec3_f32_array_element, 4u>,
}

struct S {
  a : vec3<f32>,
  b : vec3<f32>,
  arr : array<vec3<f32>, 4>,
}

@group(0) @binding(0) var<storage> s : S_tint_packed_vec3;

fn f() {
  let value_arr : array<vec3<f32>, 4> = array<vec3<f32>, 4>();
  let x = S(value_arr[0], vec3<f32>(s.arr[0].elements), value_arr);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, WrapperStructLayout_MixedUsage) {
    // Test the layout of the generated wrapper struct(s) when vec3s are used in both structures and
    // arrays.
    auto* src = R"(
struct S {
  v : vec3<f32>,
  a : u32,
}

@group(0) @binding(0) var<storage, read_write> str : S;
@group(0) @binding(1) var<storage, read_write> arr : array<vec3<f32>, 4>;

fn main() {
  str.v = arr[0];
  arr[1] = str.v;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct S_tint_packed_vec3 {
  @align(16)
  v : __packed_vec3<f32>,
  a : u32,
}

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S {
  v : vec3<f32>,
  a : u32,
}

@group(0) @binding(0) var<storage, read_write> str : S_tint_packed_vec3;

@group(0) @binding(1) var<storage, read_write> arr : array<tint_packed_vec3_f32_array_element, 4u>;

fn main() {
  str.v = arr[0].elements;
  arr[1].elements = str.v;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    auto& vars = got.program.AST().GlobalVariables();
    ASSERT_EQ(vars.Length(), 2u);

    {
        // Check the layout of the struct type of "str".
        // The first member should have an alignment of 16 bytes, a size of 12 bytes, and the second
        // member should have an offset of 12 bytes.
        auto* sem_str = got.program.Sem().Get(vars[0]);
        auto* str_ty = sem_str->Type()->UnwrapRef()->As<type::Struct>();
        ASSERT_NE(str_ty, nullptr);
        ASSERT_EQ(str_ty->Members().Length(), 2u);
        EXPECT_EQ(str_ty->Members()[0]->Align(), 16u);
        EXPECT_EQ(str_ty->Members()[0]->Size(), 12u);
        EXPECT_EQ(str_ty->Members()[1]->Offset(), 12u);
    }

    {
        // Check the layout of the array type of "arr".
        // The element stride should be 16 bytes.
        auto* sem_arr = got.program.Sem().Get(vars[1]);
        auto* arr_ty = sem_arr->Type()->UnwrapRef()->As<type::Array>();
        ASSERT_NE(arr_ty, nullptr);
        EXPECT_EQ(arr_ty->Stride(), 16u);
    }

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, PackUnpackStructWithNonVec3Members) {
    auto* src = R"(
struct S {
  v : vec3<f32>,
  arr : array<vec3<f32>, 4>,
  a : u32,
  b : vec4<f32>,
  c : array<vec4<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> P : S;

fn f() {
  let x = P;
  P = x;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  v : __packed_vec3<f32>,
  @align(16)
  arr : array<tint_packed_vec3_f32_array_element, 4u>,
  a : u32,
  b : vec4<f32>,
  c : array<vec4<f32>, 4>,
}

fn tint_unpack_vec3_in_composite(in : array<tint_packed_vec3_f32_array_element, 4u>) -> array<vec3<f32>, 4u> {
  var result : array<vec3<f32>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_1(in : S_tint_packed_vec3) -> S {
  var result : S;
  result.v = vec3<f32>(in.v);
  result.arr = tint_unpack_vec3_in_composite(in.arr);
  result.a = in.a;
  result.b = in.b;
  result.c = in.c;
  return result;
}

fn tint_pack_vec3_in_composite(in : array<vec3<f32>, 4u>) -> array<tint_packed_vec3_f32_array_element, 4u> {
  var result : array<tint_packed_vec3_f32_array_element, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_packed_vec3_f32_array_element(__packed_vec3<f32>(in[i]));
  }
  return result;
}

fn tint_pack_vec3_in_composite_1(in : S) -> S_tint_packed_vec3 {
  var result : S_tint_packed_vec3;
  result.v = __packed_vec3<f32>(in.v);
  result.arr = tint_pack_vec3_in_composite(in.arr);
  result.a = in.a;
  result.b = in.b;
  result.c = in.c;
  return result;
}

struct S {
  v : vec3<f32>,
  arr : array<vec3<f32>, 4>,
  a : u32,
  b : vec4<f32>,
  c : array<vec4<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> P : S_tint_packed_vec3;

fn f() {
  let x = tint_unpack_vec3_in_composite_1(P);
  P = tint_pack_vec3_in_composite_1(x);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, Struct_ShaderIO) {
    // Test that we do not modify structures that are used for shader IO.
    auto* src = R"(
struct S1 {
  @location(0) v : vec3<f32>,
}

struct S2 {
  @location(0) v : vec3<f32>,
  @builtin(position) pos : vec4<f32>,
}

@vertex
fn main(s1 : S1) -> S2 {
  let v : vec3<f32> = s1.v;
  var s2 : S2;
  s2.v = v;
  return s2;
}
)";

    auto* expect = R"(
struct S1 {
  @location(0)
  v : vec3<f32>,
}

struct S2 {
  @location(0)
  v : vec3<f32>,
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn main(s1 : S1) -> S2 {
  let v : vec3<f32> = s1.v;
  var s2 : S2;
  s2.v = v;
  return s2;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ModfReturnStruct) {
    // Test that we do not try to modify accessors on the anonymous structure returned by modf.
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> output : vec3<f32>;

const values = array(modf(vec3(1.0, 2.0, 3.0)).fract);

@compute @workgroup_size(1)
fn main() {
  output = values[0];
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

@group(0) @binding(0) var<storage, read_write> output : __packed_vec3<f32>;

const values = array(modf(vec3(1.0, 2.0, 3.0)).fract);

@compute @workgroup_size(1)
fn main() {
  output = __packed_vec3<f32>(values[0]);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ModfReturnStruct_PointerToMember) {
    // Test that we can pass a pointer to the vec3 member of the modf return struct to a function
    // parameter to which we also pass a pointer to a vec3 member on a host-shareable struct.
    auto* src = R"(
enable chromium_experimental_full_ptr_parameters;

struct S {
  v : vec3<f32>
}

@group(0) @binding(0) var<storage, read_write> output : S;

fn foo(p : ptr<function, vec3<f32>>) {
  (*p) = vec3(1, 2, 3);
}

@compute @workgroup_size(1)
fn main() {
  var f : S;
  var modf_ret = modf(vec3(1.0, 2.0, 3.0));
  foo(&f.v);
  foo(&modf_ret.fract);
  output.v = modf_ret.fract;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;
enable chromium_experimental_full_ptr_parameters;

struct S_tint_packed_vec3 {
  @align(16)
  v : __packed_vec3<f32>,
}

struct S {
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage, read_write> output : S_tint_packed_vec3;

fn foo(p : ptr<function, vec3<f32>>) {
  *(p) = vec3(1, 2, 3);
}

@compute @workgroup_size(1)
fn main() {
  var f : S;
  var modf_ret = modf(vec3(1.0, 2.0, 3.0));
  foo(&(f.v));
  foo(&(modf_ret.fract));
  output.v = __packed_vec3<f32>(modf_ret.fract);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, MultipleStructMembers) {
    auto* src = R"(
struct S {
  v2_a : vec2<f32>,
  v3_a : vec3<f32>,
  v4_a : vec4<f32>,
  v2_b : vec2<f32>,
  v3_b : vec3<f32>,
  v4_b : vec4<f32>,
  v2_arr : array<vec2<f32>, 4>,
  v3_arr : array<vec3<f32>, 4>,
  v4_arr : array<vec4<f32>, 4>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let v2_a = P.v2_a;
  let v3_a = P.v3_a;
  let v4_a = P.v4_a;
  let v2_b = P.v2_b;
  let v3_b = P.v3_b;
  let v4_b = P.v4_b;
  let v2_arr : array<vec2<f32>, 4> = P.v2_arr;
  let v3_arr : array<vec3<f32>, 4> = P.v3_arr;
  let v4_arr : array<vec4<f32>, 4> = P.v4_arr;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  v2_a : vec2<f32>,
  @align(16)
  v3_a : __packed_vec3<f32>,
  v4_a : vec4<f32>,
  v2_b : vec2<f32>,
  @align(16)
  v3_b : __packed_vec3<f32>,
  v4_b : vec4<f32>,
  v2_arr : array<vec2<f32>, 4>,
  @align(16)
  v3_arr : array<tint_packed_vec3_f32_array_element, 4u>,
  v4_arr : array<vec4<f32>, 4>,
}

fn tint_unpack_vec3_in_composite(in : array<tint_packed_vec3_f32_array_element, 4u>) -> array<vec3<f32>, 4u> {
  var result : array<vec3<f32>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

struct S {
  v2_a : vec2<f32>,
  v3_a : vec3<f32>,
  v4_a : vec4<f32>,
  v2_b : vec2<f32>,
  v3_b : vec3<f32>,
  v4_b : vec4<f32>,
  v2_arr : array<vec2<f32>, 4>,
  v3_arr : array<vec3<f32>, 4>,
  v4_arr : array<vec4<f32>, 4>,
}

@group(0) @binding(0) var<storage> P : S_tint_packed_vec3;

fn f() {
  let v2_a = P.v2_a;
  let v3_a = vec3<f32>(P.v3_a);
  let v4_a = P.v4_a;
  let v2_b = P.v2_b;
  let v3_b = vec3<f32>(P.v3_b);
  let v4_b = P.v4_b;
  let v2_arr : array<vec2<f32>, 4> = P.v2_arr;
  let v3_arr : array<vec3<f32>, 4> = tint_unpack_vec3_in_composite(P.v3_arr);
  let v4_arr : array<vec4<f32>, 4> = P.v4_arr;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, Vec3Pointers) {
    auto* src = R"(
struct S {
  v : vec3<f32>,
  m : mat3x3<f32>,
  arr_v : array<vec3<f32>, 4>,
  arr_m : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> v : vec3<f32>;
@group(0) @binding(1) var<storage, read_write> arr_v : array<vec3<f32>, 4>;
@group(0) @binding(2) var<storage, read_write> m : mat3x3<f32>;
@group(0) @binding(3) var<storage, read_write> arr_m : array<mat3x3<f32>, 4>;
@group(0) @binding(4) var<storage, read_write> str : S;

fn f() {
  let p_v = &v;
  let v = *p_v;
  *p_v = v;

  let p_arr_v = &arr_v[0];
  let arr_v = *p_arr_v;
  *p_arr_v = arr_v;

  let p_m = &m[0];
  let m = *p_m;
  *p_m = m;

  let p_arr_m = &arr_m[0][1];
  let arr_m = *p_arr_m;
  *p_arr_m = arr_m;

  let p_str_v = &str.v;
  let str_v = *p_str_v;
  *p_str_v = str_v;

  let p_str_arr_v = &str.arr_v[0];
  let str_arr_v = *p_str_arr_v;
  *p_str_arr_v = str_arr_v;

  let p_str_m = &str.m[0];
  let str_m = *p_str_m;
  *p_str_m = str_m;

  let p_str_arr_m = &str.arr_m[0][1];
  let str_arr_m = *p_str_arr_m;
  *p_str_arr_m = str_arr_m;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  v : __packed_vec3<f32>,
  @align(16)
  m : array<tint_packed_vec3_f32_array_element, 3u>,
  @align(16)
  arr_v : array<tint_packed_vec3_f32_array_element, 4u>,
  @align(16)
  arr_m : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>,
}

struct S {
  v : vec3<f32>,
  m : mat3x3<f32>,
  arr_v : array<vec3<f32>, 4>,
  arr_m : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> v : __packed_vec3<f32>;

@group(0) @binding(1) var<storage, read_write> arr_v : array<tint_packed_vec3_f32_array_element, 4u>;

@group(0) @binding(2) var<storage, read_write> m : array<tint_packed_vec3_f32_array_element, 3u>;

@group(0) @binding(3) var<storage, read_write> arr_m : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>;

@group(0) @binding(4) var<storage, read_write> str : S_tint_packed_vec3;

fn f() {
  let p_v = &(v);
  let v = vec3<f32>(*(p_v));
  *(p_v) = __packed_vec3<f32>(v);
  let p_arr_v = &(arr_v[0].elements);
  let arr_v = vec3<f32>(*(p_arr_v));
  *(p_arr_v) = __packed_vec3<f32>(arr_v);
  let p_m = &(m[0].elements);
  let m = vec3<f32>(*(p_m));
  *(p_m) = __packed_vec3<f32>(m);
  let p_arr_m = &(arr_m[0][1].elements);
  let arr_m = vec3<f32>(*(p_arr_m));
  *(p_arr_m) = __packed_vec3<f32>(arr_m);
  let p_str_v = &(str.v);
  let str_v = vec3<f32>(*(p_str_v));
  *(p_str_v) = __packed_vec3<f32>(str_v);
  let p_str_arr_v = &(str.arr_v[0].elements);
  let str_arr_v = vec3<f32>(*(p_str_arr_v));
  *(p_str_arr_v) = __packed_vec3<f32>(str_arr_v);
  let p_str_m = &(str.m[0].elements);
  let str_m = vec3<f32>(*(p_str_m));
  *(p_str_m) = __packed_vec3<f32>(str_m);
  let p_str_arr_m = &(str.arr_m[0][1].elements);
  let str_arr_m = vec3<f32>(*(p_str_arr_m));
  *(p_str_arr_m) = __packed_vec3<f32>(str_arr_m);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, MatrixPointers) {
    auto* src = R"(
struct S {
  m : mat3x3<f32>,
  arr_m : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> m : mat3x3<f32>;
@group(0) @binding(1) var<storage, read_write> arr_m : array<mat3x3<f32>, 4>;
@group(0) @binding(2) var<storage, read_write> str : S;

fn f() {
  let p_m = &m;
  let m = *p_m;
  *p_m = m;

  let p_arr_m = &arr_m[0];
  let arr_m = *p_arr_m;
  *p_arr_m = arr_m;

  let p_str_m = &str.m;
  let str_m = *p_str_m;
  *p_str_m = str_m;

  let p_str_arr_m = &str.arr_m[0];
  let str_arr_m = *p_str_arr_m;
  *p_str_arr_m = str_arr_m;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  m : array<tint_packed_vec3_f32_array_element, 3u>,
  @align(16)
  arr_m : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>,
}

fn tint_unpack_vec3_in_composite(in : array<tint_packed_vec3_f32_array_element, 3u>) -> mat3x3<f32> {
  var result : mat3x3<f32>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

fn tint_pack_vec3_in_composite(in : mat3x3<f32>) -> array<tint_packed_vec3_f32_array_element, 3u> {
  var result : array<tint_packed_vec3_f32_array_element, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = tint_packed_vec3_f32_array_element(__packed_vec3<f32>(in[i]));
  }
  return result;
}

struct S {
  m : mat3x3<f32>,
  arr_m : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> m : array<tint_packed_vec3_f32_array_element, 3u>;

@group(0) @binding(1) var<storage, read_write> arr_m : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>;

@group(0) @binding(2) var<storage, read_write> str : S_tint_packed_vec3;

fn f() {
  let p_m = &(m);
  let m = tint_unpack_vec3_in_composite(*(p_m));
  *(p_m) = tint_pack_vec3_in_composite(m);
  let p_arr_m = &(arr_m[0]);
  let arr_m = tint_unpack_vec3_in_composite(*(p_arr_m));
  *(p_arr_m) = tint_pack_vec3_in_composite(arr_m);
  let p_str_m = &(str.m);
  let str_m = tint_unpack_vec3_in_composite(*(p_str_m));
  *(p_str_m) = tint_pack_vec3_in_composite(str_m);
  let p_str_arr_m = &(str.arr_m[0]);
  let str_arr_m = tint_unpack_vec3_in_composite(*(p_str_arr_m));
  *(p_str_arr_m) = tint_pack_vec3_in_composite(str_arr_m);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ArrayOfVec3Pointers) {
    auto* src = R"(
struct S {
  arr_v : array<vec3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> arr_v : array<vec3<f32>, 4>;
@group(0) @binding(1) var<storage, read_write> str : S;

fn f() {
  let p_arr_v = &arr_v;
  let arr_v = *p_arr_v;
  *p_arr_v = arr_v;

  let p_str_arr_v = &str.arr_v;
  let str_arr_v = *p_str_arr_v;
  *p_str_arr_v = str_arr_v;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr_v : array<tint_packed_vec3_f32_array_element, 4u>,
}

fn tint_unpack_vec3_in_composite(in : array<tint_packed_vec3_f32_array_element, 4u>) -> array<vec3<f32>, 4u> {
  var result : array<vec3<f32>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

fn tint_pack_vec3_in_composite(in : array<vec3<f32>, 4u>) -> array<tint_packed_vec3_f32_array_element, 4u> {
  var result : array<tint_packed_vec3_f32_array_element, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_packed_vec3_f32_array_element(__packed_vec3<f32>(in[i]));
  }
  return result;
}

struct S {
  arr_v : array<vec3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> arr_v : array<tint_packed_vec3_f32_array_element, 4u>;

@group(0) @binding(1) var<storage, read_write> str : S_tint_packed_vec3;

fn f() {
  let p_arr_v = &(arr_v);
  let arr_v = tint_unpack_vec3_in_composite(*(p_arr_v));
  *(p_arr_v) = tint_pack_vec3_in_composite(arr_v);
  let p_str_arr_v = &(str.arr_v);
  let str_arr_v = tint_unpack_vec3_in_composite(*(p_str_arr_v));
  *(p_str_arr_v) = tint_pack_vec3_in_composite(str_arr_v);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ArrayOfMatrixPointers) {
    auto* src = R"(
struct S {
  arr_m : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> arr_m : array<mat3x3<f32>, 4>;
@group(0) @binding(1) var<storage, read_write> str : S;

fn f() {
  let p_arr_m = &arr_m;
  let arr_m = *p_arr_m;
  *p_arr_m = arr_m;

  let p_str_arr_m = &str.arr_m;
  let str_arr_m = *p_str_arr_m;
  *p_str_arr_m = str_arr_m;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr_m : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>,
}

fn tint_unpack_vec3_in_composite(in : array<tint_packed_vec3_f32_array_element, 3u>) -> mat3x3<f32> {
  var result : mat3x3<f32>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_1(in : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>) -> array<mat3x3<f32>, 4u> {
  var result : array<mat3x3<f32>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_unpack_vec3_in_composite(in[i]);
  }
  return result;
}

fn tint_pack_vec3_in_composite(in : mat3x3<f32>) -> array<tint_packed_vec3_f32_array_element, 3u> {
  var result : array<tint_packed_vec3_f32_array_element, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = tint_packed_vec3_f32_array_element(__packed_vec3<f32>(in[i]));
  }
  return result;
}

fn tint_pack_vec3_in_composite_1(in : array<mat3x3<f32>, 4u>) -> array<array<tint_packed_vec3_f32_array_element, 3u>, 4u> {
  var result : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_pack_vec3_in_composite(in[i]);
  }
  return result;
}

struct S {
  arr_m : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> arr_m : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>;

@group(0) @binding(1) var<storage, read_write> str : S_tint_packed_vec3;

fn f() {
  let p_arr_m = &(arr_m);
  let arr_m = tint_unpack_vec3_in_composite_1(*(p_arr_m));
  *(p_arr_m) = tint_pack_vec3_in_composite_1(arr_m);
  let p_str_arr_m = &(str.arr_m);
  let str_arr_m = tint_unpack_vec3_in_composite_1(*(p_str_arr_m));
  *(p_str_arr_m) = tint_pack_vec3_in_composite_1(str_arr_m);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructPointers) {
    auto* src = R"(
struct S {
  v : vec3<f32>,
  m : mat3x3<f32>,
  arr_v : array<vec3<f32>, 4>,
  arr_m : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> str : S;

fn f() {
  let p_str = &str;
  let str = *p_str;
  *p_str = str;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  v : __packed_vec3<f32>,
  @align(16)
  m : array<tint_packed_vec3_f32_array_element, 3u>,
  @align(16)
  arr_v : array<tint_packed_vec3_f32_array_element, 4u>,
  @align(16)
  arr_m : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>,
}

fn tint_unpack_vec3_in_composite(in : array<tint_packed_vec3_f32_array_element, 3u>) -> mat3x3<f32> {
  var result : mat3x3<f32>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_1(in : array<tint_packed_vec3_f32_array_element, 4u>) -> array<vec3<f32>, 4u> {
  var result : array<vec3<f32>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_2(in : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>) -> array<mat3x3<f32>, 4u> {
  var result : array<mat3x3<f32>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_unpack_vec3_in_composite(in[i]);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_3(in : S_tint_packed_vec3) -> S {
  var result : S;
  result.v = vec3<f32>(in.v);
  result.m = tint_unpack_vec3_in_composite(in.m);
  result.arr_v = tint_unpack_vec3_in_composite_1(in.arr_v);
  result.arr_m = tint_unpack_vec3_in_composite_2(in.arr_m);
  return result;
}

fn tint_pack_vec3_in_composite(in : mat3x3<f32>) -> array<tint_packed_vec3_f32_array_element, 3u> {
  var result : array<tint_packed_vec3_f32_array_element, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = tint_packed_vec3_f32_array_element(__packed_vec3<f32>(in[i]));
  }
  return result;
}

fn tint_pack_vec3_in_composite_1(in : array<vec3<f32>, 4u>) -> array<tint_packed_vec3_f32_array_element, 4u> {
  var result : array<tint_packed_vec3_f32_array_element, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_packed_vec3_f32_array_element(__packed_vec3<f32>(in[i]));
  }
  return result;
}

fn tint_pack_vec3_in_composite_2(in : array<mat3x3<f32>, 4u>) -> array<array<tint_packed_vec3_f32_array_element, 3u>, 4u> {
  var result : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_pack_vec3_in_composite(in[i]);
  }
  return result;
}

fn tint_pack_vec3_in_composite_3(in : S) -> S_tint_packed_vec3 {
  var result : S_tint_packed_vec3;
  result.v = __packed_vec3<f32>(in.v);
  result.m = tint_pack_vec3_in_composite(in.m);
  result.arr_v = tint_pack_vec3_in_composite_1(in.arr_v);
  result.arr_m = tint_pack_vec3_in_composite_2(in.arr_m);
  return result;
}

struct S {
  v : vec3<f32>,
  m : mat3x3<f32>,
  arr_v : array<vec3<f32>, 4>,
  arr_m : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> str : S_tint_packed_vec3;

fn f() {
  let p_str = &(str);
  let str = tint_unpack_vec3_in_composite_3(*(p_str));
  *(p_str) = tint_pack_vec3_in_composite_3(str);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, VectorPointerParameters) {
    auto* src = R"(
enable chromium_experimental_full_ptr_parameters;

struct S {
  v : vec3<f32>,
  m : mat3x3<f32>,
  arr_v : array<vec3<f32>, 4>,
  arr_m : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> v : vec3<f32>;
@group(0) @binding(1) var<storage, read_write> arr_v : array<vec3<f32>, 4>;
@group(0) @binding(2) var<storage, read_write> m : mat3x3<f32>;
@group(0) @binding(3) var<storage, read_write> arr_m : array<mat3x3<f32>, 4>;
@group(0) @binding(4) var<storage, read_write> str : S;

fn load(p : ptr<storage, vec3<f32>, read_write>) -> vec3<f32> {
  return *p;
}

fn store(p : ptr<storage, vec3<f32>, read_write>) {
  *p = vec3(1, 2, 3);
}

fn f() {
  load(&v);
  store(&v);
  load(&arr_v[0]);
  store(&arr_v[0]);
  load(&m[0]);
  store(&m[0]);
  load(&arr_m[0][1]);
  store(&arr_m[0][1]);
  load(&str.v);
  store(&str.v);
  load(&str.arr_v[0]);
  store(&str.arr_v[0]);
  load(&str.m[0]);
  store(&str.m[0]);
  load(&str.arr_m[0][1]);
  store(&str.arr_m[0][1]);
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;
enable chromium_experimental_full_ptr_parameters;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  v : __packed_vec3<f32>,
  @align(16)
  m : array<tint_packed_vec3_f32_array_element, 3u>,
  @align(16)
  arr_v : array<tint_packed_vec3_f32_array_element, 4u>,
  @align(16)
  arr_m : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>,
}

struct S {
  v : vec3<f32>,
  m : mat3x3<f32>,
  arr_v : array<vec3<f32>, 4>,
  arr_m : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> v : __packed_vec3<f32>;

@group(0) @binding(1) var<storage, read_write> arr_v : array<tint_packed_vec3_f32_array_element, 4u>;

@group(0) @binding(2) var<storage, read_write> m : array<tint_packed_vec3_f32_array_element, 3u>;

@group(0) @binding(3) var<storage, read_write> arr_m : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>;

@group(0) @binding(4) var<storage, read_write> str : S_tint_packed_vec3;

fn load(p : ptr<storage, __packed_vec3<f32>, read_write>) -> vec3<f32> {
  return vec3<f32>(*(p));
}

fn store(p : ptr<storage, __packed_vec3<f32>, read_write>) {
  *(p) = __packed_vec3<f32>(vec3(1, 2, 3));
}

fn f() {
  load(&(v));
  store(&(v));
  load(&(arr_v[0].elements));
  store(&(arr_v[0].elements));
  load(&(m[0].elements));
  store(&(m[0].elements));
  load(&(arr_m[0][1].elements));
  store(&(arr_m[0][1].elements));
  load(&(str.v));
  store(&(str.v));
  load(&(str.arr_v[0].elements));
  store(&(str.arr_v[0].elements));
  load(&(str.m[0].elements));
  store(&(str.m[0].elements));
  load(&(str.arr_m[0][1].elements));
  store(&(str.arr_m[0][1].elements));
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, MatrixPointerParameters) {
    auto* src = R"(
enable chromium_experimental_full_ptr_parameters;

struct S {
  m : mat3x3<f32>,
  arr_m : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> m : mat3x3<f32>;
@group(0) @binding(1) var<storage, read_write> arr_m : array<mat3x3<f32>, 4>;
@group(0) @binding(2) var<storage, read_write> str : S;

fn load(p : ptr<storage, mat3x3<f32>, read_write>) -> mat3x3<f32> {
  return *p;
}

fn store(p : ptr<storage, mat3x3<f32>, read_write>) {
  *p = mat3x3(1, 2, 3, 4, 5, 6, 7, 8, 9);
}

fn f() {
  load(&m);
  store(&m);
  load(&arr_m[0]);
  store(&arr_m[0]);
  load(&str.m);
  store(&str.m);
  load(&str.arr_m[0]);
  store(&str.arr_m[0]);
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;
enable chromium_experimental_full_ptr_parameters;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  m : array<tint_packed_vec3_f32_array_element, 3u>,
  @align(16)
  arr_m : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>,
}

fn tint_unpack_vec3_in_composite(in : array<tint_packed_vec3_f32_array_element, 3u>) -> mat3x3<f32> {
  var result : mat3x3<f32>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

fn tint_pack_vec3_in_composite(in : mat3x3<f32>) -> array<tint_packed_vec3_f32_array_element, 3u> {
  var result : array<tint_packed_vec3_f32_array_element, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = tint_packed_vec3_f32_array_element(__packed_vec3<f32>(in[i]));
  }
  return result;
}

struct S {
  m : mat3x3<f32>,
  arr_m : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> m : array<tint_packed_vec3_f32_array_element, 3u>;

@group(0) @binding(1) var<storage, read_write> arr_m : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>;

@group(0) @binding(2) var<storage, read_write> str : S_tint_packed_vec3;

fn load(p : ptr<storage, array<tint_packed_vec3_f32_array_element, 3u>, read_write>) -> mat3x3<f32> {
  return tint_unpack_vec3_in_composite(*(p));
}

fn store(p : ptr<storage, array<tint_packed_vec3_f32_array_element, 3u>, read_write>) {
  *(p) = tint_pack_vec3_in_composite(mat3x3(1, 2, 3, 4, 5, 6, 7, 8, 9));
}

fn f() {
  load(&(m));
  store(&(m));
  load(&(arr_m[0]));
  store(&(arr_m[0]));
  load(&(str.m));
  store(&(str.m));
  load(&(str.arr_m[0]));
  store(&(str.arr_m[0]));
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ArrayOfVectorPointerParameters) {
    auto* src = R"(
enable chromium_experimental_full_ptr_parameters;

struct S {
  arr_v : array<vec3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> arr_v : array<vec3<f32>, 4>;
@group(0) @binding(1) var<storage, read_write> str : S;

fn load(p : ptr<storage, array<vec3<f32>, 4>, read_write>) -> array<vec3<f32>, 4> {
  return *p;
}

fn store(p : ptr<storage, array<vec3<f32>, 4>, read_write>) {
  *p = array(vec3(1.0), vec3(2.0), vec3(3.0), vec3(4.0));
}

fn f() {
  load(&arr_v);
  store(&arr_v);
  load(&str.arr_v);
  store(&str.arr_v);
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;
enable chromium_experimental_full_ptr_parameters;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr_v : array<tint_packed_vec3_f32_array_element, 4u>,
}

fn tint_unpack_vec3_in_composite(in : array<tint_packed_vec3_f32_array_element, 4u>) -> array<vec3<f32>, 4u> {
  var result : array<vec3<f32>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

fn tint_pack_vec3_in_composite(in : array<vec3<f32>, 4u>) -> array<tint_packed_vec3_f32_array_element, 4u> {
  var result : array<tint_packed_vec3_f32_array_element, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_packed_vec3_f32_array_element(__packed_vec3<f32>(in[i]));
  }
  return result;
}

struct S {
  arr_v : array<vec3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> arr_v : array<tint_packed_vec3_f32_array_element, 4u>;

@group(0) @binding(1) var<storage, read_write> str : S_tint_packed_vec3;

fn load(p : ptr<storage, array<tint_packed_vec3_f32_array_element, 4u>, read_write>) -> array<vec3<f32>, 4> {
  return tint_unpack_vec3_in_composite(*(p));
}

fn store(p : ptr<storage, array<tint_packed_vec3_f32_array_element, 4u>, read_write>) {
  *(p) = tint_pack_vec3_in_composite(array(vec3(1.0), vec3(2.0), vec3(3.0), vec3(4.0)));
}

fn f() {
  load(&(arr_v));
  store(&(arr_v));
  load(&(str.arr_v));
  store(&(str.arr_v));
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ArrayOfMatrixPointerParameters) {
    auto* src = R"(
enable chromium_experimental_full_ptr_parameters;

struct S {
  arr_m : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> arr_m : array<mat3x3<f32>, 4>;
@group(0) @binding(1) var<storage, read_write> str : S;

fn load(p : ptr<storage, array<mat3x3<f32>, 4>, read_write>) -> array<mat3x3<f32>, 4> {
  return *p;
}

fn store(p : ptr<storage, array<mat3x3<f32>, 4>, read_write>) {
  *p = array(mat3x3<f32>(), mat3x3<f32>(), mat3x3<f32>(), mat3x3<f32>());
}

fn f() {
  load(&arr_m);
  store(&arr_m);
  load(&str.arr_m);
  store(&str.arr_m);
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;
enable chromium_experimental_full_ptr_parameters;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr_m : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>,
}

fn tint_unpack_vec3_in_composite(in : array<tint_packed_vec3_f32_array_element, 3u>) -> mat3x3<f32> {
  var result : mat3x3<f32>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_1(in : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>) -> array<mat3x3<f32>, 4u> {
  var result : array<mat3x3<f32>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_unpack_vec3_in_composite(in[i]);
  }
  return result;
}

fn tint_pack_vec3_in_composite(in : mat3x3<f32>) -> array<tint_packed_vec3_f32_array_element, 3u> {
  var result : array<tint_packed_vec3_f32_array_element, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = tint_packed_vec3_f32_array_element(__packed_vec3<f32>(in[i]));
  }
  return result;
}

fn tint_pack_vec3_in_composite_1(in : array<mat3x3<f32>, 4u>) -> array<array<tint_packed_vec3_f32_array_element, 3u>, 4u> {
  var result : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_pack_vec3_in_composite(in[i]);
  }
  return result;
}

struct S {
  arr_m : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> arr_m : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>;

@group(0) @binding(1) var<storage, read_write> str : S_tint_packed_vec3;

fn load(p : ptr<storage, array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>, read_write>) -> array<mat3x3<f32>, 4> {
  return tint_unpack_vec3_in_composite_1(*(p));
}

fn store(p : ptr<storage, array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>, read_write>) {
  *(p) = tint_pack_vec3_in_composite_1(array(mat3x3<f32>(), mat3x3<f32>(), mat3x3<f32>(), mat3x3<f32>()));
}

fn f() {
  load(&(arr_m));
  store(&(arr_m));
  load(&(str.arr_m));
  store(&(str.arr_m));
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StructPointerParameters) {
    auto* src = R"(
enable chromium_experimental_full_ptr_parameters;

struct S {
  v : vec3<f32>,
  m : mat3x3<f32>,
  arr_v : array<vec3<f32>, 4>,
  arr_m : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> str : S;

fn load(p : ptr<storage, S, read_write>) -> S {
  return *p;
}

fn store(p : ptr<storage, S, read_write>) {
  *p = S();
}

fn f() {
  load(&str);
  store(&str);
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;
enable chromium_experimental_full_ptr_parameters;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  v : __packed_vec3<f32>,
  @align(16)
  m : array<tint_packed_vec3_f32_array_element, 3u>,
  @align(16)
  arr_v : array<tint_packed_vec3_f32_array_element, 4u>,
  @align(16)
  arr_m : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>,
}

fn tint_unpack_vec3_in_composite(in : array<tint_packed_vec3_f32_array_element, 3u>) -> mat3x3<f32> {
  var result : mat3x3<f32>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_1(in : array<tint_packed_vec3_f32_array_element, 4u>) -> array<vec3<f32>, 4u> {
  var result : array<vec3<f32>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_2(in : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>) -> array<mat3x3<f32>, 4u> {
  var result : array<mat3x3<f32>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_unpack_vec3_in_composite(in[i]);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_3(in : S_tint_packed_vec3) -> S {
  var result : S;
  result.v = vec3<f32>(in.v);
  result.m = tint_unpack_vec3_in_composite(in.m);
  result.arr_v = tint_unpack_vec3_in_composite_1(in.arr_v);
  result.arr_m = tint_unpack_vec3_in_composite_2(in.arr_m);
  return result;
}

fn tint_pack_vec3_in_composite(in : mat3x3<f32>) -> array<tint_packed_vec3_f32_array_element, 3u> {
  var result : array<tint_packed_vec3_f32_array_element, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = tint_packed_vec3_f32_array_element(__packed_vec3<f32>(in[i]));
  }
  return result;
}

fn tint_pack_vec3_in_composite_1(in : array<vec3<f32>, 4u>) -> array<tint_packed_vec3_f32_array_element, 4u> {
  var result : array<tint_packed_vec3_f32_array_element, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_packed_vec3_f32_array_element(__packed_vec3<f32>(in[i]));
  }
  return result;
}

fn tint_pack_vec3_in_composite_2(in : array<mat3x3<f32>, 4u>) -> array<array<tint_packed_vec3_f32_array_element, 3u>, 4u> {
  var result : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_pack_vec3_in_composite(in[i]);
  }
  return result;
}

fn tint_pack_vec3_in_composite_3(in : S) -> S_tint_packed_vec3 {
  var result : S_tint_packed_vec3;
  result.v = __packed_vec3<f32>(in.v);
  result.m = tint_pack_vec3_in_composite(in.m);
  result.arr_v = tint_pack_vec3_in_composite_1(in.arr_v);
  result.arr_m = tint_pack_vec3_in_composite_2(in.arr_m);
  return result;
}

struct S {
  v : vec3<f32>,
  m : mat3x3<f32>,
  arr_v : array<vec3<f32>, 4>,
  arr_m : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> str : S_tint_packed_vec3;

fn load(p : ptr<storage, S_tint_packed_vec3, read_write>) -> S {
  return tint_unpack_vec3_in_composite_3(*(p));
}

fn store(p : ptr<storage, S_tint_packed_vec3, read_write>) {
  *(p) = tint_pack_vec3_in_composite_3(S());
}

fn f() {
  load(&(str));
  store(&(str));
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, MixedAddressSpace_Struct) {
    auto* src = R"(
struct S {
  v : vec3<f32>,
  arr : array<vec3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> P : S;

fn f() {
  var f : S;
  let v = f.v;
  let arr = f.arr;
  P = f;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  v : __packed_vec3<f32>,
  @align(16)
  arr : array<tint_packed_vec3_f32_array_element, 4u>,
}

fn tint_pack_vec3_in_composite(in : array<vec3<f32>, 4u>) -> array<tint_packed_vec3_f32_array_element, 4u> {
  var result : array<tint_packed_vec3_f32_array_element, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_packed_vec3_f32_array_element(__packed_vec3<f32>(in[i]));
  }
  return result;
}

fn tint_pack_vec3_in_composite_1(in : S) -> S_tint_packed_vec3 {
  var result : S_tint_packed_vec3;
  result.v = __packed_vec3<f32>(in.v);
  result.arr = tint_pack_vec3_in_composite(in.arr);
  return result;
}

struct S {
  v : vec3<f32>,
  arr : array<vec3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> P : S_tint_packed_vec3;

fn f() {
  var f : S;
  let v = f.v;
  let arr = f.arr;
  P = tint_pack_vec3_in_composite_1(f);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, MixedAddressSpace_NestedStruct) {
    auto* src = R"(
struct S {
  v : vec3<f32>,
  arr : array<vec3<f32>, 4>,
}

struct Outer {
  inner : S,
}

@group(0) @binding(0) var<storage, read_write> P : Outer;

fn f() {
  var f : Outer;
  let v = f.inner.v;
  let arr = f.inner.arr;
  P = f;
  P.inner = f.inner;
  P.inner.v = f.inner.v;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  v : __packed_vec3<f32>,
  @align(16)
  arr : array<tint_packed_vec3_f32_array_element, 4u>,
}

struct Outer_tint_packed_vec3 {
  @align(16)
  inner : S_tint_packed_vec3,
}

fn tint_pack_vec3_in_composite(in : array<vec3<f32>, 4u>) -> array<tint_packed_vec3_f32_array_element, 4u> {
  var result : array<tint_packed_vec3_f32_array_element, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_packed_vec3_f32_array_element(__packed_vec3<f32>(in[i]));
  }
  return result;
}

fn tint_pack_vec3_in_composite_1(in : S) -> S_tint_packed_vec3 {
  var result : S_tint_packed_vec3;
  result.v = __packed_vec3<f32>(in.v);
  result.arr = tint_pack_vec3_in_composite(in.arr);
  return result;
}

fn tint_pack_vec3_in_composite_2(in : Outer) -> Outer_tint_packed_vec3 {
  var result : Outer_tint_packed_vec3;
  result.inner = tint_pack_vec3_in_composite_1(in.inner);
  return result;
}

struct S {
  v : vec3<f32>,
  arr : array<vec3<f32>, 4>,
}

struct Outer {
  inner : S,
}

@group(0) @binding(0) var<storage, read_write> P : Outer_tint_packed_vec3;

fn f() {
  var f : Outer;
  let v = f.inner.v;
  let arr = f.inner.arr;
  P = tint_pack_vec3_in_composite_2(f);
  P.inner = tint_pack_vec3_in_composite_1(f.inner);
  P.inner.v = __packed_vec3<f32>(f.inner.v);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, MixedAddressSpace_AnotherStructNotShared) {
    // Test that we can pass a pointers to a members of both shared and non-shared structs to the
    // same function.
    auto* src = R"(
enable chromium_experimental_full_ptr_parameters;

struct S {
  v : vec3<f32>,
  arr : array<vec3<f32>, 4>,
}

struct NotShared {
  v : vec3<f32>,
  arr : array<vec3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> P : S;

fn g(p : ptr<function, vec3<f32>>) -> vec3<f32> {
  return *p;
}

fn f() {
  var f1 : S;
  var f2 : NotShared;
  g(&f1.v);
  g(&f1.arr[0]);
  g(&f2.v);
  g(&f2.arr[0]);
  P = f1;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;
enable chromium_experimental_full_ptr_parameters;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  v : __packed_vec3<f32>,
  @align(16)
  arr : array<tint_packed_vec3_f32_array_element, 4u>,
}

fn tint_pack_vec3_in_composite(in : array<vec3<f32>, 4u>) -> array<tint_packed_vec3_f32_array_element, 4u> {
  var result : array<tint_packed_vec3_f32_array_element, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_packed_vec3_f32_array_element(__packed_vec3<f32>(in[i]));
  }
  return result;
}

fn tint_pack_vec3_in_composite_1(in : S) -> S_tint_packed_vec3 {
  var result : S_tint_packed_vec3;
  result.v = __packed_vec3<f32>(in.v);
  result.arr = tint_pack_vec3_in_composite(in.arr);
  return result;
}

struct S {
  v : vec3<f32>,
  arr : array<vec3<f32>, 4>,
}

struct NotShared {
  v : vec3<f32>,
  arr : array<vec3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> P : S_tint_packed_vec3;

fn g(p : ptr<function, vec3<f32>>) -> vec3<f32> {
  return *(p);
}

fn f() {
  var f1 : S;
  var f2 : NotShared;
  g(&(f1.v));
  g(&(f1.arr[0]));
  g(&(f2.v));
  g(&(f2.arr[0]));
  P = tint_pack_vec3_in_composite_1(f1);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, MixedAddressSpace_InitFromLoad_ExplicitVarType) {
    auto* src = R"(
struct S {
  v : vec3<f32>,
  m : mat3x3<f32>,
  arr_v : array<vec3<f32>, 4>,
  arr_m : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  var f1 : S = P;
  var f2 : vec3<f32> = P.v;
  var f3 : mat3x3<f32> = P.m;
  var f4 : array<vec3<f32>, 4> = P.arr_v;
  var f5 : array<mat3x3<f32>, 4> = P.arr_m;
  let v_1 = f1.v;
  let v_2 = f2;
  let v_3 = f3[0];
  let v_4 = f4[1];
  let v_5 = f5[2][2];
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  v : __packed_vec3<f32>,
  @align(16)
  m : array<tint_packed_vec3_f32_array_element, 3u>,
  @align(16)
  arr_v : array<tint_packed_vec3_f32_array_element, 4u>,
  @align(16)
  arr_m : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>,
}

fn tint_unpack_vec3_in_composite(in : array<tint_packed_vec3_f32_array_element, 3u>) -> mat3x3<f32> {
  var result : mat3x3<f32>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_1(in : array<tint_packed_vec3_f32_array_element, 4u>) -> array<vec3<f32>, 4u> {
  var result : array<vec3<f32>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_2(in : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>) -> array<mat3x3<f32>, 4u> {
  var result : array<mat3x3<f32>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_unpack_vec3_in_composite(in[i]);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_3(in : S_tint_packed_vec3) -> S {
  var result : S;
  result.v = vec3<f32>(in.v);
  result.m = tint_unpack_vec3_in_composite(in.m);
  result.arr_v = tint_unpack_vec3_in_composite_1(in.arr_v);
  result.arr_m = tint_unpack_vec3_in_composite_2(in.arr_m);
  return result;
}

struct S {
  v : vec3<f32>,
  m : mat3x3<f32>,
  arr_v : array<vec3<f32>, 4>,
  arr_m : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<storage> P : S_tint_packed_vec3;

fn f() {
  var f1 : S = tint_unpack_vec3_in_composite_3(P);
  var f2 : vec3<f32> = vec3<f32>(P.v);
  var f3 : mat3x3<f32> = tint_unpack_vec3_in_composite(P.m);
  var f4 : array<vec3<f32>, 4> = tint_unpack_vec3_in_composite_1(P.arr_v);
  var f5 : array<mat3x3<f32>, 4> = tint_unpack_vec3_in_composite_2(P.arr_m);
  let v_1 = f1.v;
  let v_2 = f2;
  let v_3 = f3[0];
  let v_4 = f4[1];
  let v_5 = f5[2][2];
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, MixedAddressSpace_InitFromLoad_InferredVarType) {
    auto* src = R"(
struct S {
  v : vec3<f32>,
  m : mat3x3<f32>,
  arr_v : array<vec3<f32>, 4>,
  arr_m : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  var f1 = P;
  var f2 = P.v;
  var f3 = P.m;
  var f4 = P.arr_v;
  var f5 = P.arr_m;
  let v_1 = f1.v;
  let v_2 = f2;
  let v_3 = f3[0];
  let v_4 = f4[1];
  let v_5 = f5[2][2];
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  v : __packed_vec3<f32>,
  @align(16)
  m : array<tint_packed_vec3_f32_array_element, 3u>,
  @align(16)
  arr_v : array<tint_packed_vec3_f32_array_element, 4u>,
  @align(16)
  arr_m : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>,
}

fn tint_unpack_vec3_in_composite(in : array<tint_packed_vec3_f32_array_element, 3u>) -> mat3x3<f32> {
  var result : mat3x3<f32>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_1(in : array<tint_packed_vec3_f32_array_element, 4u>) -> array<vec3<f32>, 4u> {
  var result : array<vec3<f32>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_2(in : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>) -> array<mat3x3<f32>, 4u> {
  var result : array<mat3x3<f32>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_unpack_vec3_in_composite(in[i]);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_3(in : S_tint_packed_vec3) -> S {
  var result : S;
  result.v = vec3<f32>(in.v);
  result.m = tint_unpack_vec3_in_composite(in.m);
  result.arr_v = tint_unpack_vec3_in_composite_1(in.arr_v);
  result.arr_m = tint_unpack_vec3_in_composite_2(in.arr_m);
  return result;
}

struct S {
  v : vec3<f32>,
  m : mat3x3<f32>,
  arr_v : array<vec3<f32>, 4>,
  arr_m : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<storage> P : S_tint_packed_vec3;

fn f() {
  var f1 = tint_unpack_vec3_in_composite_3(P);
  var f2 = vec3<f32>(P.v);
  var f3 = tint_unpack_vec3_in_composite(P.m);
  var f4 = tint_unpack_vec3_in_composite_1(P.arr_v);
  var f5 = tint_unpack_vec3_in_composite_2(P.arr_m);
  let v_1 = f1.v;
  let v_2 = f2;
  let v_3 = f3[0];
  let v_4 = f4[1];
  let v_5 = f5[2][2];
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, MixedAddressSpace_InitFromValue_ExplicitVarType) {
    auto* src = R"(
struct S {
  v : vec3<f32>,
  m : mat3x3<f32>,
  arr_v : array<vec3<f32>, 4>,
  arr_m : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  var f1 : S = S();
  var f2 : vec3<f32> = vec3<f32>();
  var f3 : mat3x3<f32> = mat3x3<f32>();
  var f4 : array<vec3<f32>, 4> = array(vec3<f32>(), vec3<f32>(), vec3<f32>(), vec3<f32>());
  var f5 : array<mat3x3<f32>, 4> = array(mat3x3<f32>(), mat3x3<f32>(), mat3x3<f32>(), mat3x3<f32>());
  let v_1 = f1.v;
  let v_2 = f2;
  let v_3 = f3[0];
  let v_4 = f4[1];
  let v_5 = f5[2][2];
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  v : __packed_vec3<f32>,
  @align(16)
  m : array<tint_packed_vec3_f32_array_element, 3u>,
  @align(16)
  arr_v : array<tint_packed_vec3_f32_array_element, 4u>,
  @align(16)
  arr_m : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>,
}

struct S {
  v : vec3<f32>,
  m : mat3x3<f32>,
  arr_v : array<vec3<f32>, 4>,
  arr_m : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<storage> P : S_tint_packed_vec3;

fn f() {
  var f1 : S = S();
  var f2 : vec3<f32> = vec3<f32>();
  var f3 : mat3x3<f32> = mat3x3<f32>();
  var f4 : array<vec3<f32>, 4> = array(vec3<f32>(), vec3<f32>(), vec3<f32>(), vec3<f32>());
  var f5 : array<mat3x3<f32>, 4> = array(mat3x3<f32>(), mat3x3<f32>(), mat3x3<f32>(), mat3x3<f32>());
  let v_1 = f1.v;
  let v_2 = f2;
  let v_3 = f3[0];
  let v_4 = f4[1];
  let v_5 = f5[2][2];
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, MixedAddressSpace_InitFromValue_InferredVarType) {
    auto* src = R"(
struct S {
  v : vec3<f32>,
  m : mat3x3<f32>,
  arr_v : array<vec3<f32>, 4>,
  arr_m : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  var f1 = S();
  var f2 = vec3<f32>();
  var f3 = mat3x3<f32>();
  var f4 = array(vec3<f32>(), vec3<f32>(), vec3<f32>(), vec3<f32>());
  var f5 = array(mat3x3<f32>(), mat3x3<f32>(), mat3x3<f32>(), mat3x3<f32>());
  let v_1 = f1.v;
  let v_2 = f2;
  let v_3 = f3[0];
  let v_4 = f4[1];
  let v_5 = f5[2][2];
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  v : __packed_vec3<f32>,
  @align(16)
  m : array<tint_packed_vec3_f32_array_element, 3u>,
  @align(16)
  arr_v : array<tint_packed_vec3_f32_array_element, 4u>,
  @align(16)
  arr_m : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>,
}

struct S {
  v : vec3<f32>,
  m : mat3x3<f32>,
  arr_v : array<vec3<f32>, 4>,
  arr_m : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<storage> P : S_tint_packed_vec3;

fn f() {
  var f1 = S();
  var f2 = vec3<f32>();
  var f3 = mat3x3<f32>();
  var f4 = array(vec3<f32>(), vec3<f32>(), vec3<f32>(), vec3<f32>());
  var f5 = array(mat3x3<f32>(), mat3x3<f32>(), mat3x3<f32>(), mat3x3<f32>());
  let v_1 = f1.v;
  let v_2 = f2;
  let v_3 = f3[0];
  let v_4 = f4[1];
  let v_5 = f5[2][2];
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, MixedAddressSpace_Pointers_Function) {
    auto* src = R"(
struct S {
  v : vec3<f32>,
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  var f1 : S = P;
  var f2 : vec3<f32> = P.v;
  var f3 : array<vec3<f32>, 4>;
  var f4 : mat3x3<f32> = P.m;
  let pv_1 : ptr<function, vec3<f32>> = &f1.v;
  let pv_2 : ptr<function, vec3<f32>> = &f2;
  let pv_3 : ptr<function, vec3<f32>> = &f3[0];
  let pv_4 : ptr<function, mat3x3<f32>> = &f1.m;
  let pv_5 : ptr<function, mat3x3<f32>> = &f4;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  v : __packed_vec3<f32>,
  @align(16)
  m : array<tint_packed_vec3_f32_array_element, 3u>,
}

fn tint_unpack_vec3_in_composite(in : array<tint_packed_vec3_f32_array_element, 3u>) -> mat3x3<f32> {
  var result : mat3x3<f32>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_1(in : S_tint_packed_vec3) -> S {
  var result : S;
  result.v = vec3<f32>(in.v);
  result.m = tint_unpack_vec3_in_composite(in.m);
  return result;
}

struct S {
  v : vec3<f32>,
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<storage> P : S_tint_packed_vec3;

fn f() {
  var f1 : S = tint_unpack_vec3_in_composite_1(P);
  var f2 : vec3<f32> = vec3<f32>(P.v);
  var f3 : array<vec3<f32>, 4>;
  var f4 : mat3x3<f32> = tint_unpack_vec3_in_composite(P.m);
  let pv_1 : ptr<function, vec3<f32>> = &(f1.v);
  let pv_2 : ptr<function, vec3<f32>> = &(f2);
  let pv_3 : ptr<function, vec3<f32>> = &(f3[0]);
  let pv_4 : ptr<function, mat3x3<f32>> = &(f1.m);
  let pv_5 : ptr<function, mat3x3<f32>> = &(f4);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, MixedAddressSpace_Pointers_Private) {
    auto* src = R"(
struct S {
  v : vec3<f32>,
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<storage> P : S;

var<private> p1 : S;
var<private> p2 : vec3<f32>;
var<private> p3 : array<vec3<f32>, 4>;
var<private> p4 : mat3x3<f32>;

fn f() {
  let pv_1 : ptr<private, vec3<f32>> = &p1.v;
  let pv_2 : ptr<private, vec3<f32>> = &p2;
  let pv_3 : ptr<private, vec3<f32>> = &p3[0];
  let pv_4 : ptr<private, mat3x3<f32>> = &p1.m;
  let pv_5 : ptr<private, mat3x3<f32>> = &p4;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  v : __packed_vec3<f32>,
  @align(16)
  m : array<tint_packed_vec3_f32_array_element, 3u>,
}

struct S {
  v : vec3<f32>,
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<storage> P : S_tint_packed_vec3;

var<private> p1 : S;

var<private> p2 : vec3<f32>;

var<private> p3 : array<vec3<f32>, 4>;

var<private> p4 : mat3x3<f32>;

fn f() {
  let pv_1 : ptr<private, vec3<f32>> = &(p1.v);
  let pv_2 : ptr<private, vec3<f32>> = &(p2);
  let pv_3 : ptr<private, vec3<f32>> = &(p3[0]);
  let pv_4 : ptr<private, mat3x3<f32>> = &(p1.m);
  let pv_5 : ptr<private, mat3x3<f32>> = &(p4);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, MixedAddressSpace_Pointers_Workgroup) {
    auto* src = R"(
struct S {
  v : vec3<f32>,
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<storage> P : S;

var<workgroup> w1 : S;
var<workgroup> w2 : vec3<f32>;
var<workgroup> w3 : array<vec3<f32>, 4>;
var<workgroup> w4 : mat3x3<f32>;

fn f() {
  let pv_1 : ptr<workgroup, vec3<f32>> = &w1.v;
  let pv_2 : ptr<workgroup, vec3<f32>> = &w2;
  let pv_3 : ptr<workgroup, vec3<f32>> = &w3[0];
  let pv_4 : ptr<workgroup, mat3x3<f32>> = &w1.m;
  let pv_5 : ptr<workgroup, mat3x3<f32>> = &w4;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  v : __packed_vec3<f32>,
  @align(16)
  m : array<tint_packed_vec3_f32_array_element, 3u>,
}

struct S {
  v : vec3<f32>,
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<storage> P : S_tint_packed_vec3;

var<workgroup> w1 : S;

var<workgroup> w2 : vec3<f32>;

var<workgroup> w3 : array<vec3<f32>, 4>;

var<workgroup> w4 : mat3x3<f32>;

fn f() {
  let pv_1 : ptr<workgroup, vec3<f32>> = &(w1.v);
  let pv_2 : ptr<workgroup, vec3<f32>> = &(w2);
  let pv_3 : ptr<workgroup, vec3<f32>> = &(w3[0]);
  let pv_4 : ptr<workgroup, mat3x3<f32>> = &(w1.m);
  let pv_5 : ptr<workgroup, mat3x3<f32>> = &(w4);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, MixedAddressSpace_PointerParameters) {
    auto* src = R"(
enable chromium_experimental_full_ptr_parameters;

struct S {
  v : vec3<f32>,
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<storage> P : S;

fn g_v(p : ptr<function, vec3<f32>>) -> vec3<f32> {
  return *p;
}

fn g_m(p : ptr<function, mat3x3<f32>>) -> mat3x3<f32> {
  return *p;
}

fn f() {
  var f1 : S = P;
  var f2 : vec3<f32> = P.v;
  var f3 : array<vec3<f32>, 4>;
  var f4 : mat3x3<f32> = P.m;
  g_v(&f1.v);
  g_v(&f2);
  g_v(&f3[0]);
  g_m(&f1.m);
  g_m(&f4);
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;
enable chromium_experimental_full_ptr_parameters;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  v : __packed_vec3<f32>,
  @align(16)
  m : array<tint_packed_vec3_f32_array_element, 3u>,
}

fn tint_unpack_vec3_in_composite(in : array<tint_packed_vec3_f32_array_element, 3u>) -> mat3x3<f32> {
  var result : mat3x3<f32>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_1(in : S_tint_packed_vec3) -> S {
  var result : S;
  result.v = vec3<f32>(in.v);
  result.m = tint_unpack_vec3_in_composite(in.m);
  return result;
}

struct S {
  v : vec3<f32>,
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<storage> P : S_tint_packed_vec3;

fn g_v(p : ptr<function, vec3<f32>>) -> vec3<f32> {
  return *(p);
}

fn g_m(p : ptr<function, mat3x3<f32>>) -> mat3x3<f32> {
  return *(p);
}

fn f() {
  var f1 : S = tint_unpack_vec3_in_composite_1(P);
  var f2 : vec3<f32> = vec3<f32>(P.v);
  var f3 : array<vec3<f32>, 4>;
  var f4 : mat3x3<f32> = tint_unpack_vec3_in_composite(P.m);
  g_v(&(f1.v));
  g_v(&(f2));
  g_v(&(f3[0]));
  g_m(&(f1.m));
  g_m(&(f4));
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, WriteVec3Swizzle_FromRef) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> v : vec3<f32>;

fn f() {
  v = v.zyx;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

@group(0) @binding(0) var<storage, read_write> v : __packed_vec3<f32>;

fn f() {
  v = __packed_vec3<f32>(vec3<f32>(v).zyx);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, WriteVec3Swizzle_FromValue) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> v : vec3<f32>;

fn f() {
  v = vec3f(1, 2, 3).zyx;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

@group(0) @binding(0) var<storage, read_write> v : __packed_vec3<f32>;

fn f() {
  v = __packed_vec3<f32>(vec3f(1, 2, 3).zyx);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, WriteVec3Component_FromPackedValueIndexAccessor) {
    auto* src = R"(
struct S {
  v : vec3<f32>
}

@group(0) @binding(0) var<storage, read_write> s : S;

fn g() -> S {
  return S();
}

fn f() {
  s.v[0] = g().v[1];
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct S_tint_packed_vec3 {
  @align(16)
  v : __packed_vec3<f32>,
}

struct S {
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage, read_write> s : S_tint_packed_vec3;

fn g() -> S {
  return S();
}

fn f() {
  s.v[0] = g().v[1];
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ExtractVec3FromStructValueExpression) {
    auto* src = R"(
struct S {
  v : vec3<f32>
}

@group(0) @binding(0) var<storage, read_write> buffer : S;

fn f() {
  var v_var : vec3<f32> = S().v;
  let v_let : vec3<f32> = S().v;
  v_var = S().v;
  v_var = S().v * 2.0;
  buffer = S(S().v);
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct S_tint_packed_vec3 {
  @align(16)
  v : __packed_vec3<f32>,
}

fn tint_pack_vec3_in_composite(in : S) -> S_tint_packed_vec3 {
  var result : S_tint_packed_vec3;
  result.v = __packed_vec3<f32>(in.v);
  return result;
}

struct S {
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage, read_write> buffer : S_tint_packed_vec3;

fn f() {
  var v_var : vec3<f32> = S().v;
  let v_let : vec3<f32> = S().v;
  v_var = S().v;
  v_var = (S().v * 2.0);
  buffer = tint_pack_vec3_in_composite(S(S().v));
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ExtractArrayOfVec3FromStructValueExpression) {
    auto* src = R"(
struct S {
  arr : array<vec3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> buffer : S;

fn f() {
  var arr_var : array<vec3<f32>, 4> = S().arr;
  let arr_let : array<vec3<f32>, 4> = S().arr;
  arr_var = S().arr;
  arr_var[0] = S().arr[0] * 2.0;
  buffer = S(S().arr);
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr : array<tint_packed_vec3_f32_array_element, 4u>,
}

fn tint_pack_vec3_in_composite(in : array<vec3<f32>, 4u>) -> array<tint_packed_vec3_f32_array_element, 4u> {
  var result : array<tint_packed_vec3_f32_array_element, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_packed_vec3_f32_array_element(__packed_vec3<f32>(in[i]));
  }
  return result;
}

fn tint_pack_vec3_in_composite_1(in : S) -> S_tint_packed_vec3 {
  var result : S_tint_packed_vec3;
  result.arr = tint_pack_vec3_in_composite(in.arr);
  return result;
}

struct S {
  arr : array<vec3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> buffer : S_tint_packed_vec3;

fn f() {
  var arr_var : array<vec3<f32>, 4> = S().arr;
  let arr_let : array<vec3<f32>, 4> = S().arr;
  arr_var = S().arr;
  arr_var[0] = (S().arr[0] * 2.0);
  buffer = tint_pack_vec3_in_composite_1(S(S().arr));
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ExtractNestedArrayFromStructValueExpression) {
    auto* src = R"(
struct S {
  arr : array<array<vec3<f32>, 4>, 4>,
}

@group(0) @binding(0) var<storage, read_write> buffer : S;

fn f() {
  var arr_var : array<array<vec3<f32>, 4>, 4> = S().arr;
  var inner_var : array<vec3<f32>, 4> = S().arr[0];
  let arr_let : array<array<vec3<f32>, 4>, 4> = S().arr;
  arr_var = S().arr;
  inner_var = S().arr[0];
  arr_var[0][0] = S().arr[0][0] * 2.0;
  buffer = S(S().arr);
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr : array<array<tint_packed_vec3_f32_array_element, 4u>, 4u>,
}

fn tint_pack_vec3_in_composite(in : array<vec3<f32>, 4u>) -> array<tint_packed_vec3_f32_array_element, 4u> {
  var result : array<tint_packed_vec3_f32_array_element, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_packed_vec3_f32_array_element(__packed_vec3<f32>(in[i]));
  }
  return result;
}

fn tint_pack_vec3_in_composite_1(in : array<array<vec3<f32>, 4u>, 4u>) -> array<array<tint_packed_vec3_f32_array_element, 4u>, 4u> {
  var result : array<array<tint_packed_vec3_f32_array_element, 4u>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_pack_vec3_in_composite(in[i]);
  }
  return result;
}

fn tint_pack_vec3_in_composite_2(in : S) -> S_tint_packed_vec3 {
  var result : S_tint_packed_vec3;
  result.arr = tint_pack_vec3_in_composite_1(in.arr);
  return result;
}

struct S {
  arr : array<array<vec3<f32>, 4>, 4>,
}

@group(0) @binding(0) var<storage, read_write> buffer : S_tint_packed_vec3;

fn f() {
  var arr_var : array<array<vec3<f32>, 4>, 4> = S().arr;
  var inner_var : array<vec3<f32>, 4> = S().arr[0];
  let arr_let : array<array<vec3<f32>, 4>, 4> = S().arr;
  arr_var = S().arr;
  inner_var = S().arr[0];
  arr_var[0][0] = (S().arr[0][0] * 2.0);
  buffer = tint_pack_vec3_in_composite_2(S(S().arr));
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ExtractMatrixFromStructValueExpression) {
    auto* src = R"(
struct S {
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<storage, read_write> buffer : S;

fn f() {
  var m_var : mat3x3<f32> = S().m;
  let m_let : mat3x3<f32> = S().m;
  m_var = S().m;
  m_var = S().m * 2.0;
  buffer = S(S().m);
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  m : array<tint_packed_vec3_f32_array_element, 3u>,
}

fn tint_pack_vec3_in_composite(in : mat3x3<f32>) -> array<tint_packed_vec3_f32_array_element, 3u> {
  var result : array<tint_packed_vec3_f32_array_element, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = tint_packed_vec3_f32_array_element(__packed_vec3<f32>(in[i]));
  }
  return result;
}

fn tint_pack_vec3_in_composite_1(in : S) -> S_tint_packed_vec3 {
  var result : S_tint_packed_vec3;
  result.m = tint_pack_vec3_in_composite(in.m);
  return result;
}

struct S {
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<storage, read_write> buffer : S_tint_packed_vec3;

fn f() {
  var m_var : mat3x3<f32> = S().m;
  let m_let : mat3x3<f32> = S().m;
  m_var = S().m;
  m_var = (S().m * 2.0);
  buffer = tint_pack_vec3_in_composite_1(S(S().m));
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ExtractArrayOfMatrixFromStructValueExpression) {
    auto* src = R"(
struct S {
  arr : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> buffer : S;

fn f() {
  var arr_var : array<mat3x3<f32>, 4> = S().arr;
  let arr_let : array<mat3x3<f32>, 4> = S().arr;
  arr_var = S().arr;
  arr_var[0] = S().arr[0] * 2.0;
  buffer = S(S().arr);
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>,
}

fn tint_pack_vec3_in_composite(in : mat3x3<f32>) -> array<tint_packed_vec3_f32_array_element, 3u> {
  var result : array<tint_packed_vec3_f32_array_element, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = tint_packed_vec3_f32_array_element(__packed_vec3<f32>(in[i]));
  }
  return result;
}

fn tint_pack_vec3_in_composite_1(in : array<mat3x3<f32>, 4u>) -> array<array<tint_packed_vec3_f32_array_element, 3u>, 4u> {
  var result : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_pack_vec3_in_composite(in[i]);
  }
  return result;
}

fn tint_pack_vec3_in_composite_2(in : S) -> S_tint_packed_vec3 {
  var result : S_tint_packed_vec3;
  result.arr = tint_pack_vec3_in_composite_1(in.arr);
  return result;
}

struct S {
  arr : array<mat3x3<f32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> buffer : S_tint_packed_vec3;

fn f() {
  var arr_var : array<mat3x3<f32>, 4> = S().arr;
  let arr_let : array<mat3x3<f32>, 4> = S().arr;
  arr_var = S().arr;
  arr_var[0] = (S().arr[0] * 2.0);
  buffer = tint_pack_vec3_in_composite_2(S(S().arr));
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, NestedArrays_Let) {
    auto* src = R"(
struct S {
  arr_v : array<array<vec3<f32>, 4>, 4>,
  arr_m : array<array<mat3x3<f32>, 4>, 4>,
}

@group(0) @binding(0) var<storage, read_write> arr_s : array<S, 4>;

fn f() {
  let full_let : array<S, 4> = arr_s;
  let struct_let : S = arr_s[0];
  let outer_arr_v_let : array<array<vec3<f32>, 4>, 4> = arr_s[0].arr_v;
  let inner_arr_v_let : array<vec3<f32>, 4> = arr_s[0].arr_v[1];
  let v_let : vec3<f32> = arr_s[0].arr_v[1][2];
  let v_element_let : f32 = arr_s[0].arr_v[1][2].y;
  let outer_arr_m_let : array<array<mat3x3<f32>, 4>, 4> = arr_s[0].arr_m;
  let inner_arr_m_let : array<mat3x3<f32>, 4> = arr_s[0].arr_m[1];
  let m_let : mat3x3<f32> = arr_s[0].arr_m[1][2];
  let m_col_let : vec3<f32> = arr_s[0].arr_m[1][2][0];
  let m_element_let : f32 = arr_s[0].arr_m[1][2][0].y;
}
)";

    auto* expect = R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr_v : array<array<tint_packed_vec3_f32_array_element, 4u>, 4u>,
  @align(16)
  arr_m : array<array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>, 4u>,
}

fn tint_unpack_vec3_in_composite(in : array<tint_packed_vec3_f32_array_element, 4u>) -> array<vec3<f32>, 4u> {
  var result : array<vec3<f32>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_1(in : array<array<tint_packed_vec3_f32_array_element, 4u>, 4u>) -> array<array<vec3<f32>, 4u>, 4u> {
  var result : array<array<vec3<f32>, 4u>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_unpack_vec3_in_composite(in[i]);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_2(in : array<tint_packed_vec3_f32_array_element, 3u>) -> mat3x3<f32> {
  var result : mat3x3<f32>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_3(in : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>) -> array<mat3x3<f32>, 4u> {
  var result : array<mat3x3<f32>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_unpack_vec3_in_composite_2(in[i]);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_4(in : array<array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>, 4u>) -> array<array<mat3x3<f32>, 4u>, 4u> {
  var result : array<array<mat3x3<f32>, 4u>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_unpack_vec3_in_composite_3(in[i]);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_5(in : S_tint_packed_vec3) -> S {
  var result : S;
  result.arr_v = tint_unpack_vec3_in_composite_1(in.arr_v);
  result.arr_m = tint_unpack_vec3_in_composite_4(in.arr_m);
  return result;
}

fn tint_unpack_vec3_in_composite_6(in : array<S_tint_packed_vec3, 4u>) -> array<S, 4u> {
  var result : array<S, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_unpack_vec3_in_composite_5(in[i]);
  }
  return result;
}

struct S {
  arr_v : array<array<vec3<f32>, 4>, 4>,
  arr_m : array<array<mat3x3<f32>, 4>, 4>,
}

@group(0) @binding(0) var<storage, read_write> arr_s : array<S_tint_packed_vec3, 4u>;

fn f() {
  let full_let : array<S, 4> = tint_unpack_vec3_in_composite_6(arr_s);
  let struct_let : S = tint_unpack_vec3_in_composite_5(arr_s[0]);
  let outer_arr_v_let : array<array<vec3<f32>, 4>, 4> = tint_unpack_vec3_in_composite_1(arr_s[0].arr_v);
  let inner_arr_v_let : array<vec3<f32>, 4> = tint_unpack_vec3_in_composite(arr_s[0].arr_v[1]);
  let v_let : vec3<f32> = vec3<f32>(arr_s[0].arr_v[1][2].elements);
  let v_element_let : f32 = arr_s[0].arr_v[1][2].elements.y;
  let outer_arr_m_let : array<array<mat3x3<f32>, 4>, 4> = tint_unpack_vec3_in_composite_4(arr_s[0].arr_m);
  let inner_arr_m_let : array<mat3x3<f32>, 4> = tint_unpack_vec3_in_composite_3(arr_s[0].arr_m[1]);
  let m_let : mat3x3<f32> = tint_unpack_vec3_in_composite_2(arr_s[0].arr_m[1][2]);
  let m_col_let : vec3<f32> = vec3<f32>(arr_s[0].arr_m[1][2][0].elements);
  let m_element_let : f32 = arr_s[0].arr_m[1][2][0].elements.y;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, NestedArrays_VarInit) {
    auto* src = R"(
struct S {
  arr_v : array<array<vec3<f32>, 4>, 4>,
  arr_m : array<array<mat3x3<f32>, 4>, 4>,
}

@group(0) @binding(0) var<storage, read_write> arr_s : array<S, 4>;

fn f() {
  var full_var : array<S, 4> = arr_s;
  var struct_var : S = arr_s[0];
  var outer_arr_v_var : array<array<vec3<f32>, 4>, 4> = arr_s[0].arr_v;
  var inner_arr_v_var : array<vec3<f32>, 4> = arr_s[0].arr_v[1];
  var v_var : vec3<f32> = arr_s[0].arr_v[1][2];
  var v_element_var : f32 = arr_s[0].arr_v[1][2].y;
  var outer_arr_m_var : array<array<mat3x3<f32>, 4>, 4> = arr_s[0].arr_m;
  var inner_arr_m_var : array<mat3x3<f32>, 4> = arr_s[0].arr_m[1];
  var m_var : mat3x3<f32> = arr_s[0].arr_m[1][2];
  var m_col_var : vec3<f32> = arr_s[0].arr_m[1][2][0];
  var m_element_var : f32 = arr_s[0].arr_m[1][2][0].y;
}
)";

    auto* expect =
        R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr_v : array<array<tint_packed_vec3_f32_array_element, 4u>, 4u>,
  @align(16)
  arr_m : array<array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>, 4u>,
}

fn tint_unpack_vec3_in_composite(in : array<tint_packed_vec3_f32_array_element, 4u>) -> array<vec3<f32>, 4u> {
  var result : array<vec3<f32>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_1(in : array<array<tint_packed_vec3_f32_array_element, 4u>, 4u>) -> array<array<vec3<f32>, 4u>, 4u> {
  var result : array<array<vec3<f32>, 4u>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_unpack_vec3_in_composite(in[i]);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_2(in : array<tint_packed_vec3_f32_array_element, 3u>) -> mat3x3<f32> {
  var result : mat3x3<f32>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_3(in : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>) -> array<mat3x3<f32>, 4u> {
  var result : array<mat3x3<f32>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_unpack_vec3_in_composite_2(in[i]);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_4(in : array<array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>, 4u>) -> array<array<mat3x3<f32>, 4u>, 4u> {
  var result : array<array<mat3x3<f32>, 4u>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_unpack_vec3_in_composite_3(in[i]);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_5(in : S_tint_packed_vec3) -> S {
  var result : S;
  result.arr_v = tint_unpack_vec3_in_composite_1(in.arr_v);
  result.arr_m = tint_unpack_vec3_in_composite_4(in.arr_m);
  return result;
}

fn tint_unpack_vec3_in_composite_6(in : array<S_tint_packed_vec3, 4u>) -> array<S, 4u> {
  var result : array<S, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_unpack_vec3_in_composite_5(in[i]);
  }
  return result;
}

struct S {
  arr_v : array<array<vec3<f32>, 4>, 4>,
  arr_m : array<array<mat3x3<f32>, 4>, 4>,
}

@group(0) @binding(0) var<storage, read_write> arr_s : array<S_tint_packed_vec3, 4u>;

fn f() {
  var full_var : array<S, 4> = tint_unpack_vec3_in_composite_6(arr_s);
  var struct_var : S = tint_unpack_vec3_in_composite_5(arr_s[0]);
  var outer_arr_v_var : array<array<vec3<f32>, 4>, 4> = tint_unpack_vec3_in_composite_1(arr_s[0].arr_v);
  var inner_arr_v_var : array<vec3<f32>, 4> = tint_unpack_vec3_in_composite(arr_s[0].arr_v[1]);
  var v_var : vec3<f32> = vec3<f32>(arr_s[0].arr_v[1][2].elements);
  var v_element_var : f32 = arr_s[0].arr_v[1][2].elements.y;
  var outer_arr_m_var : array<array<mat3x3<f32>, 4>, 4> = tint_unpack_vec3_in_composite_4(arr_s[0].arr_m);
  var inner_arr_m_var : array<mat3x3<f32>, 4> = tint_unpack_vec3_in_composite_3(arr_s[0].arr_m[1]);
  var m_var : mat3x3<f32> = tint_unpack_vec3_in_composite_2(arr_s[0].arr_m[1][2]);
  var m_col_var : vec3<f32> = vec3<f32>(arr_s[0].arr_m[1][2][0].elements);
  var m_element_var : f32 = arr_s[0].arr_m[1][2][0].elements.y;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, NestedArrays_VarAssignment) {
    auto* src = R"(
struct S {
  arr_v : array<array<vec3<f32>, 4>, 4>,
  arr_m : array<array<mat3x3<f32>, 4>, 4>,
}

@group(0) @binding(0) var<storage, read_write> arr_s : array<S, 4>;

fn f() {
  var full_var : array<S, 4>;
  var struct_var : S;
  var outer_arr_v_var : array<array<vec3<f32>, 4>, 4>;
  var inner_arr_v_var : array<vec3<f32>, 4>;
  var v_var : vec3<f32>;
  var v_element_var : f32;
  var outer_arr_m_var : array<array<mat3x3<f32>, 4>, 4>;
  var inner_arr_m_var : array<mat3x3<f32>, 4>;
  var m_var : mat3x3<f32>;
  var m_col_var : vec3<f32>;
  var m_element_var : f32;

  full_var = arr_s;
  struct_var = arr_s[0];
  outer_arr_v_var = arr_s[0].arr_v;
  inner_arr_v_var = arr_s[0].arr_v[1];
  v_var = arr_s[0].arr_v[1][2];
  v_element_var = arr_s[0].arr_v[1][2].y;
  outer_arr_m_var = arr_s[0].arr_m;
  inner_arr_m_var = arr_s[0].arr_m[1];
  m_var = arr_s[0].arr_m[1][2];
  m_col_var = arr_s[0].arr_m[1][2][0];
  m_element_var = arr_s[0].arr_m[1][2][0].y;
}
)";

    auto* expect =
        R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr_v : array<array<tint_packed_vec3_f32_array_element, 4u>, 4u>,
  @align(16)
  arr_m : array<array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>, 4u>,
}

fn tint_unpack_vec3_in_composite(in : array<tint_packed_vec3_f32_array_element, 4u>) -> array<vec3<f32>, 4u> {
  var result : array<vec3<f32>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_1(in : array<array<tint_packed_vec3_f32_array_element, 4u>, 4u>) -> array<array<vec3<f32>, 4u>, 4u> {
  var result : array<array<vec3<f32>, 4u>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_unpack_vec3_in_composite(in[i]);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_2(in : array<tint_packed_vec3_f32_array_element, 3u>) -> mat3x3<f32> {
  var result : mat3x3<f32>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_3(in : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>) -> array<mat3x3<f32>, 4u> {
  var result : array<mat3x3<f32>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_unpack_vec3_in_composite_2(in[i]);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_4(in : array<array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>, 4u>) -> array<array<mat3x3<f32>, 4u>, 4u> {
  var result : array<array<mat3x3<f32>, 4u>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_unpack_vec3_in_composite_3(in[i]);
  }
  return result;
}

fn tint_unpack_vec3_in_composite_5(in : S_tint_packed_vec3) -> S {
  var result : S;
  result.arr_v = tint_unpack_vec3_in_composite_1(in.arr_v);
  result.arr_m = tint_unpack_vec3_in_composite_4(in.arr_m);
  return result;
}

fn tint_unpack_vec3_in_composite_6(in : array<S_tint_packed_vec3, 4u>) -> array<S, 4u> {
  var result : array<S, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = tint_unpack_vec3_in_composite_5(in[i]);
  }
  return result;
}

struct S {
  arr_v : array<array<vec3<f32>, 4>, 4>,
  arr_m : array<array<mat3x3<f32>, 4>, 4>,
}

@group(0) @binding(0) var<storage, read_write> arr_s : array<S_tint_packed_vec3, 4u>;

fn f() {
  var full_var : array<S, 4>;
  var struct_var : S;
  var outer_arr_v_var : array<array<vec3<f32>, 4>, 4>;
  var inner_arr_v_var : array<vec3<f32>, 4>;
  var v_var : vec3<f32>;
  var v_element_var : f32;
  var outer_arr_m_var : array<array<mat3x3<f32>, 4>, 4>;
  var inner_arr_m_var : array<mat3x3<f32>, 4>;
  var m_var : mat3x3<f32>;
  var m_col_var : vec3<f32>;
  var m_element_var : f32;
  full_var = tint_unpack_vec3_in_composite_6(arr_s);
  struct_var = tint_unpack_vec3_in_composite_5(arr_s[0]);
  outer_arr_v_var = tint_unpack_vec3_in_composite_1(arr_s[0].arr_v);
  inner_arr_v_var = tint_unpack_vec3_in_composite(arr_s[0].arr_v[1]);
  v_var = vec3<f32>(arr_s[0].arr_v[1][2].elements);
  v_element_var = arr_s[0].arr_v[1][2].elements.y;
  outer_arr_m_var = tint_unpack_vec3_in_composite_4(arr_s[0].arr_m);
  inner_arr_m_var = tint_unpack_vec3_in_composite_3(arr_s[0].arr_m[1]);
  m_var = tint_unpack_vec3_in_composite_2(arr_s[0].arr_m[1][2]);
  m_col_var = vec3<f32>(arr_s[0].arr_m[1][2][0].elements);
  m_element_var = arr_s[0].arr_m[1][2][0].elements.y;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, RuntimeSizedArray) {
    auto* src = R"(
struct S {
  arr : array<vec3<f32>>,
}

@group(0) @binding(0) var<storage, read_write> arr_v : array<vec3<f32>>;
@group(0) @binding(1) var<storage, read_write> s : S;

fn main() {
  s.arr[0] = arr_v[0];
}
)";

    auto* expect =
        R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  arr : array<tint_packed_vec3_f32_array_element>,
}

struct S {
  arr : array<vec3<f32>>,
}

@group(0) @binding(0) var<storage, read_write> arr_v : array<tint_packed_vec3_f32_array_element>;

@group(0) @binding(1) var<storage, read_write> s : S_tint_packed_vec3;

fn main() {
  s.arr[0].elements = arr_v[0].elements;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, Mat3x3_F16_Uniform) {
    // Test that array element alignment validation rules do not trigger when we rewrite an f16
    // matrix into an array of vec3s in uniform storage.
    auto* src = R"(
enable f16;
enable chromium_experimental_full_ptr_parameters;

@group(0) @binding(0) var<uniform> m : mat3x3<f16>;

fn g(p : ptr<uniform, mat3x3<f16>>) -> vec3<f16> {
  return (*p)[0] + vec3<f16>(1);
}

fn f() {
  let v = g(&m);
}
)";

    auto* expect =
        R"(
enable chromium_internal_relaxed_uniform_layout;
enable f16;
enable chromium_experimental_full_ptr_parameters;

struct tint_packed_vec3_f16_array_element {
  @align(8)
  elements : __packed_vec3<f16>,
}

@group(0) @binding(0) var<uniform> m : array<tint_packed_vec3_f16_array_element, 3u>;

fn g(p : ptr<uniform, array<tint_packed_vec3_f16_array_element, 3u>>) -> vec3<f16> {
  return (vec3<f16>((*(p))[0].elements) + vec3<f16>(1));
}

fn f() {
  let v = g(&(m));
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, MultipleComponentTypes_StructMembers) {
    auto* src = R"(
enable f16;

struct S {
  f : vec3f,
  h : vec3h,
  i : vec3i,
  u : vec3u,
}

@group(0) @binding(0) var<storage, read_write> s : S;

fn f() {
  let f = s.f;
  let h = s.h;
  let i = s.i;
  let u = s.u;
}
)";

    auto* expect =
        R"(
enable chromium_internal_relaxed_uniform_layout;
enable f16;

struct S_tint_packed_vec3 {
  @align(16)
  f : __packed_vec3<f32>,
  @align(8)
  h : __packed_vec3<f16>,
  @align(16)
  i : __packed_vec3<i32>,
  @align(16)
  u : __packed_vec3<u32>,
}

struct S {
  f : vec3f,
  h : vec3h,
  i : vec3i,
  u : vec3u,
}

@group(0) @binding(0) var<storage, read_write> s : S_tint_packed_vec3;

fn f() {
  let f = vec3<f32>(s.f);
  let h = vec3<f16>(s.h);
  let i = vec3<i32>(s.i);
  let u = vec3<u32>(s.u);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, MultipleComponentTypes_ArrayElement) {
    auto* src = R"(
enable f16;

@group(0) @binding(0) var<storage, read_write> arr_f : array<vec3f>;
@group(0) @binding(1) var<storage, read_write> arr_h : array<vec3h>;
@group(0) @binding(2) var<storage, read_write> arr_i : array<vec3i>;
@group(0) @binding(3) var<storage, read_write> arr_u : array<vec3u>;

fn main() {
  let f = arr_f[0];
  let h = arr_h[0];
  let i = arr_i[0];
  let u = arr_u[0];
}
)";

    auto* expect =
        R"(
enable chromium_internal_relaxed_uniform_layout;
enable f16;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct tint_packed_vec3_f16_array_element {
  @align(8)
  elements : __packed_vec3<f16>,
}

struct tint_packed_vec3_i32_array_element {
  @align(16)
  elements : __packed_vec3<i32>,
}

struct tint_packed_vec3_u32_array_element {
  @align(16)
  elements : __packed_vec3<u32>,
}

@group(0) @binding(0) var<storage, read_write> arr_f : array<tint_packed_vec3_f32_array_element>;

@group(0) @binding(1) var<storage, read_write> arr_h : array<tint_packed_vec3_f16_array_element>;

@group(0) @binding(2) var<storage, read_write> arr_i : array<tint_packed_vec3_i32_array_element>;

@group(0) @binding(3) var<storage, read_write> arr_u : array<tint_packed_vec3_u32_array_element>;

fn main() {
  let f = vec3<f32>(arr_f[0].elements);
  let h = vec3<f16>(arr_h[0].elements);
  let i = vec3<i32>(arr_i[0].elements);
  let u = vec3<u32>(arr_u[0].elements);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, Arithmetic_FromRef) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> buffer_v : vec3<f32>;
@group(0) @binding(1) var<storage, read_write> buffer_m : mat3x3<f32>;
@group(0) @binding(2) var<storage, read_write> buffer_arr_v : array<vec3<f32>, 4>;
@group(0) @binding(3) var<storage, read_write> buffer_arr_m : array<mat3x3<f32>, 4>;
@group(0) @binding(4) var<storage, read_write> buffer_nested_arr_v : array<array<vec3<f32>, 4>, 4>;

fn f() {
  var v : vec3<f32> = buffer_v * 2;
  v = -v;
  v = buffer_m * v;
  v = buffer_m[0] + v;
  v = buffer_arr_v[0] + v;
  v = buffer_arr_m[0] * v;
  v = buffer_arr_m[0][1] + v;
  v = buffer_nested_arr_v[0][0] + v;
}
)";

    auto* expect =
        R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

fn tint_unpack_vec3_in_composite(in : array<tint_packed_vec3_f32_array_element, 3u>) -> mat3x3<f32> {
  var result : mat3x3<f32>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

@group(0) @binding(0) var<storage, read_write> buffer_v : __packed_vec3<f32>;

@group(0) @binding(1) var<storage, read_write> buffer_m : array<tint_packed_vec3_f32_array_element, 3u>;

@group(0) @binding(2) var<storage, read_write> buffer_arr_v : array<tint_packed_vec3_f32_array_element, 4u>;

@group(0) @binding(3) var<storage, read_write> buffer_arr_m : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>;

@group(0) @binding(4) var<storage, read_write> buffer_nested_arr_v : array<array<tint_packed_vec3_f32_array_element, 4u>, 4u>;

fn f() {
  var v : vec3<f32> = (vec3<f32>(buffer_v) * 2);
  v = -(v);
  v = (tint_unpack_vec3_in_composite(buffer_m) * v);
  v = (vec3<f32>(buffer_m[0].elements) + v);
  v = (vec3<f32>(buffer_arr_v[0].elements) + v);
  v = (tint_unpack_vec3_in_composite(buffer_arr_m[0]) * v);
  v = (vec3<f32>(buffer_arr_m[0][1].elements) + v);
  v = (vec3<f32>(buffer_nested_arr_v[0][0].elements) + v);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, Arithmetic_FromValue) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> buffer_v : vec3f;

fn f() {
  var v : vec3f = buffer_v;
  v = -vec3f(1, 2, 3);
  v = mat3x3f() * v;
  v = mat3x3f()[0] + v;
  v = array<vec3f, 4>()[0] + v;
  v = array<mat3x3f, 4>()[0] * v;
  v = array<mat3x3f, 4>()[0][1] + v;
  v = array<array<vec3f, 4>, 4>()[0][0] + v;
}
)";

    auto* expect =
        R"(
enable chromium_internal_relaxed_uniform_layout;

@group(0) @binding(0) var<storage, read_write> buffer_v : __packed_vec3<f32>;

fn f() {
  var v : vec3f = vec3<f32>(buffer_v);
  v = -(vec3f(1, 2, 3));
  v = (mat3x3f() * v);
  v = (mat3x3f()[0] + v);
  v = (array<vec3f, 4>()[0] + v);
  v = (array<mat3x3f, 4>()[0] * v);
  v = (array<mat3x3f, 4>()[0][1] + v);
  v = (array<array<vec3f, 4>, 4>()[0][0] + v);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, Arithmetic_FromRefStruct) {
    auto* src = R"(
struct S {
  v : vec3<f32>,
  m : mat3x3<f32>,
  arr_v : array<vec3<f32>, 4>,
  arr_m : array<mat3x3<f32>, 4>,
  nested_arr_v : array<array<vec3<f32>, 4>, 4>,
}

@group(0) @binding(0) var<storage, read_write> buffer : S;

fn f() {
  var v : vec3<f32> = buffer.v * 2;
  v = -v;
  v = buffer.m * v;
  v = buffer.m[0] + v;
  v = buffer.arr_v[0] + v;
  v = buffer.arr_m[0] * v;
  v = buffer.arr_m[0][1] + v;
  v = buffer.nested_arr_v[0][0] + v;
}
)";

    auto* expect =
        R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  v : __packed_vec3<f32>,
  @align(16)
  m : array<tint_packed_vec3_f32_array_element, 3u>,
  @align(16)
  arr_v : array<tint_packed_vec3_f32_array_element, 4u>,
  @align(16)
  arr_m : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>,
  @align(16)
  nested_arr_v : array<array<tint_packed_vec3_f32_array_element, 4u>, 4u>,
}

fn tint_unpack_vec3_in_composite(in : array<tint_packed_vec3_f32_array_element, 3u>) -> mat3x3<f32> {
  var result : mat3x3<f32>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

struct S {
  v : vec3<f32>,
  m : mat3x3<f32>,
  arr_v : array<vec3<f32>, 4>,
  arr_m : array<mat3x3<f32>, 4>,
  nested_arr_v : array<array<vec3<f32>, 4>, 4>,
}

@group(0) @binding(0) var<storage, read_write> buffer : S_tint_packed_vec3;

fn f() {
  var v : vec3<f32> = (vec3<f32>(buffer.v) * 2);
  v = -(v);
  v = (tint_unpack_vec3_in_composite(buffer.m) * v);
  v = (vec3<f32>(buffer.m[0].elements) + v);
  v = (vec3<f32>(buffer.arr_v[0].elements) + v);
  v = (tint_unpack_vec3_in_composite(buffer.arr_m[0]) * v);
  v = (vec3<f32>(buffer.arr_m[0][1].elements) + v);
  v = (vec3<f32>(buffer.nested_arr_v[0][0].elements) + v);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, Arithmetic_FromValueStruct) {
    auto* src = R"(
struct S {
  v : vec3<f32>,
  m : mat3x3<f32>,
  arr_v : array<vec3<f32>, 4>,
  arr_m : array<mat3x3<f32>, 4>,
  nested_arr_v : array<array<vec3<f32>, 4>, 4>,
}

@group(0) @binding(0) var<storage, read_write> buffer : S;

fn f() {
  var v : vec3<f32> = S().v;
  v = -S().v;
  v = S().m * v;
  v = S().m[0] + v;
  v = S().arr_v[0] + v;
  v = S().arr_m[0] * v;
  v = S().arr_m[0][1] + v;
  v = S().nested_arr_v[0][0] + v;
}
)";

    auto* expect =
        R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  v : __packed_vec3<f32>,
  @align(16)
  m : array<tint_packed_vec3_f32_array_element, 3u>,
  @align(16)
  arr_v : array<tint_packed_vec3_f32_array_element, 4u>,
  @align(16)
  arr_m : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>,
  @align(16)
  nested_arr_v : array<array<tint_packed_vec3_f32_array_element, 4u>, 4u>,
}

struct S {
  v : vec3<f32>,
  m : mat3x3<f32>,
  arr_v : array<vec3<f32>, 4>,
  arr_m : array<mat3x3<f32>, 4>,
  nested_arr_v : array<array<vec3<f32>, 4>, 4>,
}

@group(0) @binding(0) var<storage, read_write> buffer : S_tint_packed_vec3;

fn f() {
  var v : vec3<f32> = S().v;
  v = -(S().v);
  v = (S().m * v);
  v = (S().m[0] + v);
  v = (S().arr_v[0] + v);
  v = (S().arr_m[0] * v);
  v = (S().arr_m[0][1] + v);
  v = (S().nested_arr_v[0][0] + v);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, Aliases) {
    auto* src = R"(
alias VecArray = array<vec3<f32>, 4>;
alias MatArray = array<mat3x3<f32>, 4>;
alias NestedArray = array<VecArray, 4>;

struct S {
  v : VecArray,
  m : MatArray,
  n : NestedArray,
}

@group(0) @binding(0) var<storage, read_write> s : S;
@group(0) @binding(1) var<storage, read_write> arr_v : VecArray;
@group(0) @binding(2) var<storage, read_write> arr_m : MatArray;
@group(0) @binding(3) var<storage, read_write> arr_n : NestedArray;

fn g(p : ptr<function, VecArray>) {
}

fn f() {
  var f_arr_v : VecArray = s.v;
  g(&f_arr_v);

  arr_v = s.v;
  arr_m[0] = s.m[0];
  arr_n[1][2] = s.n[1][2];
}
)";

    auto* expect =
        R"(
enable chromium_internal_relaxed_uniform_layout;

struct tint_packed_vec3_f32_array_element {
  @align(16)
  elements : __packed_vec3<f32>,
}

struct S_tint_packed_vec3 {
  @align(16)
  v : array<tint_packed_vec3_f32_array_element, 4u>,
  @align(16)
  m : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>,
  @align(16)
  n : array<array<tint_packed_vec3_f32_array_element, 4u>, 4u>,
}

fn tint_unpack_vec3_in_composite(in : array<tint_packed_vec3_f32_array_element, 4u>) -> array<vec3<f32>, 4u> {
  var result : array<vec3<f32>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    result[i] = vec3<f32>(in[i].elements);
  }
  return result;
}

alias VecArray = array<vec3<f32>, 4>;

alias MatArray = array<mat3x3<f32>, 4>;

alias NestedArray = array<VecArray, 4>;

struct S {
  v : VecArray,
  m : MatArray,
  n : NestedArray,
}

@group(0) @binding(0) var<storage, read_write> s : S_tint_packed_vec3;

@group(0) @binding(1) var<storage, read_write> arr_v : array<tint_packed_vec3_f32_array_element, 4u>;

@group(0) @binding(2) var<storage, read_write> arr_m : array<array<tint_packed_vec3_f32_array_element, 3u>, 4u>;

@group(0) @binding(3) var<storage, read_write> arr_n : array<array<tint_packed_vec3_f32_array_element, 4u>, 4u>;

fn g(p : ptr<function, VecArray>) {
}

fn f() {
  var f_arr_v : VecArray = tint_unpack_vec3_in_composite(s.v);
  g(&(f_arr_v));
  arr_v = s.v;
  arr_m[0] = s.m[0];
  arr_n[1][2].elements = s.n[1][2].elements;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, Vec3Bool) {
    // Make sure that we don't rewrite vec3<bool> types, as the `packed_bool<n>` types are reserved
    // in MSL and might not be supported everywhere.
    auto* src = R"(
struct S {
  vf : vec3<f32>,
  af : array<vec3<f32>, 4>,
  vb : vec3<bool>,
  ab : array<vec3<bool>, 4>,
}

// Create a vec3 storage buffer so that the transform is not skipped.
@group(0) @binding(0) var<storage, read_write> buffer : vec3<f32>;

fn f() {
  var f : S;
  f.vf = buffer;
  buffer = f.af[0];
}
)";

    auto* expect =
        R"(
enable chromium_internal_relaxed_uniform_layout;

struct S {
  vf : vec3<f32>,
  af : array<vec3<f32>, 4>,
  vb : vec3<bool>,
  ab : array<vec3<bool>, 4>,
}

@group(0) @binding(0) var<storage, read_write> buffer : __packed_vec3<f32>;

fn f() {
  var f : S;
  f.vf = vec3<f32>(buffer);
  buffer = __packed_vec3<f32>(f.af[0]);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::transform
