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

#include "src/transform/emit_vertex_point_size.h"

#include "src/transform/test_helper.h"

namespace tint {
namespace transform {
namespace {

using EmitVertexPointSizeTest = TransformTest;

TEST_F(EmitVertexPointSizeTest, VertexStageBasic) {
  auto* src = R"(
fn non_entry_a() {
}

[[stage(vertex)]]
fn entry() {
  var builtin_assignments_should_happen_before_this : f32;
}

fn non_entry_b() {
}
)";

  auto* expect = R"(
[[builtin(pointsize)]] var<out> tint_pointsize : f32;

fn non_entry_a() {
}

[[stage(vertex)]]
fn entry() {
  tint_pointsize = 1.0;
  var builtin_assignments_should_happen_before_this : f32;
}

fn non_entry_b() {
}
)";

  auto got = Run<EmitVertexPointSize>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(EmitVertexPointSizeTest, VertexStageEmpty) {
  auto* src = R"(
fn non_entry_a() {
}

[[stage(vertex)]]
fn entry() {
}

fn non_entry_b() {
}
)";

  auto* expect = R"(
[[builtin(pointsize)]] var<out> tint_pointsize : f32;

fn non_entry_a() {
}

[[stage(vertex)]]
fn entry() {
  tint_pointsize = 1.0;
}

fn non_entry_b() {
}
)";

  auto got = Run<EmitVertexPointSize>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(EmitVertexPointSizeTest, NonVertexStage) {
  auto* src = R"(
[[stage(fragment)]]
fn fragment_entry() {
}

[[stage(compute)]]
fn compute_entry() {
}
)";

  auto* expect = R"(
[[stage(fragment)]]
fn fragment_entry() {
}

[[stage(compute)]]
fn compute_entry() {
}
)";

  auto got = Run<EmitVertexPointSize>(src);

  EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace transform
}  // namespace tint
