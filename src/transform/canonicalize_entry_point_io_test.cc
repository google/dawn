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

#include "src/transform/canonicalize_entry_point_io.h"

#include "src/transform/test_helper.h"

namespace tint {
namespace transform {
namespace {

using CanonicalizeEntryPointIOTest = TransformTest;

TEST_F(CanonicalizeEntryPointIOTest, Parameters) {
  auto* src = R"(
[[stage(fragment)]]
fn frag_main([[builtin(frag_coord)]] coord : vec4<f32>,
             [[location(1)]] loc1 : f32,
             [[location(2)]] loc2 : vec4<u32>) {
  var col : f32 = (coord.x * loc1);
}
)";

  auto* expect = R"(
struct tint_symbol_5 {
  [[builtin(frag_coord)]]
  coord : vec4<f32>;
  [[location(1)]]
  loc1 : f32;
  [[location(2)]]
  loc2 : vec4<u32>;
};

[[stage(fragment)]]
fn frag_main(tint_symbol_1 : tint_symbol_5) {
  const coord : vec4<f32> = tint_symbol_1.coord;
  const loc1 : f32 = tint_symbol_1.loc1;
  const loc2 : vec4<u32> = tint_symbol_1.loc2;
  var col : f32 = (coord.x * loc1);
}
)";

  auto got = Run<CanonicalizeEntryPointIO>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Parameter_TypeAlias) {
  auto* src = R"(
type myf32 = f32;

[[stage(fragment)]]
fn frag_main([[location(1)]] loc1 : myf32) {
  var x : myf32 = loc1;
}
)";

  auto* expect = R"(
type myf32 = f32;

struct tint_symbol_4 {
  [[location(1)]]
  loc1 : myf32;
};

[[stage(fragment)]]
fn frag_main(tint_symbol_1 : tint_symbol_4) {
  const loc1 : myf32 = tint_symbol_1.loc1;
  var x : myf32 = loc1;
}
)";

  auto got = Run<CanonicalizeEntryPointIO>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Parameters_EmptyBody) {
  auto* src = R"(
[[stage(fragment)]]
fn frag_main([[builtin(frag_coord)]] coord : vec4<f32>,
             [[location(1)]] loc1 : f32,
             [[location(2)]] loc2 : vec4<u32>) {
}
)";

  auto* expect = R"(
struct tint_symbol_5 {
  [[builtin(frag_coord)]]
  coord : vec4<f32>;
  [[location(1)]]
  loc1 : f32;
  [[location(2)]]
  loc2 : vec4<u32>;
};

[[stage(fragment)]]
fn frag_main(tint_symbol_1 : tint_symbol_5) {
}
)";

  auto got = Run<CanonicalizeEntryPointIO>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, StructParameters) {
  auto* src = R"(
struct FragBuiltins {
  [[builtin(frag_coord)]] coord : vec4<f32>;
};
struct FragLocations {
  [[location(1)]] loc1 : f32;
  [[location(2)]] loc2 : vec4<u32>;
};

[[stage(fragment)]]
fn frag_main(builtins : FragBuiltins,
             locations : FragLocations,
             [[location(0)]] loc0 : f32) {
  var col : f32 = ((builtins.coord.x * locations.loc1) + loc0);
}
)";

  auto* expect = R"(
struct FragBuiltins {
  coord : vec4<f32>;
};

struct FragLocations {
  loc1 : f32;
  loc2 : vec4<u32>;
};

struct tint_symbol_10 {
  [[builtin(frag_coord)]]
  coord : vec4<f32>;
  [[location(1)]]
  loc1 : f32;
  [[location(2)]]
  loc2 : vec4<u32>;
  [[location(0)]]
  loc0 : f32;
};

[[stage(fragment)]]
fn frag_main(tint_symbol_6 : tint_symbol_10) {
  const builtins : FragBuiltins = FragBuiltins(tint_symbol_6.coord);
  const locations : FragLocations = FragLocations(tint_symbol_6.loc1, tint_symbol_6.loc2);
  const loc0 : f32 = tint_symbol_6.loc0;
  var col : f32 = ((builtins.coord.x * locations.loc1) + loc0);
}
)";

  auto got = Run<CanonicalizeEntryPointIO>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Return_Scalar) {
  auto* src = R"(
[[stage(fragment)]]
fn frag_main() -> [[builtin(frag_depth)]] f32 {
  return 1.0;
}
)";

  auto* expect = R"(
