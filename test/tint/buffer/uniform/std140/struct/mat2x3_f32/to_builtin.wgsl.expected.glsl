#version 310 es

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[32];
} v;
mat2x3 v_1(uint start_byte_offset) {
  return mat2x3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat3x2 t = transpose(v_1(272u));
  float l = length(uintBitsToFloat(v.inner[2u].xyz).zxy);
  float a = abs(uintBitsToFloat(v.inner[2u].xyz).zxy.x);
}
