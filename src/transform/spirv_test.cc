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

#include "src/transform/spirv.h"

#include "src/transform/test_helper.h"

namespace tint {
namespace transform {
namespace {

using SpirvTest = TransformTest;

TEST_F(SpirvTest, HandleEntryPointIOTypes_Parameters) {
  auto* src = R"(
[[stage(fragment)]]
fn frag_main([[builtin(position)]] coord : vec4<f32>,
             [[location(1)]] loc1 : f32) {
  var col : f32 = (coord.x * loc1);
}

[[stage(compute), workgroup_size(8, 1, 1)]]
fn compute_main([[builtin(local_invocation_id)]] local_id : vec3<u32>,
                [[builtin(local_invocation_index)]] local_index : u32) {
  var id_x : u32 = local_id.x;
}
)";

  auto* expect = R"(
[[builtin(position), internal(disable_validation__ignore_storage_class)]] var<in> tint_symbol : vec4<f32>;

[[location(1), internal(disable_validation__ignore_storage_class)]] var<in> tint_symbol_1 : f32;

[[stage(fragment)]]
fn frag_main() {
  var col : f32 = (tint_symbol.x * tint_symbol_1);
}

[[builtin(local_invocation_id), internal(disable_validation__ignore_storage_class)]] var<in> tint_symbol_2 : vec3<u32>;

[[builtin(local_invocation_index), internal(disable_validation__ignore_storage_class)]] var<in> tint_symbol_3 : u32;

[[stage(compute), workgroup_size(8, 1, 1)]]
fn compute_main() {
  var id_x : u32 = tint_symbol_2.x;
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleEntryPointIOTypes_Parameter_TypeAlias) {
  auto* src = R"(
type myf32 = f32;

[[stage(fragment)]]
fn frag_main([[location(1)]] loc1 : myf32) {
}
)";

  auto* expect = R"(
type myf32 = f32;

[[location(1), internal(disable_validation__ignore_storage_class)]] var<in> tint_symbol : myf32;

[[stage(fragment)]]
fn frag_main() {
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleEntryPointIOTypes_ReturnBuiltin) {
  auto* src = R"(
[[stage(vertex)]]
fn vert_main() -> [[builtin(position)]] vec4<f32> {
  return vec4<f32>(1.0, 2.0, 3.0, 0.0);
}
)";

  auto* expect = R"(
[[builtin(position), internal(disable_validation__ignore_storage_class)]] var<out> tint_symbol_1 : vec4<f32>;

fn tint_symbol_2(tint_symbol : vec4<f32>) {
  tint_symbol_1 = tint_symbol;
}

[[stage(vertex)]]
fn vert_main() {
  tint_symbol_2(vec4<f32>(1.0, 2.0, 3.0, 0.0));
  return;
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleEntryPointIOTypes_ReturnLocation) {
  auto* src = R"(
[[stage(fragment)]]
fn frag_main([[location(0)]] loc_in : u32) -> [[location(0)]] f32 {
  if (loc_in > 10u) {
    return 0.5;
  }
  return 1.0;
}
)";

  auto* expect = R"(
[[location(0), internal(disable_validation__ignore_storage_class), interpolate(flat)]] var<in> tint_symbol : u32;

[[location(0), internal(disable_validation__ignore_storage_class)]] var<out> tint_symbol_2 : f32;

fn tint_symbol_3(tint_symbol_1 : f32) {
  tint_symbol_2 = tint_symbol_1;
}

[[stage(fragment)]]
fn frag_main() {
  if ((tint_symbol > 10u)) {
    tint_symbol_3(0.5);
    return;
  }
  tint_symbol_3(1.0);
  return;
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleEntryPointIOTypes_ReturnLocation_TypeAlias) {
  auto* src = R"(
type myf32 = f32;

[[stage(fragment)]]
fn frag_main([[location(0)]] loc_in : u32) -> [[location(0)]] myf32 {
  if (loc_in > 10u) {
    return 0.5;
  }
  return 1.0;
}
)";

  auto* expect = R"(
type myf32 = f32;

[[location(0), internal(disable_validation__ignore_storage_class), interpolate(flat)]] var<in> tint_symbol : u32;

[[location(0), internal(disable_validation__ignore_storage_class)]] var<out> tint_symbol_2 : myf32;

fn tint_symbol_3(tint_symbol_1 : myf32) {
  tint_symbol_2 = tint_symbol_1;
}

[[stage(fragment)]]
fn frag_main() {
  if ((tint_symbol > 10u)) {
    tint_symbol_3(0.5);
    return;
  }
  tint_symbol_3(1.0);
  return;
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleEntryPointIOTypes_StructParameters) {
  auto* src = R"(
struct FragmentInput {
  [[builtin(position)]] coord : vec4<f32>;
  [[location(1)]] value : f32;
};

[[stage(fragment)]]
fn frag_main(inputs : FragmentInput) {
  var col : f32 = inputs.coord.x * inputs.value;
}
)";

