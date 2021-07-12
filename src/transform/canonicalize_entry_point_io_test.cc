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

TEST_F(CanonicalizeEntryPointIOTest, Error_MissingTransformData) {
  auto* src = "";

  auto* expect =
      "error: missing transform data for "
      "tint::transform::CanonicalizeEntryPointIO";

  auto got = Run<CanonicalizeEntryPointIO>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Parameters_BuiltinsAsParameters) {
  auto* src = R"(
[[stage(fragment)]]
fn frag_main([[location(1)]] loc1 : f32,
             [[location(2)]] loc2 : vec4<u32>,
             [[builtin(position)]] coord : vec4<f32>) {
  var col : f32 = (coord.x * loc1);
}
)";

  auto* expect = R"(
struct tint_symbol_1 {
  [[location(1)]]
  loc1 : f32;
  [[location(2)]]
  loc2 : vec4<u32>;
};

[[stage(fragment)]]
fn frag_main([[builtin(position)]] coord : vec4<f32>, tint_symbol : tint_symbol_1) {
  let loc1 : f32 = tint_symbol.loc1;
  let loc2 : vec4<u32> = tint_symbol.loc2;
  var col : f32 = (coord.x * loc1);
}
)";

  DataMap data;
  data.Add<CanonicalizeEntryPointIO::Config>(
      CanonicalizeEntryPointIO::BuiltinStyle::kParameter);
  auto got = Run<CanonicalizeEntryPointIO>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Parameters_BuiltinsAsStructMembers) {
  auto* src = R"(
[[stage(fragment)]]
fn frag_main([[location(1)]] loc1 : f32,
             [[location(2)]] loc2 : vec4<u32>,
             [[builtin(position)]] coord : vec4<f32>) {
  var col : f32 = (coord.x * loc1);
}
)";

  auto* expect = R"(
struct tint_symbol_1 {
  [[location(1)]]
  loc1 : f32;
  [[location(2)]]
  loc2 : vec4<u32>;
  [[builtin(position)]]
  coord : vec4<f32>;
};

[[stage(fragment)]]
fn frag_main(tint_symbol : tint_symbol_1) {
  let loc1 : f32 = tint_symbol.loc1;
  let loc2 : vec4<u32> = tint_symbol.loc2;
  let coord : vec4<f32> = tint_symbol.coord;
  var col : f32 = (coord.x * loc1);
}
)";

  DataMap data;
  data.Add<CanonicalizeEntryPointIO::Config>(
      CanonicalizeEntryPointIO::BuiltinStyle::kStructMember);
  auto got = Run<CanonicalizeEntryPointIO>(src, data);

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

struct tint_symbol_1 {
  [[location(1)]]
  loc1 : myf32;
};

[[stage(fragment)]]
fn frag_main(tint_symbol : tint_symbol_1) {
  let loc1 : myf32 = tint_symbol.loc1;
  var x : myf32 = loc1;
}
)";

  DataMap data;
  data.Add<CanonicalizeEntryPointIO::Config>(
      CanonicalizeEntryPointIO::BuiltinStyle::kParameter);
  auto got = Run<CanonicalizeEntryPointIO>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest,
       Parameters_EmptyBody_BuiltinsAsParameters) {
  auto* src = R"(
[[stage(fragment)]]
fn frag_main([[location(1)]] loc1 : f32,
             [[location(2)]] loc2 : vec4<u32>,
             [[builtin(position)]] coord : vec4<f32>) {
}
)";

  auto* expect = R"(
struct tint_symbol_1 {
  [[location(1)]]
  loc1 : f32;
  [[location(2)]]
  loc2 : vec4<u32>;
};

