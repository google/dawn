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

#include "src/transform/inline_pointer_lets.h"

#include <memory>
#include <utility>
#include <vector>

#include "src/transform/test_helper.h"

namespace tint {
namespace transform {
namespace {

using InlinePointerLetsTest = TransformTest;

TEST_F(InlinePointerLetsTest, EmptyModule) {
  auto* src = "";
  auto* expect = "";

  auto got = Run<InlinePointerLets>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(InlinePointerLetsTest, Basic) {
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
  let x : i32 = *(&(v));
}
)";

  auto got = Run<InlinePointerLets>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(InlinePointerLetsTest, ComplexChain) {
  auto* src = R"(
fn f() {
  var m : mat4x4<f32>;
  let mp : ptr<function, mat4x4<f32>> = &m;
  let vp : ptr<function, vec4<f32>> = &(*mp)[2];
  let fp : ptr<function, f32> = &(*vp)[1];
  let f : f32 = *fp;
}
)";

  auto* expect = R"(
fn f() {
  var m : mat4x4<f32>;
  let f : f32 = *(&(*(&(*(&(m))[2]))[1]));
}
)";

  auto got = Run<InlinePointerLets>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(InlinePointerLetsTest, Param) {
  auto* src = R"(
fn x(p : ptr<function, i32>) -> i32 {
  return *p;
}

fn f() {
  var v : i32;
  let p : ptr<function, i32> = &v;
  var r : i32 = x(p);
}
)";

  auto* expect = R"(
fn x(p : ptr<function, i32>) -> i32 {
  return *(p);
}

fn f() {
  var v : i32;
  var r : i32 = x(&(v));
}
)";

  auto got = Run<InlinePointerLets>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(InlinePointerLetsTest, SavedVars) {
  auto* src = R"(
struct S {
  i : i32;
};

fn arr() {
  var a : array<S, 2>;
  var i : i32 = 0;
  var j : i32 = 0;
  let p : ptr<function, i32> = &a[i + j].i;
  i = 2;
  *p = 4;
}

fn vec() {
  var v : vec3<f32>;
  var i : i32 = 0;
  var j : i32 = 0;
  let p : ptr<function, f32> = &v[i + j];
  i = 2;
  *p = 4.0;
}

fn mat() {
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
  i : i32;
};

fn arr() {
  var a : array<S, 2>;
  var i : i32 = 0;
  var j : i32 = 0;
  let p_save = (i + j);
  i = 2;
  *(&(a[p_save].i)) = 4;
}

fn vec() {
  var v : vec3<f32>;
  var i : i32 = 0;
  var j : i32 = 0;
  let p_save_1 = (i + j);
  i = 2;
  *(&(v[p_save_1])) = 4.0;
}

fn mat() {
  var m : mat3x3<f32>;
  var i : i32 = 0;
  var j : i32 = 0;
  let p_save_2 = (i + j);
  i = 2;
  *(&(m[p_save_2])) = vec3<f32>(4.0, 5.0, 6.0);
}
)";

  auto got = Run<InlinePointerLets>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(InlinePointerLetsTest, DontSaveLiterals) {
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
  *(&(arr[1])) = 4;
}
)";

  auto got = Run<InlinePointerLets>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(InlinePointerLetsTest, SavedVarsChain) {
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
  *(&(*(&(arr[p_save]))[q_save])) = 12;
}
)";

  auto got = Run<InlinePointerLets>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(InlinePointerLetsTest, MultiSavedVarsInSinglePtrLetExpr) {
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
  a : array<i32, 2>;
};

struct Outer {
  a : array<Inner, 2>;
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
  a : array<i32, 2>;
};

struct Outer {
  a : array<Inner, 2>;
};

fn f() {
  var arr : array<Outer, 2>;
  let p_save = x();
  let p_save_1 = y();
  let p_save_2 = z();
  *(&(arr[p_save].a[p_save_1].a[p_save_2])) = 1;
  *(&(arr[p_save].a[p_save_1].a[p_save_2])) = 2;
}
)";

  auto got = Run<InlinePointerLets>(src);

  EXPECT_EQ(expect, str(got));
}

// TODO(crbug.com/tint/819): Enable when we support inter-scope shadowing.
TEST_F(InlinePointerLetsTest, DISABLED_ModificationAfterInline) {
  auto* src = R"(
fn x(p : ptr<function, i32>) -> i32 {
  return *p;
}

fn f() {
  var i : i32 = 1;
  let p : ptr<function, i32> = &i;
  if (true) {
    var i : i32 = 2;
    x(p);
  }
}
)";

  auto* expect = R"(<TODO>)";

  auto got = Run<InlinePointerLets>(src);

  EXPECT_EQ(expect, str(got));
}
}  // namespace
}  // namespace transform
}  // namespace tint
