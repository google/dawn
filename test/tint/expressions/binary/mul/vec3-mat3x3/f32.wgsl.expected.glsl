#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std140)
uniform f_data_block_ubo {
  uvec4 inner[4];
} v;
mat3 v_1(uint start_byte_offset) {
  return mat3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)].xyz));
}
void main() {
  vec3 v_2 = uintBitsToFloat(v.inner[3u].xyz);
  vec3 x = (v_2 * v_1(0u));
}
