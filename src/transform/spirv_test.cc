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
fn frag_main([[builtin(frag_coord)]] coord : vec4<f32>,
             [[location(1)]] loc1 : f32) -> void {
  var col : f32 = (coord.x * loc1);
}

[[stage(compute)]]
fn compute_main([[builtin(local_invocation_id)]] local_id : vec3<u32>,
                [[builtin(local_invocation_index)]] local_index : u32) -> void {
  var id_x : u32 = local_id.x;
}
)";

  auto* expect = R"(
[[builtin(frag_coord)]] var<in> tint_symbol_1 : vec4<f32>;

[[location(1)]] var<in> tint_symbol_2 : f32;

[[stage(fragment)]]
fn frag_main() -> void {
  var col : f32 = (tint_symbol_1.x * tint_symbol_2);
}

[[builtin(local_invocation_id)]] var<in> tint_symbol_6 : vec3<u32>;

[[builtin(local_invocation_index)]] var<in> tint_symbol_7 : u32;

[[stage(compute)]]
fn compute_main() -> void {
  var id_x : u32 = tint_symbol_6.x;
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleEntryPointIOTypes_Parameter_TypeAlias) {
  auto* src = R"(
type myf32 = f32;

[[stage(fragment)]]
fn frag_main([[location(1)]] loc1 : myf32) -> void {
}
)";

  auto* expect = R"(
type myf32 = f32;

[[location(1)]] var<in> tint_symbol_1 : myf32;

