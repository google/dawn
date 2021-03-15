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

#include "src/transform/bound_array_accessors.h"

#include "src/transform/test_helper.h"

namespace tint {
namespace transform {
namespace {

using BoundArrayAccessorsTest = TransformTest;

TEST_F(BoundArrayAccessorsTest, Ptrs_Clamp) {
  auto* src = R"(
var a : array<f32, 3>;

const c : u32 = 1u;

fn f() -> void {
  const b : ptr<function, f32> = a[c];
}
)";

  auto* expect = R"(
var a : array<f32, 3>;

const c : u32 = 1u;

fn f() -> void {
  const b : ptr<function, f32> = a[min(u32(c), 2u)];
}
)";

  auto got = Run<BoundArrayAccessors>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(BoundArrayAccessorsTest, Array_Idx_Nested_Scalar) {
  auto* src = R"(
var a : array<f32, 3>;

var b : array<f32, 5>;

var i : u32;

fn f() -> void {
  var c : f32 = a[ b[i] ];
}
)";

  auto* expect = R"(
var a : array<f32, 3>;

var b : array<f32, 5>;

var i : u32;

fn f() -> void {
  var c : f32 = a[min(u32(b[min(u32(i), 4u)]), 2u)];
}
)";

  auto got = Run<BoundArrayAccessors>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(BoundArrayAccessorsTest, Array_Idx_Scalar) {
  auto* src = R"(
var a : array<f32, 3>;

fn f() -> void {
  var b : f32 = a[1];
}
)";

  auto* expect = R"(
var a : array<f32, 3>;

fn f() -> void {
  var b : f32 = a[1];
}
)";

  auto got = Run<BoundArrayAccessors>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(BoundArrayAccessorsTest, Array_Idx_Expr) {
  auto* src = R"(
var a : array<f32, 3>;

var c : u32;

fn f() -> void {
  var b : f32 = a[c + 2 - 3];
}
)";

  auto* expect = R"(
var a : array<f32, 3>;

var c : u32;

fn f() -> void {
  var b : f32 = a[min(u32(((c + 2) - 3)), 2u)];
}
)";

  auto got = Run<BoundArrayAccessors>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(BoundArrayAccessorsTest, Array_Idx_Negative) {
  auto* src = R"(
var a : array<f32, 3>;

fn f() -> void {
  var b : f32 = a[-1];
}
)";

  auto* expect = R"(
var a : array<f32, 3>;

fn f() -> void {
  var b : f32 = a[0];
}
)";

  auto got = Run<BoundArrayAccessors>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(BoundArrayAccessorsTest, Array_Idx_OutOfBounds) {
  auto* src = R"(
var a : array<f32, 3>;

fn f() -> void {
  var b : f32 = a[3];
}
)";

  auto* expect = R"(
var a : array<f32, 3>;

fn f() -> void {
  var b : f32 = a[2];
}
)";

  auto got = Run<BoundArrayAccessors>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(BoundArrayAccessorsTest, Vector_Idx_Scalar) {
  auto* src = R"(
var a : vec3<f32>;

fn f() -> void {
  var b : f32 = a[1];
}
)";

  auto* expect = R"(
var a : vec3<f32>;

fn f() -> void {
  var b : f32 = a[1];
}
)";

  auto got = Run<BoundArrayAccessors>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(BoundArrayAccessorsTest, Vector_Idx_Expr) {
  auto* src = R"(
var a : vec3<f32>;

var c : u32;

fn f() -> void {
  var b : f32 = a[c + 2 - 3];
}
)";

  auto* expect = R"(
var a : vec3<f32>;

var c : u32;

fn f() -> void {
  var b : f32 = a[min(u32(((c + 2) - 3)), 2u)];
}
)";

  auto got = Run<BoundArrayAccessors>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(BoundArrayAccessorsTest, Vector_Swizzle_Idx_Scalar) {
  auto* src = R"(
var a : vec3<f32>;

fn f() -> void {
  var b : f32 = a.xy[2];
}
)";

  auto* expect = R"(
