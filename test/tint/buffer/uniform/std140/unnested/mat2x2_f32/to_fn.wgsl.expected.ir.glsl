#version 310 es

layout(binding = 0, std140)
uniform tint_symbol_1_std140_1_ubo {
  vec2 tint_symbol_col0;
  vec2 tint_symbol_col1;
} v_1;
void a(mat2 m) {
}
void b(vec2 v) {
}
void c(float f) {
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  a(mat2(v_1.tint_symbol_col0, v_1.tint_symbol_col1));
  b(mat2(v_1.tint_symbol_col0, v_1.tint_symbol_col1)[1]);
  b(mat2(v_1.tint_symbol_col0, v_1.tint_symbol_col1)[1].yx);
  c(mat2(v_1.tint_symbol_col0, v_1.tint_symbol_col1)[1][0u]);
  c(mat2(v_1.tint_symbol_col0, v_1.tint_symbol_col1)[1].yx[0u]);
}
