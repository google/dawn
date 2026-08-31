#version 310 es

layout(binding = 0, std140)
uniform m_block_1_ubo {
  uvec4 inner[3];
} v;
int counter = 0;
int i() {
  counter = int((uint(counter) + 1u));
  return counter;
}
mat3 v_1(uint start_byte_offset) {
  return mat3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)].xyz));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_2 = (min(uint(i()), 2u) * 16u);
  mat3 l_m = v_1(0u);
  vec3 l_m_i = uintBitsToFloat(v.inner[(v_2 / 16u)].xyz);
}
