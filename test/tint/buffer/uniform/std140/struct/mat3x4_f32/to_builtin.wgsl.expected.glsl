#version 310 es

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[32];
} v;
mat3x4 v_1(uint start_byte_offset) {
  return mat3x4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)]));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat4x3 t = transpose(v_1(272u));
  float l = length(uintBitsToFloat(v.inner[2u]).ywxz);
  float a = abs(uintBitsToFloat(v.inner[2u]).ywxz.x);
}
