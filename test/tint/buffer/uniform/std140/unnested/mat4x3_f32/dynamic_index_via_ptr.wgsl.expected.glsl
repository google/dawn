#version 310 es

layout(binding = 0, std140)
uniform m_block_1_ubo {
  uvec4 inner[4];
} v;
int counter = 0;
int i() {
  uint v_1 = uint(counter);
  counter = int((v_1 + uint(1)));
  return counter;
}
mat4x3 v_2(uint start_byte_offset) {
  return mat4x3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((48u + start_byte_offset) / 16u)].xyz));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_3 = (min(uint(i()), 3u) * 16u);
  mat4x3 l_m = v_2(0u);
  vec3 l_m_i = uintBitsToFloat(v.inner[(v_3 / 16u)].xyz);
}
