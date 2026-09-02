//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  uint inner;
} v;
uint tint_int_dot(uvec4 x, uvec4 y) {
  return ((((x.x * y.x) + (x.y * y.y)) + (x.z * y.z)) + (x.w * y.w));
}
uint pack4xI8Clamp_e42b2a() {
  ivec4 arg_0 = ivec4(1);
  uint res = tint_int_dot(((uvec4(clamp(arg_0, ivec4(-128), ivec4(127))) & uvec4(255u)) << uvec4(0u, 8u, 16u, 24u)), uvec4(1u));
  return res;
}
void main() {
  v.inner = pack4xI8Clamp_e42b2a();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uint inner;
} v;
uint tint_int_dot(uvec4 x, uvec4 y) {
  return ((((x.x * y.x) + (x.y * y.y)) + (x.z * y.z)) + (x.w * y.w));
}
uint pack4xI8Clamp_e42b2a() {
  ivec4 arg_0 = ivec4(1);
  uint res = tint_int_dot(((uvec4(clamp(arg_0, ivec4(-128), ivec4(127))) & uvec4(255u)) << uvec4(0u, 8u, 16u, 24u)), uvec4(1u));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = pack4xI8Clamp_e42b2a();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

layout(location = 0) flat out uint tint_interstage_location0;
uint tint_int_dot(uvec4 x, uvec4 y) {
  return ((((x.x * y.x) + (x.y * y.y)) + (x.z * y.z)) + (x.w * y.w));
}
uint pack4xI8Clamp_e42b2a() {
  ivec4 arg_0 = ivec4(1);
  uint res = tint_int_dot(((uvec4(clamp(arg_0, ivec4(-128), ivec4(127))) & uvec4(255u)) << uvec4(0u, 8u, 16u, 24u)), uvec4(1u));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v = VertexOutput(vec4(0.0f), 0u);
  v.pos = vec4(0.0f);
  v.prevent_dce = pack4xI8Clamp_e42b2a();
  return v;
}
void main() {
  VertexOutput v_1 = vertex_main_inner();
  gl_Position = vec4(v_1.pos.x, -(v_1.pos.y), ((2.0f * v_1.pos.z) - v_1.pos.w), v_1.pos.w);
  tint_interstage_location0 = v_1.prevent_dce;
  gl_PointSize = 1.0f;
}
