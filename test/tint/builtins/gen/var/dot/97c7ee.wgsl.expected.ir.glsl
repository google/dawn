#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uint tint_symbol;
} v;
uint tint_int_dot(uvec2 x, uvec2 y) {
  return ((x.x * y.x) + (x.y * y.y));
}
uint dot_97c7ee() {
  uvec2 arg_0 = uvec2(1u);
  uvec2 arg_1 = uvec2(1u);
  uint res = tint_int_dot(arg_0, arg_1);
  return res;
}
void main() {
  v.tint_symbol = dot_97c7ee();
}
#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uint tint_symbol;
} v;
uint tint_int_dot(uvec2 x, uvec2 y) {
  return ((x.x * y.x) + (x.y * y.y));
}
uint dot_97c7ee() {
  uvec2 arg_0 = uvec2(1u);
  uvec2 arg_1 = uvec2(1u);
  uint res = tint_int_dot(arg_0, arg_1);
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = dot_97c7ee();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

layout(location = 0) flat out uint vertex_main_loc0_Output;
uint tint_int_dot(uvec2 x, uvec2 y) {
  return ((x.x * y.x) + (x.y * y.y));
}
uint dot_97c7ee() {
  uvec2 arg_0 = uvec2(1u);
  uvec2 arg_1 = uvec2(1u);
  uint res = tint_int_dot(arg_0, arg_1);
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0u);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = dot_97c7ee();
  return tint_symbol;
}
void main() {
  VertexOutput v = vertex_main_inner();
  gl_Position = v.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v.prevent_dce;
  gl_PointSize = 1.0f;
}
