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

#include "src/tint/transform/unwind_discard_functions.h"
#include "src/tint/transform/promote_side_effects_to_decl.h"
#include "src/tint/transform/test_helper.h"

namespace tint::transform {
namespace {

using UnwindDiscardFunctionsTest = TransformTest;

TEST_F(UnwindDiscardFunctionsTest, EmptyModule) {
    auto* src = "";
    auto* expect = src;

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl, UnwindDiscardFunctions>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnwindDiscardFunctionsTest, ShouldRun_NoDiscardFunc) {
    auto* src = R"(
fn f() {
}
)";

    EXPECT_FALSE(ShouldRun<UnwindDiscardFunctions>(src));
}

TEST_F(UnwindDiscardFunctionsTest, SingleDiscardFunc_NoCall) {
    auto* src = R"(
fn f() {
  discard;
}
)";
    auto* expect = R"(
var<private> tint_discard : bool = false;

fn f() {
  tint_discard = true;
  return;
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl, UnwindDiscardFunctions>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnwindDiscardFunctionsTest, MultipleDiscardFuncs_NoCall) {
    auto* src = R"(
fn f() {
  discard;
  let marker1 = 0;
}

fn g() {
  discard;
  let marker1 = 0;
}
)";
    auto* expect = R"(
var<private> tint_discard : bool = false;

fn f() {
  tint_discard = true;
  return;
  let marker1 = 0;
}

fn g() {
  tint_discard = true;
  return;
  let marker1 = 0;
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl, UnwindDiscardFunctions>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnwindDiscardFunctionsTest, Call_VoidReturn) {
    auto* src = R"(
fn f() {
  discard;
  let marker1 = 0;
}

@stage(fragment)
fn main(@builtin(position) coord_in: vec4<f32>) -> @location(0) vec4<f32> {
  f();
  let marker1 = 0;
  return vec4<f32>();
}
)";
    auto* expect = R"(
var<private> tint_discard : bool = false;

fn f() {
  tint_discard = true;
  return;
  let marker1 = 0;
}

fn tint_discard_func() {
  discard;
}

@stage(fragment)
fn main(@builtin(position) coord_in : vec4<f32>) -> @location(0) vec4<f32> {
  f();
  if (tint_discard) {
    tint_discard_func();
    return vec4<f32>();
  }
  let marker1 = 0;
  return vec4<f32>();
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl, UnwindDiscardFunctions>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnwindDiscardFunctionsTest, Call_NonVoidReturn) {
    auto* src = R"(
struct S {
  x : i32,
  y : i32,
};

fn f() -> S {
  if (true) {
    discard;
  }
  let marker1 = 0;
  var s : S;
  return s;
}

@stage(fragment)
fn main(@builtin(position) coord_in: vec4<f32>) -> @location(0) vec4<f32> {
  let marker1 = 0;
  f();
  let marker2 = 0;
  return vec4<f32>();
}
)";
    auto* expect = R"(
struct S {
  x : i32,
  y : i32,
}

var<private> tint_discard : bool = false;

fn f() -> S {
  if (true) {
    tint_discard = true;
    return S();
  }
  let marker1 = 0;
  var s : S;
  return s;
}

fn tint_discard_func() {
  discard;
}

@stage(fragment)
fn main(@builtin(position) coord_in : vec4<f32>) -> @location(0) vec4<f32> {
  let marker1 = 0;
  f();
  if (tint_discard) {
    tint_discard_func();
    return vec4<f32>();
  }
  let marker2 = 0;
  return vec4<f32>();
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl, UnwindDiscardFunctions>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnwindDiscardFunctionsTest, Call_Nested) {
    auto* src = R"(
fn f() -> i32 {
  let marker1 = 0;
  if (true) {
    discard;
  }
  let marker2 = 0;
  return 0;
}

fn g() -> i32 {
  let marker1 = 0;
  f();
  let marker2 = 0;
  return 0;
}

fn h() -> i32{
  let marker1 = 0;
  g();
  let marker2 = 0;
  return 0;
}

@stage(fragment)
fn main(@builtin(position) coord_in: vec4<f32>) -> @location(0) vec4<f32> {
  let marker1 = 0;
  h();
  let marker2 = 0;
  return vec4<f32>();
}
)";
    auto* expect = R"(
var<private> tint_discard : bool = false;

fn f() -> i32 {
  let marker1 = 0;
  if (true) {
    tint_discard = true;
    return i32();
  }
  let marker2 = 0;
  return 0;
}

fn g() -> i32 {
  let marker1 = 0;
  f();
  if (tint_discard) {
    return i32();
  }
  let marker2 = 0;
  return 0;
}

fn h() -> i32 {
  let marker1 = 0;
  g();
  if (tint_discard) {
    return i32();
  }
  let marker2 = 0;
  return 0;
}

fn tint_discard_func() {
  discard;
}

@stage(fragment)
fn main(@builtin(position) coord_in : vec4<f32>) -> @location(0) vec4<f32> {
  let marker1 = 0;
  h();
  if (tint_discard) {
    tint_discard_func();
    return vec4<f32>();
  }
  let marker2 = 0;
  return vec4<f32>();
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl, UnwindDiscardFunctions>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnwindDiscardFunctionsTest, Call_Multiple) {
    auto* src = R"(
fn f() {
  discard;
  let marker1 = 0;
}

fn g() {
  discard;
  let marker1 = 0;
}

fn h() {
  discard;
  let marker1 = 0;
}

@stage(fragment)
fn main(@builtin(position) coord_in: vec4<f32>) -> @location(0) vec4<f32> {
  let marker1 = 0;
  f();
  let marker2 = 0;
  g();
  let marker3 = 0;
  h();
  let marker4 = 0;
  return vec4<f32>();
}
)";
    auto* expect = R"(
var<private> tint_discard : bool = false;

fn f() {
  tint_discard = true;
  return;
  let marker1 = 0;
}

fn g() {
  tint_discard = true;
  return;
  let marker1 = 0;
}

fn h() {
  tint_discard = true;
  return;
  let marker1 = 0;
}

fn tint_discard_func() {
  discard;
}

@stage(fragment)
fn main(@builtin(position) coord_in : vec4<f32>) -> @location(0) vec4<f32> {
  let marker1 = 0;
  f();
  if (tint_discard) {
    tint_discard_func();
    return vec4<f32>();
  }
  let marker2 = 0;
  g();
  if (tint_discard) {
    tint_discard_func();
    return vec4<f32>();
  }
  let marker3 = 0;
  h();
  if (tint_discard) {
    tint_discard_func();
    return vec4<f32>();
  }
  let marker4 = 0;
  return vec4<f32>();
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl, UnwindDiscardFunctions>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnwindDiscardFunctionsTest, Call_DiscardFuncDeclaredBelow) {
    auto* src = R"(
@stage(fragment)
fn main(@builtin(position) coord_in: vec4<f32>) -> @location(0) vec4<f32> {
  f();
  let marker1 = 0;
  return vec4<f32>();
}

fn f() {
  discard;
  let marker1 = 0;
}
)";
    auto* expect = R"(
fn tint_discard_func() {
  discard;
}

var<private> tint_discard : bool = false;

@stage(fragment)
fn main(@builtin(position) coord_in : vec4<f32>) -> @location(0) vec4<f32> {
  f();
  if (tint_discard) {
    tint_discard_func();
    return vec4<f32>();
  }
  let marker1 = 0;
  return vec4<f32>();
}

fn f() {
  tint_discard = true;
  return;
  let marker1 = 0;
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl, UnwindDiscardFunctions>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnwindDiscardFunctionsTest, If) {
    auto* src = R"(
fn f() -> i32 {
  if (true) {
    discard;
  }
  return 42;
}

@stage(fragment)
fn main(@builtin(position) coord_in: vec4<f32>) -> @location(0) vec4<f32> {
  if (f() == 42) {
    let marker1 = 0;
  }
  return vec4<f32>();
}
)";
    auto* expect = R"(
var<private> tint_discard : bool = false;

fn f() -> i32 {
  if (true) {
    tint_discard = true;
    return i32();
  }
  return 42;
}

fn tint_discard_func() {
  discard;
}

@stage(fragment)
fn main(@builtin(position) coord_in : vec4<f32>) -> @location(0) vec4<f32> {
  let tint_symbol = f();
  if (tint_discard) {
    tint_discard_func();
    return vec4<f32>();
  }
  if ((tint_symbol == 42)) {
    let marker1 = 0;
  }
  return vec4<f32>();
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl, UnwindDiscardFunctions>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnwindDiscardFunctionsTest, ElseIf) {
    auto* src = R"(
fn f() -> i32 {
  if (true) {
    discard;
  }
  return 42;
}

@stage(fragment)
fn main(@builtin(position) coord_in: vec4<f32>) -> @location(0) vec4<f32> {
  if (true) {
    let marker1 = 0;
  } else if (f() == 42) {
    let marker2 = 0;
  } else if (true) {
    let marker3 = 0;
  }
  return vec4<f32>();
}
)";
    auto* expect = R"(
var<private> tint_discard : bool = false;

fn f() -> i32 {
  if (true) {
    tint_discard = true;
    return i32();
  }
  return 42;
}

fn tint_discard_func() {
  discard;
}

@stage(fragment)
fn main(@builtin(position) coord_in : vec4<f32>) -> @location(0) vec4<f32> {
  if (true) {
    let marker1 = 0;
  } else {
    let tint_symbol = f();
    if (tint_discard) {
      tint_discard_func();
      return vec4<f32>();
    }
    if ((tint_symbol == 42)) {
      let marker2 = 0;
    } else if (true) {
      let marker3 = 0;
    }
  }
  return vec4<f32>();
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl, UnwindDiscardFunctions>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnwindDiscardFunctionsTest, ForLoop_Init_Assignment) {
    auto* src = R"(
fn f() -> i32 {
  if (true) {
    discard;
  }
  return 42;
}

@stage(fragment)
fn main(@builtin(position) coord_in: vec4<f32>) -> @location(0) vec4<f32> {
  let marker1 = 0;
  var a = 0;
  for (a = f(); ; ) {
    let marker2 = 0;
    break;
  }
  return vec4<f32>();
}
)";
    auto* expect = R"(
var<private> tint_discard : bool = false;

fn f() -> i32 {
  if (true) {
    tint_discard = true;
    return i32();
  }
  return 42;
}

fn tint_discard_func() {
  discard;
}

@stage(fragment)
fn main(@builtin(position) coord_in : vec4<f32>) -> @location(0) vec4<f32> {
  let marker1 = 0;
  var a = 0;
  var tint_symbol = f();
  if (tint_discard) {
    tint_discard_func();
    return vec4<f32>();
  }
  for(a = tint_symbol; ; ) {
    let marker2 = 0;
    break;
  }
  return vec4<f32>();
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl, UnwindDiscardFunctions>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnwindDiscardFunctionsTest, ForLoop_Init_Call) {
    auto* src = R"(
fn f() -> i32 {
  if (true) {
    discard;
  }
  return 42;
}

@stage(fragment)
fn main(@builtin(position) coord_in: vec4<f32>) -> @location(0) vec4<f32> {
  let marker1 = 0;
  for (f(); ; ) {
    let marker2 = 0;
    break;
  }
  return vec4<f32>();
}
)";
    auto* expect = R"(
var<private> tint_discard : bool = false;

fn f() -> i32 {
  if (true) {
    tint_discard = true;
    return i32();
  }
  return 42;
}

fn tint_discard_func() {
  discard;
}

@stage(fragment)
fn main(@builtin(position) coord_in : vec4<f32>) -> @location(0) vec4<f32> {
  let marker1 = 0;
  var tint_symbol = f();
  if (tint_discard) {
    tint_discard_func();
    return vec4<f32>();
  }
  for(_ = tint_symbol; ; ) {
    let marker2 = 0;
    break;
  }
  return vec4<f32>();
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl, UnwindDiscardFunctions>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnwindDiscardFunctionsTest, ForLoop_Init_VariableDecl) {
    auto* src = R"(
fn f() -> i32 {
  if (true) {
    discard;
  }
  return 42;
}

@stage(fragment)
fn main(@builtin(position) coord_in: vec4<f32>) -> @location(0) vec4<f32> {
  let marker1 = 0;
  for (let i = f(); ; ) {
    let marker2 = 0;
    break;
  }
  return vec4<f32>();
}
)";
    auto* expect = R"(
var<private> tint_discard : bool = false;

fn f() -> i32 {
  if (true) {
    tint_discard = true;
    return i32();
  }
  return 42;
}

fn tint_discard_func() {
  discard;
}

@stage(fragment)
fn main(@builtin(position) coord_in : vec4<f32>) -> @location(0) vec4<f32> {
  let marker1 = 0;
  var tint_symbol = f();
  if (tint_discard) {
    tint_discard_func();
    return vec4<f32>();
  }
  for(let i = tint_symbol; ; ) {
    let marker2 = 0;
    break;
  }
  return vec4<f32>();
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl, UnwindDiscardFunctions>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnwindDiscardFunctionsTest, ForLoop_Cond) {
    auto* src = R"(
fn f() -> i32 {
  if (true) {
    discard;
  }
  return 42;
}

@stage(fragment)
fn main(@builtin(position) coord_in: vec4<f32>) -> @location(0) vec4<f32> {
  let marker1 = 0;
  for (; f() == 42; ) {
    let marker2 = 0;
    break;
  }
  return vec4<f32>();
}
)";
    auto* expect = R"(
var<private> tint_discard : bool = false;

fn f() -> i32 {
  if (true) {
    tint_discard = true;
    return i32();
  }
  return 42;
}

fn tint_discard_func() {
  discard;
}

@stage(fragment)
fn main(@builtin(position) coord_in : vec4<f32>) -> @location(0) vec4<f32> {
  let marker1 = 0;
  loop {
    let tint_symbol = f();
    if (tint_discard) {
      tint_discard_func();
      return vec4<f32>();
    }
    if (!((tint_symbol == 42))) {
      break;
    }
    {
      let marker2 = 0;
      break;
    }
  }
  return vec4<f32>();
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl, UnwindDiscardFunctions>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnwindDiscardFunctionsTest, ForLoop_Cont) {
    auto* src = R"(
fn f() -> i32 {
  if (true) {
    discard;
  }
  return 42;
}

@stage(fragment)
fn main(@builtin(position) coord_in: vec4<f32>) -> @location(0) vec4<f32> {
  let marker1 = 0;
  for (; ; f()) {
    let marker2 = 0;
    break;
  }
  return vec4<f32>();
}
)";
    auto* expect =
        R"(test:12:12 error: cannot call a function that may discard inside a continuing block
  for (; ; f()) {
           ^
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl, UnwindDiscardFunctions>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnwindDiscardFunctionsTest, Switch) {
    auto* src = R"(
fn f() -> i32 {
  if (true) {
    discard;
  }
  return 42;
}

@stage(fragment)
fn main(@builtin(position) coord_in: vec4<f32>) -> @location(0) vec4<f32> {
  switch (f()) {
    case 0: {
      let marker1 = 0;
    }
    case 1: {
      let marker2 = 0;
    }
    case 42: {
      let marker3 = 0;
    }
    default: {
      let marker4 = 0;
    }
  }
  return vec4<f32>();
}
)";
    auto* expect = R"(
var<private> tint_discard : bool = false;

fn f() -> i32 {
  if (true) {
    tint_discard = true;
    return i32();
  }
  return 42;
}

fn tint_discard_func() {
  discard;
}

@stage(fragment)
fn main(@builtin(position) coord_in : vec4<f32>) -> @location(0) vec4<f32> {
  var tint_symbol = f();
  if (tint_discard) {
    tint_discard_func();
    return vec4<f32>();
  }
  switch(tint_symbol) {
    case 0: {
      let marker1 = 0;
    }
    case 1: {
      let marker2 = 0;
    }
    case 42: {
      let marker3 = 0;
    }
    default: {
      let marker4 = 0;
    }
  }
  return vec4<f32>();
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl, UnwindDiscardFunctions>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnwindDiscardFunctionsTest, Return) {
    auto* src = R"(
struct S {
  x : i32,
  y : i32,
};

fn f() -> S {
  if (true) {
    discard;
  }
  var s : S;
  return s;
}

fn g() -> S {
  return f();
}

@stage(fragment)
fn main(@builtin(position) coord_in: vec4<f32>) -> @location(0) vec4<f32> {
  let marker1 = 0;
  g();
  return vec4<f32>();
}
)";
    auto* expect = R"(
struct S {
  x : i32,
  y : i32,
}

var<private> tint_discard : bool = false;

fn f() -> S {
  if (true) {
    tint_discard = true;
    return S();
  }
  var s : S;
  return s;
}

fn g() -> S {
  var tint_symbol = f();
  if (tint_discard) {
    return S();
  }
  return tint_symbol;
}

fn tint_discard_func() {
  discard;
}

@stage(fragment)
fn main(@builtin(position) coord_in : vec4<f32>) -> @location(0) vec4<f32> {
  let marker1 = 0;
  g();
  if (tint_discard) {
    tint_discard_func();
    return vec4<f32>();
  }
  return vec4<f32>();
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl, UnwindDiscardFunctions>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnwindDiscardFunctionsTest, VariableDecl) {
    auto* src = R"(
fn f() -> i32 {
  if (true) {
    discard;
  }
  return 42;
}

@stage(fragment)
fn main(@builtin(position) coord_in: vec4<f32>) -> @location(0) vec4<f32> {
  var a = f();
  let marker1 = 0;
  return vec4<f32>();
}
)";
    auto* expect = R"(
var<private> tint_discard : bool = false;

fn f() -> i32 {
  if (true) {
    tint_discard = true;
    return i32();
  }
  return 42;
}

fn tint_discard_func() {
  discard;
}

@stage(fragment)
fn main(@builtin(position) coord_in : vec4<f32>) -> @location(0) vec4<f32> {
  var a = f();
  if (tint_discard) {
    tint_discard_func();
    return vec4<f32>();
  }
  let marker1 = 0;
  return vec4<f32>();
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl, UnwindDiscardFunctions>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnwindDiscardFunctionsTest, Assignment_RightDiscard) {
    auto* src = R"(
fn f() -> i32 {
  if (true) {
    discard;
  }
  return 42;
}

@stage(fragment)
fn main(@builtin(position) coord_in: vec4<f32>) -> @location(0) vec4<f32> {
  var a : i32;
  a = f();
  let marker1 = 0;
  return vec4<f32>();
}
)";
    auto* expect = R"(
var<private> tint_discard : bool = false;

fn f() -> i32 {
  if (true) {
    tint_discard = true;
    return i32();
  }
  return 42;
}

fn tint_discard_func() {
  discard;
}

@stage(fragment)
fn main(@builtin(position) coord_in : vec4<f32>) -> @location(0) vec4<f32> {
  var a : i32;
  a = f();
  if (tint_discard) {
    tint_discard_func();
    return vec4<f32>();
  }
  let marker1 = 0;
  return vec4<f32>();
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl, UnwindDiscardFunctions>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnwindDiscardFunctionsTest, Assignment_LeftDiscard) {
    auto* src = R"(
fn f() -> i32 {
  if (true) {
    discard;
  }
  return 0;
}

@stage(fragment)
fn main(@builtin(position) coord_in: vec4<f32>) -> @location(0) vec4<f32> {
  var b = array<i32, 10>();
  b[f()] = 10;
  let marker1 = 0;
  return vec4<f32>();
}
)";
    auto* expect = R"(
var<private> tint_discard : bool = false;

fn f() -> i32 {
  if (true) {
    tint_discard = true;
    return i32();
  }
  return 0;
}

fn tint_discard_func() {
  discard;
}

@stage(fragment)
fn main(@builtin(position) coord_in : vec4<f32>) -> @location(0) vec4<f32> {
  var b = array<i32, 10>();
  let tint_symbol = f();
  if (tint_discard) {
    tint_discard_func();
    return vec4<f32>();
  }
  b[tint_symbol] = 10;
  let marker1 = 0;
  return vec4<f32>();
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl, UnwindDiscardFunctions>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnwindDiscardFunctionsTest, Assignment_BothDiscard) {
    auto* src = R"(
fn f() -> i32 {
  if (true) {
    discard;
  }
  return 0;
}

fn g() -> i32 {
  if (true) {
    discard;
  }
  return 0;
}

@stage(fragment)
fn main(@builtin(position) coord_in: vec4<f32>) -> @location(0) vec4<f32> {
  var b = array<i32, 10>();
  b[f()] = g();
  let marker1 = 0;
  return vec4<f32>();
}
)";
    auto* expect = R"(
var<private> tint_discard : bool = false;

fn f() -> i32 {
  if (true) {
    tint_discard = true;
    return i32();
  }
  return 0;
}

fn g() -> i32 {
  if (true) {
    tint_discard = true;
    return i32();
  }
  return 0;
}

fn tint_discard_func() {
  discard;
}

@stage(fragment)
fn main(@builtin(position) coord_in : vec4<f32>) -> @location(0) vec4<f32> {
  var b = array<i32, 10>();
  let tint_symbol = g();
  if (tint_discard) {
    tint_discard_func();
    return vec4<f32>();
  }
  let tint_symbol_1 = f();
  if (tint_discard) {
    tint_discard_func();
    return vec4<f32>();
  }
  b[tint_symbol_1] = tint_symbol;
  let marker1 = 0;
  return vec4<f32>();
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl, UnwindDiscardFunctions>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnwindDiscardFunctionsTest, Binary_Arith_MultipleDiscardFuncs) {
    auto* src = R"(
fn f() -> i32 {
  if (true) {
    discard;
  }
  return 0;
}

fn g() -> i32 {
  if (true) {
    discard;
  }
  return 0;
}

fn h() -> i32{
  if (true) {
    discard;
  }
  return 0;
}

@stage(fragment)
fn main(@builtin(position) coord_in: vec4<f32>) -> @location(0) vec4<f32> {
  if ((f() + g() + h()) == 0) {
    let marker1 = 0;
  }
  return vec4<f32>();
}
)";
    auto* expect = R"(
var<private> tint_discard : bool = false;

fn f() -> i32 {
  if (true) {
    tint_discard = true;
    return i32();
  }
  return 0;
}

fn g() -> i32 {
  if (true) {
    tint_discard = true;
    return i32();
  }
  return 0;
}

fn h() -> i32 {
  if (true) {
    tint_discard = true;
    return i32();
  }
  return 0;
}

fn tint_discard_func() {
  discard;
}

@stage(fragment)
fn main(@builtin(position) coord_in : vec4<f32>) -> @location(0) vec4<f32> {
  let tint_symbol = f();
  if (tint_discard) {
    tint_discard_func();
    return vec4<f32>();
  }
  let tint_symbol_1 = g();
  if (tint_discard) {
    tint_discard_func();
    return vec4<f32>();
  }
  let tint_symbol_2 = h();
  if (tint_discard) {
    tint_discard_func();
    return vec4<f32>();
  }
  if ((((tint_symbol + tint_symbol_1) + tint_symbol_2) == 0)) {
    let marker1 = 0;
  }
  return vec4<f32>();
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl, UnwindDiscardFunctions>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnwindDiscardFunctionsTest, Binary_Logical_MultipleDiscardFuncs) {
    auto* src = R"(
fn f() -> i32 {
  if (true) {
    discard;
  }
  return 0;
}

fn g() -> i32 {
  if (true) {
    discard;
  }
  return 0;
}

fn h() -> i32{
  if (true) {
    discard;
  }
  return 0;
}

@stage(fragment)
fn main(@builtin(position) coord_in: vec4<f32>) -> @location(0) vec4<f32> {
  if (f() == 1 && g() == 2 && h() == 3) {
    let marker1 = 0;
  }
  return vec4<f32>();
}
)";
    auto* expect = R"(
var<private> tint_discard : bool = false;

fn f() -> i32 {
  if (true) {
    tint_discard = true;
    return i32();
  }
  return 0;
}

fn g() -> i32 {
  if (true) {
    tint_discard = true;
    return i32();
  }
  return 0;
}

fn h() -> i32 {
  if (true) {
    tint_discard = true;
    return i32();
  }
  return 0;
}

fn tint_discard_func() {
  discard;
}

@stage(fragment)
fn main(@builtin(position) coord_in : vec4<f32>) -> @location(0) vec4<f32> {
  let tint_symbol_2 = f();
  if (tint_discard) {
    tint_discard_func();
    return vec4<f32>();
  }
  var tint_symbol_1 = (tint_symbol_2 == 1);
  if (tint_symbol_1) {
    let tint_symbol_3 = g();
    if (tint_discard) {
      tint_discard_func();
      return vec4<f32>();
    }
    tint_symbol_1 = (tint_symbol_3 == 2);
  }
  var tint_symbol = tint_symbol_1;
  if (tint_symbol) {
    let tint_symbol_4 = h();
    if (tint_discard) {
      tint_discard_func();
      return vec4<f32>();
    }
    tint_symbol = (tint_symbol_4 == 3);
  }
  if (tint_symbol) {
    let marker1 = 0;
  }
  return vec4<f32>();
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl, UnwindDiscardFunctions>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnwindDiscardFunctionsTest, EnsureNoSymbolCollision) {
    auto* src = R"(
var<private> tint_discard_func : i32;
var<private> tint_discard : i32;

fn f() {
  discard;
  let marker1 = 0;
}

@stage(fragment)
fn main(@builtin(position) coord_in: vec4<f32>) -> @location(0) vec4<f32> {
  f();
  let marker1 = 0;
  return vec4<f32>();
}
)";
    auto* expect = R"(
var<private> tint_discard_func : i32;

var<private> tint_discard : i32;

var<private> tint_discard_1 : bool = false;

fn f() {
  tint_discard_1 = true;
  return;
  let marker1 = 0;
}

fn tint_discard_func_1() {
  discard;
}

@stage(fragment)
fn main(@builtin(position) coord_in : vec4<f32>) -> @location(0) vec4<f32> {
  f();
  if (tint_discard_1) {
    tint_discard_func_1();
    return vec4<f32>();
  }
  let marker1 = 0;
  return vec4<f32>();
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl, UnwindDiscardFunctions>(src, data);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::transform
