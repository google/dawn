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

#include "src/tint/transform/preserve_padding.h"

#include <utility>

#include "src/tint/transform/test_helper.h"

namespace tint::transform {
namespace {

using PreservePaddingTest = TransformTest;

TEST_F(PreservePaddingTest, ShouldRun_EmptyModule) {
    auto* src = R"()";

    EXPECT_FALSE(ShouldRun<PreservePadding>(src));
}

TEST_F(PreservePaddingTest, ShouldRun_NonStructVec3) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> v : vec3<u32>;

@compute @workgroup_size(1)
fn foo() {
  v = vec3<u32>();
}
    )";

    EXPECT_FALSE(ShouldRun<PreservePadding>(src));
}

TEST_F(PreservePaddingTest, ShouldRun_StructWithoutPadding) {
    auto* src = R"(
struct S {
  a : u32,
  b : u32,
  c : u32,
  d : u32,
  e : vec3<u32>,
  f : u32,
}

@group(0) @binding(0) var<storage, read_write> v : S;

@compute @workgroup_size(1)
fn foo() {
  v = S();
}
    )";

    EXPECT_FALSE(ShouldRun<PreservePadding>(src));
}

TEST_F(PreservePaddingTest, ShouldRun_ArrayWithoutPadding) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> v : array<vec4<u32>, 4>;

@compute @workgroup_size(1)
fn foo() {
  v = array<vec4<u32>, 4>();
}
    )";

    EXPECT_FALSE(ShouldRun<PreservePadding>(src));
}

TEST_F(PreservePaddingTest, EmptyModule) {
    auto* src = R"()";

    auto* expect = src;

    auto got = Run<PreservePadding>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PreservePaddingTest, StructTrailingPadding) {
    auto* src = R"(
struct S {
  a : u32,
  b : u32,
  c : u32,
  d : u32,
  e : vec3<u32>,
}

@group(0) @binding(0) var<storage, read_write> v : S;

@compute @workgroup_size(1)
fn foo() {
  v = S();
}
)";

    auto* expect = R"(
enable chromium_experimental_full_ptr_parameters;

struct S {
  a : u32,
  b : u32,
  c : u32,
  d : u32,
  e : vec3<u32>,
}

@group(0) @binding(0) var<storage, read_write> v : S;

fn assign_and_preserve_padding(dest : ptr<storage, S, read_write>, value : S) {
  (*(dest)).a = value.a;
  (*(dest)).b = value.b;
  (*(dest)).c = value.c;
  (*(dest)).d = value.d;
  (*(dest)).e = value.e;
}

@compute @workgroup_size(1)
fn foo() {
  assign_and_preserve_padding(&(v), S());
}
)";

    auto got = Run<PreservePadding>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PreservePaddingTest, StructInternalPadding) {
    auto* src = R"(
struct S {
  a : u32,
  b : vec4<u32>,
}

@group(0) @binding(0) var<storage, read_write> v : S;

@compute @workgroup_size(1)
fn foo() {
  v = S();
}
)";

    auto* expect = R"(
enable chromium_experimental_full_ptr_parameters;

struct S {
  a : u32,
  b : vec4<u32>,
}

@group(0) @binding(0) var<storage, read_write> v : S;

fn assign_and_preserve_padding(dest : ptr<storage, S, read_write>, value : S) {
  (*(dest)).a = value.a;
  (*(dest)).b = value.b;
}

@compute @workgroup_size(1)
fn foo() {
  assign_and_preserve_padding(&(v), S());
}
)";

    auto got = Run<PreservePadding>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PreservePaddingTest, StructExplicitSize_TrailingPadding) {
    auto* src = R"(
struct S {
  @size(16) a : u32,
}

@group(0) @binding(0) var<storage, read_write> v : S;

@compute @workgroup_size(1)
fn foo() {
  v = S();
}
)";

    auto* expect = R"(
enable chromium_experimental_full_ptr_parameters;

struct S {
  @size(16)
  a : u32,
}

@group(0) @binding(0) var<storage, read_write> v : S;

fn assign_and_preserve_padding(dest : ptr<storage, S, read_write>, value : S) {
  (*(dest)).a = value.a;
}

