#version 310 es

layout(binding = 0, std140)
uniform tint_symbol_1_std140_1_ubo {
  vec3 tint_symbol_col0;
  vec3 tint_symbol_col1;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat3x2 t = transpose(mat2x3(v.tint_symbol_col0, v.tint_symbol_col1));
  float l = length(mat2x3(v.tint_symbol_col0, v.tint_symbol_col1)[1]);
  float a = abs(mat2x3(v.tint_symbol_col0, v.tint_symbol_col1)[0].zxy[0u]);
}
