// Copyright 2020 The Tint Authors.
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

#include "src/tint/transform/robustness.h"

#include "src/tint/transform/test_helper.h"

namespace tint::transform {
namespace {

using RobustnessTest = TransformTest;

TEST_F(RobustnessTest, Array_Idx_Clamp) {
    auto* src = R"(
var<private> a : array<f32, 3>;

let c : u32 = 1u;

fn f() {
  let b : f32 = a[c];
}
)";

    auto* expect = R"(
var<private> a : array<f32, 3>;

let c : u32 = 1u;

fn f() {
  let b : f32 = a[1u];
}
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Array_Idx_Clamp_OutOfOrder) {
    auto* src = R"(
fn f() {
  let b : f32 = a[c];
}

let c : u32 = 1u;

var<private> a : array<f32, 3>;
)";

    auto* expect = R"(
fn f() {
  let b : f32 = a[1u];
}

let c : u32 = 1u;

var<private> a : array<f32, 3>;
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Array_Idx_Nested_Scalar) {
    auto* src = R"(
var<private> a : array<f32, 3>;

var<private> b : array<i32, 5>;

var<private> i : u32;

fn f() {
  var c : f32 = a[ b[i] ];
}
)";

    auto* expect = R"(
var<private> a : array<f32, 3>;

var<private> b : array<i32, 5>;

var<private> i : u32;

fn f() {
  var c : f32 = a[min(u32(b[min(i, 4u)]), 2u)];
}
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Array_Idx_Nested_Scalar_OutOfOrder) {
    auto* src = R"(
fn f() {
  var c : f32 = a[ b[i] ];
}

var<private> i : u32;

var<private> b : array<i32, 5>;

var<private> a : array<f32, 3>;
)";

    auto* expect = R"(
fn f() {
  var c : f32 = a[min(u32(b[min(i, 4u)]), 2u)];
}

var<private> i : u32;

var<private> b : array<i32, 5>;

var<private> a : array<f32, 3>;
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Array_Idx_Scalar) {
    auto* src = R"(
var<private> a : array<f32, 3>;

fn f() {
  var b : f32 = a[1];
}
)";

    auto* expect = R"(
var<private> a : array<f32, 3>;

fn f() {
  var b : f32 = a[1];
}
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Array_Idx_Scalar_OutOfOrder) {
    auto* src = R"(
fn f() {
  var b : f32 = a[1];
}

var<private> a : array<f32, 3>;
)";

    auto* expect = R"(
fn f() {
  var b : f32 = a[1];
}

var<private> a : array<f32, 3>;
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Array_Idx_Expr) {
    auto* src = R"(
var<private> a : array<f32, 3>;

var<private> c : i32;

fn f() {
  var b : f32 = a[c + 2 - 3];
}
)";

    auto* expect = R"(
var<private> a : array<f32, 3>;

var<private> c : i32;

fn f() {
  var b : f32 = a[min(u32(((c + 2) - 3)), 2u)];
}
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Array_Idx_Expr_OutOfOrder) {
    auto* src = R"(
fn f() {
  var b : f32 = a[c + 2 - 3];
}

var<private> c : i32;

var<private> a : array<f32, 3>;
)";

    auto* expect = R"(
fn f() {
  var b : f32 = a[min(u32(((c + 2) - 3)), 2u)];
}

var<private> c : i32;

var<private> a : array<f32, 3>;
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Array_Idx_Negative) {
    auto* src = R"(
var<private> a : array<f32, 3>;

fn f() {
  var b : f32 = a[-1];
}
)";

    auto* expect = R"(
var<private> a : array<f32, 3>;

fn f() {
  var b : f32 = a[0];
}
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Array_Idx_Negative_OutOfOrder) {
    auto* src = R"(
fn f() {
  var b : f32 = a[-1];
}

var<private> a : array<f32, 3>;
)";

    auto* expect = R"(
fn f() {
  var b : f32 = a[0];
}

var<private> a : array<f32, 3>;
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Array_Idx_OutOfBounds) {
    auto* src = R"(
var<private> a : array<f32, 3>;

fn f() {
  var b : f32 = a[3];
}
)";

    auto* expect = R"(
var<private> a : array<f32, 3>;

fn f() {
  var b : f32 = a[2];
}
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Array_Idx_OutOfBounds_OutOfOrder) {
    auto* src = R"(
fn f() {
  var b : f32 = a[3];
}

var<private> a : array<f32, 3>;
)";

    auto* expect = R"(
fn f() {
  var b : f32 = a[2];
}

var<private> a : array<f32, 3>;
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

// TODO(crbug.com/tint/1177) - Validation currently forbids arrays larger than
// 0xffffffff. If WGSL supports 64-bit indexing, re-enable this test.
TEST_F(RobustnessTest, DISABLED_LargeArrays_Idx) {
    auto* src = R"(
struct S {
  a : array<f32, 0x7fffffff>,
  b : array<f32>,
};
@group(0) @binding(0) var<storage, read> s : S;

fn f() {
  // Signed
  var i32_a1 : f32 = s.a[ 0x7ffffffe];
  var i32_a2 : f32 = s.a[ 1];
  var i32_a3 : f32 = s.a[ 0];
  var i32_a4 : f32 = s.a[-1];
  var i32_a5 : f32 = s.a[-0x7fffffff];

  var i32_b1 : f32 = s.b[ 0x7ffffffe];
  var i32_b2 : f32 = s.b[ 1];
  var i32_b3 : f32 = s.b[ 0];
  var i32_b4 : f32 = s.b[-1];
  var i32_b5 : f32 = s.b[-0x7fffffff];

  // Unsigned
  var u32_a1 : f32 = s.a[0u];
  var u32_a2 : f32 = s.a[1u];
  var u32_a3 : f32 = s.a[0x7ffffffeu];
  var u32_a4 : f32 = s.a[0x7fffffffu];
  var u32_a5 : f32 = s.a[0x80000000u];
  var u32_a6 : f32 = s.a[0xffffffffu];

  var u32_b1 : f32 = s.b[0u];
  var u32_b2 : f32 = s.b[1u];
  var u32_b3 : f32 = s.b[0x7ffffffeu];
  var u32_b4 : f32 = s.b[0x7fffffffu];
  var u32_b5 : f32 = s.b[0x80000000u];
  var u32_b6 : f32 = s.b[0xffffffffu];
}
)";

    auto* expect = R"(
