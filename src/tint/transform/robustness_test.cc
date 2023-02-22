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

TEST_F(RobustnessTest, Array_Let_Idx_Clamp) {
    auto* src = R"(
var<private> a : array<f32, 3>;

fn f() {
  let l : u32 = 1u;
  let b : f32 = a[l];
}
)";

    auto* expect = R"(
var<private> a : array<f32, 3>;

fn f() {
  let l : u32 = 1u;
  let b : f32 = a[min(l, 2u)];
}
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Array_Let_Idx_Clamp_OutOfOrder) {
    auto* src = R"(
fn f() {
  let c : u32 = 1u;
  let b : f32 = a[c];
}

var<private> a : array<f32, 3>;
)";

    auto* expect = R"(
fn f() {
  let c : u32 = 1u;
  let b : f32 = a[min(c, 2u)];
}

var<private> a : array<f32, 3>;
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Array_Const_Idx_Clamp) {
    auto* src = R"(
var<private> a : array<f32, 3>;

const c : u32 = 1u;

fn f() {
  let b : f32 = a[c];
}
)";

    auto* expect = R"(
var<private> a : array<f32, 3>;

const c : u32 = 1u;

fn f() {
  let b : f32 = a[c];
}
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Array_Const_Idx_Clamp_OutOfOrder) {
    auto* src = R"(
fn f() {
  let b : f32 = a[c];
}

const c : u32 = 1u;

var<private> a : array<f32, 3>;
)";

    auto* expect = R"(
fn f() {
  let b : f32 = a[c];
}

const c : u32 = 1u;

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
  var b : f32 = a[1i];
}
)";

    auto* expect = R"(
var<private> a : array<f32, 3>;

fn f() {
  var b : f32 = a[1i];
}
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Array_Idx_Scalar_OutOfOrder) {
    auto* src = R"(
fn f() {
  var b : f32 = a[1i];
}

var<private> a : array<f32, 3>;
)";

    auto* expect = R"(
fn f() {
  var b : f32 = a[1i];
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

TEST_F(RobustnessTest, Vector_Idx_Scalar) {
    auto* src = R"(
var<private> a : vec3<f32>;

fn f() {
  var b : f32 = a[1i];
}
)";

    auto* expect = R"(
var<private> a : vec3<f32>;

fn f() {
  var b : f32 = a[1i];
}
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Vector_Idx_Scalar_OutOfOrder) {
    auto* src = R"(
fn f() {
  var b : f32 = a[1i];
}

var<private> a : vec3<f32>;
)";

    auto* expect = R"(
fn f() {
  var b : f32 = a[1i];
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

TEST_F(RobustnessTest, Matrix_Idx_Scalar) {
    auto* src = R"(
var<private> a : mat3x2<f32>;

fn f() {
  var b : f32 = a[2i][1i];
}
)";

    auto* expect = R"(
var<private> a : mat3x2<f32>;

fn f() {
  var b : f32 = a[2i][1i];
}
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Matrix_Idx_Scalar_OutOfOrder) {
    auto* src = R"(
fn f() {
  var b : f32 = a[2i][1i];
}

var<private> a : mat3x2<f32>;
)";

    auto* expect = R"(
fn f() {
  var b : f32 = a[2i][1i];
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

TEST_F(RobustnessTest, Vector_Constant_Id_Clamps) {
    auto* src = R"(
@id(1300) override idx : i32;
fn f() {
  var a : vec3<f32>;
  var b : f32 = a[idx];
}
)";

    auto* expect = R"(
@id(1300) override idx : i32;

fn f() {
  var a : vec3<f32>;
  var b : f32 = a[min(u32(idx), 2u)];
}
)";

    auto got = Run<Robustness>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Array_Constant_Id_Clamps) {
    auto* src = R"(
@id(1300) override idx : i32;
fn f() {
  var a : array<f32, 4>;
  var b : f32 = a[idx];
}
)";

    auto* expect = R"(
@id(1300) override idx : i32;

fn f() {
  var a : array<f32, 4>;
  var b : f32 = a[min(u32(idx), 3u)];
}
)";

    auto got = Run<Robustness>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Matrix_Column_Constant_Id_Clamps) {
    auto* src = R"(
@id(1300) override idx : i32;
fn f() {
  var a : mat3x2<f32>;
  var b : f32 = a[idx][1];
}
)";

    auto* expect = R"(
@id(1300) override idx : i32;

