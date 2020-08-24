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

# Vertex shader
const pos : array<vec2<f32>, 3> = array<vec2<f32>, 3>(
    vec2<f32>(0.0, 0.5),
    vec2<f32>(-0.5, -0.5),
    vec2<f32>(0.5, -0.5));

[[builtin position]] var<out> Position : vec4<f32>;
[[builtin vertex_idx]] var<in> VertexIndex : i32;

fn vtx_main() -> void {
  Position = vec4<f32>(pos[VertexIndex], 0.0, 1.0);
  return;
}
entry_point vertex as "main" = vtx_main;

# Fragment shader
[[location 0]] var<out> outColor : vec4<f32>;
fn frag_main() -> void {
  outColor = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  return;
}
entry_point fragment = frag_main;
