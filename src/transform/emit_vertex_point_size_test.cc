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
fn entry() -> [[builtin(position)]] vec4<f32> {
  return vec4<f32>();
}

fn non_entry_b() {
}
)";

  auto* expect = R"(
struct tint_symbol {
  [[builtin(position)]]
  position : vec4<f32>;
  [[builtin(pointsize)]]
  tint_pointsize : f32;
};

fn non_entry_a() {
}

[[stage(vertex)]]
fn entry() -> tint_symbol {
  return tint_symbol(vec4<f32>(), 1.0);
}

fn non_entry_b() {
}
)";

  auto got = Run<EmitVertexPointSize>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(EmitVertexPointSizeTest, VertexStageBasic_Struct) {
  auto* src = R"(
struct VertexOut {
  [[builtin(position)]]
  pos : vec4<f32>;
  [[location(0)]]
  col : f32;
};

fn non_entry_a() {
}

[[stage(vertex)]]
fn entry() -> VertexOut {
  var output : VertexOut;
  output.pos = vec4<f32>();
  output.col = 0.5;
  return output;
}

fn non_entry_b() {
}
)";

  auto* expect = R"(
struct tint_symbol {
  [[builtin(position)]]
  pos : vec4<f32>;
  [[location(0)]]
  col : f32;
  [[builtin(pointsize)]]
  tint_pointsize : f32;
};

struct VertexOut {
  [[builtin(position)]]
  pos : vec4<f32>;
  [[location(0)]]
  col : f32;
};

fn non_entry_a() {
}

[[stage(vertex)]]
fn entry() -> tint_symbol {
  var output : VertexOut;
  output.pos = vec4<f32>();
  output.col = 0.5;
  return tint_symbol(output.pos, output.col, 1.0);
}

fn non_entry_b() {
}
)";

  auto got = Run<EmitVertexPointSize>(src);

  EXPECT_EQ(expect, str(got));
}

// Make sure we capture the function return value in a temporary instead of
// re-evaluating it multiple times.
TEST_F(EmitVertexPointSizeTest, VertexStage_ReturnStructFromFunctionCall) {
  auto* src = R"(
struct VertexOut {
  [[builtin(position)]]
  pos : vec4<f32>;
  [[location(0)]]
  col : f32;
};

fn foo() -> VertexOut {
  var output : VertexOut;
  output.pos = vec4<f32>();
  output.col = 0.5;
  return output;
}

[[stage(vertex)]]
fn entry() -> VertexOut {
  return foo();
}
)";

  auto* expect = R"(
struct tint_symbol {
  [[builtin(position)]]
  pos : vec4<f32>;
  [[location(0)]]
  col : f32;
  [[builtin(pointsize)]]
  tint_pointsize : f32;
};

struct VertexOut {
  [[builtin(position)]]
  pos : vec4<f32>;
  [[location(0)]]
  col : f32;
};

fn foo() -> VertexOut {
  var output : VertexOut;
  output.pos = vec4<f32>();
  output.col = 0.5;
  return output;
}

[[stage(vertex)]]
fn entry() -> tint_symbol {
  let tint_symbol_1 : VertexOut = foo();
  return tint_symbol(tint_symbol_1.pos, tint_symbol_1.col, 1.0);
}
)";

  auto got = Run<EmitVertexPointSize>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(EmitVertexPointSizeTest, VertexStage_MultipleReturnStatements) {
  auto* src = R"(
[[stage(vertex)]]
fn entry([[location(0)]] toggle : u32) -> [[builtin(position)]] vec4<f32> {
  if (toggle == 1u) {
    return vec4<f32>(0.5, 0.5, 0.5, 0.5);
  }
  return vec4<f32>(1.0, 1.0, 1.0, 1.0);
}
)";

  auto* expect = R"(
struct tint_symbol {
  [[builtin(position)]]
  position : vec4<f32>;
  [[builtin(pointsize)]]
  tint_pointsize : f32;
};

