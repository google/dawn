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

#include "src/tint/transform/expand_compound_assignment.h"

#include <utility>

#include "src/tint/transform/test_helper.h"

namespace tint::transform {
namespace {

using ExpandCompoundAssignmentTest = TransformTest;

TEST_F(ExpandCompoundAssignmentTest, ShouldRunEmptyModule) {
    auto* src = R"()";

    EXPECT_FALSE(ShouldRun<ExpandCompoundAssignment>(src));
}

TEST_F(ExpandCompoundAssignmentTest, ShouldRunHasCompoundAssignment) {
    auto* src = R"(
fn foo() {
  var v : i32;
  v += 1;
}
)";

    EXPECT_TRUE(ShouldRun<ExpandCompoundAssignment>(src));
}

TEST_F(ExpandCompoundAssignmentTest, ShouldRunHasIncrementDecrement) {
    auto* src = R"(
fn foo() {
  var v : i32;
  v++;
}
)";

    EXPECT_TRUE(ShouldRun<ExpandCompoundAssignment>(src));
}

TEST_F(ExpandCompoundAssignmentTest, Basic) {
    auto* src = R"(
fn main() {
  var v : i32;
  v += 1;
}
)";

    auto* expect = R"(
fn main() {
  var v : i32;
  v = (v + 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, LhsPointer) {
    auto* src = R"(
fn main() {
  var v : i32;
  let p = &v;
  *p += 1;
}
)";

    auto* expect = R"(
fn main() {
  var v : i32;
  let p = &(v);
  let tint_symbol = &(*(p));
  *(tint_symbol) = (*(tint_symbol) + 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, LhsStructMember) {
    auto* src = R"(
struct S {
  m : f32,
}

fn main() {
  var s : S;
  s.m += 1.0;
}
)";

    auto* expect = R"(
struct S {
  m : f32,
}

fn main() {
  var s : S;
  s.m = (s.m + 1.0);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, LhsArrayElement) {
    auto* src = R"(
var<private> a : array<i32, 4>;

fn idx() -> i32 {
  a[1] = 42;
  return 1;
}

fn main() {
  a[idx()] += 1;
}
)";

    auto* expect = R"(
var<private> a : array<i32, 4>;

fn idx() -> i32 {
  a[1] = 42;
  return 1;
}

fn main() {
  let tint_symbol = &(a[idx()]);
  *(tint_symbol) = (*(tint_symbol) + 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, LhsVectorComponent_ArrayAccessor) {
    auto* src = R"(
var<private> v : vec4<i32>;

fn idx() -> i32 {
  v.y = 42;
  return 1;
}

fn main() {
  v[idx()] += 1;
}
)";

    auto* expect = R"(
var<private> v : vec4<i32>;

fn idx() -> i32 {
  v.y = 42;
  return 1;
}

fn main() {
  let tint_symbol = &(v);
  let tint_symbol_1 = idx();
  (*(tint_symbol))[tint_symbol_1] = ((*(tint_symbol))[tint_symbol_1] + 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, LhsVectorComponent_MemberAccessor) {
    auto* src = R"(
fn main() {
  var v : vec4<i32>;
  v.y += 1;
}
)";

    auto* expect = R"(
fn main() {
  var v : vec4<i32>;
  v.y = (v.y + 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, LhsMatrixColumn) {
    auto* src = R"(
var<private> m : mat4x4<f32>;

fn idx() -> i32 {
  m[0].y = 42.0;
  return 1;
}

fn main() {
  m[idx()] += 1.0;
}
)";

    auto* expect = R"(
var<private> m : mat4x4<f32>;

fn idx() -> i32 {
  m[0].y = 42.0;
  return 1;
}

fn main() {
  let tint_symbol = &(m[idx()]);
  *(tint_symbol) = (*(tint_symbol) + 1.0);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, LhsMatrixElement) {
    auto* src = R"(
var<private> m : mat4x4<f32>;

fn idx1() -> i32 {
  m[0].y = 42.0;
  return 1;
}

fn idx2() -> i32 {
  m[1].z = 42.0;
  return 1;
}

fn main() {
  m[idx1()][idx2()] += 1.0;
}
)";

    auto* expect = R"(
var<private> m : mat4x4<f32>;

fn idx1() -> i32 {
  m[0].y = 42.0;
  return 1;
}

fn idx2() -> i32 {
  m[1].z = 42.0;
  return 1;
}

fn main() {
  let tint_symbol = &(m[idx1()]);
  let tint_symbol_1 = idx2();
  (*(tint_symbol))[tint_symbol_1] = ((*(tint_symbol))[tint_symbol_1] + 1.0);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, LhsMultipleSideEffects) {
    auto* src = R"(
struct S {
  a : array<vec4<f32>, 3>,
}

@group(0) @binding(0) var<storage, read_write> buffer : array<S>;

var<private> p : i32;

fn idx1() -> i32 {
  p += 1;
  return 3;
}

fn idx2() -> i32 {
  p *= 3;
  return 2;
}

fn idx3() -> i32 {
  p -= 2;
  return 1;
}

fn main() {
  buffer[idx1()].a[idx2()][idx3()] += 1.0;
}
)";

    auto* expect = R"(
struct S {
  a : array<vec4<f32>, 3>,
}

@group(0) @binding(0) var<storage, read_write> buffer : array<S>;

var<private> p : i32;

fn idx1() -> i32 {
  p = (p + 1);
  return 3;
}

fn idx2() -> i32 {
  p = (p * 3);
  return 2;
}

fn idx3() -> i32 {
  p = (p - 2);
  return 1;
}

fn main() {
  let tint_symbol = &(buffer[idx1()].a[idx2()]);
  let tint_symbol_1 = idx3();
  (*(tint_symbol))[tint_symbol_1] = ((*(tint_symbol))[tint_symbol_1] + 1.0);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, ForLoopInit) {
    auto* src = R"(
var<private> a : array<vec4<i32>, 4>;

var<private> p : i32;

fn idx1() -> i32 {
  p = (p + 1);
  return 3;
}

fn idx2() -> i32 {
  p = (p * 3);
  return 2;
}

fn main() {
  for (a[idx1()][idx2()] += 1; ; ) {
    break;
  }
}
)";

    auto* expect = R"(
var<private> a : array<vec4<i32>, 4>;

var<private> p : i32;

fn idx1() -> i32 {
  p = (p + 1);
  return 3;
}

fn idx2() -> i32 {
  p = (p * 3);
  return 2;
}

fn main() {
  {
    let tint_symbol = &(a[idx1()]);
    let tint_symbol_1 = idx2();
    (*(tint_symbol))[tint_symbol_1] = ((*(tint_symbol))[tint_symbol_1] + 1);
    loop {
      {
        break;
      }
    }
  }
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, ForLoopCont) {
    auto* src = R"(
var<private> a : array<vec4<i32>, 4>;

var<private> p : i32;

fn idx1() -> i32 {
  p = (p + 1);
  return 3;
}

fn idx2() -> i32 {
  p = (p * 3);
  return 2;
}

fn main() {
  for (; ; a[idx1()][idx2()] += 1) {
    break;
  }
}
)";

    auto* expect = R"(
var<private> a : array<vec4<i32>, 4>;

var<private> p : i32;

fn idx1() -> i32 {
  p = (p + 1);
  return 3;
}

fn idx2() -> i32 {
  p = (p * 3);
  return 2;
}

fn main() {
  loop {
    {
      break;
    }

    continuing {
      let tint_symbol = &(a[idx1()]);
      let tint_symbol_1 = idx2();
      (*(tint_symbol))[tint_symbol_1] = ((*(tint_symbol))[tint_symbol_1] + 1);
    }
  }
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, Increment_I32) {
    auto* src = R"(
fn main() {
  var v : i32;
  v++;
}
)";

    auto* expect = R"(
fn main() {
  var v : i32;
  v = (v + 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, Increment_U32) {
    auto* src = R"(
fn main() {
  var v : u32;
  v++;
}
)";

    auto* expect = R"(
fn main() {
  var v : u32;
  v = (v + 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, Decrement_I32) {
    auto* src = R"(
fn main() {
  var v : i32;
  v--;
}
)";

    auto* expect = R"(
fn main() {
  var v : i32;
  v = (v - 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, Decrement_U32) {
    auto* src = R"(
fn main() {
  var v : u32;
  v--;
}
)";

    auto* expect = R"(
fn main() {
  var v : u32;
  v = (v - 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, Increment_LhsPointer) {
    auto* src = R"(
fn main() {
  var v : i32;
  let p = &v;
  *p++;
}
)";

    auto* expect = R"(
fn main() {
  var v : i32;
  let p = &(v);
  let tint_symbol = &(*(p));
  *(tint_symbol) = (*(tint_symbol) + 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, Increment_LhsStructMember) {
    auto* src = R"(
struct S {
  m : i32,
}

fn main() {
  var s : S;
  s.m++;
}
)";

    auto* expect = R"(
struct S {
  m : i32,
}

fn main() {
  var s : S;
  s.m = (s.m + 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, Increment_LhsArrayElement) {
    auto* src = R"(
var<private> a : array<i32, 4>;

fn idx() -> i32 {
  a[1] = 42;
  return 1;
}

fn main() {
  a[idx()]++;
}
)";

    auto* expect = R"(
var<private> a : array<i32, 4>;

fn idx() -> i32 {
  a[1] = 42;
  return 1;
}

fn main() {
  let tint_symbol = &(a[idx()]);
  *(tint_symbol) = (*(tint_symbol) + 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, Increment_LhsVectorComponent_ArrayAccessor) {
    auto* src = R"(
var<private> v : vec4<i32>;

fn idx() -> i32 {
  v.y = 42;
  return 1;
}

fn main() {
  v[idx()]++;
}
)";

    auto* expect = R"(
var<private> v : vec4<i32>;

fn idx() -> i32 {
  v.y = 42;
  return 1;
}

fn main() {
  let tint_symbol = &(v);
  let tint_symbol_1 = idx();
  (*(tint_symbol))[tint_symbol_1] = ((*(tint_symbol))[tint_symbol_1] + 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, Increment_LhsVectorComponent_MemberAccessor) {
    auto* src = R"(
fn main() {
  var v : vec4<i32>;
  v.y++;
}
)";

    auto* expect = R"(
fn main() {
  var v : vec4<i32>;
  v.y = (v.y + 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, Increment_ForLoopCont) {
    auto* src = R"(
var<private> a : array<vec4<i32>, 4>;

var<private> p : i32;

fn idx1() -> i32 {
  p = (p + 1);
  return 3;
}

fn idx2() -> i32 {
  p = (p * 3);
  return 2;
}

fn main() {
  for (; ; a[idx1()][idx2()]++) {
    break;
  }
}
)";

    auto* expect = R"(
var<private> a : array<vec4<i32>, 4>;

var<private> p : i32;

fn idx1() -> i32 {
  p = (p + 1);
  return 3;
}

fn idx2() -> i32 {
  p = (p * 3);
  return 2;
}

fn main() {
  loop {
    {
      break;
    }

    continuing {
      let tint_symbol = &(a[idx1()]);
      let tint_symbol_1 = idx2();
      (*(tint_symbol))[tint_symbol_1] = ((*(tint_symbol))[tint_symbol_1] + 1);
    }
  }
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::transform
