// Copyright 2020 The Tint Authors.
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

#include "src/transform/first_index_offset.h"

#include <memory>
#include <utility>
#include <vector>

#include "src/transform/test_helper.h"

namespace tint {
namespace transform {
namespace {

using FirstIndexOffsetTest = TransformTest;

TEST_F(FirstIndexOffsetTest, Error_AlreadyTransformed) {
  auto* src = R"(
[[builtin(vertex_index)]] var<in> vert_idx : u32;

fn test() -> u32 {
  return vert_idx;
}

[[stage(vertex)]]
fn entry() -> void {
  test();
}
)";

  auto* expect =
      "error: First index offset transform has already been applied.";

  std::vector<std::unique_ptr<transform::Transform>> transforms;
  transforms.emplace_back(std::make_unique<FirstIndexOffset>(0, 0));
  transforms.emplace_back(std::make_unique<FirstIndexOffset>(1, 1));

  auto got = Run(src, std::move(transforms));

  EXPECT_EQ(expect, str(got));

  auto* data = got.data.Get<FirstIndexOffset::Data>();

  ASSERT_NE(data, nullptr);
  EXPECT_EQ(data->has_vertex_index, true);
  EXPECT_EQ(data->has_instance_index, false);
  EXPECT_EQ(data->first_vertex_offset, 0u);
  EXPECT_EQ(data->first_instance_offset, 0u);
}

TEST_F(FirstIndexOffsetTest, EmptyModule) {
  auto* src = "";
  auto* expect = "";

  auto got = Run(src, std::make_unique<FirstIndexOffset>(0, 0));

  EXPECT_EQ(expect, str(got));

  auto* data = got.data.Get<FirstIndexOffset::Data>();

  ASSERT_NE(data, nullptr);
  EXPECT_EQ(data->has_vertex_index, false);
  EXPECT_EQ(data->has_instance_index, false);
  EXPECT_EQ(data->first_vertex_offset, 0u);
  EXPECT_EQ(data->first_instance_offset, 0u);
}

TEST_F(FirstIndexOffsetTest, BasicModuleVertexIndex) {
  auto* src = R"(
[[builtin(vertex_index)]] var<in> vert_idx : u32;

fn test() -> u32 {
  return vert_idx;
}

[[stage(vertex)]]
fn entry() -> void {
  test();
}
)";

  auto* expect = R"(
[[block]]
struct TintFirstIndexOffsetData {
  tint_first_vertex_index : u32;
};

[[binding(1), group(2)]] var<uniform> tint_first_index_data : TintFirstIndexOffsetData;

[[builtin(vertex_index)]] var<in> tint_first_index_offset_vert_idx : u32;

fn test() -> u32 {
  const vert_idx : u32 = (tint_first_index_offset_vert_idx + tint_first_index_data.tint_first_vertex_index);
  return vert_idx;
}

[[stage(vertex)]]
fn entry() -> void {
  test();
}
)";

  auto got = Run(src, std::make_unique<FirstIndexOffset>(1, 2));

  EXPECT_EQ(expect, str(got));

  auto* data = got.data.Get<FirstIndexOffset::Data>();

  ASSERT_NE(data, nullptr);
  EXPECT_EQ(data->has_vertex_index, true);
  EXPECT_EQ(data->has_instance_index, false);
  EXPECT_EQ(data->first_vertex_offset, 0u);
  EXPECT_EQ(data->first_instance_offset, 0u);
}

TEST_F(FirstIndexOffsetTest, BasicModuleInstanceIndex) {
  auto* src = R"(
[[builtin(instance_index)]] var<in> inst_idx : u32;

fn test() -> u32 {
  return inst_idx;
}

[[stage(vertex)]]
fn entry() -> void {
  test();
}
)";

  auto* expect = R"(
[[block]]
struct TintFirstIndexOffsetData {
  tint_first_instance_index : u32;
};

[[binding(1), group(7)]] var<uniform> tint_first_index_data : TintFirstIndexOffsetData;

[[builtin(instance_index)]] var<in> tint_first_index_offset_inst_idx : u32;

