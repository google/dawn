#version 310 es

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[4];
} v;
mat4x3 p = mat4x3(vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f));
mat4x3 v_1(uint start_byte_offset) {
  return mat4x3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((48u + start_byte_offset) / 16u)].xyz));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  p = v_1(0u);
  p[1u] = uintBitsToFloat(v.inner[0u].xyz);
  p[1u] = uintBitsToFloat(v.inner[0u].xyz).zxy;
  uvec4 v_2 = v.inner[1u];
  p[0u].y = uintBitsToFloat(v_2.x);
}
