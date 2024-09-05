#version 310 es


struct S {
  int before;
  mat4 m;
  int after;
};

layout(binding = 0, std140)
uniform tint_symbol_1_1_ubo {
  S tint_symbol[4];
} v_1;
void a(S a_1[4]) {
}
void b(S s) {
}
void c(mat4 m) {
}
void d(vec4 v) {
}
void e(float f) {
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  a(v_1.tint_symbol);
  b(v_1.tint_symbol[2]);
  c(v_1.tint_symbol[2].m);
  d(v_1.tint_symbol[0].m[1].ywxz);
  e(v_1.tint_symbol[0].m[1].ywxz[0u]);
}
