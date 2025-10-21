#version 310 es

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[12];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float inner;
} v_1;
mat3 v_2(uint start_byte_offset) {
  return mat3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)].xyz));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat3 t = transpose(v_2(96u));
  float l = length(uintBitsToFloat(v.inner[1u].xyz).zxy);
  float a = abs(uintBitsToFloat(v.inner[1u].xyz).zxy.x);
  float v_3 = (t[0u].x + float(l));
  v_1.inner = (v_3 + float(a));
}
