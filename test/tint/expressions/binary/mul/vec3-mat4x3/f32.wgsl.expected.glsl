#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std140)
uniform f_data_block_ubo {
  uvec4 inner[5];
} v;
mat4x3 v_1(uint start_byte_offset) {
  return mat4x3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((48u + start_byte_offset) / 16u)].xyz));
}
void main() {
  vec3 v_2 = uintBitsToFloat(v.inner[4u].xyz);
  vec4 x = (v_2 * v_1(0u));
}
