#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std140)
uniform f_data_block_ubo {
  uvec4 inner[3];
} v;
mat3x2 v_1(uint start_byte_offset) {
  uvec4 v_2 = v.inner[(start_byte_offset / 16u)];
  vec2 v_3 = uintBitsToFloat(mix(v_2.xy, v_2.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_4 = (8u + start_byte_offset);
  uvec4 v_5 = v.inner[(v_4 / 16u)];
  vec2 v_6 = uintBitsToFloat(mix(v_5.xy, v_5.zw, bvec2((((v_4 & 15u) >> 2u) == 2u))));
  uint v_7 = (16u + start_byte_offset);
  uvec4 v_8 = v.inner[(v_7 / 16u)];
  return mat3x2(v_3, v_6, uintBitsToFloat(mix(v_8.xy, v_8.zw, bvec2((((v_7 & 15u) >> 2u) == 2u)))));
}
void main() {
  mat3x2 v_9 = v_1(0u);
  vec2 x = (v_9 * uintBitsToFloat(v.inner[2u].xyz));
}
