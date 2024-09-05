#version 310 es

layout(binding = 0, std140)
uniform tint_symbol_2_std140_1_ubo {
  vec2 tint_symbol_1_col0;
  vec2 tint_symbol_1_col1;
  vec2 tint_symbol_1_col2;
} v;
layout(binding = 1, std430)
buffer tint_symbol_4_1_ssbo {
  mat3x2 tint_symbol_3;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat3x2 x = mat3x2(v.tint_symbol_1_col0, v.tint_symbol_1_col1, v.tint_symbol_1_col2);
  v_1.tint_symbol_3 = x;
}