struct tint_symbol_2 {
  [[builtin(frag_depth)]]
  value : f32;
};

[[stage(fragment)]]
fn frag_main() -> tint_symbol_2 {
  return tint_symbol_2(1.0);
}
)";

  auto got = Run<CanonicalizeEntryPointIO>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Return_Struct) {
  auto* src = R"(
struct FragOutput {
  [[builtin(frag_depth)]] depth : f32;
  [[builtin(sample_mask_out)]] mask : u32;
  [[location(0)]] color : vec4<f32>;
};

[[stage(fragment)]]
fn frag_main() -> FragOutput {
  var output : FragOutput;
  output.depth = 1.0;
  output.mask = 7u;
  output.color = vec4<f32>(0.5, 0.5, 0.5, 1.0);
  return output;
}
)";

  auto* expect = R"(
struct FragOutput {
  depth : f32;
  mask : u32;
  color : vec4<f32>;
};

struct tint_symbol_5 {
  [[builtin(frag_depth)]]
  depth : f32;
  [[builtin(sample_mask_out)]]
  mask : u32;
  [[location(0)]]
  color : vec4<f32>;
};

[[stage(fragment)]]
fn frag_main() -> tint_symbol_5 {
  var output : FragOutput;
  output.depth = 1.0;
  output.mask = 7u;
  output.color = vec4<f32>(0.5, 0.5, 0.5, 1.0);
  return tint_symbol_5(output.depth, output.mask, output.color);
}
)";

  auto got = Run<CanonicalizeEntryPointIO>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, StructParameters_SharedDeviceFunction) {
  auto* src = R"(
struct FragmentInput {
  [[location(0)]] value : f32;
  [[location(1)]] mul : f32;
};

fn foo(x : FragmentInput) -> f32 {
  return x.value * x.mul;
}

[[stage(fragment)]]
fn frag_main1(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}

[[stage(fragment)]]
fn frag_main2(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}
)";

  auto* expect = R"(
struct FragmentInput {
  value : f32;
  mul : f32;
};

fn foo(x : FragmentInput) -> f32 {
  return (x.value * x.mul);
}

struct tint_symbol_6 {
  [[location(0)]]
  value : f32;
  [[location(1)]]
  mul : f32;
};

[[stage(fragment)]]
fn frag_main1(tint_symbol_4 : tint_symbol_6) {
  const inputs : FragmentInput = FragmentInput(tint_symbol_4.value, tint_symbol_4.mul);
  var x : f32 = foo(inputs);
}

struct tint_symbol_11 {
  [[location(0)]]
  value : f32;
  [[location(1)]]
  mul : f32;
};

[[stage(fragment)]]
fn frag_main2(tint_symbol_10 : tint_symbol_11) {
  const inputs : FragmentInput = FragmentInput(tint_symbol_10.value, tint_symbol_10.mul);
  var x : f32 = foo(inputs);
}
)";

  auto got = Run<CanonicalizeEntryPointIO>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Struct_ModuleScopeVariable) {
  auto* src = R"(
struct FragmentInput {
  [[location(0)]] col1 : f32;
  [[location(1)]] col2 : f32;
};

var<private> global_inputs : FragmentInput;

fn foo() -> f32 {
  return global_inputs.col1 * 0.5;
}

fn bar() -> f32 {
  return global_inputs.col2 * 2.0;
}

[[stage(fragment)]]
fn frag_main1(inputs : FragmentInput) {
 global_inputs = inputs;
 var r : f32 = foo();
 var g : f32 = bar();
}
)";

  auto* expect = R"(
struct FragmentInput {
  col1 : f32;
  col2 : f32;
};

var<private> global_inputs : FragmentInput;

fn foo() -> f32 {
  return (global_inputs.col1 * 0.5);
}

