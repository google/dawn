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

[[location 0]] var<out> gl_FragColor : vec4<f32>;

fn bar() -> void {
  return;
}

fn main() -> void {
    var a : vec2<f32> = vec2<f32>();
    gl_FragColor = vec4<f32>(0.4, 0.4, 0.8, 1.0);
    bar();
    return;
}
entry_point fragment = main;
