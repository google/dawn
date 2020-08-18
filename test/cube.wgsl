# Copyright 2020 The Tint Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

entry_point vertex = vtx_main;
entry_point fragment = frag_main;

# Vertex shader
type Uniforms = [[block]] struct {
  [[offset 0]] modelViewProjectionMatrix : mat4x4<f32>;
};

[[binding 0, set 0]] var<uniform> uniforms : Uniforms;

[[location 0]] var<in> cur_position : vec4<f32>;
[[location 1]] var<in> color : vec4<f32>;
[[location 0]] var<out> vtxFragColor : vec4<f32>;
[[builtin position]] var<out> Position : vec4<f32>;

fn vtx_main() -> void {
   Position = uniforms.modelViewProjectionMatrix * cur_position;
   vtxFragColor = color;
   return;
}

# Fragment shader
[[location 0]] var<in> fragColor : vec4<f32>;
[[location 0]] var<out> outColor : vec4<f32>;

fn frag_main() -> void {
  outColor = fragColor;
  return;
}
