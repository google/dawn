// Copyright 2023 The Tint Authors.
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

#include "src/tint/lang/wgsl/ast/transform/fold_trivial_lets.h"

#include "src/tint/lang/wgsl/ast/transform/helper_test.h"

namespace tint::ast::transform {
namespace {

using FoldTrivialLetsTest = TransformTest;

TEST_F(FoldTrivialLetsTest, Fold_IdentInitializer_AssignRHS) {
    auto* src = R"(
fn f() {
  var v = 42;
  let x = v;
  v = (x + 1);
}
)";

    auto* expect = R"(
fn f() {
  var v = 42;
  v = (v + 1);
}
)";

    auto got = Run<FoldTrivialLets>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldTrivialLetsTest, Fold_IdentInitializer_IfCondition) {
    auto* src = R"(
fn f() {
  var v = 42;
  let x = v;
  if (x > 0) {
    v = 0;
  }
}
)";

    auto* expect = R"(
fn f() {
  var v = 42;
  if ((v > 0)) {
    v = 0;
  }
}
)";

    auto got = Run<FoldTrivialLets>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldTrivialLetsTest, NoFold_IdentInitializer_StoreBeforeUse) {
    auto* src = R"(
fn f() {
  var v = 42;
  let x = v;
  v = 0;
  v = (x + 1);
}
)";

    auto* expect = src;

    auto got = Run<FoldTrivialLets>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldTrivialLetsTest, NoFold_IdentInitializer_SideEffectsInUseExpression) {
    auto* src = R"(
var<private> v = 42;

fn g() -> i32 {
  v = 0;
  return 1;
}

fn f() {
  let x = v;
  v = (g() + x);
}
)";

    auto* expect = src;

    auto got = Run<FoldTrivialLets>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldTrivialLetsTest, Fold_IdentInitializer_MultiUse) {
    auto* src = R"(
fn f() {
  var v = 42;
  let x = v;
  v = (x + x);
}
)";

    auto* expect = R"(
fn f() {
  var v = 42;
  v = (v + v);
}
)";

    auto got = Run<FoldTrivialLets>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldTrivialLetsTest, Fold_IdentInitializer_MultiUse_OnlySomeInlineable) {
    auto* src = R"(
fn f() {
  var v = 42;
  let x = v;
  v = (x + x);
  if (x > 0) {
    v = 0;
  }
}
)";

    auto* expect = R"(
fn f() {
  var v = 42;
  let x = v;
  v = (v + v);
  if ((x > 0)) {
    v = 0;
  }
}
)";

    auto got = Run<FoldTrivialLets>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldTrivialLetsTest, Fold_ComplexInitializer_SingleUse) {
    auto* src = R"(
fn f(idx : i32) {
  var v = array<vec4i, 4>();
  let x = v[idx].y;
  v[0].x = (x + 1);
}
)";

    auto* expect = R"(
fn f(idx : i32) {
  var v = array<vec4i, 4>();
  v[0].x = (v[idx].y + 1);
}
)";

    auto got = Run<FoldTrivialLets>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldTrivialLetsTest, NoFold_ComplexInitializer_SingleUseWithSideEffects) {
    auto* src = R"(
var<private> i = 0;

fn bar() -> i32 {
  i++;
  return i;
}

fn f() -> i32 {
  var v = array<vec4i, 4>();
  let x = v[bar()].y;
  return (i + x);
}
)";

    auto* expect = src;

    auto got = Run<FoldTrivialLets>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldTrivialLetsTest, NoFold_ComplexInitializer_MultiUse) {
    auto* src = R"(
fn f(idx : i32) {
  var v = array<vec4i, 4>();
  let x = v[idx].y;
  v[0].x = (x + x);
}
)";

    auto* expect = src;

    auto got = Run<FoldTrivialLets>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldTrivialLetsTest, Fold_ComplexInitializer_SingleUseViaSimpleLet) {
    auto* src = R"(
fn f(a : i32, b : i32, c : i32) -> i32 {
  let x = ((a * b) + c);
  let y = x;
  return y;
}
)";

    auto* expect = R"(
fn f(a : i32, b : i32, c : i32) -> i32 {
  let y = ((a * b) + c);
  return y;
}
)";

    auto got = Run<FoldTrivialLets>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldTrivialLetsTest, Fold_ComplexInitializer_SingleUseViaSimpleLetUsedTwice) {
    auto* src = R"(
fn f(a : i32, b : i32, c : i32) -> i32 {
  let x = (a * b) + c;
  let y = x;
  let z = y + y;
  return z;
}
)";

    auto* expect = R"(
fn f(a : i32, b : i32, c : i32) -> i32 {
  let x = ((a * b) + c);
  let z = (x + x);
  return z;
}
)";

    auto got = Run<FoldTrivialLets>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldTrivialLetsTest, Fold_ComplexInitializer_MultiUseUseDifferentLets) {
    auto* src = R"(
fn f(a : i32, b : i32, c : i32) -> i32 {
  let x = (a * b) + c;
  let y = x;
  let z = x + y;
  return z;
}
)";

    auto* expect = R"(
fn f(a : i32, b : i32, c : i32) -> i32 {
  let x = ((a * b) + c);
  let z = (x + x);
  return z;
}
)";

    auto got = Run<FoldTrivialLets>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::ast::transform
