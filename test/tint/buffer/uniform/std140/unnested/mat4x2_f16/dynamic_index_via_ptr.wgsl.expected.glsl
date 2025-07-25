#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform m_block_std140_1_ubo {
  f16vec2 inner_col0;
  f16vec2 inner_col1;
  f16vec2 inner_col2;
  f16vec2 inner_col3;
} v;
int counter = 0;
int i() {
  uint v_1 = uint(counter);
  counter = int((v_1 + uint(1)));
  return counter;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f16mat4x2 v_2 = f16mat4x2(v.inner_col0, v.inner_col1, v.inner_col2, v.inner_col3);
  f16mat4x2 l_m = v_2;
  f16vec2 l_m_i = v_2[min(uint(i()), 3u)];
}