struct S {
  a : array<f32, 2147483647>,
  b : array<f32>,
};

@group(0) @binding(0) var<storage, read> s : S;

fn f() {
  var i32_a1 : f32 = s.a[2147483646];
  var i32_a2 : f32 = s.a[1];
  var i32_a3 : f32 = s.a[0];
  var i32_a4 : f32 = s.a[0];
  var i32_a5 : f32 = s.a[0];
  var i32_b1 : f32 = s.b[min(2147483646u, (arrayLength(&(s.b)) - 1u))];
  var i32_b2 : f32 = s.b[min(1u, (arrayLength(&(s.b)) - 1u))];
  var i32_b3 : f32 = s.b[min(0u, (arrayLength(&(s.b)) - 1u))];
  var i32_b4 : f32 = s.b[min(0u, (arrayLength(&(s.b)) - 1u))];
  var i32_b5 : f32 = s.b[min(0u, (arrayLength(&(s.b)) - 1u))];
  var u32_a1 : f32 = s.a[0u];
  var u32_a2 : f32 = s.a[1u];
  var u32_a3 : f32 = s.a[2147483646u];
  var u32_a4 : f32 = s.a[2147483646u];
  var u32_a5 : f32 = s.a[2147483646u];
  var u32_a6 : f32 = s.a[2147483646u];
  var u32_b1 : f32 = s.b[min(0u, (arrayLength(&(s.b)) - 1u))];
  var u32_b2 : f32 = s.b[min(1u, (arrayLength(&(s.b)) - 1u))];
  var u32_b3 : f32 = s.b[min(2147483646u, (arrayLength(&(s.b)) - 1u))];
  var u32_b4 : f32 = s.b[min(2147483647u, (arrayLength(&(s.b)) - 1u))];
  var u32_b5 : f32 = s.b[min(2147483648u, (arrayLength(&(s.b)) - 1u))];
  var u32_b6 : f32 = s.b[min(4294967295u, (arrayLength(&(s.b)) - 1u))];
}
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Vector_Idx_Scalar) {
    auto* src = R"(
var<private> a : vec3<f32>;

fn f() {
  var b : f32 = a[1];
}
)";

    auto* expect = R"(
var<private> a : vec3<f32>;

fn f() {
  var b : f32 = a[1];
}
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Vector_Idx_Scalar_OutOfOrder) {
    auto* src = R"(
fn f() {
  var b : f32 = a[1];
}

var<private> a : vec3<f32>;
)";

    auto* expect = R"(
fn f() {
  var b : f32 = a[1];
}

var<private> a : vec3<f32>;
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Vector_Idx_Expr) {
    auto* src = R"(
var<private> a : vec3<f32>;

var<private> c : i32;

fn f() {
  var b : f32 = a[c + 2 - 3];
}
)";

    auto* expect = R"(
var<private> a : vec3<f32>;

var<private> c : i32;

fn f() {
  var b : f32 = a[min(u32(((c + 2) - 3)), 2u)];
}
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Vector_Idx_Expr_OutOfOrder) {
    auto* src = R"(
fn f() {
  var b : f32 = a[c + 2 - 3];
}

var<private> c : i32;

var<private> a : vec3<f32>;
)";

    auto* expect = R"(
fn f() {
  var b : f32 = a[min(u32(((c + 2) - 3)), 2u)];
}

var<private> c : i32;

var<private> a : vec3<f32>;
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Vector_Swizzle_Idx_Scalar) {
    auto* src = R"(
var<private> a : vec3<f32>;

fn f() {
  var b : f32 = a.xy[2];
}
)";

    auto* expect = R"(
var<private> a : vec3<f32>;

fn f() {
  var b : f32 = a.xy[1];
}
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Vector_Swizzle_Idx_Scalar_OutOfOrder) {
    auto* src = R"(
fn f() {
  var b : f32 = a.xy[2];
}

var<private> a : vec3<f32>;
)";

    auto* expect = R"(
fn f() {
  var b : f32 = a.xy[1];
}

var<private> a : vec3<f32>;
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Vector_Swizzle_Idx_Var) {
    auto* src = R"(
var<private> a : vec3<f32>;

var<private> c : i32;

fn f() {
  var b : f32 = a.xy[c];
}
)";

    auto* expect = R"(
var<private> a : vec3<f32>;

var<private> c : i32;

fn f() {
  var b : f32 = a.xy[min(u32(c), 1u)];
}
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Vector_Swizzle_Idx_Var_OutOfOrder) {
    auto* src = R"(
fn f() {
  var b : f32 = a.xy[c];
}

var<private> c : i32;

var<private> a : vec3<f32>;
)";

    auto* expect = R"(
fn f() {
  var b : f32 = a.xy[min(u32(c), 1u)];
}

var<private> c : i32;

var<private> a : vec3<f32>;
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Vector_Swizzle_Idx_Expr) {
    auto* src = R"(
var<private> a : vec3<f32>;

var<private> c : i32;

fn f() {
  var b : f32 = a.xy[c + 2 - 3];
}
)";

    auto* expect = R"(
var<private> a : vec3<f32>;

var<private> c : i32;

fn f() {
  var b : f32 = a.xy[min(u32(((c + 2) - 3)), 1u)];
}
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Vector_Swizzle_Idx_Expr_OutOfOrder) {
    auto* src = R"(
fn f() {
  var b : f32 = a.xy[c + 2 - 3];
}

var<private> c : i32;

var<private> a : vec3<f32>;
)";

    auto* expect = R"(
fn f() {
  var b : f32 = a.xy[min(u32(((c + 2) - 3)), 1u)];
}

var<private> c : i32;

var<private> a : vec3<f32>;
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Vector_Idx_Negative) {
    auto* src = R"(
var<private> a : vec3<f32>;

fn f() {
  var b : f32 = a[-1];
}
)";

    auto* expect = R"(
var<private> a : vec3<f32>;

