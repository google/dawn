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

#include "src/transform/vertex_pulling.h"

#include <utility>

#include "src/transform/test_helper.h"

namespace tint {
namespace transform {
namespace {

using VertexPullingTest = TransformTest;

TEST_F(VertexPullingTest, Error_NoEntryPoint) {
  auto* src = "";

  auto* expect = "error: Vertex stage entry point not found";

  DataMap data;
  data.Add<VertexPulling::Config>();
  auto got = Run<VertexPulling>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(VertexPullingTest, Error_InvalidEntryPoint) {
  auto* src = R"(
[[stage(vertex)]]
fn main() -> [[builtin(position)]] vec4<f32> {
  return vec4<f32>();
}
)";

  auto* expect = "error: Vertex stage entry point not found";

  VertexPulling::Config cfg;
  cfg.entry_point_name = "_";

  DataMap data;
  data.Add<VertexPulling::Config>(cfg);
  auto got = Run<VertexPulling>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(VertexPullingTest, Error_EntryPointWrongStage) {
  auto* src = R"(
[[stage(fragment)]]
fn main() {}
)";

  auto* expect = "error: Vertex stage entry point not found";

  VertexPulling::Config cfg;
  cfg.entry_point_name = "main";

  DataMap data;
  data.Add<VertexPulling::Config>(cfg);
  auto got = Run<VertexPulling>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(VertexPullingTest, BasicModule) {
  auto* src = R"(
[[stage(vertex)]]
fn main() -> [[builtin(position)]] vec4<f32> {
  return vec4<f32>();
}
)";

  auto* expect = R"(
[[block]]
struct TintVertexData {
  tint_vertex_data : [[stride(4)]] array<u32>;
};

[[stage(vertex)]]
fn main() -> [[builtin(position)]] vec4<f32> {
  {
    var tint_pulling_pos : u32;
  }
  return vec4<f32>();
}
)";

  VertexPulling::Config cfg;
  cfg.entry_point_name = "main";

  DataMap data;
  data.Add<VertexPulling::Config>(cfg);
  auto got = Run<VertexPulling>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(VertexPullingTest, OneAttribute) {
  auto* src = R"(
[[stage(vertex)]]
fn main([[location(0)]] var_a : f32) -> [[builtin(position)]] vec4<f32> {
  return vec4<f32>(var_a, 0.0, 0.0, 1.0);
}
)";

  auto* expect = R"(
[[block]]
struct TintVertexData {
  tint_vertex_data : [[stride(4)]] array<u32>;
};

[[binding(0), group(4)]] var<storage> tint_pulling_vertex_buffer_0 : [[access(read)]] TintVertexData;

[[stage(vertex)]]
fn main([[builtin(vertex_index)]] tint_pulling_vertex_index : u32) -> [[builtin(position)]] vec4<f32> {
  var var_a : f32;
  {
    var tint_pulling_pos : u32;
    tint_pulling_pos = ((tint_pulling_vertex_index * 4u) + 0u);
    var_a = bitcast<f32>(tint_pulling_vertex_buffer_0.tint_vertex_data[(tint_pulling_pos / 4u)]);
  }
  return vec4<f32>(var_a, 0.0, 0.0, 1.0);
}
)";

  VertexPulling::Config cfg;
  cfg.vertex_state = {
      {{4, InputStepMode::kVertex, {{VertexFormat::kF32, 0, 0}}}}};
  cfg.entry_point_name = "main";

  DataMap data;
  data.Add<VertexPulling::Config>(cfg);
  auto got = Run<VertexPulling>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(VertexPullingTest, OneInstancedAttribute) {
  auto* src = R"(
[[stage(vertex)]]
fn main([[location(0)]] var_a : f32) -> [[builtin(position)]] vec4<f32> {
  return vec4<f32>(var_a, 0.0, 0.0, 1.0);
}
)";