@compute @workgroup_size(1)
fn foo() {
  assign_and_preserve_padding(&(v), S());
}
)";

    auto got = Run<PreservePadding>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PreservePaddingTest, StructExplicitSize_InternalPadding) {
    auto* src = R"(
struct S {
  @size(16) a : u32,
  b : u32,
}

@group(0) @binding(0) var<storage, read_write> v : S;

@compute @workgroup_size(1)
fn foo() {
  v = S();
}
)";

    auto* expect = R"(
enable chromium_experimental_full_ptr_parameters;

struct S {
  @size(16)
  a : u32,
  b : u32,
}

@group(0) @binding(0) var<storage, read_write> v : S;

fn assign_and_preserve_padding(dest : ptr<storage, S, read_write>, value : S) {
  (*(dest)).a = value.a;
  (*(dest)).b = value.b;
}

@compute @workgroup_size(1)
fn foo() {
  assign_and_preserve_padding(&(v), S());
}
)";

    auto got = Run<PreservePadding>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PreservePaddingTest, NestedStructs) {
    auto* src = R"(
struct S1 {
  a1 : u32,
  b1 : vec3<u32>,
  c1 : u32,
}

struct S2 {
  a2 : u32,
  b2 : S1,
  c2 : S1,
}

struct S3 {
  a3 : S1,
  b3 : S2,
  c3 : S2,
}

@group(0) @binding(0) var<storage, read_write> v : S3;

@compute @workgroup_size(1)
fn foo() {
  v = S3();
}
)";

    auto* expect = R"(
enable chromium_experimental_full_ptr_parameters;

struct S1 {
  a1 : u32,
  b1 : vec3<u32>,
  c1 : u32,
}

struct S2 {
  a2 : u32,
  b2 : S1,
  c2 : S1,
}

struct S3 {
  a3 : S1,
  b3 : S2,
  c3 : S2,
}

@group(0) @binding(0) var<storage, read_write> v : S3;

fn assign_and_preserve_padding_1(dest : ptr<storage, S1, read_write>, value : S1) {
  (*(dest)).a1 = value.a1;
  (*(dest)).b1 = value.b1;
  (*(dest)).c1 = value.c1;
}

fn assign_and_preserve_padding_2(dest : ptr<storage, S2, read_write>, value : S2) {
  (*(dest)).a2 = value.a2;
  assign_and_preserve_padding_1(&((*(dest)).b2), value.b2);
  assign_and_preserve_padding_1(&((*(dest)).c2), value.c2);
}

fn assign_and_preserve_padding(dest : ptr<storage, S3, read_write>, value : S3) {
  assign_and_preserve_padding_1(&((*(dest)).a3), value.a3);
  assign_and_preserve_padding_2(&((*(dest)).b3), value.b3);
  assign_and_preserve_padding_2(&((*(dest)).c3), value.c3);
}

@compute @workgroup_size(1)
fn foo() {
  assign_and_preserve_padding(&(v), S3());
}
)";

    auto got = Run<PreservePadding>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PreservePaddingTest, ArrayOfVec3) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> v : array<vec3<u32>, 4>;

@compute @workgroup_size(1)
fn foo() {
  v = array<vec3<u32>, 4>();
}
)";

    auto* expect = R"(
enable chromium_experimental_full_ptr_parameters;

@group(0) @binding(0) var<storage, read_write> v : array<vec3<u32>, 4>;

fn assign_and_preserve_padding(dest : ptr<storage, array<vec3<u32>, 4u>, read_write>, value : array<vec3<u32>, 4u>) {
  for(var i = 0u; (i < 4u); i = (i + 1u)) {
    (*(dest))[i] = value[i];
  }
}

@compute @workgroup_size(1)
fn foo() {
  assign_and_preserve_padding(&(v), array<vec3<u32>, 4>());
}
)";

    auto got = Run<PreservePadding>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PreservePaddingTest, ArrayOfArray) {
    auto* src = R"(
alias Array = array<array<vec3<u32>, 4>, 3>;

@group(0) @binding(0) var<storage, read_write> v : Array;

@compute @workgroup_size(1)
fn foo() {
  v = Array();
}
)";

    auto* expect = R"(
enable chromium_experimental_full_ptr_parameters;

alias Array = array<array<vec3<u32>, 4>, 3>;

@group(0) @binding(0) var<storage, read_write> v : Array;