fn f() {
  var b : f32 = a[0];
}
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Vector_Idx_Negative_OutOfOrder) {
    auto* src = R"(
fn f() {
  var b : f32 = a[-1];
}

var<private> a : vec3<f32>;
)";

    auto* expect = R"(
fn f() {
  var b : f32 = a[0];
}

var<private> a : vec3<f32>;
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Vector_Idx_OutOfBounds) {
    auto* src = R"(
var<private> a : vec3<f32>;

fn f() {
  var b : f32 = a[3];
}
)";

    auto* expect = R"(
var<private> a : vec3<f32>;

fn f() {
  var b : f32 = a[2];
}
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Vector_Idx_OutOfBounds_OutOfOrder) {
    auto* src = R"(
fn f() {
  var b : f32 = a[3];
}

var<private> a : vec3<f32>;
)";

    auto* expect = R"(
fn f() {
  var b : f32 = a[2];
}

var<private> a : vec3<f32>;
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Matrix_Idx_Scalar) {
    auto* src = R"(
var<private> a : mat3x2<f32>;

fn f() {
  var b : f32 = a[2][1];
}
)";

    auto* expect = R"(
var<private> a : mat3x2<f32>;

fn f() {
  var b : f32 = a[2][1];
}
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Matrix_Idx_Scalar_OutOfOrder) {
    auto* src = R"(
fn f() {
  var b : f32 = a[2][1];
}

var<private> a : mat3x2<f32>;
)";

    auto* expect = R"(
fn f() {
  var b : f32 = a[2][1];
}

var<private> a : mat3x2<f32>;
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Matrix_Idx_Expr_Column) {
    auto* src = R"(
var<private> a : mat3x2<f32>;

var<private> c : i32;

fn f() {
  var b : f32 = a[c + 2 - 3][1];
}
)";

    auto* expect = R"(
var<private> a : mat3x2<f32>;

var<private> c : i32;

fn f() {
  var b : f32 = a[min(u32(((c + 2) - 3)), 2u)][1];
}
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Matrix_Idx_Expr_Column_OutOfOrder) {
    auto* src = R"(
fn f() {
  var b : f32 = a[c + 2 - 3][1];
}

var<private> c : i32;

var<private> a : mat3x2<f32>;
)";

    auto* expect = R"(
fn f() {
  var b : f32 = a[min(u32(((c + 2) - 3)), 2u)][1];
}

var<private> c : i32;

var<private> a : mat3x2<f32>;
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Matrix_Idx_Expr_Row) {
    auto* src = R"(
var<private> a : mat3x2<f32>;

var<private> c : i32;

fn f() {
  var b : f32 = a[1][c + 2 - 3];
}
)";

    auto* expect = R"(
var<private> a : mat3x2<f32>;

var<private> c : i32;

fn f() {
  var b : f32 = a[1][min(u32(((c + 2) - 3)), 1u)];
}
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Matrix_Idx_Expr_Row_OutOfOrder) {
    auto* src = R"(
fn f() {
  var b : f32 = a[1][c + 2 - 3];
}

var<private> c : i32;

var<private> a : mat3x2<f32>;
)";

    auto* expect = R"(
fn f() {
  var b : f32 = a[1][min(u32(((c + 2) - 3)), 1u)];
}

var<private> c : i32;

var<private> a : mat3x2<f32>;
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Matrix_Idx_Negative_Column) {
    auto* src = R"(
var<private> a : mat3x2<f32>;

fn f() {
  var b : f32 = a[-1][1];
}
)";

    auto* expect = R"(
var<private> a : mat3x2<f32>;

fn f() {
  var b : f32 = a[0][1];
}
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Matrix_Idx_Negative_Column_OutOfOrder) {
    auto* src = R"(
fn f() {
  var b : f32 = a[-1][1];
}

var<private> a : mat3x2<f32>;
)";

    auto* expect = R"(
fn f() {
  var b : f32 = a[0][1];
}

var<private> a : mat3x2<f32>;
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Matrix_Idx_Negative_Row) {
    auto* src = R"(
var<private> a : mat3x2<f32>;

fn f() {
  var b : f32 = a[2][-1];
}
)";

    auto* expect = R"(
var<private> a : mat3x2<f32>;

fn f() {
  var b : f32 = a[2][0];
}
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Matrix_Idx_Negative_Row_OutOfOrder) {
    auto* src = R"(
fn f() {
  var b : f32 = a[2][-1];
}

var<private> a : mat3x2<f32>;
)";

    auto* expect = R"(
fn f() {
  var b : f32 = a[2][0];
}

var<private> a : mat3x2<f32>;
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Matrix_Idx_OutOfBounds_Column) {
    auto* src = R"(
var<private> a : mat3x2<f32>;

fn f() {
  var b : f32 = a[5][1];
}
)";

    auto* expect = R"(
var<private> a : mat3x2<f32>;

fn f() {
  var b : f32 = a[2][1];
}
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Matrix_Idx_OutOfBounds_Column_OutOfOrder) {
    auto* src = R"(
fn f() {
  var b : f32 = a[5][1];
}

var<private> a : mat3x2<f32>;
)";

    auto* expect = R"(
fn f() {
  var b : f32 = a[2][1];
}

var<private> a : mat3x2<f32>;
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Matrix_Idx_OutOfBounds_Row) {
    auto* src = R"(
var<private> a : mat3x2<f32>;

fn f() {
  var b : f32 = a[2][5];
}
)";

    auto* expect = R"(
var<private> a : mat3x2<f32>;

fn f() {
  var b : f32 = a[2][1];
}
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Matrix_Idx_OutOfBounds_Row_OutOfOrder) {
    auto* src = R"(
fn f() {
  var b : f32 = a[2][5];
}

var<private> a : mat3x2<f32>;
)";

    auto* expect = R"(
fn f() {
  var b : f32 = a[2][1];
}

var<private> a : mat3x2<f32>;
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

// TODO(dsinclair): Implement when constant_id exists
TEST_F(RobustnessTest, DISABLED_Vector_Constant_Id_Clamps) {
    // @id(1300) override idx : i32;
    // var a : vec3<f32>
    // var b : f32 = a[idx]
    //
    // ->var b : f32 = a[min(u32(idx), 2)]
}