  auto* expect = R"(
[[block]]
struct TintVertexData {
  tint_vertex_data : [[stride(4)]] array<u32>;
};

[[binding(0), group(4)]] var<storage> tint_pulling_vertex_buffer_0 : [[access(read)]] TintVertexData;

[[stage(vertex)]]
fn main([[builtin(instance_index)]] tint_pulling_instance_index : u32) -> [[builtin(position)]] vec4<f32> {
  var var_a : f32;
  {
    var tint_pulling_pos : u32;
    tint_pulling_pos = ((tint_pulling_instance_index * 4u) + 0u);
    var_a = bitcast<f32>(tint_pulling_vertex_buffer_0.tint_vertex_data[(tint_pulling_pos / 4u)]);
  }
  return vec4<f32>(var_a, 0.0, 0.0, 1.0);
}
)";

  VertexPulling::Config cfg;
  cfg.vertex_state = {
      {{4, InputStepMode::kInstance, {{VertexFormat::kF32, 0, 0}}}}};
  cfg.entry_point_name = "main";

  DataMap data;
  data.Add<VertexPulling::Config>(cfg);
  auto got = Run<VertexPulling>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(VertexPullingTest, OneAttributeDifferentOutputSet) {
  auto* src = R"(
[[stage(vertex)]]
fn main([[location(0)]] var_a : f32) -> [[builtin(position)]] vec4<f32> {
  return vec4<f32>(var_a, 0.0, 0.0, 1.0);
}
)";

  auto* expect = R"(
[[block]]
struct TintVertexData {
  tint_vertex_data : [[stride(4)]] array<u32>;
};

[[binding(0), group(5)]] var<storage> tint_pulling_vertex_buffer_0 : [[access(read)]] TintVertexData;

[[stage(vertex)]]
fn main([[builtin(vertex_index)]] tint_pulling_vertex_index : u32) -> [[builtin(position)]] vec4<f32> {
  var var_a : f32;
  {
    var tint_pulling_pos : u32;
    tint_pulling_pos = ((tint_pulling_vertex_index * 4u) + 0u);
    var_a = bitcast<f32>(tint_pulling_vertex_buffer_0.tint_vertex_data[(tint_pulling_pos / 4u)]);
  }
  return vec4<f32>(var_a, 0.0, 0.0, 1.0);
}
)";

  VertexPulling::Config cfg;
  cfg.vertex_state = {
      {{4, InputStepMode::kVertex, {{VertexFormat::kF32, 0, 0}}}}};
  cfg.pulling_group = 5;
  cfg.entry_point_name = "main";

  DataMap data;
  data.Add<VertexPulling::Config>(cfg);
  auto got = Run<VertexPulling>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(VertexPullingTest, OneAttribute_Struct) {
  auto* src = R"(
struct Inputs {
  [[location(0)]] var_a : f32;
};

[[stage(vertex)]]
fn main(inputs : Inputs) -> [[builtin(position)]] vec4<f32> {
  return vec4<f32>(inputs.var_a, 0.0, 0.0, 1.0);
}
)";

  auto* expect = R"(
[[block]]
struct TintVertexData {
  tint_vertex_data : [[stride(4)]] array<u32>;
};

[[binding(0), group(4)]] var<storage> tint_pulling_vertex_buffer_0 : [[access(read)]] TintVertexData;

struct Inputs {
  [[location(0)]]
  var_a : f32;
};

[[stage(vertex)]]
fn main([[builtin(vertex_index)]] tint_pulling_vertex_index : u32) -> [[builtin(position)]] vec4<f32> {
  var inputs : Inputs;
  {
    var tint_pulling_pos : u32;
    tint_pulling_pos = ((tint_pulling_vertex_index * 4u) + 0u);
    inputs.var_a = bitcast<f32>(tint_pulling_vertex_buffer_0.tint_vertex_data[(tint_pulling_pos / 4u)]);
  }
  return vec4<f32>(inputs.var_a, 0.0, 0.0, 1.0);
}
)";

  VertexPulling::Config cfg;
  cfg.vertex_state = {
      {{4, InputStepMode::kVertex, {{VertexFormat::kF32, 0, 0}}}}};
  cfg.entry_point_name = "main";

  DataMap data;
  data.Add<VertexPulling::Config>(cfg);
  auto got = Run<VertexPulling>(src, data);

  EXPECT_EQ(expect, str(got));
}

