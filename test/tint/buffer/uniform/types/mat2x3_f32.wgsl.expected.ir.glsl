#version 310 es

layout(binding = 0, std140)
uniform tint_symbol_2_std140_1_ubo {
  vec3 tint_symbol_1_col0;
  vec3 tint_symbol_1_col1;
} v;
layout(binding = 1, std430)
buffer tint_symbol_4_1_ssbo {
  mat2x3 tint_symbol_3;
} v_1;
void tint_store_and_preserve_padding(inout mat2x3 target, mat2x3 value_param) {
  target[0u] = value_param[0u];
  target[1u] = value_param[1u];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat2x3 x = mat2x3(v.tint_symbol_1_col0, v.tint_symbol_1_col1);
  tint_store_and_preserve_padding(v_1.tint_symbol_3, x);
}
