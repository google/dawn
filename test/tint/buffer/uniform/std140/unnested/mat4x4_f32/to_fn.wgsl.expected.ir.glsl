#version 310 es

layout(binding = 0, std140)
uniform tint_symbol_1_1_ubo {
  mat4 tint_symbol;
} v_1;
void a(mat4 m) {
}
void b(vec4 v) {
}
void c(float f) {
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  a(v_1.tint_symbol);
  b(v_1.tint_symbol[1]);
  b(v_1.tint_symbol[1].ywxz);
  c(v_1.tint_symbol[1].x);
  c(v_1.tint_symbol[1].ywxz[0u]);
}