var a : vec3<f32>;

fn f() -> void {
  var b : f32 = a.xy[1];
}
)";

  auto got = Run<BoundArrayAccessors>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(BoundArrayAccessorsTest, Vector_Swizzle_Idx_Var) {
  auto* src = R"(
var a : vec3<f32>;

var c : u32;

fn f() -> void {
  var b : f32 = a.xy[c];
}
)";

  auto* expect = R"(
var a : vec3<f32>;

var c : u32;

fn f() -> void {
  var b : f32 = a.xy[min(u32(c), 1u)];
}
)";

  auto got = Run<BoundArrayAccessors>(src);

  EXPECT_EQ(expect, str(got));
}
TEST_F(BoundArrayAccessorsTest, Vector_Swizzle_Idx_Expr) {
  auto* src = R"(
var a : vec3<f32>;

var c : u32;

fn f() -> void {
  var b : f32 = a.xy[c + 2 - 3];
}
)";

  auto* expect = R"(
var a : vec3<f32>;

var c : u32;

fn f() -> void {
  var b : f32 = a.xy[min(u32(((c + 2) - 3)), 1u)];
}
)";

  auto got = Run<BoundArrayAccessors>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(BoundArrayAccessorsTest, Vector_Idx_Negative) {
  auto* src = R"(
var a : vec3<f32>;

fn f() -> void {
  var b : f32 = a[-1];
}
)";

  auto* expect = R"(
var a : vec3<f32>;

fn f() -> void {
  var b : f32 = a[0];
}
)";

  auto got = Run<BoundArrayAccessors>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(BoundArrayAccessorsTest, Vector_Idx_OutOfBounds) {
  auto* src = R"(
var a : vec3<f32>;

fn f() -> void {
  var b : f32 = a[3];
}
)";

  auto* expect = R"(
var a : vec3<f32>;

fn f() -> void {
  var b : f32 = a[2];
}
)";

  auto got = Run<BoundArrayAccessors>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(BoundArrayAccessorsTest, Matrix_Idx_Scalar) {
  auto* src = R"(
var a : mat3x2<f32>;

fn f() -> void {
  var b : f32 = a[2][1];
}
)";

  auto* expect = R"(
var a : mat3x2<f32>;

fn f() -> void {
  var b : f32 = a[2][1];
}
)";

  auto got = Run<BoundArrayAccessors>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(BoundArrayAccessorsTest, Matrix_Idx_Expr_Column) {
  auto* src = R"(
var a : mat3x2<f32>;

var c : u32;

fn f() -> void {
  var b : f32 = a[c + 2 - 3][1];
}
)";

  auto* expect = R"(
var a : mat3x2<f32>;

var c : u32;

fn f() -> void {
  var b : f32 = a[min(u32(((c + 2) - 3)), 2u)][1];
}
)";

  auto got = Run<BoundArrayAccessors>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(BoundArrayAccessorsTest, Matrix_Idx_Expr_Row) {
  auto* src = R"(
var a : mat3x2<f32>;

var c : u32;

fn f() -> void {
  var b : f32 = a[1][c + 2 - 3];
}
)";

  auto* expect = R"(
var a : mat3x2<f32>;

var c : u32;

fn f() -> void {
  var b : f32 = a[1][min(u32(((c + 2) - 3)), 1u)];
}
)";

  auto got = Run<BoundArrayAccessors>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(BoundArrayAccessorsTest, Matrix_Idx_Negative_Column) {
  auto* src = R"(
var a : mat3x2<f32>;

fn f() -> void {
  var b : f32 = a[-1][1];
}
)";

  auto* expect = R"(
var a : mat3x2<f32>;

fn f() -> void {
  var b : f32 = a[0][1];
}
)";

  auto got = Run<BoundArrayAccessors>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(BoundArrayAccessorsTest, Matrix_Idx_Negative_Row) {
  auto* src = R"(
var a : mat3x2<f32>;

fn f() -> void {
  var b : f32 = a[2][-1];
}
)";

  auto* expect = R"(
