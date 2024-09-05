#version 310 es
precision highp float;
precision highp int;


struct S_std140 {
  vec2 matrix_col0;
  vec2 matrix_col1;
  vec2 matrix_col2;
  vec3 vector;
};

layout(binding = 0, std140)
uniform tint_symbol_2_std140_1_ubo {
  S_std140 tint_symbol_1;
} v;
void main() {
  mat3x2 v_1 = mat3x2(v.tint_symbol_1.matrix_col0, v.tint_symbol_1.matrix_col1, v.tint_symbol_1.matrix_col2);
  vec2 x = (v_1 * v.tint_symbol_1.vector);
}
