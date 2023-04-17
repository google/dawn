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

#include "src/tint/transform/simplify_pointers.h"

#include "src/tint/transform/test_helper.h"
#include "src/tint/transform/unshadow.h"

namespace tint::transform {
namespace {

using SimplifyPointersTest = TransformTest;

TEST_F(SimplifyPointersTest, EmptyModule) {
    auto* src = "";

    EXPECT_FALSE(ShouldRun<SimplifyPointers>(src));
}

TEST_F(SimplifyPointersTest, FoldPointer) {
    auto* src = R"(
fn f() {
  var v : i32;
  let p : ptr<function, i32> = &v;
  let x : i32 = *p;
}
)";

    auto* expect = R"(
fn f() {
  var v : i32;
  let x : i32 = v;
}
)";

    auto got = Run<Unshadow, SimplifyPointers>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SimplifyPointersTest, UnusedPointerParam) {
    auto* src = R"(
fn f(p : ptr<function, i32>) {
}
)";

    auto* expect = src;

    auto got = Run<Unshadow, SimplifyPointers>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SimplifyPointersTest, ReferencedPointerParam) {
    auto* src = R"(
fn f(p : ptr<function, i32>) {
  let x = p;
}
)";

    auto* expect = R"(
fn f(p : ptr<function, i32>) {
}
)";

    auto got = Run<Unshadow, SimplifyPointers>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SimplifyPointersTest, AddressOfDeref) {
    auto* src = R"(
fn f() {
  var v : i32;
  let p : ptr<function, i32> = &(v);
  let x : ptr<function, i32> = &(*(p));
  let y : ptr<function, i32> = &(*(&(*(p))));
  let z : ptr<function, i32> = &(*(&(*(&(*(&(*(p))))))));
  var a = *x;
  var b = *y;
  var c = *z;
}
)";

    auto* expect = R"(
fn f() {
  var v : i32;
  var a = v;
  var b = v;
  var c = v;
}
)";

    auto got = Run<Unshadow, SimplifyPointers>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SimplifyPointersTest, DerefAddressOf) {
    auto* src = R"(
fn f() {
  var v : i32;
  let x : i32 = *(&(v));
  let y : i32 = *(&(*(&(v))));
  let z : i32 = *(&(*(&(*(&(*(&(v))))))));
}
)";

    auto* expect = R"(
fn f() {
  var v : i32;
  let x : i32 = v;
  let y : i32 = v;
  let z : i32 = v;
}
)";

    auto got = Run<Unshadow, SimplifyPointers>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SimplifyPointersTest, ComplexChain) {
    auto* src = R"(
fn f() {
  var a : array<mat4x4<f32>, 4>;
  let ap : ptr<function, array<mat4x4<f32>, 4>> = &a;
  let mp : ptr<function, mat4x4<f32>> = &(*ap)[3];
  let vp : ptr<function, vec4<f32>> = &(*mp)[2];
  let v : vec4<f32> = *vp;
}
)";

    auto* expect = R"(
fn f() {
  var a : array<mat4x4<f32>, 4>;
  let v : vec4<f32> = a[3][2];
}
)";

    auto got = Run<Unshadow, SimplifyPointers>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SimplifyPointersTest, SavedVars) {
    auto* src = R"(
struct S {
  i : i32,
};

fn arr() {
  var a : array<S, 2>;
  var i : i32 = 0;
  var j : i32 = 0;
  let p : ptr<function, i32> = &a[i + j].i;
  i = 2;
  *p = 4;
}

fn matrix() {
  var m : mat3x3<f32>;
  var i : i32 = 0;
  var j : i32 = 0;
  let p : ptr<function, vec3<f32>> = &m[i + j];
  i = 2;
  *p = vec3<f32>(4.0, 5.0, 6.0);
}
)";

    auto* expect = R"(
struct S {
  i : i32,
}