[[stage(fragment)]]
fn frag_main([[builtin(position)]] coord : vec4<f32>, tint_symbol : tint_symbol_1) {
  let loc1 : f32 = tint_symbol.loc1;
  let loc2 : vec4<u32> = tint_symbol.loc2;
}
)";

  DataMap data;
  data.Add<CanonicalizeEntryPointIO::Config>(
      CanonicalizeEntryPointIO::BuiltinStyle::kParameter);
  auto got = Run<CanonicalizeEntryPointIO>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest,
       Parameters_EmptyBody_BuiltinsAsStructMembers) {
  auto* src = R"(
[[stage(fragment)]]
fn frag_main([[location(1)]] loc1 : f32,
             [[location(2)]] loc2 : vec4<u32>,
             [[builtin(position)]] coord : vec4<f32>) {
}
)";

  auto* expect = R"(
struct tint_symbol_1 {
  [[location(1)]]
  loc1 : f32;
  [[location(2)]]
  loc2 : vec4<u32>;
  [[builtin(position)]]
  coord : vec4<f32>;
};

[[stage(fragment)]]
fn frag_main(tint_symbol : tint_symbol_1) {
  let loc1 : f32 = tint_symbol.loc1;
  let loc2 : vec4<u32> = tint_symbol.loc2;
  let coord : vec4<f32> = tint_symbol.coord;
}
)";

  DataMap data;
  data.Add<CanonicalizeEntryPointIO::Config>(
      CanonicalizeEntryPointIO::BuiltinStyle::kStructMember);
  auto got = Run<CanonicalizeEntryPointIO>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, StructParameters_BuiltinsAsParameters) {
  auto* src = R"(
struct FragBuiltins {
  [[builtin(position)]] coord : vec4<f32>;
};
struct FragLocations {
  [[location(1)]] loc1 : f32;
  [[location(2)]] loc2 : vec4<u32>;
};

[[stage(fragment)]]
fn frag_main([[location(0)]] loc0 : f32,
             locations : FragLocations,
             builtins : FragBuiltins) {
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

struct tint_symbol_2 {
  [[location(0)]]
  loc0 : f32;
  [[location(1)]]
  loc1 : f32;
  [[location(2)]]
  loc2 : vec4<u32>;
};

[[stage(fragment)]]
fn frag_main([[builtin(position)]] tint_symbol_1 : vec4<f32>, tint_symbol : tint_symbol_2) {
  let loc0 : f32 = tint_symbol.loc0;
  let locations : FragLocations = FragLocations(tint_symbol.loc1, tint_symbol.loc2);
  let builtins : FragBuiltins = FragBuiltins(tint_symbol_1);
  var col : f32 = ((builtins.coord.x * locations.loc1) + loc0);
}
)";

  DataMap data;
  data.Add<CanonicalizeEntryPointIO::Config>(
      CanonicalizeEntryPointIO::BuiltinStyle::kParameter);
  auto got = Run<CanonicalizeEntryPointIO>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, StructParameters_BuiltinsAsStructMembers) {
  auto* src = R"(
struct FragBuiltins {
  [[builtin(position)]] coord : vec4<f32>;
};
struct FragLocations {
  [[location(1)]] loc1 : f32;
  [[location(2)]] loc2 : vec4<u32>;
};

[[stage(fragment)]]
fn frag_main([[location(0)]] loc0 : f32,
             locations : FragLocations,
             builtins : FragBuiltins) {
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

struct tint_symbol_1 {
  [[location(0)]]
  loc0 : f32;
  [[location(1)]]
  loc1 : f32;
  [[location(2)]]
  loc2 : vec4<u32>;
  [[builtin(position)]]
  coord : vec4<f32>;
};

[[stage(fragment)]]
fn frag_main(tint_symbol : tint_symbol_1) {
  let loc0 : f32 = tint_symbol.loc0;
  let locations : FragLocations = FragLocations(tint_symbol.loc1, tint_symbol.loc2);
  let builtins : FragBuiltins = FragBuiltins(tint_symbol.coord);
  var col : f32 = ((builtins.coord.x * locations.loc1) + loc0);
}
)";

  DataMap data;
  data.Add<CanonicalizeEntryPointIO::Config>(
      CanonicalizeEntryPointIO::BuiltinStyle::kStructMember);
  auto got = Run<CanonicalizeEntryPointIO>(src, data);

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
struct tint_symbol {
  [[builtin(frag_depth)]]
  value : f32;
};

[[stage(fragment)]]
fn frag_main() -> tint_symbol {
  return tint_symbol(1.0);
}
)";

  DataMap data;
  data.Add<CanonicalizeEntryPointIO::Config>(
      CanonicalizeEntryPointIO::BuiltinStyle::kParameter);
  auto got = Run<CanonicalizeEntryPointIO>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Return_Struct) {
  auto* src = R"(
struct FragOutput {
  [[location(0)]] color : vec4<f32>;
  [[builtin(frag_depth)]] depth : f32;
  [[builtin(sample_mask)]] mask : u32;
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
  color : vec4<f32>;
  depth : f32;
  mask : u32;
};

struct tint_symbol {
  [[location(0)]]
  color : vec4<f32>;
  [[builtin(frag_depth)]]
  depth : f32;
  [[builtin(sample_mask)]]
  mask : u32;
};

[[stage(fragment)]]
fn frag_main() -> tint_symbol {
  var output : FragOutput;
  output.depth = 1.0;
  output.mask = 7u;
  output.color = vec4<f32>(0.5, 0.5, 0.5, 1.0);
  return tint_symbol(output.color, output.depth, output.mask);
}
)";

  DataMap data;
  data.Add<CanonicalizeEntryPointIO::Config>(
      CanonicalizeEntryPointIO::BuiltinStyle::kParameter);
  auto got = Run<CanonicalizeEntryPointIO>(src, data);

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

