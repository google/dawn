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

#include "src/tint/transform/fold_constants.h"

#include <memory>
#include <utility>
#include <vector>

#include "src/tint/transform/test_helper.h"

namespace tint::transform {
namespace {

using FoldConstantsTest = TransformTest;

TEST_F(FoldConstantsTest, Module_Scalar_NoConversion) {
    auto* src = R"(
var<private> a : i32 = i32(123);
var<private> b : u32 = u32(123u);
var<private> c : f32 = f32(123.0);
var<private> d : bool = bool(true);

fn f() {
}
)";

    auto* expect = R"(
var<private> a : i32 = 123;

var<private> b : u32 = 123u;

var<private> c : f32 = 123.0;

var<private> d : bool = true;

fn f() {
}
)";

    auto got = Run<FoldConstants>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldConstantsTest, Module_Scalar_Conversion) {
    auto* src = R"(
var<private> a : i32 = i32(123.0);
var<private> b : u32 = u32(123);
var<private> c : f32 = f32(123u);
var<private> d : bool = bool(123);

fn f() {
}
)";

    auto* expect = R"(
var<private> a : i32 = 123;

var<private> b : u32 = 123u;

var<private> c : f32 = 123.0;

var<private> d : bool = true;

fn f() {
}
)";

    auto got = Run<FoldConstants>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldConstantsTest, Module_Scalar_MultipleConversions) {
    auto* src = R"(
var<private> a : i32 = i32(u32(f32(u32(i32(123.0)))));
var<private> b : u32 = u32(i32(f32(i32(u32(123)))));
var<private> c : f32 = f32(u32(i32(u32(f32(123u)))));
var<private> d : bool = bool(i32(f32(i32(u32(123)))));

fn f() {
}
)";

    auto* expect = R"(
var<private> a : i32 = 123;

var<private> b : u32 = 123u;

var<private> c : f32 = 123.0;

var<private> d : bool = true;

fn f() {
}
)";

    auto got = Run<FoldConstants>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldConstantsTest, Module_Vector_NoConversion) {
    auto* src = R"(
var<private> a : vec3<i32> = vec3<i32>(123);
var<private> b : vec3<u32> = vec3<u32>(123u);
var<private> c : vec3<f32> = vec3<f32>(123.0);
var<private> d : vec3<bool> = vec3<bool>(true);

fn f() {
}
)";

    auto* expect = R"(
var<private> a : vec3<i32> = vec3<i32>(123);

var<private> b : vec3<u32> = vec3<u32>(123u);

var<private> c : vec3<f32> = vec3<f32>(123.0);

var<private> d : vec3<bool> = vec3<bool>(true);

fn f() {
}
)";

    auto got = Run<FoldConstants>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldConstantsTest, Module_Vector_Conversion) {
    auto* src = R"(
var<private> a : vec3<i32> = vec3<i32>(vec3<f32>(123.0));
var<private> b : vec3<u32> = vec3<u32>(vec3<i32>(123));
var<private> c : vec3<f32> = vec3<f32>(vec3<u32>(123u));
var<private> d : vec3<bool> = vec3<bool>(vec3<i32>(123));

fn f() {
}
)";

    auto* expect = R"(
var<private> a : vec3<i32> = vec3<i32>(123);

var<private> b : vec3<u32> = vec3<u32>(123u);

var<private> c : vec3<f32> = vec3<f32>(123.0);

var<private> d : vec3<bool> = vec3<bool>(true);

fn f() {
}
)";

    auto got = Run<FoldConstants>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldConstantsTest, Module_Vector_MultipleConversions) {
    auto* src = R"(
var<private> a : vec3<i32> = vec3<i32>(vec3<u32>(vec3<f32>(vec3<u32>(u32(123.0)))));
var<private> b : vec3<u32> = vec3<u32>(vec3<i32>(vec3<f32>(vec3<i32>(i32(123)))));
var<private> c : vec3<f32> = vec3<f32>(vec3<u32>(vec3<i32>(vec3<u32>(u32(123u)))));
var<private> d : vec3<bool> = vec3<bool>(vec3<i32>(vec3<f32>(vec3<i32>(i32(123)))));

fn f() {
}
)";

    auto* expect = R"(