// We expect the transform to use an existing builtin variables if it finds them
TEST_F(VertexPullingTest, ExistingVertexIndexAndInstanceIndex) {
  auto* src = R"(
[[stage(vertex)]]
fn main([[location(0)]] var_a : f32,
        [[location(1)]] var_b : f32,
        [[builtin(vertex_index)]] custom_vertex_index : u32,
        [[builtin(instance_index)]] custom_instance_index : u32
        ) -> [[builtin(position)]] vec4<f32> {
  return vec4<f32>(var_a, var_b, 0.0, 1.0);
}
)";

  auto* expect = R"(
[[block]]
struct TintVertexData {
  tint_vertex_data : [[stride(4)]] array<u32>;
};

[[binding(0), group(4)]] var<storage> tint_pulling_vertex_buffer_0 : [[access(read)]] TintVertexData;

[[binding(1), group(4)]] var<storage> tint_pulling_vertex_buffer_1 : [[access(read)]] TintVertexData;

[[stage(vertex)]]
fn main([[builtin(vertex_index)]] custom_vertex_index : u32, [[builtin(instance_index)]] custom_instance_index : u32) -> [[builtin(position)]] vec4<f32> {
  var var_a : f32;
  var var_b : f32;
  {
    var tint_pulling_pos : u32;
    tint_pulling_pos = ((custom_vertex_index * 4u) + 0u);
    var_a = bitcast<f32>(tint_pulling_vertex_buffer_0.tint_vertex_data[(tint_pulling_pos / 4u)]);
    tint_pulling_pos = ((custom_instance_index * 4u) + 0u);
    var_b = bitcast<f32>(tint_pulling_vertex_buffer_1.tint_vertex_data[(tint_pulling_pos / 4u)]);
  }
  return vec4<f32>(var_a, var_b, 0.0, 1.0);
}
)";

  VertexPulling::Config cfg;
  cfg.vertex_state = {{
      {
          4,
          InputStepMode::kVertex,
          {{VertexFormat::kF32, 0, 0}},
      },
      {
          4,
          InputStepMode::kInstance,
          {{VertexFormat::kF32, 0, 1}},
      },
  }};
  cfg.entry_point_name = "main";

  DataMap data;
  data.Add<VertexPulling::Config>(cfg);
  auto got = Run<VertexPulling>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(VertexPullingTest, ExistingVertexIndexAndInstanceIndex_Struct) {
  auto* src = R"(
struct Inputs {
  [[location(0)]] var_a : f32;
  [[location(1)]] var_b : f32;
  [[builtin(vertex_index)]] custom_vertex_index : u32;
  [[builtin(instance_index)]] custom_instance_index : u32;
};

[[stage(vertex)]]
fn main(inputs : Inputs) -> [[builtin(position)]] vec4<f32> {
  return vec4<f32>(inputs.var_a, inputs.var_b, 0.0, 1.0);
}
)";

  auto* expect = R"(
[[block]]
struct TintVertexData {
  tint_vertex_data : [[stride(4)]] array<u32>;
};

[[binding(0), group(4)]] var<storage> tint_pulling_vertex_buffer_0 : [[access(read)]] TintVertexData;

[[binding(1), group(4)]] var<storage> tint_pulling_vertex_buffer_1 : [[access(read)]] TintVertexData;

struct tint_symbol {
  [[builtin(vertex_index)]]
  custom_vertex_index : u32;
  [[builtin(instance_index)]]
  custom_instance_index : u32;
};

