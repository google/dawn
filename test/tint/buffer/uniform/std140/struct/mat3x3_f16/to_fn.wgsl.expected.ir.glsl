#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

struct S {
  int before;
  f16mat3 m;
  int after;
};

uniform S u[4];
void a(S a_1[4]) {
}
void b(S s) {
}
void c(f16mat3 m) {
}
void d(f16vec3 v) {
}
void e(float16_t f) {
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  a(u);
  b(u[2]);
  c(u[2].m);
  d(u[0].m[1].zxy);
  e(u[0].m[1].zxy[0u]);
}
