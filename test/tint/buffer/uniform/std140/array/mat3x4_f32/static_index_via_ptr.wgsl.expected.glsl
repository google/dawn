#version 310 es

layout(binding = 0, std140) uniform a_block_ubo {
  mat3x4 inner[4];
} a;

layout(binding = 1, std430) buffer s_block_ssbo {
  float inner;
} s;

void f() {
  mat3x4 l_a[4] = a.inner;
  mat3x4 l_a_i = a.inner[2];
  vec4 l_a_i_i = a.inner[2][1];
  s.inner = (((a.inner[2][1].x + l_a[0][0].x) + l_a_i[0].x) + l_a_i_i.x);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
