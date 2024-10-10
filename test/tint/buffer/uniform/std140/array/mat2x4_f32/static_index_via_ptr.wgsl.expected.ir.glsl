#version 310 es

layout(binding = 0, std140)
uniform a_block_1_ubo {
  mat2x4 inner[4];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float inner;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat2x4 l_a[4] = v.inner;
  mat2x4 l_a_i = v.inner[2];
  vec4 l_a_i_i = v.inner[2][1];
  v_1.inner = (((v.inner[2][1].x + l_a[0][0][0u]) + l_a_i[0][0u]) + l_a_i_i[0u]);
}