// TODO(dsinclair): Implement when constant_id exists
TEST_F(RobustnessTest, DISABLED_Array_Constant_Id_Clamps) {
    // @id(1300) override idx : i32;
    // var a : array<f32, 4>
    // var b : f32 = a[idx]
    //
    // -> var b : f32 = a[min(u32(idx), 3)]
}

// TODO(dsinclair): Implement when constant_id exists
TEST_F(RobustnessTest, DISABLED_Matrix_Column_Constant_Id_Clamps) {
    // @id(1300) override idx : i32;
    // var a : mat3x2<f32>
    // var b : f32 = a[idx][1]
    //
    // -> var b : f32 = a[min(u32(idx), 2)][1]
}

// TODO(dsinclair): Implement when constant_id exists
TEST_F(RobustnessTest, DISABLED_Matrix_Row_Constant_Id_Clamps) {
    // @id(1300) override idx : i32;
    // var a : mat3x2<f32>
    // var b : f32 = a[1][idx]
    //
    // -> var b : f32 = a[1][min(u32(idx), 0, 1)]
}

TEST_F(RobustnessTest, RuntimeArray_Clamps) {
    auto* src = R"(
struct S {
  a : f32,
  b : array<f32>,
};
@group(0) @binding(0) var<storage, read> s : S;

fn f() {
  var d : f32 = s.b[25];
}
)";

    auto* expect = R"(
struct S {
  a : f32,
  b : array<f32>,
}

@group(0) @binding(0) var<storage, read> s : S;

fn f() {
  var d : f32 = s.b[min(25u, (arrayLength(&(s.b)) - 1u))];
}
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, RuntimeArray_Clamps_OutOfOrder) {
    auto* src = R"(
fn f() {
  var d : f32 = s.b[25];
}

@group(0) @binding(0) var<storage, read> s : S;

struct S {
  a : f32,
  b : array<f32>,
};
)";

    auto* expect = R"(
fn f() {
  var d : f32 = s.b[min(25u, (arrayLength(&(s.b)) - 1u))];
}

@group(0) @binding(0) var<storage, read> s : S;

struct S {
  a : f32,
  b : array<f32>,
}
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

// Clamp textureLoad() coord, array_index and level values
TEST_F(RobustnessTest, TextureLoad_Clamp) {
    auto* src = R"(
@group(0) @binding(0) var tex_1d : texture_1d<f32>;
@group(0) @binding(0) var tex_2d : texture_2d<f32>;
@group(0) @binding(0) var tex_2d_arr : texture_2d_array<f32>;
@group(0) @binding(0) var tex_3d : texture_3d<f32>;
@group(0) @binding(0) var tex_ms_2d : texture_multisampled_2d<f32>;
@group(0) @binding(0) var tex_depth_2d : texture_depth_2d;
@group(0) @binding(0) var tex_depth_2d_arr : texture_depth_2d_array;
@group(0) @binding(0) var tex_external : texture_external;

fn f() {
  var array_idx : i32;
  var level_idx : i32;
  var sample_idx : i32;

  textureLoad(tex_1d, 1, level_idx);
  textureLoad(tex_2d, vec2<i32>(1, 2), level_idx);
  textureLoad(tex_2d_arr, vec2<i32>(1, 2), array_idx, level_idx);
  textureLoad(tex_3d, vec3<i32>(1, 2, 3), level_idx);
  textureLoad(tex_ms_2d, vec2<i32>(1, 2), sample_idx);
  textureLoad(tex_depth_2d, vec2<i32>(1, 2), level_idx);
  textureLoad(tex_depth_2d_arr, vec2<i32>(1, 2), array_idx, level_idx);
  textureLoad(tex_external, vec2<i32>(1, 2));
}
)";

    auto* expect =
        R"(
@group(0) @binding(0) var tex_1d : texture_1d<f32>;

@group(0) @binding(0) var tex_2d : texture_2d<f32>;

@group(0) @binding(0) var tex_2d_arr : texture_2d_array<f32>;

@group(0) @binding(0) var tex_3d : texture_3d<f32>;

@group(0) @binding(0) var tex_ms_2d : texture_multisampled_2d<f32>;

@group(0) @binding(0) var tex_depth_2d : texture_depth_2d;

@group(0) @binding(0) var tex_depth_2d_arr : texture_depth_2d_array;

@group(0) @binding(0) var tex_external : texture_external;

fn f() {
  var array_idx : i32;
  var level_idx : i32;
  var sample_idx : i32;
  textureLoad(tex_1d, clamp(1, i32(), (textureDimensions(tex_1d, clamp(level_idx, 0, (textureNumLevels(tex_1d) - 1))) - i32(1))), clamp(level_idx, 0, (textureNumLevels(tex_1d) - 1)));
  textureLoad(tex_2d, clamp(vec2<i32>(1, 2), vec2<i32>(), (textureDimensions(tex_2d, clamp(level_idx, 0, (textureNumLevels(tex_2d) - 1))) - vec2<i32>(1))), clamp(level_idx, 0, (textureNumLevels(tex_2d) - 1)));
  textureLoad(tex_2d_arr, clamp(vec2<i32>(1, 2), vec2<i32>(), (textureDimensions(tex_2d_arr, clamp(level_idx, 0, (textureNumLevels(tex_2d_arr) - 1))) - vec2<i32>(1))), clamp(array_idx, 0, (textureNumLayers(tex_2d_arr) - 1)), clamp(level_idx, 0, (textureNumLevels(tex_2d_arr) - 1)));
  textureLoad(tex_3d, clamp(vec3<i32>(1, 2, 3), vec3<i32>(), (textureDimensions(tex_3d, clamp(level_idx, 0, (textureNumLevels(tex_3d) - 1))) - vec3<i32>(1))), clamp(level_idx, 0, (textureNumLevels(tex_3d) - 1)));
  textureLoad(tex_ms_2d, clamp(vec2<i32>(1, 2), vec2<i32>(), (textureDimensions(tex_ms_2d) - vec2<i32>(1))), sample_idx);
  textureLoad(tex_depth_2d, clamp(vec2<i32>(1, 2), vec2<i32>(), (textureDimensions(tex_depth_2d, clamp(level_idx, 0, (textureNumLevels(tex_depth_2d) - 1))) - vec2<i32>(1))), clamp(level_idx, 0, (textureNumLevels(tex_depth_2d) - 1)));
  textureLoad(tex_depth_2d_arr, clamp(vec2<i32>(1, 2), vec2<i32>(), (textureDimensions(tex_depth_2d_arr, clamp(level_idx, 0, (textureNumLevels(tex_depth_2d_arr) - 1))) - vec2<i32>(1))), clamp(array_idx, 0, (textureNumLayers(tex_depth_2d_arr) - 1)), clamp(level_idx, 0, (textureNumLevels(tex_depth_2d_arr) - 1)));
  textureLoad(tex_external, clamp(vec2<i32>(1, 2), vec2<i32>(), (textureDimensions(tex_external) - vec2<i32>(1))));
}
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