var<private> a : vec3<i32> = vec3<i32>(123);

var<private> b : vec3<u32> = vec3<u32>(123u);

var<private> c : vec3<f32> = vec3<f32>(123.0);

var<private> d : vec3<bool> = vec3<bool>(true);

fn f() {
}
)";

    auto got = Run<FoldConstants>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldConstantsTest, Module_Vector_MixedSizeConversions) {
    auto* src = R"(
var<private> a : vec4<i32> = vec4<i32>(vec3<i32>(vec3<u32>(1u, 2u, 3u)), 4);
var<private> b : vec4<i32> = vec4<i32>(vec2<i32>(vec2<u32>(1u, 2u)), vec2<i32>(4, 5));
var<private> c : vec4<i32> = vec4<i32>(1, vec2<i32>(vec2<f32>(2.0, 3.0)), 4);
var<private> d : vec4<i32> = vec4<i32>(1, 2, vec2<i32>(vec2<f32>(3.0, 4.0)));
var<private> e : vec4<bool> = vec4<bool>(false, bool(f32(1.0)), vec2<bool>(vec2<i32>(0, i32(4u))));

fn f() {
}
)";

    auto* expect = R"(
var<private> a : vec4<i32> = vec4<i32>(1, 2, 3, 4);

var<private> b : vec4<i32> = vec4<i32>(1, 2, 4, 5);

var<private> c : vec4<i32> = vec4<i32>(1, 2, 3, 4);

var<private> d : vec4<i32> = vec4<i32>(1, 2, 3, 4);

var<private> e : vec4<bool> = vec4<bool>(false, true, false, true);

fn f() {
}
)";

    auto got = Run<FoldConstants>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldConstantsTest, Function_Scalar_NoConversion) {
    auto* src = R"(
fn f() {
  var a : i32 = i32(123);
  var b : u32 = u32(123u);
  var c : f32 = f32(123.0);
  var d : bool = bool(true);
}
)";

    auto* expect = R"(
fn f() {
  var a : i32 = 123;
  var b : u32 = 123u;
  var c : f32 = 123.0;
  var d : bool = true;
}
)";

    auto got = Run<FoldConstants>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldConstantsTest, Function_Scalar_Conversion) {
    auto* src = R"(
fn f() {
  var a : i32 = i32(123.0);
  var b : u32 = u32(123);
  var c : f32 = f32(123u);
  var d : bool = bool(123);
}
)";

    auto* expect = R"(
fn f() {
  var a : i32 = 123;
  var b : u32 = 123u;
  var c : f32 = 123.0;
  var d : bool = true;
}
)";

    auto got = Run<FoldConstants>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldConstantsTest, Function_Scalar_MultipleConversions) {
    auto* src = R"(
fn f() {
  var a : i32 = i32(u32(f32(u32(i32(123.0)))));
  var b : u32 = u32(i32(f32(i32(u32(123)))));
  var c : f32 = f32(u32(i32(u32(f32(123u)))));
  var d : bool = bool(i32(f32(i32(u32(123)))));
}
)";

    auto* expect = R"(
fn f() {
  var a : i32 = 123;
  var b : u32 = 123u;
  var c : f32 = 123.0;
  var d : bool = true;
}
)";

    auto got = Run<FoldConstants>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldConstantsTest, Function_Vector_NoConversion) {
    auto* src = R"(
fn f() {
  var a : vec3<i32> = vec3<i32>(123);
  var b : vec3<u32> = vec3<u32>(123u);
  var c : vec3<f32> = vec3<f32>(123.0);
  var d : vec3<bool> = vec3<bool>(true);
}
)";

    auto* expect = R"(