[[stage(fragment)]]
fn frag_main() -> void {
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
[[builtin(position)]] var<out> tint_symbol_2 : vec4<f32>;

fn tint_symbol_3(tint_symbol_1 : vec4<f32>) -> void {
  tint_symbol_2 = tint_symbol_1;
}

[[stage(vertex)]]
fn vert_main() -> void {
  tint_symbol_3(vec4<f32>(1.0, 2.0, 3.0, 0.0));
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
[[location(0)]] var<in> tint_symbol_1 : u32;

[[location(0)]] var<out> tint_symbol_3 : f32;

fn tint_symbol_4(tint_symbol_2 : f32) -> void {
  tint_symbol_3 = tint_symbol_2;
}

[[stage(fragment)]]
fn frag_main() -> void {
  if ((tint_symbol_1 > 10u)) {
    tint_symbol_4(0.5);
    return;
  }
  tint_symbol_4(1.0);
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

[[location(0)]] var<in> tint_symbol_1 : u32;

[[location(0)]] var<out> tint_symbol_3 : myf32;

fn tint_symbol_5(tint_symbol_2 : myf32) -> void {
  tint_symbol_3 = tint_symbol_2;
}

[[stage(fragment)]]
fn frag_main() -> void {
  if ((tint_symbol_1 > 10u)) {
    tint_symbol_5(0.5);
    return;
  }
  tint_symbol_5(1.0);
  return;
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleEntryPointIOTypes_StructParameters) {
  auto* src = R"(
struct FragmentInput {
  [[builtin(frag_coord)]] coord : vec4<f32>;
  [[location(1)]] value : f32;
};

[[stage(fragment)]]
fn frag_main(inputs : FragmentInput) -> void {
  var col : f32 = inputs.coord.x * inputs.value;
}
)";

  auto* expect = R"(
struct FragmentInput {
  coord : vec4<f32>;
  value : f32;
};

[[builtin(frag_coord)]] var<in> tint_symbol_4 : vec4<f32>;

[[location(1)]] var<in> tint_symbol_5 : f32;

[[stage(fragment)]]
fn frag_main() -> void {
  const tint_symbol_6 : FragmentInput = FragmentInput(tint_symbol_4, tint_symbol_5);
  var col : f32 = (tint_symbol_6.coord.x * tint_symbol_6.value);
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleEntryPointIOTypes_StructParameters_Nested) {
  auto* src = R"(
struct Builtins {
  [[builtin(frag_coord)]] coord : vec4<f32>;
};

struct Locations {
  [[location(2)]] l2 : f32;
  [[location(3)]] l3 : f32;
};

struct Other {
  l : Locations;
};

struct FragmentInput {
  b : Builtins;
  o : Other;
  [[location(1)]] value : f32;
};

[[stage(fragment)]]
fn frag_main(inputs : FragmentInput) -> void {
  var col : f32 = inputs.b.coord.x * inputs.value;
  var l : f32 = inputs.o.l.l2 + inputs.o.l.l3;
}
)";

  auto* expect = R"(
struct Builtins {
  coord : vec4<f32>;
};

struct Locations {
  l2 : f32;
  l3 : f32;
};

struct Other {
  l : Locations;
};

struct FragmentInput {
  b : Builtins;
  o : Other;
  value : f32;
};

[[builtin(frag_coord)]] var<in> tint_symbol_12 : vec4<f32>;

[[location(2)]] var<in> tint_symbol_14 : f32;

[[location(3)]] var<in> tint_symbol_15 : f32;

[[location(1)]] var<in> tint_symbol_18 : f32;

[[stage(fragment)]]
fn frag_main() -> void {
  const tint_symbol_13 : Builtins = Builtins(tint_symbol_12);
  const tint_symbol_16 : Locations = Locations(tint_symbol_14, tint_symbol_15);
  const tint_symbol_17 : Other = Other(tint_symbol_16);
  const tint_symbol_19 : FragmentInput = FragmentInput(tint_symbol_13, tint_symbol_17, tint_symbol_18);
  var col : f32 = (tint_symbol_19.b.coord.x * tint_symbol_19.value);
  var l : f32 = (tint_symbol_19.o.l.l2 + tint_symbol_19.o.l.l3);
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleEntryPointIOTypes_StructParameters_EmptyBody) {
  auto* src = R"(
struct Locations {
  [[location(1)]] value : f32;
};

struct FragmentInput {
  locations : Locations;
};

[[stage(fragment)]]
fn frag_main(inputs : FragmentInput) -> void {
}
)";

  auto* expect = R"(
struct Locations {
  value : f32;
};

struct FragmentInput {
  locations : Locations;
};

[[location(1)]] var<in> tint_symbol_5 : f32;

[[stage(fragment)]]
fn frag_main() -> void {
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

[[builtin(position)]] var<out> tint_symbol_5 : vec4<f32>;

[[location(1)]] var<out> tint_symbol_6 : f32;

fn tint_symbol_7(tint_symbol_4 : VertexOutput) -> void {
  tint_symbol_5 = tint_symbol_4.pos;
  tint_symbol_6 = tint_symbol_4.value;
}

[[stage(vertex)]]
fn vert_main() -> void {
  if (false) {
    tint_symbol_7(VertexOutput());
    return;
  }
  var pos : vec4<f32> = vec4<f32>(1.0, 2.0, 3.0, 0.0);
  tint_symbol_7(VertexOutput(pos, 2.0));
  return;
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleEntryPointIOTypes_ReturnStruct_Nested) {
  auto* src = R"(
struct Builtins {
  [[builtin(position)]] pos : vec4<f32>;
};

struct Locations {
  [[location(2)]] l2 : f32;
  [[location(3)]] l3 : f32;
};

struct Other {
  l : Locations;
};

struct VertexOutput {
  b : Builtins;
  o : Other;
  [[location(1)]] value : f32;
};

[[stage(vertex)]]
fn vert_main() -> VertexOutput {
  if (false) {
    return VertexOutput();
  }
  var output : VertexOutput = VertexOutput();
  output.b.pos = vec4<f32>(1.0, 2.0, 3.0, 0.0);
  output.o.l.l2 = 4.0;
  output.o.l.l3 = 5.0;
  output.value = 6.0;
  return output;
}
)";

  auto* expect = R"(
struct Builtins {
  pos : vec4<f32>;
};

struct Locations {
  l2 : f32;
  l3 : f32;
};

struct Other {
  l : Locations;
};

struct VertexOutput {
  b : Builtins;
  o : Other;
  value : f32;
};

[[builtin(position)]] var<out> tint_symbol_13 : vec4<f32>;

[[location(2)]] var<out> tint_symbol_14 : f32;

[[location(3)]] var<out> tint_symbol_15 : f32;

[[location(1)]] var<out> tint_symbol_16 : f32;

fn tint_symbol_17(tint_symbol_12 : VertexOutput) -> void {
  tint_symbol_13 = tint_symbol_12.b.pos;
  tint_symbol_14 = tint_symbol_12.o.l.l2;
  tint_symbol_15 = tint_symbol_12.o.l.l3;
  tint_symbol_16 = tint_symbol_12.value;
}

[[stage(vertex)]]
fn vert_main() -> void {
  if (false) {
    tint_symbol_17(VertexOutput());
    return;
  }
  var output : VertexOutput = VertexOutput();
  output.b.pos = vec4<f32>(1.0, 2.0, 3.0, 0.0);
  output.o.l.l2 = 4.0;
  output.o.l.l3 = 5.0;
  output.value = 6.0;
  tint_symbol_17(output);
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

[[stage(vertex)]]
fn vert_main(inputs : Interface) -> Interface {
  return inputs;
}
)";

  auto* expect = R"(
struct Interface {
  value : f32;
};

[[location(1)]] var<in> tint_symbol_3 : f32;

[[location(1)]] var<out> tint_symbol_6 : f32;

fn tint_symbol_7(tint_symbol_5 : Interface) -> void {
  tint_symbol_6 = tint_symbol_5.value;
}

[[stage(vertex)]]
fn vert_main() -> void {
  const tint_symbol_4 : Interface = Interface(tint_symbol_3);
  tint_symbol_7(tint_symbol_4);
  return;
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleEntryPointIOTypes_SharedStruct_DifferentShaders) {
  auto* src = R"(
struct Interface {
  [[location(1)]] value : f32;
};

[[stage(vertex)]]
fn vert_main() -> Interface {
  return Interface(42.0);
}

[[stage(fragment)]]
fn frag_main(inputs : Interface) -> void {
  var x : f32 = inputs.value;
}
)";

  auto* expect = R"(
struct Interface {
  value : f32;
};

[[location(1)]] var<out> tint_symbol_4 : f32;

fn tint_symbol_5(tint_symbol_3 : Interface) -> void {
  tint_symbol_4 = tint_symbol_3.value;
}

[[stage(vertex)]]
fn vert_main() -> void {
  tint_symbol_5(Interface(42.0));
  return;
}

[[location(1)]] var<in> tint_symbol_7 : f32;

[[stage(fragment)]]
fn frag_main() -> void {
  const tint_symbol_8 : Interface = Interface(tint_symbol_7);
  var x : f32 = tint_symbol_8.value;
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleEntryPointIOTypes_SharedSubStruct) {
  auto* src = R"(
struct Interface {
  [[location(1)]] value : f32;
};

struct VertexOutput {
  [[builtin(position)]] pos : vec4<f32>;
  interface : Interface;
};

struct FragmentInput {
  [[builtin(sample_index)]] index : u32;
  interface : Interface;
};

[[stage(vertex)]]
fn vert_main() -> VertexOutput {
  return VertexOutput(vec4<f32>(), Interface(42.0));
}

[[stage(fragment)]]
fn frag_main(inputs : FragmentInput) -> void {
  var x : f32 = inputs.interface.value;
}
)";

  auto* expect = R"(
struct Interface {
  value : f32;
};

struct VertexOutput {
  pos : vec4<f32>;
  interface : Interface;
};

struct FragmentInput {
  index : u32;
  interface : Interface;
};

[[builtin(position)]] var<out> tint_symbol_9 : vec4<f32>;

[[location(1)]] var<out> tint_symbol_10 : f32;

fn tint_symbol_11(tint_symbol_8 : VertexOutput) -> void {
  tint_symbol_9 = tint_symbol_8.pos;
  tint_symbol_10 = tint_symbol_8.interface.value;
}

[[stage(vertex)]]
fn vert_main() -> void {
  tint_symbol_11(VertexOutput(vec4<f32>(), Interface(42.0)));
  return;
}

[[builtin(sample_index)]] var<in> tint_symbol_13 : u32;

[[location(1)]] var<in> tint_symbol_14 : f32;

[[stage(fragment)]]
fn frag_main() -> void {
  const tint_symbol_15 : Interface = Interface(tint_symbol_14);
  const tint_symbol_16 : FragmentInput = FragmentInput(tint_symbol_13, tint_symbol_15);
  var x : f32 = tint_symbol_16.interface.value;
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleEntryPointIOTypes_NestedStruct_TypeAlias) {
  auto* src = R"(
type myf32 = f32;

struct Location {
  [[location(2)]] l2 : myf32;
};

type MyLocation = Location;

struct VertexIO {
  l : MyLocation;
  [[location(1)]] value : myf32;
};

type MyVertexInput = VertexIO;

type MyVertexOutput = VertexIO;

[[stage(vertex)]]
fn vert_main(inputs : MyVertexInput) -> MyVertexOutput {
  return inputs;
}
)";

  auto* expect = R"(
type myf32 = f32;

struct Location {
  l2 : myf32;
};

type MyLocation = Location;

struct VertexIO {
  l : MyLocation;
  value : myf32;
};

type MyVertexInput = VertexIO;

type MyVertexOutput = VertexIO;

[[location(2)]] var<in> tint_symbol_8 : myf32;

[[location(1)]] var<in> tint_symbol_10 : myf32;

[[location(2)]] var<out> tint_symbol_14 : myf32;

[[location(1)]] var<out> tint_symbol_15 : myf32;

fn tint_symbol_17(tint_symbol_13 : MyVertexOutput) -> void {
  tint_symbol_14 = tint_symbol_13.l.l2;
  tint_symbol_15 = tint_symbol_13.value;
}

[[stage(vertex)]]
fn vert_main() -> void {
  const tint_symbol_9 : MyLocation = MyLocation(tint_symbol_8);
  const tint_symbol_11 : MyVertexInput = MyVertexInput(tint_symbol_9, tint_symbol_10);
  tint_symbol_17(tint_symbol_11);
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

[[location(1)]] var<in> tint_symbol_5 : f32;

[[builtin(frag_coord)]] var<in> tint_symbol_6 : vec4<f32>;

[[location(1)]] var<out> tint_symbol_9 : f32;

fn tint_symbol_10(tint_symbol_8 : FragmentOutput) -> void {
  tint_symbol_9 = tint_symbol_8.value;
}

[[stage(fragment)]]
fn frag_main() -> void {
  const tint_symbol_7 : FragmentInput = FragmentInput(tint_symbol_5, tint_symbol_6);
  tint_symbol_10(FragmentOutput((tint_symbol_7.coord.x * tint_symbol_7.value)));
  return;
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleSampleMaskBuiltins_Basic) {
  auto* src = R"(
[[builtin(sample_index)]] var<in> sample_index : u32;

[[builtin(sample_mask_in)]] var<in> mask_in : u32;

[[builtin(sample_mask_out)]] var<out> mask_out : u32;

[[stage(fragment)]]
fn main() -> void {
  mask_out = mask_in;
}
)";

  auto* expect = R"(
[[builtin(sample_index)]] var<in> sample_index : u32;

[[builtin(sample_mask_in)]] var<in> mask_in : array<u32, 1>;

[[builtin(sample_mask_out)]] var<out> mask_out : array<u32, 1>;

[[stage(fragment)]]
fn main() -> void {
  mask_out[0] = mask_in[0];
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, HandleSampleMaskBuiltins_FunctionArg) {
  auto* src = R"(
[[builtin(sample_mask_in)]] var<in> mask_in : u32;

[[builtin(sample_mask_out)]] var<out> mask_out : u32;

fn filter(mask: u32) -> u32 {
  return (mask & 3u);
}

fn set_mask(input : u32) -> void {
  mask_out = input;
}

[[stage(fragment)]]
fn main() -> void {
  set_mask(filter(mask_in));
}
)";

  auto* expect = R"(
[[builtin(sample_mask_in)]] var<in> mask_in : array<u32, 1>;

[[builtin(sample_mask_out)]] var<out> mask_out : array<u32, 1>;

fn filter(mask : u32) -> u32 {
  return (mask & 3u);
}

fn set_mask(input : u32) -> void {
  mask_out[0] = input;
}

[[stage(fragment)]]
fn main() -> void {
  set_mask(filter(mask_in[0]));
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SpirvTest, AddEmptyEntryPoint) {
  auto* src = R"()";

  auto* expect = R"(
[[stage(compute)]]
fn _tint_unused_entry_point() -> void {
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

// Test that different transforms within the sanitizer interact correctly.
TEST_F(SpirvTest, MultipleTransforms) {
  auto* src = R"(
[[stage(fragment)]]
fn main([[builtin(sample_index)]] sample_index : u32,
        [[builtin(sample_mask_in)]] mask_in : u32)
        -> [[builtin(sample_mask_out)]] u32 {
  return mask_in;
}
)";

  auto* expect = R"(
[[builtin(sample_index)]] var<in> tint_symbol_1 : u32;

[[builtin(sample_mask_in)]] var<in> tint_symbol_2 : array<u32, 1>;

[[builtin(sample_mask_out)]] var<out> tint_symbol_4 : array<u32, 1>;

fn tint_symbol_5(tint_symbol_3 : u32) -> void {
  tint_symbol_4[0] = tint_symbol_3;
}

[[stage(fragment)]]
fn main() -> void {
  tint_symbol_5(tint_symbol_2[0]);
  return;
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace transform
}  // namespace tint