fn assign_and_preserve_padding_1(dest : ptr<storage, array<vec3<u32>, 4u>, read_write>, value : array<vec3<u32>, 4u>) {
  for(var i = 0u; (i < 4u); i = (i + 1u)) {
    (*(dest))[i] = value[i];
  }
}

fn assign_and_preserve_padding(dest : ptr<storage, array<array<vec3<u32>, 4u>, 3u>, read_write>, value : array<array<vec3<u32>, 4u>, 3u>) {
  for(var i = 0u; (i < 3u); i = (i + 1u)) {
    assign_and_preserve_padding_1(&((*(dest))[i]), value[i]);
  }
}

@compute @workgroup_size(1)
fn foo() {
  assign_and_preserve_padding(&(v), Array());
}
)";

    auto got = Run<PreservePadding>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PreservePaddingTest, ArrayOfStructOfArray) {
    auto* src = R"(
struct S {
  a : u32,
  b : array<vec3<u32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> v : array<S, 3>;

@compute @workgroup_size(1)
fn foo() {
  v = array<S, 3>();
}
)";

    auto* expect = R"(
enable chromium_experimental_full_ptr_parameters;

struct S {
  a : u32,
  b : array<vec3<u32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> v : array<S, 3>;

fn assign_and_preserve_padding_2(dest : ptr<storage, array<vec3<u32>, 4u>, read_write>, value : array<vec3<u32>, 4u>) {
  for(var i = 0u; (i < 4u); i = (i + 1u)) {
    (*(dest))[i] = value[i];
  }
}

fn assign_and_preserve_padding_1(dest : ptr<storage, S, read_write>, value : S) {
  (*(dest)).a = value.a;
  assign_and_preserve_padding_2(&((*(dest)).b), value.b);
}

fn assign_and_preserve_padding(dest : ptr<storage, array<S, 3u>, read_write>, value : array<S, 3u>) {
  for(var i = 0u; (i < 3u); i = (i + 1u)) {
    assign_and_preserve_padding_1(&((*(dest))[i]), value[i]);
  }
}

@compute @workgroup_size(1)
fn foo() {
  assign_and_preserve_padding(&(v), array<S, 3>());
}
)";

    auto got = Run<PreservePadding>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PreservePaddingTest, Mat3x3) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> m : mat3x3<f32>;

@compute @workgroup_size(1)
fn foo() {
  m = mat3x3<f32>();
}
)";

    auto* expect = R"(
enable chromium_experimental_full_ptr_parameters;

@group(0) @binding(0) var<storage, read_write> m : mat3x3<f32>;

fn assign_and_preserve_padding(dest : ptr<storage, mat3x3<f32>, read_write>, value : mat3x3<f32>) {
  (*(dest))[0u] = value[0u];
  (*(dest))[1u] = value[1u];
  (*(dest))[2u] = value[2u];
}

@compute @workgroup_size(1)
fn foo() {
  assign_and_preserve_padding(&(m), mat3x3<f32>());
}
)";

    auto got = Run<PreservePadding>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PreservePaddingTest, Mat3x3_InStruct) {
    auto* src = R"(
struct S {
  a : u32,
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<storage, read_write> buffer : S;

@compute @workgroup_size(1)
fn foo() {
  buffer = S();
}
)";

    auto* expect = R"(
enable chromium_experimental_full_ptr_parameters;

struct S {
  a : u32,
  m : mat3x3<f32>,
}

@group(0) @binding(0) var<storage, read_write> buffer : S;

fn assign_and_preserve_padding_1(dest : ptr<storage, mat3x3<f32>, read_write>, value : mat3x3<f32>) {
  (*(dest))[0u] = value[0u];
  (*(dest))[1u] = value[1u];
  (*(dest))[2u] = value[2u];
}

fn assign_and_preserve_padding(dest : ptr<storage, S, read_write>, value : S) {
  (*(dest)).a = value.a;
  assign_and_preserve_padding_1(&((*(dest)).m), value.m);
}

@compute @workgroup_size(1)
fn foo() {
  assign_and_preserve_padding(&(buffer), S());
}
)";

    auto got = Run<PreservePadding>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PreservePaddingTest, ArrayOfMat3x3) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> arr_m : array<mat3x3<f32>, 4>;

@compute @workgroup_size(1)
fn foo() {
  arr_m = array<mat3x3<f32>, 4>();
  arr_m[0] = mat3x3<f32>();
}
)";

    auto* expect = R"(
