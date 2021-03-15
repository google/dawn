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

[[builtin(local_invocation_id)]] var<in> tint_symbol_6 : vec3<u32>;

[[builtin(local_invocation_index)]] var<in> tint_symbol_7 : u32;

[[stage(fragment)]]
fn frag_main() -> void {
  var col : f32 = (tint_symbol_1.x * tint_symbol_2);
}

[[stage(compute)]]
fn compute_main() -> void {
  var id_x : u32 = tint_symbol_6.x;
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

// Test that different transforms within the sanitizer interact correctly.
TEST_F(SpirvTest, MultipleTransforms) {
  // TODO(jrprice): Make `mask_out` a return value when supported.
  auto* src = R"(
[[builtin(sample_mask_out)]] var<out> mask_out : u32;

[[stage(fragment)]]
fn main([[builtin(sample_index)]] sample_index : u32,
        [[builtin(sample_mask_in)]] mask_in : u32) -> void {
  mask_out = mask_in;
}
)";

  auto* expect = R"(
[[builtin(sample_index)]] var<in> tint_symbol_1 : u32;

[[builtin(sample_mask_in)]] var<in> tint_symbol_2 : array<u32, 1>;

[[builtin(sample_mask_out)]] var<out> mask_out : array<u32, 1>;

[[stage(fragment)]]
fn main() -> void {
  mask_out[0] = tint_symbol_2[0];
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace transform
}  // namespace tint