struct Inputs {
  [[location(0)]]
  var_a : f32;
  [[location(1)]]
  var_b : f32;
  [[builtin(vertex_index)]]
  custom_vertex_index : u32;
  [[builtin(instance_index)]]
  custom_instance_index : u32;
};

[[stage(vertex)]]
fn main(tint_symbol_1 : tint_symbol) -> [[builtin(position)]] vec4<f32> {
  var inputs : Inputs;
  inputs.custom_vertex_index = tint_symbol_1.custom_vertex_index;
  inputs.custom_instance_index = tint_symbol_1.custom_instance_index;
  {
    var tint_pulling_pos : u32;
    tint_pulling_pos = ((inputs.custom_vertex_index * 4u) + 0u);
    inputs.var_a = bitcast<f32>(tint_pulling_vertex_buffer_0.tint_vertex_data[(tint_pulling_pos / 4u)]);
    tint_pulling_pos = ((inputs.custom_instance_index * 4u) + 0u);
    inputs.var_b = bitcast<f32>(tint_pulling_vertex_buffer_1.tint_vertex_data[(tint_pulling_pos / 4u)]);
  }
  return vec4<f32>(inputs.var_a, inputs.var_b, 0.0, 1.0);
}
)";

  VertexPulling::Config cfg;
  cfg.vertex_state = {{
      {
          4,
          InputStepMode::kVertex,
          {{VertexFormat::kF32, 0, 0}},
      },
      {
          4,
          InputStepMode::kInstance,
          {{VertexFormat::kF32, 0, 1}},
      },
  }};
  cfg.entry_point_name = "main";

  DataMap data;
  data.Add<VertexPulling::Config>(cfg);
  auto got = Run<VertexPulling>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(VertexPullingTest, ExistingVertexIndexAndInstanceIndex_SeparateStruct) {
  auto* src = R"(
struct Inputs {
  [[location(0)]] var_a : f32;
  [[location(1)]] var_b : f32;
};

struct Indices {
  [[builtin(vertex_index)]] custom_vertex_index : u32;
  [[builtin(instance_index)]] custom_instance_index : u32;
};

[[stage(vertex)]]
fn main(inputs : Inputs, indices : Indices) -> [[builtin(position)]] vec4<f32> {
  return vec4<f32>(inputs.var_a, inputs.var_b, 0.0, 1.0);
}
)";

  auto* expect = R"(
[[block]]
struct TintVertexData {
  tint_vertex_data : [[stride(4)]] array<u32>;
};

[[binding(0), group(4)]] var<storage> tint_pulling_vertex_buffer_0 : [[access(read)]] TintVertexData;

[[binding(1), group(4)]] var<storage> tint_pulling_vertex_buffer_1 : [[access(read)]] TintVertexData;

struct Inputs {
  [[location(0)]]
  var_a : f32;
  [[location(1)]]
  var_b : f32;
};

struct Indices {
  [[builtin(vertex_index)]]
  custom_vertex_index : u32;
  [[builtin(instance_index)]]
  custom_instance_index : u32;
};

