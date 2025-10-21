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
  vec2 v_4 = uintBitsToFloat(mix(v_3.xy, v_3.zw, bvec2((((start_byte_offset % 16u) / 4u) == 2u))));
  uvec4 v_5 = v.inner[((8u + start_byte_offset) / 16u)];
  vec2 v_6 = uintBitsToFloat(mix(v_5.xy, v_5.zw, bvec2(((((8u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_7 = v.inner[((16u + start_byte_offset) / 16u)];
  vec2 v_8 = uintBitsToFloat(mix(v_7.xy, v_7.zw, bvec2(((((16u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_9 = v.inner[((24u + start_byte_offset) / 16u)];
  return mat4x2(v_4, v_6, v_8, uintBitsToFloat(mix(v_9.xy, v_9.zw, bvec2(((((24u + start_byte_offset) % 16u) / 4u) == 2u)))));
}
mat4x2[4] v_10(uint start_byte_offset) {
  mat4x2 a[4] = mat4x2[4](mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f)), mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f)), mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f)), mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f)));
  {
    uint v_11 = 0u;
    v_11 = 0u;
    while(true) {
      uint v_12 = v_11;
      if ((v_12 >= 4u)) {
        break;
      }
      a[v_12] = v_2((start_byte_offset + (v_12 * 32u)));
      {
        v_11 = (v_12 + 1u);
      }
      continue;
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat4x2 l_a[4] = v_10(0u);
  mat4x2 l_a_i = v_2(64u);
  vec2 l_a_i_i = uintBitsToFloat(v.inner[4u].zw);
  uvec4 v_13 = v.inner[4u];
  v_1.inner = (((uintBitsToFloat(v_13.z) + l_a[0u][0u].x) + l_a_i[0u].x) + l_a_i_i.x);
}
