#version 310 es

layout(binding = 0, std140)
uniform a_block_1_ubo {
  uvec4 inner[8];
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
mat4x2 v_3(uint start_byte_offset) {
  uvec4 v_4 = v.inner[(start_byte_offset / 16u)];
  vec2 v_5 = uintBitsToFloat(mix(v_4.xy, v_4.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_6 = (8u + start_byte_offset);
  uvec4 v_7 = v.inner[(v_6 / 16u)];
  vec2 v_8 = uintBitsToFloat(mix(v_7.xy, v_7.zw, bvec2((((v_6 & 15u) >> 2u) == 2u))));
  uint v_9 = (16u + start_byte_offset);
  uvec4 v_10 = v.inner[(v_9 / 16u)];
  vec2 v_11 = uintBitsToFloat(mix(v_10.xy, v_10.zw, bvec2((((v_9 & 15u) >> 2u) == 2u))));
  uint v_12 = (24u + start_byte_offset);
  uvec4 v_13 = v.inner[(v_12 / 16u)];
  return mat4x2(v_5, v_8, v_11, uintBitsToFloat(mix(v_13.xy, v_13.zw, bvec2((((v_12 & 15u) >> 2u) == 2u)))));
}
mat4x2[4] v_14(uint start_byte_offset) {
  mat4x2 a[4] = mat4x2[4](mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f)), mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f)), mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f)), mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f)));
  {
    uint v_15 = 0u;
    v_15 = 0u;
    while(true) {
      uint v_16 = v_15;
      if ((v_16 >= 4u)) {
        break;
      }
      a[v_16] = v_3((start_byte_offset + (v_16 * 32u)));
      {
        v_15 = (v_16 + 1u);
      }
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_17 = (min(uint(i()), 3u) * 32u);
  uint v_18 = (min(uint(i()), 3u) * 8u);
  mat4x2 l_a[4] = v_14(0u);
  mat4x2 l_a_i = v_3(v_17);
  uint v_19 = (v_17 + v_18);
  uvec4 v_20 = v.inner[(v_19 / 16u)];
  vec2 l_a_i_i = uintBitsToFloat(mix(v_20.xy, v_20.zw, bvec2((((v_19 & 15u) >> 2u) == 2u))));
  uint v_21 = (v_17 + v_18);
  uvec4 v_22 = v.inner[(v_21 / 16u)];
  v_1.inner = (((uintBitsToFloat(v_22[((v_21 & 15u) >> 2u)]) + l_a[0u][0u].x) + l_a_i[0u].x) + l_a_i_i.x);
}
