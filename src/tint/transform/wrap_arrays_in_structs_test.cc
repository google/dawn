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

#include "src/tint/transform/wrap_arrays_in_structs.h"

#include <memory>
#include <utility>

#include "src/tint/transform/test_helper.h"

namespace tint::transform {
namespace {

using WrapArraysInStructsTest = TransformTest;

TEST_F(WrapArraysInStructsTest, ShouldRunEmptyModule) {
    auto* src = R"()";

    EXPECT_FALSE(ShouldRun<WrapArraysInStructs>(src));
}

TEST_F(WrapArraysInStructsTest, ShouldRunHasArray) {
    auto* src = R"(
var<private> arr : array<i32, 4>;
)";

    EXPECT_TRUE(ShouldRun<WrapArraysInStructs>(src));
}

TEST_F(WrapArraysInStructsTest, EmptyModule) {
    auto* src = R"()";
    auto* expect = src;

    auto got = Run<WrapArraysInStructs>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(WrapArraysInStructsTest, ArrayAsGlobal) {
    auto* src = R"(
var<private> arr : array<i32, 4>;
)";
    auto* expect = R"(
struct tint_array_wrapper {
  arr : array<i32, 4u>,
}

var<private> arr : tint_array_wrapper;
)";

    auto got = Run<WrapArraysInStructs>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(WrapArraysInStructsTest, ArrayAsFunctionVar) {
    auto* src = R"(
fn f() {
  var arr : array<i32, 4>;
  let x = arr[3];
}
)";
    auto* expect = R"(
struct tint_array_wrapper {
  arr : array<i32, 4u>,
}

fn f() {
  var arr : tint_array_wrapper;
  let x = arr.arr[3];
}
)";

    auto got = Run<WrapArraysInStructs>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(WrapArraysInStructsTest, ArrayAsParam) {
    auto* src = R"(
fn f(a : array<i32, 4>) -> i32 {
  return a[2];
}
)";
    auto* expect = R"(
struct tint_array_wrapper {
  arr : array<i32, 4u>,
}

fn f(a : tint_array_wrapper) -> i32 {
  return a.arr[2];
}
)";

    auto got = Run<WrapArraysInStructs>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(WrapArraysInStructsTest, ArrayAsReturn) {
    auto* src = R"(
fn f() -> array<i32, 4> {
  return array<i32, 4>(1, 2, 3, 4);
}
)";
    auto* expect = R"(
struct tint_array_wrapper {
  arr : array<i32, 4u>,
}

fn f() -> tint_array_wrapper {
  return tint_array_wrapper(array<i32, 4u>(1, 2, 3, 4));
}
)";

    auto got = Run<WrapArraysInStructs>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(WrapArraysInStructsTest, ArrayAlias) {
    auto* src = R"(
type Inner = array<i32, 2>;
type Array = array<Inner, 2>;

fn f() {
  var arr : Array;
  arr = Array();
  arr = Array(Inner(1, 2), Inner(3, 4));
  let vals : Array = Array(Inner(1, 2), Inner(3, 4));
  arr = vals;
  let x = arr[3];
}
)";
    auto* expect = R"(
struct tint_array_wrapper {
  arr : array<i32, 2u>,
}

type Inner = tint_array_wrapper;

struct tint_array_wrapper_1 {
  arr : array<tint_array_wrapper, 2u>,
}

type Array = tint_array_wrapper_1;

fn f() {
  var arr : tint_array_wrapper_1;
  arr = tint_array_wrapper_1(array<tint_array_wrapper, 2u>());
  arr = tint_array_wrapper_1(array<tint_array_wrapper, 2u>(tint_array_wrapper(array<i32, 2u>(1, 2)), tint_array_wrapper(array<i32, 2u>(3, 4))));
  let vals : tint_array_wrapper_1 = tint_array_wrapper_1(array<tint_array_wrapper, 2u>(tint_array_wrapper(array<i32, 2u>(1, 2)), tint_array_wrapper(array<i32, 2u>(3, 4))));
  arr = vals;
  let x = arr.arr[3];
}
)";

    auto got = Run<WrapArraysInStructs>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(WrapArraysInStructsTest, ArrayAlias_OutOfOrder) {
    auto* src = R"(
fn f() {
  var arr : Array;
  arr = Array();
  arr = Array(Inner(1, 2), Inner(3, 4));
  let vals : Array = Array(Inner(1, 2), Inner(3, 4));
  arr = vals;
  let x = arr[3];
}

type Array = array<Inner, 2>;
type Inner = array<i32, 2>;
)";
    auto* expect = R"(
struct tint_array_wrapper_1 {
  arr : array<i32, 2u>,
}

struct tint_array_wrapper {
  arr : array<tint_array_wrapper_1, 2u>,
}

