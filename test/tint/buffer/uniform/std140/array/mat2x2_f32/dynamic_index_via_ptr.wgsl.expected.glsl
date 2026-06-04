#version 310 es

layout(binding = 0, std140)
uniform a_block_1_ubo {
  uvec4 inner[4];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float inner;
} v_1;
int counter = 0;
int i() {
  uint v_2 = uint(counter);
  counter = int((v_2 + uint(1)));
  return counter;
}
mat2 v_3(uint start_byte_offset) {
  uvec4 v_4 = v.inner[(start_byte_offset / 16u)];
  vec2 v_5 = uintBitsToFloat(mix(v_4.xy, v_4.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_6 = (8u + start_byte_offset);
  uvec4 v_7 = v.inner[(v_6 / 16u)];
  return mat2(v_5, uintBitsToFloat(mix(v_7.xy, v_7.zw, bvec2((((v_6 & 15u) >> 2u) == 2u)))));
}
mat2[4] v_8(uint start_byte_offset) {
  mat2 a[4] = mat2[4](mat2(vec2(0.0f), vec2(0.0f)), mat2(vec2(0.0f), vec2(0.0f)), mat2(vec2(0.0f), vec2(0.0f)), mat2(vec2(0.0f), vec2(0.0f)));
  {
    uint v_9 = 0u;
    v_9 = 0u;
    while(true) {
      uint v_10 = v_9;
      if ((v_10 >= 4u)) {
        break;
      }
      a[v_10] = v_3((start_byte_offset + (v_10 * 16u)));
      {
        v_9 = (v_10 + 1u);
      }
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_11 = (min(uint(i()), 3u) * 16u);
  uint v_12 = (min(uint(i()), 1u) * 8u);
  mat2 l_a[4] = v_8(0u);
  mat2 l_a_i = v_3(v_11);
  uint v_13 = (v_11 + v_12);
  uvec4 v_14 = v.inner[(v_13 / 16u)];
  vec2 l_a_i_i = uintBitsToFloat(mix(v_14.xy, v_14.zw, bvec2((((v_13 & 15u) >> 2u) == 2u))));
  uint v_15 = (v_11 + v_12);
  uvec4 v_16 = v.inner[(v_15 / 16u)];
  v_1.inner = (((uintBitsToFloat(v_16[((v_15 & 15u) >> 2u)]) + l_a[0u][0u].x) + l_a_i[0u].x) + l_a_i_i.x);
}
