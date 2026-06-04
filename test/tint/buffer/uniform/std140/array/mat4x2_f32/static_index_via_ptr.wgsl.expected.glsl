#version 310 es

layout(binding = 0, std140)
uniform a_block_1_ubo {
  uvec4 inner[8];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float inner;
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
mat4x2[4] v_13(uint start_byte_offset) {
  mat4x2 a[4] = mat4x2[4](mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f)), mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f)), mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f)), mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f)));
  {
    uint v_14 = 0u;
    v_14 = 0u;
    while(true) {
      uint v_15 = v_14;
      if ((v_15 >= 4u)) {
        break;
      }
      a[v_15] = v_2((start_byte_offset + (v_15 * 32u)));
      {
        v_14 = (v_15 + 1u);
      }
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat4x2 l_a[4] = v_13(0u);
  mat4x2 l_a_i = v_2(64u);
  vec2 l_a_i_i = uintBitsToFloat(v.inner[4u].zw);
  uvec4 v_16 = v.inner[4u];
  v_1.inner = (((uintBitsToFloat(v_16.z) + l_a[0u][0u].x) + l_a_i[0u].x) + l_a_i_i.x);
}