fn f() {
  var a : vec3<i32> = vec3<i32>(123);
  var b : vec3<u32> = vec3<u32>(123u);
  var c : vec3<f32> = vec3<f32>(123.0);
  var d : vec3<bool> = vec3<bool>(true);
}
)";

    auto got = Run<FoldConstants>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldConstantsTest, Function_Vector_Conversion) {
    auto* src = R"(
fn f() {
  var a : vec3<i32> = vec3<i32>(vec3<f32>(123.0));
  var b : vec3<u32> = vec3<u32>(vec3<i32>(123));
  var c : vec3<f32> = vec3<f32>(vec3<u32>(123u));
  var d : vec3<bool> = vec3<bool>(vec3<i32>(123));
}
)";

    auto* expect = R"(
fn f() {
  var a : vec3<i32> = vec3<i32>(123);
  var b : vec3<u32> = vec3<u32>(123u);
  var c : vec3<f32> = vec3<f32>(123.0);
  var d : vec3<bool> = vec3<bool>(true);
}
)";

    auto got = Run<FoldConstants>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldConstantsTest, Function_Vector_MultipleConversions) {
    auto* src = R"(
fn f() {
  var a : vec3<i32> = vec3<i32>(vec3<u32>(vec3<f32>(vec3<u32>(u32(123.0)))));
  var b : vec3<u32> = vec3<u32>(vec3<i32>(vec3<f32>(vec3<i32>(i32(123)))));
  var c : vec3<f32> = vec3<f32>(vec3<u32>(vec3<i32>(vec3<u32>(u32(123u)))));
  var d : vec3<bool> = vec3<bool>(vec3<i32>(vec3<f32>(vec3<i32>(i32(123)))));
}
)";

    auto* expect = R"(
fn f() {
  var a : vec3<i32> = vec3<i32>(123);
  var b : vec3<u32> = vec3<u32>(123u);
  var c : vec3<f32> = vec3<f32>(123.0);
  var d : vec3<bool> = vec3<bool>(true);
}
)";

    auto got = Run<FoldConstants>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldConstantsTest, Function_Vector_MixedSizeConversions) {
    auto* src = R"(
fn f() {
  var a : vec4<i32> = vec4<i32>(vec3<i32>(vec3<u32>(1u, 2u, 3u)), 4);
  var b : vec4<i32> = vec4<i32>(vec2<i32>(vec2<u32>(1u, 2u)), vec2<i32>(4, 5));
  var c : vec4<i32> = vec4<i32>(1, vec2<i32>(vec2<f32>(2.0, 3.0)), 4);
  var d : vec4<i32> = vec4<i32>(1, 2, vec2<i32>(vec2<f32>(3.0, 4.0)));
  var e : vec4<bool> = vec4<bool>(false, bool(f32(1.0)), vec2<bool>(vec2<i32>(0, i32(4u))));
}
)";

    auto* expect = R"(
fn f() {
  var a : vec4<i32> = vec4<i32>(1, 2, 3, 4);
  var b : vec4<i32> = vec4<i32>(1, 2, 4, 5);
  var c : vec4<i32> = vec4<i32>(1, 2, 3, 4);
  var d : vec4<i32> = vec4<i32>(1, 2, 3, 4);
  var e : vec4<bool> = vec4<bool>(false, true, false, true);
}
)";

    auto got = Run<FoldConstants>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldConstantsTest, Function_Vector_ConstantWithNonConstant) {
    auto* src = R"(
fn f() {
  var a : f32 = f32();
  var b : vec2<f32> = vec2<f32>(f32(i32(1)), a);
}
)";

    auto* expect = R"(
fn f() {
  var a : f32 = f32();
  var b : vec2<f32> = vec2<f32>(1.0, a);
}
)";

    auto got = Run<FoldConstants>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::transform