// Clamp textureLoad() coord, array_index and level values
TEST_F(RobustnessTest, TextureLoad_Clamp_OutOfOrder) {
    auto* src = R"(
fn f() {
  var array_idx : i32;
  var level_idx : i32;
  var sample_idx : i32;

  textureLoad(tex_1d, 1, level_idx);
  textureLoad(tex_2d, vec2<i32>(1, 2), level_idx);
  textureLoad(tex_2d_arr, vec2<i32>(1, 2), array_idx, level_idx);
  textureLoad(tex_3d, vec3<i32>(1, 2, 3), level_idx);
  textureLoad(tex_ms_2d, vec2<i32>(1, 2), sample_idx);
  textureLoad(tex_depth_2d, vec2<i32>(1, 2), level_idx);
  textureLoad(tex_depth_2d_arr, vec2<i32>(1, 2), array_idx, level_idx);
  textureLoad(tex_external, vec2<i32>(1, 2));
}

@group(0) @binding(0) var tex_1d : texture_1d<f32>;
@group(0) @binding(0) var tex_2d : texture_2d<f32>;
@group(0) @binding(0) var tex_2d_arr : texture_2d_array<f32>;
@group(0) @binding(0) var tex_3d : texture_3d<f32>;
@group(0) @binding(0) var tex_ms_2d : texture_multisampled_2d<f32>;
@group(0) @binding(0) var tex_depth_2d : texture_depth_2d;
@group(0) @binding(0) var tex_depth_2d_arr : texture_depth_2d_array;
@group(0) @binding(0) var tex_external : texture_external;
)";

    auto* expect =
        R"(
fn f() {
  var array_idx : i32;
  var level_idx : i32;
  var sample_idx : i32;
  textureLoad(tex_1d, clamp(1, i32(), (textureDimensions(tex_1d, clamp(level_idx, 0, (textureNumLevels(tex_1d) - 1))) - i32(1))), clamp(level_idx, 0, (textureNumLevels(tex_1d) - 1)));
  textureLoad(tex_2d, clamp(vec2<i32>(1, 2), vec2<i32>(), (textureDimensions(tex_2d, clamp(level_idx, 0, (textureNumLevels(tex_2d) - 1))) - vec2<i32>(1))), clamp(level_idx, 0, (textureNumLevels(tex_2d) - 1)));
  textureLoad(tex_2d_arr, clamp(vec2<i32>(1, 2), vec2<i32>(), (textureDimensions(tex_2d_arr, clamp(level_idx, 0, (textureNumLevels(tex_2d_arr) - 1))) - vec2<i32>(1))), clamp(array_idx, 0, (textureNumLayers(tex_2d_arr) - 1)), clamp(level_idx, 0, (textureNumLevels(tex_2d_arr) - 1)));
  textureLoad(tex_3d, clamp(vec3<i32>(1, 2, 3), vec3<i32>(), (textureDimensions(tex_3d, clamp(level_idx, 0, (textureNumLevels(tex_3d) - 1))) - vec3<i32>(1))), clamp(level_idx, 0, (textureNumLevels(tex_3d) - 1)));
  textureLoad(tex_ms_2d, clamp(vec2<i32>(1, 2), vec2<i32>(), (textureDimensions(tex_ms_2d) - vec2<i32>(1))), sample_idx);
  textureLoad(tex_depth_2d, clamp(vec2<i32>(1, 2), vec2<i32>(), (textureDimensions(tex_depth_2d, clamp(level_idx, 0, (textureNumLevels(tex_depth_2d) - 1))) - vec2<i32>(1))), clamp(level_idx, 0, (textureNumLevels(tex_depth_2d) - 1)));
  textureLoad(tex_depth_2d_arr, clamp(vec2<i32>(1, 2), vec2<i32>(), (textureDimensions(tex_depth_2d_arr, clamp(level_idx, 0, (textureNumLevels(tex_depth_2d_arr) - 1))) - vec2<i32>(1))), clamp(array_idx, 0, (textureNumLayers(tex_depth_2d_arr) - 1)), clamp(level_idx, 0, (textureNumLevels(tex_depth_2d_arr) - 1)));
  textureLoad(tex_external, clamp(vec2<i32>(1, 2), vec2<i32>(), (textureDimensions(tex_external) - vec2<i32>(1))));
}

@group(0) @binding(0) var tex_1d : texture_1d<f32>;

@group(0) @binding(0) var tex_2d : texture_2d<f32>;

@group(0) @binding(0) var tex_2d_arr : texture_2d_array<f32>;

@group(0) @binding(0) var tex_3d : texture_3d<f32>;

@group(0) @binding(0) var tex_ms_2d : texture_multisampled_2d<f32>;

@group(0) @binding(0) var tex_depth_2d : texture_depth_2d;

@group(0) @binding(0) var tex_depth_2d_arr : texture_depth_2d_array;

@group(0) @binding(0) var tex_external : texture_external;
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

