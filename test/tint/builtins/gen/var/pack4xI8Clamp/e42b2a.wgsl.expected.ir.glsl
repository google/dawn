#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uint tint_symbol;
} v;
uint tint_int_dot(uvec4 x, uvec4 y) {
  return ((((x.x * y.x) + (x.y * y.y)) + (x.z * y.z)) + (x.w * y.w));
}
uint pack4xI8Clamp_e42b2a() {
  ivec4 arg_0 = ivec4(1);
  ivec4 v_1 = arg_0;
  uvec4 v_2 = uvec4(0u, 8u, 16u, 24u);
  ivec4 v_3 = ivec4(-128);
  uvec4 v_4 = uvec4(clamp(v_1, v_3, ivec4(127)));
  uvec4 v_5 = ((v_4 & uvec4(255u)) << v_2);
  uint res = tint_int_dot(v_5, uvec4(1u));
  return res;
}
void main() {
  v.tint_symbol = pack4xI8Clamp_e42b2a();
}
#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uint tint_symbol;
} v;
uint tint_int_dot(uvec4 x, uvec4 y) {
  return ((((x.x * y.x) + (x.y * y.y)) + (x.z * y.z)) + (x.w * y.w));
}
uint pack4xI8Clamp_e42b2a() {
  ivec4 arg_0 = ivec4(1);
  ivec4 v_1 = arg_0;
  uvec4 v_2 = uvec4(0u, 8u, 16u, 24u);
  ivec4 v_3 = ivec4(-128);
  uvec4 v_4 = uvec4(clamp(v_1, v_3, ivec4(127)));
  uvec4 v_5 = ((v_4 & uvec4(255u)) << v_2);
  uint res = tint_int_dot(v_5, uvec4(1u));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = pack4xI8Clamp_e42b2a();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

layout(location = 0) flat out uint vertex_main_loc0_Output;
uint tint_int_dot(uvec4 x, uvec4 y) {
  return ((((x.x * y.x) + (x.y * y.y)) + (x.z * y.z)) + (x.w * y.w));
}
uint pack4xI8Clamp_e42b2a() {
  ivec4 arg_0 = ivec4(1);
  ivec4 v = arg_0;
  uvec4 v_1 = uvec4(0u, 8u, 16u, 24u);
  ivec4 v_2 = ivec4(-128);
  uvec4 v_3 = uvec4(clamp(v, v_2, ivec4(127)));
  uvec4 v_4 = ((v_3 & uvec4(255u)) << v_1);
  uint res = tint_int_dot(v_4, uvec4(1u));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0u);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = pack4xI8Clamp_e42b2a();
  return tint_symbol;
}
void main() {
  VertexOutput v_5 = vertex_main_inner();
  gl_Position = v_5.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_5.prevent_dce;
  gl_PointSize = 1.0f;
}
