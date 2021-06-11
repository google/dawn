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

#include "src/transform/msl.h"

#include "src/transform/test_helper.h"

namespace tint {
namespace transform {
namespace {

using MslTest = TransformTest;

TEST_F(MslTest, HandlePrivateAndWorkgroupVariables_Basic) {
  auto* src = R"(
var<private> p : f32;
var<workgroup> w : f32;

[[stage(compute)]]
fn main() {
  w = p;
}
)";

  auto* expect = R"(
[[stage(compute)]]
fn main() {
  [[internal(disable_validation__function_var_storage_class)]] var<workgroup> tint_symbol : f32;
  [[internal(disable_validation__function_var_storage_class)]] var<private> tint_symbol_1 : f32;
  tint_symbol = tint_symbol_1;
}
)";

  auto got = Run<Msl>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(MslTest, HandlePrivateAndWorkgroupVariables_FunctionCalls) {
  auto* src = R"(
var<private> p : f32;
var<workgroup> w : f32;

fn no_uses() {
}

fn bar(a : f32, b : f32) {
  p = a;
  w = b;
}

fn foo(a : f32) {
  let b : f32 = 2.0;
  bar(a, b);
  no_uses();
}

[[stage(compute)]]
fn main() {
  foo(1.0);
}
)";

  auto* expect = R"(
fn no_uses() {
}

fn bar(a : f32, b : f32, tint_symbol : ptr<private, f32>, tint_symbol_1 : ptr<workgroup, f32>) {
  *(tint_symbol) = a;
  *(tint_symbol_1) = b;
}

fn foo(a : f32, tint_symbol_2 : ptr<private, f32>, tint_symbol_3 : ptr<workgroup, f32>) {
  let b : f32 = 2.0;
  bar(a, b, tint_symbol_2, tint_symbol_3);
  no_uses();
}

[[stage(compute)]]
fn main() {
  [[internal(disable_validation__function_var_storage_class)]] var<private> tint_symbol_4 : f32;
  [[internal(disable_validation__function_var_storage_class)]] var<workgroup> tint_symbol_5 : f32;
  foo(1.0, &(tint_symbol_4), &(tint_symbol_5));
}
)";

  auto got = Run<Msl>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(MslTest, HandlePrivateAndWorkgroupVariables_Constructors) {
  auto* src = R"(
var<private> a : f32 = 1.0;
var<private> b : f32 = f32();

[[stage(compute)]]
fn main() {
  let x : f32 = a + b;
}
)";

  auto* expect = R"(
[[stage(compute)]]
fn main() {
  [[internal(disable_validation__function_var_storage_class)]] var<private> tint_symbol : f32 = 1.0;
  [[internal(disable_validation__function_var_storage_class)]] var<private> tint_symbol_1 : f32 = f32();
  let x : f32 = (tint_symbol + tint_symbol_1);
}
)";

  auto got = Run<Msl>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(MslTest, HandlePrivateAndWorkgroupVariables_Pointers) {
  auto* src = R"(
var<private> p : f32;
var<workgroup> w : f32;

[[stage(compute)]]
fn main() {
  let p_ptr : ptr<private, f32> = &p;
  let w_ptr : ptr<workgroup, f32> = &w;
  let x : f32 = *p_ptr + *w_ptr;
  *p_ptr = x;
}
)";

  auto* expect = R"(
[[stage(compute)]]
fn main() {
  [[internal(disable_validation__function_var_storage_class)]] var<private> tint_symbol : f32;
  [[internal(disable_validation__function_var_storage_class)]] var<workgroup> tint_symbol_1 : f32;
  let p_ptr : ptr<private, f32> = &(tint_symbol);
  let w_ptr : ptr<workgroup, f32> = &(tint_symbol_1);
  let x : f32 = (*(p_ptr) + *(w_ptr));
  *(p_ptr) = x;
}
)";

  auto got = Run<Msl>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(MslTest, HandlePrivateAndWorkgroupVariables_UnusedVariables) {
  auto* src = R"(
var<private> p : f32;
var<workgroup> w : f32;

[[stage(compute)]]
fn main() {
}
)";

  auto* expect = R"(
[[stage(compute)]]
fn main() {
}
)";

  auto got = Run<Msl>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(MslTest, HandlePrivateAndWorkgroupVariables_OtherVariables) {
  auto* src = R"(
[[block]]
struct S {
};

[[group(0), binding(0)]]
var<uniform> u : S;

[[stage(compute)]]
fn main() {
}
)";

  auto* expect = R"(
[[block]]
struct S {
};

[[group(0), binding(0)]] var<uniform> u : S;

[[stage(compute)]]
fn main() {
}
)";

  auto got = Run<Msl>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(MslTest, HandlePrivateAndWorkgroupVariables_EmtpyModule) {
  auto* src = "";

  auto got = Run<Msl>(src);

  EXPECT_EQ(src, str(got));
}

}  // namespace
}  // namespace transform
}  // namespace tint