struct tint_symbol_1 {
  [[location(0)]]
  value : f32;
  [[location(1)]]
  mul : f32;
};

[[stage(fragment)]]
fn frag_main1(tint_symbol : tint_symbol_1) {
  let inputs : FragmentInput = FragmentInput(tint_symbol.value, tint_symbol.mul);
  var x : f32 = foo(inputs);
}

struct tint_symbol_3 {
  [[location(0)]]
  value : f32;
  [[location(1)]]
  mul : f32;
};

[[stage(fragment)]]
fn frag_main2(tint_symbol_2 : tint_symbol_3) {
  let inputs : FragmentInput = FragmentInput(tint_symbol_2.value, tint_symbol_2.mul);
  var x : f32 = foo(inputs);
}
)";

  DataMap data;
  data.Add<CanonicalizeEntryPointIO::Config>(
      CanonicalizeEntryPointIO::BuiltinStyle::kParameter);
  auto got = Run<CanonicalizeEntryPointIO>(src, data);

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

struct tint_symbol_1 {
  [[location(0)]]
  col1 : f32;
  [[location(1)]]
  col2 : f32;
};

[[stage(fragment)]]
fn frag_main1(tint_symbol : tint_symbol_1) {
  let inputs : FragmentInput = FragmentInput(tint_symbol.col1, tint_symbol.col2);
  global_inputs = inputs;
  var r : f32 = foo();
  var g : f32 = bar();
}
)";

  DataMap data;
  data.Add<CanonicalizeEntryPointIO::Config>(
      CanonicalizeEntryPointIO::BuiltinStyle::kParameter);
  auto got = Run<CanonicalizeEntryPointIO>(src, data);

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

struct tint_symbol_1 {
  [[location(0)]]
  col1 : myf32;
  [[location(1)]]
  col2 : myf32;
};

struct tint_symbol_2 {
  [[location(0)]]
  col1 : myf32;
  [[location(1)]]
  col2 : myf32;
};

