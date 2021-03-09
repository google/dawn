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

  auto got = Transform<Spirv>(src);

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

  auto got = Transform<Spirv>(src);

  EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace transform
}  // namespace tint
