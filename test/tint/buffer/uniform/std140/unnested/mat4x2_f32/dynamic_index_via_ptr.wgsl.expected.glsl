#version 310 es

layout(binding = 0, std140)
uniform m_block_1_ubo {
  uvec4 inner[2];
} v;
int counter = 0;
int i() {
  uint v_1 = uint(counter);
  counter = int((v_1 + uint(1)));
  return counter;
}
mat4x2 v_2(uint start_byte_offset) {
  uvec4 v_3 = v.inner[(start_byte_offset / 16u)];
  vec2 v_4 = uintBitsToFloat(mix(v_3.xy, v_3.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uvec4 v_5 = v.inner[((8u + start_byte_offset) / 16u)];
  vec2 v_6 = uintBitsToFloat(mix(v_5.xy, v_5.zw, bvec2(((((8u + start_byte_offset) & 15u) >> 2u) == 2u))));
  uvec4 v_7 = v.inner[((16u + start_byte_offset) / 16u)];
  vec2 v_8 = uintBitsToFloat(mix(v_7.xy, v_7.zw, bvec2(((((16u + start_byte_offset) & 15u) >> 2u) == 2u))));
  uvec4 v_9 = v.inner[((24u + start_byte_offset) / 16u)];
  return mat4x2(v_4, v_6, v_8, uintBitsToFloat(mix(v_9.xy, v_9.zw, bvec2(((((24u + start_byte_offset) & 15u) >> 2u) == 2u)))));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_10 = (min(uint(i()), 3u) * 8u);
  mat4x2 l_m = v_2(0u);
  uvec4 v_11 = v.inner[(v_10 / 16u)];
  vec2 l_m_i = uintBitsToFloat(mix(v_11.xy, v_11.zw, bvec2((((v_10 & 15u) >> 2u) == 2u))));
}
