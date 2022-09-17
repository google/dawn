#version 310 es

layout(binding = 0, std140) uniform m_block_std140_ubo {
  vec2 inner_0;
  vec2 inner_1;
  vec2 inner_2;
  vec2 inner_3;
} m;

mat4x2 load_m_inner() {
  return mat4x2(m.inner_0, m.inner_1, m.inner_2, m.inner_3);
}

void f() {
  mat4x2 p_m = load_m_inner();
  vec2 p_m_1 = m.inner_1;
  mat4x2 l_m = load_m_inner();
  vec2 l_m_1 = m.inner_1;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
