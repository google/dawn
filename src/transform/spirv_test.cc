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
[[stage(fragment)]]
fn main([[builtin(sample_index)]] sample_index : u32,
        [[builtin(sample_mask)]] mask_in : u32
        ) -> [[builtin(sample_mask)]] u32 {
  return mask_in;
}
)";

  auto* expect = R"(
[[builtin(sample_index), internal(disable_validation__ignore_storage_class)]] var<in> sample_index_1 : u32;

[[builtin(sample_mask), internal(disable_validation__ignore_storage_class)]] var<in> mask_in_1 : array<u32, 1>;

[[builtin(sample_mask), internal(disable_validation__ignore_storage_class)]] var<out> value : array<u32, 1>;

fn main_inner(sample_index : u32, mask_in : u32) -> u32 {
  return mask_in;
}

[[stage(fragment)]]
fn main() {
  let inner_result = main_inner(sample_index_1, mask_in_1[0]);
  value[0] = inner_result;
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
[[builtin(sample_mask), internal(disable_validation__ignore_storage_class)]] var<in> mask_in_1 : array<u32, 1>;

[[builtin(sample_mask), internal(disable_validation__ignore_storage_class)]] var<out> value : array<u32, 1>;

fn filter(mask : u32) -> u32 {
  return (mask & 3u);
}

fn set_mask(input : u32) -> u32 {
  return input;
}

fn main_inner(mask_in : u32) -> u32 {
  return set_mask(filter(mask_in));
}

[[stage(fragment)]]
fn main() {
  let inner_result = main_inner(mask_in_1[0]);
  value[0] = inner_result;
}
)";

  auto got = Run<Spirv>(src);

  EXPECT_EQ(expect, str(got));
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
[[builtin(position), internal(disable_validation__ignore_storage_class)]] var<out> value : vec4<f32>;

[[builtin(pointsize), internal(disable_validation__ignore_storage_class)]] var<out> vertex_point_size : f32;

[[builtin(sample_index), internal(disable_validation__ignore_storage_class)]] var<in> sample_index_1 : u32;

[[builtin(sample_mask), internal(disable_validation__ignore_storage_class)]] var<in> mask_in_1 : array<u32, 1>;

[[builtin(sample_mask), internal(disable_validation__ignore_storage_class)]] var<out> value_1 : array<u32, 1>;

fn vert_main_inner() -> vec4<f32> {
  return vec4<f32>();
}

[[stage(vertex)]]
fn vert_main() {
  let inner_result = vert_main_inner();
  value = inner_result;
  vertex_point_size = 1.0;
}

fn frag_main_inner(sample_index : u32, mask_in : u32) -> u32 {
  return mask_in;
}

[[stage(fragment)]]
fn frag_main() {
  let inner_result_1 = frag_main_inner(sample_index_1, mask_in_1[0]);
  value_1[0] = inner_result_1;
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