fn f() {
  var a : mat3x2<f32>;
  var b : f32 = a[min(u32(idx), 2u)][1];
}
)";

    auto got = Run<Robustness>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Matrix_Row_Constant_Id_Clamps) {
    auto* src = R"(
@id(1300) override idx : i32;
fn f() {
  var a : mat3x2<f32>;
  var b : f32 = a[1][idx];
}
)";

    auto* expect = R"(
@id(1300) override idx : i32;

fn f() {
  var a : mat3x2<f32>;
  var b : f32 = a[1][min(u32(idx), 1u)];
}
)";

    auto got = Run<Robustness>(src);
    EXPECT_EQ(expect, str(got));
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
  var d : f32 = s.b[min(u32(25), (arrayLength(&(s.b)) - 1u))];
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
  var d : f32 = s.b[min(u32(25), (arrayLength(&(s.b)) - 1u))];
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

fn idx_signed() {
  var array_idx : i32;
  var level_idx : i32;
  var sample_idx : i32;

  _ = textureLoad(tex_1d, 1i, level_idx);
  _ = textureLoad(tex_2d, vec2<i32>(1, 2), level_idx);
  _ = textureLoad(tex_2d_arr, vec2<i32>(1, 2), array_idx, level_idx);
  _ = textureLoad(tex_3d, vec3<i32>(1, 2, 3), level_idx);
  _ = textureLoad(tex_ms_2d, vec2<i32>(1, 2), sample_idx);
  _ = textureLoad(tex_depth_2d, vec2<i32>(1, 2), level_idx);
  _ = textureLoad(tex_depth_2d_arr, vec2<i32>(1, 2), array_idx, level_idx);
  _ = textureLoad(tex_external, vec2<i32>(1, 2));
}