fn arr() {
  var a : array<S, 2>;
  var i : i32 = 0;
  var j : i32 = 0;
  let p_save = (i + j);
  i = 2;
  a[p_save].i = 4;
}

fn matrix() {
  var m : mat3x3<f32>;
  var i : i32 = 0;
  var j : i32 = 0;
  let p_save_1 = (i + j);
  i = 2;
  m[p_save_1] = vec3<f32>(4.0, 5.0, 6.0);
}
)";

    auto got = Run<Unshadow, SimplifyPointers>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SimplifyPointersTest, DontSaveLiterals) {
    auto* src = R"(
fn f() {
  var arr : array<i32, 2>;
  let p1 : ptr<function, i32> = &arr[1];
  *p1 = 4;
}
)";

    auto* expect = R"(
fn f() {
  var arr : array<i32, 2>;
  arr[1] = 4;
}
)";

    auto got = Run<Unshadow, SimplifyPointers>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SimplifyPointersTest, SavedVarsChain) {
    auto* src = R"(
fn f() {
  var arr : array<array<i32, 2>, 2>;
  let i : i32 = 0;
  let j : i32 = 1;
  let p : ptr<function, array<i32, 2>> = &arr[i];
  let q : ptr<function, i32> = &(*p)[j];
  *q = 12;
}
)";

    auto* expect = R"(
fn f() {
  var arr : array<array<i32, 2>, 2>;
  let i : i32 = 0;
  let j : i32 = 1;
  let p_save = i;
  let q_save = j;
  arr[p_save][q_save] = 12;
}
)";

    auto got = Run<Unshadow, SimplifyPointers>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SimplifyPointersTest, ForLoopInit) {
    auto* src = R"(
fn foo() -> i32 {
  return 1;
}

@fragment
fn main() {
  var arr = array<f32, 4>();
  for (let a = &arr[foo()]; ;) {
    let x = *a;
    break;
  }
}
)";

    auto* expect = R"(
fn foo() -> i32 {
  return 1;
}

@fragment
fn main() {
  var arr = array<f32, 4>();
  let a_save = foo();
  for(; ; ) {
    let x = arr[a_save];
    break;
  }
}
)";

    auto got = Run<Unshadow, SimplifyPointers>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SimplifyPointersTest, MultiSavedVarsInSinglePtrLetExpr) {
    auto* src = R"(
fn x() -> i32 {
  return 1;
}

fn y() -> i32 {
  return 1;
}

fn z() -> i32 {
  return 1;
}

struct Inner {
  a : array<i32, 2>,
};

struct Outer {
  a : array<Inner, 2>,
};

fn f() {
  var arr : array<Outer, 2>;
  let p : ptr<function, i32> = &arr[x()].a[y()].a[z()];
  *p = 1;
  *p = 2;
}
)";

    auto* expect = R"(
fn x() -> i32 {
  return 1;
}

fn y() -> i32 {
  return 1;
}

fn z() -> i32 {
  return 1;
}

struct Inner {
  a : array<i32, 2>,
}

struct Outer {
  a : array<Inner, 2>,
}

fn f() {
  var arr : array<Outer, 2>;
  let p_save = x();
  let p_save_1 = y();
  let p_save_2 = z();
  arr[p_save].a[p_save_1].a[p_save_2] = 1;
  arr[p_save].a[p_save_1].a[p_save_2] = 2;
}
)";

    auto got = Run<Unshadow, SimplifyPointers>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SimplifyPointersTest, ShadowPointer) {
    auto* src = R"(
var<private> a : array<i32, 2>;

@compute @workgroup_size(1)
fn main() {
  let x = &a;
  var a : i32 = (*x)[0];
  {
    var a : i32 = (*x)[1];
  }
}
)";

    auto* expect = R"(
var<private> a : array<i32, 2>;

@compute @workgroup_size(1)
fn main() {
  var a_1 : i32 = a[0];
  {
    var a_2 : i32 = a[1];
  }
}
)";

    auto got = Run<Unshadow, SimplifyPointers>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::transform
