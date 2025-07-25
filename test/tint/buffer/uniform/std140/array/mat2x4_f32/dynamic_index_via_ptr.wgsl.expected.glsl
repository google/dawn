#version 310 es

layout(binding = 0, std140)
uniform a_block_1_ubo {
  mat2x4 inner[4];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float inner;
} v_1;
int counter = 0;
int i() {
  uint v_2 = uint(counter);
  counter = int((v_2 + uint(1)));
  return counter;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_3 = min(uint(i()), 3u);
  uint v_4 = min(uint(i()), 1u);
  mat2x4 l_a[4] = v.inner;
  mat2x4 l_a_i = v.inner[v_3];
  vec4 l_a_i_i = v.inner[v_3][v_4];
  v_1.inner = (((v.inner[v_3][v_4].x + l_a[0u][0u].x) + l_a_i[0u].x) + l_a_i_i.x);
}
