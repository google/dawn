#version 310 es

layout(binding = 0, std140) uniform u_block_ubo {
  mat2x3 inner;
} u;

void a(mat2x3 m) {
}

void b(vec3 v) {
}

void c(float f_1) {
}

void f() {
  a(u.inner);
  b(u.inner[1]);
  b(u.inner[1].zxy);
  c(u.inner[1].x);
  c(u.inner[1].zxy.x);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