// Clamp textureStore() coord, array_index and level values
TEST_F(RobustnessTest, TextureStore_Clamp) {
    auto* src = R"(
@group(0) @binding(0) var tex1d : texture_storage_1d<rgba8sint, write>;

@group(0) @binding(1) var tex2d : texture_storage_2d<rgba8sint, write>;

@group(0) @binding(2) var tex2d_arr : texture_storage_2d_array<rgba8sint, write>;

@group(0) @binding(3) var tex3d : texture_storage_3d<rgba8sint, write>;

fn f() {
  textureStore(tex1d, 10, vec4<i32>());
  textureStore(tex2d, vec2<i32>(10, 20), vec4<i32>());
  textureStore(tex2d_arr, vec2<i32>(10, 20), 50, vec4<i32>());
  textureStore(tex3d, vec3<i32>(10, 20, 30), vec4<i32>());
}
)";

    auto* expect = R"(
@group(0) @binding(0) var tex1d : texture_storage_1d<rgba8sint, write>;

@group(0) @binding(1) var tex2d : texture_storage_2d<rgba8sint, write>;

@group(0) @binding(2) var tex2d_arr : texture_storage_2d_array<rgba8sint, write>;

@group(0) @binding(3) var tex3d : texture_storage_3d<rgba8sint, write>;

fn f() {
  textureStore(tex1d, clamp(10, i32(), (textureDimensions(tex1d) - i32(1))), vec4<i32>());
  textureStore(tex2d, clamp(vec2<i32>(10, 20), vec2<i32>(), (textureDimensions(tex2d) - vec2<i32>(1))), vec4<i32>());
  textureStore(tex2d_arr, clamp(vec2<i32>(10, 20), vec2<i32>(), (textureDimensions(tex2d_arr) - vec2<i32>(1))), clamp(50, 0, (textureNumLayers(tex2d_arr) - 1)), vec4<i32>());
  textureStore(tex3d, clamp(vec3<i32>(10, 20, 30), vec3<i32>(), (textureDimensions(tex3d) - vec3<i32>(1))), vec4<i32>());
}
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

// Clamp textureStore() coord, array_index and level values
TEST_F(RobustnessTest, TextureStore_Clamp_OutOfOrder) {
    auto* src = R"(
fn f() {
  textureStore(tex1d, 10, vec4<i32>());
  textureStore(tex2d, vec2<i32>(10, 20), vec4<i32>());
  textureStore(tex2d_arr, vec2<i32>(10, 20), 50, vec4<i32>());
  textureStore(tex3d, vec3<i32>(10, 20, 30), vec4<i32>());
}

@group(0) @binding(0) var tex1d : texture_storage_1d<rgba8sint, write>;

@group(0) @binding(1) var tex2d : texture_storage_2d<rgba8sint, write>;

@group(0) @binding(2) var tex2d_arr : texture_storage_2d_array<rgba8sint, write>;

@group(0) @binding(3) var tex3d : texture_storage_3d<rgba8sint, write>;

)";

    auto* expect = R"(
fn f() {
  textureStore(tex1d, clamp(10, i32(), (textureDimensions(tex1d) - i32(1))), vec4<i32>());
  textureStore(tex2d, clamp(vec2<i32>(10, 20), vec2<i32>(), (textureDimensions(tex2d) - vec2<i32>(1))), vec4<i32>());
  textureStore(tex2d_arr, clamp(vec2<i32>(10, 20), vec2<i32>(), (textureDimensions(tex2d_arr) - vec2<i32>(1))), clamp(50, 0, (textureNumLayers(tex2d_arr) - 1)), vec4<i32>());
  textureStore(tex3d, clamp(vec3<i32>(10, 20, 30), vec3<i32>(), (textureDimensions(tex3d) - vec3<i32>(1))), vec4<i32>());
}

@group(0) @binding(0) var tex1d : texture_storage_1d<rgba8sint, write>;

@group(0) @binding(1) var tex2d : texture_storage_2d<rgba8sint, write>;

@group(0) @binding(2) var tex2d_arr : texture_storage_2d_array<rgba8sint, write>;

@group(0) @binding(3) var tex3d : texture_storage_3d<rgba8sint, write>;
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

// TODO(dsinclair): Test for scoped variables when shadowing is implemented
TEST_F(RobustnessTest, DISABLED_Shadowed_Variable) {
    // var a : array<f32, 3>;
    // var i : u32;
    // {
    //    var a : array<f32, 5>;
    //    var b : f32 = a[i];
    // }
    // var c : f32 = a[i];
    //
    // -> var b : f32 = a[min(u32(i), 4)];
    //    var c : f32 = a[min(u32(i), 2)];
    FAIL();
}

// Check that existing use of min() and arrayLength() do not get renamed.
TEST_F(RobustnessTest, DontRenameSymbols) {
    auto* src = R"(
struct S {
  a : f32,
  b : array<f32>,
};

@group(0) @binding(0) var<storage, read> s : S;

let c : u32 = 1u;

fn f() {
  let b : f32 = s.b[c];
  let x : i32 = min(1, 2);
  let y : u32 = arrayLength(&s.b);
}
)";

    auto* expect = R"(
struct S {
  a : f32,
  b : array<f32>,
}

@group(0) @binding(0) var<storage, read> s : S;

let c : u32 = 1u;

fn f() {
  let b : f32 = s.b[min(1u, (arrayLength(&(s.b)) - 1u))];
  let x : i32 = min(1, 2);
  let y : u32 = arrayLength(&(s.b));
}
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

const char* kOmitSourceShader = R"(
struct S {
  a : array<f32, 4>,
  b : array<f32>,
};
@group(0) @binding(0) var<storage, read> s : S;

type UArr = array<vec4<f32>, 4>;
struct U {
  a : UArr,
};
@group(1) @binding(0) var<uniform> u : U;

