#version 310 es

layout(binding = 0, std140) uniform u_block_ubo {
  mat4x2 inner[4];
} u;

void a(mat4x2 a_1[4]) {
}

void b(mat4x2 m) {
}

void c(vec2 v) {
}

void d(float f_1) {
}

void f() {
  a(u.inner);
  b(u.inner[1]);
  c(u.inner[1][0].yx);
  d(u.inner[1][0].yx.x);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