var a : mat3x2<f32>;

fn f() -> void {
  var b : f32 = a[2][0];
}
)";

  auto got = Run<BoundArrayAccessors>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(BoundArrayAccessorsTest, Matrix_Idx_OutOfBounds_Column) {
  auto* src = R"(
var a : mat3x2<f32>;

fn f() -> void {
  var b : f32 = a[5][1];
}
)";

  auto* expect = R"(
var a : mat3x2<f32>;

fn f() -> void {
  var b : f32 = a[2][1];
}
)";

  auto got = Run<BoundArrayAccessors>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(BoundArrayAccessorsTest, Matrix_Idx_OutOfBounds_Row) {
  auto* src = R"(
var a : mat3x2<f32>;

fn f() -> void {
  var b : f32 = a[2][5];
}
)";

  auto* expect = R"(
var a : mat3x2<f32>;

fn f() -> void {
  var b : f32 = a[2][1];
}
)";

  auto got = Run<BoundArrayAccessors>(src);

  EXPECT_EQ(expect, str(got));
}

// TODO(dsinclair): Implement when constant_id exists
TEST_F(BoundArrayAccessorsTest, DISABLED_Vector_Constant_Id_Clamps) {
  // [[constant_id(1300)]] const idx : i32;
  // var a : vec3<f32>
  // var b : f32 = a[idx]
  //
  // ->var b : f32 = a[min(u32(idx), 2)]
}

// TODO(dsinclair): Implement when constant_id exists
TEST_F(BoundArrayAccessorsTest, DISABLED_Array_Constant_Id_Clamps) {
  // [[constant_id(1300)]] const idx : i32;
  // var a : array<f32, 4>
  // var b : f32 = a[idx]
  //
  // -> var b : f32 = a[min(u32(idx), 3)]
}

// TODO(dsinclair): Implement when constant_id exists
TEST_F(BoundArrayAccessorsTest, DISABLED_Matrix_Column_Constant_Id_Clamps) {
  // [[constant_id(1300)]] const idx : i32;
  // var a : mat3x2<f32>
  // var b : f32 = a[idx][1]
  //
  // -> var b : f32 = a[min(u32(idx), 2)][1]
}

// TODO(dsinclair): Implement when constant_id exists
TEST_F(BoundArrayAccessorsTest, DISABLED_Matrix_Row_Constant_Id_Clamps) {
  // [[constant_id(1300)]] const idx : i32;
  // var a : mat3x2<f32>
  // var b : f32 = a[1][idx]
  //
  // -> var b : f32 = a[1][min(u32(idx), 0, 1)]
}

TEST_F(BoundArrayAccessorsTest, RuntimeArray_Clamps) {
  auto* src = R"(
struct S {
  a : f32;
  b : array<f32>;
};
var s : S;

fn f() -> void {
  var d : f32 = s.b[25];
}
)";

  auto* expect = R"(
struct S {
  a : f32;
  b : array<f32>;
};

var s : S;

fn f() -> void {
  var d : f32 = s.b[min(u32(25), (arrayLength(s.b) - 1u))];
}
)";

  auto got = Run<BoundArrayAccessors>(src);

  EXPECT_EQ(expect, str(got));
}

// TODO(dsinclair): Clamp atomics when available.
TEST_F(BoundArrayAccessorsTest, DISABLED_Atomics_Clamp) {
  FAIL();
}

// TODO(dsinclair): Clamp texture coord values. Depends on:
// https://github.com/gpuweb/gpuweb/issues/1107
TEST_F(BoundArrayAccessorsTest, DISABLED_TextureCoord_Clamp) {
  FAIL();
}

// TODO(dsinclair): Test for scoped variables when Lexical Scopes implemented
TEST_F(BoundArrayAccessorsTest, DISABLED_Scoped_Variable) {
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

}  // namespace
}  // namespace transform
}  // namespace tint
