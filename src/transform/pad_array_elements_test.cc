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

#include "src/transform/pad_array_elements.h"

#include <utility>

#include "src/transform/test_helper.h"

namespace tint {
namespace transform {
namespace {

using PadArrayElementsTest = TransformTest;

TEST_F(PadArrayElementsTest, EmptyModule) {
  auto* src = "";
  auto* expect = "";

  auto got = Run<PadArrayElements>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PadArrayElementsTest, ImplicitArrayStride) {
  auto* src = R"(
var<private> arr : array<i32, 4>;
)";
  auto* expect = src;

  auto got = Run<PadArrayElements>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PadArrayElementsTest, ArrayAsGlobal) {
  auto* src = R"(
var<private> arr : [[stride(8)]] array<i32, 4>;
)";
  auto* expect = R"(
struct tint_padded_array_element {
  [[size(8)]]
  el : i32;
};

var<private> arr : array<tint_padded_array_element, 4u>;
)";

  auto got = Run<PadArrayElements>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PadArrayElementsTest, RuntimeArray) {
  auto* src = R"(
[[block]]
struct S {
  rta : [[stride(8)]] array<i32>;
};
)";
  auto* expect = R"(
struct tint_padded_array_element {
  [[size(8)]]
  el : i32;
};

[[block]]
struct S {
  rta : array<tint_padded_array_element>;
};
)";

  auto got = Run<PadArrayElements>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PadArrayElementsTest, ArrayFunctionVar) {
  auto* src = R"(
fn f() {
  var arr : [[stride(16)]] array<i32, 4>;
  arr = [[stride(16)]] array<i32, 4>();
  arr = [[stride(16)]] array<i32, 4>(1, 2, 3, 4);
  let x = arr[3];
}
)";
  auto* expect = R"(
struct tint_padded_array_element {
  [[size(16)]]
  el : i32;
};

fn f() {
  var arr : array<tint_padded_array_element, 4u>;
  arr = array<tint_padded_array_element, 4u>();
  arr = array<tint_padded_array_element, 4u>(tint_padded_array_element(1), tint_padded_array_element(2), tint_padded_array_element(3), tint_padded_array_element(4));
  let x = arr[3].el;
}
)";

  auto got = Run<PadArrayElements>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PadArrayElementsTest, ArrayAsParam) {
  auto* src = R"(
fn f(a : [[stride(12)]] array<i32, 4>) -> i32 {
  return a[2];
}
)";
  auto* expect = R"(
struct tint_padded_array_element {
  [[size(12)]]
  el : i32;
};

fn f(a : array<tint_padded_array_element, 4u>) -> i32 {
  return a[2].el;
}
)";

  auto got = Run<PadArrayElements>(src);

  EXPECT_EQ(expect, str(got));
}

// TODO(crbug.com/tint/781): Cannot parse the stride on the return array type.
TEST_F(PadArrayElementsTest, DISABLED_ArrayAsReturn) {
  auto* src = R"(
fn f() -> [[stride(8)]] array<i32, 4> {
  return array<i32, 4>(1, 2, 3, 4);
}
)";
  auto* expect = R"(
struct tint_padded_array_element {
  el : i32;
  [[size(4)]]
  padding : u32;
};

fn f() -> array<tint_padded_array_element, 4> {
  return array<tint_padded_array_element, 4>(tint_padded_array_element(1, 0u), tint_padded_array_element(2, 0u), tint_padded_array_element(3, 0u), tint_padded_array_element(4, 0u));
}
)";

  auto got = Run<PadArrayElements>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PadArrayElementsTest, ArrayAlias) {
  auto* src = R"(
type Array = [[stride(16)]] array<i32, 4>;

fn f() {
  var arr : Array;
  arr = Array();
  arr = Array(1, 2, 3, 4);
  let vals : Array = Array(1, 2, 3, 4);
  arr = vals;
  let x = arr[3];
}
)";
  auto* expect = R"(
struct tint_padded_array_element {
  [[size(16)]]
  el : i32;
};

type Array = array<tint_padded_array_element, 4u>;

fn f() {
  var arr : array<tint_padded_array_element, 4u>;
  arr = array<tint_padded_array_element, 4u>();
  arr = array<tint_padded_array_element, 4u>(tint_padded_array_element(1), tint_padded_array_element(2), tint_padded_array_element(3), tint_padded_array_element(4));
  let vals : array<tint_padded_array_element, 4u> = array<tint_padded_array_element, 4u>(tint_padded_array_element(1), tint_padded_array_element(2), tint_padded_array_element(3), tint_padded_array_element(4));
  arr = vals;
  let x = arr[3].el;
}
)";

  auto got = Run<PadArrayElements>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PadArrayElementsTest, ArraysInStruct) {
  auto* src = R"(
struct S {
  a : [[stride(8)]] array<i32, 4>;
  b : [[stride(8)]] array<i32, 8>;
  c : [[stride(8)]] array<i32, 4>;
  d : [[stride(12)]] array<i32, 8>;
};
)";
  auto* expect = R"(
