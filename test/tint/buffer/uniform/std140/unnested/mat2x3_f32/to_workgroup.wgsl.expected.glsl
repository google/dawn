#version 310 es

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[2];
} v;
shared mat2x3 w;
mat2x3 v_1(uint start_byte_offset) {
  return mat2x3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz));
}
void f_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    w = mat2x3(vec3(0.0f), vec3(0.0f));
  }
  barrier();
  w = v_1(0u);
  w[1u] = uintBitsToFloat(v.inner[0u].xyz);
  w[1u] = uintBitsToFloat(v.inner[0u].xyz).zxy;
  uvec4 v_2 = v.inner[1u];
  w[0u].y = uintBitsToFloat(v_2.x);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_inner(gl_LocalInvocationIndex);
}