  auto* expect = R"(
struct FragmentInput {
  coord : vec4<f32>;
  value : f32;
};

[[builtin(position), internal(disable_validation__ignore_storage_class)]] var<in> tint_symbol : vec4<f32>;

[[location(1), internal(disable_validation__ignore_storage_class)]] var<in> tint_symbol_1 : f32;

[[stage(fragment)]]
fn frag_main() {
  let tint_symbol_2 : FragmentInput = FragmentInput(tint_symbol, tint_symbol_1);
  var col : f32 = (tint_symbol_2.coord.x * tint_symbol_2.value);
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleEntryPointIOTypes_StructParameters_EmptyBody) {
  auto* src = R"(
struct FragmentInput {
  [[location(1)]] value : f32;
};

[[stage(fragment)]]
fn frag_main(inputs : FragmentInput) {
}
)";

  auto* expect = R"(
struct FragmentInput {
  value : f32;
};

[[location(1), internal(disable_validation__ignore_storage_class)]] var<in> tint_symbol : f32;

[[stage(fragment)]]
fn frag_main() {
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleEntryPointIOTypes_ReturnStruct) {
  auto* src = R"(
struct VertexOutput {
  [[builtin(position)]] pos : vec4<f32>;
  [[location(1)]] value : f32;
};

[[stage(vertex)]]
fn vert_main() -> VertexOutput {
  if (false) {
    return VertexOutput();
  }
  var pos : vec4<f32> = vec4<f32>(1.0, 2.0, 3.0, 0.0);
  return VertexOutput(pos, 2.0);
}
)";

  auto* expect = R"(
struct VertexOutput {
  pos : vec4<f32>;
  value : f32;
};

[[builtin(position), internal(disable_validation__ignore_storage_class)]] var<out> tint_symbol_1 : vec4<f32>;

[[location(1), internal(disable_validation__ignore_storage_class)]] var<out> tint_symbol_2 : f32;

fn tint_symbol_3(tint_symbol : VertexOutput) {
  tint_symbol_1 = tint_symbol.pos;
  tint_symbol_2 = tint_symbol.value;
}

[[stage(vertex)]]
fn vert_main() {
  if (false) {
    tint_symbol_3(VertexOutput());
    return;
  }
  var pos : vec4<f32> = vec4<f32>(1.0, 2.0, 3.0, 0.0);
  tint_symbol_3(VertexOutput(pos, 2.0));
  return;
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleEntryPointIOTypes_SharedStruct_SameShader) {
  auto* src = R"(
struct Interface {
  [[location(1)]] value : f32;
};

[[stage(fragment)]]
fn frag_main(inputs : Interface) -> Interface {
  return inputs;
}
)";

  auto* expect = R"(
struct Interface {
  value : f32;
};

[[location(1), internal(disable_validation__ignore_storage_class)]] var<in> tint_symbol : f32;

[[location(1), internal(disable_validation__ignore_storage_class)]] var<out> tint_symbol_3 : f32;

fn tint_symbol_4(tint_symbol_2 : Interface) {
  tint_symbol_3 = tint_symbol_2.value;
}

[[stage(fragment)]]
fn frag_main() {
  let tint_symbol_1 : Interface = Interface(tint_symbol);
  tint_symbol_4(tint_symbol_1);
  return;
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleEntryPointIOTypes_SharedStruct_DifferentShaders) {
  auto* src = R"(
struct Interface {
  [[builtin(position)]] pos : vec4<f32>;
  [[location(1)]] value : f32;
};

[[stage(vertex)]]
fn vert_main() -> Interface {
  return Interface(vec4<f32>(), 42.0);
}

[[stage(fragment)]]
fn frag_main(inputs : Interface) {
  var x : f32 = inputs.value;
}
)";

