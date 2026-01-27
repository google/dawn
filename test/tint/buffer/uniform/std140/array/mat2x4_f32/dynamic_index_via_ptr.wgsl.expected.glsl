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
mat2x4 v_3(uint start_byte_offset) {
  return mat2x4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]));
}
mat2x4[4] v_4(uint start_byte_offset) {
  mat2x4 a[4] = mat2x4[4](mat2x4(vec4(0.0f), vec4(0.0f)), mat2x4(vec4(0.0f), vec4(0.0f)), mat2x4(vec4(0.0f), vec4(0.0f)), mat2x4(vec4(0.0f), vec4(0.0f)));
  {
    uint v_5 = 0u;
    v_5 = 0u;
    while(true) {
      uint v_6 = v_5;
      if ((v_6 >= 4u)) {
        break;
      }
      a[v_6] = v_3((start_byte_offset + (v_6 * 32u)));
      {
        v_5 = (v_6 + 1u);
      }
      continue;
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_7 = (min(uint(i()), 3u) * 32u);
  uint v_8 = (min(uint(i()), 1u) * 16u);
  mat2x4 l_a[4] = v_4(0u);
  mat2x4 l_a_i = v_3(v_7);
  vec4 l_a_i_i = uintBitsToFloat(v.inner[((v_7 + v_8) / 16u)]);
  uvec4 v_9 = v.inner[((v_7 + v_8) / 16u)];
  v_1.inner = (((uintBitsToFloat(v_9[(((v_7 + v_8) & 15u) >> 2u)]) + l_a[0u][0u].x) + l_a_i[0u].x) + l_a_i_i.x);
}