[[stage(vertex)]]
fn entry([[location(0)]] toggle : u32) -> tint_symbol {
  if ((toggle == 1u)) {
    return tint_symbol(vec4<f32>(0.5, 0.5, 0.5, 0.5), 1.0);
  }
  return tint_symbol(vec4<f32>(1.0, 1.0, 1.0, 1.0), 1.0);
}
)";

  auto got = Run<EmitVertexPointSize>(src);

  EXPECT_EQ(expect, str(got));
}

// Test that we re-use generated structures when we've seen the original return
// type before.
TEST_F(EmitVertexPointSizeTest, VertexStage_MultipleShaders) {
  auto* src = R"(
struct VertexOut {
  [[builtin(position)]]
  pos : vec4<f32>;
  [[location(0)]]
  col : f32;
};

[[stage(vertex)]]
fn entry1() -> [[builtin(position)]] vec4<f32> {
  return vec4<f32>();
}

[[stage(vertex)]]
fn entry2() -> [[builtin(position)]] vec4<f32> {
  return vec4<f32>(1.0, 1.0, 1.0, 1.0);
}

[[stage(vertex)]]
fn entry3() -> VertexOut {
  var output : VertexOut;
  output.pos = vec4<f32>();
  output.col = 0.5;
  return output;
}

[[stage(vertex)]]
fn entry4() -> VertexOut {
  var output : VertexOut;
  output.pos = vec4<f32>();
  output.col = 0.75;
  return output;
}

)";

  auto* expect = R"(
struct tint_symbol {
  [[builtin(position)]]
  position : vec4<f32>;
  [[builtin(pointsize)]]
  tint_pointsize : f32;
};

struct tint_symbol_1 {
  [[builtin(position)]]
  pos : vec4<f32>;
  [[location(0)]]
  col : f32;
  [[builtin(pointsize)]]
  tint_pointsize_1 : f32;
};

struct VertexOut {
  [[builtin(position)]]
  pos : vec4<f32>;
  [[location(0)]]
  col : f32;
};

[[stage(vertex)]]
fn entry1() -> tint_symbol {
  return tint_symbol(vec4<f32>(), 1.0);
}

[[stage(vertex)]]
fn entry2() -> tint_symbol {
  return tint_symbol(vec4<f32>(1.0, 1.0, 1.0, 1.0), 1.0);
}

[[stage(vertex)]]
fn entry3() -> tint_symbol_1 {
  var output : VertexOut;
  output.pos = vec4<f32>();
  output.col = 0.5;
  return tint_symbol_1(output.pos, output.col, 1.0);
}

[[stage(vertex)]]
fn entry4() -> tint_symbol_1 {
  var output : VertexOut;
  output.pos = vec4<f32>();
  output.col = 0.75;
  return tint_symbol_1(output.pos, output.col, 1.0);
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

TEST_F(EmitVertexPointSizeTest, AttemptSymbolCollision) {
  auto* src = R"(
struct VertexOut {
  [[builtin(position)]]
  tint_pointsize : vec4<f32>;
};

[[stage(vertex)]]
fn entry() -> VertexOut {
  return VertexOut(vec4<f32>());
}
)";

  auto* expect = R"(
struct tint_symbol {
  [[builtin(position)]]
  tint_pointsize : vec4<f32>;
  [[builtin(pointsize)]]
  tint_pointsize_1 : f32;
};

struct VertexOut {
  [[builtin(position)]]
  tint_pointsize : vec4<f32>;
};

[[stage(vertex)]]
fn entry() -> tint_symbol {
  let tint_symbol_1 : VertexOut = VertexOut(vec4<f32>());
  return tint_symbol(tint_symbol_1.tint_pointsize, 1.0);
}
)";

  auto got = Run<EmitVertexPointSize>(src);

  EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace transform
}  // namespace tint