[[stage(fragment)]]
fn frag_main(tint_symbol : tint_symbol_1) -> tint_symbol_2 {
  let inputs : MyFragmentInput = MyFragmentInput(tint_symbol.col1, tint_symbol.col2);
  var x : myf32 = foo(inputs);
  let tint_symbol_3 : FragmentOutput = MyFragmentOutput(x, inputs.col2);
  return tint_symbol_2(tint_symbol_3.col1, tint_symbol_3.col2);
}
)";

  DataMap data;
  data.Add<CanonicalizeEntryPointIO::Config>(
      CanonicalizeEntryPointIO::BuiltinStyle::kParameter);
  auto got = Run<CanonicalizeEntryPointIO>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, InterpolateAttributes) {
  auto* src = R"(
struct VertexOut {
  [[builtin(position)]] pos : vec4<f32>;
  [[location(1), interpolate(flat)]] loc1: f32;
  [[location(2), interpolate(linear, sample)]] loc2 : f32;
  [[location(3), interpolate(perspective, centroid)]] loc3 : f32;
};

struct FragmentIn {
  [[location(1), interpolate(flat)]] loc1: f32;
  [[location(2), interpolate(linear, sample)]] loc2 : f32;
};

[[stage(vertex)]]
fn vert_main() -> VertexOut {
  return VertexOut();
}

[[stage(fragment)]]
fn frag_main(inputs : FragmentIn,
             [[location(3), interpolate(perspective, centroid)]] loc3 : f32) {
  let x = inputs.loc1 + inputs.loc2 + loc3;
}
)";

  auto* expect = R"(
struct VertexOut {
  pos : vec4<f32>;
  loc1 : f32;
  loc2 : f32;
  loc3 : f32;
};

struct FragmentIn {
  loc1 : f32;
  loc2 : f32;
};

struct tint_symbol {
  [[location(1), interpolate(flat)]]
  loc1 : f32;
  [[location(2), interpolate(linear, sample)]]
  loc2 : f32;
  [[location(3), interpolate(perspective, centroid)]]
  loc3 : f32;
  [[builtin(position)]]
  pos : vec4<f32>;
};

[[stage(vertex)]]
fn vert_main() -> tint_symbol {
  let tint_symbol_1 : VertexOut = VertexOut();
  return tint_symbol(tint_symbol_1.loc1, tint_symbol_1.loc2, tint_symbol_1.loc3, tint_symbol_1.pos);
}

struct tint_symbol_3 {
  [[location(1), interpolate(flat)]]
  loc1 : f32;
  [[location(2), interpolate(linear, sample)]]
  loc2 : f32;
  [[location(3), interpolate(perspective, centroid)]]
  loc3 : f32;
};

[[stage(fragment)]]
fn frag_main(tint_symbol_2 : tint_symbol_3) {
  let inputs : FragmentIn = FragmentIn(tint_symbol_2.loc1, tint_symbol_2.loc2);
  let loc3 : f32 = tint_symbol_2.loc3;
  let x = ((inputs.loc1 + inputs.loc2) + loc3);
}
)";

  DataMap data;
  data.Add<CanonicalizeEntryPointIO::Config>(
      CanonicalizeEntryPointIO::BuiltinStyle::kStructMember);
  auto got = Run<CanonicalizeEntryPointIO>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, InvariantAttributes) {
  auto* src = R"(
struct VertexOut {
  [[builtin(position), invariant]] pos : vec4<f32>;
};

[[stage(vertex)]]
fn main1() -> VertexOut {
  return VertexOut();
}

[[stage(vertex)]]
fn main2() -> [[builtin(position), invariant]] vec4<f32> {
  return vec4<f32>();
}
)";

  auto* expect = R"(
struct VertexOut {
  pos : vec4<f32>;
};

struct tint_symbol {
  [[builtin(position), invariant]]
  pos : vec4<f32>;
};

[[stage(vertex)]]
fn main1() -> tint_symbol {
  let tint_symbol_1 : VertexOut = VertexOut();
  return tint_symbol(tint_symbol_1.pos);
}

struct tint_symbol_2 {
  [[builtin(position), invariant]]
  value : vec4<f32>;
};

