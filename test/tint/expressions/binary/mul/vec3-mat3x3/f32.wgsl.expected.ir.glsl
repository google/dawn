#version 310 es
precision highp float;
precision highp int;


struct S_std140 {
  vec3 matrix_col0;
  vec3 matrix_col1;
  vec3 matrix_col2;
  vec3 vector;
};

layout(binding = 0, std140)
uniform tint_symbol_2_std140_1_ubo {
  S_std140 tint_symbol_1;
} v;
void main() {
  vec3 v_1 = v.tint_symbol_1.vector;
  vec3 x = (v_1 * mat3(v.tint_symbol_1.matrix_col0, v.tint_symbol_1.matrix_col1, v.tint_symbol_1.matrix_col2));
}