fn bar() -> f32 {
  return (global_inputs.col2 * 2.0);
}

struct tint_symbol_6 {
  [[location(0)]]
  col1 : f32;
  [[location(1)]]
  col2 : f32;
};

[[stage(fragment)]]
fn frag_main1(tint_symbol_4 : tint_symbol_6) {
  const inputs : FragmentInput = FragmentInput(tint_symbol_4.col1, tint_symbol_4.col2);
  global_inputs = inputs;
  var r : f32 = foo();
  var g : f32 = bar();
}
)";

  auto got = Run<CanonicalizeEntryPointIO>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Struct_TypeAliases) {
  auto* src = R"(
type myf32 = f32;

struct FragmentInput {
  [[location(0)]] col1 : myf32;
  [[location(1)]] col2 : myf32;
};

struct FragmentOutput {
  [[location(0)]] col1 : myf32;
  [[location(1)]] col2 : myf32;
};

type MyFragmentInput = FragmentInput;

type MyFragmentOutput = FragmentOutput;

fn foo(x : MyFragmentInput) -> myf32 {
  return x.col1;
}

[[stage(fragment)]]
fn frag_main(inputs : MyFragmentInput) -> MyFragmentOutput {
  var x : myf32 = foo(inputs);
  return MyFragmentOutput(x, inputs.col2);
}
)";

  auto* expect = R"(
type myf32 = f32;

struct FragmentInput {
  col1 : myf32;
  col2 : myf32;
};

struct FragmentOutput {
  col1 : myf32;
  col2 : myf32;
};

type MyFragmentInput = FragmentInput;

type MyFragmentOutput = FragmentOutput;

fn foo(x : MyFragmentInput) -> myf32 {
  return x.col1;
}

struct tint_symbol_9 {
  [[location(0)]]
  col1 : myf32;
  [[location(1)]]
  col2 : myf32;
};

struct tint_symbol_10 {
  [[location(0)]]
  col1 : myf32;
  [[location(1)]]
  col2 : myf32;
};

[[stage(fragment)]]
fn frag_main(tint_symbol_6 : tint_symbol_9) -> tint_symbol_10 {
  const inputs : MyFragmentInput = MyFragmentInput(tint_symbol_6.col1, tint_symbol_6.col2);
  var x : myf32 = foo(inputs);
  const tint_symbol_13 : FragmentOutput = MyFragmentOutput(x, inputs.col2);
  return tint_symbol_10(tint_symbol_13.col1, tint_symbol_13.col2);
}
)";

  auto got = Run<CanonicalizeEntryPointIO>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Struct_LayoutDecorations) {
  auto* src = R"(
[[block]]
struct FragmentInput {
  [[size(16), location(1)]] value : f32;
  [[builtin(frag_coord)]] [[align(32)]] coord : vec4<f32>;
};

struct FragmentOutput {
  [[size(16), location(1)]] value : f32;
};

[[stage(fragment)]]
fn frag_main(inputs : FragmentInput) -> FragmentOutput {
  return FragmentOutput(inputs.coord.x * inputs.value);
}
)";

  auto* expect = R"(
[[block]]
struct FragmentInput {
  [[size(16)]]
  value : f32;
  [[align(32)]]
  coord : vec4<f32>;
};

struct FragmentOutput {
  [[size(16)]]
  value : f32;
};

struct tint_symbol_7 {
  [[location(1)]]
  value : f32;
  [[builtin(frag_coord)]]
  coord : vec4<f32>;
};

struct tint_symbol_8 {
  [[location(1)]]
  value : f32;
};

[[stage(fragment)]]
fn frag_main(tint_symbol_5 : tint_symbol_7) -> tint_symbol_8 {
  const inputs : FragmentInput = FragmentInput(tint_symbol_5.value, tint_symbol_5.coord);
  const tint_symbol_10 : FragmentOutput = FragmentOutput((inputs.coord.x * inputs.value));
  return tint_symbol_8(tint_symbol_10.value);
}
)";

  auto got = Run<CanonicalizeEntryPointIO>(src);

  EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace transform
}  // namespace tint
