#version 310 es

layout(binding = 0, std140)
uniform tint_symbol_1_1_ubo {
  mat3x4 tint_symbol;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat4x3 t = transpose(v.tint_symbol);
  float l = length(v.tint_symbol[1]);
  float a = abs(v.tint_symbol[0].ywxz[0u]);
}
