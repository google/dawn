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

#include "src/tint/transform/unshadow.h"

#include "src/tint/transform/test_helper.h"

namespace tint::transform {
namespace {

using UnshadowTest = TransformTest;

TEST_F(UnshadowTest, EmptyModule) {
    auto* src = "";
    auto* expect = "";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, Noop) {
    auto* src = R"(
var<private> a : i32;

let b : i32 = 1;

fn F(c : i32) {
  var d : i32;
  let e : i32 = 1;
  {
    var f : i32;
    let g : i32 = 1;
  }
}
)";

    auto* expect = src;

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, LocalShadowsAlias) {
    auto* src = R"(
type a = i32;

fn X() {
  var a = false;
}

fn Y() {
  let a = true;
}
)";

    auto* expect = R"(
type a = i32;

fn X() {
  var a_1 = false;
}

fn Y() {
  let a_2 = true;
}
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, LocalShadowsAlias_OutOfOrder) {
    auto* src = R"(
fn X() {
  var a = false;
}

fn Y() {
  let a = true;
}

type a = i32;
)";

    auto* expect = R"(
fn X() {
  var a_1 = false;
}

fn Y() {
  let a_2 = true;
}

type a = i32;
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, LocalShadowsStruct) {
    auto* src = R"(
struct a {
  m : i32,
};

fn X() {
  var a = true;
}

fn Y() {
  let a = false;
}
)";

    auto* expect = R"(
struct a {
  m : i32,
}

fn X() {
  var a_1 = true;
}

fn Y() {
  let a_2 = false;
}
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, LocalShadowsStruct_OutOfOrder) {
    auto* src = R"(
fn X() {
  var a = true;
}

fn Y() {
  let a = false;
}

struct a {
  m : i32,
};

)";

    auto* expect = R"(
fn X() {
  var a_1 = true;
}

fn Y() {
  let a_2 = false;
}

struct a {
  m : i32,
}
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, LocalShadowsFunction) {
    auto* src = R"(
fn a() {
  var a = true;
  var b = false;
}

fn b() {
  let a = true;
  let b = false;
}
)";

    auto* expect = R"(
fn a() {
  var a_1 = true;
  var b_1 = false;
}

fn b() {
  let a_2 = true;
  let b_2 = false;
}
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, LocalShadowsFunction_OutOfOrder) {
    auto* src = R"(
fn b() {
  let a = true;
  let b = false;
}

fn a() {
  var a = true;
  var b = false;
}

)";

    auto* expect = R"(
fn b() {
  let a_1 = true;
  let b_1 = false;
}

fn a() {
  var a_2 = true;
  var b_2 = false;
}
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, LocalShadowsGlobalVar) {
    auto* src = R"(
var<private> a : i32;

fn X() {
  var a = (a == 123);
}

fn Y() {
  let a = (a == 321);
}
)";

    auto* expect = R"(
var<private> a : i32;

fn X() {
  var a_1 = (a == 123);
}

fn Y() {
  let a_2 = (a == 321);
}
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, LocalShadowsGlobalVar_OutOfOrder) {
    auto* src = R"(
fn X() {
  var a = (a == 123);
}

fn Y() {
  let a = (a == 321);
}

var<private> a : i32;
)";

    auto* expect = R"(
fn X() {
  var a_1 = (a == 123);
}

fn Y() {
  let a_2 = (a == 321);
}

var<private> a : i32;
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, LocalShadowsGlobalLet) {
    auto* src = R"(
let a : i32 = 1;

fn X() {
  var a = (a == 123);
}

fn Y() {
  let a = (a == 321);
}
)";

    auto* expect = R"(
let a : i32 = 1;

fn X() {
  var a_1 = (a == 123);
}

fn Y() {
  let a_2 = (a == 321);
}
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, LocalShadowsGlobalLet_OutOfOrder) {
    auto* src = R"(
fn X() {
  var a = (a == 123);
}

fn Y() {
  let a = (a == 321);
}

let a : i32 = 1;
)";

    auto* expect = R"(
fn X() {
  var a_1 = (a == 123);
}

fn Y() {
  let a_2 = (a == 321);
}

let a : i32 = 1;
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, LocalShadowsLocalVar) {
    auto* src = R"(
fn X() {
  var a : i32;
  {
    var a = (a == 123);
  }
  {
    let a = (a == 321);
  }
}
)";

    auto* expect = R"(
fn X() {
  var a : i32;
  {
    var a_1 = (a == 123);
  }
  {
    let a_2 = (a == 321);
  }
}
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, LocalShadowsLocalLet) {
    auto* src = R"(
fn X() {
  let a = 1;
  {
    var a = (a == 123);
  }
  {
    let a = (a == 321);
  }
}
)";

    auto* expect = R"(
fn X() {
  let a = 1;
  {
    var a_1 = (a == 123);
  }
  {
    let a_2 = (a == 321);
  }
}
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, LocalShadowsParam) {
    auto* src = R"(
fn F(a : i32) {
  {
    var a = (a == 123);
  }
  {
    let a = (a == 321);
  }
}
)";

    auto* expect = R"(
fn F(a : i32) {
  {
    var a_1 = (a == 123);
  }
  {
    let a_2 = (a == 321);
  }
}
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, ParamShadowsFunction) {
    auto* src = R"(
fn a(a : i32) {
  {
    var a = (a == 123);
  }
  {
    let a = (a == 321);
  }
}
)";

    auto* expect = R"(
fn a(a_1 : i32) {
  {
    var a_2 = (a_1 == 123);
  }
  {
    let a_3 = (a_1 == 321);
  }
}
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, ParamShadowsGlobalVar) {
    auto* src = R"(
var<private> a : i32;

fn F(a : bool) {
}
)";

    auto* expect = R"(
var<private> a : i32;

fn F(a_1 : bool) {
}
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, ParamShadowsGlobalLet) {
    auto* src = R"(
let a : i32 = 1;

fn F(a : bool) {
}
)";

    auto* expect = R"(
let a : i32 = 1;

fn F(a_1 : bool) {
}
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, ParamShadowsGlobalLet_OutOfOrder) {
    auto* src = R"(
fn F(a : bool) {
}

let a : i32 = 1;
)";

    auto* expect = R"(
fn F(a_1 : bool) {
}

let a : i32 = 1;
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, ParamShadowsAlias) {
    auto* src = R"(
type a = i32;

fn F(a : a) {
  {
    var a = (a == 123);
  }
  {
    let a = (a == 321);
  }
}
)";

    auto* expect = R"(
type a = i32;

fn F(a_1 : a) {
  {
    var a_2 = (a_1 == 123);
  }
  {
    let a_3 = (a_1 == 321);
  }
}
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, ParamShadowsAlias_OutOfOrder) {
    auto* src = R"(
fn F(a : a) {
  {
    var a = (a == 123);
  }
  {
    let a = (a == 321);
  }
}

type a = i32;
)";

    auto* expect = R"(
fn F(a_1 : a) {
  {
    var a_2 = (a_1 == 123);
  }
  {
    let a_3 = (a_1 == 321);
  }
}

type a = i32;
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::transform
