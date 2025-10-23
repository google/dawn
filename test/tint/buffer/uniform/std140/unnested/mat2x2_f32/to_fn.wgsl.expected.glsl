#version 310 es

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[1];
} v_1;
void a(mat2 m) {
}
void b(vec2 v) {
}
void c(float f_1) {
}
mat2 v_2(uint start_byte_offset) {
  uvec4 v_3 = v_1.inner[(start_byte_offset / 16u)];
  vec2 v_4 = uintBitsToFloat(mix(v_3.xy, v_3.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uvec4 v_5 = v_1.inner[((8u + start_byte_offset) / 16u)];
  return mat2(v_4, uintBitsToFloat(mix(v_5.xy, v_5.zw, bvec2(((((8u + start_byte_offset) & 15u) >> 2u) == 2u)))));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  a(v_2(0u));
  b(uintBitsToFloat(v_1.inner[0u].zw));
  b(uintBitsToFloat(v_1.inner[0u].zw).yx);
  uvec4 v_6 = v_1.inner[0u];
  c(uintBitsToFloat(v_6.z));
  c(uintBitsToFloat(v_1.inner[0u].zw).yx.x);
}
