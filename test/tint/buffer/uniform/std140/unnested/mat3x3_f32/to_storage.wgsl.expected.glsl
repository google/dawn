#version 310 es

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[3];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  mat3 inner;
} v_1;
void tint_store_and_preserve_padding(mat3 value_param) {
  v_1.inner[0u] = value_param[0u];
  v_1.inner[1u] = value_param[1u];
  v_1.inner[2u] = value_param[2u];
}
mat3 v_2(uint start_byte_offset) {
  return mat3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)].xyz));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_store_and_preserve_padding(v_2(0u));
  v_1.inner[1u] = uintBitsToFloat(v.inner[0u].xyz);
  v_1.inner[1u] = uintBitsToFloat(v.inner[0u].xyz).zxy;
  uvec4 v_3 = v.inner[1u];
  v_1.inner[0u].y = uintBitsToFloat(v_3.x);
}
