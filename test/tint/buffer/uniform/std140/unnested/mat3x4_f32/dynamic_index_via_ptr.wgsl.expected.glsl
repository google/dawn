#version 310 es

layout(binding = 0, std140)
uniform m_block_1_ubo {
  uvec4 inner[3];
} v;
int counter = 0;
int i() {
  uint v_1 = uint(counter);
  counter = int((v_1 + uint(1)));
  return counter;
}
mat3x4 v_2(uint start_byte_offset) {
  return mat3x4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)]));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_3 = (16u * min(uint(i()), 2u));
  mat3x4 l_m = v_2(0u);
  vec4 l_m_i = uintBitsToFloat(v.inner[(v_3 / 16u)]);
}
