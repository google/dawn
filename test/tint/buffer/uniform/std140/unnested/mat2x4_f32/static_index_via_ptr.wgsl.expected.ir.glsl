#version 310 es

layout(binding = 0, std140)
uniform tint_symbol_1_1_ubo {
  mat2x4 tint_symbol;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat2x4 l_m = v.tint_symbol;
  vec4 l_m_1 = v.tint_symbol[1];
}
