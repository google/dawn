#version 310 es

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[4];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  mat2 inner[4];
} v_1;
mat2 v_2(uint start_byte_offset) {
  uvec4 v_3 = v.inner[(start_byte_offset / 16u)];
  vec2 v_4 = uintBitsToFloat(mix(v_3.xy, v_3.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uvec4 v_5 = v.inner[((8u + start_byte_offset) / 16u)];
  return mat2(v_4, uintBitsToFloat(mix(v_5.xy, v_5.zw, bvec2(((((8u + start_byte_offset) & 15u) >> 2u) == 2u)))));
}
mat2[4] v_6(uint start_byte_offset) {
  mat2 a[4] = mat2[4](mat2(vec2(0.0f), vec2(0.0f)), mat2(vec2(0.0f), vec2(0.0f)), mat2(vec2(0.0f), vec2(0.0f)), mat2(vec2(0.0f), vec2(0.0f)));
  {
    uint v_7 = 0u;
    v_7 = 0u;
    while(true) {
      uint v_8 = v_7;
      if ((v_8 >= 4u)) {
        break;
      }
      a[v_8] = v_2((start_byte_offset + (v_8 * 16u)));
      {
        v_7 = (v_8 + 1u);
      }
      continue;
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v_1.inner = v_6(0u);
  v_1.inner[1u] = v_2(32u);
  v_1.inner[1u][0u] = uintBitsToFloat(v.inner[0u].zw).yx;
  uvec4 v_9 = v.inner[0u];
  v_1.inner[1u][0u].x = uintBitsToFloat(v_9.z);
}
