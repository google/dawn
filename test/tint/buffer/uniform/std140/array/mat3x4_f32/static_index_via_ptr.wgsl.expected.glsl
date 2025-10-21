#version 310 es

layout(binding = 0, std140)
uniform a_block_1_ubo {
  uvec4 inner[12];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float inner;
} v_1;
mat3x4 v_2(uint start_byte_offset) {
  return mat3x4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)]));
}
mat3x4[4] v_3(uint start_byte_offset) {
  mat3x4 a[4] = mat3x4[4](mat3x4(vec4(0.0f), vec4(0.0f), vec4(0.0f)), mat3x4(vec4(0.0f), vec4(0.0f), vec4(0.0f)), mat3x4(vec4(0.0f), vec4(0.0f), vec4(0.0f)), mat3x4(vec4(0.0f), vec4(0.0f), vec4(0.0f)));
  {
    uint v_4 = 0u;
    v_4 = 0u;
    while(true) {
      uint v_5 = v_4;
      if ((v_5 >= 4u)) {
        break;
      }
      a[v_5] = v_2((start_byte_offset + (v_5 * 48u)));
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
  mat3x4 l_a[4] = v_3(0u);
  mat3x4 l_a_i = v_2(96u);
  vec4 l_a_i_i = uintBitsToFloat(v.inner[7u]);
  uvec4 v_6 = v.inner[7u];
  v_1.inner = (((uintBitsToFloat(v_6.x) + l_a[0u][0u].x) + l_a_i[0u].x) + l_a_i_i.x);
}
