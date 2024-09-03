#version 310 es

uniform mat4x2 u;
void a(mat4x2 m) {
}
void b(vec2 v) {
}
void c(float f) {
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  a(u);
  b(u[1]);
  b(u[1].yx);
  c(u[1].x);
  c(u[1].yx[0u]);
}