fn f() {
  // Signed
  var i32_sa1 : f32 = s.a[4];
  var i32_sa2 : f32 = s.a[1];
  var i32_sa3 : f32 = s.a[0];
  var i32_sa4 : f32 = s.a[-1];
  var i32_sa5 : f32 = s.a[-4];

  var i32_sb1 : f32 = s.b[4];
  var i32_sb2 : f32 = s.b[1];
  var i32_sb3 : f32 = s.b[0];
  var i32_sb4 : f32 = s.b[-1];
  var i32_sb5 : f32 = s.b[-4];

  var i32_ua1 : f32 = u.a[4].x;
  var i32_ua2 : f32 = u.a[1].x;
  var i32_ua3 : f32 = u.a[0].x;
  var i32_ua4 : f32 = u.a[-1].x;
  var i32_ua5 : f32 = u.a[-4].x;

  // Unsigned
  var u32_sa1 : f32 = s.a[0u];
  var u32_sa2 : f32 = s.a[1u];
  var u32_sa3 : f32 = s.a[3u];
  var u32_sa4 : f32 = s.a[4u];
  var u32_sa5 : f32 = s.a[10u];
  var u32_sa6 : f32 = s.a[100u];

  var u32_sb1 : f32 = s.b[0u];
  var u32_sb2 : f32 = s.b[1u];
  var u32_sb3 : f32 = s.b[3u];
  var u32_sb4 : f32 = s.b[4u];
  var u32_sb5 : f32 = s.b[10u];
  var u32_sb6 : f32 = s.b[100u];

  var u32_ua1 : f32 = u.a[0u].x;
  var u32_ua2 : f32 = u.a[1u].x;
  var u32_ua3 : f32 = u.a[3u].x;
  var u32_ua4 : f32 = u.a[4u].x;
  var u32_ua5 : f32 = u.a[10u].x;
  var u32_ua6 : f32 = u.a[100u].x;
}
)";

