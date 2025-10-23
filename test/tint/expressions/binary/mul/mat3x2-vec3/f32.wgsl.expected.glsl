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
  uvec4 v_4 = v.inner[((8u + start_byte_offset) / 16u)];
  vec2 v_5 = uintBitsToFloat(mix(v_4.xy, v_4.zw, bvec2(((((8u + start_byte_offset) & 15u) >> 2u) == 2u))));
  uvec4 v_6 = v.inner[((16u + start_byte_offset) / 16u)];
  return mat3x2(v_3, v_5, uintBitsToFloat(mix(v_6.xy, v_6.zw, bvec2(((((16u + start_byte_offset) & 15u) >> 2u) == 2u)))));
}
void main() {
  mat3x2 v_7 = v_1(0u);
  vec2 x = (v_7 * uintBitsToFloat(v.inner[2u].xyz));
}
