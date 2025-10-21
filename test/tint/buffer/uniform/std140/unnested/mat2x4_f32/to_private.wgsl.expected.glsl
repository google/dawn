#version 310 es

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[2];
} v;
mat2x4 p = mat2x4(vec4(0.0f), vec4(0.0f));
mat2x4 v_1(uint start_byte_offset) {
  return mat2x4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  p = v_1(0u);
  p[1u] = uintBitsToFloat(v.inner[0u]);
  p[1u] = uintBitsToFloat(v.inner[0u]).ywxz;
  uvec4 v_2 = v.inner[1u];
  p[0u].y = uintBitsToFloat(v_2.x);
}
