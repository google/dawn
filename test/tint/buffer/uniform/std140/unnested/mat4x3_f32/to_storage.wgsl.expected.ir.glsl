#version 310 es

layout(binding = 0, std140)
uniform tint_symbol_1_std140_1_ubo {
  vec3 tint_symbol_col0;
  vec3 tint_symbol_col1;
  vec3 tint_symbol_col2;
  vec3 tint_symbol_col3;
} v;
layout(binding = 1, std430)
buffer tint_symbol_3_1_ssbo {
  mat4x3 tint_symbol_2;
} v_1;
void tint_store_and_preserve_padding(inout mat4x3 target, mat4x3 value_param) {
  target[0u] = value_param[0u];
  target[1u] = value_param[1u];
  target[2u] = value_param[2u];
  target[3u] = value_param[3u];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_store_and_preserve_padding(v_1.tint_symbol_2, mat4x3(v.tint_symbol_col0, v.tint_symbol_col1, v.tint_symbol_col2, v.tint_symbol_col3));
  v_1.tint_symbol_2[1] = mat4x3(v.tint_symbol_col0, v.tint_symbol_col1, v.tint_symbol_col2, v.tint_symbol_col3)[0];
  v_1.tint_symbol_2[1] = mat4x3(v.tint_symbol_col0, v.tint_symbol_col1, v.tint_symbol_col2, v.tint_symbol_col3)[0].zxy;
  v_1.tint_symbol_2[0][1] = mat4x3(v.tint_symbol_col0, v.tint_symbol_col1, v.tint_symbol_col2, v.tint_symbol_col3)[1][0];
}