struct tint_padded_array_element {
  [[size(8)]]
  el : i32;
};

struct tint_padded_array_element_1 {
  [[size(8)]]
  el : i32;
};

struct tint_padded_array_element_2 {
  [[size(12)]]
  el : i32;
};

struct S {
  a : array<tint_padded_array_element, 4u>;
  b : array<tint_padded_array_element_1, 8u>;
  c : array<tint_padded_array_element, 4u>;
  d : array<tint_padded_array_element_2, 8u>;
};
)";

  auto got = Run<PadArrayElements>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PadArrayElementsTest, ArraysOfArraysInStruct) {
  auto* src = R"(
struct S {
  a : [[stride(512)]] array<i32, 4>;
  b : [[stride(512)]] array<[[stride(32)]] array<i32, 4>, 4>;
  c : [[stride(512)]] array<[[stride(64)]] array<[[stride(8)]] array<i32, 4>, 4>, 4>;
};
)";
  auto* expect = R"(
struct tint_padded_array_element {
  [[size(512)]]
  el : i32;
};

struct tint_padded_array_element_2 {
  [[size(32)]]
  el : i32;
};

struct tint_padded_array_element_1 {
  [[size(512)]]
  el : array<tint_padded_array_element_2, 4u>;
};

struct tint_padded_array_element_5 {
  [[size(8)]]
  el : i32;
};

struct tint_padded_array_element_4 {
  [[size(64)]]
  el : array<tint_padded_array_element_5, 4u>;
};

struct tint_padded_array_element_3 {
  [[size(512)]]
  el : array<tint_padded_array_element_4, 4u>;
};

struct S {
  a : array<tint_padded_array_element, 4u>;
  b : array<tint_padded_array_element_1, 4u>;
  c : array<tint_padded_array_element_3, 4u>;
};
)";

  auto got = Run<PadArrayElements>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PadArrayElementsTest, AccessArraysOfArraysInStruct) {
  auto* src = R"(
struct S {
  a : [[stride(512)]] array<i32, 4>;
  b : [[stride(512)]] array<[[stride(32)]] array<i32, 4>, 4>;
  c : [[stride(512)]] array<[[stride(64)]] array<[[stride(8)]] array<i32, 4>, 4>, 4>;
};

fn f(s : S) -> i32 {
  return s.a[2] + s.b[1][2] + s.c[3][1][2];
}
)";
  auto* expect = R"(
struct tint_padded_array_element {
  [[size(512)]]
  el : i32;
};

struct tint_padded_array_element_2 {
  [[size(32)]]
  el : i32;
};

struct tint_padded_array_element_1 {
  [[size(512)]]
  el : array<tint_padded_array_element_2, 4u>;
};

struct tint_padded_array_element_5 {
  [[size(8)]]
  el : i32;
};

struct tint_padded_array_element_4 {
  [[size(64)]]
  el : array<tint_padded_array_element_5, 4u>;
};

struct tint_padded_array_element_3 {
  [[size(512)]]
  el : array<tint_padded_array_element_4, 4u>;
};

struct S {
  a : array<tint_padded_array_element, 4u>;
  b : array<tint_padded_array_element_1, 4u>;
  c : array<tint_padded_array_element_3, 4u>;
};

fn f(s : S) -> i32 {
  return ((s.a[2].el + s.b[1].el[2].el) + s.c[3].el[1].el[2].el);
}
)";

  auto got = Run<PadArrayElements>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PadArrayElementsTest, DeclarationOrder) {
  auto* src = R"(
type T0 = i32;

type T1 = [[stride(8)]] array<i32, 1>;

type T2 = i32;

fn f1(a : [[stride(8)]] array<i32, 2>) {
}

type T3 = i32;

fn f2() {
  var v : [[stride(8)]] array<i32, 3>;
}
)";
  auto* expect = R"(
type T0 = i32;

struct tint_padded_array_element {
  [[size(8)]]
  el : i32;
};

type T1 = array<tint_padded_array_element, 1u>;

type T2 = i32;

struct tint_padded_array_element_1 {
  [[size(8)]]
  el : i32;
};

fn f1(a : array<tint_padded_array_element_1, 2u>) {
}

type T3 = i32;

struct tint_padded_array_element_2 {
  [[size(8)]]
  el : i32;
};

fn f2() {
  var v : array<tint_padded_array_element_2, 3u>;
}
)";

  auto got = Run<PadArrayElements>(src);

  EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace transform
}  // namespace tint
