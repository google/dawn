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
  uvec4 v_6 = v.inner[((8u + start_byte_offset) / 16u)];
  return mat2(v_5, uintBitsToFloat(mix(v_6.xy, v_6.zw, bvec2(((((8u + start_byte_offset) & 15u) >> 2u) == 2u)))));
}
mat2[4] v_7(uint start_byte_offset) {
  mat2 a[4] = mat2[4](mat2(vec2(0.0f), vec2(0.0f)), mat2(vec2(0.0f), vec2(0.0f)), mat2(vec2(0.0f), vec2(0.0f)), mat2(vec2(0.0f), vec2(0.0f)));
  {
    uint v_8 = 0u;
    v_8 = 0u;
    while(true) {
      uint v_9 = v_8;
      if ((v_9 >= 4u)) {
        break;
      }
      a[v_9] = v_3((start_byte_offset + (v_9 * 16u)));
      {
        v_8 = (v_9 + 1u);
      }
      continue;
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_10 = (16u * min(uint(i()), 3u));
  uint v_11 = (8u * min(uint(i()), 1u));
  mat2 l_a[4] = v_7(0u);
  mat2 l_a_i = v_3(v_10);
  uvec4 v_12 = v.inner[((v_10 + v_11) / 16u)];
  vec2 l_a_i_i = uintBitsToFloat(mix(v_12.xy, v_12.zw, bvec2(((((v_10 + v_11) & 15u) >> 2u) == 2u))));
  uvec4 v_13 = v.inner[((v_10 + v_11) / 16u)];
  v_1.inner = (((uintBitsToFloat(v_13[(((v_10 + v_11) & 15u) >> 2u)]) + l_a[0u][0u].x) + l_a_i[0u].x) + l_a_i_i.x);
}