fn idx_unsigned() {
  var array_idx : u32;
  var level_idx : u32;
  var sample_idx : u32;

  _ = textureLoad(tex_1d, 1u, level_idx);
  _ = textureLoad(tex_2d, vec2<u32>(1, 2), level_idx);
  _ = textureLoad(tex_2d_arr, vec2<u32>(1, 2), array_idx, level_idx);
  _ = textureLoad(tex_3d, vec3<u32>(1, 2, 3), level_idx);
  _ = textureLoad(tex_ms_2d, vec2<u32>(1, 2), sample_idx);
  _ = textureLoad(tex_depth_2d, vec2<u32>(1, 2), level_idx);
  _ = textureLoad(tex_depth_2d_arr, vec2<u32>(1, 2), array_idx, level_idx);
  _ = textureLoad(tex_external, vec2<u32>(1, 2));
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

fn idx_signed() {
  var array_idx : i32;
  var level_idx : i32;
  var sample_idx : i32;
  _ = textureLoad(tex_1d, clamp(1i, 0, i32((u32(textureDimensions(tex_1d, clamp(level_idx, 0, i32((u32(textureNumLevels(tex_1d)) - 1))))) - 1))), clamp(level_idx, 0, i32((u32(textureNumLevels(tex_1d)) - 1))));
  _ = textureLoad(tex_2d, clamp(vec2<i32>(1, 2), vec2(0), vec2<i32>((vec2<u32>(textureDimensions(tex_2d, clamp(level_idx, 0, i32((u32(textureNumLevels(tex_2d)) - 1))))) - vec2(1)))), clamp(level_idx, 0, i32((u32(textureNumLevels(tex_2d)) - 1))));
  _ = textureLoad(tex_2d_arr, clamp(vec2<i32>(1, 2), vec2(0), vec2<i32>((vec2<u32>(textureDimensions(tex_2d_arr, clamp(level_idx, 0, i32((u32(textureNumLevels(tex_2d_arr)) - 1))))) - vec2(1)))), clamp(array_idx, 0, i32((u32(textureNumLayers(tex_2d_arr)) - 1))), clamp(level_idx, 0, i32((u32(textureNumLevels(tex_2d_arr)) - 1))));
  _ = textureLoad(tex_3d, clamp(vec3<i32>(1, 2, 3), vec3(0), vec3<i32>((vec3<u32>(textureDimensions(tex_3d, clamp(level_idx, 0, i32((u32(textureNumLevels(tex_3d)) - 1))))) - vec3(1)))), clamp(level_idx, 0, i32((u32(textureNumLevels(tex_3d)) - 1))));
  _ = textureLoad(tex_ms_2d, clamp(vec2<i32>(1, 2), vec2(0), vec2<i32>((vec2<u32>(textureDimensions(tex_ms_2d)) - vec2(1)))), sample_idx);
  _ = textureLoad(tex_depth_2d, clamp(vec2<i32>(1, 2), vec2(0), vec2<i32>((vec2<u32>(textureDimensions(tex_depth_2d, clamp(level_idx, 0, i32((u32(textureNumLevels(tex_depth_2d)) - 1))))) - vec2(1)))), clamp(level_idx, 0, i32((u32(textureNumLevels(tex_depth_2d)) - 1))));
  _ = textureLoad(tex_depth_2d_arr, clamp(vec2<i32>(1, 2), vec2(0), vec2<i32>((vec2<u32>(textureDimensions(tex_depth_2d_arr, clamp(level_idx, 0, i32((u32(textureNumLevels(tex_depth_2d_arr)) - 1))))) - vec2(1)))), clamp(array_idx, 0, i32((u32(textureNumLayers(tex_depth_2d_arr)) - 1))), clamp(level_idx, 0, i32((u32(textureNumLevels(tex_depth_2d_arr)) - 1))));
  _ = textureLoad(tex_external, clamp(vec2<i32>(1, 2), vec2(0), vec2<i32>((vec2<u32>(textureDimensions(tex_external)) - vec2(1)))));
}

fn idx_unsigned() {
  var array_idx : u32;
  var level_idx : u32;
  var sample_idx : u32;
  _ = textureLoad(tex_1d, min(1u, (u32(textureDimensions(tex_1d, min(level_idx, (u32(textureNumLevels(tex_1d)) - 1)))) - 1)), min(level_idx, (u32(textureNumLevels(tex_1d)) - 1)));
  _ = textureLoad(tex_2d, min(vec2<u32>(1, 2), (vec2<u32>(textureDimensions(tex_2d, min(level_idx, (u32(textureNumLevels(tex_2d)) - 1)))) - vec2(1))), min(level_idx, (u32(textureNumLevels(tex_2d)) - 1)));
  _ = textureLoad(tex_2d_arr, min(vec2<u32>(1, 2), (vec2<u32>(textureDimensions(tex_2d_arr, min(level_idx, (u32(textureNumLevels(tex_2d_arr)) - 1)))) - vec2(1))), min(array_idx, (u32(textureNumLayers(tex_2d_arr)) - 1)), min(level_idx, (u32(textureNumLevels(tex_2d_arr)) - 1)));
  _ = textureLoad(tex_3d, min(vec3<u32>(1, 2, 3), (vec3<u32>(textureDimensions(tex_3d, min(level_idx, (u32(textureNumLevels(tex_3d)) - 1)))) - vec3(1))), min(level_idx, (u32(textureNumLevels(tex_3d)) - 1)));
  _ = textureLoad(tex_ms_2d, min(vec2<u32>(1, 2), (vec2<u32>(textureDimensions(tex_ms_2d)) - vec2(1))), sample_idx);
  _ = textureLoad(tex_depth_2d, min(vec2<u32>(1, 2), (vec2<u32>(textureDimensions(tex_depth_2d, min(level_idx, (u32(textureNumLevels(tex_depth_2d)) - 1)))) - vec2(1))), min(level_idx, (u32(textureNumLevels(tex_depth_2d)) - 1)));
  _ = textureLoad(tex_depth_2d_arr, min(vec2<u32>(1, 2), (vec2<u32>(textureDimensions(tex_depth_2d_arr, min(level_idx, (u32(textureNumLevels(tex_depth_2d_arr)) - 1)))) - vec2(1))), min(array_idx, (u32(textureNumLayers(tex_depth_2d_arr)) - 1)), min(level_idx, (u32(textureNumLevels(tex_depth_2d_arr)) - 1)));
  _ = textureLoad(tex_external, min(vec2<u32>(1, 2), (vec2<u32>(textureDimensions(tex_external)) - vec2(1))));
}
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

// Clamp textureLoad() coord, array_index and level values
TEST_F(RobustnessTest, TextureLoad_Clamp_OutOfOrder) {
    auto* src = R"(
fn idx_signed() {
  var array_idx : i32;
  var level_idx : i32;
  var sample_idx : i32;

  _ = textureLoad(tex_1d, 1i, level_idx);
  _ = textureLoad(tex_2d, vec2<i32>(1, 2), level_idx);
  _ = textureLoad(tex_2d_arr, vec2<i32>(1, 2), array_idx, level_idx);
  _ = textureLoad(tex_3d, vec3<i32>(1, 2, 3), level_idx);
  _ = textureLoad(tex_ms_2d, vec2<i32>(1, 2), sample_idx);
  _ = textureLoad(tex_depth_2d, vec2<i32>(1, 2), level_idx);
  _ = textureLoad(tex_depth_2d_arr, vec2<i32>(1, 2), array_idx, level_idx);
  _ = textureLoad(tex_external, vec2<i32>(1, 2));
}

fn idx_unsigned() {
  var array_idx : u32;
  var level_idx : u32;
  var sample_idx : u32;

  _ = textureLoad(tex_1d, 1u, level_idx);
  _ = textureLoad(tex_2d, vec2<u32>(1, 2), level_idx);
  _ = textureLoad(tex_2d_arr, vec2<u32>(1, 2), array_idx, level_idx);
  _ = textureLoad(tex_3d, vec3<u32>(1, 2, 3), level_idx);
  _ = textureLoad(tex_ms_2d, vec2<u32>(1, 2), sample_idx);
  _ = textureLoad(tex_depth_2d, vec2<u32>(1, 2), level_idx);
  _ = textureLoad(tex_depth_2d_arr, vec2<u32>(1, 2), array_idx, level_idx);
  _ = textureLoad(tex_external, vec2<u32>(1, 2));
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
fn idx_signed() {
  var array_idx : i32;
  var level_idx : i32;
  var sample_idx : i32;
  _ = textureLoad(tex_1d, clamp(1i, 0, i32((u32(textureDimensions(tex_1d, clamp(level_idx, 0, i32((u32(textureNumLevels(tex_1d)) - 1))))) - 1))), clamp(level_idx, 0, i32((u32(textureNumLevels(tex_1d)) - 1))));
  _ = textureLoad(tex_2d, clamp(vec2<i32>(1, 2), vec2(0), vec2<i32>((vec2<u32>(textureDimensions(tex_2d, clamp(level_idx, 0, i32((u32(textureNumLevels(tex_2d)) - 1))))) - vec2(1)))), clamp(level_idx, 0, i32((u32(textureNumLevels(tex_2d)) - 1))));
  _ = textureLoad(tex_2d_arr, clamp(vec2<i32>(1, 2), vec2(0), vec2<i32>((vec2<u32>(textureDimensions(tex_2d_arr, clamp(level_idx, 0, i32((u32(textureNumLevels(tex_2d_arr)) - 1))))) - vec2(1)))), clamp(array_idx, 0, i32((u32(textureNumLayers(tex_2d_arr)) - 1))), clamp(level_idx, 0, i32((u32(textureNumLevels(tex_2d_arr)) - 1))));
  _ = textureLoad(tex_3d, clamp(vec3<i32>(1, 2, 3), vec3(0), vec3<i32>((vec3<u32>(textureDimensions(tex_3d, clamp(level_idx, 0, i32((u32(textureNumLevels(tex_3d)) - 1))))) - vec3(1)))), clamp(level_idx, 0, i32((u32(textureNumLevels(tex_3d)) - 1))));
  _ = textureLoad(tex_ms_2d, clamp(vec2<i32>(1, 2), vec2(0), vec2<i32>((vec2<u32>(textureDimensions(tex_ms_2d)) - vec2(1)))), sample_idx);
  _ = textureLoad(tex_depth_2d, clamp(vec2<i32>(1, 2), vec2(0), vec2<i32>((vec2<u32>(textureDimensions(tex_depth_2d, clamp(level_idx, 0, i32((u32(textureNumLevels(tex_depth_2d)) - 1))))) - vec2(1)))), clamp(level_idx, 0, i32((u32(textureNumLevels(tex_depth_2d)) - 1))));
  _ = textureLoad(tex_depth_2d_arr, clamp(vec2<i32>(1, 2), vec2(0), vec2<i32>((vec2<u32>(textureDimensions(tex_depth_2d_arr, clamp(level_idx, 0, i32((u32(textureNumLevels(tex_depth_2d_arr)) - 1))))) - vec2(1)))), clamp(array_idx, 0, i32((u32(textureNumLayers(tex_depth_2d_arr)) - 1))), clamp(level_idx, 0, i32((u32(textureNumLevels(tex_depth_2d_arr)) - 1))));
  _ = textureLoad(tex_external, clamp(vec2<i32>(1, 2), vec2(0), vec2<i32>((vec2<u32>(textureDimensions(tex_external)) - vec2(1)))));
}

fn idx_unsigned() {
  var array_idx : u32;
  var level_idx : u32;
  var sample_idx : u32;
  _ = textureLoad(tex_1d, min(1u, (u32(textureDimensions(tex_1d, min(level_idx, (u32(textureNumLevels(tex_1d)) - 1)))) - 1)), min(level_idx, (u32(textureNumLevels(tex_1d)) - 1)));
  _ = textureLoad(tex_2d, min(vec2<u32>(1, 2), (vec2<u32>(textureDimensions(tex_2d, min(level_idx, (u32(textureNumLevels(tex_2d)) - 1)))) - vec2(1))), min(level_idx, (u32(textureNumLevels(tex_2d)) - 1)));
  _ = textureLoad(tex_2d_arr, min(vec2<u32>(1, 2), (vec2<u32>(textureDimensions(tex_2d_arr, min(level_idx, (u32(textureNumLevels(tex_2d_arr)) - 1)))) - vec2(1))), min(array_idx, (u32(textureNumLayers(tex_2d_arr)) - 1)), min(level_idx, (u32(textureNumLevels(tex_2d_arr)) - 1)));
  _ = textureLoad(tex_3d, min(vec3<u32>(1, 2, 3), (vec3<u32>(textureDimensions(tex_3d, min(level_idx, (u32(textureNumLevels(tex_3d)) - 1)))) - vec3(1))), min(level_idx, (u32(textureNumLevels(tex_3d)) - 1)));
  _ = textureLoad(tex_ms_2d, min(vec2<u32>(1, 2), (vec2<u32>(textureDimensions(tex_ms_2d)) - vec2(1))), sample_idx);
  _ = textureLoad(tex_depth_2d, min(vec2<u32>(1, 2), (vec2<u32>(textureDimensions(tex_depth_2d, min(level_idx, (u32(textureNumLevels(tex_depth_2d)) - 1)))) - vec2(1))), min(level_idx, (u32(textureNumLevels(tex_depth_2d)) - 1)));
  _ = textureLoad(tex_depth_2d_arr, min(vec2<u32>(1, 2), (vec2<u32>(textureDimensions(tex_depth_2d_arr, min(level_idx, (u32(textureNumLevels(tex_depth_2d_arr)) - 1)))) - vec2(1))), min(array_idx, (u32(textureNumLayers(tex_depth_2d_arr)) - 1)), min(level_idx, (u32(textureNumLevels(tex_depth_2d_arr)) - 1)));
  _ = textureLoad(tex_external, min(vec2<u32>(1, 2), (vec2<u32>(textureDimensions(tex_external)) - vec2(1))));
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

fn idx_signed() {
  textureStore(tex1d, 10i, vec4<i32>());
  textureStore(tex2d, vec2<i32>(10, 20), vec4<i32>());
  textureStore(tex2d_arr, vec2<i32>(10, 20), 50i, vec4<i32>());
  textureStore(tex3d, vec3<i32>(10, 20, 30), vec4<i32>());
}

fn idx_unsigned() {
  textureStore(tex1d, 10u, vec4<i32>());
  textureStore(tex2d, vec2<u32>(10, 20), vec4<i32>());
  textureStore(tex2d_arr, vec2<u32>(10, 20), 50u, vec4<i32>());
  textureStore(tex3d, vec3<u32>(10, 20, 30), vec4<i32>());
}
)";

    auto* expect = R"(
@group(0) @binding(0) var tex1d : texture_storage_1d<rgba8sint, write>;

@group(0) @binding(1) var tex2d : texture_storage_2d<rgba8sint, write>;

@group(0) @binding(2) var tex2d_arr : texture_storage_2d_array<rgba8sint, write>;

@group(0) @binding(3) var tex3d : texture_storage_3d<rgba8sint, write>;

fn idx_signed() {
  textureStore(tex1d, clamp(10i, 0, i32((u32(textureDimensions(tex1d)) - 1))), vec4<i32>());
  textureStore(tex2d, clamp(vec2<i32>(10, 20), vec2(0), vec2<i32>((vec2<u32>(textureDimensions(tex2d)) - vec2(1)))), vec4<i32>());
  textureStore(tex2d_arr, clamp(vec2<i32>(10, 20), vec2(0), vec2<i32>((vec2<u32>(textureDimensions(tex2d_arr)) - vec2(1)))), clamp(50i, 0, i32((u32(textureNumLayers(tex2d_arr)) - 1))), vec4<i32>());
  textureStore(tex3d, clamp(vec3<i32>(10, 20, 30), vec3(0), vec3<i32>((vec3<u32>(textureDimensions(tex3d)) - vec3(1)))), vec4<i32>());
}

fn idx_unsigned() {
  textureStore(tex1d, min(10u, (u32(textureDimensions(tex1d)) - 1)), vec4<i32>());
  textureStore(tex2d, min(vec2<u32>(10, 20), (vec2<u32>(textureDimensions(tex2d)) - vec2(1))), vec4<i32>());
  textureStore(tex2d_arr, min(vec2<u32>(10, 20), (vec2<u32>(textureDimensions(tex2d_arr)) - vec2(1))), min(50u, (u32(textureNumLayers(tex2d_arr)) - 1)), vec4<i32>());
  textureStore(tex3d, min(vec3<u32>(10, 20, 30), (vec3<u32>(textureDimensions(tex3d)) - vec3(1))), vec4<i32>());
}
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

// Clamp textureStore() coord, array_index and level values
TEST_F(RobustnessTest, TextureStore_Clamp_OutOfOrder) {
    auto* src = R"(
fn idx_signed() {
  textureStore(tex1d, 10i, vec4<i32>());
  textureStore(tex2d, vec2<i32>(10, 20), vec4<i32>());
  textureStore(tex2d_arr, vec2<i32>(10, 20), 50i, vec4<i32>());
  textureStore(tex3d, vec3<i32>(10, 20, 30), vec4<i32>());
}

fn idx_unsigned() {
  textureStore(tex1d, 10u, vec4<i32>());
  textureStore(tex2d, vec2<u32>(10, 20), vec4<i32>());
  textureStore(tex2d_arr, vec2<u32>(10, 20), 50u, vec4<i32>());
  textureStore(tex3d, vec3<u32>(10, 20, 30), vec4<i32>());
}

@group(0) @binding(0) var tex1d : texture_storage_1d<rgba8sint, write>;

@group(0) @binding(1) var tex2d : texture_storage_2d<rgba8sint, write>;

@group(0) @binding(2) var tex2d_arr : texture_storage_2d_array<rgba8sint, write>;

@group(0) @binding(3) var tex3d : texture_storage_3d<rgba8sint, write>;

)";

    auto* expect = R"(
fn idx_signed() {
  textureStore(tex1d, clamp(10i, 0, i32((u32(textureDimensions(tex1d)) - 1))), vec4<i32>());
  textureStore(tex2d, clamp(vec2<i32>(10, 20), vec2(0), vec2<i32>((vec2<u32>(textureDimensions(tex2d)) - vec2(1)))), vec4<i32>());
  textureStore(tex2d_arr, clamp(vec2<i32>(10, 20), vec2(0), vec2<i32>((vec2<u32>(textureDimensions(tex2d_arr)) - vec2(1)))), clamp(50i, 0, i32((u32(textureNumLayers(tex2d_arr)) - 1))), vec4<i32>());
  textureStore(tex3d, clamp(vec3<i32>(10, 20, 30), vec3(0), vec3<i32>((vec3<u32>(textureDimensions(tex3d)) - vec3(1)))), vec4<i32>());
}

fn idx_unsigned() {
  textureStore(tex1d, min(10u, (u32(textureDimensions(tex1d)) - 1)), vec4<i32>());
  textureStore(tex2d, min(vec2<u32>(10, 20), (vec2<u32>(textureDimensions(tex2d)) - vec2(1))), vec4<i32>());
  textureStore(tex2d_arr, min(vec2<u32>(10, 20), (vec2<u32>(textureDimensions(tex2d_arr)) - vec2(1))), min(50u, (u32(textureNumLayers(tex2d_arr)) - 1)), vec4<i32>());
  textureStore(tex3d, min(vec3<u32>(10, 20, 30), (vec3<u32>(textureDimensions(tex3d)) - vec3(1))), vec4<i32>());
}

@group(0) @binding(0) var tex1d : texture_storage_1d<rgba8sint, write>;

@group(0) @binding(1) var tex2d : texture_storage_2d<rgba8sint, write>;

@group(0) @binding(2) var tex2d_arr : texture_storage_2d_array<rgba8sint, write>;

@group(0) @binding(3) var tex3d : texture_storage_3d<rgba8sint, write>;
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, Shadowed_Variable) {
    auto* src = R"(
fn f() {
  var a : array<f32, 3>;
  var i : u32;
  {
     var a : array<f32, 5>;
     var b : f32 = a[i];
  }
  var c : f32 = a[i];
}
)";

    auto* expect = R"(
fn f() {
  var a : array<f32, 3>;
  var i : u32;
  {
    var a : array<f32, 5>;
    var b : f32 = a[min(i, 4u)];
  }
  var c : f32 = a[min(i, 2u)];
}
)";

    auto got = Run<Robustness>(src);
    EXPECT_EQ(expect, str(got));
}

