#version 310 es

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[4];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float inner;
} v_1;
mat2 p[4] = mat2[4](mat2(vec2(0.0f), vec2(0.0f)), mat2(vec2(0.0f), vec2(0.0f)), mat2(vec2(0.0f), vec2(0.0f)), mat2(vec2(0.0f), vec2(0.0f)));
mat2 v_2(uint start_byte_offset) {
  uvec4 v_3 = v.inner[(start_byte_offset / 16u)];
  vec2 v_4 = uintBitsToFloat(mix(v_3.xy, v_3.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_5 = (8u + start_byte_offset);
  uvec4 v_6 = v.inner[(v_5 / 16u)];
  return mat2(v_4, uintBitsToFloat(mix(v_6.xy, v_6.zw, bvec2((((v_5 & 15u) >> 2u) == 2u)))));
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
      a[v_9] = v_2((start_byte_offset + (v_9 * 16u)));
      {
        v_8 = (v_9 + 1u);
      }
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  p = v_7(0u);
  p[1u] = v_2(32u);
  p[1u][0u] = uintBitsToFloat(v.inner[0u].zw).yx;
  uvec4 v_10 = v.inner[0u];
  p[1u][0u].x = uintBitsToFloat(v_10.z);
  v_1.inner = p[1u][0u].x;
}