[[stage(vertex)]]
fn main2() -> tint_symbol_2 {
  return tint_symbol_2(vec4<f32>());
}
)";

  DataMap data;
  data.Add<CanonicalizeEntryPointIO::Config>(
      CanonicalizeEntryPointIO::BuiltinStyle::kStructMember);
  auto got = Run<CanonicalizeEntryPointIO>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Struct_LayoutDecorations) {
  auto* src = R"(
[[block]]
struct FragmentInput {
  [[size(16), location(1)]] value : f32;
  [[builtin(position)]] [[align(32)]] coord : vec4<f32>;
  [[location(0), interpolate(linear, sample)]] [[align(128)]] loc0 : f32;
};

struct FragmentOutput {
  [[size(16), location(1), interpolate(flat)]] value : f32;
};

[[stage(fragment)]]
fn frag_main(inputs : FragmentInput) -> FragmentOutput {
  return FragmentOutput(inputs.coord.x * inputs.value + inputs.loc0);
}
)";

  auto* expect = R"(
[[block]]
struct FragmentInput {
  [[size(16)]]
  value : f32;
  [[align(32)]]
  coord : vec4<f32>;
  [[align(128)]]
  loc0 : f32;
};

struct FragmentOutput {
  [[size(16)]]
  value : f32;
};

struct tint_symbol_1 {
  [[location(0), interpolate(linear, sample)]]
  loc0 : f32;
  [[location(1)]]
  value : f32;
  [[builtin(position)]]
  coord : vec4<f32>;
};

struct tint_symbol_2 {
  [[location(1), interpolate(flat)]]
  value : f32;
};

[[stage(fragment)]]
fn frag_main(tint_symbol : tint_symbol_1) -> tint_symbol_2 {
  let inputs : FragmentInput = FragmentInput(tint_symbol.value, tint_symbol.coord, tint_symbol.loc0);
  let tint_symbol_3 : FragmentOutput = FragmentOutput(((inputs.coord.x * inputs.value) + inputs.loc0));
  return tint_symbol_2(tint_symbol_3.value);
}
)";

  DataMap data;
  data.Add<CanonicalizeEntryPointIO::Config>(
      CanonicalizeEntryPointIO::BuiltinStyle::kStructMember);
  auto got = Run<CanonicalizeEntryPointIO>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, SortedMembers) {
  auto* src = R"(
struct VertexOutput {
  [[location(1)]] b : u32;
  [[builtin(position)]] pos : vec4<f32>;
  [[location(3)]] d : u32;
  [[location(0)]] a : f32;
  [[location(2)]] c : i32;
};

struct FragmentInputExtra {
  [[location(3)]] d : u32;
  [[builtin(position)]] pos : vec4<f32>;
  [[location(0)]] a : f32;
};

[[stage(vertex)]]
fn vert_main() -> VertexOutput {
  return VertexOutput();
}

[[stage(fragment)]]
fn frag_main([[builtin(front_facing)]] ff : bool,
             [[location(2)]] c : i32,
             inputs : FragmentInputExtra,
             [[location(1)]] b : u32) {
}
)";

  auto* expect = R"(
struct VertexOutput {
  b : u32;
  pos : vec4<f32>;
  d : u32;
  a : f32;
  c : i32;
};

struct FragmentInputExtra {
  d : u32;
  pos : vec4<f32>;
  a : f32;
};

struct tint_symbol {
  [[location(0)]]
  a : f32;
  [[location(1)]]
  b : u32;
  [[location(2)]]
  c : i32;
  [[location(3)]]
  d : u32;
  [[builtin(position)]]
  pos : vec4<f32>;
};

[[stage(vertex)]]
fn vert_main() -> tint_symbol {
  let tint_symbol_1 : VertexOutput = VertexOutput();
  return tint_symbol(tint_symbol_1.a, tint_symbol_1.b, tint_symbol_1.c, tint_symbol_1.d, tint_symbol_1.pos);
}

struct tint_symbol_3 {
  [[location(0)]]
  a : f32;
  [[location(1)]]
  b : u32;
  [[location(2)]]
  c : i32;
  [[location(3)]]
  d : u32;
  [[builtin(position)]]
  pos : vec4<f32>;
  [[builtin(front_facing)]]
  ff : bool;
};

