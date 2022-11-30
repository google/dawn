#version 310 es

layout(binding = 0, std140) uniform a_block_ubo {
  mat3 inner[4];
} a;

void f() {
  mat3 l_a[4] = a.inner;
  mat3 l_a_i = a.inner[2];
  vec3 l_a_i_i = a.inner[2][1];
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
