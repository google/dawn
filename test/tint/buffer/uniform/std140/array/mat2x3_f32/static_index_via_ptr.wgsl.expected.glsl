#version 310 es

layout(binding = 0, std140)
uniform a_block_1_ubo {
  uvec4 inner[8];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float inner;
} v_1;
mat2x3 v_2(uint start_byte_offset) {
  return mat2x3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz));
}
mat2x3[4] v_3(uint start_byte_offset) {
  mat2x3 a[4] = mat2x3[4](mat2x3(vec3(0.0f), vec3(0.0f)), mat2x3(vec3(0.0f), vec3(0.0f)), mat2x3(vec3(0.0f), vec3(0.0f)), mat2x3(vec3(0.0f), vec3(0.0f)));
  {
    uint v_4 = 0u;
    v_4 = 0u;
    while(true) {
      uint v_5 = v_4;
      if ((v_5 >= 4u)) {
        break;
      }
      a[v_5] = v_2((start_byte_offset + (v_5 * 32u)));
      {
        v_4 = (v_5 + 1u);
      }
      continue;
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat2x3 l_a[4] = v_3(0u);
  mat2x3 l_a_i = v_2(64u);
  vec3 l_a_i_i = uintBitsToFloat(v.inner[5u].xyz);
  uvec4 v_6 = v.inner[5u];
  v_1.inner = (((uintBitsToFloat(v_6.x) + l_a[0u][0u].x) + l_a_i[0u].x) + l_a_i_i.x);
}