  auto* expect = R"(
struct Interface {
  pos : vec4<f32>;
  value : f32;
};

[[builtin(position), internal(disable_validation__ignore_storage_class)]] var<out> tint_symbol_1 : vec4<f32>;

[[location(1), internal(disable_validation__ignore_storage_class)]] var<out> tint_symbol_2 : f32;

fn tint_symbol_3(tint_symbol : Interface) {
  tint_symbol_1 = tint_symbol.pos;
  tint_symbol_2 = tint_symbol.value;
}

[[stage(vertex)]]
fn vert_main() {
  tint_symbol_3(Interface(vec4<f32>(), 42.0));
  return;
}

[[builtin(position), internal(disable_validation__ignore_storage_class)]] var<in> tint_symbol_4 : vec4<f32>;

[[location(1), internal(disable_validation__ignore_storage_class)]] var<in> tint_symbol_5 : f32;

[[stage(fragment)]]
fn frag_main() {
  let tint_symbol_6 : Interface = Interface(tint_symbol_4, tint_symbol_5);
  var x : f32 = tint_symbol_6.value;
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleEntryPointIOTypes_InterpolateAttributes) {
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

[[builtin(position), internal(disable_validation__ignore_storage_class)]] var<out> tint_symbol_1 : vec4<f32>;

[[location(1), interpolate(flat), internal(disable_validation__ignore_storage_class)]] var<out> tint_symbol_2 : f32;

[[location(2), interpolate(linear, sample), internal(disable_validation__ignore_storage_class)]] var<out> tint_symbol_3 : f32;

[[location(3), interpolate(perspective, centroid), internal(disable_validation__ignore_storage_class)]] var<out> tint_symbol_4 : f32;

fn tint_symbol_5(tint_symbol : VertexOut) {
  tint_symbol_1 = tint_symbol.pos;
  tint_symbol_2 = tint_symbol.loc1;
  tint_symbol_3 = tint_symbol.loc2;
  tint_symbol_4 = tint_symbol.loc3;
}

[[stage(vertex)]]
fn vert_main() {
  tint_symbol_5(VertexOut());
  return;
}

[[location(1), interpolate(flat), internal(disable_validation__ignore_storage_class)]] var<in> tint_symbol_6 : f32;

[[location(2), interpolate(linear, sample), internal(disable_validation__ignore_storage_class)]] var<in> tint_symbol_7 : f32;

[[location(3), interpolate(perspective, centroid), internal(disable_validation__ignore_storage_class)]] var<in> tint_symbol_9 : f32;

[[stage(fragment)]]
fn frag_main() {
  let tint_symbol_8 : FragmentIn = FragmentIn(tint_symbol_6, tint_symbol_7);
  let x = ((tint_symbol_8.loc1 + tint_symbol_8.loc2) + tint_symbol_9);
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleEntryPointIOTypes_InterpolateAttributes_Integers) {
  // Test that we add a Flat attribute to integers that are vertex outputs and
  // fragment inputs, but not vertex inputs or fragment outputs.
  auto* src = R"(
struct VertexIn {
  [[location(0)]] i : i32;
  [[location(1)]] u : u32;
  [[location(2)]] vi : vec4<i32>;
  [[location(3)]] vu : vec4<u32>;
};

struct VertexOut {
  [[location(0)]] i : i32;
  [[location(1)]] u : u32;
  [[location(2)]] vi : vec4<i32>;
  [[location(3)]] vu : vec4<u32>;
  [[builtin(position)]] pos : vec4<f32>;
};

struct FragmentInterface {
  [[location(0)]] i : i32;
  [[location(1)]] u : u32;
  [[location(2)]] vi : vec4<i32>;
  [[location(3)]] vu : vec4<u32>;
};

[[stage(vertex)]]
fn vert_main(in : VertexIn) -> VertexOut {
  return VertexOut(in.i, in.u, in.vi, in.vu, vec4<f32>());
}

[[stage(fragment)]]
fn frag_main(inputs : FragmentInterface) -> FragmentInterface {
  return inputs;
}
)";

  auto* expect = R"(
struct VertexIn {
  i : i32;
  u : u32;
  vi : vec4<i32>;
  vu : vec4<u32>;
};

struct VertexOut {
  i : i32;
  u : u32;
  vi : vec4<i32>;
  vu : vec4<u32>;
  pos : vec4<f32>;
};

struct FragmentInterface {
  i : i32;
  u : u32;
  vi : vec4<i32>;
  vu : vec4<u32>;
};

[[location(0), internal(disable_validation__ignore_storage_class)]] var<in> tint_symbol : i32;

[[location(1), internal(disable_validation__ignore_storage_class)]] var<in> tint_symbol_1 : u32;

[[location(2), internal(disable_validation__ignore_storage_class)]] var<in> tint_symbol_2 : vec4<i32>;

[[location(3), internal(disable_validation__ignore_storage_class)]] var<in> tint_symbol_3 : vec4<u32>;

[[location(0), internal(disable_validation__ignore_storage_class), interpolate(flat)]] var<out> tint_symbol_6 : i32;

[[location(1), internal(disable_validation__ignore_storage_class), interpolate(flat)]] var<out> tint_symbol_7 : u32;

[[location(2), internal(disable_validation__ignore_storage_class), interpolate(flat)]] var<out> tint_symbol_8 : vec4<i32>;

[[location(3), internal(disable_validation__ignore_storage_class), interpolate(flat)]] var<out> tint_symbol_9 : vec4<u32>;

[[builtin(position), internal(disable_validation__ignore_storage_class)]] var<out> tint_symbol_10 : vec4<f32>;

fn tint_symbol_11(tint_symbol_5 : VertexOut) {
  tint_symbol_6 = tint_symbol_5.i;
  tint_symbol_7 = tint_symbol_5.u;
  tint_symbol_8 = tint_symbol_5.vi;
  tint_symbol_9 = tint_symbol_5.vu;
  tint_symbol_10 = tint_symbol_5.pos;
}

[[stage(vertex)]]
fn vert_main() {
  let tint_symbol_4 : VertexIn = VertexIn(tint_symbol, tint_symbol_1, tint_symbol_2, tint_symbol_3);
  tint_symbol_11(VertexOut(tint_symbol_4.i, tint_symbol_4.u, tint_symbol_4.vi, tint_symbol_4.vu, vec4<f32>()));
  return;
}

[[location(0), internal(disable_validation__ignore_storage_class), interpolate(flat)]] var<in> tint_symbol_12 : i32;

[[location(1), internal(disable_validation__ignore_storage_class), interpolate(flat)]] var<in> tint_symbol_13 : u32;

[[location(2), internal(disable_validation__ignore_storage_class), interpolate(flat)]] var<in> tint_symbol_14 : vec4<i32>;

[[location(3), internal(disable_validation__ignore_storage_class), interpolate(flat)]] var<in> tint_symbol_15 : vec4<u32>;

[[location(0), internal(disable_validation__ignore_storage_class)]] var<out> tint_symbol_18 : i32;

[[location(1), internal(disable_validation__ignore_storage_class)]] var<out> tint_symbol_19 : u32;

[[location(2), internal(disable_validation__ignore_storage_class)]] var<out> tint_symbol_20 : vec4<i32>;

[[location(3), internal(disable_validation__ignore_storage_class)]] var<out> tint_symbol_21 : vec4<u32>;

fn tint_symbol_22(tint_symbol_17 : FragmentInterface) {
  tint_symbol_18 = tint_symbol_17.i;
  tint_symbol_19 = tint_symbol_17.u;
  tint_symbol_20 = tint_symbol_17.vi;
  tint_symbol_21 = tint_symbol_17.vu;
}

[[stage(fragment)]]
fn frag_main() {
  let tint_symbol_16 : FragmentInterface = FragmentInterface(tint_symbol_12, tint_symbol_13, tint_symbol_14, tint_symbol_15);
  tint_symbol_22(tint_symbol_16);
  return;
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleEntryPointIOTypes_InvariantAttributes) {
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

[[builtin(position), invariant, internal(disable_validation__ignore_storage_class)]] var<out> tint_symbol_1 : vec4<f32>;

fn tint_symbol_2(tint_symbol : VertexOut) {
  tint_symbol_1 = tint_symbol.pos;
}

[[stage(vertex)]]
fn main1() {
  tint_symbol_2(VertexOut());
  return;
}

[[builtin(position), invariant, internal(disable_validation__ignore_storage_class)]] var<out> tint_symbol_4 : vec4<f32>;

fn tint_symbol_5(tint_symbol_3 : vec4<f32>) {
  tint_symbol_4 = tint_symbol_3;
}

[[stage(vertex)]]
fn main2() {
  tint_symbol_5(vec4<f32>());
  return;
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleEntryPointIOTypes_StructLayoutDecorations) {
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

[[location(1), internal(disable_validation__ignore_storage_class)]] var<in> tint_symbol : f32;

[[builtin(position), internal(disable_validation__ignore_storage_class)]] var<in> tint_symbol_1 : vec4<f32>;

[[location(0), interpolate(linear, sample), internal(disable_validation__ignore_storage_class)]] var<in> tint_symbol_2 : f32;

[[location(1), interpolate(flat), internal(disable_validation__ignore_storage_class)]] var<out> tint_symbol_5 : f32;

fn tint_symbol_6(tint_symbol_4 : FragmentOutput) {
  tint_symbol_5 = tint_symbol_4.value;
}

[[stage(fragment)]]
fn frag_main() {
  let tint_symbol_3 : FragmentInput = FragmentInput(tint_symbol, tint_symbol_1, tint_symbol_2);
  tint_symbol_6(FragmentOutput(((tint_symbol_3.coord.x * tint_symbol_3.value) + tint_symbol_3.loc0)));
  return;
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleEntryPointIOTypes_WithPrivateGlobalVariable) {
  // Test with a global variable to ensure that symbols are cloned correctly.
  // crbug.com/tint/701
  auto* src = R"(
var<private> x : f32;

struct VertexOutput {
  [[builtin(position)]] Position : vec4<f32>;
};

[[stage(vertex)]]
fn main() -> VertexOutput {
    return VertexOutput(vec4<f32>());
}
)";

  auto* expect = R"(
var<private> x : f32;

struct VertexOutput {
  Position : vec4<f32>;
};

[[builtin(position), internal(disable_validation__ignore_storage_class)]] var<out> tint_symbol_1 : vec4<f32>;

fn tint_symbol_2(tint_symbol : VertexOutput) {
  tint_symbol_1 = tint_symbol.Position;
}

[[stage(vertex)]]
fn main() {
  tint_symbol_2(VertexOutput(vec4<f32>()));
  return;
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleSampleMaskBuiltins_Basic) {
  auto* src = R"(
[[stage(fragment)]]
fn main([[builtin(sample_index)]] sample_index : u32,
        [[builtin(sample_mask)]] mask_in : u32
        ) -> [[builtin(sample_mask)]] u32 {
  return mask_in;
}
)";

  auto* expect = R"(
[[builtin(sample_index), internal(disable_validation__ignore_storage_class)]] var<in> tint_symbol : u32;

[[builtin(sample_mask), internal(disable_validation__ignore_storage_class)]] var<in> tint_symbol_1 : array<u32, 1>;

[[builtin(sample_mask), internal(disable_validation__ignore_storage_class)]] var<out> tint_symbol_3 : array<u32, 1>;

fn tint_symbol_4(tint_symbol_2 : u32) {
  tint_symbol_3[0] = tint_symbol_2;
}

[[stage(fragment)]]
fn main() {
  tint_symbol_4(tint_symbol_1[0]);
  return;
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleSampleMaskBuiltins_FunctionArg) {
  auto* src = R"(
fn filter(mask: u32) -> u32 {
  return (mask & 3u);
}

fn set_mask(input : u32) -> u32 {
  return input;
}

[[stage(fragment)]]
fn main([[builtin(sample_mask)]] mask_in : u32
        ) -> [[builtin(sample_mask)]] u32 {
  return set_mask(filter(mask_in));
}
)";

  auto* expect = R"(
fn filter(mask : u32) -> u32 {
  return (mask & 3u);
}

fn set_mask(input : u32) -> u32 {
  return input;
}

[[builtin(sample_mask), internal(disable_validation__ignore_storage_class)]] var<in> tint_symbol : array<u32, 1>;

[[builtin(sample_mask), internal(disable_validation__ignore_storage_class)]] var<out> tint_symbol_2 : array<u32, 1>;

fn tint_symbol_3(tint_symbol_1 : u32) {
  tint_symbol_2[0] = tint_symbol_1;
}

[[stage(fragment)]]
fn main() {
  tint_symbol_3(set_mask(filter(tint_symbol[0])));
  return;
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, EmitVertexPointSize_Basic) {
  auto* src = R"(
fn non_entry_point() {
}

[[stage(vertex)]]
fn main() -> [[builtin(position)]] vec4<f32> {
  non_entry_point();
  return vec4<f32>();
}
)";

  auto* expect = R"(
[[builtin(pointsize), internal(disable_validation__ignore_storage_class)]] var<out> tint_pointsize : f32;

fn non_entry_point() {
}

[[builtin(position), internal(disable_validation__ignore_storage_class)]] var<out> tint_symbol_1 : vec4<f32>;

fn tint_symbol_2(tint_symbol : vec4<f32>) {
  tint_symbol_1 = tint_symbol;
}

[[stage(vertex)]]
fn main() {
  tint_pointsize = 1.0;
  non_entry_point();
  tint_symbol_2(vec4<f32>());
  return;
}
)";

  DataMap data;
  data.Add<Spirv::Config>(true);
  auto got = Run<Spirv>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, EmitVertexPointSize_MultipleVertexShaders) {
  auto* src = R"(
[[stage(vertex)]]
fn main1() -> [[builtin(position)]] vec4<f32> {
  return vec4<f32>();
}

[[stage(vertex)]]
fn main2() -> [[builtin(position)]] vec4<f32> {
  return vec4<f32>();
}

[[stage(vertex)]]
fn main3() -> [[builtin(position)]] vec4<f32> {
  return vec4<f32>();
}
)";

  auto* expect = R"(
[[builtin(pointsize), internal(disable_validation__ignore_storage_class)]] var<out> tint_pointsize : f32;

[[builtin(position), internal(disable_validation__ignore_storage_class)]] var<out> tint_symbol_1 : vec4<f32>;

fn tint_symbol_2(tint_symbol : vec4<f32>) {
  tint_symbol_1 = tint_symbol;
}

[[stage(vertex)]]
fn main1() {
  tint_pointsize = 1.0;
  tint_symbol_2(vec4<f32>());
  return;
}

[[builtin(position), internal(disable_validation__ignore_storage_class)]] var<out> tint_symbol_4 : vec4<f32>;

fn tint_symbol_5(tint_symbol_3 : vec4<f32>) {
  tint_symbol_4 = tint_symbol_3;
}

[[stage(vertex)]]
fn main2() {
  tint_pointsize = 1.0;
  tint_symbol_5(vec4<f32>());
  return;
}

[[builtin(position), internal(disable_validation__ignore_storage_class)]] var<out> tint_symbol_7 : vec4<f32>;

fn tint_symbol_8(tint_symbol_6 : vec4<f32>) {
  tint_symbol_7 = tint_symbol_6;
}

[[stage(vertex)]]
fn main3() {
  tint_pointsize = 1.0;
  tint_symbol_8(vec4<f32>());
  return;
}
)";

