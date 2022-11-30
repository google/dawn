#version 310 es

layout(binding = 0, std140) uniform u_block_ubo {
  mat3 inner;
} u;

mat3 p = mat3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
void f() {
  p = u.inner;
  p[1] = u.inner[0];
  p[1] = u.inner[0].zxy;
  p[0][1] = u.inner[1][0];
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
