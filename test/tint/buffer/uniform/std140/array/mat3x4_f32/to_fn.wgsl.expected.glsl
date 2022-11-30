#version 310 es

layout(binding = 0, std140) uniform u_block_ubo {
  mat3x4 inner[4];
} u;

void a(mat3x4 a_1[4]) {
}

void b(mat3x4 m) {
}

void c(vec4 v) {
}

void d(float f_1) {
}

void f() {
  a(u.inner);
  b(u.inner[1]);
  c(u.inner[1][0].ywxz);
  d(u.inner[1][0].ywxz.x);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