  DataMap data;
  data.Add<Spirv::Config>(true);
  auto got = Run<Spirv>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, EmitVertexPointSize_NoVertexShaders) {
  auto* src = R"(
[[stage(compute), workgroup_size(8, 1, 1)]]
fn main() {
}
)";

  DataMap data;
  data.Add<Spirv::Config>(true);
  auto got = Run<Spirv>(src, data);

  EXPECT_EQ(src, str(got));
}

TEST_F(SpirvTest, AddEmptyEntryPoint) {
  auto* src = R"()";

  auto* expect = R"(
[[stage(compute), workgroup_size(1)]]
fn unused_entry_point() {
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

// Test that different transforms within the sanitizer interact correctly.
TEST_F(SpirvTest, MultipleTransforms) {
  auto* src = R"(
[[stage(vertex)]]
fn vert_main() -> [[builtin(position)]] vec4<f32> {
  return vec4<f32>();
}

[[stage(fragment)]]
fn frag_main([[builtin(sample_index)]] sample_index : u32,
        [[builtin(sample_mask)]] mask_in : u32)
        -> [[builtin(sample_mask)]] u32 {
  return mask_in;
}
)";

  auto* expect = R"(
[[builtin(pointsize), internal(disable_validation__ignore_storage_class)]] var<out> tint_pointsize : f32;

[[builtin(position), internal(disable_validation__ignore_storage_class)]] var<out> tint_symbol_1 : vec4<f32>;

fn tint_symbol_2(tint_symbol : vec4<f32>) {
  tint_symbol_1 = tint_symbol;
}

[[stage(vertex)]]
fn vert_main() {
  tint_pointsize = 1.0;
  tint_symbol_2(vec4<f32>());
  return;
}

[[builtin(sample_index), internal(disable_validation__ignore_storage_class)]] var<in> tint_symbol_3 : u32;

[[builtin(sample_mask), internal(disable_validation__ignore_storage_class)]] var<in> tint_symbol_4 : array<u32, 1>;

[[builtin(sample_mask), internal(disable_validation__ignore_storage_class)]] var<out> tint_symbol_6 : array<u32, 1>;

fn tint_symbol_7(tint_symbol_5 : u32) {
  tint_symbol_6[0] = tint_symbol_5;
}

[[stage(fragment)]]
fn frag_main() {
  tint_symbol_7(tint_symbol_4[0]);
  return;
}
)";

  DataMap data;
  data.Add<Spirv::Config>(true);
  auto got = Run<Spirv>(src, data);

  EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace transform
}  // namespace tint
