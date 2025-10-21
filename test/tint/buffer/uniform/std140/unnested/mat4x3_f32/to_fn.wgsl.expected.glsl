#version 310 es

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[4];
} v_1;
void a(mat4x3 m) {
}
void b(vec3 v) {
}
void c(float f_1) {
}
mat4x3 v_2(uint start_byte_offset) {
  return mat4x3(uintBitsToFloat(v_1.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v_1.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v_1.inner[((32u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v_1.inner[((48u + start_byte_offset) / 16u)].xyz));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  a(v_2(0u));
  b(uintBitsToFloat(v_1.inner[1u].xyz));
  b(uintBitsToFloat(v_1.inner[1u].xyz).zxy);
  uvec4 v_3 = v_1.inner[1u];
  c(uintBitsToFloat(v_3.x));
  c(uintBitsToFloat(v_1.inner[1u].xyz).zxy.x);
}
