#version 310 es

layout(binding = 0, std140) uniform u_block_ubo {
  mat3 inner[4];
} u;

layout(binding = 1, std430) buffer s_block_ssbo {
  float inner;
} s;

mat3 p[4] = mat3[4](mat3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), mat3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), mat3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), mat3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f));
void f() {
  p = u.inner;
  p[1] = u.inner[2];
  p[1][0] = u.inner[0][1].zxy;
  p[1][0].x = u.inner[0][1].x;
  s.inner = p[1][0].x;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