[[stage(vertex)]]
fn main(indices : Indices) -> [[builtin(position)]] vec4<f32> {
  var inputs : Inputs;
  {
    var tint_pulling_pos : u32;
    tint_pulling_pos = ((indices.custom_vertex_index * 4u) + 0u);
    inputs.var_a = bitcast<f32>(tint_pulling_vertex_buffer_0.tint_vertex_data[(tint_pulling_pos / 4u)]);
    tint_pulling_pos = ((indices.custom_instance_index * 4u) + 0u);
    inputs.var_b = bitcast<f32>(tint_pulling_vertex_buffer_1.tint_vertex_data[(tint_pulling_pos / 4u)]);
  }
  return vec4<f32>(inputs.var_a, inputs.var_b, 0.0, 1.0);
}
)";

  VertexPulling::Config cfg;
  cfg.vertex_state = {{
      {
          4,
          InputStepMode::kVertex,
          {{VertexFormat::kF32, 0, 0}},
      },
      {
          4,
          InputStepMode::kInstance,
          {{VertexFormat::kF32, 0, 1}},
      },
  }};
  cfg.entry_point_name = "main";

  DataMap data;
  data.Add<VertexPulling::Config>(cfg);
  auto got = Run<VertexPulling>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(VertexPullingTest, TwoAttributesSameBuffer) {
  auto* src = R"(
[[stage(vertex)]]
fn main([[location(0)]] var_a : f32,
        [[location(1)]] var_b : vec4<f32>) -> [[builtin(position)]] vec4<f32> {
  return vec4<f32>();
}
)";

  auto* expect = R"(
[[block]]
struct TintVertexData {
  tint_vertex_data : [[stride(4)]] array<u32>;
};

[[binding(0), group(4)]] var<storage> tint_pulling_vertex_buffer_0 : [[access(read)]] TintVertexData;

[[stage(vertex)]]
fn main([[builtin(vertex_index)]] tint_pulling_vertex_index : u32) -> [[builtin(position)]] vec4<f32> {
  var var_a : f32;
  var var_b : vec4<f32>;
  {
    var tint_pulling_pos : u32;
    tint_pulling_pos = ((tint_pulling_vertex_index * 16u) + 0u);
    var_a = bitcast<f32>(tint_pulling_vertex_buffer_0.tint_vertex_data[(tint_pulling_pos / 4u)]);
    tint_pulling_pos = ((tint_pulling_vertex_index * 16u) + 0u);
    var_b = vec4<f32>(bitcast<f32>(tint_pulling_vertex_buffer_0.tint_vertex_data[((tint_pulling_pos + 0u) / 4u)]), bitcast<f32>(tint_pulling_vertex_buffer_0.tint_vertex_data[((tint_pulling_pos + 4u) / 4u)]), bitcast<f32>(tint_pulling_vertex_buffer_0.tint_vertex_data[((tint_pulling_pos + 8u) / 4u)]), bitcast<f32>(tint_pulling_vertex_buffer_0.tint_vertex_data[((tint_pulling_pos + 12u) / 4u)]));
  }
  return vec4<f32>();
}
)";

  VertexPulling::Config cfg;
  cfg.vertex_state = {
      {{16,
        InputStepMode::kVertex,
        {{VertexFormat::kF32, 0, 0}, {VertexFormat::kVec4F32, 0, 1}}}}};
  cfg.entry_point_name = "main";

  DataMap data;
  data.Add<VertexPulling::Config>(cfg);
  auto got = Run<VertexPulling>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(VertexPullingTest, FloatVectorAttributes) {
  auto* src = R"(
[[stage(vertex)]]
fn main([[location(0)]] var_a : vec2<f32>,
        [[location(1)]] var_b : vec3<f32>,
        [[location(2)]] var_c : vec4<f32>
        ) -> [[builtin(position)]] vec4<f32> {
  return vec4<f32>();
}
)";

  auto* expect = R"(
[[block]]
struct TintVertexData {
  tint_vertex_data : [[stride(4)]] array<u32>;
};

[[binding(0), group(4)]] var<storage> tint_pulling_vertex_buffer_0 : [[access(read)]] TintVertexData;

[[binding(1), group(4)]] var<storage> tint_pulling_vertex_buffer_1 : [[access(read)]] TintVertexData;

[[binding(2), group(4)]] var<storage> tint_pulling_vertex_buffer_2 : [[access(read)]] TintVertexData;

