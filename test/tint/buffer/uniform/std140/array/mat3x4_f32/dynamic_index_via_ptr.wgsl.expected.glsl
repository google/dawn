#version 310 es

layout(binding = 0, std140)
uniform a_block_1_ubo {
  uvec4 inner[12];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float inner;
} v_1;
int counter = 0;
int i() {
  counter = int((uint(counter) + 1u));
  return counter;
}
mat3x4 v_2(uint start_byte_offset) {
  return mat3x4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)]));
}
mat3x4[4] v_3(uint start_byte_offset) {
  mat3x4 a[4] = mat3x4[4](mat3x4(vec4(0.0f), vec4(0.0f), vec4(0.0f)), mat3x4(vec4(0.0f), vec4(0.0f), vec4(0.0f)), mat3x4(vec4(0.0f), vec4(0.0f), vec4(0.0f)), mat3x4(vec4(0.0f), vec4(0.0f), vec4(0.0f)));
  {
    uint idx = 0u;
    while(true) {
      uint v_4 = idx;
      if ((v_4 >= 4u)) {
        break;
      }
      a[v_4] = v_2((start_byte_offset + (v_4 * 48u)));
      {
        idx = (idx + 1u);
      }
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_5 = (min(uint(i()), 3u) * 48u);
  uint v_6 = (min(uint(i()), 2u) * 16u);
  mat3x4 l_a[4] = v_3(0u);
  mat3x4 l_a_i = v_2(v_5);
  vec4 l_a_i_i = uintBitsToFloat(v.inner[((v_5 + v_6) / 16u)]);
  uint v_7 = (v_5 + v_6);
  uvec4 v_8 = v.inner[(v_7 / 16u)];
  v_1.inner = (((uintBitsToFloat(v_8[((v_7 & 15u) >> 2u)]) + l_a[0][0].x) + l_a_i[0].x) + l_a_i_i.x);
}
