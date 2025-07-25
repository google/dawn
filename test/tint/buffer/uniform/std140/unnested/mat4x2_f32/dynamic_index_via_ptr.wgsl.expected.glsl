#version 310 es

layout(binding = 0, std140)
uniform m_block_std140_1_ubo {
  vec2 inner_col0;
  vec2 inner_col1;
  vec2 inner_col2;
  vec2 inner_col3;
} v;
int counter = 0;
int i() {
  uint v_1 = uint(counter);
  counter = int((v_1 + uint(1)));
  return counter;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat4x2 v_2 = mat4x2(v.inner_col0, v.inner_col1, v.inner_col2, v.inner_col3);
  mat4x2 l_m = v_2;
  vec2 l_m_i = v_2[min(uint(i()), 3u)];
}
