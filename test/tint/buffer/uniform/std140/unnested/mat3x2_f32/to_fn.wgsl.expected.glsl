#version 310 es

layout(binding = 0, std140) uniform u_block_std140_ubo {
  vec2 inner_0;
  vec2 inner_1;
  vec2 inner_2;
} u;

void a(mat3x2 m) {
}

void b(vec2 v) {
}

void c(float f_1) {
}

mat3x2 load_u_inner() {
  return mat3x2(u.inner_0, u.inner_1, u.inner_2);
}

void f() {
  a(load_u_inner());
  b(u.inner_1);
  b(u.inner_1.yx);
  c(u.inner_1[0u]);
  c(u.inner_1.yx[0u]);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
