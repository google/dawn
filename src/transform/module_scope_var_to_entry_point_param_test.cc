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

#include "src/transform/module_scope_var_to_entry_point_param.h"

#include <utility>

#include "src/transform/test_helper.h"

namespace tint {
namespace transform {
namespace {

using ModuleScopeVarToEntryPointParamTest = TransformTest;

TEST_F(ModuleScopeVarToEntryPointParamTest, Basic) {
  auto* src = R"(
var<private> p : f32;
var<workgroup> w : f32;

[[stage(compute), workgroup_size(1)]]
fn main() {
  w = p;
}
)";

  auto* expect = R"(
[[stage(compute), workgroup_size(1)]]
fn main() {
  [[internal(disable_validation__ignore_storage_class)]] var<workgroup> tint_symbol : f32;
  [[internal(disable_validation__ignore_storage_class)]] var<private> tint_symbol_1 : f32;
  tint_symbol = tint_symbol_1;
}
)";

  auto got = Run<ModuleScopeVarToEntryPointParam>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, FunctionCalls) {
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

[[stage(compute), workgroup_size(1)]]
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

[[stage(compute), workgroup_size(1)]]
fn main() {
  [[internal(disable_validation__ignore_storage_class)]] var<private> tint_symbol_4 : f32;
  [[internal(disable_validation__ignore_storage_class)]] var<workgroup> tint_symbol_5 : f32;
  foo(1.0, &(tint_symbol_4), &(tint_symbol_5));
}
)";

  auto got = Run<ModuleScopeVarToEntryPointParam>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, Constructors) {
  auto* src = R"(
var<private> a : f32 = 1.0;
var<private> b : f32 = f32();

[[stage(compute), workgroup_size(1)]]
fn main() {
  let x : f32 = a + b;
}
)";

  auto* expect = R"(
[[stage(compute), workgroup_size(1)]]
fn main() {
  [[internal(disable_validation__ignore_storage_class)]] var<private> tint_symbol : f32 = 1.0;
  [[internal(disable_validation__ignore_storage_class)]] var<private> tint_symbol_1 : f32 = f32();
  let x : f32 = (tint_symbol + tint_symbol_1);
}
)";

  auto got = Run<ModuleScopeVarToEntryPointParam>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, Pointers) {
  auto* src = R"(
var<private> p : f32;
var<workgroup> w : f32;

[[stage(compute), workgroup_size(1)]]
fn main() {
  let p_ptr : ptr<private, f32> = &p;
  let w_ptr : ptr<workgroup, f32> = &w;
  let x : f32 = *p_ptr + *w_ptr;
  *p_ptr = x;
}
)";

  auto* expect = R"(
[[stage(compute), workgroup_size(1)]]
fn main() {
  [[internal(disable_validation__ignore_storage_class)]] var<private> tint_symbol : f32;
  [[internal(disable_validation__ignore_storage_class)]] var<workgroup> tint_symbol_1 : f32;
  let p_ptr : ptr<private, f32> = &(tint_symbol);
  let w_ptr : ptr<workgroup, f32> = &(tint_symbol_1);
  let x : f32 = (*(p_ptr) + *(w_ptr));
  *(p_ptr) = x;
}
)";

  auto got = Run<ModuleScopeVarToEntryPointParam>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, UnusedVariables) {
  auto* src = R"(
var<private> p : f32;
var<workgroup> w : f32;

[[stage(compute), workgroup_size(1)]]
fn main() {
}
)";

  auto* expect = R"(
[[stage(compute), workgroup_size(1)]]
fn main() {
}
)";

  auto got = Run<ModuleScopeVarToEntryPointParam>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, OtherVariables) {
  auto* src = R"(
[[block]]
struct S {
  a : f32;
};

[[group(0), binding(0)]]
var<uniform> u : S;

[[stage(compute), workgroup_size(1)]]
fn main() {
}
)";

  auto* expect = R"(
[[block]]
struct S {
  a : f32;
};

[[group(0), binding(0)]] var<uniform> u : S;

[[stage(compute), workgroup_size(1)]]
fn main() {
}
)";

  auto got = Run<ModuleScopeVarToEntryPointParam>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, HandleTypes_Basic) {
  auto* src = R"(
[[group(0), binding(0)]] var t : texture_2d<f32>;
[[group(0), binding(1)]] var s : sampler;

[[stage(compute), workgroup_size(1)]]
fn main() {
  ignore(t);
  ignore(s);
}
)";

  auto* expect = R"(
[[stage(compute), workgroup_size(1)]]
fn main([[group(0), binding(0), internal(disable_validation__entry_point_parameter)]] tint_symbol : texture_2d<f32>, [[group(0), binding(1), internal(disable_validation__entry_point_parameter)]] tint_symbol_1 : sampler) {
  ignore(tint_symbol);
  ignore(tint_symbol_1);
}
)";

  auto got = Run<ModuleScopeVarToEntryPointParam>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, HandleTypes_FunctionCalls) {
  auto* src = R"(
[[group(0), binding(0)]] var t : texture_2d<f32>;
[[group(0), binding(1)]] var s : sampler;

fn no_uses() {
}

fn bar(a : f32, b : f32) {
  ignore(t);
  ignore(s);
}

fn foo(a : f32) {
  let b : f32 = 2.0;
  ignore(t);
  bar(a, b);
  no_uses();
}

[[stage(compute), workgroup_size(1)]]
fn main() {
  foo(1.0);
}
)";

  auto* expect = R"(
fn no_uses() {
}

fn bar(a : f32, b : f32, tint_symbol : texture_2d<f32>, tint_symbol_1 : sampler) {
  ignore(tint_symbol);
  ignore(tint_symbol_1);
}

fn foo(a : f32, tint_symbol_2 : texture_2d<f32>, tint_symbol_3 : sampler) {
  let b : f32 = 2.0;
  ignore(tint_symbol_2);
  bar(a, b, tint_symbol_2, tint_symbol_3);
  no_uses();
}

[[stage(compute), workgroup_size(1)]]
fn main([[group(0), binding(0), internal(disable_validation__entry_point_parameter)]] tint_symbol_4 : texture_2d<f32>, [[group(0), binding(1), internal(disable_validation__entry_point_parameter)]] tint_symbol_5 : sampler) {
  foo(1.0, tint_symbol_4, tint_symbol_5);
}
)";

  auto got = Run<ModuleScopeVarToEntryPointParam>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, EmtpyModule) {
  auto* src = "";

  auto got = Run<ModuleScopeVarToEntryPointParam>(src);

  EXPECT_EQ(src, str(got));
}

}  // namespace
}  // namespace transform
}  // namespace tint