fn test() -> u32 {
  const inst_idx : u32 = (tint_first_index_offset_inst_idx + tint_first_index_data.tint_first_instance_index);
  return inst_idx;
}

[[stage(vertex)]]
fn entry() -> void {
  test();
}
)";

  auto got = Run(src, std::make_unique<FirstIndexOffset>(1, 7));

  EXPECT_EQ(expect, str(got));

  auto* data = got.data.Get<FirstIndexOffset::Data>();

  ASSERT_NE(data, nullptr);
  EXPECT_EQ(data->has_vertex_index, false);
  EXPECT_EQ(data->has_instance_index, true);
  EXPECT_EQ(data->first_vertex_offset, 0u);
  EXPECT_EQ(data->first_instance_offset, 0u);
}

TEST_F(FirstIndexOffsetTest, BasicModuleBothIndex) {
  auto* src = R"(
[[builtin(instance_index)]] var<in> instance_idx : u32;
[[builtin(vertex_index)]] var<in> vert_idx : u32;

fn test() -> u32 {
  return instance_idx + vert_idx;
}

[[stage(vertex)]]
fn entry() -> void {
  test();
}
)";

  auto* expect = R"(
[[block]]
struct TintFirstIndexOffsetData {
  tint_first_vertex_index : u32;
  tint_first_instance_index : u32;
};

[[binding(1), group(2)]] var<uniform> tint_first_index_data : TintFirstIndexOffsetData;

[[builtin(instance_index)]] var<in> tint_first_index_offset_instance_idx : u32;

[[builtin(vertex_index)]] var<in> tint_first_index_offset_vert_idx : u32;

fn test() -> u32 {
  const instance_idx : u32 = (tint_first_index_offset_instance_idx + tint_first_index_data.tint_first_instance_index);
  const vert_idx : u32 = (tint_first_index_offset_vert_idx + tint_first_index_data.tint_first_vertex_index);
  return (instance_idx + vert_idx);
}

[[stage(vertex)]]
fn entry() -> void {
  test();
}
)";

  auto got = Run(src, std::make_unique<FirstIndexOffset>(1, 2));

  EXPECT_EQ(expect, str(got));

  auto* data = got.data.Get<FirstIndexOffset::Data>();

  ASSERT_NE(data, nullptr);
  EXPECT_EQ(data->has_vertex_index, true);
  EXPECT_EQ(data->has_instance_index, true);
  EXPECT_EQ(data->first_vertex_offset, 0u);
  EXPECT_EQ(data->first_instance_offset, 4u);
}

TEST_F(FirstIndexOffsetTest, NestedCalls) {
  auto* src = R"(
[[builtin(vertex_index)]] var<in> vert_idx : u32;

fn func1() -> u32 {
  return vert_idx;
}

fn func2() -> u32 {
  return func1();
}

[[stage(vertex)]]
fn entry() -> void {
  func2();
}
)";

  auto* expect = R"(
[[block]]
struct TintFirstIndexOffsetData {
  tint_first_vertex_index : u32;
};

[[binding(1), group(2)]] var<uniform> tint_first_index_data : TintFirstIndexOffsetData;

[[builtin(vertex_index)]] var<in> tint_first_index_offset_vert_idx : u32;

fn func1() -> u32 {
  const vert_idx : u32 = (tint_first_index_offset_vert_idx + tint_first_index_data.tint_first_vertex_index);
  return vert_idx;
}

fn func2() -> u32 {
  return func1();
}

[[stage(vertex)]]
fn entry() -> void {
  func2();
}
)";

  auto got = Run(src, std::make_unique<FirstIndexOffset>(1, 2));

  EXPECT_EQ(expect, str(got));

  auto* data = got.data.Get<FirstIndexOffset::Data>();

  ASSERT_NE(data, nullptr);
  EXPECT_EQ(data->has_vertex_index, true);
  EXPECT_EQ(data->has_instance_index, false);
  EXPECT_EQ(data->first_vertex_offset, 0u);
  EXPECT_EQ(data->first_instance_offset, 0u);
}

}  // namespace
}  // namespace transform
}  // namespace tint