// Check that existing use of min() and arrayLength() do not get renamed.
TEST_F(RobustnessTest, DontRenameSymbols) {
    auto* src = R"(
struct S {
  a : f32,
  b : array<f32>,
};

@group(0) @binding(0) var<storage, read> s : S;

const c : u32 = 1u;

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

const c : u32 = 1u;

fn f() {
  let b : f32 = s.b[min(c, (arrayLength(&(s.b)) - 1u))];
  let x : i32 = min(1, 2);
  let y : u32 = arrayLength(&(s.b));
}
)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

const char* kOmitSourceShader = R"(
struct S {
  vector : vec3<f32>,
  fixed_arr : array<f32, 4>,
  runtime_arr : array<f32>,
};
@group(0) @binding(0) var<storage, read> s : S;

struct U {
  vector : vec4<f32>,
  fixed_arr : array<vec4<f32>, 4>,
};
@group(1) @binding(0) var<uniform> u : U;

fn f() {
  // i32
  {
    let i = 0i;
    var storage_vector : f32 = s.vector[i];
    var storage_fixed_arr : f32 = s.fixed_arr[i];
    var storage_runtime_arr : f32 = s.runtime_arr[i];
    var uniform_vector : f32 = u.vector[i];
    var uniform_fixed_arr : vec4<f32> = u.fixed_arr[i];
    var uniform_fixed_arr_vector : f32 = u.fixed_arr[0][i];
  }
  // u32
  {
    let i = 0u;
    var storage_vector : f32 = s.vector[i];
    var storage_fixed_arr : f32 = s.fixed_arr[i];
    var storage_runtime_arr : f32 = s.runtime_arr[i];
    var uniform_vector : f32 = u.vector[i];
    var uniform_fixed_arr : vec4<f32> = u.fixed_arr[i];
    var uniform_fixed_arr_vector : f32 = u.fixed_arr[0][i];
  }
}
)";

