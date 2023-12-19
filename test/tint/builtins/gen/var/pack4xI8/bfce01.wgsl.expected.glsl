#version 310 es

uint tint_pack_4xi8(ivec4 a) {
  uvec4 a_i8 = uvec4(((a & ivec4(255)) << uvec4(0u, 8u, 16u, 24u)));
  return (a_i8[0] | (a_i8[1] | (a_i8[2] | a_i8[3])));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uint inner;
} prevent_dce;

void pack4xI8_bfce01() {
  ivec4 arg_0 = ivec4(1);
  uint res = tint_pack_4xi8(arg_0);
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  pack4xI8_bfce01();
  return vec4(0.0f);
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
precision highp float;

uint tint_pack_4xi8(ivec4 a) {
  uvec4 a_i8 = uvec4(((a & ivec4(255)) << uvec4(0u, 8u, 16u, 24u)));
  return (a_i8[0] | (a_i8[1] | (a_i8[2] | a_i8[3])));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uint inner;
} prevent_dce;

void pack4xI8_bfce01() {
  ivec4 arg_0 = ivec4(1);
  uint res = tint_pack_4xi8(arg_0);
  prevent_dce.inner = res;
}

void fragment_main() {
  pack4xI8_bfce01();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

uint tint_pack_4xi8(ivec4 a) {
  uvec4 a_i8 = uvec4(((a & ivec4(255)) << uvec4(0u, 8u, 16u, 24u)));
  return (a_i8[0] | (a_i8[1] | (a_i8[2] | a_i8[3])));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uint inner;
} prevent_dce;

void pack4xI8_bfce01() {
  ivec4 arg_0 = ivec4(1);
  uint res = tint_pack_4xi8(arg_0);
  prevent_dce.inner = res;
}

void compute_main() {
  pack4xI8_bfce01();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