fn f() {
  var arr : tint_array_wrapper;
  arr = tint_array_wrapper(array<tint_array_wrapper_1, 2u>());
  arr = tint_array_wrapper(array<tint_array_wrapper_1, 2u>(tint_array_wrapper_1(array<i32, 2u>(1, 2)), tint_array_wrapper_1(array<i32, 2u>(3, 4))));
  let vals : tint_array_wrapper = tint_array_wrapper(array<tint_array_wrapper_1, 2u>(tint_array_wrapper_1(array<i32, 2u>(1, 2)), tint_array_wrapper_1(array<i32, 2u>(3, 4))));
  arr = vals;
  let x = arr.arr[3];
}

type Array = tint_array_wrapper;

type Inner = tint_array_wrapper_1;
)";

    auto got = Run<WrapArraysInStructs>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(WrapArraysInStructsTest, ArraysInStruct) {
    auto* src = R"(
struct S {
  a : array<i32, 4>,
  b : array<i32, 8>,
  c : array<i32, 4>,
};
)";
    auto* expect = R"(
struct tint_array_wrapper {
  arr : array<i32, 4u>,
}

struct tint_array_wrapper_1 {
  arr : array<i32, 8u>,
}

struct S {
  a : tint_array_wrapper,
  b : tint_array_wrapper_1,
  c : tint_array_wrapper,
}
)";

    auto got = Run<WrapArraysInStructs>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(WrapArraysInStructsTest, ArraysOfArraysInStruct) {
    auto* src = R"(
struct S {
  a : array<i32, 4>,
  b : array<array<i32, 4>, 4>,
  c : array<array<array<i32, 4>, 4>, 4>,
};
)";
    auto* expect = R"(
struct tint_array_wrapper {
  arr : array<i32, 4u>,
}

struct tint_array_wrapper_1 {
  arr : array<tint_array_wrapper, 4u>,
}

struct tint_array_wrapper_2 {
  arr : array<tint_array_wrapper_1, 4u>,
}

struct S {
  a : tint_array_wrapper,
  b : tint_array_wrapper_1,
  c : tint_array_wrapper_2,
}
)";

    auto got = Run<WrapArraysInStructs>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(WrapArraysInStructsTest, AccessArraysOfArraysInStruct) {
    auto* src = R"(
struct S {
  a : array<i32, 4>,
  b : array<array<i32, 4>, 4>,
  c : array<array<array<i32, 4>, 4>, 4>,
};

fn f(s : S) -> i32 {
  return s.a[2] + s.b[1][2] + s.c[3][1][2];
}
)";
    auto* expect = R"(
struct tint_array_wrapper {
  arr : array<i32, 4u>,
}

struct tint_array_wrapper_1 {
  arr : array<tint_array_wrapper, 4u>,
}

struct tint_array_wrapper_2 {
  arr : array<tint_array_wrapper_1, 4u>,
}

struct S {
  a : tint_array_wrapper,
  b : tint_array_wrapper_1,
  c : tint_array_wrapper_2,
}

fn f(s : S) -> i32 {
  return ((s.a.arr[2] + s.b.arr[1].arr[2]) + s.c.arr[3].arr[1].arr[2]);
}
)";

    auto got = Run<WrapArraysInStructs>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(WrapArraysInStructsTest, DeclarationOrder) {
    auto* src = R"(
type T0 = i32;

type T1 = array<i32, 1>;

type T2 = i32;

fn f1(a : array<i32, 2>) {
}

type T3 = i32;

fn f2() {
  var v : array<i32, 3>;
}
)";
    auto* expect = R"(
type T0 = i32;

struct tint_array_wrapper {
  arr : array<i32, 1u>,
}

type T1 = tint_array_wrapper;

type T2 = i32;

struct tint_array_wrapper_1 {
  arr : array<i32, 2u>,
}

fn f1(a : tint_array_wrapper_1) {
}

type T3 = i32;

struct tint_array_wrapper_2 {
  arr : array<i32, 3u>,
}

fn f2() {
  var v : tint_array_wrapper_2;
}
)";

    auto got = Run<WrapArraysInStructs>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(WrapArraysInStructsTest, DeclarationOrder_OutOfOrder) {
    auto* src = R"(
fn f2() {
  var v : array<i32, 3>;
}

type T3 = i32;

fn f1(a : array<i32, 2>) {
}

type T2 = i32;

type T1 = array<i32, 1>;

type T0 = i32;
)";
    auto* expect = R"(
struct tint_array_wrapper {
  arr : array<i32, 3u>,
}

fn f2() {
  var v : tint_array_wrapper;
}

type T3 = i32;

struct tint_array_wrapper_1 {
  arr : array<i32, 2u>,
}

fn f1(a : tint_array_wrapper_1) {
}

type T2 = i32;

struct tint_array_wrapper_2 {
  arr : array<i32, 1u>,
}

type T1 = tint_array_wrapper_2;

type T0 = i32;
)";

    auto got = Run<WrapArraysInStructs>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::transform
