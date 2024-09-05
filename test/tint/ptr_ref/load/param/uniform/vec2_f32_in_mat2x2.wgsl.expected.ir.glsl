#version 310 es

layout(binding = 0, std140)
uniform tint_symbol_2_std140_1_ubo {
  vec2 tint_symbol_1_col0;
  vec2 tint_symbol_1_col1;
} v;
vec2 func(uint pointer_indices[1]) {
  return mat2(v.tint_symbol_1_col0, v.tint_symbol_1_col1)[pointer_indices[0u]];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  vec2 r = func(uint[1](uint(1)));
}
