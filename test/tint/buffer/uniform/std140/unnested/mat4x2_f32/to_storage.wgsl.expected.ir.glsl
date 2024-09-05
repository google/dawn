#version 310 es

layout(binding = 0, std140)
uniform tint_symbol_1_std140_1_ubo {
  vec2 tint_symbol_col0;
  vec2 tint_symbol_col1;
  vec2 tint_symbol_col2;
  vec2 tint_symbol_col3;
} v;
layout(binding = 1, std430)
buffer tint_symbol_3_1_ssbo {
  mat4x2 tint_symbol_2;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v_1.tint_symbol_2 = mat4x2(v.tint_symbol_col0, v.tint_symbol_col1, v.tint_symbol_col2, v.tint_symbol_col3);
  v_1.tint_symbol_2[1] = mat4x2(v.tint_symbol_col0, v.tint_symbol_col1, v.tint_symbol_col2, v.tint_symbol_col3)[0];
  v_1.tint_symbol_2[1] = mat4x2(v.tint_symbol_col0, v.tint_symbol_col1, v.tint_symbol_col2, v.tint_symbol_col3)[0].yx;
  v_1.tint_symbol_2[0][1] = mat4x2(v.tint_symbol_col0, v.tint_symbol_col1, v.tint_symbol_col2, v.tint_symbol_col3)[1][0];
}
