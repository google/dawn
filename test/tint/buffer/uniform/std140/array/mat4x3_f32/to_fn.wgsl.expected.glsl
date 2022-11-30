#version 310 es

layout(binding = 0, std140) uniform u_block_ubo {
  mat4x3 inner[4];
} u;

void a(mat4x3 a_1[4]) {
}

void b(mat4x3 m) {
}

void c(vec3 v) {
}

void d(float f_1) {
}

void f() {
  a(u.inner);
  b(u.inner[1]);
  c(u.inner[1][0].zxy);
  d(u.inner[1][0].zxy.x);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