[[stage(vertex)]]
fn main([[builtin(vertex_index)]] tint_pulling_vertex_index : u32) -> [[builtin(position)]] vec4<f32> {
  var var_a : vec2<f32>;
  var var_b : vec3<f32>;
  var var_c : vec4<f32>;
  {
    var tint_pulling_pos : u32;
    tint_pulling_pos = ((tint_pulling_vertex_index * 8u) + 0u);
    var_a = vec2<f32>(bitcast<f32>(tint_pulling_vertex_buffer_0.tint_vertex_data[((tint_pulling_pos + 0u) / 4u)]), bitcast<f32>(tint_pulling_vertex_buffer_0.tint_vertex_data[((tint_pulling_pos + 4u) / 4u)]));
    tint_pulling_pos = ((tint_pulling_vertex_index * 12u) + 0u);
    var_b = vec3<f32>(bitcast<f32>(tint_pulling_vertex_buffer_1.tint_vertex_data[((tint_pulling_pos + 0u) / 4u)]), bitcast<f32>(tint_pulling_vertex_buffer_1.tint_vertex_data[((tint_pulling_pos + 4u) / 4u)]), bitcast<f32>(tint_pulling_vertex_buffer_1.tint_vertex_data[((tint_pulling_pos + 8u) / 4u)]));
    tint_pulling_pos = ((tint_pulling_vertex_index * 16u) + 0u);
    var_c = vec4<f32>(bitcast<f32>(tint_pulling_vertex_buffer_2.tint_vertex_data[((tint_pulling_pos + 0u) / 4u)]), bitcast<f32>(tint_pulling_vertex_buffer_2.tint_vertex_data[((tint_pulling_pos + 4u) / 4u)]), bitcast<f32>(tint_pulling_vertex_buffer_2.tint_vertex_data[((tint_pulling_pos + 8u) / 4u)]), bitcast<f32>(tint_pulling_vertex_buffer_2.tint_vertex_data[((tint_pulling_pos + 12u) / 4u)]));
  }
  return vec4<f32>();
}
)";

  VertexPulling::Config cfg;
  cfg.vertex_state = {{
      {8, InputStepMode::kVertex, {{VertexFormat::kVec2F32, 0, 0}}},
      {12, InputStepMode::kVertex, {{VertexFormat::kVec3F32, 0, 1}}},
      {16, InputStepMode::kVertex, {{VertexFormat::kVec4F32, 0, 2}}},
  }};
  cfg.entry_point_name = "main";

  DataMap data;
  data.Add<VertexPulling::Config>(cfg);
  auto got = Run<VertexPulling>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(VertexPullingTest, AttemptSymbolCollision) {
  auto* src = R"(
[[stage(vertex)]]
fn main([[location(0)]] var_a : f32,
        [[location(1)]] var_b : vec4<f32>) -> [[builtin(position)]] vec4<f32> {
  var tint_pulling_vertex_index : i32;
  var tint_pulling_vertex_buffer_0 : i32;
  var tint_vertex_data : i32;
  var tint_pulling_pos : i32;
  return vec4<f32>();
}
)";

  auto* expect = R"(
[[block]]
struct TintVertexData {
  tint_vertex_data_1 : [[stride(4)]] array<u32>;
};

[[binding(0), group(4)]] var<storage> tint_pulling_vertex_buffer_0_1 : [[access(read)]] TintVertexData;

