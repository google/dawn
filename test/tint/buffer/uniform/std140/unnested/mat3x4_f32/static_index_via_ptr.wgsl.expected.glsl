#version 310 es

layout(binding = 0, std140) uniform m_block_ubo {
  mat3x4 inner;
} m;

void f() {
  mat3x4 l_m = m.inner;
  vec4 l_m_1 = m.inner[1];
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