TEST_F(RobustnessTest, OmitNone) {
    auto* expect =
        R"(
struct S {
  vector : vec3<f32>,
  fixed_arr : array<f32, 4>,
  runtime_arr : array<f32>,
}

@group(0) @binding(0) var<storage, read> s : S;

struct U {
  vector : vec4<f32>,
  fixed_arr : array<vec4<f32>, 4>,
}

@group(1) @binding(0) var<uniform> u : U;

fn f() {
  {
    let i = 0i;
    var storage_vector : f32 = s.vector[min(u32(i), 2u)];
    var storage_fixed_arr : f32 = s.fixed_arr[min(u32(i), 3u)];
    var storage_runtime_arr : f32 = s.runtime_arr[min(u32(i), (arrayLength(&(s.runtime_arr)) - 1u))];
    var uniform_vector : f32 = u.vector[min(u32(i), 3u)];
    var uniform_fixed_arr : vec4<f32> = u.fixed_arr[min(u32(i), 3u)];
    var uniform_fixed_arr_vector : f32 = u.fixed_arr[0][min(u32(i), 3u)];
  }
  {
    let i = 0u;
    var storage_vector : f32 = s.vector[min(i, 2u)];
    var storage_fixed_arr : f32 = s.fixed_arr[min(i, 3u)];
    var storage_runtime_arr : f32 = s.runtime_arr[min(i, (arrayLength(&(s.runtime_arr)) - 1u))];
    var uniform_vector : f32 = u.vector[min(i, 3u)];
    var uniform_fixed_arr : vec4<f32> = u.fixed_arr[min(i, 3u)];
    var uniform_fixed_arr_vector : f32 = u.fixed_arr[0][min(i, 3u)];
  }
}
)";

    Robustness::Config cfg;
    DataMap data;
    data.Add<Robustness::Config>(cfg);

    auto got = Run<Robustness>(kOmitSourceShader, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, OmitStorage) {
    auto* expect =
        R"(
struct S {
  vector : vec3<f32>,
  fixed_arr : array<f32, 4>,
  runtime_arr : array<f32>,
}

@group(0) @binding(0) var<storage, read> s : S;

struct U {
  vector : vec4<f32>,
  fixed_arr : array<vec4<f32>, 4>,
}

@group(1) @binding(0) var<uniform> u : U;

fn f() {
  {
    let i = 0i;
    var storage_vector : f32 = s.vector[i];
    var storage_fixed_arr : f32 = s.fixed_arr[i];
    var storage_runtime_arr : f32 = s.runtime_arr[i];
    var uniform_vector : f32 = u.vector[min(u32(i), 3u)];
    var uniform_fixed_arr : vec4<f32> = u.fixed_arr[min(u32(i), 3u)];
    var uniform_fixed_arr_vector : f32 = u.fixed_arr[0][min(u32(i), 3u)];
  }
  {
    let i = 0u;
    var storage_vector : f32 = s.vector[i];
    var storage_fixed_arr : f32 = s.fixed_arr[i];
    var storage_runtime_arr : f32 = s.runtime_arr[i];
    var uniform_vector : f32 = u.vector[min(i, 3u)];
    var uniform_fixed_arr : vec4<f32> = u.fixed_arr[min(i, 3u)];
    var uniform_fixed_arr_vector : f32 = u.fixed_arr[0][min(i, 3u)];
  }
}
)";

    Robustness::Config cfg;
    cfg.omitted_address_spaces.insert(Robustness::AddressSpace::kStorage);

    DataMap data;
    data.Add<Robustness::Config>(cfg);

    auto got = Run<Robustness>(kOmitSourceShader, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, OmitUniform) {
    auto* expect =
        R"(
struct S {
  vector : vec3<f32>,
  fixed_arr : array<f32, 4>,
  runtime_arr : array<f32>,
}

@group(0) @binding(0) var<storage, read> s : S;

struct U {
  vector : vec4<f32>,
  fixed_arr : array<vec4<f32>, 4>,
}

@group(1) @binding(0) var<uniform> u : U;

fn f() {
  {
    let i = 0i;
    var storage_vector : f32 = s.vector[min(u32(i), 2u)];
    var storage_fixed_arr : f32 = s.fixed_arr[min(u32(i), 3u)];
    var storage_runtime_arr : f32 = s.runtime_arr[min(u32(i), (arrayLength(&(s.runtime_arr)) - 1u))];
    var uniform_vector : f32 = u.vector[i];
    var uniform_fixed_arr : vec4<f32> = u.fixed_arr[i];
    var uniform_fixed_arr_vector : f32 = u.fixed_arr[0][i];
  }
  {
    let i = 0u;
    var storage_vector : f32 = s.vector[min(i, 2u)];
    var storage_fixed_arr : f32 = s.fixed_arr[min(i, 3u)];
    var storage_runtime_arr : f32 = s.runtime_arr[min(i, (arrayLength(&(s.runtime_arr)) - 1u))];
    var uniform_vector : f32 = u.vector[i];
    var uniform_fixed_arr : vec4<f32> = u.fixed_arr[i];
    var uniform_fixed_arr_vector : f32 = u.fixed_arr[0][i];
  }
}
)";

    Robustness::Config cfg;
    cfg.omitted_address_spaces.insert(Robustness::AddressSpace::kUniform);

    DataMap data;
    data.Add<Robustness::Config>(cfg);

    auto got = Run<Robustness>(kOmitSourceShader, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, OmitBoth) {
    auto* expect =
        R"(
struct S {
  vector : vec3<f32>,
  fixed_arr : array<f32, 4>,
  runtime_arr : array<f32>,
}

@group(0) @binding(0) var<storage, read> s : S;

struct U {
  vector : vec4<f32>,
  fixed_arr : array<vec4<f32>, 4>,
}

@group(1) @binding(0) var<uniform> u : U;

fn f() {
  {
    let i = 0i;
    var storage_vector : f32 = s.vector[i];
    var storage_fixed_arr : f32 = s.fixed_arr[i];
    var storage_runtime_arr : f32 = s.runtime_arr[i];
    var uniform_vector : f32 = u.vector[i];
    var uniform_fixed_arr : vec4<f32> = u.fixed_arr[i];
    var uniform_fixed_arr_vector : f32 = u.fixed_arr[0][i];
  }
  {
    let i = 0u;
    var storage_vector : f32 = s.vector[i];
    var storage_fixed_arr : f32 = s.fixed_arr[i];
    var storage_runtime_arr : f32 = s.runtime_arr[i];
    var uniform_vector : f32 = u.vector[i];
    var uniform_fixed_arr : vec4<f32> = u.fixed_arr[i];
    var uniform_fixed_arr_vector : f32 = u.fixed_arr[0][i];
  }
}
)";

    Robustness::Config cfg;
    cfg.omitted_address_spaces.insert(Robustness::AddressSpace::kStorage);
    cfg.omitted_address_spaces.insert(Robustness::AddressSpace::kUniform);

    DataMap data;
    data.Add<Robustness::Config>(cfg);

    auto got = Run<Robustness>(kOmitSourceShader, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RobustnessTest, WorkgroupOverrideCount) {
    auto* src = R"(
override N = 123;
var<workgroup> w : array<f32, N>;

fn f() {
  var b : f32 = w[1i];
}
)";

    auto* expect =
        R"(error: array size is an override-expression, when expected a constant-expression.
Was the SubstituteOverride transform run?)";

    auto got = Run<Robustness>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::transform