[[stage(vertex)]]
fn main([[builtin(vertex_index)]] tint_pulling_vertex_index_1 : u32) -> [[builtin(position)]] vec4<f32> {
  var var_a : f32;
  var var_b : vec4<f32>;
  {
    var tint_pulling_pos_1 : u32;
    tint_pulling_pos_1 = ((tint_pulling_vertex_index_1 * 16u) + 0u);
    var_a = bitcast<f32>(tint_pulling_vertex_buffer_0_1.tint_vertex_data_1[(tint_pulling_pos_1 / 4u)]);
    tint_pulling_pos_1 = ((tint_pulling_vertex_index_1 * 16u) + 0u);
    var_b = vec4<f32>(bitcast<f32>(tint_pulling_vertex_buffer_0_1.tint_vertex_data_1[((tint_pulling_pos_1 + 0u) / 4u)]), bitcast<f32>(tint_pulling_vertex_buffer_0_1.tint_vertex_data_1[((tint_pulling_pos_1 + 4u) / 4u)]), bitcast<f32>(tint_pulling_vertex_buffer_0_1.tint_vertex_data_1[((tint_pulling_pos_1 + 8u) / 4u)]), bitcast<f32>(tint_pulling_vertex_buffer_0_1.tint_vertex_data_1[((tint_pulling_pos_1 + 12u) / 4u)]));
  }
  var tint_pulling_vertex_index : i32;
  var tint_pulling_vertex_buffer_0 : i32;
  var tint_vertex_data : i32;
  var tint_pulling_pos : i32;
  return vec4<f32>();
}
)";

  VertexPulling::Config cfg;
  cfg.vertex_state = {
      {{16,
        InputStepMode::kVertex,
        {{VertexFormat::kF32, 0, 0}, {VertexFormat::kVec4F32, 0, 1}}}}};
  cfg.entry_point_name = "main";

  DataMap data;
  data.Add<VertexPulling::Config>(cfg);
  auto got = Run<VertexPulling>(src, std::move(data));

  EXPECT_EQ(expect, str(got));
}

// TODO(crbug.com/tint/697): Remove this.
TEST_F(VertexPullingTest, OneAttributeDifferentOutputSet_Legacy) {
  auto* src = R"(
[[location(0)]] var<in> var_a : f32;

[[stage(vertex)]]
fn main() -> [[builtin(position)]] vec4<f32> {
  return vec4<f32>();
}
)";

  auto* expect = R"(
[[builtin(vertex_index)]] var<in> tint_pulling_vertex_index : u32;

[[block]]
struct TintVertexData {
  tint_vertex_data : [[stride(4)]] array<u32>;
};

[[binding(0), group(5)]] var<storage> tint_pulling_vertex_buffer_0 : [[access(read)]] TintVertexData;

var<private> var_a : f32;

[[stage(vertex)]]
fn main() -> [[builtin(position)]] vec4<f32> {
  {
    var tint_pulling_pos : u32;
    tint_pulling_pos = ((tint_pulling_vertex_index * 4u) + 0u);
    var_a = bitcast<f32>(tint_pulling_vertex_buffer_0.tint_vertex_data[(tint_pulling_pos / 4u)]);
  }
  return vec4<f32>();
}
)";

  VertexPulling::Config cfg;
  cfg.vertex_state = {
      {{4, InputStepMode::kVertex, {{VertexFormat::kF32, 0, 0}}}}};
  cfg.pulling_group = 5;
  cfg.entry_point_name = "main";

  DataMap data;
  data.Add<VertexPulling::Config>(cfg);
  auto got = Run<VertexPulling>(src, data);

  EXPECT_EQ(expect, str(got));
}

// TODO(crbug.com/tint/697): Remove this.
// We expect the transform to use an existing builtin variables if it finds them
TEST_F(VertexPullingTest, ExistingVertexIndexAndInstanceIndex_Legacy) {
  auto* src = R"(
[[location(0)]] var<in> var_a : f32;
[[location(1)]] var<in> var_b : f32;
[[builtin(vertex_index)]] var<in> custom_vertex_index : u32;
[[builtin(instance_index)]] var<in> custom_instance_index : u32;

[[stage(vertex)]]
fn main() -> [[builtin(position)]] vec4<f32> {
  return vec4<f32>();
}
)";

  auto* expect = R"(
[[block]]
struct TintVertexData {
  tint_vertex_data : [[stride(4)]] array<u32>;
};

[[binding(0), group(4)]] var<storage> tint_pulling_vertex_buffer_0 : [[access(read)]] TintVertexData;

