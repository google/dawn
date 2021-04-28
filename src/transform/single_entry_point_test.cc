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

#include "src/transform/single_entry_point.h"

#include <utility>

#include "src/transform/test_helper.h"

namespace tint {
namespace transform {
namespace {

using SingleEntryPointTest = TransformTest;

TEST_F(SingleEntryPointTest, Error_MissingTransformData) {
  auto* src = "";

  auto* expect = "error: missing transform data for SingleEntryPoint";

  auto got = Run<SingleEntryPoint>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SingleEntryPointTest, Error_NoEntryPoints) {
  auto* src = "";

  auto* expect = "error: entry point 'main' not found";

  DataMap data;
  data.Add<SingleEntryPoint::Config>("main");
  auto got = Run<SingleEntryPoint>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SingleEntryPointTest, Error_InvalidEntryPoint) {
  auto* src = R"(
[[stage(vertex)]]
fn main() -> [[builtin(position)]] vec4<f32> {
  return vec4<f32>();
}
)";

  auto* expect = "error: entry point '_' not found";

  SingleEntryPoint::Config cfg("_");

  DataMap data;
  data.Add<SingleEntryPoint::Config>(cfg);
  auto got = Run<SingleEntryPoint>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SingleEntryPointTest, Error_NotAnEntryPoint) {
  auto* src = R"(
fn foo() {}

[[stage(fragment)]]
fn main() {}
)";

  auto* expect = "error: entry point 'foo' not found";

  SingleEntryPoint::Config cfg("foo");

  DataMap data;
  data.Add<SingleEntryPoint::Config>(cfg);
  auto got = Run<SingleEntryPoint>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SingleEntryPointTest, SingleEntryPoint) {
  auto* src = R"(
[[stage(compute)]]
fn main() {
}
)";

  SingleEntryPoint::Config cfg("main");

  DataMap data;
  data.Add<SingleEntryPoint::Config>(cfg);
  auto got = Run<SingleEntryPoint>(src, data);

  EXPECT_EQ(src, str(got));
}

TEST_F(SingleEntryPointTest, MultipleEntryPoints) {
  auto* src = R"(
[[stage(vertex)]]
fn vert_main() {
}

[[stage(fragment)]]
fn frag_main() {
}

[[stage(compute)]]
fn comp_main1() {
}

[[stage(compute)]]
fn comp_main2() {
}
)";

  auto* expect = R"(
[[stage(compute)]]
fn comp_main1() {
}
)";

  SingleEntryPoint::Config cfg("comp_main1");

  DataMap data;
  data.Add<SingleEntryPoint::Config>(cfg);
  auto got = Run<SingleEntryPoint>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SingleEntryPointTest, GlobalVariables) {
  auto* src = R"(
var<private> a : f32;

var<private> b : f32;

var<private> c : f32;

var<private> d : f32;

[[stage(vertex)]]
fn vert_main() {
  a = 0.0;
}

[[stage(fragment)]]
fn frag_main() {
  b = 0.0;
}

[[stage(compute)]]
fn comp_main1() {
  c = 0.0;
}

[[stage(compute)]]
fn comp_main2() {
  d = 0.0;
}
)";

  auto* expect = R"(
var<private> c : f32;

[[stage(compute)]]
fn comp_main1() {
  c = 0.0;
}
)";

  SingleEntryPoint::Config cfg("comp_main1");

  DataMap data;
  data.Add<SingleEntryPoint::Config>(cfg);
  auto got = Run<SingleEntryPoint>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SingleEntryPointTest, GlobalConstants) {
  auto* src = R"(
let a : f32 = 1.0;

let b : f32 = 1.0;

let c : f32 = 1.0;

let d : f32 = 1.0;

[[stage(vertex)]]
fn vert_main() {
  let local_a : f32 = a;
}

[[stage(fragment)]]
fn frag_main() {
  let local_b : f32 = b;
}

[[stage(compute)]]
fn comp_main1() {
  let local_c : f32 = c;
}

[[stage(compute)]]
fn comp_main2() {
  let local_d : f32 = d;
}
)";

  auto* expect = R"(
let a : f32 = 1.0;

let b : f32 = 1.0;

let c : f32 = 1.0;

let d : f32 = 1.0;

[[stage(compute)]]
fn comp_main1() {
  let local_c : f32 = c;
}
)";

  SingleEntryPoint::Config cfg("comp_main1");

  DataMap data;
  data.Add<SingleEntryPoint::Config>(cfg);
  auto got = Run<SingleEntryPoint>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SingleEntryPointTest, CalledFunctions) {
  auto* src = R"(
fn inner1() {
}

fn inner2() {
}

fn inner_shared() {
}

fn outer1() {
  inner1();
  inner_shared();
}

fn outer2() {
  inner2();
  inner_shared();
}

[[stage(compute)]]
fn comp_main1() {
  outer1();
}

[[stage(compute)]]
fn comp_main2() {
  outer2();
}
)";

  auto* expect = R"(
fn inner1() {
}

fn inner_shared() {
}

fn outer1() {
  inner1();
  inner_shared();
}

[[stage(compute)]]
fn comp_main1() {
  outer1();
}
)";

  SingleEntryPoint::Config cfg("comp_main1");

  DataMap data;
  data.Add<SingleEntryPoint::Config>(cfg);
  auto got = Run<SingleEntryPoint>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SingleEntryPointTest, GlobalsReferencedByCalledFunctions) {
  auto* src = R"(
var<private> inner1_var : f32;

var<private> inner2_var : f32;

var<private> inner_shared_var : f32;

var<private> outer1_var : f32;

var<private> outer2_var : f32;

fn inner1() {
  inner1_var = 0.0;
}

fn inner2() {
  inner2_var = 0.0;
}

fn inner_shared() {
  inner_shared_var = 0.0;
}

fn outer1() {
  inner1();
  inner_shared();
  outer1_var = 0.0;
}

fn outer2() {
  inner2();
  inner_shared();
  outer2_var = 0.0;
}

[[stage(compute)]]
fn comp_main1() {
  outer1();
}

[[stage(compute)]]
fn comp_main2() {
  outer2();
}
)";

  auto* expect = R"(
var<private> inner1_var : f32;

var<private> inner_shared_var : f32;

var<private> outer1_var : f32;

fn inner1() {
  inner1_var = 0.0;
}

fn inner_shared() {
  inner_shared_var = 0.0;
}

fn outer1() {
  inner1();
  inner_shared();
  outer1_var = 0.0;
}

[[stage(compute)]]
fn comp_main1() {
  outer1();
}
)";

  SingleEntryPoint::Config cfg("comp_main1");

  DataMap data;
  data.Add<SingleEntryPoint::Config>(cfg);
  auto got = Run<SingleEntryPoint>(src, data);

  EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace transform
}  // namespace tint
