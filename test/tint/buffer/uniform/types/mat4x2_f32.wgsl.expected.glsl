#version 310 es

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[2];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  mat4x2 inner;
} v_1;
mat4x2 v_2(uint start_byte_offset) {
  uvec4 v_3 = v.inner[(start_byte_offset / 16u)];
  vec2 v_4 = uintBitsToFloat(mix(v_3.xy, v_3.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_5 = (8u + start_byte_offset);
  uvec4 v_6 = v.inner[(v_5 / 16u)];
  vec2 v_7 = uintBitsToFloat(mix(v_6.xy, v_6.zw, bvec2((((v_5 & 15u) >> 2u) == 2u))));
  uint v_8 = (16u + start_byte_offset);
  uvec4 v_9 = v.inner[(v_8 / 16u)];
  vec2 v_10 = uintBitsToFloat(mix(v_9.xy, v_9.zw, bvec2((((v_8 & 15u) >> 2u) == 2u))));
  uint v_11 = (24u + start_byte_offset);
  uvec4 v_12 = v.inner[(v_11 / 16u)];
  return mat4x2(v_4, v_7, v_10, uintBitsToFloat(mix(v_12.xy, v_12.zw, bvec2((((v_11 & 15u) >> 2u) == 2u)))));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat4x2 x = v_2(0u);
  v_1.inner = x;
}
