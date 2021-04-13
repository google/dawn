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

#include "src/transform/renamer.h"

#include "gmock/gmock.h"
#include "src/transform/test_helper.h"

namespace tint {
namespace transform {
namespace {

using ::testing::ContainerEq;

using RenamerTest = TransformTest;

TEST_F(RenamerTest, EmptyModule) {
  auto* src = "";
  auto* expect = "";

  auto got = Run<Renamer>(src);

  EXPECT_EQ(expect, str(got));

  auto* data = got.data.Get<Renamer::Data>();

  ASSERT_EQ(data->remappings.size(), 0u);
}

TEST_F(RenamerTest, BasicModuleVertexIndex) {
  auto* src = R"(
[[builtin(vertex_index)]] var<in> vert_idx : u32;

fn test() -> u32 {
  return vert_idx;
}

[[stage(vertex)]]
fn entry() {
  test();
}
)";

  auto* expect = R"(
[[builtin(vertex_index)]] var<in> tint_symbol : u32;

fn tint_symbol_1() -> u32 {
  return tint_symbol;
}

[[stage(vertex)]]
fn tint_symbol_2() {
  tint_symbol_1();
}
)";

  auto got = Run<Renamer>(src);

  EXPECT_EQ(expect, str(got));

  auto* data = got.data.Get<Renamer::Data>();

  ASSERT_NE(data, nullptr);
  Renamer::Data::Remappings expected_remappings = {
      {"vert_idx", "tint_symbol"},
      {"test", "tint_symbol_1"},
      {"entry", "tint_symbol_2"},
  };
  EXPECT_THAT(data->remappings, ContainerEq(expected_remappings));
}

TEST_F(RenamerTest, PreserveSwizzles) {
  auto* src = R"(
[[stage(vertex)]]
fn entry() -> [[builtin(position)]] vec4<f32> {
  var v : vec4<f32>;
  var rgba : f32;
  var xyzw : f32;
  return v.zyxw + v.rgab;
}
)";

  auto* expect = R"(
[[stage(vertex)]]
fn tint_symbol() -> [[builtin(position)]] vec4<f32> {
  var tint_symbol_1 : vec4<f32>;
  var tint_symbol_2 : f32;
  var tint_symbol_3 : f32;
  return (tint_symbol_1.zyxw + tint_symbol_1.rgab);
}
)";

  auto got = Run<Renamer>(src);

  EXPECT_EQ(expect, str(got));

  auto* data = got.data.Get<Renamer::Data>();

  ASSERT_NE(data, nullptr);
  Renamer::Data::Remappings expected_remappings = {
      {"entry", "tint_symbol"},
      {"v", "tint_symbol_1"},
      {"rgba", "tint_symbol_2"},
      {"xyzw", "tint_symbol_3"},
  };
  EXPECT_THAT(data->remappings, ContainerEq(expected_remappings));
}

TEST_F(RenamerTest, PreserveIntrinsics) {
  auto* src = R"(
[[stage(vertex)]]
fn entry() -> [[builtin(position)]] vec4<f32> {
  var blah : vec4<f32>;
  return abs(blah);
}
)";

  auto* expect = R"(
[[stage(vertex)]]
fn tint_symbol() -> [[builtin(position)]] vec4<f32> {
  var tint_symbol_1 : vec4<f32>;
  return abs(tint_symbol_1);
}
)";

  auto got = Run<Renamer>(src);

  EXPECT_EQ(expect, str(got));

  auto* data = got.data.Get<Renamer::Data>();

  ASSERT_NE(data, nullptr);
  Renamer::Data::Remappings expected_remappings = {
      {"entry", "tint_symbol"},
      {"blah", "tint_symbol_1"},
  };
  EXPECT_THAT(data->remappings, ContainerEq(expected_remappings));
}

TEST_F(RenamerTest, AttemptSymbolCollision) {
  auto* src = R"(
[[stage(vertex)]]
fn entry() -> [[builtin(position)]] vec4<f32> {
  var tint_symbol : vec4<f32>;
  var tint_symbol_2 : vec4<f32>;
  var tint_symbol_4 : vec4<f32>;
  return tint_symbol + tint_symbol_2 + tint_symbol_4;
}
)";

  auto* expect = R"(
[[stage(vertex)]]
fn tint_symbol() -> [[builtin(position)]] vec4<f32> {
  var tint_symbol_1 : vec4<f32>;
  var tint_symbol_2 : vec4<f32>;
  var tint_symbol_3 : vec4<f32>;
  return ((tint_symbol_1 + tint_symbol_2) + tint_symbol_3);
}
)";

  auto got = Run<Renamer>(src);

  EXPECT_EQ(expect, str(got));

  auto* data = got.data.Get<Renamer::Data>();

  ASSERT_NE(data, nullptr);
  Renamer::Data::Remappings expected_remappings = {
      {"entry", "tint_symbol"},
      {"tint_symbol", "tint_symbol_1"},
      {"tint_symbol_2", "tint_symbol_2"},
      {"tint_symbol_4", "tint_symbol_3"},
  };
  EXPECT_THAT(data->remappings, ContainerEq(expected_remappings));
}

}  // namespace
}  // namespace transform
}  // namespace tint
