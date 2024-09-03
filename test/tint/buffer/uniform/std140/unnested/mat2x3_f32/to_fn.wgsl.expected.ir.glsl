#version 310 es

uniform mat2x3 u;
void a(mat2x3 m) {
}
void b(vec3 v) {
}
void c(float f) {
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  a(u);
  b(u[1]);
  b(u[1].zxy);
  c(u[1].x);
  c(u[1].zxy[0u]);
}