[[stage(fragment)]]
fn frag_main(tint_symbol_2 : tint_symbol_3) {
  let ff : bool = tint_symbol_2.ff;
  let c : i32 = tint_symbol_2.c;
  let inputs : FragmentInputExtra = FragmentInputExtra(tint_symbol_2.d, tint_symbol_2.pos, tint_symbol_2.a);
  let b : u32 = tint_symbol_2.b;
}
)";

  DataMap data;
  data.Add<CanonicalizeEntryPointIO::Config>(
      CanonicalizeEntryPointIO::BuiltinStyle::kStructMember);
  auto got = Run<CanonicalizeEntryPointIO>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, DontRenameSymbols) {
  auto* src = R"(
[[stage(fragment)]]
fn tint_symbol_1([[location(0)]] col : f32) {
}
)";

  auto* expect = R"(
struct tint_symbol_2 {
  [[location(0)]]
  col : f32;
};

[[stage(fragment)]]
fn tint_symbol_1(tint_symbol : tint_symbol_2) {
  let col : f32 = tint_symbol.col;
}
)";

  DataMap data;
  data.Add<CanonicalizeEntryPointIO::Config>(
      CanonicalizeEntryPointIO::BuiltinStyle::kParameter);
  auto got = Run<CanonicalizeEntryPointIO>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, FixedSampleMask_VoidNoReturn) {
  auto* src = R"(
[[stage(fragment)]]
fn frag_main() {
}
)";

  auto* expect = R"(
struct tint_symbol_1 {
  [[builtin(sample_mask)]]
  tint_symbol : u32;
};

[[stage(fragment)]]
fn frag_main() -> tint_symbol_1 {
  return tint_symbol_1(3u);
}
)";

  DataMap data;
  data.Add<CanonicalizeEntryPointIO::Config>(
      CanonicalizeEntryPointIO::BuiltinStyle::kParameter, 0x03u);
  auto got = Run<CanonicalizeEntryPointIO>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, FixedSampleMask_VoidWithReturn) {
  auto* src = R"(
[[stage(fragment)]]
fn frag_main() {
  return;
}
)";

  auto* expect = R"(
struct tint_symbol_1 {
  [[builtin(sample_mask)]]
  tint_symbol : u32;
};

[[stage(fragment)]]
fn frag_main() -> tint_symbol_1 {
  return tint_symbol_1(3u);
}
)";

  DataMap data;
  data.Add<CanonicalizeEntryPointIO::Config>(
      CanonicalizeEntryPointIO::BuiltinStyle::kParameter, 0x03u);
  auto got = Run<CanonicalizeEntryPointIO>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, FixedSampleMask_WithAuthoredMask) {
  auto* src = R"(
[[stage(fragment)]]
fn frag_main() -> [[builtin(sample_mask)]] u32 {
  return 7u;
}
)";

  auto* expect = R"(
struct tint_symbol {
  [[builtin(sample_mask)]]
  value : u32;
};

[[stage(fragment)]]
fn frag_main() -> tint_symbol {
  return tint_symbol((7u & 3u));
}
)";

  DataMap data;
  data.Add<CanonicalizeEntryPointIO::Config>(
      CanonicalizeEntryPointIO::BuiltinStyle::kParameter, 0x03u);
  auto got = Run<CanonicalizeEntryPointIO>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, FixedSampleMask_WithoutAuthoredMask) {
  auto* src = R"(
[[stage(fragment)]]
fn frag_main() -> [[location(0)]] f32 {
  return 1.0;
}
)";

  auto* expect = R"(
struct tint_symbol_1 {
  [[location(0)]]
  value : f32;
  [[builtin(sample_mask)]]
  tint_symbol : u32;
};

[[stage(fragment)]]
fn frag_main() -> tint_symbol_1 {
  return tint_symbol_1(1.0, 3u);
}
)";

  DataMap data;
  data.Add<CanonicalizeEntryPointIO::Config>(
      CanonicalizeEntryPointIO::BuiltinStyle::kParameter, 0x03u);
  auto got = Run<CanonicalizeEntryPointIO>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, FixedSampleMask_StructWithAuthoredMask) {
  auto* src = R"(
struct Output {
  [[builtin(frag_depth)]] depth : f32;
  [[builtin(sample_mask)]] mask : u32;
  [[location(0)]] value : f32;
};

[[stage(fragment)]]
fn frag_main() -> Output {
  return Output(0.5, 7u, 1.0);
}
)";

  auto* expect = R"(
