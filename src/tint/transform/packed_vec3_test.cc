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

#include "src/tint/transform/test_helper.h"
#include "src/tint/utils/string.h"

namespace tint::transform {
namespace {

using PackedVec3Test = TransformTest;

TEST_F(PackedVec3Test, ShouldRun_EmptyModule) {
    auto* src = R"()";

    EXPECT_FALSE(ShouldRun<PackedVec3>(src));
}

TEST_F(PackedVec3Test, ShouldRun_NonHostSharableStruct) {
    auto* src = R"(
struct S {
  v : vec3<f32>,
}

fn f() {
  var v : S; // function address-space - not host sharable
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

@group(0) @binding(0) var<uniform> P : S; // Host sharable
)";

    EXPECT_FALSE(ShouldRun<PackedVec3>(src));
}

TEST_F(PackedVec3Test, ShouldRun_HostSharableStruct) {
    auto* src = R"(
struct S {
  v : vec3<f32>,
}

@group(0) @binding(0) var<uniform> P : S; // Host sharable
)";

    EXPECT_TRUE(ShouldRun<PackedVec3>(src));
}

TEST_F(PackedVec3Test, UniformAddressSpace) {
    auto* src = R"(
struct S {
  v : vec3<f32>,
}

@group(0) @binding(0) var<uniform> P : S;

fn f() {
  let x = P.v;
}
)";

    auto* expect = R"(
struct S {
  @internal(packed_vector)
  v : vec3<f32>,
}

@group(0) @binding(0) var<uniform> P : S;

fn f() {
  let x = vec3<f32>(P.v);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, StorageAddressSpace) {
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
struct S {
  @internal(packed_vector)
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let x = vec3<f32>(P.v);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ExistingMemberAttributes) {
    auto* src = R"(
struct S {
  @align(32) @size(64) v : vec3<f32>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let x = P.v;
}
)";

    auto* expect = R"(
struct S {
  @internal(packed_vector) @align(32) @size(64)
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let x = vec3<f32>(P.v);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, MultipleVectors) {
    auto* src = R"(
struct S {
  v2_a : vec2<f32>,
  v3_a : vec3<f32>,
  v4_a : vec4<f32>,
  v2_b : vec2<f32>,
  v3_b : vec3<f32>,
  v4_b : vec4<f32>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let v2_a = P.v2_a;
  let v3_a = P.v3_a;
  let v4_a = P.v4_a;
  let v2_b = P.v2_b;
  let v3_b = P.v3_b;
  let v4_b = P.v4_b;
}
)";

    auto* expect = R"(
struct S {
  v2_a : vec2<f32>,
  @internal(packed_vector)
  v3_a : vec3<f32>,
  v4_a : vec4<f32>,
  v2_b : vec2<f32>,
  @internal(packed_vector)
  v3_b : vec3<f32>,
  v4_b : vec4<f32>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let v2_a = P.v2_a;
  let v3_a = vec3<f32>(P.v3_a);
  let v4_a = P.v4_a;
  let v2_b = P.v2_b;
  let v3_b = vec3<f32>(P.v3_b);
  let v4_b = P.v4_b;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, MixedAddressSpace) {
    auto* src = R"(
struct S {
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  var f : S;
  let x = f.v;
}
)";

    auto* expect = R"(
struct S {
  @internal(packed_vector)
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  var f : S;
  let x = vec3<f32>(f.v);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ReadMemberAccessChain) {
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
struct S {
  @internal(packed_vector)
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let x = P.v.yz.x;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ReadVector) {
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
struct S {
  @internal(packed_vector)
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let x = vec3<f32>(P.v);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ReadIndexAccessor) {
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
struct S {
  @internal(packed_vector)
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let x = P.v[1];
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ReadViaStructPtrDirect) {
    auto* src = R"(
struct S {
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let x = (*(&(*(&P)))).v;
}
)";

    auto* expect = R"(
struct S {
  @internal(packed_vector)
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let x = vec3<f32>((*(&(*(&(P))))).v);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ReadViaVectorPtrDirect) {
    auto* src = R"(
struct S {
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let x = *(&(*(&(P.v))));
}
)";

    auto* expect = R"(
struct S {
  @internal(packed_vector)
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let x = vec3<f32>(*(&(*(&(P.v)))));
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ReadViaStructPtrViaLet) {
    auto* src = R"(
struct S {
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let p0 = &P;
  let p1 = &(*(p0));
  let a = (*p1).v;
  let p2 = &(*(p1));
  let b = (*p2).v;
  let c = (*p2).v;
}
)";

    auto* expect = R"(
struct S {
  @internal(packed_vector)
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let p0 = &(P);
  let p1 = &(*(p0));
  let a = vec3<f32>((*(p1)).v);
  let p2 = &(*(p1));
  let b = vec3<f32>((*(p2)).v);
  let c = vec3<f32>((*(p2)).v);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ReadViaVectorPtrViaLet) {
    auto* src = R"(
struct S {
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let p0 = &(P.v);
  let p1 = &(*(p0));
  let a = *p1;
  let p2 = &(*(p1));
  let b = *p2;
  let c = *p2;
}
)";

    auto* expect = R"(
struct S {
  @internal(packed_vector)
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let p0 = &(P.v);
  let p1 = &(*(p0));
  let a = vec3<f32>(*(p1));
  let p2 = &(*(p1));
  let b = vec3<f32>(*(p2));
  let c = vec3<f32>(*(p2));
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ReadUnaryOp) {
    auto* src = R"(
struct S {
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let x = -P.v;
}
)";

    auto* expect = R"(
struct S {
  @internal(packed_vector)
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let x = -(vec3<f32>(P.v));
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, ReadBinaryOp) {
    auto* src = R"(
struct S {
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let x = P.v + P.v;
}
)";

    auto* expect = R"(
struct S {
  @internal(packed_vector)
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage> P : S;

fn f() {
  let x = (vec3<f32>(P.v) + vec3<f32>(P.v));
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, WriteVector) {
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
struct S {
  @internal(packed_vector)
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage, read_write> P : S;

fn f() {
  P.v = vec3(1.23);
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, WriteMemberAccess) {
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
struct S {
  @internal(packed_vector)
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage, read_write> P : S;

fn f() {
  P.v.y = 1.23;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PackedVec3Test, WriteIndexAccessor) {
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
struct S {
  @internal(packed_vector)
  v : vec3<f32>,
}

@group(0) @binding(0) var<storage, read_write> P : S;

fn f() {
  P.v[1] = 1.23;
}
)";

    DataMap data;
    auto got = Run<PackedVec3>(src, data);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::transform
