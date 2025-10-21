#version 310 es

layout(binding = 0, std140)
uniform m_block_1_ubo {
  uvec4 inner[4];
} v;
mat4 v_1(uint start_byte_offset) {
  return mat4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((48u + start_byte_offset) / 16u)]));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat4 l_m = v_1(0u);
  vec4 l_m_1 = uintBitsToFloat(v.inner[1u]);
}