struct Output {
  depth : f32;
  mask : u32;
  value : f32;
};

struct tint_symbol {
  [[location(0)]]
  value : f32;
  [[builtin(frag_depth)]]
  depth : f32;
  [[builtin(sample_mask)]]
  mask : u32;
};

[[stage(fragment)]]
fn frag_main() -> tint_symbol {
  let tint_symbol_1 : Output = Output(0.5, 7u, 1.0);
  return tint_symbol(tint_symbol_1.value, tint_symbol_1.depth, (tint_symbol_1.mask & 3u));
}
)";

  DataMap data;
  data.Add<CanonicalizeEntryPointIO::Config>(
      CanonicalizeEntryPointIO::BuiltinStyle::kParameter, 0x03u);
  auto got = Run<CanonicalizeEntryPointIO>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest,
       FixedSampleMask_StructWithoutAuthoredMask) {
  auto* src = R"(
struct Output {
  [[builtin(frag_depth)]] depth : f32;
  [[location(0)]] value : f32;
};

[[stage(fragment)]]
fn frag_main() -> Output {
  return Output(0.5, 1.0);
}
)";

  auto* expect = R"(
struct Output {
  depth : f32;
  value : f32;
};

struct tint_symbol_1 {
  [[location(0)]]
  value : f32;
  [[builtin(frag_depth)]]
  depth : f32;
  [[builtin(sample_mask)]]
  tint_symbol : u32;
};

[[stage(fragment)]]
fn frag_main() -> tint_symbol_1 {
  let tint_symbol_2 : Output = Output(0.5, 1.0);
  return tint_symbol_1(tint_symbol_2.value, tint_symbol_2.depth, 3u);
}
)";

  DataMap data;
  data.Add<CanonicalizeEntryPointIO::Config>(
      CanonicalizeEntryPointIO::BuiltinStyle::kParameter, 0x03u);
  auto got = Run<CanonicalizeEntryPointIO>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, FixedSampleMask_MultipleShaders) {
  auto* src = R"(
[[stage(fragment)]]
fn frag_main1() -> [[builtin(sample_mask)]] u32 {
  return 7u;
}

[[stage(fragment)]]
fn frag_main2() -> [[location(0)]] f32 {
  return 1.0;
}

[[stage(vertex)]]
fn vert_main1() -> [[builtin(position)]] vec4<f32> {
  return vec4<f32>();
}

[[stage(compute), workgroup_size(1)]]
fn comp_main1() {
}
)";

  auto* expect = R"(
struct tint_symbol {
  [[builtin(sample_mask)]]
  value : u32;
};

[[stage(fragment)]]
fn frag_main1() -> tint_symbol {
  return tint_symbol((7u & 3u));
}

struct tint_symbol_2 {
  [[location(0)]]
  value : f32;
  [[builtin(sample_mask)]]
  tint_symbol_1 : u32;
};

[[stage(fragment)]]
fn frag_main2() -> tint_symbol_2 {
  return tint_symbol_2(1.0, 3u);
}

struct tint_symbol_3 {
  [[builtin(position)]]
  value : vec4<f32>;
};

[[stage(vertex)]]
fn vert_main1() -> tint_symbol_3 {
  return tint_symbol_3(vec4<f32>());
}

[[stage(compute), workgroup_size(1)]]
fn comp_main1() {
}
)";

  DataMap data;
  data.Add<CanonicalizeEntryPointIO::Config>(
      CanonicalizeEntryPointIO::BuiltinStyle::kParameter, 0x03u);
  auto got = Run<CanonicalizeEntryPointIO>(src, data);

  EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace transform
}  // namespace tint