[[binding(1), group(4)]] var<storage> tint_pulling_vertex_buffer_1 : [[access(read)]] TintVertexData;

var<private> var_a : f32;

var<private> var_b : f32;

[[builtin(vertex_index)]] var<in> custom_vertex_index : u32;

[[builtin(instance_index)]] var<in> custom_instance_index : u32;

[[stage(vertex)]]
fn main() -> [[builtin(position)]] vec4<f32> {
  {
    var tint_pulling_pos : u32;
    tint_pulling_pos = ((custom_vertex_index * 4u) + 0u);
    var_a = bitcast<f32>(tint_pulling_vertex_buffer_0.tint_vertex_data[(tint_pulling_pos / 4u)]);
    tint_pulling_pos = ((custom_instance_index * 4u) + 0u);
    var_b = bitcast<f32>(tint_pulling_vertex_buffer_1.tint_vertex_data[(tint_pulling_pos / 4u)]);
  }
  return vec4<f32>();
}
)";

  VertexPulling::Config cfg;
  cfg.vertex_state = {{
      {
          4,
          InputStepMode::kVertex,
          {{VertexFormat::kF32, 0, 0}},
      },
      {
          4,
          InputStepMode::kInstance,
          {{VertexFormat::kF32, 0, 1}},
      },
  }};
  cfg.entry_point_name = "main";

  DataMap data;
  data.Add<VertexPulling::Config>(cfg);
  auto got = Run<VertexPulling>(src, data);

  EXPECT_EQ(expect, str(got));
}

// TODO(crbug.com/tint/697): Remove this.
TEST_F(VertexPullingTest, TwoAttributesSameBuffer_Legacy) {
  auto* src = R"(
[[location(0)]] var<in> var_a : f32;
[[location(1)]] var<in> var_b : vec4<f32>;

[[stage(vertex)]]
fn main() -> [[builtin(position)]] vec4<f32> {
  return vec4<f32>();
}
)";

  auto* expect = R"(
[[builtin(vertex_index)]] var<in> tint_pulling_vertex_index : u32;

[[block]]
struct TintVertexData {
  tint_vertex_data : [[stride(4)]] array<u32>;
};

[[binding(0), group(4)]] var<storage> tint_pulling_vertex_buffer_0 : [[access(read)]] TintVertexData;

var<private> var_a : f32;

var<private> var_b : vec4<f32>;

[[stage(vertex)]]
fn main() -> [[builtin(position)]] vec4<f32> {
  {
    var tint_pulling_pos : u32;
    tint_pulling_pos = ((tint_pulling_vertex_index * 16u) + 0u);
    var_a = bitcast<f32>(tint_pulling_vertex_buffer_0.tint_vertex_data[(tint_pulling_pos / 4u)]);
    tint_pulling_pos = ((tint_pulling_vertex_index * 16u) + 0u);
    var_b = vec4<f32>(bitcast<f32>(tint_pulling_vertex_buffer_0.tint_vertex_data[((tint_pulling_pos + 0u) / 4u)]), bitcast<f32>(tint_pulling_vertex_buffer_0.tint_vertex_data[((tint_pulling_pos + 4u) / 4u)]), bitcast<f32>(tint_pulling_vertex_buffer_0.tint_vertex_data[((tint_pulling_pos + 8u) / 4u)]), bitcast<f32>(tint_pulling_vertex_buffer_0.tint_vertex_data[((tint_pulling_pos + 12u) / 4u)]));
  }
  return vec4<f32>();
}
)";

  VertexPulling::Config cfg;
  cfg.vertex_state = {
      {{16,
        InputStepMode::kVertex,
        {{VertexFormat::kF32, 0, 0}, {VertexFormat::kVec4F32, 0, 1}}}}};
  cfg.entry_point_name = "main";

  DataMap data;
  data.Add<VertexPulling::Config>(cfg);
  auto got = Run<VertexPulling>(src, data);

  EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace transform
}  // namespace tint
