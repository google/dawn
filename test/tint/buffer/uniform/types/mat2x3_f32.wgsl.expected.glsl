#version 310 es

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[2];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  mat2x3 inner;
} v_1;
void tint_store_and_preserve_padding(mat2x3 value_param) {
  v_1.inner[0u] = value_param[0u];
  v_1.inner[1u] = value_param[1u];
}
mat2x3 v_2(uint start_byte_offset) {
  return mat2x3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat2x3 x = v_2(0u);
  tint_store_and_preserve_padding(x);
}
