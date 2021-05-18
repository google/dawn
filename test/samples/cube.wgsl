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

// Vertex shader
[[block]] struct Uniforms {
  modelViewProjectionMatrix : mat4x4<f32>;
};

[[binding(0), group(0)]] var<uniform> uniforms : Uniforms;

struct VertexInput {
  [[location(0)]] cur_position : vec4<f32>;
  [[location(1)]] color : vec4<f32>;
};

struct VertexOutput {
  [[location(0)]] vtxFragColor : vec4<f32>;
  [[builtin(position)]] Position : vec4<f32>;
};

[[stage(vertex)]]
fn vtx_main(input : VertexInput) -> VertexOutput {
  var output : VertexOutput;
  output.Position = uniforms.modelViewProjectionMatrix * input.cur_position;
  output.vtxFragColor = input.color;
  return output;
}

// Fragment shader

[[stage(fragment)]]
fn frag_main([[location(0)]] fragColor : vec4<f32>)
          -> [[location(0)]] vec4<f32> {
  return fragColor;
}