enable chromium_experimental_full_ptr_parameters;

@group(0) @binding(0) var<storage, read_write> arr_m : array<mat3x3<f32>, 4>;

fn assign_and_preserve_padding_1(dest : ptr<storage, mat3x3<f32>, read_write>, value : mat3x3<f32>) {
  (*(dest))[0u] = value[0u];
  (*(dest))[1u] = value[1u];
  (*(dest))[2u] = value[2u];
}

fn assign_and_preserve_padding(dest : ptr<storage, array<mat3x3<f32>, 4u>, read_write>, value : array<mat3x3<f32>, 4u>) {
  for(var i = 0u; (i < 4u); i = (i + 1u)) {
    assign_and_preserve_padding_1(&((*(dest))[i]), value[i]);
  }
}

@compute @workgroup_size(1)
fn foo() {
  assign_and_preserve_padding(&(arr_m), array<mat3x3<f32>, 4>());
  assign_and_preserve_padding_1(&(arr_m[0]), mat3x3<f32>());
}
)";

    auto got = Run<PreservePadding>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PreservePaddingTest, NoModify_Vec3) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> v : vec3<u32>;

@compute @workgroup_size(1)
fn foo() {
  v = vec3<u32>();
}
)";

    auto* expect = src;

    auto got = Run<PreservePadding>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PreservePaddingTest, AvoidDuplicateEnables) {
    auto* src = R"(
enable chromium_experimental_full_ptr_parameters;

struct S {
  @size(16) a : u32,
}

@group(0) @binding(0) var<storage, read_write> v : S;

@compute @workgroup_size(1)
fn foo() {
  v = S();
}
)";

    auto* expect = R"(
enable chromium_experimental_full_ptr_parameters;

struct S {
  @size(16)
  a : u32,
}

@group(0) @binding(0) var<storage, read_write> v : S;

fn assign_and_preserve_padding(dest : ptr<storage, S, read_write>, value : S) {
  (*(dest)).a = value.a;
}

@compute @workgroup_size(1)
fn foo() {
  assign_and_preserve_padding(&(v), S());
}
)";

    auto got = Run<PreservePadding>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PreservePaddingTest, NoModify_StructNoPadding) {
    auto* src = R"(
struct S {
  a : u32,
  b : u32,
  c : u32,
  d : u32,
  e : vec4<u32>,
}

@group(0) @binding(0) var<storage, read_write> v : S;

@compute @workgroup_size(1)
fn foo() {
  v = S();
}
)";

    auto* expect = src;

    auto got = Run<PreservePadding>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PreservePaddingTest, NoModify_ArrayNoPadding) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> v : array<vec4<u32>, 4>;

@compute @workgroup_size(1)
fn foo() {
  v = array<vec4<u32>, 4>();
}
)";

    auto* expect = src;

    auto got = Run<PreservePadding>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PreservePaddingTest, NoModify_ArrayOfStructNoPadding) {
    auto* src = R"(
struct S {
  a : u32,
  b : u32,
  c : u32,
  d : u32,
  e : vec4<u32>,
}

@group(0) @binding(0) var<storage, read_write> v : array<S, 4>;

@compute @workgroup_size(1)
fn foo() {
  v = array<S, 4>();
}
)";

    auto* expect = src;

    auto got = Run<PreservePadding>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PreservePaddingTest, NoModify_Workgroup) {
    auto* src = R"(
struct S {
  a : u32,
  b : vec3<u32>,
}

var<workgroup> v : S;

@compute @workgroup_size(1)
fn foo() {
  v = S();
}
)";

    auto* expect = src;

    auto got = Run<PreservePadding>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PreservePaddingTest, NoModify_Private) {
    auto* src = R"(
struct S {
  a : u32,
  b : vec3<u32>,
}

var<private> v : S;

@compute @workgroup_size(1)
fn foo() {
  v = S();
}
)";

    auto* expect = src;

    auto got = Run<PreservePadding>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PreservePaddingTest, NoModify_Function) {
    auto* src = R"(
struct S {
  a : u32,
  b : vec3<u32>,
}

@compute @workgroup_size(1)
fn foo() {
  var<function> v : S;
  v = S();
}
)";

    auto* expect = src;

    auto got = Run<PreservePadding>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::transform