TEST_F(RobustnessTest, OmitNone) {
    auto* expect = R"(
struct S {
  a : array<f32, 4>,
  b : array<f32>,
}

@group(0) @binding(0) var<storage, read> s : S;

type UArr = array<vec4<f32>, 4>;

struct U {
  a : UArr,
}

@group(1) @binding(0) var<uniform> u : U;

fn f() {
  var i32_sa1 : f32 = s.a[3];
  var i32_sa2 : f32 = s.a[1];
  var i32_sa3 : f32 = s.a[0];
  var i32_sa4 : f32 = s.a[0];
  var i32_sa5 : f32 = s.a[0];
  var i32_sb1 : f32 = s.b[min(4u, (arrayLength(&(s.b)) - 1u))];
  var i32_sb2 : f32 = s.b[min(1u, (arrayLength(&(s.b)) - 1u))];
  var i32_sb3 : f32 = s.b[min(0u, (arrayLength(&(s.b)) - 1u))];
  var i32_sb4 : f32 = s.b[min(0u, (arrayLength(&(s.b)) - 1u))];
  var i32_sb5 : f32 = s.b[min(0u, (arrayLength(&(s.b)) - 1u))];
  var i32_ua1 : f32 = u.a[3].x;
  var i32_ua2 : f32 = u.a[1].x;
  var i32_ua3 : f32 = u.a[0].x;
  var i32_ua4 : f32 = u.a[0].x;
  var i32_ua5 : f32 = u.a[0].x;
  var u32_sa1 : f32 = s.a[0u];
  var u32_sa2 : f32 = s.a[1u];
  var u32_sa3 : f32 = s.a[3u];
  var u32_sa4 : f32 = s.a[3u];
  var u32_sa5 : f32 = s.a[3u];
  var u32_sa6 : f32 = s.a[3u];
  var u32_sb1 : f32 = s.b[min(0u, (arrayLength(&(s.b)) - 1u))];
  var u32_sb2 : f32 = s.b[min(1u, (arrayLength(&(s.b)) - 1u))];
  var u32_sb3 : f32 = s.b[min(3u, (arrayLength(&(s.b)) - 1u))];
  var u32_sb4 : f32 = s.b[min(4u, (arrayLength(&(s.b)) - 1u))];
  var u32_sb5 : f32 = s.b[min(10u, (arrayLength(&(s.b)) - 1u))];
  var u32_sb6 : f32 = s.b[min(100u, (arrayLength(&(s.b)) - 1u))];
  var u32_ua1 : f32 = u.a[0u].x;
  var u32_ua2 : f32 = u.a[1u].x;
  var u32_ua3 : f32 = u.a[3u].x;
  var u32_ua4 : f32 = u.a[3u].x;
  var u32_ua5 : f32 = u.a[3u].x;
  var u32_ua6 : f32 = u.a[3u].x;
}
)";

    Robustness::Config cfg;
    DataMap data;
    data.Add<Robustness::Config>(cfg);

    auto got = Run<Robustness>(kOmitSourceShader, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, OmitStorage) {
    auto* expect = R"(
struct S {
  a : array<f32, 4>,
  b : array<f32>,
}

@group(0) @binding(0) var<storage, read> s : S;

type UArr = array<vec4<f32>, 4>;

struct U {
  a : UArr,
}

@group(1) @binding(0) var<uniform> u : U;

fn f() {
  var i32_sa1 : f32 = s.a[4];
  var i32_sa2 : f32 = s.a[1];
  var i32_sa3 : f32 = s.a[0];
  var i32_sa4 : f32 = s.a[-1];
  var i32_sa5 : f32 = s.a[-4];
  var i32_sb1 : f32 = s.b[4];
  var i32_sb2 : f32 = s.b[1];
  var i32_sb3 : f32 = s.b[0];
  var i32_sb4 : f32 = s.b[-1];
  var i32_sb5 : f32 = s.b[-4];
  var i32_ua1 : f32 = u.a[3].x;
  var i32_ua2 : f32 = u.a[1].x;
  var i32_ua3 : f32 = u.a[0].x;
  var i32_ua4 : f32 = u.a[0].x;
  var i32_ua5 : f32 = u.a[0].x;
  var u32_sa1 : f32 = s.a[0u];
  var u32_sa2 : f32 = s.a[1u];
  var u32_sa3 : f32 = s.a[3u];
  var u32_sa4 : f32 = s.a[4u];
  var u32_sa5 : f32 = s.a[10u];
  var u32_sa6 : f32 = s.a[100u];
  var u32_sb1 : f32 = s.b[0u];
  var u32_sb2 : f32 = s.b[1u];
  var u32_sb3 : f32 = s.b[3u];
  var u32_sb4 : f32 = s.b[4u];
  var u32_sb5 : f32 = s.b[10u];
  var u32_sb6 : f32 = s.b[100u];
  var u32_ua1 : f32 = u.a[0u].x;
  var u32_ua2 : f32 = u.a[1u].x;
  var u32_ua3 : f32 = u.a[3u].x;
  var u32_ua4 : f32 = u.a[3u].x;
  var u32_ua5 : f32 = u.a[3u].x;
  var u32_ua6 : f32 = u.a[3u].x;
}
)";

    Robustness::Config cfg;
    cfg.omitted_classes.insert(Robustness::StorageClass::kStorage);

    DataMap data;
    data.Add<Robustness::Config>(cfg);

    auto got = Run<Robustness>(kOmitSourceShader, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, OmitUniform) {
    auto* expect = R"(
struct S {
  a : array<f32, 4>,
  b : array<f32>,
}

@group(0) @binding(0) var<storage, read> s : S;

type UArr = array<vec4<f32>, 4>;

struct U {
  a : UArr,
}

@group(1) @binding(0) var<uniform> u : U;

fn f() {
  var i32_sa1 : f32 = s.a[3];
  var i32_sa2 : f32 = s.a[1];
  var i32_sa3 : f32 = s.a[0];
  var i32_sa4 : f32 = s.a[0];
  var i32_sa5 : f32 = s.a[0];
  var i32_sb1 : f32 = s.b[min(4u, (arrayLength(&(s.b)) - 1u))];
  var i32_sb2 : f32 = s.b[min(1u, (arrayLength(&(s.b)) - 1u))];
  var i32_sb3 : f32 = s.b[min(0u, (arrayLength(&(s.b)) - 1u))];
  var i32_sb4 : f32 = s.b[min(0u, (arrayLength(&(s.b)) - 1u))];
  var i32_sb5 : f32 = s.b[min(0u, (arrayLength(&(s.b)) - 1u))];
  var i32_ua1 : f32 = u.a[4].x;
  var i32_ua2 : f32 = u.a[1].x;
  var i32_ua3 : f32 = u.a[0].x;
  var i32_ua4 : f32 = u.a[-1].x;
  var i32_ua5 : f32 = u.a[-4].x;
  var u32_sa1 : f32 = s.a[0u];
  var u32_sa2 : f32 = s.a[1u];
  var u32_sa3 : f32 = s.a[3u];
  var u32_sa4 : f32 = s.a[3u];
  var u32_sa5 : f32 = s.a[3u];
  var u32_sa6 : f32 = s.a[3u];
  var u32_sb1 : f32 = s.b[min(0u, (arrayLength(&(s.b)) - 1u))];
  var u32_sb2 : f32 = s.b[min(1u, (arrayLength(&(s.b)) - 1u))];
  var u32_sb3 : f32 = s.b[min(3u, (arrayLength(&(s.b)) - 1u))];
  var u32_sb4 : f32 = s.b[min(4u, (arrayLength(&(s.b)) - 1u))];
  var u32_sb5 : f32 = s.b[min(10u, (arrayLength(&(s.b)) - 1u))];
  var u32_sb6 : f32 = s.b[min(100u, (arrayLength(&(s.b)) - 1u))];
  var u32_ua1 : f32 = u.a[0u].x;
  var u32_ua2 : f32 = u.a[1u].x;
  var u32_ua3 : f32 = u.a[3u].x;
  var u32_ua4 : f32 = u.a[4u].x;
  var u32_ua5 : f32 = u.a[10u].x;
  var u32_ua6 : f32 = u.a[100u].x;
}
)";

    Robustness::Config cfg;
    cfg.omitted_classes.insert(Robustness::StorageClass::kUniform);

    DataMap data;
    data.Add<Robustness::Config>(cfg);

    auto got = Run<Robustness>(kOmitSourceShader, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, OmitBoth) {
    auto* expect = R"(
struct S {
  a : array<f32, 4>,
  b : array<f32>,
}

@group(0) @binding(0) var<storage, read> s : S;

type UArr = array<vec4<f32>, 4>;

struct U {
  a : UArr,
}

@group(1) @binding(0) var<uniform> u : U;

fn f() {
  var i32_sa1 : f32 = s.a[4];
  var i32_sa2 : f32 = s.a[1];
  var i32_sa3 : f32 = s.a[0];
  var i32_sa4 : f32 = s.a[-1];
  var i32_sa5 : f32 = s.a[-4];
  var i32_sb1 : f32 = s.b[4];
  var i32_sb2 : f32 = s.b[1];
  var i32_sb3 : f32 = s.b[0];
  var i32_sb4 : f32 = s.b[-1];
  var i32_sb5 : f32 = s.b[-4];
  var i32_ua1 : f32 = u.a[4].x;
  var i32_ua2 : f32 = u.a[1].x;
  var i32_ua3 : f32 = u.a[0].x;
  var i32_ua4 : f32 = u.a[-1].x;
  var i32_ua5 : f32 = u.a[-4].x;
  var u32_sa1 : f32 = s.a[0u];
  var u32_sa2 : f32 = s.a[1u];
  var u32_sa3 : f32 = s.a[3u];
  var u32_sa4 : f32 = s.a[4u];
  var u32_sa5 : f32 = s.a[10u];
  var u32_sa6 : f32 = s.a[100u];
  var u32_sb1 : f32 = s.b[0u];
  var u32_sb2 : f32 = s.b[1u];
  var u32_sb3 : f32 = s.b[3u];
  var u32_sb4 : f32 = s.b[4u];
  var u32_sb5 : f32 = s.b[10u];
  var u32_sb6 : f32 = s.b[100u];
  var u32_ua1 : f32 = u.a[0u].x;
  var u32_ua2 : f32 = u.a[1u].x;
  var u32_ua3 : f32 = u.a[3u].x;
  var u32_ua4 : f32 = u.a[4u].x;
  var u32_ua5 : f32 = u.a[10u].x;
  var u32_ua6 : f32 = u.a[100u].x;
}
)";

    Robustness::Config cfg;
    cfg.omitted_classes.insert(Robustness::StorageClass::kStorage);
    cfg.omitted_classes.insert(Robustness::StorageClass::kUniform);

    DataMap data;
    data.Add<Robustness::Config>(cfg);

    auto got = Run<Robustness>(kOmitSourceShader, data);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::transform
